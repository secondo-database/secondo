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

[1] Implementation of a generic Tcp Client

[toc]

1 Tcpclient class implementation
see TcpClient.cpp for details.

*/

#ifndef __TCPCLIENT_H__
#define __TCPCLIENT_H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

// #include "Protocols.h" // wird hier nicht gebraucht
#include <mutex>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <string>

namespace continuousqueries {

// make sure to change this in TcpServer.h as well!
#define MAXPACKETSIZE 4096

class TcpClient {
public:
    struct Message {
        int socket=-1;
        int64_t timestamp=0;
        std::string body="";
    };

    TcpClient(std::string ip, int port);
    ~TcpClient();
    
    void Run();
    
    int GetMasterPort();
    int GetMasterSocket();

    TcpClient::Message CreateMsg(int sockd, std::string body);
    TcpClient::Message CreateConnectMsg(int sockd);
    TcpClient::Message CreateDisconnectMsg(int sockd);

    void PushMsgToQueue(TcpClient::Message msg);
    bool IsRunning();

    int Send(std::string msg);
    
    std::queue<TcpClient::Message> messages;
    std::mutex mqMutex;
    std::condition_variable mqCondition;

private:
    std::string _ip;
    int _port;
    bool _running;
    int _master_socket;
};

}

#endif