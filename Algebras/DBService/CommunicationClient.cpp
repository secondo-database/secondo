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

#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationClient.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/CommunicationUtils.hpp"
#include "Algebras/DBService/SecondoUtils.hpp"



using namespace std;
using namespace distributed2;

namespace DBService {

CommunicationClient::CommunicationClient(
        std::string& _server, int _port, Socket* _socket)
:Client(_server, _port, _socket)
{
}

int CommunicationClient::start()
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

int CommunicationClient::getNodesForReplication(string& relationName)
{
    iostream& io = socket->GetSocketStream();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::CommunicationServer()))
    {
        return 1;
    }

    io << CommunicationProtocol::CommunicationClient() << endl;
    io << CommunicationProtocol::ProvideReplica() << endl;
    io << relationName
       << ":"
       << SecondoSystem::GetInstance()->GetDatabaseName()
       << endl;
    io.flush();

    if(!CommunicationUtils::receivedExpectedLine(io,
            CommunicationProtocol::LocationRequest()))
    {
        return 2;
    }

    string location;
    buildLocationString(location);
    io << location << endl;
    io.flush();
    return 0;
}

void CommunicationClient::buildLocationString(string& location)
{
    string host;
    SecondoUtils::readFromConfigFile(host,
                                       "Environment",
                                       "SecondoHost",
                                       "");
    string port;
    SecondoUtils::readFromConfigFile(port,
                                       "Environment",
                                       "SecondoPort",
                                       "");
    string disk;
    SecondoUtils::readFromConfigFile(disk,
                                       "Environment",
                                       "SecondoHome",
                                       "");
    stringstream ss;
    ss << host << ":" << port << ":" << disk;
    location = ss.str();
}

int CommunicationClient::getReplicaLocation()
{
    return 0;
}

} /* namespace DBService */
