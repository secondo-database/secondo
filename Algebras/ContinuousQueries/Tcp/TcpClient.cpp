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

TcpClient::TcpClient(std::string targetAddress, int targetPort): 
    _targetAddress(targetAddress), 
    _targetPort(targetPort), 
    _running(false) {}

TcpClient::~TcpClient() {}

void TcpClient::Shutdown()
{
    _running = false;
}

void TcpClient::Initialize()
{
    //create the master socket  
    if( (_master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");
        exit(EXIT_FAILURE);
    }   
     
    // create the address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons( _targetPort );
    inet_pton(AF_INET, _targetAddress.c_str(), &address.sin_addr);

    // connect to the server
    if ( connect(
        _master_socket, (struct sockaddr *)&address, sizeof(address)
    ) < 0 )
    {   
        perror("connecting failed");   
        exit(EXIT_FAILURE);   
    }
         
    _running = true;
}

void TcpClient::Receive()
{
    char buffer[MAXPACKETSIZE];
    memset(buffer, 0, MAXPACKETSIZE);

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
            TcpClient::Message msg = CreateMsg(
                _master_socket, 
                (std::string) buffer
            );
            
            PushMsgToQueue(msg);

        }
        
        memset(buffer, 0, MAXPACKETSIZE);
    }
}

TcpClient::Message TcpClient::CreateMsg(int sockd, std::string body)
{
    TcpClient::Message msg;

    msg.body = body;
    msg.socket = sockd;
    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    return msg;
}

TcpClient::Message TcpClient::CreateConnectMsg(int sockd) 
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);   

    std::string body = "connected|" +
        (std::string) inet_ntoa(ad.sin_addr) + " " +
        std::to_string(ntohs(ad.sin_port));

    return TcpClient::CreateMsg(sockd, body);
}

TcpClient::Message TcpClient::CreateDisconnectMsg(int sockd)
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);

    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);   

    std::string body = "disconnected|" +
        (std::string) inet_ntoa(ad.sin_addr) + " " +
        std::to_string(ntohs(ad.sin_port));

    return TcpClient::CreateMsg(sockd, body);
}

void TcpClient::PushMsgToQueue(TcpClient::Message msg) 
{
    std::lock_guard<std::mutex> guard(mqMutex);
    std::cout << "TcpClient received '" << msg.body 
        << "' from socket " << std::to_string(msg.socket)
        << ". Pushing to queue.\n";

    messages.push(msg);

    mqCondition.notify_one();
}

int TcpClient::Send(std::string msg) 
{
    std::cout << "TcpClient sending '" + msg + "'.\n";
    return send(_master_socket, msg.c_str(), msg.length() + 1, 0) - 1;
}

void TcpClient::SendAsync(std::string msg)
{
    std::cout << "TcpClient adds '" + msg + "' to outgoing queue...";

    if (msg == "") return;
    
    std::lock_guard<std::mutex> outgoingLock(_outgoingMsgsMutex);
    _outgoingMsgsQueue.push(msg);
    _outgoingMsgsCondition.notify_one();

    std::cout << " done! \n";
}

void TcpClient::AsyncHandler()
{
    while (_running) 
    {
        std::unique_lock<std::mutex> outgoingLock(_outgoingMsgsMutex);

        _outgoingMsgsCondition.wait(outgoingLock, [this] {
            return !_outgoingMsgsQueue.empty();
        });

        std::string msg = _outgoingMsgsQueue.front();
        _outgoingMsgsQueue.pop();
        
        outgoingLock.unlock();

        Send(msg);
    }
}

int TcpClient::GetServerPort()
{ 
    return _targetPort;
}

std::string TcpClient::GetServerAddress()
{ 
    return _targetAddress;
}

int TcpClient::GetMasterSocket()
{ 
    return _master_socket;
}

bool TcpClient::IsRunning() {
    return _running;
}

}