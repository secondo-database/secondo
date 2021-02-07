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

*/

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/algorithm/string.hpp>
#include "Algebras/Distributed2/ConnectionInfo.h"

#include "BasicEngine_Thread.h"

namespace BasicEngine {

/*
4.1.3 ~getResult~

Function for joining the threads.

Returns true if everything is OK and there are no failure.

*/
bool BasicEngine_Thread::getResult(){
   boost::lock_guard<boost::mutex> guard(mtx);
   started =false;
   runner.join();
   return val;
}

/*
4.1.4 ~startImport~

Starting the import thread.

*/
void BasicEngine_Thread::startImport(std::string _tab,
                std::string _remoteCreateName, std::string _remoteName) {
  
  tab = _tab;
  remoteCreateName = _remoteCreateName;
  remoteName = _remoteName;
  boost::lock_guard<boost::mutex> guard(mtx);
  
  if(!started && ci) {
    started = true;
    val = true;
    runner = boost::thread(&BasicEngine_Thread::runImport, this);
  }

}

/*
4.1.5 ~startExport~

Starting the export thread.

*/
void BasicEngine_Thread::startExport(std::string _tab, std::string _path, 
              std::string _nr, std::string _remoteCreateName, 
              std::string _remoteName) {

  tab = _tab;
  path = _path;
  nr = _nr;
  remoteCreateName = _remoteCreateName;
  remoteName = _remoteName;
  boost::lock_guard<boost::mutex> guard(mtx);

  if(!started && ci) {
    started = true;
    val = true;
    runner = boost::thread(&BasicEngine_Thread::runExport, this);
  }
}

/*
4.1.6 ~startQuery~

Starting a query on all workers.

*/
void BasicEngine_Thread::startQuery(std::string _tab, std::string _query) {
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
void BasicEngine_Thread::startCommand(std::string command) {
  query = command;
  boost::lock_guard<boost::mutex> guard(mtx);
  if(!started && ci ){
    started = true;
    val = true;
    runner = boost::thread(&BasicEngine_Thread::runCommand, this);
  };
};

/*
4.3.1 ~simpleCommand~

Execute a command or query on the worker.

Returns true if everything is OK and there are no failure.
Displays an error massage if something goes wrong.

*/
bool BasicEngine_Thread::simpleCommand(std::string *cmd) {

  int err = 0;
  double rt;
  const int defaultTimeout = 0;
  distributed2::CommandLog CommandLog;
  std::string res;

  ci->simpleCommand(*cmd, err, res, false, rt, false,
                    CommandLog, true, defaultTimeout);
  
  if(err != 0){
    cout << std::string("ErrCode:" + err) << endl;
    return false;
  }

  return (res == "(bool TRUE)");
 }

/*
4.3.2 ~runImport~

Starting the import from data at the worker.

*/
void BasicEngine_Thread::runImport() {
  std::string importPath;
  std::string cmd;

  importPath =ci->getSendPath() + "/"+ remoteCreateName;
  cmd = "query be_runsql('"+ importPath + "');";
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
4.3.3 ~runExport~

Starting the export from data at the worker.

*/
void BasicEngine_Thread::runExport() {
  std::string from;
  std::string to;
  std::string cmd;
  std::string transfer_path = path.substr(0,path.find(remoteName));

  //export the table structure file
  if(nr == "1") {
    //export tab structure
    cmd = "query be_struct('"+ tab + "');";
    val = simpleCommand(&cmd);

    //move the structure-file into the request-folder
    from = transfer_path + remoteCreateName;
    to = ci->getRequestPath() + "/" + remoteCreateName;
    cmd ="query moveFile('"+ from + "','" + to +"')";
    if(val) val = simpleCommand(&cmd);

    //sending file to master
    if(val) val = (ci->requestFile(remoteCreateName,from,true)==0);

    //delete create file on system
    cmd ="query removeFile('"+ to + "')";
    if(val) val = simpleCommand(&cmd);
  }

  //export the date to a file
  cmd = "query be_copy('"+ tab+"','"+ path+"');";
  if (val) val = simpleCommand(&cmd);

  //move the data-file to the request-folder
  to = ci->getRequestPath() + "/" + remoteName;
  cmd = "query moveFile('"+ path + "','" + to +"')";
  if(val) val = simpleCommand(&cmd);

  //sendig the File to the master
  if(val) val =(ci->requestFile(remoteName ,
                          transfer_path + remoteName ,true)==0);

  //delete data file on system
  cmd ="query removeFile('"+ to + "')";
  if(val) val = simpleCommand(&cmd);
 }

/*
4.3.4 ~runQuery~

Starting a query at the worker.

*/
void BasicEngine_Thread::runQuery() {
  std::string escapedQuery(query);

  boost::replace_all(escapedQuery, "'", "\\'");

  //run the query
  std::string cmd = "query be_query('"
         "" + escapedQuery + "','"
         "" + tab + "');";

  val = simpleCommand(&cmd);
}

/*
4.3.5 ~runCommand~

Starting a command at the worker.

*/
void BasicEngine_Thread::runCommand() {
  std::string escapedCommand(query);

  //run the command
  boost::replace_all(escapedCommand, "'", "\\'");
  std::string cmd = "query be_command('" + escapedCommand + "');";

  val = simpleCommand(&cmd);
 }

};