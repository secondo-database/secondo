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

using namespace std;

namespace BasicEngine {

/*
6 Class ~ConnectionPG~

Implementation.

6.1 ~Constructor~

*/
ConnectionPG::ConnectionPG(const std::string &_dbUser, 
  const std::string &_dbPass, const int _dbPort, 
  const std::string &_dbName) : ConnectionGeneric(
    _dbUser, _dbPass, _dbPort, _dbName) {
}

/*
6.1.1 Destructor

*/
ConnectionPG::~ConnectionPG() {
  if(conn != nullptr) {
    PQfinish(conn);
    conn = nullptr;
  }
}

/*
6.2 ~createConnection~

*/
bool ConnectionPG::createConnection() {

  if(conn != nullptr) {
    return false;
  }

  string keyword = "host=localhost user=" + dbUser  
      + " password=" + dbPass + " port=" + to_string(dbPort)
      + " dbname=" + dbName + " connect_timeout=10";
  
  conn = PQconnectdb(keyword.c_str());

  return true;
}

/*
6.2 ~checkConnection~

Returns TRUE if the connection is ok.

*/
bool ConnectionPG::checkConnection() {

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
bool ConnectionPG::sendCommand(const string &command, bool print) {
  
  const char *query_exec = command.c_str();

  if (! checkConnection()) {
    return false;
  }

  PGresult* res = PQexec(conn, query_exec);

  if (!res) {
    if(print) {
      BOOST_LOG_TRIVIAL(error) 
        << "Error with Command: " << command;
    }

    return false;
  } 
  
  if(PQresultStatus(res) != PGRES_COMMAND_OK) {
    if(print) {
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
PGresult* ConnectionPG::sendQuery(const string &query) {
   
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
string ConnectionPG::getCreateTableSQL(const string &tab){

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
bool ConnectionPG::partitionRoundRobin(const string &tab, 
  const string &key, const size_t anzSlots, const string &targetTab) {

  // Sequence counter
  string createSequenceSQL = "CREATE TEMP SEQUENCE IF NOT EXISTS temp_seq;";

  bool res = sendCommand(createSequenceSQL);

  if(! res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to create sequence";
    return false;
  }

  // Apply sequence counter to the relation
  string selectSQL = "SELECT (nextval('temp_seq') %" 
    + to_string(anzSlots) + ""
    " ) As slot," + key + " FROM " + tab;

  string createTableSQL = getCreateTabSQL(targetTab, selectSQL);

  res = sendCommand(createTableSQL);

  if(! res) {
    BOOST_LOG_TRIVIAL(error) << "Unable to create round robin table";
    return false;
  }

  return true;
}

/*
6.7 ~get\_partHash~

Creates a table in postgreSQL with the partitioned data by hash value.

*/
string ConnectionPG::getPartitionHashSQL(const string &tab, const string &key,
                  const size_t anzSlots, const string &targetTab) {

  string usedKey(key);
  boost::replace_all(usedKey, ",", ",'%_%',");

  string select = "SELECT DISTINCT (get_byte(decode(md5(concat("
        "" + usedKey + ")),'hex'),15) %"
        " " + to_string(anzSlots) + " ) As slot,"
        "" + key + " FROM "+ tab;

  return getCreateTabSQL(targetTab, select);
}

/*
6.9 ~getFieldInfoFunction~

This function collects all information about fields and value Mappings.
This is needed for a special definition of a partitioning function.

*/
void ConnectionPG::getFieldInfoFunction(const string &tab, 
  const string &key, string &fields, string &valueMap, 
  string &select) {

  string query_exec;
  PGresult* res;

  string usedKey(key);
  boost::replace_all(usedKey, ",","','");

  query_exec ="SELECT a.attname as column_name,a.attname || '_orig' as t,"
        "pg_catalog.format_type(a.atttypid, a.atttypmod) as column_type"
        " FROM pg_catalog.pg_attribute a "
        " INNER JOIN (SELECT oid FROM pg_catalog.pg_class WHERE relname ='"
        "" + tab + "' AND pg_catalog.pg_table_is_visible(oid)) b"
        "  ON a.attrelid = b.oid"
        " WHERE a.attnum > 0 "
        " AND NOT a.attisdropped and a.attname in ('"
        "" + usedKey + "');";
  
  res = sendQuery(query_exec);

  for (int i = 0; i<PQntuples(res) ; i++){
    fields.append(",");
    fields.append(PQgetvalue (res,i,1));
    fields.append(" ");
    fields.append(PQgetvalue(res,i,2)) ;

    valueMap.append(PQgetvalue (res,i,1));
    valueMap.append(" := var_r.");
    valueMap.append(PQgetvalue(res,i,0));
    valueMap.append(";");

    select.append(",");
    select.append(PQgetvalue (res,i,1));
    select.append(" AS ");
    select.append(PQgetvalue(res,i,0));
  }
}

/*
6.10 ~createFunctionRandom~

Creates a table in postgreSQL with the partitioned data by random
and uses for that a function in postgres.

*/
bool ConnectionPG::createFunctionRandom(const string &tab, 
  const string &key, const size_t anzSlots, 
  string &select) {

  string query_exec;
  string fields;
  string valueMap;

  query_exec = "DROP FUNCTION fun()";
  sendCommand(query_exec,false);

  select.append("SELECT slot ");

  getFieldInfoFunction(tab,key,fields,valueMap,select);

  select.append(" FROM fun()");

  query_exec = "create or replace function fun() "
      "returns table ("
      " slot integer " + fields + ") "
      "language plpgsql"
      " as $$ "
      " declare "
      "    var_r record;"
      " begin"
      " for var_r in("
      "            select " + key + ""
      "            from " + tab + ")"
      "        loop  slot := floor(random() * " + to_string(anzSlots) +" );"
      "        " + valueMap + ""
      "        return next;"
      " end loop;"
      " end; $$;";

  return sendCommand(query_exec);
}

/*
6.11 ~get\_partFun~

This function is for organizing the special partitioning functions.

*/
string ConnectionPG::getPartitionSQL(const string &tab, const string &key, 
  const size_t anzSlots, const string &fun, const string &targetTab) {

  string select = "";

  if (boost::iequals(fun, "random")) {
    createFunctionRandom(tab, key, anzSlots, select);
  } else {
    BOOST_LOG_TRIVIAL(error)
        << "Function " + fun + " not recognized!";
  }

  if(select == "") {
    return "";
  }

  return getCreateTabSQL(targetTab, select);
}

/*
6.12 ~get\_exportData~

Creating a statement for exporting the data from a portioning table.

*/
string ConnectionPG::getExportDataSQL(const string &tab, const string &join_tab,
                  const string &key, const string &nr, const string &path,
                  size_t numberOfWorker) {

  string filename = getFilenameForPartition(tab, nr);
  
  return "COPY (SELECT a.* FROM "+ tab +" a INNER JOIN " + join_tab  + " b "
            "" + getjoin(key) + " WHERE ((slot % "
            "" + to_string(numberOfWorker) + ") "
            ") =" + nr + ") TO "
            "'" + path + filename + "' BINARY;";
}

/*
6.13 ~getImportTableSQL~

Creating a statement for exporting the data. 

*/
string ConnectionPG::getImportTableSQL(const std::string &table, 
  const std::string &full_path) {

    return "COPY " + table + " FROM '" + full_path + "' BINARY;";
}

/*
6.14 ~getExportTableSQL~

Creating a statement for exporting the data. 

*/
string ConnectionPG::getExportTableSQL(const std::string &table, 
  const std::string &full_path) {

  return "COPY " + table + " TO '" + full_path + "' BINARY;";
}

/*
6.14 ~getjoin~

Returns the join-part of a join-Statement from a given key-list

*/
string ConnectionPG::getjoin(const string &key) {

  string res= "ON ";
  vector<string> result;
  boost::split(result, key, boost::is_any_of(","));

  for (size_t i = 0; i < result.size(); i++) {
    if (i>0) {
      res = res + " AND ";
    }

    string attribute = result[i];
    boost::replace_all(attribute," ","");

    res = res + "a." + attribute + " = b." + attribute;
  }

  return res;
}

/*
6.15 ~get\_partGrid~

Creates a table in postgreSQL with the partitioned data by a grid.
The key specified a column which content is a object like a line or a polygon.

*/
string ConnectionPG::getPartitionGridSQL(const std::string &tab, 
  const std::string &key, const std::string &geo_col, 
  const size_t anzSlots, const std::string &x0,
  const std::string &y0, const std::string &size, 
  const std::string &targetTab) {

  string query_exec;
  string gridTable = "grid_tab";
  string gridCol = "geom";

  //Creating function
  query_exec = "CREATE OR REPLACE FUNCTION ST_CreateGrid("
        "nrow integer, ncol integer, "
        "xsize float8, ysize float8, "
        "x0 float8, y0 float8, "
        "OUT num integer, "
        "OUT geom geometry) "
        "RETURNS SETOF record AS "
        "$$ "
        "SELECT (i * nrow) + j AS num, ST_Translate(cell,"
        " j * $3 + $5, i * $4 + $6) AS geom "
        "FROM generate_series(0, $1 - 1) AS i, "
        "generate_series(0, $2 - 1) AS j, "
        "( "
        "SELECT ('POLYGON((0 0,0 '||$4||','||$3||' '||$4||','||$3||' 0,0 0))')"
        "::geometry AS cell) AS foo; "
        "$$ LANGUAGE sql IMMUTABLE STRICT;";
    
  sendCommand(query_exec,false);

  //Creating the new grid
  query_exec = getDropTableSQL(gridTable);
  sendCommand(query_exec, false);

  //creating the grid table
  query_exec ="CREATE TABLE " + gridTable + " as (SELECT * "
                   "FROM ST_CreateGrid(" +  to_string(anzSlots) + ","
                   "" + to_string(anzSlots) + ","+ size + "," 
                   + size + ","+  x0 + "," + y0 +"));";

  sendCommand(query_exec,false);

  //creating index on grid
  query_exec = getCreateGeoIndexSQL(gridTable, gridCol);
  sendCommand(query_exec, false);

  string usedKey(key);
  boost::replace_all(usedKey, ",", ",r.");

  query_exec = "SELECT r." + usedKey + ", "
                      "g.num as slot "
               "FROM " + gridTable + " g INNER JOIN "+ tab + " r "
                     "ON ST_INTERSECTS(g.geom,r."+ geo_col +")";

  return getCreateTabSQL(targetTab, query_exec);
}

/*
6.16 ~getTypeFromQuery~

Get the SECONDO type of the given PGresult

*/
bool ConnectionPG::getTypeFromQuery(const PGresult* res, ListExpr &resultList) {

  if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to fetch type from non tuple returning result";
    return false;
  }

  int columns = PQnfields(res);
  
  ListExpr attrList = nl->TheEmptyList();
  ListExpr attrListBegin;

  for(int i = 0; i < columns; i++) {

    int columnType = PQftype(res, i);
    string attributeName = string(PQfname(res, i));

    // Ensure secondo is happy with the name
    attributeName[0] = toupper(attributeName[0]);

    ListExpr attribute;
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
6.16 ~getTypeFromSQLQuery~

Get the SECONDO type of the given SQL query

*/
bool ConnectionPG::getTypeFromSQLQuery(const std::string &sqlQuery, 
    ListExpr &resultList) {

  string usedSQLQuery = limitSQLQuery(sqlQuery);

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection is not ready in getTypeFromSQLQuery";
    return false;
  }
  
  PGresult* res = PQexec(conn, usedSQLQuery.c_str());

  bool result = getTypeFromQuery(res, resultList);

  if(! result) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to get type from SQL query" << usedSQLQuery;
  }

  PQclear(res);

  return result;
}

/*
6.16 ~performSQLSelectQuery~

Perform the given query and return a result iterator

*/
ResultIteratorGeneric* ConnectionPG::performSQLSelectQuery(
  const std::string &sqlQuery) {

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection check failed in performSQLSelectQuery()";
    return nullptr;
  }

  ListExpr resultList;
  PGresult* res = PQexec(conn, sqlQuery.c_str());
  bool result = getTypeFromQuery(res, resultList);

  if(!result) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to get tuple type form query: " << sqlQuery;
    return nullptr;
  }

  return new ResultIteratorPostgres(res, resultList);
}


}/* namespace BasicEngine */


