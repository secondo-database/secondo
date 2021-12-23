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
#ifndef _BasicEngineControl_H_
#define _BasicEngineControl_H_

#include <optional>
#include <memory>
#include <future>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Distributed2/DArray.h"
#include "ConnectionGeneric.h"
#include "ResultIteratorGeneric.h"
#include "StandardTypes.h"

namespace BasicEngine {

/*
1.0 ENum ~PartitionMode~

*/
enum PartitionMode {hash, rr, random, grid, fun};

/*
1.1 Struct ~PartitonData~

*/
typedef struct {
  std::string table;
  std::string key;
  size_t slotnum;

  // For fuction based partitioning
  std::string partitionfun;

  // For grid based partitioning
  std::string attribute;
  std::string gridname;
} PartitionData;

/*
1.2 Struct ~RemoteConnectionInfo~

*/
struct RemoteConnectionInfo {
  std::string host;
  std::string port;
  std::string config;
  std::string dbUser;
  std::string dbPass;
  std::string dbPort;
  std::string dbName;
};

/*
1.2 Struct ~ExportedSlotData~

*/
typedef struct {
  size_t slot;
  bool partitionedTable;
  std::string destinationTable;
  std::string filename;
  distributed2::ConnectionInfo* workerConnection;
} ExportedSlotData;

/*
1.3 identifyer for distributed tables

*/
#define DISTRIBUTED_TABLE_MARKER "_D"

/*
2 Class ~BasicEngine\_Control~

This class represents the controling from the system.

*/
class BasicEngineControl {

public:

/*
2.1 Public Methods

*/
  BasicEngineControl(ConnectionGeneric* _dbms_connection, 
    Relation* _workerRelation, std::string _workerRelationName, 
    bool _isMaster);

  virtual ~BasicEngineControl();

  ConnectionGeneric* getDBMSConnection() {
    return dbms_connection;
  }

  bool checkAllConnections();

  distributed2::DArray repartitionTable(PartitionData &partitionData, 
    const PartitionMode &repartitionMode);

  distributed2::DArray partitionTable(PartitionData &partitionData, 
    const PartitionMode &repartitionMode, const bool repartition);

  distributed2::DArray
  repartitionTableMaster(const PartitionData &partitionData,
                         const PartitionMode &repartitionMode);

  distributed2::DArray partitionTableByHash(const std::string &table, 
    const std::string &key, const size_t slotnum,
    const bool repartition);

  distributed2::DArray partitionTableByRR(const std::string &table, 
    const size_t slotnum, const bool repartition);

  distributed2::DArray partitionTableByFun(const std::string &table, 
    const std::string &key, const std::string &fun, 
    const size_t slotnum, const bool repartition);

  distributed2::DArray partitionTableByGrid(const std::string &table, 
    const std::string &key, const size_t slotnum, 
    const std::string &attribute, const std::string &gridname, 
    const bool repartition);

  distributed2::DArray partitionTableByRandom(const std::string &table, 
    const size_t slotnum, const bool repartition);

  void exportTableCreateStatementSQL(const std::string &table, 
    const std::string &outputFile,
    const std::string &renameExportTable = "");

  std::string requestRemoteTableSchema(
    const std::string &table, distributed2::ConnectionInfo *ci);

  bool importTable(const std::string &table, const std::string &full_path) {
    std::string sqlQuery = dbms_connection->getImportTableSQL(table, full_path);
    return dbms_connection->sendCommand(sqlQuery);
  }

  bool exportTable(const std::string &table, const std::string &full_path) {
    std::string sqlQuery = dbms_connection->getExportTableSQL(table, full_path);
    return dbms_connection->sendCommand(sqlQuery);
  }

  bool createTable(const std::string &table, const std::string &query) {
    std::string sqlQuery =
        dbms_connection->getSQLDialect()
        ->getCreateTableFromPredicateSQL(table, query);
    return dbms_connection->sendCommand(sqlQuery);
  }

  std::string getTablenameForPartition(const std::string &table, size_t slot) {
    return table + "_" + std::to_string(slot);
  }

  bool shareTable(const std::string &table);

  bool munion(const std::string &table);

  bool mquery(const std::string &query, const std::string &table);

  bool mcommand(const std::string &query);

  bool msecondocommand(const std::string &query);

  bool runsql(const std::string &filepath);

  bool initBasicEngineOnWorker(distributed2::ConnectionInfo* ci, 
    const RemoteConnectionInfo* remoteConnectionInfo);

