/*
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
#include "Algebras/DBService/DBServiceConnector.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;

namespace DBService {

DBServiceConnector::DBServiceConnector()
{
    printFunction("DBServiceConnector::DBServiceConnector");

    if(!SecondoUtilsLocal::lookupDBServiceLocation(
            dbServiceHost,
            dbServicePort))
    {
        throw new SecondoException("Unable to connect to DBService");
    }
    print("dbServiceHost", dbServiceHost);
    print("dbServicePort", dbServicePort);
    startReplicationServer();
}

void DBServiceConnector::startReplicationServer()
{
    printFunction("DBServiceConnector::startReplicationServer");
    string fileTransferPort;
    SecondoUtilsLocal::readFromConfigFile(fileTransferPort,
            "DBService",
            "FileTransferPort",
            "");
    print(fileTransferPort);
    ServerRunnable replicationServer(atoi(fileTransferPort.c_str()));
    replicationServer.run<ReplicationServer>();
}

DBServiceConnector* DBServiceConnector::getInstance()
{
    printFunction("DBServiceConnector::getInstance");
    if (!_instance)
    {
        _instance = new DBServiceConnector();
    }
    return _instance;
}

bool DBServiceConnector::triggerReplication(const std::string& databaseName,
                                            const std::string& relationName,
                                            const ListExpr relType)
{
    printFunction("DBServiceConnector::triggerReplication");
    print("databaseName", relationName);
    print("relationName", relationName);
    print(relType);

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
        replicator.run();
    }
    return true;
}

bool DBServiceConnector::getReplicaLocation(const string& databaseName,
                                            const string& relationName,
                                            string& host,
                                            string& transferPort)
{
    printFunction("DBServiceConnector::getReplicaLocation");
    print("databaseName", databaseName);
    print("relationName", relationName);
    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);
    return dbServiceMasterClient.getReplicaLocation(databaseName,
                                                    relationName,
                                                    host,
                                                    transferPort);
}

string DBServiceConnector::retrieveReplicaAndGetFileName(
                                const string& databaseName,
                                const string& relationName,
                                const string& functionAsNestedListString)
{
    printFunction("DBServiceConnector::retrieveReplicaAndGetFileName");
    print("databaseName", databaseName);
    print("relationName", relationName);

    string host;
    string transferPort;
    if(!getReplicaLocation(databaseName, relationName, host, transferPort))
    {
        print("No replica available");
        print("host", host);
        print("transferPort", transferPort);
        return string("");
    }

    print("host", host);
    print("transferPort", transferPort);

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
            fileName);
    return fileName;
}

bool DBServiceConnector::deleteReplicas(const string& databaseName,
                                        const string& relationName)
{
    printFunction("DBServiceConnector::deleteReplicas");
    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);
    if(!dbServiceMasterClient.requestReplicaDeletion(
            databaseName,
            relationName))
    {
        return false;
    }
    return true;
}

bool DBServiceConnector::pingDBService()
{
    printFunction("DBServiceConnector::pingDBService");
    CommunicationClient dbServiceMasterClient(dbServiceHost,
                                              atoi(dbServicePort.c_str()),
                                              0);
    return dbServiceMasterClient.pingDBService();
}

DBServiceConnector* DBServiceConnector::_instance = nullptr;

} /* namespace DBService */
