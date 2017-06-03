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
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClientRunnable.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/ReplicationClientRunnable.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"

using namespace distributed2;
using namespace std;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        MultiClientServer(port)
{
    string context("CommunicationServer");
    traceWriter= auto_ptr<TraceWriter>
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

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 2, receivedLines);

    string databaseName = receivedLines.front();
    receivedLines.pop();
    string relationName = receivedLines.front();
    receivedLines.pop();

    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    CommunicationUtils::receiveLines(io, 4, receivedLines);
    string host = receivedLines.front();
    receivedLines.pop();
    string port = receivedLines.front();
    receivedLines.pop();
    string disk = receivedLines.front();
    receivedLines.pop();
    string transferPort = receivedLines.front();
    receivedLines.pop();

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
        CommunicationClientRunnable clientToDBServiceWorker(
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

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 5, receivedLines);
    string host = receivedLines.front();
    receivedLines.pop();
    string port = receivedLines.front();
    receivedLines.pop();
    string fileName = receivedLines.front();
    receivedLines.pop();
    string databaseName = receivedLines.front();
    receivedLines.pop();
    string relationName = receivedLines.front();
    receivedLines.pop();
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

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 2, receivedLines);

    string databaseName = receivedLines.front();
    receivedLines.pop();
    string relationName = receivedLines.front();
    receivedLines.pop();

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


} /* namespace DBService */
