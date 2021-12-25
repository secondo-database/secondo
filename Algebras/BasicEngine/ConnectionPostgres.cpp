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
#include "ConnectionPostgres.h"
#include "BasicEngineControl.h"

using namespace std;

namespace BasicEngine {

/*
6 Class ~ConnectionPostgres~

Implementation.

6.1 ~Constructor~

*/
ConnectionPostgres::ConnectionPostgres(const std::string &_dbUser, 
  const std::string &_dbPass, const int _dbPort, 
  const std::string &_dbName) : ConnectionGeneric(
    _dbUser, _dbPass, _dbPort, _dbName) {

    sqlDialect = buildSQLDialect();
}

/*
6.1.1 Destructor

*/
ConnectionPostgres::~ConnectionPostgres() {
  if(conn != nullptr) {
    PQfinish(conn);
    conn = nullptr;
  }
}

/*
6.2 ~createConnection~

*/
bool ConnectionPostgres::createConnection() {

  if(conn != nullptr) {
    return false;
  }

  string keyword = "host=localhost user=" + dbUser  
      + " password=" + dbPass + " port=" + to_string(dbPort)
      + " dbname=" + dbName + " connect_timeout=10";
  
  conn = PQconnectdb(keyword.c_str());
  
  if (PQstatus(conn) == CONNECTION_BAD) {
    BOOST_LOG_TRIVIAL(error) << "Unable to connect to PostgresSQL: "
      << PQerrorMessage(conn);
    
    if(conn != nullptr) {
      PQfinish(conn);
      conn = nullptr;
    }

    return false;
  }

  return true;
}

/*
6.2 ~checkConnection~

Returns TRUE if the connection is ok.

*/
bool ConnectionPostgres::checkConnection() {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  if(conn == nullptr) {
    return false;
  }

  if (PQstatus(conn) == CONNECTION_BAD) {
    printf("Connection Error:%s\n", PQerrorMessage(conn));
    return false;
  }

  return true;
};

/*
6.3 ~sendCommand~

Sending a command to postgres.

Returns TRUE if the execution was ok.

*/
bool ConnectionPostgres::sendCommand(const string &command, bool printErrors) {
  
  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  const char *query_exec = command.c_str();

  if (! checkConnection()) {
    return false;
  }

  PGresult* res = PQexec(conn, query_exec);

  if (!res) {
    if(printErrors) {
      BOOST_LOG_TRIVIAL(error) 
        << "Error with Command: " << command;
    }

    return false;
  } 
  
  if(PQresultStatus(res) != PGRES_COMMAND_OK) {
    if(printErrors) {
      BOOST_LOG_TRIVIAL(error) 
        << "Error with Command: " << command
        << " error is: " <<  PQresultErrorMessage(res);
    }

    PQclear(res);
    res = nullptr;
  
    return false;
  }

  PQclear(res);
  res = nullptr;

  return true;
}

/*
6.4 ~sendQuery~

Sending a query to postgres.

Returns the Result of the query.

*/
PGresult* ConnectionPostgres::sendQuery(const string &query) {
   
  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  const char *query_exec = query.c_str();

  if (! checkConnection()) {
    return nullptr;
  }

  PGresult *res = PQexec(conn, query_exec);
  
  if (!res) {
      BOOST_LOG_TRIVIAL(error)
        << "Error with Query: " <<  PQresultErrorMessage(res);
      return nullptr;
  } 
  
  if(PQresultStatus(res) != PGRES_TUPLES_OK) {
      BOOST_LOG_TRIVIAL(error)
        << "Error with Query: " <<  PQresultErrorMessage(res);

    PQclear(res);
    res = nullptr;
    return nullptr;
  }

  return res;
}

/*
6.5 ~getCreateTableSQL~

Creates a Create-Statement of a given table and
return this string.

*/
string ConnectionPostgres::getCreateTableSQL(const string &tab){

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);
  
  string query_exec;
  PGresult* res;
  string write="";

  query_exec = "SELECT a.attname as column_name, "
      "    pg_catalog.format_type(a.atttypid, a.atttypmod) as column_type "
      "FROM pg_catalog.pg_attribute a "
      "INNER JOIN (SELECT oid FROM pg_catalog.pg_class "
      "WHERE relname ='" + tab +
      "' AND pg_catalog.pg_table_is_visible(oid)) b "
        "ON a.attrelid = b.oid "
      "WHERE a.attnum > 0 "
      "    AND NOT a.attisdropped "
      "ORDER BY a.attnum ";

  res = sendQuery(query_exec);

