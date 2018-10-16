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

1 Protocols class implementation

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
1 ...

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
2 Protocol for general Coordinator and IdleHandlers

*/

class CgenToHidleP {
public:
    static std::string registerHandler(std::string type="idle", 
        bool create=false);
    static std::string confirmHandler(int id=0, bool create=false);
    static std::string specializeHandler(std::string type="worker|loop", 
        bool create=false);
    static std::string shutdownHandler(std::string msg="", bool create=false);
};

/*
3 ...

*/

class CgenToWloopP {
public:
    static std::string addNomo(int id=0, std::string address="", 
        bool create=false);
    static std::string addQuery(int id=0, std::string function="", 
        bool create=false);
};

/*
4 ...

*/

class CgenToNoMoP {
public:
    static std::string addUserQuery(int queryId=0, std::string query="", 
        std::string mail="", bool create=false);
};

/*
5 ...

*/

class CgenToStSuP {
public:
    static std::string addWorker(int workerId=0, std::string address="", 
        bool create=false);
};

/*
6 ...

*/

class WgenToNoMoP {
public:
    static std::string hit(std::string tuple="", std::string hitlist="", 
        bool create=false);
};

/*
7 ...

*/

class StSuToWgenP {
public:
    static std::string tuple(int id=0, std::string tuple="",
        bool create=false);
};


} /* namespace continuousqueries */

#endif /* _PROTOCOLS_H_ */
