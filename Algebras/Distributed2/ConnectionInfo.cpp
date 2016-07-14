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
#include "RelationAlgebra.h"

using namespace std;

extern boost::mutex nlparsemtx;
boost::mutex createRelMut;
boost::mutex copylistmutex;

namespace distributed2
{

/*
 1.1 Constructor

 Creates a new connection instance.

*/
ConnectionInfo::ConnectionInfo(const string& _host,
                               const int _port,
                               const string& _config,
                               SecondoInterfaceCS* _si,
                               NestedList* _mynl) :
        host(_host), port(_port), config(_config), si(_si)
{
    mynl = _mynl;
    serverPID = 0;
    secondoHome = "";
    requestFolder = "";
    sendFolder = "";
    sendPath = "";
}

/*
 1.2 Destructor

 Terminates the connection and destroys the object.

*/
ConnectionInfo::~ConnectionInfo()
{
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        si->Terminate();
    }
    delete si;
    si = 0;
    delete mynl;
}

/*
 1.3 ~getHost~

 returns the remote host.

*/
string ConnectionInfo::getHost() const
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
string ConnectionInfo::getConfig() const
{
    return config;
}

/*
 1.6 check

 Checks whether the remote server is working by sending a simple command.

*/
bool ConnectionInfo::check(bool showCommands, bool logOn, 
                           CommandLog& commandLog)
{
    ListExpr res;
    string cmd = "list databases";
    SecErrInfo err;
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        si->Secondo(cmd, res, err);
        double rt = sw.diffSecondsReal();
        if (logOn)
        {
            commandLog.insert(this, this->getHost(), 
                              this->getSecondoHome(showCommands, commandLog),
                              cmd, rt, err.code);
        }
        showCommand(si, host, port, cmd, false, showCommands);
    }
    return err.code == 0;
}

/*
 1.7 setId

 Sets the id.

*/
void ConnectionInfo::setId(const int i)
{
    if (si)
    {
        si->setId(i);
        ;
    }
}

/*
 1.8 simpleCommand

 Performs a command on the remote server. The result is stored as a string.


*/
void ConnectionInfo::simpleCommand(string command1,
                                   int& err,
                                   string& result,
                                   bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   bool logOn,
                                   CommandLog& commandLog)
{
    string command;
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
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        StopWatch sw;
        showCommand(si, host, port, command, true, showCommands);
        si->Secondo(command, resList, serr);
        showCommand(si, host, port, command, false, showCommands);
        runtime = sw.diffSecondsReal();
        err = serr.code;
        if (logOn)
        {
            commandLog.insert(this, this->getHost(), 
                              this->getSecondoHome(showCommands, commandLog),
                              command, runtime, err);
        }
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
string ConnectionInfo::getSecondoHome(bool showCommands,
                                      CommandLog& commandLog)
{
    if (secondoHome.length() == 0)
    {
        retrieveSecondoHome(showCommands,
                            commandLog);
    }
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
                             bool logOn,
                             CommandLog& commandLog)
{
    // first step : remove database objects
    string command = "query getcatalog() "
            "filter[.ObjectName startsWith \"TMP_\"] "
            "extend[OK : deleteObject(.ObjectName)] "
            " count";
    int err;
    string res;
    double rt;
    simpleCommand(command, err, res, false, rt, showCommands, 
                  logOn, commandLog);
    bool result = err == 0;
    string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
    string path = getSecondoHome(showCommands, commandLog) 
                + "/dfarrays/" + dbname + "/";
    command = "query getDirectory('" + path + "') "
            "filter[basename(.) startsWith \"TMP_\"] "
            "namedtransformstream[F] "
            "extend[ OK : removeDirectory(.F, TRUE)] count";
    simpleCommand(command, err, res, false, rt, showCommands, 
                  logOn, commandLog);
    result = result && (err == 0);
    return result;
}

/*
 1.11 switchDatabase

 Switches the remote server to another database.

*/
bool ConnectionInfo::switchDatabase(const string& dbname,
                                    bool createifnotexists,
                                    bool showCommands)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    ;
    // close database ignore errors
    SecErrInfo serr;
    ListExpr resList;
    string cmd = "close database";
    showCommand(si, host, port, cmd, true, showCommands);
    si->Secondo(cmd, resList, serr);
    showCommand(si, host, port, cmd, false, showCommands);
    // create database ignore errors
    if (createifnotexists)
    {
        cmd = "create database " + dbname;
        showCommand(si, host, port, cmd, true, showCommands);
        si->Secondo(cmd, resList, serr);
        showCommand(si, host, port, cmd, false, showCommands);
    }
    // open database
    cmd = "open database " + dbname;
    showCommand(si, host, port, cmd, true, showCommands);
    si->Secondo(cmd, resList, serr);
    showCommand(si, host, port, cmd, false, showCommands);
    bool res = serr.code == 0;
    return res;
}

