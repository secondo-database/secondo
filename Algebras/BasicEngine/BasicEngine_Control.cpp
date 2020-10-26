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

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#include "BasicEngine_Control.h"
#include "FileSystem.h"
#include <boost/algorithm/string.hpp> //wichtig, damit es auf newton läuft


using namespace distributed2;
using namespace std;

namespace BasicEngine {

/*
3 Class ~BasicEngine\_Control~

Implementation.

3.1 ~createConnection~

Creating a specified and saves it in the vec\_ci. Additionally add an entry
to the importer vector.

*/
template<class T>
bool BasicEngine_Control<T>::createConnection(long unsigned int* index){
NestedList* mynl = new NestedList("temp_nested_list");
const int defaultTimeout = 0;

bool val = false;
string host;
string config;
string port;
string dbName;
string dbPort;
string errMsg;
int err = 0;
double rt;
CommandLog CommandLog;
string res;
SecondoInterfaceCS* si ;
ConnectionInfo* ci;
Tuple* tuple;
GenericRelationIterator* it = worker->MakeScan();

  tuple = it->GetNthTuple(*index+1,false);
  si = new SecondoInterfaceCS(true,mynl, true);
  host = tuple->GetAttribute(0)->toText();
  port = tuple->GetAttribute(1)->toText();
  config = tuple->GetAttribute(2)->toText();
  dbPort = tuple->GetAttribute(3)->toText();
  dbName = tuple->GetAttribute(4)->toText();

  if (si->Initialize("", "", host, port, config,"", errMsg, true)){
    ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);
    if(ci){
        vec_ci.push_back(ci);
        importer.push_back(new BasicEngine_Thread(vec_ci[*index]));

        val=ci->switchDatabase(dbName,true, false, true, defaultTimeout);

        ci->simpleCommand(dbs_conn->get_init(&dbName,&dbPort),err,res,false
            ,rt,false,CommandLog,true,defaultTimeout);
        if(err != 0){
          cout << std::string("ErrCode:" + err) << endl;
        }else{
          val = true && val;
        }
    }
  }else{
        cout << std::string("Couldn't connect to secondo-Worker on host"
        "" + host + " with port " + port + "!\n") << endl;
  }
return val;
}

/*
3.2 ~createAllConnection~

Creating all connection from the worker relation.

*/
template<class T>
bool BasicEngine_Control<T>::createAllConnection(){
bool val = true;
long unsigned int i = 0;

  while (i < anzWorker and val){
    val = createConnection(&i);
    i++;
  }
return val;
}

/*
3.3 ~replaceStringAll~

Replacing a string with an other string.

Returns the new string.

*/
template<class T>
string BasicEngine_Control<T>::replaceStringAll(string str
                    , const string& replace, const string& with) {
    if(!replace.empty()) {
      size_t pos = 0;
      while ((pos = str.find(replace, pos)) != string::npos) {
        str.replace(pos, replace.length(), with);
        pos += with.length();
      }
    }
    return str;
}

/*
3.4 ~getjoin~

Returns the join-part of a join-Statement from a given key-list

*/
template<class T>
string BasicEngine_Control<T>::getjoin(string *key){
string res= "ON ";
vector<string> result;
    boost::split(result, *key, boost::is_any_of(","));

    for (long unsigned int i = 0; i < result.size(); i++) {
      if (i>0) res = res + " AND ";
      res = res + " a."+ replaceStringAll(result[i]," ","") + " "
          "  = b." + replaceStringAll(result[i]," ","");
    }
return res;
}

/*
3.5 ~getparttabname~

Returns a name of a table with the keys included.

*/
template<class T>
string BasicEngine_Control<T>::getparttabname(string* tab, string* key){
string res;
  res = *tab + "_" + replaceStringAll(*key,",","_");
  res = replaceStringAll(res," ","");
  return(res);
}

/*
3.6 ~readFile~

Reads a file and returns the input as a string text.

*/
template<class T>
string BasicEngine_Control<T>::readFile(string *path){
string line = "";
ifstream myfile (*path);
string res;

  if (myfile.is_open()){
    while ( getline (myfile,line)){
      res.append(line);
    }
    myfile.close();
  }
  return res;
}

