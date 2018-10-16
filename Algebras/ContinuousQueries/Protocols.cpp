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

uint64_t ProtocolHelpers::getUnixTimestamp(const std::time_t* t) {
    std::time_t st = t == nullptr ? std::time(nullptr) : *t;
    auto secs = static_cast<std::chrono::seconds>(st).count();
    return static_cast<uint64_t>(secs);
}

void ProtocolHelpers::printMessage(ProtocolHelpers::Message msg) 
{   
    std::string v = "invalid";
    if (msg.valid) v = "valid";

    std::cout << "Socket " << std::to_string(msg.socket) 
        << " sends the " << v << " Command " << msg.cmd 
        << " with the parameters: " << msg.params << ".\n";
}

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
    size_t cmdPos = target->body.find("|");

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

std::string CgenToHidleP::registerHandler(std::string type, bool create)
{
    std::string r = "hello";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += type;

    return r;
}

std::string CgenToHidleP::confirmHandler(int id, bool create)
{
    std::string r = "welcome";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);

    return r;
}

std::string CgenToHidleP::shutdownHandler(std::string msg, bool create) 
{
    std::string r = "shutdown";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += msg;

    return r;
}

std::string CgenToHidleP::specializeHandler(std::string type, bool create)
{
    std::string r = "specialize";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += type;

    return r;
}

/*
3 Protocol for general Coordinator and LoopWorker

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CgenToWloopP::addNomo(int id, std::string address, bool create)
{
    std::string r = "addnomo";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += address;

    return r;
}

std::string CgenToWloopP::addQuery(int id, std::string function, bool create)
{
    std::string r = "addquery";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += function;

    return r;
}

/*
4 Protocol for general Coordinator and NoMo

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CgenToNoMoP::addUserQuery(int queryId, std::string query, 
        std::string mail, bool create)
{
    std::string r = "adduserquery";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(queryId);
    r += ProtocolHelpers::seperator;
    r += query;
    r += ProtocolHelpers::seperator;
    r += mail;

    return r;
}


/*
6 Protocol for general Coordinator and StreamSupplier

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CgenToStSuP::addWorker(int workerId, std::string address, 
    bool create)
{
    std::string r = "addworker";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(workerId);
    r += ProtocolHelpers::seperator;
    r += address;

    return r;
}

/*
6 Protocol for general Coordinator and NoMo

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string WgenToNoMoP::hit(std::string tuple, std::string hitlist, 
    bool create)
{
    std::string r = "hit";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += tuple;
    r += ProtocolHelpers::seperator;
    r += hitlist;

    return r;
}

/*
6 Protocol for Stream Supplier and Worker

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string StSuToWgenP::tuple(int id, std::string tuple, bool create)
{
    std::string r = "tuple";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += tuple;

    return r;
}

} /* namespace continuousqueries */
