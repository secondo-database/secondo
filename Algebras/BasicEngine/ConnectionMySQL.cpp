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

    if(mysql_real_connect(conn, "127.0.0.1", 
            dbUser.c_str(), dbPass.c_str(),
            dbName.c_str(), dbPort, NULL, 0) == nullptr) {

        BOOST_LOG_TRIVIAL(error) << "Unable to connect to MySQL server: "
            << mysql_error(conn);

        if(conn != nullptr) {
            mysql_close(conn);
            conn = nullptr;
        }

        return false;
    }

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
                << " / " << mysql_error(conn);
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
      << " / " << mysql_error(conn);
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
        BOOST_LOG_TRIVIAL(error) << "MySQL ping failed"
          << " / " << mysql_error(conn);
        return false;
    }

    return true;
}

/*
6.6 ~getCreateTableSQL~

*/
std::string ConnectionMySQL::getCreateTableSQL(const std::string &table) {

    string createTableSQL = "DROP TABLE IF EXISTS " + table +";\n";

    string getTableStructure = "SHOW CREATE TABLE " + table;

    MYSQL_RES* res = sendQuery(getTableStructure);

    if(res != nullptr) {
        MYSQL_ROW row = mysql_fetch_row(res);

        // 0 = Tablename
        // 1 = Create Statement
        char* createTable = row[1];
        createTableSQL = createTableSQL + createTable + ";";
    }

    if(res != nullptr) {
        mysql_free_result(res);
        res = nullptr;
    }

    BOOST_LOG_TRIVIAL(debug) 
      << "Create table statement is: " << createTableSQL;

    return createTableSQL;
}

/*
6.7 ~partitionRoundRobin~

*/
bool ConnectionMySQL::partitionRoundRobin(
    const std::string &table, const std::string &key, 
    const size_t slots, const std::string &targetTab) {
    
    // Apply sequence counter to the relation
    string selectSQL = "SELECT @n := ((@n + 1) % " 
        + to_string(slots) + ") Slot, t.* "
        + "FROM (SELECT @n:=0) AS initvars, " + table + " AS t";
    
    string createTableSQL = getCreateTabSQL(targetTab, selectSQL);

    bool res = sendCommand(createTableSQL);

    if(! res) {
        BOOST_LOG_TRIVIAL(error) << "Unable to create round robin table";
        return false;
    }

    return true;
}

/*
6.8 ~getPartitionHashSQL~

*/
std::string ConnectionMySQL::getPartitionHashSQL(const std::string &table, 
    const std::string &key, const size_t anzSlots, 
    const std::string &targetTab) {

  string usedKey(key);
  boost::replace_all(usedKey, ",", ",'%_%',");

  string selectSQL = "SELECT DISTINCT md5(" + usedKey + ") % " 
        + to_string(anzSlots) + " As slot, " + key
        + " FROM "+ table;

    BOOST_LOG_TRIVIAL(debug) 
      << "Partition hash statement is: " << selectSQL;

  return getCreateTabSQL(targetTab, selectSQL);
}

/*
6.9 ~getPartitionSQL~

*/
std::string ConnectionMySQL::getPartitionSQL(const std::string &table, 
    const std::string &key, const size_t anzSlots,
    const std::string &fun, const std::string &targetTab) {

    string usedKey(key);
    boost::replace_all(usedKey, ",", ",'%_%',");

    string selectSQL = "";
    
    if (boost::iequals(fun, "random")) {
        selectSQL = "SELECT DISTINCT rand() % " 
            + to_string(anzSlots) + " As slot, " + key
            + " FROM "+ table;
    } else {
        BOOST_LOG_TRIVIAL(error) 
                << "Unknown partitioning function: " << fun;
        return "";
    }

    BOOST_LOG_TRIVIAL(debug) 
        << "Partition hash statement is: " << selectSQL;

  return getCreateTabSQL(targetTab, selectSQL);
}

