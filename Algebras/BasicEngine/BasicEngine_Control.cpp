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
#include "BasicEngine_Control.h"
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
BasicEngine_Control::BasicEngine_Control(ConnectionGeneric* _dbms_connection, 
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
ConnectionInfo* BasicEngine_Control::createConnection(
    const RemoteConnectionInfo* remoteConnection) {

  string config = string(remoteConnection->config);

  ConnectionInfo* ci = ConnectionInfo::createConnection(
      remoteConnection->host, stoi(remoteConnection->port), 
      config, 
      BasicEngine_Control::defaultTimeout, 
      BasicEngine_Control::defaultHeartbeat);

  if (ci == nullptr) {  
    BOOST_LOG_TRIVIAL(error) 
        << "Couldn't connect to secondo-Worker on host "
        << remoteConnection->host  
        << " with port " << remoteConnection->port << "!";

    return nullptr;
  } 
  
  bool switchResult = ci->switchDatabase(remoteConnection->dbName, 
    true, false, true, BasicEngine_Control::defaultTimeout);

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
BasicEngine_Control::~BasicEngine_Control() {
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
void BasicEngine_Control::shutdownAllConnections() {

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
bool BasicEngine_Control::initBasicEngineOnWorker(ConnectionInfo* ci, 
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
bool BasicEngine_Control::executeSecondoCommand(ConnectionInfo* ci, 
  const string &command, const bool checkResult) {

    string errMsg;
    int err = 0;
    double rt;
    string res;
    CommandLog commandLog;

    ci->simpleCommand(command,err,res,false,
      rt,false,commandLog,true, BasicEngine_Control::defaultTimeout);

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
bool BasicEngine_Control::exportWorkerRelationToWorker(ConnectionInfo* ci, 
  const optional<string> &workerRelationFileName) {

      if(! workerRelationFileName.has_value()) {
          BOOST_LOG_TRIVIAL(error) 
            << "We are in master mode, but worker relation is not exported";
          return false;
        }

      CommandLog commandLog;

      return ci->createOrUpdateRelationFromBinFile(
        workerRelationName, workerRelationFileName.value(), false, 
        commandLog, true, false, BasicEngine_Control::defaultTimeout);
}

/*
3.2 ~createAllConnections~

Creating all connection from the worker relation.

*/
bool BasicEngine_Control::createAllConnections(){

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
        &BasicEngine_Control::createAndInitConnection, 
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

ConnectionInfo* BasicEngine_Control::createAndInitConnection(
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
string BasicEngine_Control::getTableNameForPartitioning(
  const string &tab, const string &key) {

  string usedKey(key);

  boost::replace_all(usedKey, ",", "_");
  string res = tab + "_" + usedKey;
  boost::replace_all(res, " ", "");

  return res;
}

/*
3.7 ~getCreateTableSQL~

Creates a table create statement from the input tab
and store the statement in a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::getCreateTableSQL(const string &tab) {

  ofstream write;
  string statement;
  bool val = false;

  statement = dbms_connection->getCreateTableSQL(tab);

  if (statement.length() > 0){
    write.open(getFilePath() + getCreateTableSQLName(tab));
    if (write.is_open()){
      write << statement;
      write.close();
      val = write.good();
    } else { 
      BOOST_LOG_TRIVIAL(error) 
        << "Couldn't write file into "<< getFilePath() 
        << ". Please check the folder and permissions.";
    }
  } else { 
     BOOST_LOG_TRIVIAL(error) << "Table " << tab << " not found.";
  }

   return val;
}

/*
3.8 ~partRoundRobin~

The data were partitions in the database by round robin.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partRoundRobin(const string &tab,
                    const string &key, size_t slotnum) {

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);

  partTabName = getTableNameForPartitioning(tab, key);
  drop_table(partTabName);

  query_exec = dbms_connection->getPartitionRoundRobinSQL(tab, key,
      anzSlots, partTabName);
  
  if (query_exec != "") {
    val = dbms_connection->sendCommand(query_exec);
  }

  return val;
}

/*
3.8 ~repartition\_table~

Repartition the given table

*/
bool BasicEngine_Control::repartition_table(const std::string &table, 
  const std::string &key, const size_t slotnum, 
  const RepartitionMode &repartitionMode) {

   BOOST_LOG_TRIVIAL(debug) << "Repartiton on "
    << table << " with mode " << repartitionMode
    << " master: " << master;

    if(! master) {
      return repartition_table_worker(table, key, slotnum, repartitionMode);
    }

    return repartition_table_master(table, key, slotnum, repartitionMode);
  }

/*
3.8 ~repartition\_table\_worker~

Repartition the given table - worker version

*/
  bool BasicEngine_Control::repartition_table_worker(const std::string &table, 
    const std::string &key, const size_t slotnum, 
    const RepartitionMode &repartitionMode) {

    // Open connections
    bool connectionCreateResult = createAllConnections();
    if(!connectionCreateResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to open connections";
      return false;
    }

    string repartTableName = getRepartitionTableName(table);

    // On the worker: Create export table
    if(repartitionMode == rr) {
      bool partResult = partRoundRobin(table, key, slotnum);
      if(! partResult) {
          BOOST_LOG_TRIVIAL(error) << "Unable to partition table";
          return false;
      }
    } else if(repartitionMode == hash) {
      bool partResult = partHash(table, key, slotnum);
      if(! partResult) {
          BOOST_LOG_TRIVIAL(error) << "Unable to partition table";
          return false;
      }
    } else {
      BOOST_LOG_TRIVIAL(error) << "Unknown partition mode" << repartitionMode;
      return false;
    }

    // On the worker: Export data
    bool exportDataResult 
      = exportData(table, key, remoteConnectionInfos.size());
    
    if(! exportDataResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to export table data";
      return false;
    }

    // On the worker: Call transfer and import data
    bool importResult = exportToWorker(table, repartTableName, false);
    if(! importResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to transfer and import table data";
      return false;
    }

    return true;
  }


/*
3.8 ~repartition\_table\_master~

Repartition the given table - master version

*/
  bool BasicEngine_Control::repartition_table_master(const std::string &table, 
    const std::string &key, const size_t slotnum, 
    const RepartitionMode &repartitionMode) {

    string repartTableName = getRepartitionTableName(table);

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
    string copySchemaSQL = dbms_connection->getCopySchemaSQL(table);
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
    } else if(repartitionMode == hash) {
      repartitionQuery.append("be_repart_hash");
    }

    repartitionQuery.append("('" + table + "','" + key 
      + "'," + to_string(slotnum) + ")");
    BOOST_LOG_TRIVIAL(debug) << "Execute repartition job on worker: "
      << repartitionQuery;

    bool partitioningResult = msecondocommand(repartitionQuery);

    if(! partitioningResult) {
      BOOST_LOG_TRIVIAL(error) << "Unable to execute on worker: "
        << repartitionQuery;
        return false;
    }

    // On the worker: Drop source table
    string dropSourceTableSQL = dbms_connection ->getDropTableSQL(table);
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
      repartTableName, table);
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
3.8 ~repartition\_table\_by\_hash~

Repartition the given table by round hash

*/
bool BasicEngine_Control::repartition_table_by_hash(const std::string &table, 
  const std::string &key, const size_t slotnum) {

    BOOST_LOG_TRIVIAL(debug) << "Repartiton by hash called on " << table;

    return repartition_table(table, key, slotnum, hash);
}

/*
3.8 ~repartition\_table\_by\_rr~

Repartition the given table by round robin

*/
bool BasicEngine_Control::repartition_table_by_rr(const std::string &table, 
  const std::string &key, const size_t slotnum) {
    
    BOOST_LOG_TRIVIAL(debug) << "Repartiton by rr called on " << table;

    return repartition_table(table, key, slotnum, rr);
}


/*
3.9 ~exportToWorker~

The data of a partitions table were sanded to the worker
and after that imported by the worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::exportToWorker(const string &sourceTable, 
  const string &destinationTable, const bool exportSchema) {


  if(connections.size() != remoteConnectionInfos.size()) {
    BOOST_LOG_TRIVIAL(error) << "No all connections are available";
    return false;
  }

  string remoteCreateName = getCreateTableSQLName(sourceTable);
  string localCreateName = getFilePath() + remoteCreateName;

  for(size_t index = 0; index < connections.size(); index++){
    
    SecondoInterfaceCS* si = connections[index]->getInterface();

    //sending data
    string strindex = to_string(index+1);
    string remoteName = getFilenameForPartition(sourceTable, strindex);
    string localName = getFilePath() + remoteName;

    int sendFileRes = si->sendFile(localName, remoteName, true);

    if (sendFileRes != 0) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't send the data to the worker.";
      return false;
    }

    bool removeFileRes = (remove(localName.c_str()) == 0);

    if (!removeFileRes) {
      BOOST_LOG_TRIVIAL(error) << "Couldn't remove the local file.";
      return false;
    }

    //sending create Table
    if(exportSchema) {
      int sendFileRes = si->sendFile(localCreateName, remoteCreateName, true);

      if (sendFileRes != 0) {
        BOOST_LOG_TRIVIAL(error) 
          << "Couldn't send the structure-file to the worker.";
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
    string strindex = to_string(index + 1);
    string remoteName = getFilenameForPartition(sourceTable, strindex);

    std::future<bool> asyncResult = std::async(
      &BasicEngine_Control::performImport, 
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
3.10 ~partHash~

The data were partitions in the database by an hash value.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partHash(const string &tab,
                    const string &key, size_t slotnum) {

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);

  partTabName = getTableNameForPartitioning(tab, key);
  drop_table(partTabName);

  query_exec = dbms_connection->getPartitionHashSQL(tab, key,
    anzSlots, partTabName);
  
  if (query_exec != "") {
    val = dbms_connection->sendCommand(query_exec);
  } 

  return val;
}

/*
3.11 ~partFun~

The data were partitions in the database by an defined function.
This function have to be defined before using it.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partFun(const string &tab,
    const string &key, const string &fun, size_t slotnum){
                
  bool val = false;
  string query_exec = "";
  string partTabName = getTableNameForPartitioning(tab,key);
  string anzSlots;

  drop_table(partTabName);

  if (boost::iequals(fun, "share")){
    anzSlots = to_string(remoteConnectionInfos.size());
  } else {
    anzSlots = to_string(slotnum);
  }

  query_exec = dbms_connection->getPartitionSQL(tab, key,
      anzSlots, fun, partTabName);

  if (query_exec != "") {
    val = dbms_connection->sendCommand(query_exec);
  }

  return val;
}

/*
3.12 ~exportData~

Exporting the data from the DBMS to a local file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::exportData(const string &tab, 
  const string &key, size_t slotnum){

  bool val = true;
  string path = getFilePath();
  string parttabname = getTableNameForPartitioning(tab, key);
  string strindex;

  // Starting with 1 to <= numberOfWorker
  for(size_t i=1; i<=remoteConnectionInfos.size(); i++) {
    strindex = to_string(i);

    string exportDataSQL = dbms_connection->getExportDataSQL(tab,
          parttabname, key, strindex, path, slotnum);

    BOOST_LOG_TRIVIAL(debug) << "Export table from DB: "
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
bool BasicEngine_Control::importData(const string &tab) {

  bool val = true;
  string full_path;
  string cmd;
  string strindex;

  //create Table
  full_path = getFilePath() + getCreateTableSQLName(tab);

  // Read data into memory
  ifstream inFile;
  stringstream strStream;
  inFile.open(full_path);
  strStream << inFile.rdbuf();
  cmd = strStream.str();

  val = dbms_connection->sendCommand(cmd) && val;
  
  if(!val) {
    return val;
  }

  FileSystem::DeleteFileOrFolder(full_path);

  //import data (local files from worker)
  // Starting with 1 to <= numberOfWorker
  for(size_t i=1;i<=remoteConnectionInfos.size(); i++){
    strindex = to_string(i);
    full_path = getFilePath() + getFilenameForPartition(tab, strindex);
    val = copy(full_path, tab, true) && val;
    FileSystem::DeleteFileOrFolder(full_path);
  }

  return val;
}

/*
3.14 ~partTable~

Partitions the data, export the data and
import them into the worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partTable(const string &tab, const string &key, 
  const string &art, size_t slotnum, const string &geo_col, 
  float x0, float y0, float slotsize) {

  bool val = true;

  if (boost::iequals(art, "RR")) {
    val = partRoundRobin(tab, key, slotnum);
  } else if (boost::iequals(art, "Hash")) {
    val = partHash(tab, key, slotnum);
  } else if (boost::iequals(art, "Grid")) {
    val = partGrid(tab, key, geo_col,
                    slotnum, x0,  y0, slotsize);
  } else {
    val = partFun(tab, key, art, slotnum);
  }

  if(!val) {
    BOOST_LOG_TRIVIAL(error) 
      << "Couldn't partition the table.";
    return val;
  }

  val = exportData(tab, key, remoteConnectionInfos.size());

  if(!val) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't export the data from the table.";
    return val;
  }

  val = getCreateTableSQL(tab);

  if(!val){
    BOOST_LOG_TRIVIAL(error) << "\n Couldn't create the structure-file";
    return val;
  }

  val = exportToWorker(tab, tab, true);

  if(!val) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't transfer the data to the worker.";
  }

  return val;
}

/*
3.15 ~munion~

Exports the data from the worker and sending them to the master.
The master imports them into the local db.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::munion(const string &table) {

  bool val = true;
  int i = 0;
  vector<std::future<bool>> futures;

  //doing the export with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {

    string strindex = to_string(i+1);
    string path = getFilePath() + getFilenameForPartition(table, strindex);

    string tableName = getCreateTableSQLName(table);
    string filename = getFilenameForPartition(table, strindex);

    std::future<bool> asyncResult = std::async(
      &BasicEngine_Control::performExport, 
      this, ci, table, path, strindex, tableName, filename);
        
    futures.push_back(std::move(asyncResult));

    i++;
  }

  //waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    val = future.get() && val;
  }

  //import in local PG-Master
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
bool BasicEngine_Control::mquery(const string &query,
                    const string &table) {
                
  bool val = true;
  vector<std::future<bool>> futures;

  //doing the query with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {
    std::future<bool> asyncResult = std::async(
    &BasicEngine_Control::performBEQuery, 
    this, ci, table, query);
        
    futures.push_back(std::move(asyncResult));
  }

  //waiting for finishing the threads
  for(std::future<bool> &future : futures) {
    val = future.get() && val;
  }

  return val;
}

/*
3.17 ~mcommand~

The multi command sends and execute a query to all worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::mcommand(const string &query) {

  bool val = true;
  vector<std::future<bool>> futures;

  //doing the command with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {
    std::future<bool> asyncResult = std::async(
    &BasicEngine_Control::performBECommand, 
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
bool BasicEngine_Control::msecondocommand(const string &query) {

  bool val = true;
  vector<std::future<bool>> futures;
 
  //doing the command with one thread for each worker
  for(distributed2::ConnectionInfo* ci : connections) {
    std::future<bool> asyncResult = std::async(
    &BasicEngine_Control::performSimpleSecondoCommand, 
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
bool BasicEngine_Control::shutdownWorker() {

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
bool BasicEngine_Control::checkAllConnections() {

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
bool BasicEngine_Control::runsql(const string &filepath) {

  if (access(filepath.c_str(), 0) == 0) {

    // Read file into memory
    ifstream inFile;
    stringstream strStream;

    inFile.open(filepath);
    strStream << inFile.rdbuf();
    string query = strStream.str();

    //execute the sql-Statement
    if (query != "") {
      bool result = dbms_connection->sendCommand(query);
      return result;
    }

  } else {
    BOOST_LOG_TRIVIAL(error) 
      << "Couldn't find the file at path:" + filepath;
    return false;
  }

  return false;
}

/*
3.20 ~partGrid~

The data were partitions in the database by a grid.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partGrid(const std::string &tab, 
  const std::string &key, const std::string &geo_col, 
  size_t slotnum, float x0, float y0, float slotsize) {

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);
  string x_start = to_string(x0);
  string y_start = to_string(y0);
  string sizSlots = to_string(slotsize);

  //Dropping parttable
  partTabName = getTableNameForPartitioning(tab,key);
  drop_table(partTabName);

  //creating Index on table
  query_exec =  dbms_connection->getDropIndexSQL(tab) + " "
            "" + dbms_connection->getCreateGeoIndexSQL(tab, geo_col);
  val = dbms_connection->sendCommand(query_exec);

  //
  query_exec = dbms_connection->getPartitionGridSQL(tab, key, geo_col, 
    anzSlots, x_start, y_start, sizSlots, partTabName);

  if (query_exec != "" && val) {
    val = dbms_connection->sendCommand(query_exec);
  }

  return val;
} 

/*
3.21 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 bool BasicEngine_Control::getTypeFromSQLQuery(const std::string &sqlQuery, 
    ListExpr &resultList) {

   return dbms_connection->getTypeFromSQLQuery(sqlQuery, resultList);
 }


/*
3.22 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 ResultIteratorGeneric* BasicEngine_Control::performSQLSelectQuery(
   const std::string &sqlQuery) {

   return dbms_connection->performSQLSelectQuery(sqlQuery);
 }


/*
3.23 ~exportWorkerRelation~

Export the worker relation into a file.

*/
string BasicEngine_Control::exportWorkerRelation(
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
bool BasicEngine_Control::performImport(
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
3.25 ~performExport~

Perform the data export operation.

*/
bool BasicEngine_Control::performExport(
      distributed2::ConnectionInfo* ci,
      const std::string &table, 
      const std::string &path, 
      const std::string &nr,
      const std::string &remoteCreateName, 
      const std::string &remoteName) {


  std::string from;
  std::string to;
  std::string cmd;
  std::string transfer_path = path.substr(0,path.find(remoteName));
  bool result = true;

  //export the table structure file
  if(nr == "1") {
    //export tab structure
    cmd = "query be_struct('"+ table + "');";
    result = performSimpleSecondoCommand(ci, cmd);

    //move the structure-file into the request-folder
    if(result) {
      from = transfer_path + remoteCreateName;
      to = ci->getRequestPath() + "/" + remoteCreateName;
      cmd ="query moveFile('"+ from + "','" + to +"')";
      result = performSimpleSecondoCommand(ci, cmd);
    }

    //sending file to master
    if(result) {
      result = (ci->requestFile(remoteCreateName,from,true)==0);

      if(! result) {
        BOOST_LOG_TRIVIAL(error) 
          << "Error while requesting struct file"
          << remoteCreateName << " / " << from;
      }
    }
    
    //delete create file on system
    cmd ="query removeFile('"+ to + "')";
    if(result) {
      result = performSimpleSecondoCommand(ci, cmd);
    }
  }

  //export the date to a file
  cmd = "query be_copy('"+ table + "','"+ path+"');";
  if (result) {
    result = performSimpleSecondoCommand(ci, cmd);
  }

  //move the data-file to the request-folder
  to = ci->getRequestPath() + "/" + remoteName;
  cmd = "query moveFile('"+ path + "','" + to +"')";
  if(result) {
    result = performSimpleSecondoCommand(ci, cmd);
  }

  //sendig the File to the master
  if(result) {
    result =(ci->requestFile(remoteName ,
                          transfer_path + remoteName ,true)==0);

    if(! result) {
      BOOST_LOG_TRIVIAL(error) 
          << "Error while requesting export file"
          << remoteName << " / " << transfer_path + remoteName;
    }
  }

  //delete data file on system
  if(result) {
    cmd = "query removeFile('"+ to + "')";
    result = performSimpleSecondoCommand(ci, cmd);
  }

  return result;
}

/*
3.26 ~runBEQuery~

Starting a query at the worker.

*/
bool BasicEngine_Control::performBEQuery(
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
bool BasicEngine_Control::performBECommand(
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
bool BasicEngine_Control::performSimpleSecondoCommand(
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


} /* namespace BasicEngine */
