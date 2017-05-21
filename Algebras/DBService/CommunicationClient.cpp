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
        std::string& _server, int _port, Socket* _socket)
:Client(_server, _port, _socket)
{
    string context("CommunicationClient");
    traceWriter= auto_ptr<TraceWriter>
    (new TraceWriter(context));

    traceWriter->writeFunction("CommunicationClient::CommunicationClient");
    traceWriter->write("server", _server);
    traceWriter->write("port", _port);
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

int CommunicationClient::triggerReplication(const string& databaseName,
                                            const string& relationName,
                                            vector<LocationInfo>& locations)
{
    traceWriter->writeFunction("CommunicationClient::triggerReplication");
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        traceWriter->write("Not connected to CommunicationServer");
        return 1;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplication());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        traceWriter->write("Did not receive expected RelationRequest keyword");
        return 2;
    }
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        traceWriter->write("Did not receive expected LocationRequest keyword");
        return 3;
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
            CommunicationProtocol::ReplicaLocation()))
    {
        traceWriter->write("Did not receive expected ReplicaLocation keyword");
        return 4;
    }
    string count;
    CommunicationUtils::receiveLine(io, count);
    int locationCount = atoi(count.c_str());
    traceWriter->write("number of replica locations: ", locationCount);
    return 0;
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

int CommunicationClient::getReplicaLocation()
{
    traceWriter->writeFunction("CommunicationClient::getReplicaLocation");
    return 0;
}

} /* namespace DBService */
