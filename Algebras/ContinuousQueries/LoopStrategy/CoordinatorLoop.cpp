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

[1] Implementation of the Coordinator (Worker type: Loop)

[toc]

1 CoordinatorLoop class implementation

*/

#include "CoordinatorLoop.h"
// #include "CoordinationServer.h"

namespace continuousqueries {

/*
1.1 Constructor

Creates a new CoordinatorLoop object.

*/

CoordinatorLoop::CoordinatorLoop(int port): 
    _lastId(0),
    _coordinationPort(port),
    _coordinationServer(port)
{

}

/*
1.2 Destructor

Destroys the CoordinatorLoop object and sets all ressources free.

*/

CoordinatorLoop::~CoordinatorLoop() {
    // TODO: Verbindungen lösen, etc.
}

/*
1.3 Receive Coordination Message

Receives 

*/

void CoordinatorLoop::ReceiveCoordinationMessage(std::string message) {

}

/*
1.4 Initialize

Creates the communication channel and then waits for all neccessary handlers 
to connect. (Minimum configuration is 1 stream supplier, 1 worker and 1 
notification & monitoring handler.)

The coordinationServer runs in a seperate thread and keeps running after the 
initialitation is done. It pushes incoming messages to its queue.

*/

void CoordinatorLoop::Initialize() {
    // start the server, received messages will be pushed to _MessageQueque
    _coordinationServerThread = std::thread(
        &TcpServer::Run, 
        &_coordinationServer
    );
    _coordinationServerThread.detach();
    
    // wait for the server to be started
    if (!_coordinationServer.IsRunning()) 
    {
        int count = 0;
        std::cout << "Waiting a maximum of 60 seconds for the server "
            << "to start...";

        while (!_coordinationServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << ".";
            count = count + 100;
        }
        if (_coordinationServer.IsRunning()) std::cout << " Done!\n";
    }

    if (!_coordinationServer.IsRunning()) return;

    std::cout << "New handler have to connect to this host on port " 
        << std::to_string(_coordinationServer.GetMasterPort()) << ". \n";

    // wait for enough handler to connect
    bool networkIncomplete = true;
    // bool hasWorker = false;
    // bool hasNoMo = false;
    // bool hasStreamSupplier = false;

    while (networkIncomplete) {
        std::unique_lock<std::mutex> lock(_coordinationServer.mqMutex);

        _coordinationServer.mqCondition.wait(lock, [this] {
            return !_coordinationServer.messages.empty();
        });

        ProtocolHelpers::Message msg = ProtocolHelpers::decodeMessage(
            _coordinationServer.messages.front()
        );
        _coordinationServer.messages.pop();
        
        lock.unlock();

        if (msg.valid) {
            std::cout << "Socket " << std::to_string(msg.socket) 
                << " sends the Command " << msg.cmd 
                << " with the parameters: " << msg.params << ".\n";
            
            // add a new idle handler to the network
            if (msg.cmd == CgenToHidleP::registerHandler()) {
                int handlerId = registerNewHandler(msg);
                int sendl = _coordinationServer.Send(
                    msg.socket,
                    CgenToHidleP::confirmHandler(handlerId, true)
                );

                if (
                    (size_t)sendl != 
                    CgenToHidleP::confirmHandler(handlerId, true).length()
                )
                    std::cout << "error while sending "
                        << "confirmHandler message \n";
            } else

            // new handler confirms his id
            if (msg.cmd == CgenToHidleP::confirmHandler()) {
                int handlerConfirmed = confirmHandlerIntegrity(msg);

                if (handlerConfirmed) {
                    _handlers[handlerConfirmed].status = active;
                } else {
                    _coordinationServer.Send(
                        msg.socket,
                        CgenToHidleP::shutdownHandler(
                            "failed to respond with correct id", 
                            true
                        )
                    );
                }
            }

        } else {
            std::cout << "Message '" + msg.body + "' is invalid... \n";
        }   
    }
}

// returns the id of the new handler
int CoordinatorLoop::registerNewHandler(ProtocolHelpers::Message msg) 
{
    handlerStruct handler;
    handler.id = ++_lastId;
    handler.socket = msg.socket;
    handler.address = _coordinationServer.GetIpFromSocket(msg.socket);
    handler.port = _coordinationServer.GetPortFromSocket(msg.socket);
    handler.type = idle;
    handler.status = inactice;

    _handlers.insert( std::pair<int, handlerStruct>(handler.id, handler));

    return handler.id;
}

// returns the id of the checked handler
int CoordinatorLoop::confirmHandlerIntegrity(
    ProtocolHelpers::Message msg, 
    int testId
)
{
    // generate the testId from the message, especially for new handlers
    if (!testId) {
        try {
            testId = std::stoi(msg.params);
        } catch(...) {
            return 0;
        }
    }

    // test if an handler with this id exits
    if (_handlers.find(testId) == _handlers.end()) return 0;

    // saved and received socket should be identical
    if (msg.socket == _handlers[testId].socket) return testId;

    return 0;
}


}