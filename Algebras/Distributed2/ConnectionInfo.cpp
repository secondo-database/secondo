/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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
//[_][\_]

*/
#include "ConnectionInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Timeout.h"
#include "Dist2Helper.h"
#include "FileSystem.h"
#include "FileRelations.h"
#include "FileAttribute.h"


extern boost::mutex nlparsemtx;
boost::mutex createRelMut;
boost::mutex copylistmutex;

extern NestedList* nl;

namespace distributed2
{

/*
 1.1 Constructor

 Creates a new connection instance.

*/
ConnectionInfo::ConnectionInfo(const std::string& _host,
                               const int _port,
                               const std::string& _config,
                               SecondoInterfaceCS* _si,
                               NestedList* _mynl,
                               const size_t timeout,
                               const int heartbeat) :
        host(_host), port(_port), config(_config), si(_si)
{
    mynl = _mynl;
    secondoHome = "";
    requestFolder = "";
    requestPath = "";
    sendFolder = "";
    sendPath = "";
    serverPID=0;
    num = -1;
    cmdLog = 0; 
    noReferences = 1;
    guard_type guard(simtx);
    tonotifier = new TimeoutNotifier<ConnectionInfo>(this);
    hbobserver = new HeartbeatObserver<ConnectionInfo>(this);
    if(si!=0){
      si->setHeartbeat(heartbeat,heartbeat);
      si->addMessageHandler(hbobserver);
      try{
        if(timeout>0){
           startTimeout(timeout,false);
        }
        serverPID = si->getPid();
        secondoHome = si->getHome();
        sendFolder = si->getSendFileFolder();
        requestFolder = si->getRequestFileFolder();
        requestPath = si->getRequestFilePath();
        sendPath = si->getSendFilePath();
        if(timeout>0){
           stopTimeout(false);
        }
      } catch(...){
        std::cerr << " Problem in ConnectionInfo constructor" << endl;
        if(timeout>0){
           stopTimeout(false);
        }
      }
    }
}

bool ConnectionInfo::reconnect(bool showCommands, CommandLog& log, 
                               const size_t timeout,
                               const int heartbeat){

    //cout << "try to reconnect " << host <<":" << port << endl;

    guard_type guard(simtx);
    si->removeMessageHandler(hbobserver);
    try{
       this->si->Terminate();
    } catch(...) {
       std::cerr << "reconnect: Exception during terminate" << endl;
    }
    try{
       delete this->si;
    } catch(...) {
       std::cerr << "reconnect: Exception during deletion of si " << endl;
    }
    si = new SecondoInterfaceCS(true, mynl, true);
    si->addMessageHandler(hbobserver);
    std::string user = "";
    std::string passwd = "";
    std::string errMsg;
    si->setMaxAttempts(4);
    si->setTimeout(2);
    if (!si->Initialize(user, passwd, host, stringutils::int2str(port), config,
                        "",errMsg, true)) {
      std::cerr << "reconnect: Initialisation of newly created "
              "secondoInterface failed" << endl;
      std::cerr << "Error : " << errMsg << endl;
      return false;
    }
    if(si!=0){
      try{
        if(timeout>0){
           startTimeout(timeout,false);
        }
        serverPID = si->getPid();
        switchDatabase(SecondoSystem::GetInstance()->GetDatabaseName(), 
                       true, false, true, timeout);
        retrieveSecondoHome(showCommands,log);
        si->setHeartbeat(heartbeat, heartbeat);
        if(timeout>0){
           stopTimeout(false);
        }
      } catch(...){
        std::cerr << "error during collecting standard information" << endl;
        if(timeout>0){
           stopTimeout(false);
        }
        return false;
      }
    }
    return si!=0;
}


/*
 1.2 Destructor

 Terminates the connection and destroys the object.

*/
ConnectionInfo::~ConnectionInfo()
{

   //cout << "Destroy connectionInfo" << endl;

   try
    {
        guard_type guard(simtx);
        si->Terminate();
    }
    catch(...) {}
    delete si;
    si = 0;
    delete mynl;
    delete hbobserver;
    delete tonotifier;
}

void ConnectionInfo::deleteIfAllowed() {
       norefmtx.lock();
       assert(noReferences>0);
       noReferences--;
       if(noReferences==0){
          norefmtx.unlock();
          delete this;
       } else {
          norefmtx.unlock();
       }
}


/*
 1.3 ~getHost~

 returns the remote host.

*/
std::string ConnectionInfo::getHost() const
{
    return host;
}

/*
 1.4 getPort

*/
int ConnectionInfo::getPort() const
{
    return port;
}

/*
 1.5 getConfig

 Returns the configuration file used for the client.
 Has nothing to do with the configuration file used by the
 remove monitor.

*/
std::string ConnectionInfo::getConfig() const
{
    return config;
}

/*
 1.6 check

 Checks whether the remote server is working by sending a simple command.
 Here, the command is always executed even if the command logger is not null.

*/
bool ConnectionInfo::check(bool showCommands,  
                           CommandLog& commandLog,
                           const size_t timeout)
{
    ListExpr res;
    std::string cmd = "list databases";
    SecErrInfo err;
    {
        guard_type guard(simtx);
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        if(timeout>0){
           startTimeout(timeout,true);
        }
        si->Secondo(cmd, res, err);
        if(err.code){
           //cerr << "Secondo function used during check() returns an error"
           //     << endl;
           //cerr << "Code is " << err.code << endl;
           //cerr << "This means " << err.msg << endl;
           return false;
        }


        double rt = sw.diffSecondsReal();
        std::string home = this->getSecondoHome(showCommands, commandLog);
        commandLog.insert(this, this->getHost(), 
                          home,
                          cmd, rt, err.code);
        showCommand(si, host, port, cmd, false, showCommands);
        if(timeout>0){
          stopTimeout(true);
        }
    }
    return err.code == 0;
}

/*
 1.7 setId

 Sets the id.

*/
void ConnectionInfo::setId(const int i)
{
    guard_type guard(simtx);
    if (si) {
        si->setId(i);
    }
}

/*
 1.8 simpleCommand

 Performs a command on the remote server. The result is stored as a string.


*/
void ConnectionInfo::simpleCommand(std::string command1,
                                   int& err,
                                   std::string& result,
                                   bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   CommandLog& commandLog,
                                   bool forceExec /*=false*/,
                                   const size_t timeout /*=0*/)
{
    std::string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    SecErrInfo serr;
    ListExpr resList;
    {
        guard_type guard(simtx);
        StopWatch sw;
        showCommand(si, host, port, command, true, showCommands);
        if(!cmdLog || forceExec){
          if(timeout>0){
             startTimeout(timeout,true);
          }
          si->Secondo(command, resList, serr);
          if(timeout>0){
             stopTimeout(true);
          }
        } else {
          cmdLog->insert(this, command);
          serr.code = 0;
          serr.msg = "command not evaluated";
          resList = mynl->TheEmptyList();
        }
        showCommand(si, host, port, command, false, showCommands);
        runtime = sw.diffSecondsReal();
        err = serr.code;
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          command, runtime, err);
        if (err == 0)
        {
            result = mynl->ToString(resList);
        } else
        {
            result = si->GetErrorMessage(err);
        }
    }
}