  if (PQntuples(res) > 0){
    
    write = "DROP TABLE IF EXISTS public." + tab +";\n"
      "CREATE TABLE public." + tab +" (\n";
    
    for (int i = 0; i<PQntuples(res); i++) {
      if (i>0) write.append(",");
      write.append(PQgetvalue (res,i,0));
      write.append(" ");
      write.append(PQgetvalue(res,i,1)) ;
      write.append("\n");
    }

    write.append(");");
  }

  PQclear(res);

  return write;
}

/*
6.6 ~get\_partRoundRobin~

Creates a table in postgreSQL with the partitioned data by round robin.

*/
void ConnectionPostgres::partitionRoundRobin(const string &tab, 
  const size_t anzSlots, const string &targetTab) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  // Sequence counter
  string createSequenceSQL = "CREATE TEMP SEQUENCE IF NOT EXISTS temp_seq;";

  bool res = sendCommand(createSequenceSQL);

  if(! res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to create sequence";
    throw SecondoException(
      "Error in rr paritioning: Unable to create sequence");
  }

  // Apply sequence counter to the relation
  string selectSQL = "SELECT (nextval('temp_seq') %" 
    + to_string(anzSlots) + ""
    " ) As " + be_partition_slot + ",t.* FROM " + tab + " AS t";

  string createTableSQL = sqlDialect 
    -> getCreateTableFromPredicateSQL(targetTab, selectSQL);

  res = sendCommand(createTableSQL);

  if(! res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to create round robin table";
    throw SecondoException(
      "Error in rr paritioning: Unable to create round robin table");
  }
}

/*
6.7 ~partitionHash~

Creates a table in postgreSQL with the partitioned data by hash value.

*/
void ConnectionPostgres::partitionHash(const string &tab, const string &key,
                                 const size_t anzSlots,
                                 const string &targetTab) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  string usedKey(key);
  boost::replace_all(usedKey, ",", ",'%_%',");

  string select = "SELECT (get_byte(decode(md5(concat("
        "" + usedKey + ")),'hex'),15) %"
        " " + to_string(anzSlots) + " ) As " + be_partition_slot + ","
        "t.* FROM "+ tab + " AS t";

  string createTableSQL = sqlDialect 
    -> getCreateTableFromPredicateSQL(targetTab, select);

  bool res = sendCommand(createTableSQL);

  if(! res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to execute hash partitioning";
    throw SecondoException("Error in hash paritioning");
  }
}

/*
6.11 ~partitionFunc~

This function is for organizing the special partitioning functions.

*/
void ConnectionPostgres::partitionFunc(const string &table, const string &key,
                                 const size_t anzSlots, const string &fun,
                                 const string &targetTab) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  string selectSQL = "";

  if (boost::iequals(fun, "random")) {
    selectSQL = "SELECT (random() * 10000)::int % " + to_string(anzSlots) +
                " As " + be_partition_slot + ", t.* FROM " + table + " AS t";
  } else {
    BOOST_LOG_TRIVIAL(error) << "Function " << fun << " not recognized!";
    throw SecondoException("Function " + fun + " not recognized!");
  }

  BOOST_LOG_TRIVIAL(debug) << "Partition SQL statement is: " << selectSQL;

  string createTableSQL = sqlDialect 
    -> getCreateTableFromPredicateSQL(targetTab, selectSQL);

  bool res = sendCommand(createTableSQL);

  if (!res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to execute func partitioning";
    throw SecondoException("Error in func paritioning");
  }
}

/*
6.13 ~getImportTableSQL~

Creating a statement for exporting the data. 

*/
string ConnectionPostgres::getImportTableSQL(const std::string &table, 
  const std::string &full_path) {

    return "COPY " + table + " FROM '" + full_path + "' BINARY;";
}

/*
6.14 ~getExportTableSQL~

Creating a statement for exporting the data. 

*/
string ConnectionPostgres::getExportTableSQL(const std::string &table, 
  const std::string &full_path) {

  return "COPY " + table + " TO '" + full_path + "' BINARY;";
}

/*
6.16 ~getTypeFromSQLQuery~

Get the SECONDO type of the given SQL query

*/
std::vector<std::tuple<string, string>> 
    ConnectionPostgres::getTypeFromSQLQuery(const std::string &sqlQuery) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  string usedSQLQuery = limitSQLQuery(sqlQuery);
  vector<tuple<string, string>> result;

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection is not ready in getTypeFromSQLQuery";
    return result;
  }
  
  PGresult* res = PQexec(conn, usedSQLQuery.c_str());

  if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to fetch type from non tuple returning result";
    return result;
  }

  result = getTypeFromQuery(res);

  PQclear(res);

  return result;
}

