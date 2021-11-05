/*
----
This file is part of SECONDO.

Copyright (C) 2021,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//[$][\$]

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#include "BasicEngineControl.h"
#include "FileSystem.h"

#include <future>
#include <utility>
#include <iostream>

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
3 Class ~BasicEngine\_Control~

Implementation.

3.1 Constructor

*/
BasicEngineControl::BasicEngineControl(ConnectionGeneric* _dbms_connection, 
    Relation* _workerRelation, std::string _workerRelationName, 
    bool _isMaster) : dbms_connection(_dbms_connection),
    workerRelationName(_workerRelationName), master(_isMaster) {

    // Check relation type
    TupleType* tt = _workerRelation->GetTupleType();

    if(! tt) {
      BOOST_LOG_TRIVIAL(error) << "Unable to get tuple type from relation";
      return;
    }

    if(tt->GetNoAttributes() != 7) {
      BOOST_LOG_TRIVIAL(error) 
        << "Provided relation has to contain 7 attributes "
        << " provided number of attributes: "  << tt->GetNoAttributes();
        return;
    }

    unique_ptr<GenericRelationIterator> it(_workerRelation->MakeScan());
    Tuple* tuple = nullptr;

    while ((tuple = it->GetNextTuple()) != 0) {

      RemoteConnectionInfo* info = new RemoteConnectionInfo();

      info->host = tuple->GetAttribute(0)->toText();
      info->port = tuple->GetAttribute(1)->toText();
      info->config = tuple->GetAttribute(2)->toText();
      info->dbUser = tuple->GetAttribute(3)->toText();
      info->dbPass = tuple->GetAttribute(4)->toText();
      info->dbPort = tuple->GetAttribute(5)->toText();
      info->dbName = tuple->GetAttribute(6)->toText();

      if(tuple != nullptr) {
        tuple->DeleteIfAllowed();
      }

      remoteConnectionInfos.push_back(info);
    }
}

/*
3.1 ~createConnection~

Creating a specified and saves it in the connections vector. 
Additionally add an entry to the importer vector.

*/
ConnectionInfo* BasicEngineControl::createConnection(
    const RemoteConnectionInfo* remoteConnection) {

  string config = string(remoteConnection->config);

  ConnectionInfo* ci = ConnectionInfo::createConnection(
      remoteConnection->host, stoi(remoteConnection->port), 
      config, 
      BasicEngineControl::defaultTimeout, 
      BasicEngineControl::defaultHeartbeat);

  if (ci == nullptr) {  
    BOOST_LOG_TRIVIAL(error) 
        << "Couldn't connect to secondo-Worker on host "
        << remoteConnection->host  
        << " with port " << remoteConnection->port << "!";

    return nullptr;
  } 
  
  bool switchResult = ci->switchDatabase(remoteConnection->dbName, 
    true, false, true, BasicEngineControl::defaultTimeout);

  if(! switchResult) { 
    BOOST_LOG_TRIVIAL(error) 
        << "Unable to switch to database " << remoteConnection->dbName
        << " on host " << remoteConnection->host 
        << " with port " << remoteConnection->port << "!";

    ci->deleteIfAllowed();
    return nullptr;
  }

  return ci;
}

/*
3.2 Destructor

*/
BasicEngineControl::~BasicEngineControl() {
    if(dbms_connection != nullptr) {
      delete dbms_connection;
      dbms_connection = nullptr;
    }

    shutdownAllConnections();

    // Shutdown remote connections
    for(const RemoteConnectionInfo* remoteConnection: remoteConnectionInfos) {
      delete remoteConnection;
    }
}

/*
3.2 ~shutdownAllConnections~

Shutdown all connections

*/
void BasicEngineControl::shutdownAllConnections() {

  // Delete connections
  for(distributed2::ConnectionInfo* ci: connections) {
    BOOST_LOG_TRIVIAL(debug) 
        << "Closing connection to " << ci->getHost() << " / " << ci->getPort();

    ci->deleteIfAllowed();
  }
  connections.clear();
}

/*
3.2 ~initBasicEngineOnWorker~

Init the basic engine on the given worker

*/
bool BasicEngineControl::initBasicEngineOnWorker(ConnectionInfo* ci, 
  const RemoteConnectionInfo* remoteConnectionInfo) {

    string dbType = dbms_connection->getDbType();

    // Call be worker init on remote nodes
    string initCommand = "query be_init('" + dbType + "','"
      + remoteConnectionInfo->dbUser + "','" 
      + remoteConnectionInfo->dbPass + "'," 
      + remoteConnectionInfo->dbPort + ",'"
      + remoteConnectionInfo->dbName + "'," 
      + workerRelationName + ");";
  
    return executeSecondoCommand(ci, initCommand, true);
}

/*
3.2 ~initBasicEngineOnWorker~

Init the basic engine on the given worker

*/
bool BasicEngineControl::executeSecondoCommand(ConnectionInfo* ci, 
  const string &command, const bool checkResult) {

    string errMsg;
    int err = 0;
    double rt;
    string res;
    CommandLog commandLog;

    ci->simpleCommand(command,err,res,false,
      rt,false,commandLog,true, BasicEngineControl::defaultTimeout);

    if(err != 0) {
      BOOST_LOG_TRIVIAL(error) 
        << "Got ErrCode:" << err <<  " / command was: " << command;

      return false;
    } 
    
    if(! checkResult) {
      return true;
    }

    if (res != "(bool TRUE)") {
      BOOST_LOG_TRIVIAL(error) 
        << "Error: Got invalid result from remote node " 
        << res << " / command was: " << command;
    
      return false;
    }
    
    return true;
}