/*
 1.9 getSecondoHome

 Returns the path of the secondo databases of the remote server.

*/
std::string ConnectionInfo::getSecondoHome(bool showCommands,
                                      CommandLog& commandLog)
{
    return secondoHome;
}

/*
 1.10 cleanUp

 This command removes temporal objects on the remote server.
 Such objects having names starting with TMP[_]. Furthermore,
 files starting with TMP are removed within the dfarray directory
 of the remote server.

*/

bool ConnectionInfo::cleanUp(bool showCommands,
                             CommandLog& commandLog, 
                             const size_t timeout)
{   
    guard_type guard(simtx);
    // first step : remove database objects
    std::string command = "query getcatalog() "
            "filter[.ObjectName startsWith \"TMP_\"] "
            "extend[OK : deleteObject(.ObjectName)] "
            " count";
    int err;
    std::string res;
    double rt;
    simpleCommand(command, err, res, false, rt, showCommands, 
                  commandLog, false,timeout);
    bool result = err == 0;
    std::string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    std::string path = getSecondoHome(showCommands, commandLog) 
                + "/dfarrays/" + dbname + "/";
    command = "query getDirectory('" + path + "') "
            "filter[basename(.) startsWith \"TMP_\"] "
            "namedtransformstream[F] "
            "extend[ OK : removeDirectory(.F, TRUE)] count";
    simpleCommand(command, err, res, false, rt, showCommands, 
                  commandLog, false, timeout);
    result = result && (err == 0);
    return result;
}


bool ConnectionInfo::cleanUp1(const size_t timeout) {
   static CommandLog log;
   return cleanUp(false,log, timeout);
}

/*
 1.11 switchDatabase

 Switches the remote server to another database.

*/
bool ConnectionInfo::switchDatabase(const std::string& dbname,
                                    bool createifnotexists,
                                    bool showCommands,
                                    bool forceExec,
                                    const size_t timeout)
{
    guard_type guard(simtx);
    
    // close database ignore errors
    SecErrInfo serr;
    ListExpr resList;
    std::string cmd = "close database";
    showCommand(si, host, port, cmd, true, showCommands);
    if(timeout>0){
       startTimeout(timeout,false);
    }
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    showCommand(si, host, port, cmd, false, showCommands);
    // create database ignore errors
    if (createifnotexists)
    {
        cmd = "create database " + dbname;
        showCommand(si, host, port, cmd, true, showCommands);
        if(!cmdLog || forceExec){
           si->Secondo(cmd, resList, serr);
        } else {
           cmdLog->insert(this, cmd);
           resList = mynl->TheEmptyList();
           serr.code = 0;
           serr.msg = "command not executed";
        }
        showCommand(si, host, port, cmd, false, showCommands);
    }
    // open database
    cmd = "open database " + dbname;
    showCommand(si, host, port, cmd, true, showCommands);
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    showCommand(si, host, port, cmd, false, showCommands);
    bool res = serr.code == 0;
    if(timeout>0){
      stopTimeout(false);
    }
    return res;
}