  bool shutdownWorker();

  ListExpr getTypeFromSQLQuery(const std::string &sqlQuery);

  ResultIteratorGeneric* performSQLSelectQuery(const std::string &sqlQuery);

  std::string exportWorkerRelation(const std::string &relationName);

  bool exportWorkerRelationToWorker(distributed2::ConnectionInfo* ci, 
    const std::optional<std::string> &workerRelationFileName);

  bool isMaster() {
    return master;
  }

  std::string getWorkerRelationName() {
    return workerRelationName;
  }

  bool createAllConnections();

  void shutdownAllConnections();

  bool validateQuery(const std::string &query);

  bool performPartitionImport(distributed2::ConnectionInfo* ci,
        const std::string &table,
        const std::string &remoteFileName);

  bool performSchemaImport(distributed2::ConnectionInfo* ci,
        const std::string &remoteSchemaFileName);

  bool performPartitionExport(distributed2::ConnectionInfo* ci,
        size_t workerId,
        const std::string &table, 
        const std::string &path, 
        const std::string &partitionFile);

  bool performBEQuery(distributed2::ConnectionInfo* ci,
        const std::string &table, 
        const std::string &query);

  bool performBECommand(distributed2::ConnectionInfo* ci,
        const std::string &command);

  bool performSimpleSecondoCommand(distributed2::ConnectionInfo* ci,
        const std::string &command);

  std::string getSchemaFile(const std::string &table) {
    return "schema_" + table + ".sql";
  }
  std::string getSchemaFile(const std::string &table, size_t partition) {
    return "schema_" + table + "_" + std::to_string(partition) + ".sql";
  }

  std::string getBasePath() {
    return std::string("/home/") + getenv("USER") + "/filetransfer";
  }

  static const size_t defaultTimeout = 0;

  static const int defaultHeartbeat = 0;
  
private:

/*
2.2 Members

2.2.1 ~dbs\_conn~

In this template variable were stores the connection,
to a secondary dbms (for example postgresql)

*/
ConnectionGeneric* dbms_connection;

/*
2.2.2 workerRelationName is the name of the used worker relation

*/
std::string workerRelationName = "";

/*
2.2.3 ~remoteConnectionInfos~

The remoteConnectionInfos is a vector with all informations about the
worker connection like port, connection-file, ip

*/
std::vector<RemoteConnectionInfo*> remoteConnectionInfos;

/*
2.2.4 ~connections~

In this vector all connection to the worker are stored.

*/
std::vector<distributed2::ConnectionInfo*> connections;

/*
2.2.5 master is a variable which shows, if this system is a master (true)
or a worker(false).

*/
bool master = false;


/*
2.3 Private Methods

*/
distributed2::ConnectionInfo* createAndInitConnection(
  const RemoteConnectionInfo* remoteConnectionInfo,
  const std::optional<std::string> &workerRelationFileName);

distributed2::ConnectionInfo* createConnection(
  const RemoteConnectionInfo* remoteConnection);

  std::string partRoundRobin(const std::string &tab, size_t slotnum);

  std::string partHash(const std::string &tab, const std::string &key, 
    size_t slotnum);

  std::string partFun(const std::string &tab, const std::string &key,
         const std::string &fun, size_t slotnum);

  std::string partGrid(const std::string &tab, const std::string &key,
                       const std::string &geo_col, const std::string &gridname,
                       size_t slotsize);

  std::list<ExportedSlotData>
  exportAllPartitions(const std::string &sourceTable,
                      const std::string &destinationTable,
                      size_t noOfPartitions, size_t noOfWorker);

  bool importData(const std::string &table);

  bool exportPartitionsToWorker(std::list<ExportedSlotData>);

  void exportSchemaToWorker(const std::string &table, 
    std::list<ExportedSlotData>);

  std::string getTableNameForPartitioning(const std::string &tab, 
    const std::string &key);

  std::string getRepartitionTableName(const std::string &table) {
    return table + "_repartition";
  }

  bool executeSecondoCommand(distributed2::ConnectionInfo* ci, 
    const std::string &command, const bool checkResult);

  std::string getFirstAttributeNameFromTable(const std::string &table);

  void dropAttributeIfExists(const std::string &table, 
    const std::string &attribute);

  ConnectionGeneric* getDbmsConnection() {
    return dbms_connection;
  }

};
};  /* namespace BasicEngine */

#endif //_BasicEngineControl_H_