/*
3.2 ~exportWorkerRelationToWorker~

Init the basic engine on the given worker

*/
bool BasicEngineControl::exportWorkerRelationToWorker(ConnectionInfo* ci, 
  const optional<string> &workerRelationFileName) {

      if(! workerRelationFileName.has_value()) {
          BOOST_LOG_TRIVIAL(error) 
            << "We are in master mode, but worker relation is not exported";
          return false;
        }

      CommandLog commandLog;

      return ci->createOrUpdateRelationFromBinFile(
        workerRelationName, workerRelationFileName.value(), false, 
        commandLog, true, false, BasicEngineControl::defaultTimeout);
}

/*
3.2 ~createAllConnections~

Creating all connection from the worker relation.

*/
bool BasicEngineControl::createAllConnections(){

  optional<string> workerRelationFileName = nullopt;

  // Are the connections already present?
  if(! connections.empty()) {
    return true;
  }

  if(remoteConnectionInfos.empty()) {
    BOOST_LOG_TRIVIAL(warning) << "No known remote nodes known"; 
    return false;
  }

  // Share the worker relation with the clients
  if(master) {
    try {
      string exportedFile = exportWorkerRelation(workerRelationName);
      workerRelationFileName.emplace(exportedFile); 
    } catch(std::exception &e) {
      BOOST_LOG_TRIVIAL(error) 
        << "Got an exception during export worker relation" 
        << e.what();
      return false;
    }
  }

  vector<std::future<ConnectionInfo*>> connectionFutures;
  
  // Establish the connections async in futures
  for(const RemoteConnectionInfo* remoteConnectionInfo: 
    remoteConnectionInfos) {

    std::future<ConnectionInfo*> asyncResult = std::async(
        &BasicEngineControl::createAndInitConnection, 
        this, 
        remoteConnectionInfo, 
        workerRelationFileName);
        
    connectionFutures.push_back(std::move(asyncResult));
  }

  // Get future results
  for(future<ConnectionInfo*> &connectionFuture : connectionFutures) {

    ConnectionInfo* ci = connectionFuture.get();

    if (ci != nullptr) {
      connections.push_back(ci);
    }
  }

  // Delete relation file
  if(workerRelationFileName.has_value()){
      FileSystem::DeleteFileOrFolder(workerRelationFileName.value());
      workerRelationFileName.reset();
  }

  if(remoteConnectionInfos.size() != connections.size()) {
    BOOST_LOG_TRIVIAL(error)
         << "Error: Number of worker connections does not match relation size";
    return false;
  }

  return true;
}

ConnectionInfo* BasicEngineControl::createAndInitConnection(
  const RemoteConnectionInfo* remoteConnectionInfo, 
  const optional<string> &workerRelationFileName) {

  ConnectionInfo* ci = createConnection(remoteConnectionInfo);

  if(ci == nullptr) {
    BOOST_LOG_TRIVIAL(error)  
          << "Error: Unable to establish connection to worker: "
          << remoteConnectionInfo->host << " / " << remoteConnectionInfo->port;

    return nullptr;
  }
  
  // Share the worker relation
  if(master) {
    bool exportResult 
      = exportWorkerRelationToWorker(ci, workerRelationFileName);

    if(! exportResult) {
      BOOST_LOG_TRIVIAL(error) 
        << "Error while distributing worker relation to" 
        << ci -> getHost() << " / " << ci -> getPort();
      return nullptr;
    }
  }

  // Init basic engine connection on worker
  bool initResult 
    = initBasicEngineOnWorker(ci, remoteConnectionInfo);

  if(! initResult) {
    BOOST_LOG_TRIVIAL(error) << "Error while init basic engine on" 
      << ci -> getHost() << " / " << ci -> getPort();
    return nullptr;
  }

  return ci;
}

/*
3.5 ~getTableNameForPartitioning~

Returns a name of a table with the keys included.

*/
string BasicEngineControl::getTableNameForPartitioning(
  const string &tab, const string &key) {

  string usedKey(key);

  boost::replace_all(usedKey, ",", "_");
  string res = tab + "_" + usedKey;
  boost::replace_all(res, " ", "");

  return res;
}

/*
3.7 ~exportTableCreateStatementSQL~

Creates a table create statement from the input tab
and store the statement in a file.
Returns true if everything is OK and there are no failure.

*/
void BasicEngineControl::exportTableCreateStatementSQL(
    const string &table, const string &renameExportTable) {

  string filename = getBasePath() + "/" + getSchemaFile(table);

  // Build the create table staement in SQL
  string statement = dbms_connection->getCreateTableSQL(table);

  if (statement.empty()) {
    BOOST_LOG_TRIVIAL(error) << "Unable to get create statement for " << table
                             << " does the table exists?";
    throw SecondoException("Unable to build create statement for " + table +
                           " does the table exists?");
  }

  // Rename the table to create if needed
  if (!renameExportTable.empty()) {
    size_t start_pos = statement.find(table);
    if (start_pos == std::string::npos) {
      throw SecondoException("Unable to replace table " + table);
    }

    statement.replace(start_pos, table.length(), renameExportTable);
  }

  // Write the SQL statement into the given output file
  ofstream write;
  write.open(filename);

  if (!write.is_open()) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't write file into " << filename
                             << ". Please check the folder and permissions.";
    throw SecondoException("Could not open file for writing: " + filename);
  }

  write << statement;
  write.close();
  bool writeResult = write.good();

  if (!writeResult) {
    BOOST_LOG_TRIVIAL(error) << "Writing file " << filename << " failed.";
    throw SecondoException("Unable to write file: " + filename);
  }
}

