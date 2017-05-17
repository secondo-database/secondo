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

#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/ReplicationClientRunnable.hpp"

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
}

CommunicationServer::~CommunicationServer()
{
    traceWriter->writeFunction("CommunicationServer::~CommunicationServer");
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

        queue<string> receivedLines;
        CommunicationUtils::receiveLines(io, 1, receivedLines);
        string request = receivedLines.front();
        receivedLines.pop();

        traceWriter->write("request");
        traceWriter->write(request);

        if(request ==
                CommunicationProtocol::TriggerReplication())
        {
            handleTriggerReplicationRequest(io);
        }else if(request ==
                CommunicationProtocol::TriggerFileTransfer())
        {
            handleTriggerFileTransferRequest(io);
//        }else if(request ==
//                CommunicationProtocol::UseReplica())
//        {
            //TODO contact DBServiceManager and find out where replica is
            // stored
            //DBServiceManager::getInstance();
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

    CommunicationUtils::receiveLines(io, 3, receivedLines);
    string host = receivedLines.front();
    receivedLines.pop();
    string port = receivedLines.front();
    receivedLines.pop();
    string disk = receivedLines.front();
    receivedLines.pop();

    traceWriter->write("host", host);
    traceWriter->write("port", port);
    traceWriter->write("disk", disk);

    DBServiceManager* dbService = DBServiceManager::getInstance();
    dbService->storeRelationInfo(databaseName,
                                 relationName,
                                 host,
                                 port,
                                 disk);
    vector<ConnectionID> connections;
    dbService->getReplicaLocations(RelationInfo::getIdentifier(
            databaseName, relationName),
            connections);

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::ReplicaLocation());
    sendBuffer.push(stringutils::int2str(connections.size()));

    // TODO react on cancel as soon as it is sent by client

    // TODO trigger replication on worker node

//    traceWriter->write("number of locations: ", connections.size());
//
//    traceWriter->write("sending locations");
//    for(vector<ConnectionID>::const_iterator it = connections.begin();
//            it != connections.end(); it++)
//    {
//        LocationInfo& location = dbService->getLocation(*it);
//        traceWriter->write(location);
//        sendBuffer.push(location.getHost());
//        sendBuffer.push(location.getPort());
//        sendBuffer.push(location.getConfig());
//        sendBuffer.push(location.getDisk());
//        sendBuffer.push(location.getCommPort());
//        sendBuffer.push(location.getTransferPort());
//    }
//    CommunicationUtils::sendBatch(io, sendBuffer);
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
    traceWriter->write("fileName", fileName);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    ReplicationClientRunnable replicationClient(
            host,
            atoi(port.c_str()),
            fileName,
            databaseName,
            relationName);
    replicationClient.run();
    return true;
}

bool CommunicationServer::handleUseReplicaRequest(
        std::iostream& io)
{
    traceWriter->writeFunction("CommunicationServer::handleUseReplicaRequest");
    return true;
}


} /* namespace DBService */