/*
 1.12 simpleCommand

 This variant of ~simpleCommand~ returns the result as a list.


*/
void ConnectionInfo::simpleCommand(const std::string& command1,
                                   int& error,
                                   std::string& errMsg,
                                   std::string& resList,
                                   const bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   CommandLog& commandLog,
                                   bool forceExec,
                                   const size_t timeout)
{

    std::string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    {
        guard_type guard(simtx);
        if(timeout>0){
          startTimeout(timeout,true);
        } 
        SecErrInfo serr;
        ListExpr myResList = mynl->TheEmptyList();
        StopWatch sw;
        showCommand(si, host, port, command, true, showCommands);
        if(!cmdLog || forceExec){
           si->Secondo(command, myResList, serr);
        } else {
           cmdLog->insert(this, command);
           myResList = mynl->TheEmptyList();
           serr.code = 0;
           serr.msg = "command not executed";
        }
        if(timeout>0){
          stopTimeout(true);
        }

        showCommand(si, host, port, command, false, showCommands);
        runtime = sw.diffSecondsReal();
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          command, runtime, serr.code);
        error = serr.code;
        errMsg = serr.msg;

        resList = mynl->ToString(myResList);
        mynl->Destroy(myResList);
    }
}

/*
 1.13 simpleCommandFromList

 Performs a command that is given in nested list format.

*/
void ConnectionInfo::simpleCommandFromList(const std::string& command1,
                                           int& error,
                                           std::string& errMsg,
                                           std::string& resList,
                                           const bool rewrite,
                                           double& runtime,
                                           bool showCommands,
                                           CommandLog& commandLog,
                                           bool forceExec,
                                           int timeout)
{
   guard_type guard(simtx);
   if(timeout>0){
       startTimeout(timeout,true);
   }
   try{
    std::string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }

    ListExpr cmd = mynl->TheEmptyList();
    {
        boost::lock_guard < boost::mutex > guard(nlparsemtx);
        if (!mynl->ReadFromString(command, cmd))
        {
            error = 3;
            errMsg = "error in parsing list";
            return;
        }
    }
    SecErrInfo serr;
    ListExpr myResList = mynl->TheEmptyList();
    StopWatch sw;
    showCommand(si, host, port, command, true, showCommands);
    if(!cmdLog || forceExec){
       si->Secondo(cmd, myResList, serr);
    } else {
       cmdLog->insert(this, mynl->ToString(cmd));
       myResList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    if(serr.code != 0){
       std::cerr << "error during secondo command" << endl;
       std::cerr << "code : " << serr.code << endl;
       std::cerr << "msg : " << serr.msg << endl;
    }


    showCommand(si, host, port, command, false, showCommands);
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          command, runtime, serr.code);
    error = serr.code;
    errMsg = serr.msg;

    resList = mynl->ToString(myResList);
    if (mynl->AtomType(cmd) != NoAtom && !mynl->IsEmpty(cmd))
    {
        mynl->Destroy(cmd);
    }
    if (mynl->AtomType(myResList) != NoAtom && !mynl->IsEmpty(myResList))
    {
        mynl->Destroy(myResList);
    }
  } catch(...){
     std::cerr << "Exception during simpleCommandFromList " << endl;
  }
  if(timeout>0){
      stopTimeout(true);
  }
}

/*
 1.13 simpleCommand

 This variant provides the result as a list.

*/

void ConnectionInfo::simpleCommand(const std::string& command1,
                                   int& error,
                                   std::string& errMsg,
                                   ListExpr& resList,
                                   const bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   CommandLog& commandLog,
                                   bool forceExec,
                                   const size_t timeout)
{

    std::string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    guard_type guard(simtx);
    if(timeout>0){
       startTimeout(timeout,true);
    } 
    SecErrInfo serr;
    ListExpr myResList = mynl->TheEmptyList();
    StopWatch sw;
    showCommand(si, host, port, command, true, showCommands);
    if(!cmdLog || forceExec){
       si->Secondo(command, myResList, serr);
    } else {
       cmdLog->insert(this, command);
       myResList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    if(timeout>0){
       stopTimeout(true);
    } 
    showCommand(si, host, port, command, false, showCommands);
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog),
                      command, runtime, serr.code);
    error = serr.code;
    errMsg = serr.msg;
    if(nl!=mynl){
        // copy resultlist from local nested list to global nested list
        boost::lock_guard < boost::mutex > guard(copylistmutex);
        resList = mynl->CopyList(myResList, nl);
        mynl->Destroy(myResList);
    } else {
       resList = myResList;
    }    
}