/*
3.8 ~repartition\_table~

Repartition the given table

*/
bool BasicEngineControl::repartitionTable(PartitionData &partitionData,
  const PartitionMode &repartitionMode) {

   BOOST_LOG_TRIVIAL(debug) << "Repartiton on "
    << partitionData.table << " with mode " << repartitionMode
    << " master: " << master;

    if(! master) {
      return partitionTable(partitionData, repartitionMode, true);
    }

    return repartitionTableMaster(partitionData, repartitionMode);
  }

/**
3.8 Drop the attribute from the given table

*/
  void BasicEngineControl::dropAttributeIfExists(const std::string &table, 
    const std::string &attributeToRemove) {

    std::vector<std::tuple<std::string, std::string>> attributes = 
      dbms_connection -> getTypeFromSQLQuery("SELECT * FROM " + table);


    for(std::tuple<std::string, std::string> attribute : attributes) {
      string attributeName = std::get<0>(attribute);
      if(attributeName == attributeToRemove) {

        BOOST_LOG_TRIVIAL(debug) << "Found partitioning attribute " 
          << attributeToRemove  << " on table " << table 
          << ", removing ";

        dbms_connection -> removeColumnFromTable(table, attributeToRemove);
        return;
      }
    }
  }

/*
3.8 ~repartition\_table\_worker~

Repartition the given table - worker version

*/
  bool BasicEngineControl::partitionTable(PartitionData &partitionData,
                                            const PartitionMode &partitionMode,
                                            const bool repartition) {

    // Open connections
    bool connectionCreateResult = createAllConnections();
    if (!connectionCreateResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to open connections";
      return false;
    }

    // Remove old cellnumber and slotnumber attributes 
    // (e.g., needed for repartition the table)
    dropAttributeIfExists(partitionData.table, be_partition_cellnumber);
    dropAttributeIfExists(partitionData.table, be_partition_slot);

    string resultTable;
    partitionData.key = getFirstAttributeNameFromTable(partitionData.table);

    switch (partitionMode) {
    case PartitionMode::rr:
      resultTable = partRoundRobin(partitionData.table, partitionData.slotnum);
      break;

    case PartitionMode::random:
      resultTable = partFun(partitionData.table, partitionData.key, "random",
                           partitionData.slotnum);
      break;

    case PartitionMode::hash:
      resultTable = partHash(partitionData.table, partitionData.key,
                            partitionData.slotnum);
      break;

    case PartitionMode::grid:
      resultTable = partGrid(partitionData.table, partitionData.key,
                            partitionData.attribute, partitionData.gridname,
                            partitionData.slotnum);
      break;

    case PartitionMode::fun:
      resultTable = partFun(partitionData.table, partitionData.key,
                           partitionData.partitionfun, partitionData.slotnum);
      break;

    default:
      BOOST_LOG_TRIVIAL(error) << "Unknown partition mode" << partitionMode;
      return false;
    }

    // Export data
    bool exportDataResult = exportData(partitionData.table, partitionData.key,
                                       remoteConnectionInfos.size());

    if (!exportDataResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to export table data";
      return false;
    }

    string destinationTable;
    bool transferSchemaFile;

    if (repartition) {
      destinationTable = getRepartitionTableName(partitionData.table);
      transferSchemaFile = false;
    } else {
      destinationTable = partitionData.table;
      exportTableCreateStatementSQL(partitionData.table);
      transferSchemaFile = true;
    }

    // Call transfer and import data
    bool importResult = exportToWorker(partitionData.table, destinationTable,
                                       transferSchemaFile);

    if (!importResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to transfer and import table data";
      return false;
    }

    // Delete the temporary repartition table
    BOOST_LOG_TRIVIAL(debug) << "Deleting temporary table" << resultTable;
    string deleteTable = dbms_connection->getDropTableSQL(resultTable);
    bool deleteResult = dbms_connection->sendCommand(deleteTable);

    if (!deleteResult) {
      BOOST_LOG_TRIVIAL(error)
          << "Unable to delete temporary table" << resultTable;
    }
    
    return true;
  }

