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
#include "BasicEngine_Control.h"
#include "FileSystem.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
3 Class ~BasicEngine\_Control~

Implementation.

3.1 ~createConnection~

Creating a specified and saves it in the connections vector. 
Additionally add an entry to the importer vector.

*/
bool BasicEngine_Control::createConnection(string host, string port, 
  string config, string dbPort, string dbName) {

  bool val = false;

  ConnectionInfo* ci = ConnectionInfo::createConnection(
      host, stoi(port), config, 
      BasicEngine_Control::defaultTimeout, 
      BasicEngine_Control::defaultHeartbeat);

  if (ci == nullptr) {  
    cout << "Couldn't connect to secondo-Worker on host "
         << host << " with port " << port << "!" 
         << endl << endl;
  } else {
    connections.push_back(ci);
    BasicEngine_Thread* basicEngineThread = new BasicEngine_Thread(ci);
    importer.push_back(basicEngineThread);

    val = ci->switchDatabase(dbName, true, false, true, 
      BasicEngine_Control::defaultTimeout);

    if(! val) { 
      cerr << "Unable to switch to database " << dbName 
           << " on host " << host << " with port " << port << "!" 
           << endl << endl;
    } else {
      string errMsg;
      int err = 0;
      double rt;
      string res;
      CommandLog commandLog;

      string initCommand = dbms_connection->getInitSecondoCMD(&dbName, &dbPort);

      ci->simpleCommand(initCommand,err,res,false,
          rt,false,commandLog,true, BasicEngine_Control::defaultTimeout);

      if(err != 0) {
        cout << std::string("ErrCode:" + err) << endl;
      } else {
        val = (res == "(bool TRUE)") && val;
      }
    }
  }

  return val;
}

/*
3.2 Destructor

*/
BasicEngine_Control::~BasicEngine_Control() {
    if(dbms_connection != NULL) {
      delete dbms_connection;
      dbms_connection = NULL;
    }

    // Delete importer
    for(const BasicEngine_Thread* basic_engine_thread: importer) {
      delete basic_engine_thread;
    }
    importer.clear();

    // Delete connections
    for(distributed2::ConnectionInfo* ci: connections) {
      ci->deleteIfAllowed();
    }
    connections.clear();

    // Delete cloned worker relation
    if(worker != NULL) {
      worker -> Delete();
      worker = NULL;
    }
  }

/*
3.2 ~createAllConnections~

Creating all connection from the worker relation.

*/
bool BasicEngine_Control::createAllConnections(){

  GenericRelationIterator* it = worker->MakeScan();
  Tuple* tuple = nullptr;
  
  while ((tuple = it->GetNextTuple()) != 0) {
    string host = tuple->GetAttribute(0)->toText();
    string port = tuple->GetAttribute(1)->toText();
    string config = tuple->GetAttribute(2)->toText();
    string dbPort = tuple->GetAttribute(3)->toText();
    string dbName = tuple->GetAttribute(4)->toText();

    if(tuple != NULL) {
      tuple->DeleteIfAllowed();
    }

    bool connectionResult = createConnection(
      host, port, config, dbPort, dbName);
    
    if(! connectionResult) {
      cout << endl 
           << "Error: Unable to establish connection to worker: "
           << host << " / " << port << endl << endl;

      break;
    }
  }

  if(it != NULL) {
    delete it;
    it = NULL;
  }

  if(numberOfWorker != connections.size()) {
    cerr << endl 
         << "Error: Number of worker connections does not match relation size"
         << endl << endl;
    return false;
  }

  return true;
}

/*
3.5 ~getparttabname~

Returns a name of a table with the keys included.

*/
string BasicEngine_Control::getparttabname(string* tab, string* key){

  string usedKey(*key);

  boost::replace_all(usedKey, ",", "_");
  string res = *tab + "_" + usedKey;
  boost::replace_all(res, " ", "");

  return res;
}

