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
    mysql_library_end();
  }
}

/*
6.2 ~createConnection~

*/
bool ConnectionMySQL::createConnection() {

    if(conn != nullptr) {
        return false;
    }

    int mysqlInitRes = mysql_library_init(0, NULL, NULL);

    if(mysqlInitRes != 0) {
        BOOST_LOG_TRIVIAL(error) << "Unable to init MySQL client library";
        return false;
    }

    conn = mysql_init(conn);

    if(conn == nullptr) {
        BOOST_LOG_TRIVIAL(error) << "Unable to create MySQL connection object";
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

    int mysqlExecRes = mysql_query(conn, command.c_str());

    if(mysqlExecRes != 0) {
        if(print) {
            BOOST_LOG_TRIVIAL(error) 
                << "Unable to perform command: " << command
                << mysql_error(conn);
        }

        return false;
    }

    return true;
}

/*
6.4 ~sendQuery~

*/
MYSQL_RES* ConnectionMySQL::sendQuery(const std::string &query) {

 int mysqlExecRes = mysql_query(conn, query.c_str());

  if(mysqlExecRes != 0) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to perform query" << query
      << mysql_error(conn);
    return nullptr;
  }

  MYSQL_RES *res = mysql_store_result(conn);

  return res;
}


/*
6.5 ~checkConnection~

*/
bool ConnectionMySQL::checkConnection() {

    if(conn == nullptr) {
        return false;
    }

    int pingResult = mysql_ping(conn);

    if(pingResult != 0) {
        BOOST_LOG_TRIVIAL(error) << "MySQL ping failed";
        return false;
    }

    return true;
}

/*
6.6 ~getCreateTableSQL~

*/
std::string ConnectionMySQL::getCreateTableSQL(const std::string &table) {
    // TODO
    return string("");
}

/*
6.7 ~getPartitionRoundRobinSQL~

*/
std::string ConnectionMySQL::getPartitionRoundRobinSQL(
    const std::string &table, 
    const std::string &key, const size_t anzSlots, 
    const std::string &targetTab) {
    
    // TODO
    return string("");
}

/*
6.8 ~getPartitionHashSQL~

*/
std::string ConnectionMySQL::getPartitionHashSQL(const std::string &table, 
    const std::string &key, const size_t anzSlots, 
    const std::string &targetTab) {

    // TODO
    return string("");
}

/*
6.9 ~getPartitionSQL~

*/
std::string ConnectionMySQL::getPartitionSQL(const std::string &table, 
    const std::string &keyS, const size_t anzSlots,
    const std::string &fun, const std::string &targetTab) {

    // TODO
    return string("");
}

/*
6.10 ~getPartitionGridSQL~

*/
std::string ConnectionMySQL::getPartitionGridSQL(const std::string &table,
    const std::string &key, const std::string &geo_col, 
    const size_t anzSlots, const std::string &x0, 
    const std::string &y0, const std::string &size, 
    const std::string &targetTab) {
 
    // TODO
    return string("");
}

/*
6.11 ~getExportDataSQL~

*/
std::string ConnectionMySQL::getExportDataSQL(const std::string &table, 
    const std::string &join_table, const std::string &key, 
    const std::string &nr, const std::string &path,
    size_t numberOfWorker) {
 
    // TODO
    return string("");
}

/*
6.12 ~getCopySQL~

*/
std::string ConnectionMySQL::getCopySQL(const std::string &table, 
    const std::string &full_path, bool direct) {
 
    // TODO
    return string("");
}

/*
6.13 ~getTypeFromSQLQuery~

*/
bool ConnectionMySQL::getTypeFromSQLQuery(const std::string &sqlQuery, 
    ListExpr &resultList) {

  string usedSQLQuery = sqlQuery;

  string sqlQueryUpper = boost::to_upper_copy<std::string>(sqlQuery);

  if(sqlQueryUpper.find("LIMIT") == std::string::npos) {
    // Remove existing ";" at the query end, if exists
    if(boost::algorithm::ends_with(usedSQLQuery, ";")) {
      usedSQLQuery.pop_back();
    }

    // Limit query to 1 result tuple
    usedSQLQuery = usedSQLQuery + " LIMIT 1;";
  }

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection is not ready in getTypeFromSQLQuery";
    return false;
  }
  
  int mysqlExecRes = mysql_query(conn, usedSQLQuery.c_str());

  if(mysqlExecRes != 0) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to perform query" << usedSQLQuery
      << mysql_error(conn);
    return false;
  }

  MYSQL_RES *res = mysql_store_result(conn);

  bool result = getTypeFromQuery(res, resultList);

  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to get type from SQL query" << usedSQLQuery;
  }

  mysql_free_result(res);

  return result;
}

/*
6.14 ~getTypeFromQuery~

*/
bool ConnectionMySQL::getTypeFromQuery(MYSQL_RES* res, 
    ListExpr &resultList) {

    ListExpr attrList = nl->TheEmptyList();
    ListExpr attrListBegin;

    int columns = mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    for(int i = 0; i < columns; i++) {       
        enum_field_types columnType = fields[i].type;
        
        string attributeName = string(fields[i].name);

        // Ensure secondo is happy with the name
        attributeName[0] = toupper(attributeName[0]);

        ListExpr attribute;
        string attributeType;

        // Convert to SECONDO attribute type
        switch(columnType) {
#if LIBMYSQL_VERSION_ID >= 80000
        case MYSQL_TYPE_BOOL:
            attributeType = CcBool::BasicType();
            break;
#endif
        
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
            attributeType = FText::BasicType();
            break;

        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_LONGLONG:
            attributeType = CcInt::BasicType();
            break;

        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
            attributeType = CcReal::BasicType();
            break;

        default:
            BOOST_LOG_TRIVIAL(warning) 
                << "Unknown column type: " << attributeName << " / " 
                << columnType << " will be mapped to text";
            attributeType = FText::BasicType();
        }

        // Attribute name and type
        attribute = nl->TwoElemList(
            nl->SymbolAtom(attributeName), 
            nl->SymbolAtom(attributeType)
        );
        
        if(nl->IsEmpty(attrList)) {
            attrList = nl -> OneElemList(attribute);
            attrListBegin = attrList;
        } else {
            attrList = nl -> Append(attrList, attribute);
        }
    }

    resultList = nl->TwoElemList(
        listutils::basicSymbol<Stream<Tuple> >(),
        nl->TwoElemList(
            listutils::basicSymbol<Tuple>(),
            attrListBegin));

    return true;
}

/*
6.15 ~performSQLSelectQuery~

*/
ResultIteratorGeneric* ConnectionMySQL::performSQLSelectQuery(
    const std::string &sqlQuery) {

    if( ! checkConnection()) {
        BOOST_LOG_TRIVIAL(error) 
             << "Connection check failed in performSQLSelectQuery()";
        return nullptr;
    }

    MYSQL_RES *res = sendQuery(sqlQuery.c_str());

    if(res == nullptr) {
        return nullptr;
    }    

    ListExpr resultList;
    bool result = getTypeFromQuery(res, resultList);

    if(!result) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to get tuple type form query: " << sqlQuery;
            return nullptr;
    }

    return new ResultIteratorMySQL(res, resultList);
}
}