/*
 1.14 serverPid

 returns the process id of the remote server

*/
int ConnectionInfo::serverPid()
{
    return serverPID;
}

/*
 1.15 sendFile

 transfers a locally stored file to the remote server.
 It returns an error code.

*/
int ConnectionInfo::sendFile(const std::string& local,
                             const std::string& remote,
                             const bool allowOverwrite,
                             const size_t timeout)
{
    guard_type guard(simtx);
    if(timeout>0){
      startTimeout(timeout,false);
    }
    int res = si->sendFile(local, remote, allowOverwrite);
    if(timeout>0){
      stopTimeout(false);
    }
    return res;
}

/*
 1.16 requestFile

 Transfers a remotely stored file to the local file system.

*/
int ConnectionInfo::requestFile(const std::string& remote,
                                const std::string& local,
                                const bool allowOverwrite,
                                const size_t timeout)
{
    guard_type guard(simtx);
    if(timeout>0){
      startTimeout(timeout,false);
    }
    int res = si->requestFile(remote, local, allowOverwrite);
    if(timeout>0){
      stopTimeout(false);
    }
    return res;
}

/*
 1.17 getRequestFolder

 Returns the path on remote machine for requesting files.

*/
std::string ConnectionInfo::getRequestFolder()
{
    return requestFolder;
}

std::string ConnectionInfo::getRequestPath(){
   return requestPath;
} 


/*
 1.18 getSendFolder

 returns the folder the remote machine where files are stored.
 This folder is a subdirectory of the request folder.

*/
std::string ConnectionInfo::getSendFolder()
{
    return sendFolder;
}

/*
 1.19 getSendPath

 Returns the complete path where files are stored.

*/

std::string ConnectionInfo::getSendPath()
{
    return sendPath;
}

/*
 1.20 Factory function

*/
ConnectionInfo* ConnectionInfo::createConnection(const std::string& host,
                                                 const int port,
                                                 std::string& config,
                                                 const size_t timeout,
                                                 const int heartbeat)
{

    NestedList* mynl = new NestedList("temp_nested_list");
    SecondoInterfaceCS* si = new SecondoInterfaceCS(true, mynl, true);
    std::string user = "";
    std::string passwd = "";
    std::string errMsg;
    si->setMaxAttempts(4);
    si->setTimeout(1);
    if (!si->Initialize(user, passwd, host, stringutils::int2str(port), config,
                        "",errMsg, true))
    {
        delete si;
        si = 0;
        delete mynl;
        return 0;
    } else
    {
        return new ConnectionInfo(host, port, config, si, mynl, 
                                  timeout, heartbeat);
    }
}

/*
 1.21 createOrUpdateObject

 creates a new object or updates an existing one on remote server.

*/
bool ConnectionInfo::createOrUpdateObject(const std::string& name,
                                          ListExpr typelist,
                                          Word& value,
                                          bool showCommands,
                                          CommandLog& commandLog,
                                          bool forceExec,
                                          const size_t timeout)
{
    if (Relation::checkType(typelist))
    {
        return createOrUpdateRelation(name, typelist, value, 
                                      showCommands, commandLog,
                                      forceExec,
                                      timeout);
    }

    guard_type guard(simtx);
    SecErrInfo serr;
    ListExpr resList;
    std::string cmd = "delete " + name;
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    if(timeout>0){
       startTimeout(timeout,false);
    }
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    double runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);

    showCommand(si, host, port, cmd, false, showCommands);
    // ignore error (object must not exist)
    std::string filename = name + "_" + stringutils::int2str(WinUnix::getpid())
            + ".obj";
    storeObjectToFile(name, value, typelist, filename);
    cmd = "restore " + name + " from '" + filename + "'";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);
    FileSystem::DeleteFileOrFolder(filename);
    if(timeout>0){
       stopTimeout(false);
    }
    return serr.code == 0;
}

/*
 1.22 createOrUpdateRelation

 Accelerated version for relations.

*/
bool ConnectionInfo::createOrUpdateRelation(const std::string& name,
                                            ListExpr typeList,
                                            Word& value,
                                            bool showCommands,
                                            CommandLog& commandLog,
                                            bool forceExec,
                                            const size_t timeout)
{

    //guard_type guard(simtx);
    // write relation to a file
    std::string filename = name + "_" + stringutils::int2str(WinUnix::getpid())
            + ".bin";
    if (!saveRelationToFile(typeList, value, filename))
    {
        return false;
    }
    // restore remote relation from local file
    bool ok = createOrUpdateRelationFromBinFile(name, filename, 
                                   showCommands, commandLog,
                                   true,
                                   forceExec, timeout);
    // remove temporarly file
    FileSystem::DeleteFileOrFolder(filename);
    return ok;
}

/*
 1.23 createOrUpdateRelationFromBinFile

 Creates a new relation or updates an existing one on remote server from
 local file.

*/

