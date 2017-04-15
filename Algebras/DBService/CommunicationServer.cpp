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
#include "SocketIO.h"
#include "StringUtils.h"

#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/CommunicationProtocol.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"

using namespace distributed2;
using namespace std;

namespace DBService {

CommunicationServer::CommunicationServer(int port) :
        Server(port)
{
    cout << "Initializing CommunicationServer (port " << port << ")"
            << endl;
}

CommunicationServer::~CommunicationServer()
{}

int CommunicationServer::start()
{
    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    if (!listener->IsOk())
    {
        return 1;
    }
    server = listener->Accept();
    if (!server->IsOk())
    {
        return 2;
    }
    return communicate();
}

iostream& CommunicationServer::getSocketStream()
{
    return server->GetSocketStream();
}

int CommunicationServer::communicate()
{
    try
    {
        iostream& io = getSocketStream();
        io << CommunicationProtocol::CommunicationServer() << endl;
        io.flush();

        string line;
        getline(io, line);
        while(line != CommunicationProtocol::ShutDown())
        {
            if (line != CommunicationProtocol::CommunicationClient())
            {
                cerr << "Protocol error" << endl;
                continue;
            }
            getline(io, line);
            if(line == CommunicationProtocol::ProvideReplica())
            {
                handleProvideReplicaRequest(io);
            }else if(line == CommunicationProtocol::UseReplica())
            {
                //TODO contact DBServiceManager and find out where replica is
                // stored
                //DBServiceManager::getInstance();
            }else
            {
                cerr << "Protocol error" << endl;
                continue;
            }
            getline(io, line);
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
    string line;
    getline(io, line);
    string relationAndDatabaseName = line;
    io << CommunicationProtocol::LocationRequest() << endl;
    io.flush();
    getline(io, line);
    // TODO line contains host, port and disk of remote system
    // -> store in LocationInfo object
    return true;
}

bool CommunicationServer::handleUseReplicaRequest(
        std::iostream& io)
{
    return true;
}

} /* namespace DBService */
