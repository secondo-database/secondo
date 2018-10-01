/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of protocols for SSP communication.

[toc]

*/

#include "Protocols.h"
#include <iostream>

namespace continuousqueries {

/*
1 ProtocolHelpers class implementation

Messages come as string, as nested lists. The TcpServer sents 
(SOCKET)(MESSAGE). The SOCKET is an integer. The MESSAGE a string, 
hopefully constructed by an Protocol Class. The MESSAGE itself 
should be (COMMAND) (PARAMETERS).

*/

// Should come as (SOCKET)((COMMAND) (PARAMETERS))
ProtocolHelpers::Message ProtocolHelpers::decodeMessage(std::string msg)
{
    Message m;
        m.valid = false;
        m.cmd = msg;
        m.params = msg;
        m.socket = -1;

    size_t endSocket, endCmd;
    std::string cmd, params;

    endSocket = msg.find(")((");
    endCmd = msg.find(") (", endSocket);

    if (msg.substr(0,1) != "(" || 
        endSocket == std::string::npos || 
        endCmd == std::string::npos) 
    {
        std::cout << "error 1 in message conversion\n";
        return m;
    }

    try {
        m.socket = std::stoi(msg.substr(1, endSocket-1));
    } catch(...) {
        std::cout << "error 2 in message conversion\n";
        m.socket = -1;
        return m;
    }
    
    m.cmd = msg.substr(endSocket+3, endCmd-endSocket-3);
    m.params = msg.substr(endCmd+3, msg.size()-endCmd-3-2);
    m.valid = true;

    return m;
}

/*
2 Protocol for general Coordinator and IdleHandlers

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CgenToHidleP::registerHandler(bool create)
{
    if (!create) return "hello";
    return "(hello) ()";
}

std::string CgenToHidleP::confirmHandler(int id, bool create)
{
    if (!create) return "welcome";
    return "(welcome) (" + std::to_string(id) + ")";
}

std::string CgenToHidleP::shutdownHandler(std::string msg, bool create) 
{
    if (!create) return "shutdown";
    return "(shutdown) (" + msg + ")";
}

std::string CgenToHidleP::becomeSpecificHandler(std::string type, bool create)
{
    if (!create) return "become";
    return "(become) (" + type + ")";
}

} /* namespace continuousqueries */