bool ConnectionInfo::createOrUpdateRelationFromBinFile(const std::string& name,
                                                const std::string& filename,
                                                bool showCommands,
                                                CommandLog& commandLog,
                                                const bool allowOverwrite,
                                                bool forceExec,
                                                const size_t timeout)
{
    guard_type guard(simtx);

    if(timeout>0){
      startTimeout(timeout,false);
    }
    SecErrInfo serr;
    ListExpr resList;
    // transfer file to remote server
    int error = si->sendFile(filename, filename, true);
    if (error != 0)
    {
        if(timeout>0){
          stopTimeout(false);
        }
        std::cerr << "error during transfer file " << filename  
                  << "from master to " << (*this) << std::endl;
        return false;
    }

    // retrieve folder from which the filename can be read
    std::string sendFolder = sendPath;

    std::string rfilename = sendFolder + "/" + filename;
    // delete existing object

    std::string cmd = "delete " + name;
    if (allowOverwrite)
    {
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        if(!cmdLog || forceExec){
           si->Secondo(cmd, resList, serr);
        } else {
           cmdLog->insert(this, cmd);
           resList = mynl->TheEmptyList();
           serr.code = 0;
           serr.msg = "command not executed";
        }
        double runtime = sw.diffSecondsReal();
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          cmd, runtime, serr.code);
        showCommand(si, host, port, cmd, false, showCommands);
    }

    cmd = "let " + name + " =  '" + rfilename + "' ffeed5 consume ";

    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    double runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);

    bool ok = serr.code == 0;
    if(!ok){
       std::cerr << "error during creation of a relation from local file " 
                 << filename << std::endl;
       std::cerr << "ErrorCode :" << serr.code << std::endl;
       std::cerr << "ErrorMessage : " << serr.msg << std::endl;
       std::cerr << "Command :" << std::endl << cmd << std::endl;
    }

    cmd = "query removeFile('" + rfilename + "')";
    sw.start();
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    if(timeout>0){
       stopTimeout(false);
    }

    return ok;
}

/*
 1.24 createOrUpdateAttributeFromBinFile

 Creates an new attribute object or updates an existing one on remote server
 from a local file.

*/
bool ConnectionInfo::createOrUpdateAttributeFromBinFile(const std::string& name,
                                                 const std::string& filename,
                                                 bool showCommands,
                                                 CommandLog& commandLog,
                                                 const bool allowOverwrite,
                                                 bool forceExec,
                                                 const size_t timeout)
{
    guard_type guard(simtx);

    if(timeout>0){
       startTimeout(timeout,false);
    }

    SecErrInfo serr;
    ListExpr resList;
    // transfer file to remote server
    int error = si->sendFile(filename, filename, true);
    if (error != 0)
    {
        if(timeout>0){
          stopTimeout(false);
        }
        return false;
    }

    // retrieve folder from which the filename can be read
    std::string sendFolder = sendPath;

    std::string rfilename = sendFolder + "/" + filename;
    // delete existing object

    std::string cmd = "delete " + name;
    if (allowOverwrite)
    {
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        if(!cmdLog || forceExec){
           si->Secondo(cmd, resList, serr);
        } else {
           cmdLog->insert(this, cmd);
           resList = mynl->TheEmptyList();
           serr.code = 0;
           serr.msg = "command not executed";
        }
        double runtime = sw.diffSecondsReal();
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          cmd, runtime, serr.code);
        showCommand(si, host, port, cmd, false, showCommands);
    }

    cmd = "let " + name + " =  loadAttr('" + rfilename + "') ";

    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    double runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);

    bool ok = serr.code == 0;

    cmd = "query removeFile('" + rfilename + "')";

    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    if(timeout>0){
      stopTimeout(false);
    }

    return ok;
}

/*
 1.26 saveRelationToFile

 Stores a local relation info a binary local file.

*/
bool ConnectionInfo::saveRelationToFile(ListExpr relType,
                                        Word& value,
                                        const std::string& filename)
{
    BinRelWriter brw(filename, relType, FILE_BUFFER_SIZE);
    if(!brw.ok()){
      return false;
    }
    Relation* rel = (Relation*) value.addr;
    GenericRelationIterator* it = rel->MakeScan();
    bool ok = true;
    Tuple* tuple = 0;
    while (ok && ((tuple = it->GetNextTuple()) != 0))
    {
        ok = brw.writeNextTuple(tuple);
        tuple->DeleteIfAllowed();
    }
    delete it;
    return ok;
}

/*
 1.27 saveAttributeToFile

 saves an attribute to a local file.

*/
bool ConnectionInfo::saveAttributeToFile(ListExpr type,
                                         Word& value,
                                         const std::string& filename)
{
    Attribute* attr = (Attribute*) value.addr;
    return FileAttribute::saveAttribute(type, attr, filename);
}

/*
 1.28 storeObjectToFile

 Stores any object to a file using its nested list represnetation.

*/

