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

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include "Algebras/Distributed2/ConnectionInfo.h"

#ifndef _BasicEngine_Thread_H_
#define _BasicEngine_Thread_H_

namespace BasicEngine {

/*
4 Class ~BasicEngine\_Thread~

This class represents the controling from the system.

*/
class BasicEngine_Thread{

public:

  /*
  4.1 Public Methods

  4.1.1 Constructor
  */
  BasicEngine_Thread(distributed2::ConnectionInfo* _ci){ci = _ci; };

  /*
  4.1.2 Destructor

  */
  virtual ~BasicEngine_Thread(){};

  /*
  4.1.3 ~getResult~

  Function for joining the threads.

  Returns true if everything is OK and there are no failure.

  */
  bool getResult(){
       boost::lock_guard<boost::mutex> guard(mtx);
       runner.join();
       started =false;
       return val;
  }

  /*
  4.1.4 ~startImport~

  Starting the import thread.

  */
  void startImport(std::string _tab
                  ,std::string _remoteCreateName,std::string _remoteName){
    tab = _tab;
    remoteCreateName = _remoteCreateName;
    remoteName = _remoteName;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci){
      started = true;
      val = true;
      runner = boost::thread(&BasicEngine_Thread::runImport, this);
    };
  };

  /*
  4.1.5 ~startExport~

  Starting the export thread.

  */
  void startExport(std::string _tab, std::string _path, std::string _nr){
    tab = _tab;
    path = _path;
    nr = _nr;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci){
      started = true;
      val = true;
      runner = boost::thread(&BasicEngine_Thread::runExport, this);
    };
  };

  /*
  4.1.6 ~startExport~

  Starting a query on all workers.

  */
  void startQuery(std::string _tab, std::string _query){
    tab = _tab;
    query = _query;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      val = true;
      runner = boost::thread(&BasicEngine_Thread::runQuery, this);
    };
  };

  /*
  4.1.7 ~startCommand~

  Starting a command on all workers.

  */
  void startCommand(std::string command){
    query = command;
    boost::lock_guard<boost::mutex> guard(mtx);
    if(!started && ci ){
      started = true;
      val = true;
      runner = boost::thread(&BasicEngine_Thread::runCommand, this);
    };
  };

