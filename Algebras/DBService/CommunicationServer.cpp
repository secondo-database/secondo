/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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
#include <boost/bind.hpp>

#include "StringUtils.h"

#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/ReplicationClient.hpp"

using namespace distributed2;
using namespace std;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        MultiClientServer(port)
{
    cout << "Initializing CommunicationServer (port " << port << ")"
            << endl;
}

CommunicationServer::~CommunicationServer()
{}

int CommunicationServer::communicate(iostream& io)
{
    try
    {
        CommunicationUtils::sendLine(io,
                CommunicationProtocol::CommunicationServer());

        while(!CommunicationUtils::receivedExpectedLine(io,
                CommunicationProtocol::ShutDown()))
        {
            if (!CommunicationUtils::receivedExpectedLine(io,
                    CommunicationProtocol::CommunicationClient()))
            {
                cerr << "Protocol error" << endl;
                continue;
            }

            queue<string> receivedLines;
            CommunicationUtils::receiveLines(io, 1, receivedLines);
            string request = receivedLines.front();
            receivedLines.pop();

            if(request ==
                    CommunicationProtocol::ProvideReplica())
            {
                handleProvideReplicaRequest(io);
            }else if(request ==
                    CommunicationProtocol::TriggerReplication())
            {
                handleTriggerFileTransferRequest(io);
            }else if(request ==
                    CommunicationProtocol::UseReplica())
            {
                //TODO contact DBServiceManager and find out where replica is
                // stored
                //DBServiceManager::getInstance();
            }else
            {
                cerr << "Protocol error" << endl;
                continue;
            }
        }
    } catch (...)
    {
        cerr << "CommunicationServer: communication error" << endl;
        return 5;
    }
    return 0;
}

bool CommunicationServer::handleProvideReplicaRequest(
        std::iostream& io)
{
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::RelationRequest());

    queue<string> receivedLines;
    CommunicationUtils::receiveLines(io, 2, receivedLines);

    string databaseName = receivedLines.front();
    receivedLines.pop();
    string relationName = receivedLines.front();
    receivedLines.pop();

    CommunicationUtils::sendLine(io,
            CommunicationProtocol::LocationRequest());

    CommunicationUtils::receiveLines(io, 3, receivedLines);
    string host = receivedLines.front();
    receivedLines.pop();
    string port = receivedLines.front();
    receivedLines.pop();
    string disk = receivedLines.front();
    receivedLines.pop();

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

    for(vector<ConnectionID>::const_iterator it = connections.begin();
            it != connections.end(); it++)
    {
        LocationInfo& location = dbService->getLocation(*it);
        sendBuffer.push(location.getHost());
        sendBuffer.push(location.getPort());
        sendBuffer.push(location.getDisk());
        sendBuffer.push(location.getCommPort());
        sendBuffer.push(location.getTransferPort());
    }
    CommunicationUtils::sendBatch(io, sendBuffer);
    return true;
}

bool CommunicationServer::handleTriggerFileTransferRequest(std::iostream& io)
{
    CommunicationUtils::sendLine(io,
            CommunicationProtocol::ReplicationDetailsRequest());
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

    ReplicationClient client(host,
                             atoi(port.c_str()),
                             fileName,
                             databaseName,
                             relationName);
    client.start();
    return true;
}

bool CommunicationServer::handleUseReplicaRequest(
        std::iostream& io)
{
    return true;
}


} /* namespace DBService */