bool ConnectionInfo::storeObjectToFile(const std::string& objName,
                                       Word& value,
                                       ListExpr typeList,
                                       const std::string& fileName)
{
    SecondoCatalog* ctl = SecondoSystem::GetCatalog();
    ListExpr valueList = ctl->OutObject(typeList, value);
    ListExpr objList = nl->FiveElemList(nl->SymbolAtom("OBJECT"),
                                        nl->SymbolAtom(objName),
                                        nl->TheEmptyList(), typeList,
                                        valueList);
    return nl->WriteToFile(fileName, objList);
}

/*
 1.29 retrieve

 Retrieves a remote object.

*/

bool ConnectionInfo::retrieve(const std::string& objName,
                              ListExpr& resType,
                              Word& result,
                              bool checkType,
                              bool showCommands,
                              CommandLog& commandLog,
                              int fileIndex,
                              bool forceExec,
                              const size_t timeout)
{

    //cout << "retrieve" << endl;
    //cout << "objName = " << objName << endl;
    //cout << "resType = " << nl->ToString(resType) << endl;
    //cout << "checkType = " << checkType << endl;
    //cout << "fileIndex = " << fileIndex << endl;

    if (Relation::checkType(resType))
    {
        if (retrieveRelation(objName, resType, result, 
                             showCommands, commandLog, fileIndex,
                             false,  timeout))
        {
            return true;
        }
        std::cerr << "Could not use fast retrieval for a relation, failback" 
                  << endl;
    }
    {
      guard_type  guard(simtx);
      if(timeout>0){
         startTimeout(timeout,false);
      }
      SecErrInfo serr;
      ListExpr myResList;
      std::string cmd = "query " + objName;
      showCommand(si, host, port, cmd, true, showCommands);
      StopWatch sw;
      if(!cmdLog || forceExec){
         si->Secondo(cmd, myResList, serr);
      } else {
         cmdLog->insert(this, cmd);
         myResList = mynl->TheEmptyList();
         serr.code = 0;
         serr.msg = "command not executed";
      }
      
      if(timeout>0){
         stopTimeout(false);
      }
      double runtime = sw.diffSecondsReal();
      commandLog.insert(this, this->getHost(), 
                        this->getSecondoHome(showCommands, commandLog), cmd,
                        runtime, serr.code);
      showCommand(si, host, port, cmd, false, showCommands);
      SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
      if (serr.code != 0)
      {
          return false;
      }
      if (!mynl->HasLength(myResList, 2))
      {
          return false;
      }
      // copy result list into global list memory
      ListExpr resList;
      {
          boost::lock_guard < boost::mutex > guard(copylistmutex);
          resList = mynl->CopyList(myResList, nl);
          mynl->Destroy(myResList);
      }
      ListExpr resType2 = nl->First(resList);
      if (checkType && !nl->Equal(resType, resType2))
      {
          // other type than expected
          return false;
      }
      ListExpr value = nl->Second(resList);
  
      int errorPos = 0;
      ListExpr errorInfo = listutils::emptyErrorInfo();
      bool correct;
      {
          boost::lock_guard < boost::mutex > guard(createRelMut);
          result = ctlg->InObject(resType, value, errorPos, errorInfo, correct);
      }
      if (!correct)
      {
          result.addr = 0;
      }
      return correct;
    }
}
/*
 1.30 retrieveRelation

 Special accelerated version for relations

*/
bool ConnectionInfo::retrieveRelation(const std::string& objName,
                                      ListExpr& resType,
                                      Word& result,
                                      bool showCommands,
                                      CommandLog& commandLog,
                                      const int fileIndex,
                                      bool forceExec,
                                      const size_t timeout)
{

    //guard_type guard(simtx);
    std::string fi = stringutils::int2str(fileIndex);
    std::string fname1 = objName + "_" + fi + ".bin";
    if (!retrieveRelationFile(objName, fname1, showCommands,  
                              commandLog, forceExec,timeout))
    {
        return false;
    }
    result = createRelationFromFile(fname1, resType);
    FileSystem::DeleteFileOrFolder(fname1);
    return true;
}