/*
6.16 ~getTypeFromQuery~

*/
vector<std::tuple<std::string, std::string>> 
  ConnectionPostgres::getTypeFromQuery(PGresult* res) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  vector<tuple<string, string>> result;
  int columns = PQnfields(res);
  
  for(int i = 0; i < columns; i++) {

    int columnType = PQftype(res, i);
    string attributeName = string(PQfname(res, i));

    // Ensure secondo is happy with the name
    attributeName[0] = toupper(attributeName[0]);

    string attributeType;

    // Convert to SECONDO attribute type
    switch(columnType) {
      case BOOLOID:
        attributeType = CcBool::BasicType();
        break;

      case CHAROID:
      case TEXTOID:
      case VARCHAROID:
        attributeType = FText::BasicType();
        break;

      case INT2OID:
      case INT4OID:
      case INT8OID:
        attributeType = CcInt::BasicType();
        break;

      case FLOAT4OID:
      case FLOAT8OID:
          attributeType = CcReal::BasicType();
        break;

      default:
          BOOST_LOG_TRIVIAL(warning) 
              << "Unknown column type: " << attributeName << " / " 
              << columnType << " will be mapped to text";
          attributeType = FText::BasicType();
    }

    // Attribute name and type
    auto tuple = std::make_tuple(attributeName, attributeType);
    result.push_back(tuple);
  }

  return result;
}

/*
6.16 ~performSQLSelectQuery~

Perform the given query and return a result iterator

*/
ResultIteratorGeneric* ConnectionPostgres::performSQLSelectQuery(
  const std::string &sqlQuery) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection check failed in performSQLSelectQuery()";
    return nullptr;
  }

  PGresult* res = sendQuery(sqlQuery.c_str());

  if(res == nullptr) {
    return nullptr;
  }    

  vector<std::tuple<std::string, std::string>> types = getTypeFromQuery(res);
  ListExpr resultList = convertTypeVectorIntoSecondoNL(types);

  if(nl->IsEmpty(resultList)) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to get tuple type form query: " << sqlQuery;
    return nullptr;
  }

  return new ResultIteratorPostgres(res, resultList);
}


/*
6.16 Validate the given query

*/
bool ConnectionPostgres::validateQuery(const std::string &query) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

    if( ! checkConnection()) {
        BOOST_LOG_TRIVIAL(error) 
             << "Connection check failed in validateQuery()";
        return false;
    }

    string restrictedQuery = limitSQLQuery(query);

    PGresult* res = sendQuery(restrictedQuery.c_str());

    if(res == nullptr) {
      return false;
    }

    PQclear(res);
    res = nullptr;

    return true;
}

/*
6.17 Create the table for the grid

*/
bool ConnectionPostgres::createGridTable(const std::string &table) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

    string createTable = "CREATE TABLE " + table + " (id " 
        + " SERIAL PRIMARY KEY, "
        + " cell geometry NOT NULL);";

    PGresult *res = sendQuery(createTable.c_str());

    if(res == nullptr) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to execute: " + createTable;
        return false;
    }

    // Create index on grid
    string createIndex = "CREATE INDEX " + table + "_idx ON"
                " " + table + " USING GIST (cell);";

    res = sendQuery(createIndex.c_str());

    if(res == nullptr) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to execute: " + createIndex;
        return false;
    }

    PQclear(res);
    res = nullptr;

    return true;
}

/*
6.17 Insert a rectangle into the grid table

*/
bool ConnectionPostgres::insertRectangle(const std::string &table, 
        double x, double y, double sizeX, double sizeY) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

    string polygon = "POLYGON((" + to_string(x) + " " + to_string(y) + 
        "," + to_string(x+sizeX) + " " + to_string(y) + 
        "," + to_string(x+sizeX) + " " + to_string(y+sizeY) + 
        "," + to_string(x) + " " + to_string(y+sizeY) + 
        "," + to_string(x) + " " + to_string(y) + "))";


    string insertSQL = "INSERT INTO " + table 
        + "(cell) values(ST_Polygon('" + polygon + "'::geometry, 4326))";

    PGresult *res = sendQuery(insertSQL.c_str());

    if(res == nullptr) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to execute: " + insertSQL;
        return false;
    }

    PQclear(res);
    res = nullptr;

    return true;
}

/*
6.18 Add a new column to the table

*/
void ConnectionPostgres::addColumnToTable(const std::string &table, 
    const std::string &name, SQLAttribute type) {

  const std::lock_guard<std::recursive_mutex> lock(connection_mutex);

    string sql = "ALTER TABLE " + table + " ADD COLUMN " + name;

    switch(type) {
        case SQLAttribute::sqlinteger:
            sql.append(" INTEGER");
        break;

        default:
            throw SecondoException("Unsupported datatyepe: " + type);
    }


   bool res = sendCommand(sql.c_str());

   if(res == false) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to execute: " << sql;
        throw SecondoException("Unable to add column to table");
   }
}

}/* namespace BasicEngine */
