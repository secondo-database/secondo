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

#ifndef _DISTRIBUTE_STREAM_PROTOCOL_H_
#define _DISTRIBUTE_STREAM_PROTOCOL_H_

#include <string>
#include <iostream>
#include <chrono>
#include "Tcp/TcpServer.h"
#include "Tcp/TcpClient.h"

namespace continuousqueries {


/*
1 ProtocolHelpers class implementation

Messages come as string, a TcpServer Message or a TcpClient Message.
It will be converted to a standardized ProtocolHelpers Message, which
includes a valid flag and splits the body to the cmd and params.

*/

class ProtocolHelpers {
public:
    struct Message {
        bool valid = false;
        int socket = -1;
        uint64_t timestamp = 0;
        std::string body = "";
        std::string cmd = "";
        std::string params = "";
    };

    static const char seperator = '|';

    static uint64_t getUnixTimestamp(const std::time_t* t=nullptr);

    static void printMessage(ProtocolHelpers::Message msg);

    static ProtocolHelpers::Message decodeMessage(std::string msg);
    static ProtocolHelpers::Message decodeMessage(TcpClient::Message msg);
    static ProtocolHelpers::Message decodeMessage(TcpServer::Message msg);

private:
    static void decodeBody(ProtocolHelpers::Message* target);
};

/*
2 Protocol for messages send by the (general/loop) Coordinator

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

class CoordinatorGenP {
public:
    static std::string confirmhello(int id=0, std::string tupledescr="", 
        bool create=false);

    static std::string specialize(std::string type="", bool create=false);
    
    static std::string addhandler(int id=0, std::string type="", 
        std::string address="", bool create=false);
    
    static std::string addquery(int id=0, std::string function="", 
        bool create=false);

    static std::string addquery(int queryId=0, std::string function="", 
        std::string userhash="", std::string mail="", bool create=false);

    static std::string shutdown(std::string reason="", bool create=false);

    // Webschnittstelle
    static std::string userauth(std::string type="", std::string tupledescr="",
        bool create=false);

    static std::string getqueries(int id=0, std::string func="",
        bool create=false);

    // Debug
    static std::string remote(bool create=false);
    static std::string status(bool create=false);
};

/*
3 Protocol for messages send by the Idle Handler

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

class IdleGenP {
public:
    static std::string hello(std::string type="idle", 
        bool create=false);
    
    static std::string confirmspecialize(std::string type="", 
        bool create=false);
};

/*
4 Protocol for messages send by the (general/loop) Worker

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

class WorkerGenP {
public:
    static std::string hit(int id=0, std::string tupleString="", 
        std::string hitlist="", bool create=false);

    static std::string confirmspecialize(bool create=false);
};

/*
5 Protocol for messages send by the (general) NoMo

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

class NoMoGenP {
public:
    static std::string confirmspecialize(bool create=false);
};

/*
6 Protocol for messages send by the (general) Stream Supplier

Returns the name of the command as default. Can also create the whole 
message to be sent by setting 'create' to true.

*/

class StSuGenP {
public:
    static std::string hello(bool create=false);

    static std::string tuple(int id=0, std::string data="",
        bool create=false);
};

/*
7 LOG Helper

Remove DEBUG OUTPUT to stop output.

*/

#define DEBUG_OUTPUT

class Log
{
public:

    bool Enabled() {
#ifdef DEBUG_OUTPUT
        return true;
#endif
        return false;
    }

    Log& operator<<(const char* s) {
        if (Enabled()) std::cout << s;
        return *this;
    }
    
    Log& operator<<(char* s) {
        if (Enabled()) std::cout << s;
        return *this;
    }
    
    Log& operator<<(std::string s) {
        if (Enabled()) std::cout << s;
        return *this;
    }

    Log& operator<<(int i) {
        if (Enabled()) std::cout << i;
        return *this;
    }

    Log& operator<<(short n) {
        if (Enabled()) std::cout << n;
        return *this;
    }

    Log& operator<<(unsigned int ui) {
        if (Enabled()) std::cout << ui;
        return *this;
    }

    Log& operator<<(char c) {
        if (Enabled()) std::cout << c;
        return *this;
    }

    Log& operator<<(double d) {
        if (Enabled()) std::cout << d;
        return *this;
    }

    Log& operator<<(float f) {
        if (Enabled()) std::cout << f;
        return *this;
    }

    static Log& Inst()
    {
        static Log inst;
        return inst;
    }

protected:
    Log() {};
    Log(const Log&) {};
    ~Log() {};
};

#define LOG Log::Inst()
#define ENDL "\n"

} /* namespace continuousqueries */

#endif /* _PROTOCOLS_H_ */