/*
 1.31 retrieveRelationInFile

 Retrieves a relation which is on a remotely stored file.

*/
bool ConnectionInfo::retrieveRelationInFile(const std::string& fileName,
                                            ListExpr& resType,
                                            Word& result,
                                            bool showCommands,
                                            CommandLog& commandLog,
                                            bool forceExec,
                                            const size_t timeout)
{
    guard_type guard(simtx);
    result.addr = 0;
    // get the full path for requesting files
    std::string rfpath;
    try{
       rfpath = requestPath + "/";
    } catch(...){
       return false;
    }
    std::string base = FileSystem::Basename(fileName);
    if (stringutils::startsWith(fileName, rfpath))
    {
        if(timeout>0){
           startTimeout(timeout,false);
        }
        bool ok = si->requestFile(fileName.substr(rfpath.length()), 
                                  base + ".tmp",
                                  true);
        if(timeout>0){
           stopTimeout(false);
        }

        // we can just get the file without copying it
        if (!ok) {
            return false;
        }
        result = createRelationFromFile(base + ".tmp", resType);
        FileSystem::DeleteFileOrFolder(base + ".tmp");
        return true;
    }
    // remote file located in a not accessible folder, send command
    // to copy it into a such folder
    std::string cmd = "query copyFile('" + fileName + "', '" + rfpath + base
            + ".tmp')";
    SecErrInfo serr;
    ListExpr resList;
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    if(timeout>0){
       startTimeout(timeout,false);
    }
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    if(timeout>0){
       stopTimeout(false);
    }
    double runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);

    if (serr.code != 0)
    {
        showError(si, cmd, serr.code, serr.msg);
        return false;
    }
    if (!mynl->HasLength(resList, 2)
            || mynl->AtomType(mynl->Second(resList)) != BoolType)
    {
        std::cerr << "command " << cmd << " returns unexpected result" << endl;
        std::cerr << mynl->ToString(resList) << endl;
        return false;
    }
    if (!mynl->BoolValue(mynl->Second(resList)))
    {
        mynl->Destroy(resList);
        return false;
    }

    mynl->Destroy(resList);

    // copy the file to local file system
    if (si->requestFile(base + ".tmp", base + ".tmp", true) != 0)
    {
        std::cerr << "Requesting file " + base + ".tmp failed" << endl;
        return false;
    }
    result = createRelationFromFile(base + ".tmp", resType);

    FileSystem::DeleteFileOrFolder(base + ".tmp");
    cmd = "query removeFile('" + rfpath + base + ".tmp' )";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    if(timeout>0){
       startTimeout(timeout,true);
    }
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    if(timeout>0){
       stopTimeout(false);
    }
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);
    if (serr.code != 0)
    {
        showError(si, cmd, serr.code, serr.msg);
    }
    return true;
}

/*
 1.32 retrieveRelationFile

 retrieves a remote relation and stored it into a local file.

*/

bool ConnectionInfo::retrieveRelationFile(const std::string& objName,
                                          const std::string& fname1,
                                          bool showCommands,
                                          CommandLog& commandLog,
                                          bool forceExec, 
                                          const size_t timeout)
{
    guard_type guard(simtx);
    std::string rfname;
    try{
      rfname = requestPath + "/" + fname1;
    } catch(...){
      return false;
    }
    // save the remove relation into a binary file
    SecErrInfo serr;
    ListExpr resList;
    std::string cmd = "query createDirectory('" + requestPath 
            + "', TRUE) ";
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;

    if(timeout>0){
       startTimeout(timeout,false);
    }

    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    double runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);

    showCommand(si, host, port, cmd, false, showCommands);
    if (serr.code != 0)
    {
        std::cerr << "Creating filetransfer directory failed" << endl;
        std::cerr << "serr.code = " << serr.code << endl;
        std::cerr << "serr.Msg = " << serr.msg << endl;
        if(timeout>0){
          stopTimeout(false);
        }
        return false;
    }

    cmd = "query " + objName + " feed fconsume5['" + rfname + "'] count";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);

    if (serr.code != 0)
    {
        if(timeout>0){
          stopTimeout(false);
        }
        return false;
    }

    if (si->requestFile(fname1, fname1, true) != 0)
    {
        return false;
    }

    // delete remote file
    cmd = "query removeFile('" + rfname + "')";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    if(!cmdLog || forceExec){
       si->Secondo(cmd, resList, serr);
    } else {
       cmdLog->insert(this, cmd);
       resList = mynl->TheEmptyList();
       serr.code = 0;
       serr.msg = "command not executed";
    }
    runtime = sw.diffSecondsReal();
    commandLog.insert(this, this->getHost(), 
                      this->getSecondoHome(showCommands, commandLog), cmd,
                      runtime, serr.code);
    showCommand(si, host, port, cmd, false, showCommands);
    if(timeout>0){
      stopTimeout(false);
    }
    return true;
}