/*
 1.12 simpleCommand

 This variant of ~simpleCommand~ returns the result as a list.


*/
void ConnectionInfo::simpleCommand(const string& command1,
                                   int& error,
                                   string& errMsg,
                                   string& resList,
                                   const bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   bool logOn,
                                   CommandLog& commandLog)
{

    string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        ;
        SecErrInfo serr;
        ListExpr myResList = mynl->TheEmptyList();
        StopWatch sw;
        showCommand(si, host, port, command, true, showCommands);
        si->Secondo(command, myResList, serr);
        showCommand(si, host, port, command, false, showCommands);
        runtime = sw.diffSecondsReal();
        if (logOn)
        {
            commandLog.insert(this, this->getHost(), 
                              this->getSecondoHome(showCommands, commandLog),
                              command, runtime, serr.code);
        }
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
void ConnectionInfo::simpleCommandFromList(const string& command1,
                                           int& error,
                                           string& errMsg,
                                           string& resList,
                                           const bool rewrite,
                                           double& runtime,
                                           bool showCommands,
                                           bool logOn,
                                           CommandLog& commandLog)
{

    string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
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
    si->Secondo(cmd, myResList, serr);
    showCommand(si, host, port, command, false, showCommands);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          command, runtime, serr.code);
    }
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

}

/*
 1.13 simpleCommand

 This variant provides the result as a list.

*/

void ConnectionInfo::simpleCommand(const string& command1,
                                   int& error,
                                   string& errMsg,
                                   ListExpr& resList,
                                   const bool rewrite,
                                   double& runtime,
                                   bool showCommands,
                                   bool logOn,
                                   CommandLog& commandLog)
{

    string command;
    if (rewrite)
    {
        rewriteQuery(command1, command);
    } else
    {
        command = command1;
    }
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    ;
    SecErrInfo serr;
    ListExpr myResList = mynl->TheEmptyList();
    StopWatch sw;
    showCommand(si, host, port, command, true, showCommands);
    si->Secondo(command, myResList, serr);
    showCommand(si, host, port, command, false, showCommands);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog),
                          command, runtime, serr.code);
    }
    error = serr.code;
    errMsg = serr.msg;
    // copy resultlist from local nested list to global nested list
    {
        boost::lock_guard < boost::mutex > guard(copylistmutex);
        assert(mynl != nl);
        resList = mynl->CopyList(myResList, nl);
        mynl->Destroy(myResList);
    }
}

/*
 1.14 serverPid

 returns the process id of the remote server

*/
int ConnectionInfo::serverPid()
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    if (serverPID == 0)
    {
        serverPID = si->getPid();
    }
    return serverPID;
}

/*
 1.15 sendFile

 transfers a locally stored file to the remote server.
 It returns an error code.

*/
int ConnectionInfo::sendFile(const string& local,
                             const string& remote,
                             const bool allowOverwrite)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    int res = si->sendFile(local, remote, allowOverwrite);
    return res;
}

/*
 1.16 requestFile

 Transfers a remotely stored file to the local file system.

*/
int ConnectionInfo::requestFile(const string& remote,
                                const string& local,
                                const bool allowOverwrite)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    int res = si->requestFile(remote, local, allowOverwrite);
    return res;
}

