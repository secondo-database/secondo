/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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
#include "ConnectionPG.h"
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>

using namespace std;

namespace BasicEngine {

ConnectionPG::ConnectionPG(int nport, string ndbname) {
  string keyword;
  keyword = "host=localhost port=" + to_string(nport)
      + " dbname=" + ndbname + " connect_timeout=10";
  conn = PQconnectdb(keyword.c_str());
  port = nport;
  dbname = ndbname;
}

bool ConnectionPG::checkConn() {
  if (PQstatus(conn) == CONNECTION_BAD) {
    printf("Connection Error:%s\n", PQerrorMessage(conn));
    return false;
  }
  return true;
};

bool ConnectionPG::sendCommand(string command, bool print) {
PGresult *res;
const char *query_exec = command.c_str();
//cout << command << endl;
if (checkConn()) {
  res = PQexec(conn, query_exec);
  if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
    if(print){printf("Error with Command:%s\n", PQresultErrorMessage(res));}
    PQclear(res);
    return false;
  }
  PQclear(res);
}
return true;
}

PGresult* ConnectionPG::sendQuery(string query) {
PGresult *res;
const char *query_exec = query.c_str();

//cout << query<< endl;
if (checkConn()) {
  res = PQexec(conn, query_exec);
  if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
    printf("Error with Query:%s\n", PQresultErrorMessage(res));
  }
}
return res;
}

string ConnectionPG::createTabFile(string tab){
string query_exec;
PGresult* res;
string write;

query_exec = "SELECT a.attname as column_name, "
      "    pg_catalog.format_type(a.atttypid, a.atttypmod) as column_type "
      "FROM pg_catalog.pg_attribute a "
      "INNER JOIN (SELECT oid FROM pg_catalog.pg_class "
      "WHERE relname ='" +  tab +
      "' AND pg_catalog.pg_table_is_visible(oid)) b "
        "ON a.attrelid = b.oid "
      "WHERE a.attnum > 0 "
      "    AND NOT a.attisdropped "
      "ORDER BY a.attnum ";
res = sendQuery(query_exec);


write = "DROP TABLE IF EXISTS public." + tab +";\n"
      "CREATE TABLE public." + tab +" (\n";
for (int i = 0; i<PQntuples(res) ; i++){
  if (i>0) write.append(",");
  write.append(PQgetvalue (res,i,0));
  write.append(" ");
  write.append(PQgetvalue(res,i,1)) ;
  write.append("\n");
}
write.append(");");

return write;
}

string ConnectionPG::get_partRoundRobin(string tab, string key
                    , string anzWorker, string targetTab){
string select = "SELECT (nextval('temp_seq') %" +anzWorker+ ""
    " ) + 1 As worker_number," + key +" FROM "+ tab;
return "CREATE TEMP SEQUENCE IF NOT EXISTS temp_seq;"
    + get_createTab(targetTab,select);
}

string ConnectionPG::get_partHash(string tab, string key
                  , string anzWorker, string targetTab){
string select = "SELECT (get_byte(decode(md5(concat(" + key + ")),'hex'),15) %"
    " " + anzWorker + " ) + 1 As worker_number," + key +" FROM "+ tab;
return get_createTab(targetTab,select);
}

string replaceStringAll(string str, const string& replace
            , const string& with) {
    if(!replace.empty()) {
        size_t pos = 0;
        while ((pos = str.find(replace, pos)) != string::npos) {
            str.replace(pos, replace.length(), with);
            pos += with.length();
        }
    }
    return str;
}