/*
 1.34 retrieveAnyFile

 This function can be used to get any file from a remote server
 even if the file is located outside the requestFile directory.

*/
bool ConnectionInfo::retrieveAnyFile(const std::string& remoteName,
                                     const std::string& localName,
                                     bool showCommands,
                                     CommandLog& commandLog,
                                     bool forceExec,
                                     const size_t timeout)
{
    guard_type guard(simtx);
    std::string rf = getRequestFolder();
    bool copyRequired = !stringutils::startsWith(remoteName, rf);
    std::string cn;
    int err;
    std::string errMsg;
    ListExpr result;
    double rt;
    if (!copyRequired)
    {
        cn = remoteName;
    } else
    {
        // create request dir
        std::string cmd = "query createDirectory('" + rf + "', TRUE)";
        simpleCommand(cmd, err, errMsg, result, false, rt, 
                      showCommands, commandLog, forceExec, timeout);
        if (err)
        {
            showError(this, cmd, err, errMsg);
            return false;
        }
        cn = "tmp_" + stringutils::int2str(serverPid()) + "_"
                + FileSystem::Basename(remoteName);
        std::string cf = getSecondoHome(showCommands, commandLog) 
                  + "/" + rf + "/" + cn;
        cmd = "query copyFile('" + remoteName + "','" + cf + "')";
        simpleCommand(cmd, err, errMsg, result, false, rt, showCommands, 
                      commandLog, forceExec, timeout);
        if (err)
        {
            std::cerr << "command " << cmd << " failed" << endl;
            return false;
        }
        if (!nl->HasLength(result, 2))
        {
            std::cerr << "unexpected result for command " << cmd << endl;
            std::cerr << "expected (type value), got " << nl->ToString(result)
                    << endl;
            return false;
        }
        if (nl->AtomType(nl->Second(result)) != BoolType)
        {
            std::cerr << "unexpected result for command " << cmd << endl;
            std::cerr << "expected (bool boolatom), got " 
                      << nl->ToString(result) << endl;
            return false;
        }
        if (!nl->BoolValue(nl->Second(result)))
        {
            std::cerr << "copying file failed" << endl;
            if(true) {
               cout <<  "remote Server : " << (*this) << endl;
               cout << "command " << endl << cmd << endl;
               cout << "returned false" << endl;
            }
            return false;
        }

    }

    int errres = requestFile(cn, localName, true, timeout);

    if (copyRequired)
    {
        // remove copy
        std::string cmd = "query removeFile('" + cn + "')";
        simpleCommand(cmd, err, errMsg, result, false, rt, showCommands, 
                      commandLog, forceExec, timeout);
        if (err)
        {
            std::cerr << "command " << cmd << " failed" << endl;
        }
    }
    return errres == 0;
}

/*
 1.35 creates a relation from its binary file representation.

*/
Word ConnectionInfo::createRelationFromFile(const std::string& fname,
                                            ListExpr& resType)
{

    // guard_type guard(simtx);

    Word result((void*) 0);
    // create result relation

    ListExpr tType = nl->Second(resType);
    tType = SecondoSystem::GetCatalog()->NumericType(resType);
    TupleType* tt = new TupleType(nl->Second(tType));
    ffeed5Info reader(fname, tt);

    if (!reader.isOK())
    {
        tt->DeleteIfAllowed();
        return result;
    }

    ListExpr typeInFile = reader.getRelType();
    if (!nl->Equal(resType, typeInFile))
    {
        std::cerr << "Type conflict between expected type and type in file" 
                  << endl
                  << "Expected : " << nl->ToString(resType) << endl
                  << "Type in  File " << nl->ToString(typeInFile) << endl;
        tt->DeleteIfAllowed();
        return result;
    }
    Relation* resultrel;
    {
        boost::lock_guard < boost::mutex > guard2(createRelMut);
        resultrel = new Relation(tType);
    }

    Tuple* tuple;
    while ((tuple = reader.next()))
    {
        boost::lock_guard < boost::mutex > guard2(createRelMut);
        resultrel->AppendTuple(tuple);
        tuple->DeleteIfAllowed();
    }
    tt->DeleteIfAllowed();
    result.addr = resultrel;
    return result;
}

std::ostream& ConnectionInfo::print(std::ostream& o) const
{
    o << host << ", " << port << ", " << config;
    return o;
}

// retrieves secondoHome from remote server
void ConnectionInfo::retrieveSecondoHome(bool showCommands,
                                         CommandLog& commandLog)
{
   guard_type guard(simtx);
   secondoHome = si?si->getHome():"";
}

void ConnectionInfo::retrieveSecondoHome()
{
   guard_type guard(simtx);
   secondoHome = si?si->getHome():"";
}

void ConnectionInfo::killConnection(){
   if(si){
     si->killConnection();
   }
}

void ConnectionInfo::timeout(){
  std::cout << "received timeout signal" << endl;
  killConnection();
}


void ConnectionInfo::startTimeout(int second, bool withMessages){
   if(withMessages){
      hbobserver->stop();
      hbobserver->start(second);
   } else {
      tonotifier->stop();
      tonotifier->start(second);
   }
}

void ConnectionInfo::stopTimeout(const bool msg){
   if(msg){
     hbobserver->stop();
   } else {
     tonotifier->stop();
   }
}



std::ostream& operator<<(std::ostream& o, const ConnectionInfo& sc)
{
    return sc.print(o);
}

void showError(const ConnectionInfo* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage){

   static boost::mutex mtx;
   boost::lock_guard<boost::mutex> lock(mtx);
   if(errorCode){
      std::cerr << "command " << command << endl
           << " failed on server " << (*ci) << endl
           << "with code " << errorCode << " : " << errorMessage << endl;
   }
}

void showError(const SecondoInterfaceCS* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage){

   static boost::mutex mtx;
   boost::lock_guard<boost::mutex> lock(mtx);
   if(errorCode){
      std::cerr << "command " << command << endl
           << " failed on server " << (ci->getHost()) << endl
           << "with code " << errorCode << " : " << errorMessage << endl;
   }
}

}/* namespace distributed2 */