/*
 1.17 getRequestFolder

 Returns the path on remote machine for requesting files.

*/
string ConnectionInfo::getRequestFolder()
{
    if (requestFolder.length() == 0)
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        requestFolder = si->getRequestFileFolder();
    }
    return requestFolder;
}

/*
 1.18 getSendFolder

 returns the folder the remote machine where files are stored.
 This folder is a subdirectory of the request folder.

*/
string ConnectionInfo::getSendFolder()
{
    if (sendFolder.length() == 0)
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        sendFolder = si->getSendFileFolder();
    }
    return sendFolder;
}

/*
 1.19 getSendPath

 Returns the complete path where files are stored.

*/

string ConnectionInfo::getSendPath()
{
    if (sendPath.length() == 0)
    {
        boost::lock_guard < boost::recursive_mutex > guard(simtx);
        sendPath = si->getSendFilePath();
    }
    return sendPath;
}

/*
 1.20 Factory function

*/
ConnectionInfo* ConnectionInfo::createConnection(const string& host,
                                                 const int port,
                                                 string& config)
{

    NestedList* mynl = new NestedList();
    SecondoInterfaceCS* si = new SecondoInterfaceCS(true, mynl, true);
    string user = "";
    string passwd = "";
    string errMsg;
    si->setMaxAttempts(4);
    si->setTimeout(1);
    if (!si->Initialize(user, passwd, host, stringutils::int2str(port), config,
                        errMsg, true))
    {
        delete si;
        si = 0;
        delete mynl;
        return 0;
    } else
    {
        return new ConnectionInfo(host, port, config, si, mynl);
    }
}

/*
 1.21 createOrUpdateObject

 creates a new object or updates an existing one on remote server.

*/
bool ConnectionInfo::createOrUpdateObject(const string& name,
                                          ListExpr typelist,
                                          Word& value,
                                          bool showCommands,
                                          bool logOn,
                                          CommandLog& commandLog)
{
    if (Relation::checkType(typelist))
    {
        return createOrUpdateRelation(name, typelist, value, 
                                      showCommands, logOn, commandLog);
    }

    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    SecErrInfo serr;
    ListExpr resList;
    string cmd = "delete " + name;
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, resList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }

    showCommand(si, host, port, cmd, false, showCommands);
    // ignore error (object must not exist)
    string filename = name + "_" + stringutils::int2str(WinUnix::getpid())
            + ".obj";
    storeObjectToFile(name, value, typelist, filename);
    cmd = "restore " + name + " from '" + filename + "'";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    si->Secondo(cmd, resList, serr);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);
    FileSystem::DeleteFileOrFolder(filename);
    return serr.code == 0;
}

/*
 1.22 createOrUpdateRelation

 Accelerated version for relations.

*/
bool ConnectionInfo::createOrUpdateRelation(const string& name,
                                            ListExpr typeList,
                                            Word& value,
                                            bool showCommands,
                                            bool logOn,
                                            CommandLog& commandLog)
{

    // write relation to a file
    string filename = name + "_" + stringutils::int2str(WinUnix::getpid())
            + ".bin";
    if (!saveRelationToFile(typeList, value, filename))
    {
        return false;
    }
    // restore remote relation from local file
    bool ok = createOrUpdateRelationFromBinFile(name, filename, 
                                   showCommands, logOn, commandLog);
    // remove temporarly file
    FileSystem::DeleteFileOrFolder(filename);
    return ok;
}

/*
 1.23 createOrUpdateRelationFromBinFile

 Creates a new relation or updates an existing one on remote server from
 local file.

*/

