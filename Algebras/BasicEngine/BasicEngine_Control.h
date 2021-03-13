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
#ifndef _BasicEngine_Control_H_
#define _BasicEngine_Control_H_

#include <optional>
#include <memory>
#include <future>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "ConnectionGeneric.h"
#include "ResultIteratorGeneric.h"
#include "StandardTypes.h"

namespace BasicEngine {

/*
2 ENum ~RepartitionMode~

*/
enum RepartitionMode {hash, rr};

/*
2 Struct ~RemoteConnectionInfo~

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
2 Class ~BasicEngine\_Control~

This class represents the controling from the system.

*/
class BasicEngine_Control {

public:

/*
2.1 Public Methods

*/
  BasicEngine_Control(ConnectionGeneric* _dbms_connection, 
    Relation* _workerRelation, std::string _workerRelationName, 
    bool _isMaster);

  virtual ~BasicEngine_Control();

  ConnectionGeneric* getDBMSConnection() {
    return dbms_connection;
  }

  bool checkAllConnections();

  bool repartition_table(const std::string &tab, 
    const std::string &key, const size_t slotnum, 
    const RepartitionMode &repartitionMode);

  bool repartition_table_worker(const std::string &tab, 
    const std::string &key, const size_t slotnum, 
    const RepartitionMode &repartitionMode);

  bool repartition_table_master(const std::string &tab, 
    const std::string &key, const size_t slotnum, 
    const RepartitionMode &repartitionMode);

  bool repartition_table_by_hash(const std::string &tab, 
    const std::string &key, const size_t slotnum);

  bool repartition_table_by_rr(const std::string &tab, 
    const std::string &key, const size_t slotnum);

  bool partTable(const std::string &tab, const std::string &key, 
    const std::string &art, size_t slotnum, const std::string &geo_col = "", 
    float x0 = 0, float y0 = 0, float slotsize = 0);

  bool drop_table(const std::string &tab) {
    std::string sqlQuery = dbms_connection->getDropTableSQL(tab);
    return sendCommand(sqlQuery, false);
  }

  bool getCreateTableSQL(const std::string &tab);

  bool importTable(const std::string &tab, const std::string &full_path) {
    std::string sqlQuery = dbms_connection->getImportTableSQL(tab, full_path);
    return sendCommand(sqlQuery);
  }

  bool exportTable(const std::string &tab, const std::string &full_path) {
    std::string sqlQuery = dbms_connection->getExportTableSQL(tab, full_path);
    return sendCommand(sqlQuery);
  }

  bool createTab(const std::string &tab, const std::string &query) {
    std::string sqlQuery = dbms_connection->getCreateTabSQL(tab, query);
    return sendCommand(sqlQuery);
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

  bool sendCommand(const std::string &query, bool print=true) {
    return dbms_connection->sendCommand(query, print);
  }

  bool getTypeFromSQLQuery(const std::string &sqlQuery, ListExpr &resultList);

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

  bool performImport(distributed2::ConnectionInfo* ci,
        const std::string &table,
        const std::string &remoteCreateName,
        const std::string &remoteName,
        const bool importSchema);

  bool performExport(distributed2::ConnectionInfo* ci,
        const std::string &table, 
        const std::string &path, 
        const std::string &nr,
        const std::string &remoteCreateName, 
        const std::string &remoteName);

  bool performBEQuery(distributed2::ConnectionInfo* ci,
        const std::string &table, 
        const std::string &query);

  bool performBECommand(distributed2::ConnectionInfo* ci,
        const std::string &command);

  bool performSimpleSecondoCommand(distributed2::ConnectionInfo* ci,
        const std::string &command);

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

  bool partRoundRobin(const std::string &tab, const std::string &key, 
    size_t slotnum);

  bool partHash(const std::string &tab, const std::string &key, 
    size_t slotnum);

  bool partFun(const std::string &tab, const std::string &key,
         const std::string &fun, size_t slotnum);

  bool partGrid(const std::string &tab, const std::string &key, 
    const std::string &geo_col, size_t slotnum, 
    float x0, float y0, float slotsize);

  bool exportData(const std::string &tab, const std::string &key,
         size_t slotnum);

  bool importData(const std::string &tab);

  bool exportToWorker(const std::string &sourceTable, 
    const std::string &destinationTable, const bool exportSchema);

  std::string getCreateTableSQLName(const std::string &tab) {
    return "create" + tab + ".sql";
  }

  std::string getFilenameForPartition(const std::string &tab, 
    const std::string &nr) {
  
    return dbms_connection->getFilenameForPartition(tab, nr);
  }

  std::string getFilePath() {
    return std::string("/home/") + getenv("USER") + "/filetransfer/";
  }

  std::string getTableNameForPartitioning(const std::string &tab, 
    const std::string &key);

  std::string getRepartitionTableName(const std::string &table) {
    return table + "_repartition";
  }

  bool executeSecondoCommand(distributed2::ConnectionInfo* ci, 
    const std::string &command, const bool checkResult);

};
};  /* namespace BasicEngine */

#endif //_BasicEngine_Control_H_
