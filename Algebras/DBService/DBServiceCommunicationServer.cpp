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

#include "DBServiceCommunicationServer.hpp"
#include "DBServiceCommunicationProtocol.hpp"

#include "SocketIO.h"
#include "StringUtils.h"


using namespace distributed2;
using namespace std;

namespace DBService {

DBServiceCommunicationServer::DBServiceCommunicationServer(int port) :
        Server(port)
{
    cout << "Initializing DBServiceCommunicationServer (port " << port << ")"
            << endl;
}

DBServiceCommunicationServer::~DBServiceCommunicationServer()
{}

int DBServiceCommunicationServer::start()
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

iostream& DBServiceCommunicationServer::getSocketStream()
{
    return server->GetSocketStream();
}

int DBServiceCommunicationServer::communicate()
{
    try
    {
        iostream& io = getSocketStream();
        io << DBServiceCommunicationProtocol::CommunicationServer() << endl;
        io.flush();
        string line;
        getline(io, line);
        while(line != DBServiceCommunicationProtocol::ShutDown())
        {
            if (line != DBServiceCommunicationProtocol::CommunicationClient())
            {
                cerr << "Protocol error" << endl;
                continue;
            }

            getline(io, line);
        }
    } catch (...)
    {
        cerr << "DBServiceCommunicationServer: communication error" << endl;
        return 5;
    }
    return 0;
}

} /* namespace DBService */
