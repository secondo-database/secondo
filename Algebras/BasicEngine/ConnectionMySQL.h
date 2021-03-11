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

#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Stream.h"
#include "Algebras/Relation-C++/OperatorConsume.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "WinUnix.h"

#include "ConnectionGeneric.h"
#include "ResultIteratorMySQL.h"

#include <string>
#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>

#include <mysql.h>

namespace BasicEngine {

/*
5 Class ~ConnectionPG~

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

  std::string getDbType() {
    return DBTYPE;
  }

  bool sendCommand(const std::string &command, bool print = true);
  
  bool createConnection();

  bool checkConnection();

  std::string getCreateTableSQL(const std::string &table);

  std::string getDropTableSQL(const std::string &table) {
    return "DROP TABLE IF EXISTS " + table + ";";
  }

  std::string getDropIndexSQL(const std::string& index) {
    return "DROP INDEX IF EXISTS " + index + "_idx;";
  }

  std::string getCreateGeoIndexSQL(const std::string &table, 
    const std::string &geo_col) {

    return "CREATE INDEX " + table + "_idx ON"
                " " + table + " USING GIST (" + geo_col + ");";
  }

  std::string getPartitionRoundRobinSQL(const std::string &table, 
    const std::string &key, const size_t anzSlots, 
    const std::string &targetTab);

  std::string getPartitionHashSQL(const std::string &table, 
    const std::string &key, const size_t anzSlots, 
    const std::string &targetTab);

  std::string getPartitionSQL(const std::string &table, 
    const std::string &keyS, const size_t anzSlots,
    const std::string &fun, const std::string &targetTab);

  std::string getPartitionGridSQL(const std::string &table,
    const std::string &key, const std::string &geo_col, 
    const size_t anzSlots, const std::string &x0, 
    const std::string &y0, const std::string &size, 
    const std::string &targetTab);

  std::string getExportDataSQL(const std::string &table, 
    const std::string &join_table, const std::string &key, 
    const std::string &nr, const std::string &path,
    size_t numberOfWorker);

  std::string getCopySQL(const std::string &table, 
    const std::string &full_path, bool direct);

  std::string getImportTableSQL(
        const std::string &table, const std::string &full_path);

  std::string getExportTableSQL(
        const std::string &table, const std::string &full_path);

  std::string getFilenameForPartition(const std::string &table, 
    const std::string &number) {

    return table + "_" + std::to_string(WinUnix::getpid())
      + "_" + number + ".bin";
  }

  std::string getCreateTabSQL(const std::string &table, 
    const std::string &query) {

    return "CREATE TABLE " + table + " AS ("+ query + ")";
  }

  std::string getCopySchemaSQL(const std::string &table) {
    return "SELECT * FROM " + table + " LIMIT 0";
  }

  std::string getRenameTableSQL(const std::string &source, 
        const std::string &destination) {
  
    return "ALTER TABLE " + source + " RENAME TO " + destination + ";";
  }


  bool getTypeFromSQLQuery(const std::string &sqlQuery, ListExpr &resultList);

  ResultIteratorGeneric* performSQLSelectQuery(const std::string &sqlQuery);

  // The DB Type
  inline static const std::string DBTYPE = "mysql";

  private:

  /*
  5.2 Members

  5.2.1 ~conn~

  The connection to PostgreSQL

  */
  MYSQL* conn = nullptr;

  bool getTypeFromQuery(MYSQL_RES* res, ListExpr &resultList);

  MYSQL_RES* sendQuery(const std::string &query);

};

}; /* namespace BasicEngine */
#endif //_ConnectionMySQL_H_