/*
3.8 ~repartition\_table\_master~

Repartition the given table - master version

*/
  bool BasicEngineControl::repartitionTableMaster(
    const PartitionData &partitionData,
    const PartitionMode &repartitionMode) {

    string repartTableName = getRepartitionTableName(partitionData.table);

    // On the worker: Drop old re-partition table if exists
    string dropTableSQL = dbms_connection ->getDropTableSQL(repartTableName);
    BOOST_LOG_TRIVIAL(debug) << "Delete old re-parition table: "
      << dropTableSQL;

    bool dropTableResult = mcommand(dropTableSQL);

    if(! dropTableResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
        << dropTableSQL;
        return false;
    }

    // On the worker: Create destination table with same structure
    string copySchemaSQL = dbms_connection->getCopySchemaSQL(
      partitionData.table);

    BOOST_LOG_TRIVIAL(debug) << "Copy schema of table: "
      << copySchemaSQL;

    bool copySchemaResult = mquery(copySchemaSQL, repartTableName);
    if(! copySchemaResult) {
        BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
          << copySchemaSQL;
          return false;
    }

    // On the worker do the real repartiton job
    string repartitionQuery = "query ";
    if(repartitionMode == rr) {
      repartitionQuery.append("be_repart_rr");
      repartitionQuery.append("('" + partitionData.table + "','" 
        + partitionData.key 
        + "'," + to_string(partitionData.slotnum) + ")");
    } else if(repartitionMode == random) {
      repartitionQuery.append("be_repart_random");
      repartitionQuery.append("('" + partitionData.table + "',"
        + to_string(partitionData.slotnum) + ")");
    } else if(repartitionMode == hash) {
      repartitionQuery.append("be_repart_hash");
      repartitionQuery.append("('" + partitionData.table + "','" 
        + partitionData.key 
        + "'," + to_string(partitionData.slotnum) + ")");
    } else if(repartitionMode == grid) {
      repartitionQuery.append("be_repart_grid");
      repartitionQuery.append("('" + partitionData.table + "','"
      + partitionData.key + "','" + partitionData.attribute + "','"
      + partitionData.gridname + "',"
      + to_string(partitionData.slotnum) + ")");
    } else if(repartitionMode == fun) {
      repartitionQuery.append("be_repart_fun");
      repartitionQuery.append("('" + partitionData.partitionfun + "','" 
        + partitionData.table + "',"
        + to_string(partitionData.slotnum) + ")");
    } else {
      BOOST_LOG_TRIVIAL(error) << "Unsupported repartition mode: "
        << repartitionMode;
      return false;
    }

    BOOST_LOG_TRIVIAL(debug) << "Execute repartition job on worker: "
      << repartitionQuery;

    bool partitioningResult = msecondocommand(repartitionQuery);

    if(! partitioningResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
        << repartitionQuery;
        return false;
    }

    // On the worker: Drop source table
    string dropSourceTableSQL = dbms_connection 
      -> getDropTableSQL(partitionData.table);

    BOOST_LOG_TRIVIAL(debug) << "Delete source table: "
      << dropSourceTableSQL;

    bool dropSourceTableResult = mcommand(dropSourceTableSQL);

    if(! dropSourceTableResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
        << dropSourceTableSQL;
        return false;
    }

    // On the worker: Rename destination table to source table 
    string renameTableSQL = dbms_connection ->getRenameTableSQL(
      repartTableName, partitionData.table);
    BOOST_LOG_TRIVIAL(debug) << "Rename table: " << renameTableSQL;

    bool renameTableResult = mcommand(renameTableSQL);

    if(! renameTableResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
        << renameTableSQL;
        return false;
    }

    return true;
}

/*
3.8 ~partition\_table\_by\_hash~

Repartition the given table by round hash

*/
bool BasicEngineControl::partitionTableByHash(const std::string &table, 
  const std::string &key, const size_t slotnum, const bool repartition) {

    BOOST_LOG_TRIVIAL(debug) << "Partiton by hash called on " << table;

    PartitionData partitionData = {};
    partitionData.table = table;
    partitionData.key = key;
    partitionData.slotnum = slotnum;

    if(repartition) {
      return repartitionTable(partitionData, hash);
    } else {
      return partitionTable(partitionData, hash, false);
    }
}

/*
3.8 ~partition\_table\_by\_rr~

Repartition the given table by round robin

*/
bool BasicEngineControl::partitionTableByRR(const std::string &table, 
  const size_t slotnum, const bool repartition) {
    
    BOOST_LOG_TRIVIAL(debug) << "Partiton by rr called on " << table;

    PartitionData partitionData = {};
    partitionData.table = table;
    partitionData.slotnum = slotnum;

    if(repartition) {
      return repartitionTable(partitionData, rr);
    } else {
      return partitionTable(partitionData, rr, false);
    }
}

/*
3.8 ~partition\_table\_by\_random~

Repartition the given table by random

*/
bool BasicEngineControl::partitionTableByRandom(const std::string &table, 
  const size_t slotnum, const bool repartition) {
    
    BOOST_LOG_TRIVIAL(debug) << "Partiton by random called on " << table;

    PartitionData partitionData = {};
    partitionData.table = table;
    partitionData.slotnum = slotnum;

    if(repartition) {
      return repartitionTable(partitionData, random);
    } else {
      return partitionTable(partitionData, random, false);
    }
}

/*
3.8 ~partition\_table\_by\_random~

Repartition the given table by random

*/
bool BasicEngineControl::partitionTableByFun(const std::string &table, 
    const std::string &key, const std::string &partitionfun, 
    const size_t slotnum, const bool repartition) {

    PartitionData partitionData = {};
    partitionData.table = table;
    partitionData.key = key;
    partitionData.partitionfun = fun;
    partitionData.slotnum = slotnum;
    
    if(repartition) {
      return repartitionTable(partitionData, fun);
    } else {
      return partitionTable(partitionData, fun, false);
    }
}

/*
3.8 ~partition\_table\_by\_grid~

Repartition the given table by grid

*/    
bool BasicEngineControl::partitionTableByGrid(const std::string &table, 
    const std::string &key, const size_t slotnum, 
    const std::string &attribute, const std::string &gridname, 
    const bool repartition) {
    
    PartitionData partitionData = {};
    partitionData.table = table;
    partitionData.key = key;
    partitionData.attribute = attribute;
    partitionData.gridname = gridname;
    partitionData.slotnum = slotnum;

    if(repartition) {
      return repartitionTable(partitionData, grid);
    } else {
      return partitionTable(partitionData, grid, false);
    }
}