/*
6.10 ~getPartitionGridSQL~

*/
std::string ConnectionMySQL::getPartitionGridSQL(const std::string &table,
    const std::string &key, const std::string &geo_col, 
    const size_t anzSlots, const std::string &x0, 
    const std::string &y0, const std::string &size, 
    const std::string &targetTab) {
 
    
    // Drop old procedure
    string dropProcedure = "DROP PROCEDURE IF EXISTS ST_CREATE_STATIC_GRID;";

    bool res = sendCommand(dropProcedure.c_str());

    if(!res) {
        BOOST_LOG_TRIVIAL(error) 
        << "Unable to perform SQL query " << dropProcedure            
        << mysql_error(conn);
        return "";
    }

    // Create new procedure
    const char* createProcedure = R"ENDSQL(
CREATE PROCEDURE ST_CREATE_STATIC_GRID(IN rows INTEGER, IN columns INTEGER,
    IN xsize FLOAT, IN ysize FLOAT, IN x0 FLOAT, IN y0 FLOAT)

BEGIN

-- Declare variables
DECLARE cur_row INTEGER DEFAULT 0;
DECLARE cur_column INTEGER DEFAULT 0;
DECLARE polygon VARCHAR(255);
DECLARE cell_x0, cell_x1, cell_y0, cell_y1 FLOAT;
DECLARE cell_p0, cell_p1, cell_p2, cell_p3 VARCHAR(32);

DROP TABLE IF EXISTS grid_table;

CREATE TABLE grid_table ( 
    id BIGINT NOT NULL AUTO_INCREMENT, 
    cell POLYGON NOT NULL,
    PRIMARY KEY(id)
);

-- Create cells
START TRANSACTION;

WHILE cur_row < rows DO
   SET cur_column = 0;
   WHILE cur_column < columns DO
      SET cell_x0 = x0 + (cur_column * xsize);
      SET cell_y0 = y0 + (cur_row * ysize);

      SET cell_x1 = cell_x0 + xsize;
      SET cell_y1 = cell_y0 + ysize;

      SET cell_p0 = concat(cell_x0, ' ', cell_y0);
      SET cell_p1 = concat(cell_x1, ' ', cell_y0);
      SET cell_p2 = concat(cell_x1, ' ', cell_y1);
      SET cell_p3 = concat(cell_x0, ' ', cell_y1);
      
      SET polygon = concat('POLYGON((', cell_p0, ',', cell_p1, ',', cell_p2, 
        ',', cell_p3, ',', cell_p0, '))');
      INSERT INTO grid_table(cell) values(ST_PolyFromText(polygon, 4326));

      SET cur_column = cur_column + 1;
   END WHILE;
   SET cur_row = cur_row + 1;
END WHILE;

COMMIT;

END;
)ENDSQL";

    res = sendCommand(createProcedure);

    if(! res) {
        BOOST_LOG_TRIVIAL(error) 
        << "Unable to perform SQL query " << createProcedure;
        return "";
    }

    // Execute procedure to create table
    string createCell = "CALL ST_CREATE_STATIC_GRID(" + to_string(anzSlots) 
        + ", " + to_string(anzSlots) + ", " + size + " , " + size + ", " 
        + x0 + ", " + y0 + ");";

    res = sendCommand(createCell.c_str());

    if(!res) {
        BOOST_LOG_TRIVIAL(error) 
        << "Unable to perform SQL query " << createCell;
        return "";
    }

    string usedKey(key);
    boost::replace_all(usedKey, ",", ",r.");

    string query_exec = "SELECT r." + usedKey + ", "
               "g.id AS slot "
               "FROM grid_table g INNER JOIN "+ table + " r "
               "ON ST_INTERSECTS(g.cell, r."+ geo_col +")";

    return getCreateTabSQL(targetTab, query_exec);
}

