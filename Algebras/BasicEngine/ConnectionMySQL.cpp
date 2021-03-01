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
#include "ConnectionMySQL.h"

using namespace std;

namespace BasicEngine {

/*
6 Class ~ConnectionPG~

Implementation.

6.1 ~Constructor~

*/
ConnectionMySQL::ConnectionMySQL(const std::string &_dbUser, 
  const std::string &_dbPass, const int _dbPort, 
  const std::string &_dbName) : ConnectionGeneric(
    _dbUser, _dbPass, _dbPort, _dbName) {
}

/*
6.1.1 Destructor

*/
ConnectionMySQL::~ConnectionMySQL() {
  if(conn != nullptr) {
    mysql_close(conn);
    conn = nullptr;
  }
}

/*
6.2 ~createConnection~

*/
bool ConnectionMySQL::createConnection() {

    if(conn != nullptr) {
        return false;
    }

    mysql_real_connect(conn, "localhost", dbUser.c_str(), dbPass.c_str(),
            dbName.c_str(), dbPort, NULL, 0);


    return true;
}

/*
6.3 ~sendCommand~

*/
bool ConnectionMySQL::sendCommand(const std::string &command, 
    bool print) {

    // TODO
    return false;
}

/*
6.4 ~checkConnection~

*/
bool ConnectionMySQL::checkConnection() {
    // TODO
    return false;
}

/*
6.5 ~getCreateTableSQL~

*/
std::string ConnectionMySQL::getCreateTableSQL(const std::string &table) {
    // TODO
    return string("");
}

/*
6.6 ~getPartitionRoundRobinSQL~

*/
std::string ConnectionMySQL::getPartitionRoundRobinSQL(
    const std::string &table, 
    const std::string &key, const std::string &anzSlots, 
    const std::string &targetTab) {
    
    // TODO
    return string("");
}

/*
6.7 ~getPartitionHashSQL~

*/
std::string ConnectionMySQL::getPartitionHashSQL(const std::string &table, 
    const std::string &key, const std::string &anzSlots, 
    const std::string &targetTab) {

    // TODO
    return string("");
}

/*
6.8 ~getPartitionSQL~

*/
std::string ConnectionMySQL::getPartitionSQL(const std::string &table, 
    const std::string &keyS, const std::string &anzSlots,
    const std::string &fun, const std::string &targetTab) {

    // TODO
    return string("");
}

/*
6.9 ~getPartitionGridSQL~

*/
std::string ConnectionMySQL::getPartitionGridSQL(const std::string &table,
    const std::string &key, const std::string &geo_col, 
    const std::string &anzSlots, const std::string &x0, 
    const std::string &y0, const std::string &size, 
    const std::string &targetTab) {
 
    // TODO
    return string("");
}

/*
6.10 ~getExportDataSQL~

*/
std::string ConnectionMySQL::getExportDataSQL(const std::string &table, 
    const std::string &join_table, const std::string &key, 
    const std::string &nr, const std::string &path,
    size_t numberOfWorker) {
 
    // TODO
    return string("");
}

/*
6.11 ~getCopySQL~

*/
std::string ConnectionMySQL::getCopySQL(const std::string &table, 
    const std::string &full_path, bool direct) {
 
    // TODO
    return string("");
}

/*
6.12 ~getTypeFromSQLQuery~

*/
bool ConnectionMySQL::getTypeFromSQLQuery(const std::string &sqlQuery, 
    ListExpr &resultList) {

    // TODO
    return false;
}

/*
6.13 ~getTypeFromQuery~

*/
bool ConnectionMySQL::getTypeFromQuery(const PGresult* res, 
    ListExpr &resultList) {

    // TODO
    return false;
}

/*
6.14 ~performSQLSelectQuery~

*/
ResultIteratorGeneric* ConnectionMySQL::performSQLSelectQuery(
    const std::string &sqlQuery) {

    // TODO
    return nullptr;
}
}