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
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include "Algebras/Distributed2/ConnectionInfo.h"

#ifndef _BasicEngine_Thread_H_
#define _BasicEngine_Thread_H_

//using namespace distributed2;

namespace BasicEngine {

class BasicEngine_Thread{

public:
  BasicEngine_Thread(distributed2::ConnectionInfo* _ci, std::string _dbname
          , std::string  _port, std::string _init){
    ci = _ci;
    dbname = _dbname;
    port = _port;
    init = _init;
    started = false;
    val = false;
  };

  virtual ~BasicEngine_Thread(){};

  bool getResult(){
       boost::lock_guard<boost::mutex> guard(mtx);
       runner.join();
       return val;
  }

  void startImport(std::string _tab
                  ,std::string _remoteCreateName,std::string _remoteName){
    tab = _tab;
    remoteCreateName = _remoteCreateName;
    remoteName = _remoteName;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      runner = boost::thread(&BasicEngine_Thread::runImport, this);
    };
  };

  void startExport(std::string _tab, std::string _path, std::string _nr){
    tab = _tab;
    path = _path;
    nr = _nr;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      runner = boost::thread(&BasicEngine_Thread::runExport, this);
    };
  };

  void startQuery(std::string _tab, std::string _query){
    tab = _tab;
    query = _query;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      runner = boost::thread(&BasicEngine_Thread::runQuery, this);
    };
  };

  void startCommand(std::string command){
    query = command;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      runner = boost::thread(&BasicEngine_Thread::runCommand, this);
    };
  };

private:
  const int defaultTimeout = 0;
  boost::thread runner;
  boost::mutex mtx;
  std::string tab;
  std::string remoteCreateName;
  std::string remoteName;
  std::string query;
  std::string path;
  std::string nr;

  distributed2::ConnectionInfo* ci;
  std::string dbname;
  std::string  port;
  std::string init;
  bool started;
  bool val;


  bool simpleCommand(std::string cmd){
  int err = 0;
  double rt;
  distributed2::CommandLog CommandLog;
  std::string res;

    //cout << cmd<< endl;
    ci->simpleCommand(cmd,err,res,false,rt,false
                      ,CommandLog,true,defaultTimeout);

    if(err != 0){
      cout << std::string("ErrCode:" + err) + ""
           "" + std::string(" Massage: :" + res) << endl;
      return false;
    }
    return true;
  }

  std::string readFile(std::string path){
  std::string line = "";
  std::ifstream myfile (path);
  std::string res = "";

    if (myfile.is_open()){
      while ( getline (myfile,line)){
        res = res + line;
      }
      myfile.close();
    }
    return res;
  }

  void runImport(){
  std::string importPath;
  std::string cmd;

    //create and open db in dist. worker
    val = ci->switchDatabase(dbname,true, false, true, defaultTimeout);

    //init pg-worker;
    if (val) val = simpleCommand(init);

    //create tab in pg-worker
    if (val){
      importPath = ci->getSendPath() + "/"+ remoteCreateName;
      cmd = "query be_command('"+ readFile(importPath) + "');";
      val = simpleCommand(cmd);
    }

    //delete create-file on system
    if (val) val = simpleCommand("query removeFile('"+ importPath + "')");

    //import data in pg-worker
    if(val){
      importPath = ci->getSendPath() + "/"+ remoteName;
      cmd = "query be_copy('"+ importPath + "','" + tab + "')";
      val = simpleCommand(cmd);
    }

    //delete data file on system
    if(val) val = simpleCommand("query removeFile('"+ importPath + "')");

    //cout<<"wait 10sec" << endl;
    //boost::this_thread::sleep_for(boost::chrono::seconds(10));
  }

  void runExport(){
  std::string from;
  std::string to;
  std::string tab_name = tab + "_" +nr +".bin";
  std::string struct_name = "create" + tab + ".sql";
  std::string transfer_path = path.substr(0,path.find(tab_name));


    //create and open db in dist. worker
    val = ci->switchDatabase(dbname,true, false, true, defaultTimeout);

    //init the dbs connection
    if(val) val = simpleCommand(init);

    //export the table structure file
    if(nr == "1" and val){
      //export tab structure
      val = simpleCommand("query be_struct('"+ tab + "');");

      //move the structure-file into the request-folder
      from = transfer_path + struct_name;
      to = ci->getRequestPath() + "/" + struct_name;
      if(val) val = simpleCommand("query moveFile('"+ from + "','" + to +"')");

      //sending file to master
      if(val) val =(ci->requestFile(struct_name,from,true)==0);

      //delete create file on system
      if(val) val = simpleCommand("query removeFile('"+ to + "')");
    }
    //export the date to a file
    if (val) val = simpleCommand("query be_copy('"+tab+"','"+path+"');");

    //move the data-file to the request-folder
    to = ci->getRequestPath() + "/" + tab_name;
    if(val) val = simpleCommand("query moveFile('"+ path + "','" + to +"')");

    //sendig the File to the master
    if(val) val =(ci->requestFile(tab_name ,transfer_path+ tab_name ,true)==0);

    //delete data file on system
    if(val) val = simpleCommand("query removeFile('"+ to + "')");
  }

  void runQuery(){

    //create and open db in dist. worker
    val = ci->switchDatabase(dbname,true, false, true, defaultTimeout);

    //init the dbs connection
    if (val) val = simpleCommand(init);

    //run the query
    if (val) val = simpleCommand("query be_query(\""
                    "" + query + "\",\"" + tab + "\");");
  }

  void runCommand(){

    //create and open db in dist. worker
    val=ci->switchDatabase(dbname,true, false, true, defaultTimeout);

    //init the dbs connection
    if (val) val = simpleCommand(init);

    //run the command
    val = simpleCommand("query be_command(\"" + query + "\");");
  }
};
};

#endif
