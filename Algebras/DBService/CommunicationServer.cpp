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
    (new TraceWriter(context, port));

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
    traceWriter->writeFunction(
            "CommunicationServer::lookupMinimumReplicaCount");
    string replicaNumber;
    SecondoUtilsLocal::readFromConfigFile(replicaNumber,
                                           "DBService",
                                           "ReplicaNumber",
                                           "");
    minimumReplicaCount = atoi(replicaNumber.c_str());
}

int CommunicationServer::communicate(iostream& io)
{
    const boost::thread::id tid = boost::this_thread::get_id();
    traceWriter->writeFunction(tid, "CommunicationServer::communicate");
    try
    {
        traceWriter->write(tid, "Communicating...");
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::CommunicationServer());

        if (!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::CommunicationClient()))
        {
            traceWriter->write(tid,
            "Protocol error: Not connected to CommunicationClient");
            return 1;
        }

        string request;
        CommunicationUtils::receiveLine(io, request);

        traceWriter->write(tid, "request", request);

        if(request ==
                CommunicationProtocol::TriggerReplication())
        {
            handleTriggerReplicationRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::TriggerFileTransfer())
        {
            handleTriggerFileTransferRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::ReplicaLocationRequest())
        {
            handleProvideReplicaLocationRequest(io, tid);
        }else if(request ==
                CommunicationProtocol::ReplicationSuccessful())
        {
            reportSuccessfulReplication(io, tid);
        }else if(request ==
                CommunicationProtocol::DeleteReplicaRequest())
        {
            handleRequestReplicaDeletion(io, tid);
        }else if(request ==
                CommunicationProtocol::TriggerReplicaDeletion())
        {
            handleTriggerReplicaDeletion(io, tid);
        }else
        {
            traceWriter->write(
                    tid, "Protocol error: invalid request: ", request);
            return 1;
        }
    } catch (...)
    {
        traceWriter->write(tid, "CommunicationServer: communication error");
        return 2;
    }
    return 0;
}

bool CommunicationServer::handleTriggerReplicationRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerReplicationRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    if(dbService->replicaExists(databaseName, relationName))
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::ReplicaExists());
        return false;
    }

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

    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    traceWriter->write(tid, "disk", disk);
    traceWriter->write(tid, "transferPort", transferPort);

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

bool CommunicationServer::handleTriggerFileTransferRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleTriggerFileTransferRequest");
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationDetailsRequest());
    traceWriter->write(tid, "sent ReplicationDetailsRequest");

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
    traceWriter->write(tid, "received replication details");
    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);
    // TODO remove filename, is determined automatically
    traceWriter->write(tid, "fileName", fileName);
    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    ReplicationClientRunnable replicationClient(
            host,
            atoi(port.c_str()),
            databaseName,
            relationName);
    replicationClient.run();
    return true;
}

bool CommunicationServer::handleProvideReplicaLocationRequest(
        std::iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::handleProvideReplicaLocationRequest");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);

    string databaseName = receiveBuffer.front();
    receiveBuffer.pop();
    string relationName = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "databaseName", databaseName);
    traceWriter->write(tid, "relationName", relationName);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    ConnectionID randomReplicaLocation = 0;
    try
    {
        randomReplicaLocation =
            dbService->getRelationInfo(
                RelationInfo::getIdentifier(databaseName, relationName)).
                        getRandomReplicaLocation();
    }catch(...)
    {
        traceWriter->write(tid, "RelationInfo does not exist");
    }
    queue<string> sendBuffer;
    if(randomReplicaLocation == 0)
    {
        sendBuffer.push(CommunicationProtocol::None());
        sendBuffer.push(CommunicationProtocol::None());
        CommunicationUtils::sendBatch(io, sendBuffer);
        return false;
    }
    LocationInfo location = dbService->getLocation(randomReplicaLocation);

    sendBuffer.push(location.getHost());
    sendBuffer.push(location.getTransferPort());
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

bool CommunicationServer::reportSuccessfulReplication(
        iostream& io,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
            "CommunicationServer::reportSuccessfulReplication");

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    string relID;
    CommunicationUtils::receiveLine(io, relID);
    traceWriter->write(tid, "relID", relID);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    queue<string> receiveBuffer;
    CommunicationUtils::receiveLines(io, 2, receiveBuffer);
    string host = receiveBuffer.front();
    receiveBuffer.pop();
    string port = receiveBuffer.front();
    receiveBuffer.pop();

    traceWriter->write(tid, "host", host);
    traceWriter->write(tid, "port", port);

    DBServiceManager::getInstance()->maintainSuccessfulReplication(
            relID, host, port);
    return true;
}


bool CommunicationServer::handleRequestReplicaDeletion(
        iostream& io, const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
                "CommunicationServer::handleRequestReplicaDeletion");

    string relID;
    CommunicationUtils::receiveLine(io, relID);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    try{
        RelationInfo& relationInfo = dbService->getRelationInfo(relID);
        for(map<ConnectionID, bool>::const_iterator it =
                relationInfo.nodesBegin(); it != relationInfo.nodesEnd(); it++)
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
    }catch(...)
    {
        traceWriter->write(tid, "Relation does not exist");
        return false;
    }

    return true;
}

bool CommunicationServer::handleTriggerReplicaDeletion(
        std::iostream& io,
        const boost::thread::id tid)
{
    traceWriter->writeFunction(tid,
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
