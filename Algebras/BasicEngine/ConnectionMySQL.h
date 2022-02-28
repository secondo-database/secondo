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
#ifndef _ConnectionMYSQL_H_
#define _ConnectionMYSQL_H_

#include "Algebra.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/OperatorConsume.h"
#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Stream.h"
#include "WinUnix.h"

#include "ConnectionGeneric.h"
#include "SQLDialectMysql.h"
#include "ResultIteratorMySQL.h"

#include <bits/stdc++.h>
#include <boost/log/trivial.hpp>
#include <string>

#include <mysql.h>

namespace BasicEngine {

/*
5 Class ~ConnectionPostgres~

This class represents the controling from the system.

*/
class ConnectionMySQL : public ConnectionGeneric {

public:
  /*
  5.1 Public Methods

  */
  ConnectionMySQL(const std::string &_dbUser, const std::string &_dbPass,
                  const int _dbPort, const std::string &_dbName);

  virtual ~ConnectionMySQL();

  std::string getDbType() { return DBTYPE; }

  bool sendCommand(const std::string &command, bool printErrors = true);

  bool createConnection();

  bool checkConnection();

  std::string getCreateTableSQL(const std::string &table);

  bool validateQuery(const std::string &query);

  void partitionRoundRobin(const std::string &table, const size_t anzSlots,
                           const std::string &targetTab);

  void partitionHash(const std::string &table, const std::string &key,
                     const size_t anzSlots, const std::string &targetTab);

  void partitionFunc(const std::string &table, const std::string &keyS,
                     const size_t anzSlots, const std::string &fun,
                     const std::string &targetTab);

  std::string getCopySQL(const std::string &table, const std::string &full_path,
                         bool direct);

  std::string getImportTableSQL(const std::string &table,
                                const std::string &full_path);

  std::string getExportTableSQL(const std::string &table,
                                const std::string &full_path);

  std::vector<std::tuple<std::string, std::string>>
  getTypeFromSQLQuery(const std::string &sqlQuery);

  ResultIteratorGeneric *performSQLSelectQuery(const std::string &sqlQuery);

  bool createGridTable(const std::string &table);

  bool insertRectangle(const std::string &table, double x, double y,
                       double sizeX, double sizeY);

  virtual void addColumnToTable(const std::string &table,
                                const std::string &name, SQLAttribute type);
                                
  // The DB Type
  inline static const std::string DBTYPE = "mysql";

protected: 
  SQLDialect* buildSQLDialect() {
    return new SQLDialectMySQL();
  }

private:
  /*
  5.2 Members

  5.2.1 ~conn~

  The connection to PostgreSQL

  */
  MYSQL *conn = nullptr;

  MYSQL_RES *sendQuery(const std::string &query);

  std::vector<std::tuple<std::string, std::string>>
  getTypeFromQuery(MYSQL_RES *res);

  std::string getFieldNamesForExport(const std::string &table,
                                     const std::string &fieldPrefix = "");
};

};     /* namespace BasicEngine */
#endif //_ConnectionMySQL_H_