/*
3.7 ~createTabFile~

Creates a table create statement from the input tab
and store the statement in a file.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::createTabFile(string tab){
ofstream write;

  write.open(getFilePath() + createTabFileName(&tab));
  if (write.is_open()){
    write << dbs_conn->createTabFile(&tab);
    write.close();
  }else{ cout << "Couldn't write file into " + getFilePath() + ""
      ". Please check the folder and permissions." << endl;}

return write.good();
}

/*
3.8 ~partRoundRobin~

The data were partitions in the database by round robin.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::partRoundRobin(string* tab
                    ,string* key, int* slotsize){
bool val = false;
string query_exec = "";
string partTabName;
string anzSlots = to_string(*slotsize);

  partTabName = getparttabname(tab,key);
  drop_table(partTabName);

  query_exec = dbs_conn->get_partRoundRobin(tab,key
      ,&anzSlots,&partTabName);
  if (query_exec != "") val = dbs_conn->sendCommand(&query_exec);

return val;
}

/*
3.9 ~exportToWorker~

The data of a partitions table were sanded to the worker
and after that imported by the worker.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::exportToWorker(string *tab){
bool val = true;
string query_exec;
long unsigned int index = 0;
string remoteName;
string localName;
string importPath;
string strindex;
SecondoInterfaceCS* si ;

string remoteCreateName = createTabFileName(tab);
string localCreateName = getFilePath() + remoteCreateName;

  while(index < anzWorker and val){
    if (vec_ci[index]){
      si = vec_ci[index]->getInterface();

      //sending data
      strindex = to_string(index+1);
      remoteName = dbs_conn->get_partTabName(tab,&strindex);
      localName = getFilePath() + remoteName;
      val = (si->sendFile(localName, remoteName, true) == 0);
      val = val && (remove(localName.c_str()) == 0);
      if (!val){
        cout << "\n Couldn't send the data to the worker." << endl;
        return val;}

      //sending create Table
      val = (si->sendFile(localCreateName, remoteCreateName, true) == 0);
      if (!val){
    cout << "\n Couldn't send the structure-file to the worker." << endl;
        return val;}

      index++;
      } else{
        createConnection(&index);
        if(!vec_ci[index]) val = false; ;
      }
  };

  if(val){
    val = (remove(localCreateName.c_str()) == 0);
    //doing the import with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      strindex = to_string(i+1);
      remoteName = dbs_conn->get_partTabName(tab
                ,&strindex);
      importer[i]->startImport(*tab,remoteCreateName,remoteName);
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }else cout << "\n Something goes wrong with the"
		  " export or the transfer." << endl;

  if(!val) cout << "\n Something goes wrong with the "
		  "import at the worker." << endl;
return val;
}

/*
3.10 ~partHash~

The data were partitions in the database by an hash value.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::partHash(string* tab
                    , string* key, int* slotsize){
bool val = false;
string query_exec = "";
string partTabName;
string anzSlots = to_string(*slotsize);

  partTabName = getparttabname(tab,key);
  drop_table(partTabName);

  query_exec = dbs_conn->get_partHash(tab,key
    ,&anzSlots,&partTabName);
  if (query_exec != "")  val = dbs_conn->sendCommand(&query_exec);

return val;
}

/*
3.11 ~partFun~

The data were partitions in the database by an defined function.
This function have to be defined before using it.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::partFun(string* tab
                    , string* key,string* fun, int* slotsize){
bool val = false;
string query_exec = "";
string partTabName = getparttabname(tab,key);
string anzSlots = to_string(*slotsize);

  drop_table(partTabName);

  query_exec = dbs_conn->get_partFun(tab,key
      ,&anzSlots,fun,&partTabName);

  if (query_exec != "") val = dbs_conn->sendCommand(&query_exec);

return val;
}

/*
3.12 ~exportData~

Exporting the data from the DBMS to a local file.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::exportData(string* tab, string* key,
   long unsigned int* slotsize){
bool val = true;
string path = getFilePath();
string parttabname = getparttabname(tab,key);
string join = getjoin(key);
string strindex;
long unsigned int i;

    for(i=1;i<=anzWorker;i++){
      strindex = to_string(i);
      val = val && sendCommand(dbs_conn->get_exportData(tab
                   ,&parttabname,&join,&strindex,&path,slotsize));
    }

return val;
}

/*
3.13 ~importData~

Importing data from a local file into the dbms. At first the
table will be created and after that starts the import from a file.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::importData(string *tab){
bool val = true;
string full_path;
string cmd;
long unsigned int i;

  //create Table
  full_path =getFilePath() + createTabFileName(tab);
  cmd = readFile(&full_path);

  val = val && dbs_conn->sendCommand(&cmd);
  if(!val) return val;
  FileSystem::DeleteFileOrFolder(full_path);

  //import data (local files from worker)
  for(i=1;i<=anzWorker;i++){
    full_path = getFilePath() + *tab + "_" + to_string(i)+".bin";
    val = val && copy(full_path,*tab,true);
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
template<class T>
bool BasicEngine_Control<T>::partTable(string tab
            , string key, string art, int slotsize){
bool val = true;

  if (boost::iequals(art, "RR")){val = partRoundRobin(&tab, &key, &slotsize);}
  else if (boost::iequals(art, "Hash")){val = partHash(&tab, &key, &slotsize);}
  else {val = partFun(&tab, &key, &art, &slotsize);};
  if(!val){
    cout << "\n Couldn't partition the table." << endl;
    return val;
  }

  val =  exportData(&tab, &key, &anzWorker);
  if(!val){
    cout << "\n Couldn't export the data from the table." << endl;
    return val;
  }

  val = createTabFile(tab);
  if(!val){
cout << "\n Couldn't create the structure-file" << endl;
    return val;
  }

  val = exportToWorker(&tab);
  if(!val){cout << "\n Couldn't transfer the data to the worker." << endl;}

return val;
}

/*
3.15 ~munion~

Exports the data from the worker and sending them to the master.
The master imports them into the local db.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::munion(string tab){
bool val = true;
string path;
string strindex;

  //doing the export with one thread for each worker
  for(size_t i=0;i<importer.size();i++){
    strindex = to_string(i+1);
    path = getFilePath() + ""
      "" + dbs_conn->get_partTabName(&tab,&strindex);
    importer[i]->startExport(tab,path,strindex);
  }

  //waiting for finishing the threads
  for(size_t i=0;i<importer.size();i++){
    val = val && importer[i]->getResult();
  }

  //import in local PG-Master
  if(val) val = importData(&tab);

return val;
};

/*
3.16 ~mquery~

The multi query sends and execute a query to all worker
and stores the result in a table.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::mquery(string query
                    , string tab){
bool val = true;

 //doing the query with one thread for each worker
 for(size_t i=0;i<importer.size();i++){
   importer[i]->startQuery(tab,query);
 }

 //waiting for finishing the threads
 for(size_t i=0;i<importer.size();i++){
   val = val && importer[i]->getResult();
 }

return val;
}

/*
3.17 ~mcommand~

The multi command sends and execute a query to all worker.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::mcommand(string query){
bool val = true;

 //doing the command with one thread for each worker
 for(size_t i=0;i<importer.size();i++){
   importer[i]->startCommand(query);
 }

 //waiting for finishing the threads
 for(size_t i=0;i<importer.size();i++){
   val = val && importer[i]->getResult();
 }

return val;
}

/*
3.18 ~checkConn~

Checking the Connection to the secondary Master System and to the Worker.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::checkConn(){
bool val = true;
long unsigned int i = 0;
const int defaultTimeout = 0;
CommandLog CommandLog;

  //checking connection to the worker
  if (vec_ci.size() == anzWorker){
    while (i < anzWorker and val and anzWorker > 0){
      val = vec_ci[i]->check(false,CommandLog,defaultTimeout);
      i++;
    }
     //checking the connection to the secondary dbms system
     val = val && dbs_conn->checkConn();
  }
  else{
    val = false;
  }

return val;
}

/*
3.19 ~runsql~

Runs a query from a file.

Returns true if everything is OK and there are no failure.

*/
template<class T>
bool BasicEngine_Control<T>::runsql(string filepath){
bool val = false;
string query;

  //reading the file
  query = readFile(&filepath);

  //execute the sql-Statement
  if (query != "") val = dbs_conn->sendCommand(&query);

return val;
}

} /* namespace BasicEngine */