private:

  /*
  4.2 Members

  4.2.1 ~runner~

  The runner for the thread.

  */
  boost::thread runner;

  /*
  4.2.2 ~mtx~

  The variable for the mtex.

  */
  boost::mutex mtx;

  /*
  4.2.3 ~tab~

   A variable for the tab name.

  */
  std::string tab;

  /*
  4.2.4 ~remoteCreateName~

  A variable for the name of the file,
  which stores a Create-Statement.

  */
  std::string remoteCreateName;

  /*
  4.2.5 ~remoteName~

  A variable for the name of the target table.

  */
  std::string remoteName;

  /*
  4.2.6 ~query~

  A variable for the query.

  */
  std::string query;

  /*
  4.2.7 ~path~

  A variable for the path.

  */
  std::string path;

  /*
  4.2.8 ~nr~

  A variable for a number of this worker.

  */
  std::string nr;

  /*
  4.2.9 ~ci~

  This is the connection to this worker.

  */
  distributed2::ConnectionInfo* ci;

  /*
  4.2.10 ~started~

  This boolean variable shows the status of the thread.
  If one thread is running then its TRUE, else its FALSE.

  */
  bool started = false;

  /*
  4.2.11 ~val~

  This boolean variable shows if there was anywhere a failure.

  */
  bool val = true;

  /*
  4.3 Private Methods

  4.3.1 ~simpleCommand~

  Execute a command or query on the worker.

  Returns true if everything is OK and there are no failure.
  Displays an error massage if something goes wrong.

  */
  bool simpleCommand(std::string *cmd){
  int err = 0;
  double rt;
  const int defaultTimeout = 0;
  distributed2::CommandLog CommandLog;
  std::string res;

    //cout << *cmd<< endl;
    ci->simpleCommand(*cmd,err,res,false,rt,false
                      ,CommandLog,true,defaultTimeout);

    if(err != 0){
      cout << std::string("ErrCode:" + err) + ""
           "" + std::string(" Massage: :" + res) << endl;
      return false;
    }
    return true;
  }

  /*
  4.3.2 ~readFile~

  Reading a file and returns the text of this file as a string.

  */
  std::string readFile(std::string* path){
  std::string line = "";
  std::ifstream myfile (*path);
  std::string res = "";

    if (myfile.is_open()){
      while ( getline (myfile,line)){
        res = res + line;
      }
      myfile.close();
    }
    return res;
  }

  /*
  4.3.3 ~runImport~

  Starting the import from data at the worker.

  */
  void runImport(){
  std::string importPath;
  std::string cmd;

    importPath =ci->getSendPath() + "/"+ remoteCreateName;
    cmd = "query be_command('"+ readFile(&importPath) + "');";
    val = simpleCommand(&cmd);

    //delete create-file on system
    cmd = "query removeFile('"+ importPath + "')";
    if (val) val = simpleCommand(&cmd);

    //import data in pg-worker
    if(val){
      importPath = ci->getSendPath() + "/"+ remoteName;
      cmd = "query be_copy('"+ importPath + "','" + tab + "')";
      val = simpleCommand(&cmd);
    }

    //delete data file on system
    cmd = "query removeFile('"+ importPath + "')";
    if(val) val = simpleCommand(&cmd);

    //cout<<"wait 10sec" << endl;
    //boost::this_thread::sleep_for(boost::chrono::seconds(10));
  }

  /*
  4.3.4 ~runExport~

  Starting the export from data at the worker.

  */
  void runExport(){
  std::string from;
  std::string to;
  std::string cmd;
  std::string tab_name = tab + "_" + nr +".bin";
  std::string struct_name = "create" + tab + ".sql";
  std::string transfer_path = path.substr(0,path.find(tab_name));

    //export the table structure file
    if(nr == "1"){
      //export tab structure
      cmd="query be_struct('"+ tab + "');";
      val = simpleCommand(&cmd);

      //move the structure-file into the request-folder
      from = transfer_path + struct_name;
      to = ci->getRequestPath() + "/" + struct_name;
      cmd ="query moveFile('"+ from + "','" + to +"')";
      if(val) val = simpleCommand(&cmd);

      //sending file to master
      if(val) val =(ci->requestFile(struct_name,from,true)==0);

      //delete create file on system
      cmd ="query removeFile('"+ to + "')";
      if(val) val = simpleCommand(&cmd);
    }
    //export the date to a file
    cmd = "query be_copy('"+ tab+"','"+ path+"');";
    if (val) val = simpleCommand(&cmd);

    //move the data-file to the request-folder
    to = ci->getRequestPath() + "/" + tab_name;
    cmd = "query moveFile('"+ path + "','" + to +"')";
    if(val) val = simpleCommand(&cmd);

    //sendig the File to the master
    if(val) val =(ci->requestFile(tab_name ,transfer_path+ tab_name ,true)==0);

    //delete data file on system
    cmd ="query removeFile('"+ to + "')";
    if(val) val = simpleCommand(&cmd);
  }

  /*
  4.3.5 ~runQuery~

  Starting a query at the worker.

  */
  void runQuery(){
  std::string cmd;
    //run the query
  cmd = "query be_query(\""
            "" + query + "\",\"" + tab + "\");";
    val = simpleCommand(&cmd);
  }

  /*
  4.3.6 ~runCommand~

  Starting a command at the worker.

  */
  void runCommand(){
  std::string cmd;
    //run the command
    cmd = "query be_command(\"" + query + "\");";
    val = simpleCommand(&cmd);
  }
};
}; /* namespace BasicEngine */

#endif //_BasicEngine_Thread_H_