/*
3.9 ~exportToWorker~

The data of a partitions table were sended to the worker
and after that imported by the worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::exportToWorker(const string &sourceTable, 
  const string &destinationTable, const bool exportSchema) {


  if(connections.size() != remoteConnectionInfos.size()) {
    BOOST_LOG_TRIVIAL(error) << "Not all connections are available";
    return false;
  }

  string remoteCreateName = getSchemaFile(sourceTable);
  string localCreateName = getBasePath() + "/" + remoteCreateName;

  for(size_t index = 0; index < connections.size(); index++){
    
    SecondoInterfaceCS* si = connections[index]->getInterface();

    //sending data
    string strindex = to_string(index);
    string remoteName 
      = dbms_connection -> getFilenameForPartition(sourceTable, strindex);
    string localName = getBasePath() + "/" + remoteName;

    int sendFileRes = si->sendFile(localName, remoteName, true);

    if (sendFileRes != 0) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't send the data to the worker: "
        << " Localfile: " << localName << " Remotefile: "  << remoteName
        << " Return code: " << sendFileRes;
      return false;
    }

    bool removeFileRes = (remove(localName.c_str()) == 0);

    if (!removeFileRes) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't remove the local file:" 
        << localName;
      return false;
    }

    //sending create Table
    if(exportSchema) {
      int sendFileRes = si->sendFile(localCreateName, remoteCreateName, true);

      if (sendFileRes != 0) {
        BOOST_LOG_TRIVIAL(error) 
          << "Couldn't send the structure-file to the worker: " 
          << " Localfile: " << localCreateName 
          << " Remotefile: "  << remoteCreateName;
        return false;
      }
    }
  }

  // Remove schema file
  remove(localCreateName.c_str());

  //doing the import with one thread for each worker
  vector<std::future<bool>> futures;

  for(size_t index = 0; index < connections.size(); index++){
    distributed2::ConnectionInfo* ci = connections[index];
    string strindex = to_string(index);
    string remoteName = dbms_connection 
      -> getFilenameForPartition(sourceTable, strindex);

    std::future<bool> asyncResult = std::async(
      &BasicEngineControl::performImport, 
      this, ci, destinationTable, remoteCreateName, 
      remoteName, exportSchema);

    futures.push_back(std::move(asyncResult));
  }

  // Are all worker reporting true?
  bool exportRes = std::all_of(futures.begin(), futures.end(), 
    [] (std::future<bool> &future) { return future.get(); });

  if(! exportRes) {
    BOOST_LOG_TRIVIAL(error) 
      << "Something goes wrong with the import at the worker.";
      return false;
  }

  return true;
}

/*
3.8 ~partRoundRobin~

The data were partitions in the database by round robin.
Returns true if everything is OK and there are no failure.

*/
string BasicEngineControl::partRoundRobin(const string &table,
                    size_t numberOfSlots) {
  
  string destinationTable = getTableNameForPartitioning(table, "random");
  dropTable(destinationTable);

  dbms_connection->partitionRoundRobin(table, numberOfSlots, destinationTable);

  return destinationTable;
}

/*
3.10 ~partHash~

The data were partitions in the database by an hash value.
Returns true if everything is OK and there are no failure.

*/
string BasicEngineControl::partHash(const string &tab,
                    const string &key, size_t slotnum) {

  string partTabName = getTableNameForPartitioning(tab, key);
  dropTable(partTabName);

  dbms_connection->partitionHash(tab, key, slotnum, partTabName);
  
  return partTabName;
}

/*
3.11 ~partFun~

The data were partitions in the database by an defined function.
This function have to be defined before using it.

*/
string BasicEngineControl::partFun(const string &tab,
    const string &key, const string &fun, size_t slotnum){
                
  string partTabName = getTableNameForPartitioning(tab, key);
  dropTable(partTabName);

  dbms_connection->partitionFunc(tab, key, slotnum, fun, partTabName);

  return partTabName;
}


/*
3.20 ~partGrid~

The data were partitions in the database by a grid.
Returns true if everything is OK and there are no failure.

*/
string BasicEngineControl::partGrid(const std::string &tab, 
  const std::string &key, const std::string &geo_col, 
  const std::string &gridName, size_t slotnum) {

  string gridTable = "grid_" + gridName;

  // Dropping parttable
  string partTabName = getTableNameForPartitioning(tab,key);
  dropTable(partTabName);

  // Drop old index if exists (ignore failure when index does not exists)
  string dropSQL = dbms_connection->getDropIndexSQL(tab, geo_col);
  bool dropIndexRes = dbms_connection->sendCommand(dropSQL, false);

  if(! dropIndexRes) {
    BOOST_LOG_TRIVIAL(debug) 
      << "Dropping index failed, ignoring. Maybe it does not exist.";
  } 

  dbms_connection->partitionGrid(tab, key, geo_col, slotnum, 
    gridTable, partTabName);

  return partTabName;
} 

