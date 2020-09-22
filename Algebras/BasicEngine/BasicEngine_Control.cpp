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
#include "BasicEngine_Control.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "StandardTypes.h"
#include "Algebras/Distributed2/ConnectionInfo.h"
#include <boost/algorithm/string.hpp>
#include <bits/stdc++.h>
#include "FileSystem.h"

using namespace distributed2;

namespace BasicEngine {

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

template<class T>
string BasicEngine_Control<T>::getjoin(string key){
string res= "ON ";
vector<string> result;
    boost::split(result, key, boost::is_any_of(","));

    for (long unsigned int i = 0; i < result.size(); i++) {
      if (i>0) res = res + " AND ";
      res = res + " a."+ replaceStringAll(result[i]," ","") + " "
          "  = b." + replaceStringAll(result[i]," ","");
    }
return res;
}

template<class T>
string BasicEngine_Control<T>::getparttabname(string tab, string key){
string res;
  res = tab + "_" + replaceStringAll(key,",","_");
  res = replaceStringAll(res," ","");
  return(res);
}

template<class T>
string BasicEngine_Control<T>::readFile(string path){
string line = "";
ifstream myfile (path);
string res = "";

  if (myfile.is_open()){
    while ( getline (myfile,line)){
      res = res + line;
    }
    myfile.close();
  }
  return res;
}

template<class T>
bool BasicEngine_Control<T>::createTabFile(string tab){
ofstream write;

  write.open(getFilePath() + createTabFileName(tab));
  if (write.is_open()){
    write << dbs_conn->createTabFile(tab);
    write.close();
  }else{ cout << "Couldn't write file into " + getFilePath() + ""
      ". Please check the folder and permissions." << endl;}

return write.good();
}

template<class T>
bool BasicEngine_Control<T>::partRoundRobin(string tab
                    , string key, int anzWorker){
bool val = false;
string query_exec;

  drop_table(getparttabname(tab,key));

  query_exec = dbs_conn->get_partRoundRobin(tab,key
      ,to_string(anzWorker),getparttabname(tab,key));
  val = dbs_conn->sendCommand(query_exec);

return val;
}

template<class T>
bool BasicEngine_Control<T>::partHash(string tab
                    , string key, int anzWorker){
bool val = false;
string query_exec;

  drop_table(getparttabname(tab,key));

  query_exec = dbs_conn->get_partHash(tab,key
      ,to_string(anzWorker),getparttabname(tab,key));
  val = dbs_conn->sendCommand(query_exec);

return val;
}

template<class T>
bool BasicEngine_Control<T>::partFun(string tab
                    , string key, int anzWorker, string fun){
bool val = false;
string query_exec;

  drop_table(getparttabname(tab,key));

  query_exec = dbs_conn->get_partFun(tab,key
      ,to_string(anzWorker),fun,getparttabname(tab,key));

  if (query_exec != "") val = dbs_conn->sendCommand(query_exec);

return val;
}

template<class T>
bool BasicEngine_Control<T>::exportData(string tab, string key, int anzWorker){
string const path = getFilePath();
bool val = true;

    for(int i=1;i<=anzWorker;i++){
      val = val && sendCommand(dbs_conn->get_exportData(tab
                   ,getparttabname(tab,key),getjoin(key),to_string(i),path));
    }

return val;
}

template<class T>
bool BasicEngine_Control<T>::importData(string tab, int anzWorker){
string full_path;
bool val = true;

  //create Table
  full_path =getFilePath() + createTabFileName(tab);

  val = val && dbs_conn->sendCommand(readFile(full_path));
  if(!val) return val;
  FileSystem::DeleteFileOrFolder(full_path);

  //import data (local files from worker)
    for(int i=1;i<=anzWorker;i++){
      full_path = getFilePath() + tab + "_" + to_string(i)+".bin";
      val = val && copy(full_path,tab,true);
      FileSystem::DeleteFileOrFolder(full_path);
    }
return val;
}

template<class T>
bool BasicEngine_Control<T>::partTable(string tab
            , string key, int anzWorker, string art){
bool val = true;

  if (art == "RR"){val = partRoundRobin(tab, key, anzWorker);}
  else if (art == "Hash") {val = partHash(tab, key, anzWorker);}
  else {val = partFun(tab, key, anzWorker, art);};
  if(!val){return val;}

  val =  exportData(tab, key ,anzWorker);
  if(!val){return val;}

  val = createTabFile(tab);
  if(!val){return val;}

return val;
}

template<class T>
bool BasicEngine_Control<T>::munion(string tab, Relation worker){
bool val = true;
string path;
string host;
string config;
string port;
string dbName;
string dbPort;
string tabname;
Tuple* tuple;
SecondoInterfaceCS* si ;
string errMsg;
ConnectionInfo* ci;
NestedList* mynl = new NestedList("temp_nested_list");
GenericRelationIterator* it = worker.MakeScan();

  vector<BasicEngine_Thread*> importer;

  while((tuple = it->GetNextTuple()) and val){
    si = new SecondoInterfaceCS(true,mynl, true);
    host = tuple->GetAttribute(0)->toText();
    port = tuple->GetAttribute(1)->toText();
    config = tuple->GetAttribute(2)->toText();

    if (si->Initialize("","",host,port, config,"", errMsg, true)){
      ci = new ConnectionInfo(host,stoi(port),config,si,mynl,0,defaultTimeout);
      dbName = tuple->GetAttribute(4)->toText();
      dbPort = tuple->GetAttribute(3)->toText();
      importer.push_back(new BasicEngine_Thread(ci,dbName
                ,dbPort,dbs_conn->get_init(dbName,dbPort)));
    }else val = false;
  }

  if (val){
    //doing the export with one thread for each worker
    for(size_t i=0;i<importer.size();i++){
      //tabname = tab->GetValue() + "_" + to_string(i);
      path = getFilePath() + dbs_conn->get_partTabName(tab,to_string(i+1));
      importer[i]->startExport(tab, path,to_string(i+1));
    }

    //waiting for finishing the threads
    for(size_t i=0;i<importer.size();i++){
      val = val && importer[i]->getResult();
    }
  }

  //import in local PG-Master
  if(val) val = importData(tab, worker.GetNoTuples());

return val;
};

} /* namespace BasicEngine */