/*
3.7 ~createTabFile~

Creates a table create statement from the input tab
and store the statement in a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::createTabFile(string tab) {

  ofstream write;
  string statement;
  bool val = false;

  statement = dbms_connection->createTabFile(&tab);

  if (statement.length() > 0){
    write.open(getFilePath() + createTabFileName(&tab));
    if (write.is_open()){
      write << statement;
      write.close();
      val = write.good();
    }else{ cout << "Couldn't write file into " + getFilePath() + ""
      ". Please check the folder and permissions." << endl;}
  } else { 
     cout << "Table " + tab + " not found." << endl;
  }

   return val;
}

/*
3.8 ~partRoundRobin~

The data were partitions in the database by round robin.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partRoundRobin(string* tab,
                    string* key, size_t slotnum) {

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);

  partTabName = getparttabname(tab,key);
  drop_table(partTabName);

  query_exec = dbms_connection->get_partRoundRobin(tab, key,
      &anzSlots, &partTabName);
  
  if (query_exec != "") {
    val = dbms_connection->sendCommand(&query_exec);
  }

  return val;
}

/*
3.9 ~exportToWorker~

The data of a partitions table were sanded to the worker
and after that imported by the worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::exportToWorker(string *tab){

  bool val = true;
  string query_exec;
  string importPath;
  SecondoInterfaceCS* si ;

  string remoteCreateName = createTabFileName(tab);
  string localCreateName = getFilePath() + remoteCreateName;

  for(size_t index; index < numberOfWorker; index++){

    if (! connections[index]) {
      cout << endl << "Error: connection " << index 
        << " does not exists " << endl << endl;
      return false;
    } else {
      si = connections[index]->getInterface();

      //sending data
      string strindex = to_string(index+1);
      string remoteName = get_partFileName(tab,&strindex);
      string localName = getFilePath() + remoteName;

      val = (si->sendFile(localName, remoteName, true) == 0);
      val = (remove(localName.c_str()) == 0) && val;

      if (!val) {
        cout << "\n Couldn't send the data to the worker." << endl;
        return val;
      }

      //sending create Table
      val = (si->sendFile(localCreateName, remoteCreateName, true) == 0);

      if (!val) {
        cout << "\n Couldn't send the structure-file "
            "to the worker." << endl;
        return val;
      }
    } 
  }

  if(val){
    val = (remove(localCreateName.c_str()) == 0);
    //doing the import with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      string strindex = to_string(i+1);
      string remoteName =get_partFileName(tab,&strindex);
      importer[i]->startImport(*tab,remoteCreateName,remoteName);
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = importer[i]->getResult() && val;
    }
  } else {
    cout << endl << "Something goes wrong with the"
          " export or the transfer." << endl;
  }

  if(!val) {
    cout << endl << "Something goes wrong with the"
            " import at the worker." << endl;
  }

  return val;
}

/*
3.10 ~partHash~

The data were partitions in the database by an hash value.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partHash(string* tab,
                    string* key, size_t slotnum) {

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);

  partTabName = getparttabname(tab,key);
  drop_table(partTabName);

  query_exec = dbms_connection->get_partHash(tab,key
    ,&anzSlots,&partTabName);
  
  if (query_exec != "") {
    val = dbms_connection->sendCommand(&query_exec);
  } 

  return val;
}

/*
3.11 ~partFun~

The data were partitions in the database by an defined function.
This function have to be defined before using it.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partFun(string* tab,
                    string* key,string* fun, size_t slotnum){
                
  bool val = false;
  string query_exec = "";
  string partTabName = getparttabname(tab,key);
  string anzSlots;

  drop_table(partTabName);

  if (boost::iequals(*fun, "share")){
    anzSlots = to_string(numberOfWorker);
  } else {
    anzSlots = to_string(slotnum);
  }

  query_exec = dbms_connection->get_partFun(tab,key,
      &anzSlots,fun,&partTabName);

  if (query_exec != "") {
    val = dbms_connection->sendCommand(&query_exec);
  }

  return val;
}

/*
3.12 ~exportData~

Exporting the data from the DBMS to a local file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::exportData(string* tab, string* key,
   size_t slotnum){

  bool val = true;
  string path = getFilePath();
  string parttabname = getparttabname(tab,key);
  string strindex;
  long unsigned int i;

  // TODO: Check if 1 is the correct start index here (JNI)
  for(i=1;i<=numberOfWorker;i++) {
    strindex = to_string(i);

    string exportDataSQL = dbms_connection->get_exportData(tab,
          &parttabname, key, &strindex, &path, slotnum);
        
    val = sendCommand(exportDataSQL) && val;
  }

  return val;
}

/*
3.13 ~importData~

Importing data from a local file into the dbms. At first the
table will be created and after that starts the import from a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::importData(string *tab) {

  bool val = true;
  string full_path;
  string cmd;
  string strindex;
  long unsigned int i;

  //create Table
  full_path = getFilePath() + createTabFileName(tab);

  // Read data into memory
  ifstream inFile;
  stringstream strStream;
  inFile.open(full_path);
  strStream << inFile.rdbuf();
  cmd = strStream.str();

  val = dbms_connection->sendCommand(&cmd) && val;
  
  if(!val) {
    return val;
  }

  FileSystem::DeleteFileOrFolder(full_path);

  //import data (local files from worker)
  // TODO: Check if 1 is the correct start index here (JNI)
  for(i=1;i<=numberOfWorker;i++){
    strindex = to_string(i);
    full_path = getFilePath() + get_partFileName(tab, &strindex);
    val = copy(full_path,*tab,true) && val;
    FileSystem::DeleteFileOrFolder(full_path);
  }

  return val;
}

/*
3.14 ~partTable~

Partitions the data, export the data and
import them into the worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partTable(string tab, string key, string art,
      size_t slotnum, string geo_col, float x0, float y0, float slotsize){

  bool val = true;

  if (boost::iequals(art, "RR")) {
    val = partRoundRobin(&tab, &key, slotnum);
  } else if (boost::iequals(art, "Hash")) {
    val = partHash(&tab, &key, slotnum);
  } else if (boost::iequals(art, "Grid")) {
    val = partGrid(&tab, &key, &geo_col
                    ,slotnum,&x0, &y0, &slotsize);
  } else {
    val = partFun(&tab, &key, &art, slotnum);
  }

  if(!val) {
    cout << "\n Couldn't partition the table." << endl;
    return val;
  }

  val = exportData(&tab, &key, numberOfWorker);

  if(!val) {
    cout << "\n Couldn't export the data from the table." << endl;
    return val;
  }

  val = createTabFile(tab);

  if(!val){
    cout << "\n Couldn't create the structure-file" << endl;
    return val;
  }

  val = exportToWorker(&tab);

  if(!val) {
    cout << "\n Couldn't transfer the data to the worker." << endl;
  }

  return val;
}

/*
3.15 ~munion~

Exports the data from the worker and sending them to the master.
The master imports them into the local db.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::munion(string tab) {

  bool val = true;
  string path;
  string strindex;

  //doing the export with one thread for each worker
  for(size_t i=0;i<importer.size();i++){
    strindex = to_string(i+1);
    path = getFilePath() + get_partFileName(&tab,&strindex);
    importer[i]->startExport(tab,path,strindex
             ,createTabFileName(&tab),get_partFileName(&tab,&strindex));
  }

  //waiting for finishing the threads
  for(size_t i=0; i<importer.size(); i++){
    val = importer[i]->getResult() && val;
  }

  //import in local PG-Master
  if(val) {
    val = importData(&tab);
  }

  return val;
}

/*
3.16 ~mquery~

The multi query sends and execute a query to all worker
and stores the result in a table.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::mquery(string query,
                    string tab) {
                
  bool val = true;

  //doing the query with one thread for each worker
  for(size_t i=0;i<importer.size();i++){
    importer[i]->startQuery(tab,query);
  }

  //waiting for finishing the threads
  for(size_t i=0;i<importer.size();i++){
    val = importer[i]->getResult() && val;
  }

  return val;
}

/*
3.17 ~mcommand~

The multi command sends and execute a query to all worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::mcommand(string query) {

 bool val = true;

 //doing the command with one thread for each worker
 for(size_t i=0;i<importer.size();i++){
   importer[i]->startCommand(query);
 }

 //waiting for finishing the threads
 for(size_t i=0;i<importer.size();i++){
   val = importer[i]->getResult() && val;
 }

 return val;
}

/*
3.17 ~shutdownWorker~

Shutdown the remote worker

*/
bool BasicEngine_Control::shutdownWorker() {

   bool result = true;
   string shutdownCommand("query be_shutdown()");

   for(BasicEngine_Thread* remote : importer) {
     result = result && remote->simpleCommand(&shutdownCommand);
   }

   return result;
}


