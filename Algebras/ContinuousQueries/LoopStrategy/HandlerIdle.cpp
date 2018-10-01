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

[1] Implementation of the idle handler.

[toc]

1 IdleHandler class implementation

*/

#include "HandlerIdle.h"

namespace continuousqueries {

/*
1.1 Constructor

Creates a new HandlerIdle object.

*/

HandlerIdle::HandlerIdle(std::string ip, int port): 
    _coordinatorIp(ip),
    _coordinatorPort(port),
    _id(0),
    _coordinationClient(ip, port)
{
}

/*
1.2 Destructor

Destroys the HandlerIdle object and sets all ressources free.

*/

HandlerIdle::~HandlerIdle() {
    Shutdown();
    // TODO: Verbindungen lösen, etc.
}

/*
1.3 Receive Coordination Message

Receives 

*/

void HandlerIdle::ReceiveCoordinationMessage(std::string message) {

}

/*
1.4 Initialize

Connects to the Coordinator to receive instructions about specilization.

The coordinationClient runs in a seperate thread and keeps running after the 
initialitation is done. It pushes incoming messages to its queue.

*/

void HandlerIdle::Initialize() {
    // start the client, received messages will be pushed to _MessageQueque
    _coordinationClientThread = std::thread(
        &TcpClient::Run, 
        &_coordinationClient
    );
    _coordinationClientThread.detach();
    
    // wait for the connection to be established
    if (!_coordinationClient.IsRunning())
    {
        int count = 0;
        std::cout << "Waiting a maximum of 60 seconds for the connection...";
        while (!_coordinationClient.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << ".";
            count = count + 100;
        }
        if (_coordinationClient.IsRunning()) std::cout << " Done!\n";
    }

    if (!_coordinationClient.IsRunning()) return;

    // send Hello message to the Coordinator
    size_t sendl;
    do
    {
        sendl = _coordinationClient.Send(
            CgenToHidleP::registerHandler(true)
        );

        if (sendl != CgenToHidleP::registerHandler(true).length())
            std::cout << "error while sending registerHandler message \n";
    } while (sendl != CgenToHidleP::registerHandler(true).length());

    // wait for a specilization
    bool noSpecilization = true;

    while (noSpecilization) {
        std::unique_lock<std::mutex> lock(_coordinationClient.mqMutex);

        _coordinationClient.mqCondition.wait(lock, [this] {
            return !_coordinationClient.messages.empty();
        });

        ProtocolHelpers::Message msg = 
        ProtocolHelpers::decodeMessage(
            _coordinationClient.messages.front()
        );
        _coordinationClient.messages.pop();
        
        lock.unlock();

        if (msg.valid) {
            // confirm the id set by the coordinator
            if (msg.cmd == CgenToHidleP::confirmHandler()) 
            {
                int handlerId = 0;
                
                try
                {
                    handlerId = std::stoi(msg.params);
                }
                catch(...)
                {
                    std::cerr << "failed to convert id to int \n";
                }
                
                sendl = _coordinationClient.Send(
                    CgenToHidleP::confirmHandler(handlerId, true)
                );

                if (
                    sendl == CgenToHidleP::confirmHandler(handlerId, true)
                        .length() && handlerId) 
                {
                    std::cout << "Set my ID to " << handlerId << ". \n";
                    _id = handlerId;
                } else {
                    std::cout << "error while sending "
                        << "confirmHandler message \n";
                }
            } 

            // get a specialization
            if (msg.cmd == CgenToHidleP::becomeSpecificHandler()) 
            {
                // an id is neccessary for a specialization
                if (_id == 0) 
                {
                    std::cout << "can't specialize without an id \n";

                }

                // become a worker
                if (msg.params == "worker")
                {

                }

                // become a notification and monitoring hander
                else if (msg.params == "nomo")
                {

                }

                // unknown handler
                else
                {
                    std::cout << "No idea what a " << msg.params 
                        << " handler is.\n";
                }
            }

            // force shutdown
            else if (msg.cmd == CgenToHidleP::shutdownHandler() || 
                msg.cmd == "disconnected") 
            {
                
                std::cout << "shutting down due to " << msg.cmd 
                    << " " << msg.params << "\n";
                return; // TODO: graceful shutdown!
            } 

            // unknown command
            else 
            {
                std::cout << "No handler for command " << msg.cmd << ".\n";
            }

        } else {
            std::cout << "Message '" + msg.cmd + "' is invalid... \n";
        }   
    }
}

void HandlerIdle::Shutdown() {
    // TODO: Verbindungen lösen, etc.
}

}