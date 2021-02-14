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
#ifndef _ConnectionPG_H_
#define _ConnectionPG_H_

#include "Attribute.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "Algebra.h"
#include "Stream.h"
#include "Algebras/Relation-C++/OperatorConsume.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "ConnectionGeneric.h"
#include "ResultIteratorPostgres.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>
#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>

namespace BasicEngine {

/*
5 Class ~ConnectionPG~

This class represents the controling from the system.

*/
class ConnectionPG : public ConnectionGeneric {

  public:

  /*
  5.1 Public Methods

  */
  ConnectionPG(int port, std::string dbname);

  virtual ~ConnectionPG();

  bool sendCommand(const std::string &command, bool print = true);

  bool checkConnection();

  std::string createTabFile(const std::string &table);

  std::string getDatabaseName() {
    return dbname;
  }

  int getDatabasePort() {
    return port;
  }

  std::string getInitSecondoCMD(const std::string &dbname, 
    const std::string &port, const std::string &workerRelation) {

    return "query be_init('pgsql'," + port + ",'"
      + dbname + "'," + workerRelation + ");";
  }

  std::string get_drop_table(const std::string &table) {
    return "DROP TABLE IF EXISTS " + table + ";";
  }

  std::string get_drop_index(const std::string& index) {
    return "DROP INDEX IF EXISTS " + index + "_idx;";
  }

  std::string create_geo_index(const std::string &table, 
    const std::string &geo_col) {

    return "CREATE INDEX " + table + "_idx ON"
                " " + table + " USING GIST (" + geo_col + ");";
  }

  std::string get_partRoundRobin(const std::string &table, 
    const std::string &key, const std::string &anzSlots, 
    const std::string &targetTab);

  std::string get_partHash(const std::string &table, 
    const std::string &key, const std::string &anzSlots, 
    const std::string &targetTab);

  std::string get_partFun(const std::string &table, 
    const std::string &keyS, const std::string &anzSlots,
    const std::string &fun, const std::string &targetTab);

  std::string get_partGrid(const std::string &table,
    const std::string &key, const std::string &geo_col, 
    const std::string &anzSlots, const std::string &x0, 
    const std::string &y0, const std::string &size, 
    const std::string &targetTab);

  std::string get_exportData(const std::string &table, 
    const std::string &join_table, const std::string &key, 
    const std::string &nr, const std::string &path,
    size_t numberOfWorker);

  std::string get_copy(const std::string &table, 
    const std::string &full_path, bool direct);

  std::string get_partFileName(const std::string &table, 
    const std::string &number) {

    return table + "_" + number + ".bin";
  }

  std::string getCreateTabSQL(const std::string &table, 
    const std::string &query) {

    return "CREATE TABLE " + table + " AS ("+ query + ")";
  }

  bool getTypeFromSQLQuery(const std::string &sqlQuery, ListExpr &resultList);

  bool getTypeFromQuery(const PGresult* res, ListExpr &resultList);

  ResultIteratorGeneric* performSQLQuery(const std::string &sqlQuery);

  private:

  /*
  5.2 Members

  5.2.1 ~conn~

  The connection to PostgreSQL

  */
  PGconn* conn = NULL;

  /*
  5.2.2 ~port~

  The port from the PostgreSQL DB.

  */
  int port;

  /*
  5.2.3 ~dbname~

  The Name of the Database.

  */
  std::string dbname;

  /*
  5.3 Private Methods

  */
  int get_port() {
    return port;
  }

  std::string get_dbname() {
    return dbname;
  }

  PGresult* sendQuery(const std::string &query);

  bool createFunctionRandom(const std::string &table, 
    const std::string &key, const std::string &numberOfWorker, 
    std::string &select);

  bool createFunctionDDRandom(const std::string &table, 
    const std::string &key, const std::string &numberOfWorker, 
    const std::string &select);

  void getFieldInfoFunction(const std::string &table, 
    const std::string &key, std::string &fields, 
    std::string &valueMap, std::string &select);

  std::string get_partShare(const std::string &table, const std::string &key,
    const std::string &numberOfWorker);

  std::string getjoin(const std::string &key);
};

}; /* namespace BasicEngine */
#endif //_ConnectionPG_H_