/*
3.18 ~checkAllConnections~

Checking the Connection to the secondary Master System and to the Worker.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::checkAllConnections() {

  const int defaultTimeout = 0;

  //checking connection to the worker
  if (connections.size() != numberOfWorker) {
    return false;
  }

  for(size_t i = 0; i < numberOfWorker; i++) {
    CommandLog CommandLog;
    
    bool connectionState = connections[i]->check(
      false, CommandLog, defaultTimeout);
    
    if(!connectionState) {
      return false;
    }
  }

  //checking the connection to the secondary dbms system
  bool localConnectionState = dbms_connection->checkAllConnections();
  return localConnectionState;
}

/*
3.19 ~runsql~

Runs a query from a file.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::runsql(string filepath) {

  if (access(filepath.c_str(), 0) == 0) {

    // Read file into memory
    ifstream inFile;
    stringstream strStream;

    inFile.open(filepath);
    strStream << inFile.rdbuf();
    string query = strStream.str();

    //execute the sql-Statement
    if (query != "") {
      bool result = dbms_connection->sendCommand(&query);
      return result;
    }

  } else {
    cout << "Couldn't find the file at path:" + filepath << endl;
    return false;
  }

  return false;
}

/*
3.20 ~partGrid~

The data were partitions in the database by a grid.
Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Control::partGrid(std::string* tab, std::string* key
    ,std::string* geo_col, size_t slotnum,float* x0,float* y0,float* slotsize){

  bool val = false;
  string query_exec = "";
  string partTabName;
  string anzSlots = to_string(slotnum);
  string x_start = to_string(*x0);
  string y_start = to_string(*y0);
  string sizSlots = to_string(*slotsize);

  //Dropping parttable
  partTabName = getparttabname(tab,key);
  drop_table(partTabName);

  //creating Index on table
  query_exec =  dbms_connection->get_drop_index(tab) + " "
            "" + dbms_connection->create_geo_index(tab, geo_col);
  val = dbms_connection->sendCommand(&query_exec);

  //
  query_exec = dbms_connection->get_partGrid(tab,key,geo_col,&anzSlots, &x_start
                            , &y_start, &sizSlots, &partTabName);
  if (query_exec != "" && val) val = dbms_connection->sendCommand(&query_exec);

  return val;
} 

/*
3.21 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 bool BasicEngine_Control::getTypeFromSQLQuery(std::string sqlQuery, 
    ListExpr &resultList) {

   return dbms_connection->getTypeFromSQLQuery(sqlQuery, resultList);
 }


/*
3.21 ~getTypeFromSQLQuery~

Get the SECONDO type for the given SQL query.

*/
 ResultIteratorGeneric* BasicEngine_Control::performSQLQuery(
   std::string sqlQuery) {

   return dbms_connection->performSQLQuery(sqlQuery);
 }

