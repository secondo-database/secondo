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
#include <iostream>

#include "DBServiceCommunicationClient.hpp"
#include "DBServiceCommunicationProtocol.hpp"
#include "CommunicationUtils.hpp"
#include "DBServiceUtils.hpp"

#include "SocketIO.h"
#include "StringUtils.h"


using namespace std;
using namespace distributed2;

namespace DBService {

DBServiceCommunicationClient::DBServiceCommunicationClient(
        std::string& _server, int _port, Socket* _socket)
:Client(_server, _port, _socket)
{
}

int DBServiceCommunicationClient::start()
{
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

int DBServiceCommunicationClient::getNodesForReplication(string& relationName)
{
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            DBServiceCommunicationProtocol::CommunicationServer()))
    {
        return 1;
    }

    io << DBServiceCommunicationProtocol::CommunicationClient() << endl;
    io << DBServiceCommunicationProtocol::ProvideReplica() << endl;
    io << relationName
       << ":"
       << SecondoSystem::GetInstance()->GetDatabaseName()
       << endl;
    io.flush();

    if(!CommunicationUtils::receivedExpectedLine(io,
            DBServiceCommunicationProtocol::LocationRequest()))
    {
        return 2;
    }

    string location;
    buildLocationString(location);
    io << location << endl;
    io.flush();
    return 0;
}

void DBServiceCommunicationClient::buildLocationString(string& location)
{
    string host;
    DBServiceUtils::readFromConfigFile(host,
                                       "Environment",
                                       "SecondoHost",
                                       "");
    string port;
    DBServiceUtils::readFromConfigFile(port,
                                       "Environment",
                                       "SecondoPort",
                                       "");
    string disk;
    DBServiceUtils::readFromConfigFile(disk,
                                       "Environment",
                                       "SecondoHome",
                                       "");
    stringstream ss;
    ss << host << ":" << port << ":" << disk;
    location = ss.str();
}

int DBServiceCommunicationClient::getReplicaLocation()
{
    return 0;
}

} /* namespace DBService */