/*
3.12 ~exportData~

Exporting the data from the DBMS to a local file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::exportData(const string &table, 
  const string &key, size_t slotnum){

  bool val = true;
  string path = getBasePath() + "/";
  string parttabname = getTableNameForPartitioning(table, key);
  string strindex;

  for(size_t i=0; i<remoteConnectionInfos.size(); i++) {
    strindex = to_string(i);

    string exportFile 
      = dbms_connection -> getFilenameForPartition(table, strindex);

    string exportDataSQL = dbms_connection->getExportDataSQL(table,
          parttabname, key, strindex, exportFile, slotnum);

    BOOST_LOG_TRIVIAL(debug) << "Export table from DB: "s
      << exportDataSQL;
        
    val = sendCommand(exportDataSQL) && val;
  }

  return val;
}

/*
3.13 ~importData~

Importing data from a local file into the dbms. At first the
table will be created and after that starts the import from a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::importData(const string &table) {

  bool result = true;

  // create table structure
  string structureFile = getBasePath() + "/" + getSchemaFile(table);

  // Read data into memory
  ifstream inFile;
  stringstream strStream;
  inFile.open(structureFile);
  strStream << inFile.rdbuf();
  string cmd = strStream.str();

  result = dbms_connection->sendCommand(cmd);
  
  if(! result) {
    return false;
  }

  FileSystem::DeleteFileOrFolder(structureFile);

  string basePath = getBasePath();

  //import data (local files from worker)
  for(size_t i=0; i<remoteConnectionInfos.size(); i++){
    string strindex = to_string(i);
    
    string partitionFile = basePath + "/" +
      dbms_connection -> getFilenameForPartition(table, strindex);

    result = importTable(partitionFile, table) && result;
    FileSystem::DeleteFileOrFolder(partitionFile);
  }

  return result;
}

/*
3.15 ~munion~

Exports the data from the worker and sending them to the master.
The master imports them into the local db.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::munion(const string &table) {

  bool val = true;
  int workerId = 0;
  vector<std::future<bool>> futures;
  string basePath = getBasePath();

  //doing the export with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {

    string partitionFile 
      = dbms_connection -> getFilenameForPartition(table, to_string(workerId));
    string exportFile = basePath + "/" + partitionFile;

    std::future<bool> asyncResult = std::async(
      &BasicEngineControl::performExport, 
      this, ci, workerId, table, exportFile, partitionFile);
        
    futures.push_back(std::move(asyncResult));

    workerId++;
  }

  //waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    val = future.get() && val;
  }

  //import in local database
  if(val) {
    BOOST_LOG_TRIVIAL(debug) << "Starting data import on master";
    val = importData(table);
  }

  return val;
}

/*
3.16 ~mquery~

The multi query sends and execute a query to all worker
and stores the result in a table.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::mquery(const string &query,
                    const string &table) {

  // Perform query check
  if(connections.empty()) {
    cerr << "Unable to perform mquery, "
         << "no connections are known" << endl;
    return false;
  }

  // Perform the query on the first worker to do a kind of 
  // type checking
  //
  // 1) When the query returns an error, stop processing at 
  //    this point. 
  // 2) When the query returns a success result within 
  //    one second, start the query on the remaining workers.
  // 3) When the query runs for more then one second, assume 
  //    that the query is ok and start them on the remaining
  //    workers.

  distributed2::ConnectionInfo* validateConnection 
    = connections[0];

  vector<std::future<bool>> futures;

  std::future<bool> asyncResult = std::async(
    &BasicEngineControl::performBEQuery, 
    this, validateConnection, table, query);

  bool queryOk = true;

  BOOST_LOG_TRIVIAL(debug) 
      << "Waiting up to 1 sec for first result";

  std::future_status futureStatus 
    = asyncResult.wait_for(std::chrono::seconds(1));
  
  // Query result after one second
  if (futureStatus == std::future_status::ready) {
    queryOk = asyncResult.get();
    
    BOOST_LOG_TRIVIAL(debug) 
      << "Query result from first worker is: " << queryOk;

    if(queryOk == false) {
      return false;
    }
  } else {
    BOOST_LOG_TRIVIAL(debug) 
      << "Query on master is still active, starting query on workers";
    futures.push_back(std::move(asyncResult));
  }
  
  BOOST_LOG_TRIVIAL(debug) << "Starting query on worker";

  // Perform the query on the remaining workers
  for(auto it = connections.begin() + 1; 
      it != connections.end(); it++) {

    std::future<bool> asyncResult = std::async(
    &BasicEngineControl::performBEQuery, 
    this, *it, table, query);
        
    futures.push_back(std::move(asyncResult));
  }

  // Waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    queryOk = future.get() && queryOk;
  }

  return queryOk;
}

/*
3.17 ~mcommand~

The multi command sends and execute a query to all worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::mcommand(const string &query) {

  bool val = true;
  vector<std::future<bool>> futures;

  //doing the command with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {
    std::future<bool> asyncResult = std::async(
    &BasicEngineControl::performBECommand, 
    this, ci, query);
        
    futures.push_back(std::move(asyncResult));
  }

  //waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    val = future.get() && val;
  }

  return val;
}

/*
3.17 ~msecondocommand~

The multi command sends and execute a query to all worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::msecondocommand(const string &query) {

  bool val = true;
  vector<std::future<bool>> futures;
 
  //doing the command with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {
    std::future<bool> asyncResult = std::async(
    &BasicEngineControl::performSimpleSecondoCommand, 
    this, ci, query);
        
    futures.push_back(std::move(asyncResult));
  }

  //waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    val = future.get() && val;
  }

  return val;
}

/*
3.17 ~shutdownWorker~

Shutdown the remote worker

*/
bool BasicEngineControl::shutdownWorker() {

   bool result = true;
   string shutdownCommand("query be_shutdown()");

   for(distributed2::ConnectionInfo* ci : connections) {
     result = result && performSimpleSecondoCommand(ci, shutdownCommand);
   }

   return result;
}