bool ConnectionInfo::createOrUpdateRelationFromBinFile(const string& name,
                                                     const string& filename,
                                                     bool showCommands,
                                                     bool logOn,
                                                     CommandLog& commandLog,
                                                     const bool allowOverwrite)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);

    SecErrInfo serr;
    ListExpr resList;
    // transfer file to remote server
    int error = si->sendFile(filename, filename, true);
    if (error != 0)
    {
        return false;
    }

    // retrieve folder from which the filename can be read
    string sendFolder = si->getSendFilePath();

    string rfilename = sendFolder + "/" + filename;
    // delete existing object

    string cmd = "delete " + name;
    if (allowOverwrite)
    {
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        si->Secondo(cmd, resList, serr);
        double runtime = sw.diffSecondsReal();
        if (logOn)
        {
            commandLog.insert(this, this->getHost(), 
                              this->getSecondoHome(showCommands, commandLog),
                              cmd, runtime, serr.code);
        }
        showCommand(si, host, port, cmd, false, showCommands);
    }

    cmd = "let " + name + " =  '" + rfilename + "' ffeed5 consume ";

    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, resList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);

    bool ok = serr.code == 0;

    cmd = "query removeFile('" + rfilename + "')";
    sw.start();
    si->Secondo(cmd, resList, serr);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }

    return ok;
}

/*
 1.24 createOrUpdateAttributeFromBinFile

 Creates an new attribute object or updates an existing one on remote server
 from a local file.

*/
bool ConnectionInfo::createOrUpdateAttributeFromBinFile(const string& name,
                                                     const string& filename,
                                                     bool showCommands,
                                                     bool logOn,
                                                     CommandLog& commandLog,
                                                     const bool allowOverwrite)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);

    SecErrInfo serr;
    ListExpr resList;
    // transfer file to remote server
    int error = si->sendFile(filename, filename, true);
    if (error != 0)
    {
        return false;
    }

    // retrieve folder from which the filename can be read
    string sendFolder = si->getSendFilePath();

    string rfilename = sendFolder + "/" + filename;
    // delete existing object

    string cmd = "delete " + name;
    if (allowOverwrite)
    {
        showCommand(si, host, port, cmd, true, showCommands);
        StopWatch sw;
        si->Secondo(cmd, resList, serr);
        double runtime = sw.diffSecondsReal();
        if (logOn)
        {
            commandLog.insert(this, this->getHost(), 
                              this->getSecondoHome(showCommands, commandLog),
                              cmd, runtime, serr.code);
        }
        showCommand(si, host, port, cmd, false, showCommands);
    }

    cmd = "let " + name + " =  loadAttr('" + rfilename + "') ";

    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, resList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);

    bool ok = serr.code == 0;

    cmd = "query removeFile('" + rfilename + "')";

    si->Secondo(cmd, resList, serr);

    return ok;
}

/*
 1.26 saveRelationToFile

 Stores a local relation info a binary local file.

*/
bool ConnectionInfo::saveRelationToFile(ListExpr relType,
                                        Word& value,
                                        const string& filename)
{
    ofstream out(filename.c_str(), ios::out | ios::binary);
    char* buffer = new char[FILE_BUFFER_SIZE];
    out.rdbuf()->pubsetbuf(buffer, FILE_BUFFER_SIZE);
    if (!out.good())
    {
        delete[] buffer;
        return false;
    }
    if (!BinRelWriter::writeHeader(out, relType))
    {
        delete[] buffer;
        return false;
    }
    Relation* rel = (Relation*) value.addr;
    GenericRelationIterator* it = rel->MakeScan();
    bool ok = true;
    Tuple* tuple = 0;
    while (ok && ((tuple = it->GetNextTuple()) != 0))
    {
        ok = BinRelWriter::writeNextTuple(out, tuple);
        tuple->DeleteIfAllowed();
    }
    delete it;
    BinRelWriter::finish(out);
    out.close();
    delete[] buffer;
    return ok;
}

/*
 1.27 saveAttributeToFile

 saves an attribute to a local file.

*/
bool ConnectionInfo::saveAttributeToFile(ListExpr type,
                                         Word& value,
                                         const string& filename)
{
    Attribute* attr = (Attribute*) value.addr;
    return FileAttribute::saveAttribute(type, attr, filename);
}

/*
 1.28 storeObjectToFile

 Stores any object to a file using its nested list represnetation.

*/

bool ConnectionInfo::storeObjectToFile(const string& objName,
                                       Word& value,
                                       ListExpr typeList,
                                       const string& fileName)
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

