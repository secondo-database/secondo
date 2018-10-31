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
2 Protocol for messages send by the (general/loop) Coordinator

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string CoordinatorGenP::confirmhello(int id, std::string tupledescr, 
    bool create)
{
    std::string r = "confirmhello";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += tupledescr;

    return r;
}

std::string CoordinatorGenP::specialize(std::string type, bool create)
{
    std::string r = "specialize";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += type;

    return r;
}

std::string CoordinatorGenP::addhandler(int id, std::string type, 
    std::string address, bool create)
{
    std::string r = "addhandler";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += type;
    r += ProtocolHelpers::seperator;
    r += address;

    return r;
}

std::string CoordinatorGenP::addquery(int id, std::string function, bool create)
{
    std::string r = "addquery";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += function;

    return r;
}

std::string CoordinatorGenP::addquery(int queryId, std::string function, 
        std::string userhash, std::string mail, bool create)
{
    std::string r = "adduserquery";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(queryId);
    r += ProtocolHelpers::seperator;
    r += function;
    r += ProtocolHelpers::seperator;
    r += userhash;
    r += ProtocolHelpers::seperator;
    r += mail;

    return r;
}

std::string CoordinatorGenP::shutdown(std::string reason, bool create) 
{
    std::string r = "shutdown";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += reason;

    return r;
}

std::string CoordinatorGenP::userauth(std::string type, 
    std::string tupledescr, bool create)
{
    std::string r = "userauth";
    if (!create) return r;

    r += ProtocolHelpers::seperator;
    r += type;
    r += ProtocolHelpers::seperator;
    r += tupledescr;

    return r;
}

std::string CoordinatorGenP::getqueries(int id, 
    std::string func, bool create)
{
    std::string r = "getqueries";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += func;

    return r;
}

std::string CoordinatorGenP::remote(bool create) 
{
    std::string r = "remote";
    return r;
}

std::string CoordinatorGenP::status(bool create) 
{
    std::string r = "status";
    return r;
}

/*
3 Protocol for messages send by the Idle Handler

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string IdleGenP::hello(std::string type, bool create)
{
    std::string r = "hello";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += type;

    return r;
}

std::string IdleGenP::confirmspecialize(std::string type, bool create)
{
    std::string r = "confirmspecialize";
    if (!create) return r;
    
    r += ProtocolHelpers::seperator;
    r += type;

    return r;
}

/*
4 Protocol for messages send by the (general/loop) Worker

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string WorkerGenP::confirmspecialize(bool create)
{
    if (!create) return IdleGenP::confirmspecialize();
    return IdleGenP::confirmspecialize("worker", true);
}

std::string WorkerGenP::hit(int id, std::string tupleString, 
    std::string hitlist, bool create)
{
    std::string r = "hit";
    if (!create) return r;

    r += ProtocolHelpers::seperator;
    r += std::to_string(id);
    r += ProtocolHelpers::seperator;
    r += tupleString;
    r += ProtocolHelpers::seperator;
    r += hitlist;

    return r;
}

/*
5 Protocol for messages send by the (general) NoMo

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string NoMoGenP::confirmspecialize(bool create)
{
    if (!create) return IdleGenP::confirmspecialize();
    return IdleGenP::confirmspecialize("nomo", true);
}

/*
6 Protocol for messages send by the (general) Stream Supplier

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

std::string StSuGenP::hello(bool create)
{
    if (!create) return IdleGenP::hello();
    return IdleGenP::hello("streamsupplier", true);
}

std::string StSuGenP::tuple(int id, std::string tuple, bool create)
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