/*
3.21 ~shareWorkerRelation~

Share the given worker relation.

*/
bool BasicEngine_Control::shareWorkerRelation(
  string relationName, Relation* relation) {
  
  bool successFlag = true;

  // Get type for secondo object
  SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
  string tn;
  bool defined;
  bool hasTypeName;
  ListExpr typeList;
  Word value;
  value.setAddr(0);

  if(!ctlg->GetObjectExpr(relationName, tn, typeList, value, 
                          defined, hasTypeName)){
     cerr << "Error: Name " << relationName << " is not on object" << endl;
     return false;
  }

  if(!defined){
     cerr << "Error: Undefined objects cannot be shared" << endl;
     return false;
  }
  
  // Write relation to file
  bool isRelation = Relation::checkType(typeList);

  if(! isRelation) {
    cerr << "Error: provided relation name is not a relation" << endl;
    return false;
  }

  string filename = relationName + "_" 
                     + stringutils::int2str(WinUnix::getpid()) 
                     + ".bin";

  if(connections.empty()) {
    cerr << "Error: Worker are empty" << endl;
    return false;
  }

  ConnectionInfo* ci = connections.front();
  ci->saveRelationToFile(typeList, value, filename);

  // Share relation
  for(distributed2::ConnectionInfo* ci: connections) {
    CommandLog commandLog;

    bool result = ci->createOrUpdateRelationFromBinFile(
      relationName, filename, false, commandLog, true,
      false, BasicEngine_Control::defaultTimeout);

    if(! result) {
      cerr << "Error while distributing worker relation to" 
        << ci -> getHost() << " / " << ci -> getPort() << endl;
      successFlag = false;
    }
  }

  // Delete relation file
  if(filename.size()>0){
      FileSystem::DeleteFileOrFolder(filename); 
  }

  if(value.addr){
    SecondoSystem::GetCatalog()->CloseObject(typeList, value);
    value.setAddr(0);
  }

  return successFlag;
}


} /* namespace BasicEngine */