bool ConnectionInfo::retrieve(const string& objName,
                              ListExpr& resType,
                              Word& result,
                              bool checkType,
                              bool showCommands,
                              bool logOn,
                              CommandLog& commandLog)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    if (Relation::checkType(resType))
    {
        if (retrieveRelation(objName, resType, result, 
                             showCommands, logOn, commandLog))
        {
            return true;
        }
        cerr << "Could not use fast retrieval for a " << " relation, failback"
                << endl;
    }

    SecErrInfo serr;
    ListExpr myResList;
    string cmd = "query " + objName;
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, myResList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
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
/*
 1.30 retrieveRelation

 Special accelerated version for relations

*/
bool ConnectionInfo::retrieveRelation(const string& objName,
                                      ListExpr& resType,
                                      Word& result,
                                      bool showCommands,
                                      bool logOn,
                                      CommandLog& commandLog)
{

    string fname1 = objName + ".bin";
    if (!retrieveRelationFile(objName, fname1, showCommands, logOn, commandLog))
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
bool ConnectionInfo::retrieveRelationInFile(const string& fileName,
                                            ListExpr& resType,
                                            Word& result,
                                            bool showCommands,
                                            bool logOn,
                                            CommandLog& commandLog)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);
    result.addr = 0;
    // get the full path for requesting files
    string rfpath = si->getRequestFilePath() + "/";
    string base = FileSystem::Basename(fileName);
    if (stringutils::startsWith(fileName, rfpath))
    {
        // we can just get the file without copying it
        if (!si->requestFile(fileName.substr(rfpath.length()), base + ".tmp",
                             true))
        {
            return false;
        }
        result = createRelationFromFile(base + ".tmp", resType);
        FileSystem::DeleteFileOrFolder(base + ".tmp");
        return true;
    }
    // remote file located in a not accessible folder, send command
    // to copy it into a such folder
    string cmd = "query copyFile('" + fileName + "', '" + rfpath + base
            + ".tmp')";
    SecErrInfo serr;
    ListExpr resList;
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, resList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);

    if (serr.code != 0)
    {
        showError(si, cmd, serr.code, serr.msg);
        return false;
    }
    if (!mynl->HasLength(resList, 2)
            || mynl->AtomType(mynl->Second(resList)) != BoolType)
    {
        cerr << "command " << cmd << " returns unexpected result" << endl;
        cerr << mynl->ToString(resList) << endl;
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
        cerr << "Requesting file " + base + ".tmp failed" << endl;
        return false;
    }
    result = createRelationFromFile(base + ".tmp", resType);

    FileSystem::DeleteFileOrFolder(base + ".tmp");
    cmd = "query removeFile('" + rfpath + base + ".tmp' )";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    si->Secondo(cmd, resList, serr);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
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

bool ConnectionInfo::retrieveRelationFile(const string& objName,
                                          const string& fname1,
                                          bool showCommands,
                                          bool logOn,
                                          CommandLog& commandLog)
{
    boost::lock_guard < boost::recursive_mutex > guard(simtx);

    string rfname = si->getRequestFilePath() + "/" + fname1;
    // save the remove relation into a binary file
    SecErrInfo serr;
    ListExpr resList;
    string cmd = "query createDirectory('" + si->getRequestFilePath()
            + "', TRUE) ";
    showCommand(si, host, port, cmd, true, showCommands);
    StopWatch sw;
    si->Secondo(cmd, resList, serr);
    double runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }

    showCommand(si, host, port, cmd, false, showCommands);
    if (serr.code != 0)
    {
        cerr << "Creating filetransfer directory failed" << endl;
        return false;
    }

    cmd = "query " + objName + " feed fconsume5['" + rfname + "'] count";
    showCommand(si, host, port, cmd, true, showCommands);
    sw.start();
    si->Secondo(cmd, resList, serr);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);

    if (serr.code != 0)
    {
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
    si->Secondo(cmd, resList, serr);
    runtime = sw.diffSecondsReal();
    if (logOn)
    {
        commandLog.insert(this, this->getHost(), 
                          this->getSecondoHome(showCommands, commandLog), cmd,
                          runtime, serr.code);
    }
    showCommand(si, host, port, cmd, false, showCommands);
    return true;
}

