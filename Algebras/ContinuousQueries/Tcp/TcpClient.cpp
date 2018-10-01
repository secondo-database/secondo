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

[1] Implementation of generic Tcp Client.

[toc]

1 TcpClient class implementation

*/

#include "TcpClient.h"

namespace continuousqueries {

TcpClient::TcpClient(std::string ip, int port): _ip(ip), _port(port), 
    _running(false) {}

TcpClient::~TcpClient() {}

void TcpClient::Run()
{
    char buffer[MAXPACKETSIZE];
    memset(buffer, 0, MAXPACKETSIZE);

    //create the master socket  
    if( (_master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    // create the address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons( _port );
    inet_pton(AF_INET, _ip.c_str(), &address.sin_addr);

    // connect to the server
    if ( connect(
        _master_socket, (struct sockaddr *)&address, sizeof(address)
    ) < 0 )
    {   
        perror("connecting failed");   
        exit(EXIT_FAILURE);   
    }
         
    _running = true;

    while(_running) 
    {
        // wait for a message
        int bytesReceived = recv(_master_socket, buffer, MAXPACKETSIZE, 0);

        // check if it was an error
        if (bytesReceived == -1) 
        {
            // some error... ignore for the beginning
        }

        // check if it was for closing
        else if (bytesReceived == 0)
        {
            PushMsgToQueue(CreateDisconnectMsg(_master_socket));

            _running = false;
        }   
                
        // Push the incoming message to the queue
        else 
        {
            std::string msg = CreateMsg(_master_socket, buffer);
            
            PushMsgToQueue(msg);

            memset(buffer, 0, MAXPACKETSIZE);
        }
    }
}

std::string TcpClient::CreateMsg(int sockd, char buffer[MAXPACKETSIZE])
{
    std::string msg(buffer);
    // msg = msg.substr(0, msg.length()-1);

    return "(" + std::to_string(sockd) + ")(" + msg + ")";
}

std::string TcpClient::CreateConnectMsg(int sockd) 
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);   
 
    return "(" + std::to_string(sockd) + ")((connected) (" 
        + inet_ntoa(ad.sin_addr) + " " 
        + std::to_string(ntohs(ad.sin_port)) + "))";
}

std::string TcpClient::CreateDisconnectMsg(int sockd)
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);   
 
    return "(" + std::to_string(sockd) + ")((disconnected) (" 
        + inet_ntoa(ad.sin_addr) + " " 
        + std::to_string(ntohs(ad.sin_port)) + "))";
}

void TcpClient::PushMsgToQueue(std::string msg) 
{
    std::lock_guard<std::mutex> guard(mqMutex);
    std::cout << "TcpClient received '" + msg + "'. Adding to queue.\n";

    messages.push(msg);

    mqCondition.notify_one();
}

int TcpClient::GetMasterPort()
{ 
    return _port;
}

int TcpClient::GetMasterSocket()
{ 
    return _master_socket;
}

bool TcpClient::IsRunning() {
    return _running;
}

int TcpClient::Send(std::string msg) 
{
    std::cout << "TcpClient sending '" + msg + "'.\n";
    return send(_master_socket, msg.c_str(), msg.length() + 1, 0) - 1;
}


}