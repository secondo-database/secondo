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

[1] Implementation of generic Tcp Server.

[toc]

1 TcpServer class implementation

*/

#include "TcpServer.h"

namespace continuousqueries {

// Defaualt Constructor
TcpServer::TcpServer(): _port(-1), _running(false) {}

TcpServer::TcpServer(int port): _port(port), _running(false) {}

TcpServer::~TcpServer() {}

void TcpServer::Run()
{
    if (_port == -1) return;
    
    int addrlen, new_socket, client_socket[SOMAXCONN],  
        activity, i, valread, sd, max_sd, opt = 1;
    struct sockaddr_in address;
    char buffer[MAXPACKETSIZE];
    fd_set readfds;
         
    //a message  
    // char const *message = "ECHO Daemon v1.0 \r\n";
     
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < SOMAXCONN; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a master socket  
    if( (_master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(_master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( _port );   

    //bind the socket to localhost and port  
    if (bind(_master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", _port);   
         
    //try to specify maximum of 30 pending connections for the master socket
    if (listen(_master_socket, 30) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    _running = true;

    while(_running) 
    {
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(_master_socket, &readfds);
        max_sd = _master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < SOMAXCONN ; i++)
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket,
        //then its an incoming connection  
        if (FD_ISSET(_master_socket, &readfds)) 
        {
            if ((new_socket = accept(
                    _master_socket, 
                    (struct sockaddr *)&address, 
                    (socklen_t*)&addrlen)
                )<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
   
            // add new socket to array of sockets on empty 
            // position and inform via message queue
            for (i = 0; i < SOMAXCONN; i++)   
            {   
                if (client_socket[i] == 0) 
                {   
                    client_socket[i] = new_socket;
                    PushMsgToQueue(CreateConnectMsg(new_socket));
                    break;   
                }   
            }   
        }   
             
        //else it's some IO operation on some other socket 
        for (i = 0; i < SOMAXCONN; i++) 
        {
            sd = client_socket[i];   
                 
            if (FD_ISSET(sd , &readfds)) 
            {
                // Check if it was for closing and read the incoming message
                if ((valread = read( sd , buffer, MAXPACKETSIZE)) == 0)
                {
                    // Somebody disconnected, inform via message queue
                    PushMsgToQueue(CreateDisconnectMsg(sd));

                    // Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }   
                     
                // Push the incoming message to the queue
                else 
                {
                    std::string msg = CreateMsg(sd, buffer);
                    
                    PushMsgToQueue(msg);

                    memset(buffer, 0, MAXPACKETSIZE);
                }   
            }   
        }   
    }
}

std::string TcpServer::CreateMsg(int sockd, char buffer[MAXPACKETSIZE])
{
    std::string msg(buffer);
    // msg = msg.substr(0, msg.length()-1);

    return "(" + std::to_string(sockd) + ")(" + msg + ")";
}

std::string TcpServer::CreateConnectMsg(int sockd) 
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);
 
    return "(" + std::to_string(sockd) + ")((connected) (" 
        + inet_ntoa(ad.sin_addr) + " " 
        + std::to_string(ntohs(ad.sin_port)) + "))";
}

std::string TcpServer::CreateDisconnectMsg(int sockd)
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);   
 
    return "(" + std::to_string(sockd) + ")((disconnected) (" 
        + inet_ntoa(ad.sin_addr) + " " 
        + std::to_string(ntohs(ad.sin_port)) + "))";
}

void TcpServer::PushMsgToQueue(std::string msg) 
{
    std::lock_guard<std::mutex> guard(mqMutex);
    std::cout << "TcpServer received '" + msg + "'. Pushing to queue.\n";

    messages.push(msg);

    mqCondition.notify_one();
}

int TcpServer::GetPortFromSocket(int sockd) 
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);
 
    return ntohs(ad.sin_port);
}

std::string TcpServer::GetIpFromSocket(int sockd) 
{
    struct sockaddr_in ad;
    int adlen = sizeof(ad);
    
    getpeername(sockd, (struct sockaddr*)&ad, (socklen_t*)&adlen);
 
    return std::string(inet_ntoa(ad.sin_addr));
}

int TcpServer::GetMasterPort()
{ 
    return _port;
}

int TcpServer::GetMasterSocket()
{ 
    return _master_socket;
}

bool TcpServer::IsRunning() {
    return _running;
}

int TcpServer::Send(int socket, std::string msg) 
{
    std::cout << "TcpServer sending '" + msg + "' to Socket '" 
        << socket << "'.\n";
    return send(socket, msg.c_str(), msg.length() + 1, 0) - 1;
}


}