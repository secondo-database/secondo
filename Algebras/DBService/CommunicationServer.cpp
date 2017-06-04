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


//[$][\$]
//[_][\_]

*/

#include "FileSystem.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/ReplicationClientRunnable.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/TriggerFileTransferRunnable.hpp"
#include "Algebras/DBService/TriggerReplicaDeletionRunnable.hpp"

using namespace distributed2;
using namespace std;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        MultiClientServer(port)
{
    string context("CommunicationServer");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context));

    traceWriter->writeFunction("CommunicationServer::CommunicationServer");
    traceWriter->write("Initializing CommunicationServer");
    traceWriter->write("port", port);

    lookupMinimumReplicaCount();
}

CommunicationServer::~CommunicationServer()
{
    traceWriter->writeFunction("CommunicationServer::~CommunicationServer");
}

void CommunicationServer::lookupMinimumReplicaCount()
{
    string replicaNumber;
    SecondoUtilsLocal::readFromConfigFile(replicaNumber,
                                           "DBService",
                                           "ReplicaNumber",
                                           "");
    minimumReplicaCount = atoi(replicaNumber.c_str());
}

int CommunicationServer::communicate(iostream& io)
{
    traceWriter->writeFunction("CommunicationServer::communicate");
    try
    {
        traceWriter->write("Communicating...");
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::CommunicationServer());

        if (!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::CommunicationClient()))
        {
            traceWriter->write(
            "Protocol error: Not connected to CommunicationClient");
            return 1;
        }

        string request;
        CommunicationUtils::receiveLine(io, request);

        traceWriter->write("request", request);

        if(request ==
                CommunicationProtocol::TriggerReplication())
        {
            handleTriggerReplicationRequest(io);
        }else if(request ==
                CommunicationProtocol::TriggerFileTransfer())
        {
            handleTriggerFileTransferRequest(io);
        }else if(request ==
                CommunicationProtocol::ReplicaLocationRequest())
        {
            handleProvideReplicaLocationRequest(io);
        }else if(request ==
                CommunicationProtocol::ReplicationSuccessful())
        {
            reportSuccessfulReplication(io);
        }else if(request ==
                CommunicationProtocol::DeleteReplicaRequest())
        {
            handleRequestReplicaDeletion(io);
        }else if(request ==
                CommunicationProtocol::TriggerReplicaDeletion())
        {
            handleTriggerReplicaDeletion(io);
        }else
        {
            traceWriter->write("Protocol error: invalid request: ", request);
            return 2;
        }
    } catch (...)
    {
        traceWriter->write("CommunicationServer: communication error");
        return 5;
    }
    return 0;
}

bool CommunicationServer::handleTriggerReplicationRequest(
        std::iostream& io)
{
    traceWriter->writeFunction(
            "CommunicationServer::handleProvideReplicaRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    CommunicationUtils::receiveLines(io, 4, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    string disk = receiveBuffer.front();
    receiveBuffer.pop();
    string transferPort = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write("host", host);
    traceWriter->write("port", port);
    traceWriter->write("disk", disk);
    traceWriter->write("transferPort", transferPort);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    dbService->determineReplicaLocations(databaseName,
                                 relationName,
                                 host,
                                 port,
                                 disk);
    vector<pair<ConnectionID, bool> > locations;
    dbService->getReplicaLocations(RelationInfo::getIdentifier(
            databaseName, relationName),
            locations);

    if(locations.size() < (size_t)minimumReplicaCount)
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationCanceled());
        dbService->deleteReplicaLocations(databaseName, relationName);
    }else
    {
        dbService->persistReplicaLocations(databaseName, relationName);
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicationTriggered());
    }

    for(vector<pair<ConnectionID, bool> >::const_iterator it
            = locations.begin(); it != locations.end(); it++)
    {
        LocationInfo locationInfo = dbService->getLocation((*it).first);
        TriggerFileTransferRunnable clientToDBServiceWorker(
                host, /*original location*/
                atoi(transferPort.c_str()), /*original location*/
                locationInfo.getHost(), /*DBService*/
                atoi(locationInfo.getCommPort().c_str()), /*DBService*/
                databaseName,
                relationName);

        clientToDBServiceWorker.run();
    }
    return true;
}

bool CommunicationServer::handleTriggerFileTransferRequest(std::iostream& io)
{
    traceWriter->writeFunction(
            "CommunicationServer::handleTriggerFileTransferRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationDetailsRequest());
    traceWriter->write("sent ReplicationDetailsRequest");

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 5, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    string fileName = receiveBuffer.front();
    receiveBuffer.pop();
    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();
    traceWriter->write("received replication details");
    traceWriter->write("host", host);
    traceWriter->write("port", port);
    // TODO remove filename, is determined automatically
    traceWriter->write("fileName", fileName);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    ReplicationClientRunnable replicationClient(
            host,
            atoi(port.c_str()),
            databaseName,
            relationName);
    replicationClient.run();
    return true;
}

bool CommunicationServer::handleProvideReplicaLocationRequest(
        std::iostream& io)
{
    traceWriter->writeFunction(
            "CommunicationServer::handleProvideReplicaLocationRequest");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    ConnectionID randomReplicaLocation =
            dbService->getRelationInfo(
                RelationInfo::getIdentifier(databaseName, relationName)).
                        getRandomReplicaLocation();
    // TODO check whether replication to this node was successful
    //      once status table exists
    LocationInfo location = dbService->getLocation(randomReplicaLocation);

    queue<string> sendBuffer;
    sendBuffer.push(location.getHost());
    sendBuffer.push(location.getTransferPort());
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

bool CommunicationServer::reportSuccessfulReplication(iostream& io)
{
    traceWriter->writeFunction(
            "CommunicationServer::reportSuccessfulReplication");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    string relID;
    CommunicationUtils::receiveLine(io, relID);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();
    DBServiceManager::getInstance()->maintainSuccessfulReplication(
            relID, host, port);
    return true;
}


bool CommunicationServer::handleRequestReplicaDeletion(iostream& io)
{
    traceWriter->writeFunction(
                "CommunicationServer::handleRequestReplicaDeletion");

    string relID;
    CommunicationUtils::receiveLine(io, relID);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    RelationInfo& relationInfo = dbService->getRelationInfo(relID);
    for(map<ConnectionID, bool>::const_iterator it = relationInfo.nodesBegin();
            it != relationInfo.nodesEnd(); it++)
    {
        if(it->second)
        {
            LocationInfo& locationInfo = dbService->getLocation(it->first);
            TriggerReplicaDeletionRunnable replicaEraser(
                    locationInfo.getHost(),
                    atoi(locationInfo.getCommPort().c_str()),
                    relID);
            replicaEraser.run();
        }
        dbService->deleteReplicaMetadata(relID);
    }

    return true;
}

bool CommunicationServer::handleTriggerReplicaDeletion(std::iostream& io)
{
    traceWriter->writeFunction(
                "CommunicationServer::handleTriggerReplicaDeletion");

    string relID;
    CommunicationUtils::receiveLine(io, relID);

    string databaseName;
    string relationName;
    RelationInfo::parseIdentifier(relID, databaseName, relationName);

    FileSystem::DeleteFileOrFolder(
            ReplicationUtils::getFileNameOnDBServiceWorker(
                    databaseName,
                    relationName));

    //TODO check return code etc
    //TODO tracing
    return true;
}

} /* namespace DBService */
