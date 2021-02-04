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
#include <cstdlib>

#include <boost/make_shared.hpp>

#include "SecondoException.h"

#include "Algebras/DBService2/CommunicationClient.hpp"
#include "Algebras/DBService2/DBServiceClient.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/ReplicationClient.hpp"
#include "Algebras/DBService2/ReplicationServer.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"
#include "Algebras/DBService2/Replicator.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/ServerRunnable.hpp"

#include <loguru.hpp>

using namespace std;

namespace DBService {

DBServiceClient::DBServiceClient()
{
    printFunction("DBServiceClient::DBServiceClient", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    if(!SecondoUtilsLocal::lookupDBServiceLocation(
            dbServiceHost,
            dbServicePort))
    {
        dbServiceHost = "";
        dbServicePort = "";
        throw new SecondoException("Unable to connect to DBService");
    }
    print("dbServiceHost", dbServiceHost, std::cout);
    print("dbServicePort", dbServicePort, std::cout);
    LOG_F(INFO, "DBService (Host: %s, Port: %s)",
          dbServiceHost.c_str(), dbServicePort.c_str());

    startReplicationServer();
}

void DBServiceClient::startReplicationServer()
{
    printFunction("DBServiceClient::startReplicationServer", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
            "DBService",
            "FileTransferPort",
            "");
    print(fileTransferPort, std::cout);
    LOG_F(INFO, "FileTransferPort: %s", fileTransferPort.c_str());

    ServerRunnable replicationServer(atoi(fileTransferPort.c_str()));
    replicationServer.run<ReplicationServer>();
}

DBServiceClient* DBServiceClient::getInstance()
{
    printFunction("DBServiceClient::getInstance", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    if (!_instance)
    {
      try{
          _instance = new DBServiceClient();
      } catch(...){
          _instance = 0;
      }
    }
    return _instance;
}

bool DBServiceClient::triggerReplication(const std::string& databaseName,
                                            const std::string& relationName,
                                            const ListExpr relType,
                                            const bool async)
{
    LOG_SCOPE_FUNCTION(INFO);
    printFunction("DBServiceClient::triggerReplication", std::cout);
    print("databaseName", databaseName, std::cout);
    print("relationName", relationName, std::cout);
    print(relType, std::cout);

    LOG_F(INFO, "Database: %s, Relation: %s",
          databaseName.c_str(), relationName.c_str());

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    if(dbServiceMasterClient.triggerReplication(
            databaseName,
            relationName))
    {
        Replicator replicator(
                *(const_cast<string*>(&databaseName)),
                *(const_cast<string*>(&relationName)),
                relType);
        replicator.run(async);
    }
    return true;
}


bool DBServiceClient::triggerDerivation(const std::string& databaseName,
                                        const std::string& targetName,
                                        const std::string& relationName,
                                        const std::string& fundef)
{
    printFunction("DBServiceClient::triggerDerivation", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    print("databaseName", databaseName, std::cout);
    print("targetName", targetName, std::cout);
    print("relationName", relationName, std::cout);
    print("fundef", fundef, std::cout);

    LOG_F(INFO, "Database: %s, Target: %s, Relation: %s, Fundef: %s",
          databaseName.c_str(), targetName.c_str(), relationName.c_str(),
          fundef.c_str());

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    dbServiceMasterClient.triggerDerivation(
            databaseName,
            targetName,
            relationName,
            fundef);
    return true;
}




bool DBServiceClient::getReplicaLocation(
        const string& databaseName,
        const string& relationName,
        const std::vector<std::string>& otherObjects,
        string& host,
        string& transferPort,
        string& commPort)
{
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "databaseName: %s", databaseName.c_str());
    LOG_F(INFO, "relationName: %s", relationName.c_str());

    printFunction("DBServiceClient::getReplicaLocation", std::cout);
    print("databaseName", databaseName, std::cout);
    print("relationName", relationName, std::cout);

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    return dbServiceMasterClient.getReplicaLocation(databaseName,
                                                    relationName,
                                                    otherObjects,
                                                    host,
                                                    transferPort,
                                                    commPort);
}

fs::path DBServiceClient::retrieveReplicaAndGetFileName(
                                const string& databaseName,
                                const string& relationName,
                                const vector<string>& otherObjects,
                                const string& functionAsNestedListString)
{
    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "databaseName: %s", databaseName.c_str());
    LOG_F(INFO, "relationName: %s", relationName.c_str());    
    LOG_F(INFO, "function: %s", functionAsNestedListString.c_str());
    
    printFunction("DBServiceClient::retrieveReplicaAndGetFileName", std::cout);
    print("databaseName", databaseName, std::cout);
    print("relationName", relationName, std::cout);
    print("other objects", otherObjects, std::cout);
    print("function", functionAsNestedListString, std::cout);

    string host;
    string transferPort;
    string dummyCommPort;
    if(!getReplicaLocation(
            databaseName, relationName, otherObjects, host, transferPort,
            dummyCommPort))
    {
        print("No replica available", std::cout);
        LOG_F(WARNING, "No replica available.");

        return string("");
    }

    print("host", host, std::cout);
    print("transferPort", transferPort, std::cout);
    LOG_F(INFO, "Host: %s, TransferPort: %s",
          host.c_str(), transferPort.c_str());

    fs::path localPathOnClient = ReplicationUtils::expandFilenameToAbsPath(
        ReplicationUtils::getFileName(databaseName,relationName));

    string filenameOnDBSWorker = 
        ReplicationUtils::getFileNameOnDBServiceWorker(databaseName,
        relationName);

    /*
     Use case: Transfer from DBS-W to a O-Node.
     Here the DBServiceClient is used to get the fileName of the file
     to be retrieved by the requesting node. (e.g. an O-W)
    */
    ReplicationClient clientToDBServiceWorker(
            host,
            atoi(transferPort.c_str()),
            localPathOnClient, /*local*/
            filenameOnDBSWorker, /*remote*/
            *(const_cast<string*>(&databaseName)),
            *(const_cast<string*>(&relationName)));

    // requestReplica sets the fileName passed into it.
    // requestReplica acts upon the remoteFile (2nd argument of the constructor)
    fs::path fileName;
    clientToDBServiceWorker.requestReplica(
            functionAsNestedListString,
            fileName,
            otherObjects);

    LOG_F(INFO, "retrieveReplicaAndGetFileName: %s", fileName.string().c_str());

    return fileName;
}

bool DBServiceClient::deleteReplicas(const string& databaseName,
                                     const string& relationName,
                                     const string& derivateName)
{
    printFunction("DBServiceClient::deleteReplicas", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    try{
       CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);
       return dbServiceMasterClient.requestReplicaDeletion(
            databaseName,
            relationName,
            derivateName);
    } catch(...) {
       return false;
    }
}

bool DBServiceClient::pingDBService()
{
    printFunction("DBServiceClient::pingDBService", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);
    return dbServiceMasterClient.pingDBService();
}

bool DBServiceClient::getStreamType(
        const string& databaseName,
        const string& relationName,
        string& nestedListAsString)
{
    printFunction("DBServiceClient::getRelType", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    string dbServiceWorkerHost;
    string dummyTransferPort;
    string commPort;
    vector<string> dummy; // for extracting the type, we can ignore 
                          // other objects
    if(!dbServiceMasterClient.getReplicaLocation(
            databaseName,
            relationName,
            dummy,
            dbServiceWorkerHost,
            dummyTransferPort,
            commPort))
    {
        print("The relation does not exist in DBService", std::cout);
        LOG_F(WARNING, "The relation (%s, %s) does not exist in DBService.",
              databaseName.c_str(), relationName.c_str());

        return false;
    }

    CommunicationClient dbServiceWorkerClient(dbServiceWorkerHost,
                                              atoi(commPort.c_str()),
                                              0);

    return dbServiceWorkerClient.getRelType(
            RelationInfo::getIdentifier(databaseName, relationName),
            nestedListAsString);
}


bool DBServiceClient::getDerivedType(
        const string& databaseName,
        const string& relName,
        const string& derivedName,
        string& nestedListAsString)
{
    printFunction("DBServiceClient::getDerivedType", std::cout);
    LOG_SCOPE_FUNCTION(INFO);


    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    string dbServiceWorkerHost;
    string dummyTransferPort;
    string commPort;
    vector<string> derived; 
    derived.push_back(derivedName);
    if(!dbServiceMasterClient.getReplicaLocation(
            databaseName,
            relName,
            derived,
            dbServiceWorkerHost,
            dummyTransferPort,
            commPort))
    {
        print("Relation and/or Derived Object does not exist in DBService",
               std::cout);

        LOG_F(WARNING, "Relation and/or Derived Object (%s, %s) does "
                       "not exist in DBService",
                       databaseName.c_str(), relName.c_str());

        return false;
    }

    CommunicationClient dbServiceWorkerClient(dbServiceWorkerHost,
                                              atoi(commPort.c_str()),
                                              0);

    return dbServiceWorkerClient.getDerivedType(
            RelationInfo::getIdentifier(databaseName, relName),
            derivedName,
            nestedListAsString);
}



bool DBServiceClient::relationExists(
        const std::string& databaseName,
        const std::string& relationName)
{
    printFunction("DBServiceClient::relationExists", std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    string dbServiceWorkerHost;
    string dummyTransferPort;
    string commPort;
    vector<string> dummy;
    if(!dbServiceMasterClient.getReplicaLocation(
            databaseName,
            relationName,
            dummy,
            dbServiceWorkerHost,
            dummyTransferPort,
            commPort))
    {
        print("The Relation does not exist in DBService", std::cout);
        LOG_F(WARNING, "The Relation (%s, %s) does not exist in DBService",
              databaseName.c_str(), relationName.c_str());

        return false;
    }
    return true;
}

bool DBServiceClient::allExists(
         const std::string& databaseName,
         const std::string& relationName,
         const std::vector<std::string>& derivates){

    printFunction(__PRETTY_FUNCTION__, std::cout);
    LOG_SCOPE_FUNCTION(INFO);

    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);

    string dbServiceWorkerHost;
    string dummyTransferPort;
    string commPort;
    if(!dbServiceMasterClient.getReplicaLocation(
            databaseName,
            relationName,
            derivates,
            dbServiceWorkerHost,
            dummyTransferPort,
            commPort))
    {
        print("Relation or a derivate does not exist in "
              "DBService at the same location", std::cout);

        LOG_F(WARNING, "Relation or a derivate (%s, %s) does not exist in "
                       "DBService at the same location",
                       databaseName.c_str(), relationName.c_str());
        return false;
    }
    return true;
}

bool DBServiceClient::addNode(
    const std::string& nodeHost,
    const int& nodePort,
    const std::string& pathToNodeConfig) {

    LOG_SCOPE_FUNCTION(INFO);
    LOG_F(INFO, "nodeHost: %s", nodeHost.c_str());
    LOG_F(INFO, "nodePort: %d", nodePort);
    LOG_F(INFO, "pathToNodeConfig: %s", pathToNodeConfig.c_str());

    // Connect to the DBService Master to submit the worker nodes'
    // details.
    CommunicationClient dbServiceMasterClient(dbServiceHost,
        atoi(dbServicePort.c_str()),
        0);    
    
    bool success = dbServiceMasterClient.addNode(
        nodeHost, nodePort, pathToNodeConfig);

    if (!success) {
        LOG_F(ERROR, "%s", "Couldn't addNode.");
        return false;
    }

    return true;
}



DBServiceClient* DBServiceClient::_instance = nullptr;

} /* namespace DBService */
