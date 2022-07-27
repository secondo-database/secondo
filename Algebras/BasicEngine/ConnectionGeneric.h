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

*/
#ifndef _CONNECTION_GENERIC_H_
#define _CONNECTION_GENERIC_H_

#include <string>
#include <vector>
#include <mutex>

#include "Algebra.h"
#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Stream.h"

#include "SQLDialect.h"
#include "ResultIteratorGeneric.h"

namespace BasicEngine {

enum SQLAttribute { sqlinteger };

class ConnectionGeneric {

public:
  ConnectionGeneric(const std::string &_dbUser, const std::string &_dbPass,
                    const int _dbPort, const std::string &_dbName)
      : dbUser(_dbUser), dbPass(_dbPass), dbPort(_dbPort), dbName(_dbName) {}

  virtual ~ConnectionGeneric() {
    if (sqlDialect != nullptr) {
      delete sqlDialect;
      sqlDialect = nullptr;
    }
  }

  virtual std::string getDbType() = 0;

  virtual bool createConnection() = 0;

  virtual bool checkConnection() = 0;

  virtual bool validateQuery(const std::string &query) = 0;

  virtual bool sendCommand(const std::string &command,
                           bool printErrors = true) = 0;

  virtual std::string getCreateTableSQL(const std::string &table) = 0;

  virtual void dropTable(const std::string &table);

  virtual void dropIndex(const std::string &table, const std::string &column);

  virtual void partitionRoundRobin(const std::string &table,
                                   const size_t anzSlots,
                                   const std::string &targetTab) = 0;

  virtual void partitionHash(const std::string &table, const std::string &key,
                             const size_t anzSlots,
                             const std::string &targetTab) = 0;

  virtual void partitionFunc(const std::string &table, const std::string &key,
                             const size_t anzSlots, const std::string &fun,
                             const std::string &targetTab) = 0;

  virtual void partitionGrid(const std::string &table, const std::string &key,
                             const std::string &geo_col, const size_t noOfSlots,
                             const std::string &gridname,
                             const std::string &targetTab);

  virtual void exportDataForPartition(const std::string &table,
                                      const std::string &exportFile,
                                      size_t partition);

  virtual std::string getImportTableSQL(const std::string &table,
                                        const std::string &full_path) = 0;

  virtual std::string getExportTableSQL(const std::string &table,
                                        const std::string &full_path) = 0;

  virtual std::vector<std::tuple<std::string, std::string>>
  getTypeFromSQLQuery(const std::string &sqlQuery) = 0;

  virtual ListExpr convertTypeVectorIntoSecondoNL(
      const std::vector<std::tuple<std::string, std::string>> &types);

  virtual ResultIteratorGeneric *
  performSQLSelectQuery(const std::string &sqlQuery) = 0;

  virtual std::string
  getAttributeProjectionSQLForTable(const std::string &table,
                                    const std::string &prefix = "");

  virtual bool createGridTable(const std::string &table) = 0;

  virtual bool insertRectangle(const std::string &table, double x, double y,
                               double sizeX, double sizeY) = 0;

  virtual bool beginTransaction();

  virtual bool abortTransaction();

  virtual bool commitTransaction();

  virtual void addColumnToTable(const std::string &table,
                                const std::string &name, SQLAttribute type) = 0;

  virtual void removeColumnFromTable(const std::string &table,
                                     const std::string &name);

  std::string getFilenameForPartition(const std::string &table,
                                      size_t &partitionNumber) {

    return table + "_" + std::to_string(WinUnix::getpid()) + "_" +
           std::to_string(partitionNumber) + ".bin";
  }

  /*
  5.3

  5.3.1 ~getDbPort~ Return the Port of the database

  */
  int getDbPort() { return dbPort; }

  /*
  5.3.2 ~getDbName~

  Return the name of the database.

  */
  std::string getDbName() { return dbName; }

  /*
  5.3.3 ~getDbUser~

  Return the user of the database.

  */
  std::string getDbUser() { return dbUser; }

  /*
  5.3.4 ~getDbPass~

  Return the password of the database.

  */
  std::string getDbPass() { return dbPass; }

  /*
  5.3.4 ~getSQLDialect~

  Get the used SQL dialect

  */
  SQLDialect *getSQLDialect() { return sqlDialect; }

protected:
  virtual SQLDialect *buildSQLDialect() = 0;

  /*
  5.2.0 Try to limit the given SQL query to 1 result.

  Needed to determine the type of the query result.
  */
  std::string limitSQLQuery(const std::string &query);

  /*
  5.2.1 ~dbUser~

  The username for the database connection

  */
  std::string dbUser;

  /*
  5.2.2 ~dbPass~

  The password of the postgress connection

  */
  std::string dbPass;

  /*
  5.2.3 ~port~

  The port of the database

  */
  int dbPort;

  /*
  5.2.4 ~dbname~

  The Name of the Database.

  */
  std::string dbName;

  /*
  5.2.5 The referenece to the SQL dialect

  */
  SQLDialect *sqlDialect = nullptr;

  /*
  5.2.6 The connection mutex

  */
  std::recursive_mutex connection_mutex;
};
} // namespace BasicEngine

#endif