void ConnectionPG::getFieldInfoFunction(string* tab, string* key
            , string *fields, string *valueMap, string* select){

string query_exec;
PGresult* res;

  query_exec ="SELECT a.attname as column_name,a.attname || '_orig' as t,"
        "pg_catalog.format_type(a.atttypid, a.atttypmod) as column_type"
        " FROM pg_catalog.pg_attribute a "
        " INNER JOIN (SELECT oid FROM pg_catalog.pg_class WHERE relname ='"
        "" + *tab + "' AND pg_catalog.pg_table_is_visible(oid)) b"
        "  ON a.attrelid = b.oid"
        " WHERE a.attnum > 0 "
        " AND NOT a.attisdropped and a.attname in ('"
        "" + replaceStringAll(*key,",","','") + "');";
  res = sendQuery(query_exec);

  for (int i = 0; i<PQntuples(res) ; i++){
    fields->append(",");
    fields->append(PQgetvalue (res,i,1));
    fields->append(" ");
    fields->append(PQgetvalue(res,i,2)) ;

    valueMap->append(PQgetvalue (res,i,1));
    valueMap->append(" := var_r.");
    valueMap->append(PQgetvalue(res,i,0));
    valueMap->append(";");

    select->append(",");
    select->append(PQgetvalue (res,i,1));
    select->append(" AS ");
    select->append(PQgetvalue(res,i,0));
  }
}


bool ConnectionPG::createFunctionRandom(string* tab, string* key
            , string* anzWorker, string* select){
string query_exec;
string fields;
string valueMap;

sendCommand("DROP FUNCTION fun()",false);

select->append("SELECT worker_number ");

getFieldInfoFunction(tab,key,&fields,&valueMap,select);


select->append(" FROM fun()");

query_exec = "create or replace function fun() "
      "returns table ("
      " worker_number integer " + fields + ") "
      "language plpgsql"
      " as $$ "
      " declare "
      "    var_r record;"
      " begin"
      " for var_r in("
      "            select " + *key + ""
      "            from " + *tab + ")"
      "        loop  worker_number := ceil(random() * " + *anzWorker +" );"
      "        " + valueMap + ""
      "        return next;"
      " end loop;"
      " end; $$;";

//create function
return sendCommand(query_exec);
}

bool ConnectionPG::createFunctionDDRandom(string* tab, string* key
            , string* anzWorker, string* select){
string query_exec;
string fields;
string valueMap;

sendCommand("DROP FUNCTION fun()",false);

select->append("SELECT worker_number ");

getFieldInfoFunction(tab,key,&fields,&valueMap,select);


select->append(" FROM fun()");

query_exec = "create or replace function fun() "
      "returns table ("
      " worker_number integer " + fields + ") "
      "language plpgsql"
      " as $$ "
      " declare "
      "    var_r record;"
      " begin"
      " for var_r in("
      "            select " + *key + ""
      "            from " + *tab + ""
      "        FULL JOIN (SELECT 1 as c"
            "                UNION "
      "               SELECT 2 as c) a on 1=1)"
      "        loop  worker_number := ceil(random() * " + *anzWorker +" );"
      "        " + valueMap + ""
      "        return next;"
      " end loop;"
      " end; $$;";

//create function
cout << query_exec << endl;
return sendCommand(query_exec);
}

string ConnectionPG::get_partFun(string tab, string key, string anzWorker,
                  string fun, string targetTab){
string select = "";
string query = "";

  if (fun == "random"){
    createFunctionRandom(&tab,&key,&anzWorker,&select);
  }else if (fun == "DDrandom"){
    createFunctionDDRandom(&tab,&key,&anzWorker,&select);
  }
  else{
    cout<< "Function " + fun + " not recognized! "
        "Available functions are: RR, Hash, DDrandom and random." << endl;
  }

  if(select != "") query = get_createTab(targetTab,select);

return query;
}

string ConnectionPG::get_exportData(string tab, string join_tab
                  , string join_stat, string nr, string path){
  return "COPY (SELECT a.* FROM "+ tab +" a INNER JOIN " + join_tab  + " b "
            "" + join_stat + " WHERE worker_number =" + nr+ ") TO "
            "'" + path + tab + "_" + nr +".bin' BINARY;";
}

string ConnectionPG::get_copy(string tab, string full_path, bool direct ){
string res;

  if (direct)  return "COPY "+  full_path + " FROM '" + tab + "' BINARY;";
  else return "COPY "+  tab + " TO '" + full_path + "' BINARY;";
}
}/* namespace BasicEngine */