/*
3.18 ~checkAllConnections~

Checking the Connection to the secondary Master System and to the Worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::checkAllConnections() {

  const int defaultTimeout = 0;

  //checking connection to the worker
  if (connections.size() != remoteConnectionInfos.size()) {
    return false;
  }

  for(size_t i = 0; i < remoteConnectionInfos.size(); i++) {
    CommandLog commandLog;
    
    bool connectionState = connections[i]->check(
      false, commandLog, defaultTimeout);
    
    if(!connectionState) {
      return false;
    }
  }

  //checking the connection to the secondary dbms system
  bool localConnectionState = dbms_connection->checkConnection();
  return localConnectionState;
}

/*
3.19 ~runsql~

Runs a query from a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngineControl::runsql(const string &filepath) {

  if (access(filepath.c_str(), 0) != 0) {
    BOOST_LOG_TRIVIAL(error) 
      << "Couldn't find the file at path:" + filepath;
    return false;
  }

  // Read file into memory
  ifstream inFile;
  stringstream strStream;

  inFile.open(filepath);
  strStream << inFile.rdbuf();
  
  // Split up SQL queries from file and execute them
  string query;
  while (getline(strStream, query, ';')) {
    bool result = dbms_connection->sendCommand(query);

    if(! result) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute: " << query;
      return false;
    }
  }

  return true;
}

/*
3.21 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 ListExpr BasicEngineControl::getTypeFromSQLQuery(
   const std::string &sqlQuery) {

    std::vector<std::tuple<std::string, std::string>> types = 
      dbms_connection->getTypeFromSQLQuery(sqlQuery);

   return dbms_connection->convertTypeVectorIntoSecondoNL(types);
 }


/*
3.22 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 ResultIteratorGeneric* BasicEngineControl::performSQLSelectQuery(
   const std::string &sqlQuery) {

   return dbms_connection->performSQLSelectQuery(sqlQuery);
 }


/*
3.23 ~exportWorkerRelation~

Export the worker relation into a file.

*/
string BasicEngineControl::exportWorkerRelation(
  const string &relationName) {

  // Output file
  string filename = relationName + "_" 
                     + stringutils::int2str(WinUnix::getpid()) 
                     + ".bin";


  // Get type for secondo object
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  string tn;
  bool defined;
  bool hasTypeName;
  ListExpr typeList;
  Word value;
  value.setAddr(0);

  try {
    if(!ctlg->GetObjectExpr(relationName, tn, typeList, value, 
                            defined, hasTypeName)) {
      throw SecondoException("Error: Name " 
        + relationName + " is not on object");
    }

    if(!defined){
      throw SecondoException("Error: Undefined objects cannot be shared");
    }
    
    // Write relation to file
    bool isRelation = Relation::checkType(typeList);

    if(! isRelation) {
      throw SecondoException("Error: provided relation name is not a relation");
    }

    ConnectionInfo::saveRelationToFile(typeList, value, filename);

  } catch(std::exception &e) {
  
    if(value.addr){
      SecondoSystem::GetCatalog()->CloseObject(typeList, value);
      value.setAddr(0);
    }

    throw;
  }

  if(value.addr){
    SecondoSystem::GetCatalog()->CloseObject(typeList, value);
    value.setAddr(0);
  }

  return filename;
}


/*
3.24 ~performImport~

Starting the data import operation.

*/
bool BasicEngineControl::performImport(
      distributed2::ConnectionInfo* ci,
      const std::string &table,
      const std::string &remoteCreateName,
      const std::string &remoteName,
      const bool importSchema) {

  std::string importPath;
  std::string cmd;
  bool result = true;

  if(importSchema) {
    importPath =ci->getSendPath() + "/"+ remoteCreateName;
    cmd = "query be_runsql('"+ importPath + "');";
    result = performSimpleSecondoCommand(ci, cmd);

    //delete create-file on system
    cmd = "query removeFile('"+ importPath + "')";
    if (result) {
      result = performSimpleSecondoCommand(ci, cmd);
    }
  }

  //import data in pg-worker
  if(result) {
    importPath = ci->getSendPath() + "/"+ remoteName;
    cmd = "query be_copy('"+ importPath + "','" + table + "')";
    result = performSimpleSecondoCommand(ci, cmd);
  }

  //delete data file on system
  if(result) {
    cmd = "query removeFile('"+ importPath + "')";
    result = performSimpleSecondoCommand(ci, cmd);
  }

  return result;
}

/*
3.25 ~requestRemoteSchema~

Request the remote schema for a table

*/
string BasicEngineControl::requestRemoteTableSchema(
    const std::string &table, distributed2::ConnectionInfo *ci) {

  string basePath = getBasePath();
  string schemaFile = getSchemaFile(table);

  // export tab structure
  string cmd = "query be_struct('" + table + "');";
  bool result = performSimpleSecondoCommand(ci, cmd);

  if (!result) {
    BOOST_LOG_TRIVIAL(error) << "Unable to execute SECONDO command" << cmd;
  }

  // move the structure-file into the request-folder
  string from = basePath + "/" + schemaFile;
  string to = ci->getRequestPath() + "/" + schemaFile;
  cmd = "query moveFile('" + from + "','" + to + "')";
  result = performSimpleSecondoCommand(ci, cmd);

  if (!result) {
    BOOST_LOG_TRIVIAL(error) << "Unable to execute SECONDO command" << cmd;
  }

  // Requesting file from worker
  result = (ci->requestFile(schemaFile, from, true) == 0);

  if (!result) {
    BOOST_LOG_TRIVIAL(error)
        << "Error while requesting struct file" << schemaFile << " / " << from;
  }

  // delete create file on system
  cmd = "query removeFile('" + to + "')";
  result = performSimpleSecondoCommand(ci, cmd);

  if (!result) {
    BOOST_LOG_TRIVIAL(error)
        << "Error while requesting struct file" << schemaFile << " / " << from;
  }

  string exportedSchemaFile = basePath + "/" + schemaFile;

  BOOST_LOG_TRIVIAL(debug) << "Schema of table " << table << " exported to "
                           << exportedSchemaFile;

  return exportedSchemaFile;
}