/*
6.11 ~getExportDataSQL~

*/
std::string ConnectionMySQL::getExportDataSQL(const std::string &table, 
    const std::string &join_table, const std::string &key, 
    const std::string &nr, const std::string &path,
    size_t numberOfWorker) {
 
    string filename = getFilenameForPartition(table, nr);

    string attributes = getFieldNamesForExport(table, "a.");

    string exportSQL = "SELECT DISTINCT " + attributes + " FROM " + table 
        + " a INNER JOIN " + join_table  + " b " + getjoin(key) 
        + " WHERE (slot % " + to_string(numberOfWorker) + ") = " + nr
        + " INTO OUTFILE '" + path + filename + "' CHARACTER SET utf8;";

    BOOST_LOG_TRIVIAL(debug) 
      << "Export partition statement is: " << exportSQL;

    return exportSQL;
}


/*
6.11 ~getExportColumns~

Regular import and export of MySQL relations with SHAPE datatype fail. 

Cannot get geometry object from data you send to the GEOMETRY field

Therefore, all geometry fields have to be serialized manually to text. 
In addition, the spatial reference identifier (SRID) has to be exported.

This method returns the column description for an export that can be used
in queries like SELECT thisResult FROM table INTO OUTFILE. 

*/

std::string ConnectionMySQL::getFieldNamesForExport(
    const std::string &table, const std::string &fieldPrefix) {

  string sqlQuery = "SELECT * FROM " + table + " LIMIT 1";

  MYSQL_RES *res = sendQuery(sqlQuery.c_str());

  if(res == nullptr) {
    throw SecondoException("Unable to read table structure");
  }

  int columns = mysql_num_fields(res);
  MYSQL_FIELD *fields = mysql_fetch_fields(res);

  std::vector<std::string> columnNames;

  for(int i = 0; i < columns; i++) {       
    enum_field_types columnType = fields[i].type;
    string attributeName = string(fields[i].name);

    string renamedField = fieldPrefix + attributeName;

    if(columnType == MYSQL_TYPE_GEOMETRY) {
        columnNames.push_back("ST_AsWKT(" + renamedField + ")");
        columnNames.push_back("ST_SRID(" + renamedField + ")");
    } else {
        columnNames.push_back(renamedField);
    }
  }

  mysql_free_result(res);
  res = nullptr;

  // Join attribute names as string
  string attributeNames = std::accumulate(
                            std::begin(columnNames), 
                            std::end(columnNames), 
                            string(),
                            [](string &ss, string &s) {
                                    return ss.empty() ? s : ss + "," + s;
                            });

   return attributeNames;
}

