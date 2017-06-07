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
#include <iostream>

#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

CommunicationClient::CommunicationClient(
        std::string& server, int port, Socket* socket)
:Client(server, port, socket)
{
    string context("CommunicationClient");
    traceWriter= unique_ptr<TraceWriter>
    (new TraceWriter(context));

    traceWriter->writeFunction("CommunicationClient::CommunicationClient");
    traceWriter->write("Connecting to server: ", server);
    traceWriter->write("On port:", port);
}

CommunicationClient::~CommunicationClient()
{
    traceWriter->writeFunction("CommunicationClient::~CommunicationClient");
}

int CommunicationClient::start()
{
    traceWriter->writeFunction("CommunicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
                Socket::SockGlobalDomain, 3, 1);
        if (!socket) {
            traceWriter->write("socket initialization failed");
            return 8;
        }
        if (!socket->IsOk()) {
            traceWriter->write("socket not ok");
            return 9;
        }
        return 0;
}

bool CommunicationClient::triggerReplication(const string& databaseName,
                                            const string& relationName)
{
    traceWriter->writeFunction("CommunicationClient::triggerReplication");
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplication());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        traceWriter->write("Did not receive expected RelationRequest keyword");
        return false;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        traceWriter->write("Did not receive expected LocationRequest keyword");
        return false;
    }

    string originalLocation;
    getLocationParameter(originalLocation, "SecondoHost");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoPort");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoHome");
    sendBuffer.push(originalLocation);

    string transferPort;
    SecondoUtilsLocal::readFromConfigFile(transferPort,
                                       "DBService",
                                       "FileTransferPort",
                                       "");
    sendBuffer.push(transferPort);
    CommunicationUtils::sendBatch(io, sendBuffer);

    traceWriter->write("sent original location details");

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicationTriggered()))
    {
        traceWriter->write("Could not trigger replication");
        return false;
    }
    return true;
}

int CommunicationClient::triggerFileTransfer(const string& transferServerHost,
                                             const string& transferServerPort,
                                             const string& databaseName,
                                             const string& relationName)
{
    traceWriter->writeFunction("CommunicationClient::triggerFileTransfer");
    traceWriter->write("transferServerHost", transferServerHost);
    traceWriter->write("transferServerPort", transferServerPort);
    traceWriter->write("databaseName", databaseName);
    traceWriter->write("relationName", relationName);

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return 1;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerFileTransfer());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicationDetailsRequest()))
    {
        traceWriter->write(
                "Did not receive expected ReplicationDetailsRequest keyword");
        return 2;
    }
    sendBuffer.push(transferServerHost);
    sendBuffer.push(transferServerPort);
    sendBuffer.push(
            ReplicationUtils::getFileName(
                    *(const_cast<string*>(&databaseName)),
                    *(const_cast<string*>(&relationName))));
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);
    traceWriter->write("File transfer details sent to worker");
    return 0;
}

void CommunicationClient::getLocationParameter(
        string& location, const char* key)
{
    traceWriter->writeFunction("CommunicationClient::getLocationParameter");
    SecondoUtilsLocal::readFromConfigFile(location,
                                       "Environment",
                                       key,
                                       "");
}

bool CommunicationClient::getReplicaLocation(const string databaseName,
                                             const string relationName,
                                             std::string& host,
                                             std::string& transferPort)
{
    traceWriter->writeFunction("CommunicationClient::getReplicaLocation");
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::ReplicaLocationRequest());
    CommunicationUtils::sendBatch(io, sendBuffer);

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 2, receivedLines);
    host = receivedLines.front();
    receivedLines.pop();
    transferPort = receivedLines.front();
    receivedLines.pop();
    if(!host.empty() && !transferPort.empty())
    {
        return true;
    }
    return false;
}

bool CommunicationClient::reportSuccessfulReplication(
        const string& databaseName,
        const string& relationName)
{
    traceWriter->writeFunction(
            "CommunicationClient::reportSuccessfulReplication");
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::ReplicationSuccessful());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        traceWriter->write("Expected RelationRequest");
        return false;
    }

    CommunicationUtils::sendLine(io,
            RelationInfo::getIdentifier(databaseName, relationName));

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        traceWriter->write("Expected LocationRequest");
        return false;
    }

    string replicaLocation;
    getLocationParameter(replicaLocation, "SecondoHost");
    sendBuffer.push(replicaLocation);
    getLocationParameter(replicaLocation, "SecondoPort");
    sendBuffer.push(replicaLocation);
    CommunicationUtils::sendBatch(io, sendBuffer);
    return true;
}


bool CommunicationClient::requestReplicaDeletion(
        const string& databaseName,
        const string& relationName)
{
    traceWriter->writeFunction("CommunicationClient::requestReplicaDeletion");
    traceWriter->write("databaseName: ", databaseName);
    traceWriter->write("relationName: ", relationName);

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::DeleteReplicaRequest());
    sendBuffer.push(RelationInfo::getIdentifier(databaseName, relationName));
    CommunicationUtils::sendBatch(io, sendBuffer);
    return true;
}

bool CommunicationClient::triggerReplicaDeletion(
        const std::string& relID)
{
    traceWriter->writeFunction("CommunicationClient::triggerReplicaDeletion");
    traceWriter->write("relID: ", relID);

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return false;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplicaDeletion());
    sendBuffer.push(relID);
    CommunicationUtils::sendBatch(io, sendBuffer);

    return true;
}

} /* namespace DBService */