/*
3.25 ~performExport~

Perform the data export operation.

*/
bool BasicEngineControl::performExport(
      distributed2::ConnectionInfo* ci,
      size_t workerId,
      const std::string &table, 
      const std::string &exportFile, 
      const std::string &partitionFile) {

  std::string from;
  std::string to;
  std::string cmd;
  string basePath = getBasePath();
  bool result = true;

  //export the table structure file
  if(workerId == 0) {
    try {
      requestRemoteTableSchema(table, ci);
    } catch(SecondoException &e) {
      BOOST_LOG_TRIVIAL(error) 
        << "Unable to request remote schema" << e.what();
      return false;
    }
  }

  //export the date to a file
  cmd = "query be_copy('"+ table + "','"+ exportFile + "');";
  result = performSimpleSecondoCommand(ci, cmd);
  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
        << "Unable to execute SECONDO command" << cmd;
    return false;
  }

  //move the data-file to the request-folder
  to = ci->getRequestPath() + "/" + partitionFile;
  cmd = "query moveFile('"+ exportFile + "','" + to +"')";
  result = performSimpleSecondoCommand(ci, cmd);

  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
        << "Unable to execute SECONDO command" << cmd;
    return false;
  }

  //sendig the File to the master
  result =(ci->requestFile(partitionFile,
                        basePath + "/" + partitionFile ,true)==0);

  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
        << "Error while requesting export file"
        << partitionFile << " / " << basePath + "/" + partitionFile;
      return false;
  }
  
  //delete data file on system
  cmd = "query removeFile('"+ to + "')";
  result = performSimpleSecondoCommand(ci, cmd);
  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
        << "Unable to execute SECONDO command" << cmd;
    return false;
  }

  return true;
}

/*
3.26 ~runBEQuery~

Starting a query at the worker.

*/
bool BasicEngineControl::performBEQuery(
      distributed2::ConnectionInfo* ci,
      const std::string &table, 
      const std::string &query) {
  
  std::string escapedQuery(query);

  boost::replace_all(escapedQuery, "'", "\\'");

  //run the query
  std::string cmd = "query be_query('"
         "" + escapedQuery + "','"
         "" + table + "');";

  return performSimpleSecondoCommand(ci, cmd);
}

/*
3.27 ~runBECommand~

Starting a command at the worker.

*/
bool BasicEngineControl::performBECommand(
      distributed2::ConnectionInfo* ci,
      const std::string &command) {

  std::string escapedCommand(command);

  //run the command
  boost::replace_all(escapedCommand, "'", "\\'");
  std::string cmd = "query be_command('" + escapedCommand + "');";

  return performSimpleSecondoCommand(ci, cmd);
}

/*
3.28 ~simpleCommand~

Execute a command or query on the worker.

Returns true if everything is OK and there are no failure.
Displays an error massage if something goes wrong.

*/
bool BasicEngineControl::performSimpleSecondoCommand(
      distributed2::ConnectionInfo* ci, const std::string &command) {
  
  int err = 0;
  double rt;
  const int defaultTimeout = 0;
  distributed2::CommandLog CommandLog;
  std::string res;

  ci->simpleCommand(command, err, res, false, rt, false,
                    CommandLog, true, defaultTimeout);
  
  if(err != 0){
    BOOST_LOG_TRIVIAL(error)
        << "Got error from server: " 
        << ci->getHost() << ":" << ci->getPort() << " "
        << err << res << " command was: " << command;
    return false;
  }

  bool resultOk = (res == "(bool TRUE)");

  if(! resultOk) {
    BOOST_LOG_TRIVIAL(error)
        << "Got unexpected result from server: " 
        << ci->getHost() << ":" << ci->getPort() << " "
        << res << " command was: " << command;
    return false;
  }

  return true;
}


/*
3.27 ~shareTable~

Share the given table with all workers

*/
bool BasicEngineControl::shareTable(
      const std::string &table) {

  // Create trlation schema file
  exportTableCreateStatementSQL(table);

  // Export complete relation and duplicate as slots for the worker
  string path = getBasePath();
  string partZeroFile = dbms_connection -> getFilenameForPartition(table, "0");
  string partZeroFullPath = path + "/" + partZeroFile;
  string exportDataSQL = dbms_connection->getExportTableSQL(
    table, partZeroFullPath);

  bool exportDataRes = sendCommand(exportDataSQL);

  if(!exportDataRes) {
    BOOST_LOG_TRIVIAL(error) << "Unable to export from DB: "
      << exportDataSQL;

    return false;
  }

  // Copy parition 0 to partitions [1-n]
  for(size_t i = 1; i<remoteConnectionInfos.size(); i++) {
    string partitionFile = 
      dbms_connection -> getFilenameForPartition(table, to_string(i));
    string partitionFileFullPath = path + "/" + partitionFile;

    BOOST_LOG_TRIVIAL(debug) << "Copy file " << partZeroFullPath 
      << " to " << partitionFileFullPath;

    ifstream src(partZeroFullPath, std::ios::binary);
    ofstream dst(partitionFileFullPath, std::ios::binary);
    dst << src.rdbuf();
  }

  // Transfer the data to the worker
  bool transferRes = exportToWorker(table, table, true);

  if(! transferRes) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't export the data to the worker";
    return false;
  }

  return true;
}

/**
3.28 Validate the given query

*/
bool BasicEngineControl::validateQuery(const std::string &sqlQuery) {

  bool validateResult = dbms_connection->validateQuery(sqlQuery);

  if(! validateResult) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to validate SQL query: " << sqlQuery;

    return false;
  }

  return true;
}

/**
3.29 Get the first attribute name from the given table

*/
std::string BasicEngineControl::getFirstAttributeNameFromTable(
    const std::string &table) {
    
   vector<tuple<string, string>> attrs 
    = dbms_connection->getTypeFromSQLQuery("SELECT * FROM " + table + ";");

  if(attrs.empty()) {
    BOOST_LOG_TRIVIAL(error) << "Unable to determine table layout for " 
      << table;
    throw SecondoException("Unable to determine table layout for " 
      + table);
  }

  tuple<string, string> firstAttibute = attrs[0];
  string key = std::get<0>(firstAttibute);

  BOOST_LOG_TRIVIAL(debug) << "Using key " + key + " for rr join";

  return key;
}

} /* namespace BasicEngine */