/*
 1.34 retrieveAnyFile

 This function can be used to get any file from a remote server
 even if the file is located outside the requestFile directory.

*/
bool ConnectionInfo::retrieveAnyFile(const string& remoteName,
                                     const string& localName,
                                     bool showCommands,
                                     bool logOn,
                                     CommandLog& commandLog)
{
    string rf = getRequestFolder();
    bool copyRequired = !stringutils::startsWith(remoteName, rf);
    string cn;
    int err;
    string errMsg;
    ListExpr result;
    double rt;
    if (!copyRequired)
    {
        cn = remoteName;
    } else
    {
        // create request dir
        string cmd = "query createDirectory('" + rf + "', TRUE)";
        simpleCommand(cmd, err, errMsg, result, false, rt, 
                      showCommands, logOn, commandLog);
        if (err)
        {
            showError(this, cmd, err, errMsg);
            return false;
        }
        cn = "tmp_" + stringutils::int2str(serverPid()) + "_"
                + FileSystem::Basename(remoteName);
        string cf = getSecondoHome(showCommands, commandLog) 
                  + "/" + rf + "/" + cn;
        cmd = "query copyFile('" + remoteName + "','" + cf + "')";
        simpleCommand(cmd, err, errMsg, result, false, rt, showCommands, 
                      logOn, commandLog);
        if (err)
        {
            cerr << "command " << cmd << " failed" << endl;
            return false;
        }
        if (!nl->HasLength(result, 2))
        {
            cerr << "unexpected result for command " << cmd << endl;
            cerr << "expected (type value), got " << nl->ToString(result)
                    << endl;
            return false;
        }
        if (nl->AtomType(nl->Second(result)) != BoolType)
        {
            cerr << "unexpected result for command " << cmd << endl;
            cerr << "expected (bool boolatom), got " << nl->ToString(result)
                    << endl;
            return false;
        }
        if (!nl->BoolValue(nl->Second(result)))
        {
            cerr << "copying file failed" << endl;
            return false;
        }

    }

    int errres = requestFile(cn, localName, true);

    if (copyRequired)
    {
        // remove copy
        string cmd = "query removeFile('" + cn + "')";
        simpleCommand(cmd, err, errMsg, result, false, rt, showCommands, 
                      logOn, commandLog);
        if (err)
        {
            cerr << "command " << cmd << " failed" << endl;
        }
    }
    return errres == 0;
}

/*
 1.35 creates a relation from its binary file representation.

*/
Word ConnectionInfo::createRelationFromFile(const string& fname,
                                            ListExpr& resType)
{

    boost::lock_guard < boost::recursive_mutex > guard(simtx);

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
        cerr << "Type conflict between expected type and tyoe in file" << endl;
        cerr << "Expected : " << nl->ToString(resType) << endl;
        cerr << "Type in  File " << nl->ToString(typeInFile) << endl;
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

ostream& ConnectionInfo::print(ostream& o) const
{
    o << host << ", " << port << ", " << config;
    return o;
}

// retrieves secondoHome from remote server
void ConnectionInfo::retrieveSecondoHome(bool showCommands,
                                         CommandLog& commandLog)
{
    string cmd = "query secondoHome()";
    int err;
    string errmsg;
    ListExpr result;
    double rt;
    simpleCommand(cmd, err, errmsg, result, false, rt, 
                  false /*log*/, showCommands, commandLog);
    if (err != 0)
    {
        cerr << "command " << cmd << " failed" << endl;
        cerr << err << " : " << errmsg << endl;
        return;
    }
    if (!nl->HasLength(result, 2)
            || nl->AtomType(nl->Second(result)) != TextType)
    {
        cerr << "invalid result for secondoHome() query " << endl;
        cerr << nl->ToString(result);
        return;
    }
    secondoHome = nl->Text2String(nl->Second(result));
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
