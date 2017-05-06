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
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;
using namespace distributed2;

namespace DBService {

CommunicationClient::CommunicationClient(
        std::string& _server, int _port, Socket* _socket)
:Client(_server, _port, _socket)
{
    printFunction("CommunicationClient::CommunicationClient");
    print("server", _server);
    print("port", port);
}

int CommunicationClient::start()
{
    printFunction("CommunicationClient::start");
    socket = Socket::Connect(server, stringutils::int2str(port),
                Socket::SockGlobalDomain, 3, 1);
        if (!socket) {
            return 8;
        }
        if (!socket->IsOk()) {
            return 9;
        }
        return 0;
}

int CommunicationClient::getNodesForReplication(const string& relationName,
        vector<LocationInfo>& locations)
{
    printFunction("CommunicationClient::getNodesForReplication");
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        print("Not connected to CommunicationServer");
        return 1;
    }

    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::ProvideReplica());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::RelationRequest()))
    {
        print("Did not receive expected RelationRequest keyword");
        return 2;
    }
    sendBuffer.push(SecondoSystem::GetInstance()->GetDatabaseName());
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        print("Did not receive expected LocationRequest keyword");
        return 3;
    }

    string originalLocation;
    getLocationParameter(originalLocation, "SecondoHost");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoPort");
    sendBuffer.push(originalLocation);
    getLocationParameter(originalLocation, "SecondoHome");
    sendBuffer.push(originalLocation);
    CommunicationUtils::sendBatch(io, sendBuffer);
    print("sent original location details");

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicaLocation()))
    {
        print("Did not receive expected ReplicaLocation keyword");
        return 4;
    }
    string count;
    CommunicationUtils::receiveLine(io, count);
    int locationCount = atoi(count.c_str());
    print("number of replica locations: ", locationCount);

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, locationCount*5, receivedLines);
    print("Received locations for replication");
    for(int i= 0; i < locationCount; i++)
    {
        string host = receivedLines.front();
        receivedLines.pop();
        string port= receivedLines.front();
        receivedLines.pop();
        string disk= receivedLines.front();
        receivedLines.pop();
        string commPort = receivedLines.front();
        receivedLines.pop();
        string transferPort = receivedLines.front();
        receivedLines.pop();
        LocationInfo location(host, port, disk, commPort, transferPort);
        locations.push_back(location);
        print(location);
    }
    return 0;
}

int CommunicationClient::triggerFileTransfer(const string& transferServerHost,
                                             const string& transferServerPort,
                                             const string& fileName,
                                             const string& databaseName,
                                             const string& relationName)
{
    printFunction("CommunicationClient::triggerFileTransfer");
    print("transferServerHost", transferServerHost);
    print("transferServerPort", transferServerPort);
    print("fileName", fileName);
    print("databaseName", databaseName);
    print("relationName", relationName);

    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        print("Not connected to CommunicationServer");
        return 1;
    }
    queue<string> sendBuffer;
    sendBuffer.push(CommunicationProtocol::CommunicationClient());
    sendBuffer.push(CommunicationProtocol::TriggerReplication());
    CommunicationUtils::sendBatch(io, sendBuffer);

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::ReplicationDetailsRequest()))
    {
        print("Did not receive expected ReplicationDetailsRequest keyword");
        return 2;
    }
    sendBuffer.push(transferServerHost);
    sendBuffer.push(transferServerPort);
    sendBuffer.push(fileName);
    sendBuffer.push(databaseName);
    sendBuffer.push(relationName);
    CommunicationUtils::sendBatch(io, sendBuffer);
    print("File transfer details sent to DBService worker");
    return 0;
}

void CommunicationClient::getLocationParameter(
        string& location, const char* key)
{
    printFunction("CommunicationClient::getLocationParameter");
    SecondoUtilsLocal::readFromConfigFile(location,
                                       "Environment",
                                       key,
                                       "");
}

int CommunicationClient::getReplicaLocation()
{
    printFunction("CommunicationClient::getReplicaLocation");
    return 0;
}

} /* namespace DBService */
