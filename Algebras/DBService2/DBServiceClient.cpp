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

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/DBServiceClient.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;

namespace DBService {

DBServiceClient::DBServiceClient()
{
    printFunction("DBServiceClient::DBServiceClient", std::cout);

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
    startReplicationServer();
}

void DBServiceClient::startReplicationServer()
{
    printFunction("DBServiceClient::startReplicationServer", std::cout);
    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
            "DBService",
            "FileTransferPort",
            "");
    print(fileTransferPort, std::cout);
    ServerRunnable replicationServer(atoi(fileTransferPort.c_str()));
    replicationServer.run<ReplicationServer>();
}

DBServiceClient* DBServiceClient::getInstance()
{
    printFunction("DBServiceClient::getInstance", std::cout);
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
    printFunction("DBServiceClient::triggerReplication", std::cout);
    print("databaseName", relationName, std::cout);
    print("relationName", relationName, std::cout);
    print(relType, std::cout);

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
    print("databaseName", databaseName, std::cout);
    print("targetName", targetName, std::cout);
    print("relationName", relationName, std::cout);
    print("fundef", fundef, std::cout);

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

string DBServiceClient::retrieveReplicaAndGetFileName(
                                const string& databaseName,
                                const string& relationName,
                                const vector<string>& otherObjects,
                                const string& functionAsNestedListString)
{
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
        return string("");
    }

    print("host", host, std::cout);
    print("transferPort", transferPort, std::cout);

    ReplicationClient clientToDBServiceWorker(
            host,
            atoi(transferPort.c_str()),
            ReplicationUtils::getFileName(
                    databaseName, relationName), /*local*/
            ReplicationUtils::getFileNameOnDBServiceWorker(
                    databaseName, relationName), /*remote*/
            *(const_cast<string*>(&databaseName)),
            *(const_cast<string*>(&relationName)));

    string fileName;
    clientToDBServiceWorker.requestReplica(
            functionAsNestedListString,
            fileName,
            otherObjects);
    return fileName;
}

bool DBServiceClient::deleteReplicas(const string& databaseName,
                                     const string& relationName,
                                     const string& derivateName)
{
    printFunction("DBServiceClient::deleteReplicas", std::cout);
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
        print("Relation does not exist in DBService", std::cout);
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
        print("Relation does not exist in DBService", std::cout);
        return false;
    }
    return true;
}

bool DBServiceClient::allExists(
         const std::string& databaseName,
         const std::string& relationName,
         const std::vector<std::string>& derivates){

    printFunction(__PRETTY_FUNCTION__, std::cout);
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
        return false;
    }
    return true;
}



DBServiceClient* DBServiceClient::_instance = nullptr;

} /* namespace DBService */
