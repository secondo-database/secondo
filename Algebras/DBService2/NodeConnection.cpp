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

using namespace std;

extern NestedList* nl;

namespace DBService {

  NodeConnection::NodeConnection(string newHost, int newPort, 
    string newConfig) {
  
    host = newHost;
    port = newPort;
    config = newConfig;

    connect();
  }

  
  void NodeConnection::connect() {
    if (host == "" || port <= 0)
      throw "Can't connect without host and port.";
    
    //TODO learn about optional timeout and hartbeat params and make use 
    //  of them if helpful.

    // Creating a connection
    shared_ptr<distributed2::ConnectionInfo> myConnection(      
      distributed2::ConnectionInfo::createConnection(host, port, config)
    );

    // Setting the connection and thus trigger the retrieval of config params.
    setConnectionInfo(myConnection);

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

    printFunction("NodeConnection::startDBServiceWorker", std::cout);    

    string queryInit("query initdbserviceworker()");

    success = SecondoUtilsRemote::executeQuery(connection.get(), queryInit);

    //TODO add more infos about the failed node
    if (!success)
      throw "Couldn't start DBService Worker.";
  }

  void NodeConnection::createAndSelectRemoteDBServiceDatabase() {
    
    // Select and create the DBSERVICE database on the remote worker node
    connection->switchDatabase(
      string("dbservice"),
      true /*createifnotexists*/,
      false /*showCommands*/,
      true /*forceExec*/);    
  }

  int NodeConnection::obtainRemoteConfigParamComPort() {
    string comPortStr;    

    getRemoteConfigParam(comPortStr, connection.get(), "DBService", 
      "CommunicationPort");
 
    return stoi(comPortStr);
  }

  int NodeConnection::obtainRemoteConfigParamTransferPort() {
    string transferPortStr;    

    getRemoteConfigParam(transferPortStr, connection.get(),
            "DBService", "FileTransferPort");
    
    return stoi(transferPortStr);
  }

  string NodeConnection::obtainRemoteConfigParamDiskPath() {
    string diskPath;

    getRemoteConfigParam(diskPath, connection.get(),
            "Environment", "SecondoHome");
    return diskPath;    
  }


  void NodeConnection::getRemoteConfigParam(string& result,
        distributed2::ConnectionInfo* connectionInfo, const char* section,
        const char* key)
  {
    printFunction("DBServiceManager::getConfigParamFromWorker", std::cout);
    
    string resultAsString;
    stringstream query;
    
    query << "query getconfigparam(\""
          << section
          << "\", \""
          << key
          << "\")";
    
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

    ListExpr resultAsNestedList;
    nl->ReadFromString(resultAsString, resultAsNestedList);
    result.assign(nl->StringValue(nl->Second(resultAsNestedList)));
    print("NodeConnection::getRemoteConfigParam", result, std::cout);
  }
}