/*
6.14 ~getjoin~

Returns the join-part of a join-Statement from a given key-list

*/
string ConnectionMySQL::getjoin(const string &key) {

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
6.12 ~getImportTableSQL~

When the table contains a SHAPE attribute, the geometry was serialized into
two attributes (TEXT, SRID) see ~getFieldNamesForExport~. These attributes
needs to be handled in a special way.

*/
std::string ConnectionMySQL::getImportTableSQL(const std::string &table, 
    const std::string &full_path) {
 
  string sqlQuery = "SELECT * FROM " + table + " LIMIT 1";

  MYSQL_RES *res = sendQuery(sqlQuery.c_str());

  if(res == nullptr) {
    throw SecondoException("Unable to read table structure");
  }

  int columns = mysql_num_fields(res);
  MYSQL_FIELD *fields = mysql_fetch_fields(res);

  int importColumns = 0;

  // Column name, source
  std::string importColumnSQL;

  for(int i = 0; i < columns; i++) {       
    enum_field_types columnType = fields[i].type;
    string attributeName = string(fields[i].name);
    
    if(i == 0) {
        importColumnSQL = "SET `" + attributeName + "` = ";
    } else {
        importColumnSQL.append(", `" + attributeName + "` = ");
    }

    string thisColumn = "@col" + to_string(importColumns);

    if(columnType == MYSQL_TYPE_GEOMETRY) {
        string nextColumn = "@col" + to_string(importColumns + 1);
        importColumnSQL.append("ST_PolygonFromText(" + thisColumn 
            + ", " + nextColumn + ")");
        importColumns += 2;
    } else {
        importColumnSQL.append(thisColumn);
        importColumns++;
    }
  }

  mysql_free_result(res);
  res = nullptr;

  // Generate column string
  string columnSQL = "";
  for(int i = 0; i < importColumns; i++) {
      string thisColumn = "@col" + to_string(i);

      if(i == 0) {
          columnSQL.append("(" + thisColumn);
      } else {
          columnSQL.append(", " + thisColumn);
      }
  }
  columnSQL.append(")");

  // Import example
  // LOAD DATA INFILE "/tmp/water" into table water_import CHARACTER SET 
  //   utf8 (@col0, @col1, @col2, @col3, @col4, @col5, @col6) 
  //   SET `OGR_FID` = @col0, SHAPE = ST_PolygonFromText(@col1, @col2), 
  //   osm_id = @col3, code = @col4, fclass = @col5, name = @col6;

   string loadSQL = "LOAD DATA INFILE '" + full_path + "' INTO TABLE " + table 
        + " CHARACTER SET utf8 " + columnSQL + " " + importColumnSQL + ";";

  //  BOOST_LOG_TRIVIAL(error) << "Import query is: " << loadSQL;

   return loadSQL;
}

/*
6.12 ~getExportTableSQL~

*/
std::string ConnectionMySQL::getExportTableSQL(const std::string &table, 
    const std::string &full_path) {
 
    string attributes = getFieldNamesForExport(table);
 
    return "SELECT " + attributes + " INTO OUTFILE '" + full_path 
        + "' CHARACTER SET utf8 FROM " + table + ";";
}

/*
6.13 ~getTypeFromSQLQuery~

*/
std::vector<std::tuple<string, string>> 
    ConnectionMySQL::getTypeFromSQLQuery(const std::string &sqlQuery) {

  string usedSQLQuery = limitSQLQuery(sqlQuery);
  vector<tuple<string, string>> result;

  if( ! checkConnection()) {
    BOOST_LOG_TRIVIAL(error) 
      << "Connection is not ready in getTypeFromSQLQuery";
    return result;
  }
  
  MYSQL_RES *res = sendQuery(usedSQLQuery.c_str());

  if(res == nullptr) {
    BOOST_LOG_TRIVIAL(error) 
      << "Unable to perform SQL query" << usedSQLQuery;
      return result;
  }

  result = getTypeFromQuery(res);

  if(res != nullptr) {
     mysql_free_result(res);
     res = nullptr;
  }

  return result;
}


/*
6.14 ~getTypeFromQuery~

*/
vector<tuple<string, string>> ConnectionMySQL::getTypeFromQuery(
    MYSQL_RES* res) {

  vector<tuple<string, string>> result;
  int columns = mysql_num_fields(res);
  MYSQL_FIELD *fields = mysql_fetch_fields(res);

  for(int i = 0; i < columns; i++) {       
    enum_field_types columnType = fields[i].type;
        
    string attributeName = string(fields[i].name);

    // Ensure secondo is happy with the name
    attributeName[0] = toupper(attributeName[0]);

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
    auto tuple = std::make_tuple(attributeName, attributeType);
    result.push_back(tuple);
  }

  return result;
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

    vector<std::tuple<std::string, std::string>> types = getTypeFromQuery(res);
    ListExpr resultList = convertTypeVectorIntoSecondoNL(types);

    if(nl->IsEmpty(resultList)) {
        BOOST_LOG_TRIVIAL(error) 
            << "Unable to get tuple type form query: " << sqlQuery;
            return nullptr;
    }

    return new ResultIteratorMySQL(res, resultList);
}

/*
6.16 Validate the given query

*/
bool ConnectionMySQL::validateQuery(const std::string &query) {

    if( ! checkConnection()) {
        BOOST_LOG_TRIVIAL(error) 
             << "Connection check failed in validateQuery()";
        return false;
    }

    string restrictedQuery = limitSQLQuery(query);

    MYSQL_RES *res = sendQuery(restrictedQuery.c_str());

    if(res == nullptr) {
        return false;
    }

    mysql_free_result(res);
    res = nullptr;

    return true;
}

}