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
#include <boost/algorithm/string.hpp>


namespace continuousqueries {

/*
1 ProtocolHelpers class implementation

Messages come as string, a TcpServer Message or a TcpClient Message.
It will be converted to a standardized ProtocolHelpers Message, which
includes a valid flag and splits the body to the cmd and params.

*/

ProtocolHelpers::Message ProtocolHelpers::decodeMessage(std::string body)
{
    ProtocolHelpers::Message m;
    m.body = body;

    ProtocolHelpers::decodeBody(&m);

    if (m.cmd != "") m.valid = true;
    return m;
}

ProtocolHelpers::Message ProtocolHelpers::decodeMessage(TcpServer::Message msg)
{
    ProtocolHelpers::Message m;
    m.body = msg.body;
    m.timestamp = msg.timestamp;
    m.socket = msg.socket;

    ProtocolHelpers::decodeBody(&m);

    if (m.cmd != "") m.valid = true;
    return m;
}

ProtocolHelpers::Message ProtocolHelpers::decodeMessage(TcpClient::Message msg)
{
    ProtocolHelpers::Message m;
    m.body = msg.body;
    m.timestamp = msg.timestamp;
    m.socket = msg.socket;

    ProtocolHelpers::decodeBody(&m);

    if (m.cmd != "") m.valid = true;
    return m;
}

void ProtocolHelpers::decodeBody(ProtocolHelpers::Message* target) {
    size_t cmdPos = target->body.find("<");

    if (cmdPos == std::string::npos) 
    {
        target->valid = false;
        target->cmd = "";
        target->params = "";
        return;
    }

    target->cmd = boost::algorithm::trim_copy(target->body.substr(0, cmdPos));
    target->params = boost::algorithm::trim_copy(
        target->body.substr(cmdPos + 1, target->body.size() - cmdPos));
}

/*
2 Protocol for general Coordinator and IdleHandlers

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CgenToHidleP::registerHandler(bool create)
{
    if (!create) return "hello";
    return "hello<";
}

std::string CgenToHidleP::confirmHandler(int id, bool create)
{
    if (!create) return "welcome";
    return "welcome<" + std::to_string(id);
}

std::string CgenToHidleP::shutdownHandler(std::string msg, bool create) 
{
    if (!create) return "shutdown";
    return "shutdown<" + msg;
}

std::string CgenToHidleP::becomeSpecificHandler(std::string type, bool create)
{
    if (!create) return "become";
    return "become<" + type;
}

} /* namespace continuousqueries */
