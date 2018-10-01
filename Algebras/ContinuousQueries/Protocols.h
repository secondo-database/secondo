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
#include "Tcp/TcpServer.h"
#include "Tcp/TcpClient.h"

namespace continuousqueries {

class ProtocolHelpers {
public:
    struct Message {
        bool valid = false;
        int socket = -1;
        int64_t timestamp = 0;
        std::string body = "";
        std::string cmd = "";
        std::string params = "";
    };

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
    static std::string registerHandler(bool create=false);
    static std::string confirmHandler(int id=0, bool create=false);
    static std::string becomeSpecificHandler(std::string type="", 
        bool create=false);
    static std::string shutdownHandler(std::string msg="", bool create=false);
};

/*
3 ...

*/

} /* namespace continuousqueries */

#endif /* _PROTOCOLS_H_ */
