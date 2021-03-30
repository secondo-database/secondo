/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
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
#include  "Algebras/DBService2/NodeConnection.hpp"

#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/SecondoUtilsRemote.hpp"

#include "NestedList.h"

#include <loguru.hpp>

using namespace std;

extern NestedList* nl;
extern boost::recursive_mutex nlparsemtx;

namespace DBService {

  boost::mutex NodeConnection::connectionMutex;

  NodeConnection::NodeConnection(string newHost, int newPort, 
    string newConfig) {
  
    LOG_SCOPE_FUNCTION(INFO);

    host = newHost;
    port = newPort;
    config = newConfig;

    // connect();
  }

  
  void NodeConnection::connect() {
    LOG_SCOPE_FUNCTION(INFO);

    if (host == "" || port <= 0)
      throw "Can't connect without host and port.";
    

    //TODO learn about optional timeout and hartbeat params and make use 
    //  of them if helpful.

    LOG_F(INFO, "%s", "Lock acquired. Now establishing the connection...");

    boost::unique_lock<boost::mutex> connectionLock(connectionMutex);

    // Creating a connection
    shared_ptr<distributed2::ConnectionInfo> myConnection(      
      distributed2::ConnectionInfo::createConnection(host, port, config)
    );

    // Setting the connection and thus trigger the retrieval of config params.
    setConnectionInfo(myConnection);

    LOG_F(INFO, "%s", "Releasing connection lock...");
    
    connectionLock.unlock();

    LOG_F(INFO, "%s", "Creating and selecting the remote DBService database.");
    createAndSelectRemoteDBServiceDatabase();
  }

  void NodeConnection::setConnectionInfo(
    shared_ptr<distributed2::ConnectionInfo> newConnection) {
    if (newConnection != connection)
      connection = newConnection;    
  }

  shared_ptr<distributed2::ConnectionInfo> NodeConnection::getConnectionInfo() {
    return connection;
  }

  bool NodeConnection::isConnected() {
    if (connection == nullptr)
      return false;
    else
      return true;    
  }

  void NodeConnection::startDBServiceWorker() {
    bool success; 
    
    LOG_SCOPE_FUNCTION(INFO);
    printFunction("NodeConnection::startDBServiceWorker", std::cout);    

    string queryInit("query initdbserviceworker()");

    boost::lock_guard<boost::mutex> connectionGuard(connectionMutex);

    success = SecondoUtilsRemote::executeQuery(connection.get(), queryInit);

    //TODO add more infos about the failed node
    if (!success)
      throw "Couldn't start DBService Worker.";
  }

  void NodeConnection::createAndSelectRemoteDBServiceDatabase() {

    LOG_SCOPE_FUNCTION(INFO);

    boost::lock_guard<boost::mutex> connectionGuard(connectionMutex);
    
    // Select and create the DBSERVICE database on the remote worker node
    connection->switchDatabase(
      string("dbservice"),
      true /*createifnotexists*/,
      false /*showCommands*/,
      true /*forceExec*/);    
  }

  int NodeConnection::obtainRemoteConfigParamComPort() {
    LOG_SCOPE_FUNCTION(INFO);

    string comPortStr;

    getRemoteConfigParam(comPortStr, connection.get(), "DBService", 
      "CommunicationPort");
 
    return stoi(comPortStr);
  }

  int NodeConnection::obtainRemoteConfigParamTransferPort() {
    LOG_SCOPE_FUNCTION(INFO);

    string transferPortStr;

    getRemoteConfigParam(transferPortStr, connection.get(),
            "DBService", "FileTransferPort");
    
    return stoi(transferPortStr);
  }

  string NodeConnection::obtainRemoteConfigParamDiskPath() {
    LOG_SCOPE_FUNCTION(INFO);

    string diskPath;

    getRemoteConfigParam(diskPath, connection.get(),
            "Environment", "SecondoHome");
    return diskPath;    
  }


  void NodeConnection::getRemoteConfigParam(string& result,
        distributed2::ConnectionInfo* connectionInfo, const char* section,
        const char* key)
  {

    LOG_SCOPE_FUNCTION(INFO);
    printFunction("DBServiceManager::getConfigParamFromWorker", std::cout);
    
    string resultAsString;
    stringstream query;
    
    query << "query getconfigparam(\""
          << section
          << "\", \""
          << key
          << "\")";

    boost::lock_guard<boost::mutex> connectionGuard(connectionMutex);
    
    bool resultOk = SecondoUtilsRemote::executeQuery(
            connectionInfo,
            query.str(),
            resultAsString);
    
    print("resultAsString", resultAsString, std::cout);

    if(!resultOk)
    {
      stringstream errMsg("NodeConnection::getRemoteConfigParam. ");
      
      errMsg << "Couldn't execute remote command while trying to execute ";
      errMsg << "the remote query: " << query.str();
      throw(errMsg.str());        
    }

    LOG_F(INFO, "%s", "Acquiring lock to parse config param results...");

    // Lock access to the nested list.
    // This lock causes a deadlock!!!!
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);

    ListExpr resultAsNestedList;
    nl->ReadFromString(resultAsString, resultAsNestedList);
    result.assign(nl->StringValue(nl->Second(resultAsNestedList)));
    print("NodeConnection::getRemoteConfigParam", result, std::cout);
  }
}