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
#include <boost/algorithm/string.hpp>

namespace continuousqueries {

/*
1.1 Constructor

Creates a new HandlerIdle object.

*/

HandlerIdle::HandlerIdle(std::string address, int port): 
    _coordinatorAddress(address),
    _coordinatorPort(port),
    _id(0),
    _coordinationClient(address, port)
{
}

/*
1.2 Destructor

Destroys the HandlerIdle object and sets all ressources free.

*/

HandlerIdle::~HandlerIdle() 
{
    Shutdown();
}

/*
1.3 Initialize

Connects to the Coordinator to receive instructions about specilization.

The coordinationClient runs in a seperate thread and keeps running after the 
initialitation is done. It pushes incoming messages to its queue.

*/

void HandlerIdle::Initialize() 
{
    // start the client, received messages will be pushed to _MessageQueque
    _coordinationClient.Initialize();
    
    _coordinationClientThread = std::thread(
        &TcpClient::Receive, 
        &_coordinationClient
    );
    // _coordinationClientThread.detach();
    
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
        sendl = _coordinationClient.Send(IdleGenP::hello("idle", true));

        if (sendl != IdleGenP::hello("idle", true).length())
            LOG << "error while sending registerHandler message" << ENDL;
    } while (sendl != IdleGenP::hello("idle", true).length());

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
            // confirm the id and attrliststr set by the coordinator
            if (msg.cmd == CoordinatorGenP::confirmhello()) 
            {
                int handlerId = 0;
                std::string attrliststr = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, 
                    boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
                );

                try
                {
                    handlerId = std::stoi(parts[0]);
                    attrliststr = parts[1];
                }
                catch(...)
                {
                    handlerId = 0;
                    LOG << "failed to convert id to int" 
                        << " or receive the attribut list" << ENDL;
                }
                
                sendl = _coordinationClient.Send(
                    CoordinatorGenP::confirmhello(handlerId, attrliststr, true)
                );

                if (sendl == CoordinatorGenP::confirmhello(handlerId, 
                    attrliststr, true).length() && handlerId) 
                {
                    _id = handlerId;
                    _attrliststr = attrliststr;

                    LOG << "Set my ID to " << _id << ". Working with the "
                        << "Attribut list: " << _attrliststr << "." << ENDL;
                } else {
                    LOG << "Failed to confirm the hello message..."
                        << " probably shutting down soon." << ENDL;
                }
            } 

            // get a specialization
            if (msg.cmd == CoordinatorGenP::specialize()) 
            {
                // an id is neccessary for a specialization
                if (_id == 0) 
                {
                    LOG << "can't specialize without an id" << ENDL;
                    continue;
                }

                // become a loop worker
                if (msg.params.substr(0, 11) == "worker|loop")
                {
                    LOG << "becoming a loop worker" << ENDL;

                    WorkerLoop worker(_id, _attrliststr, &_coordinationClient);
                    
                    // worker.WorkerLoop::Initialize();
                    worker.WorkerGen::Initialize();

                    noSpecilization = false;
                }

                // become a join worker
                else if (msg.params.substr(0, 11) == "worker|join")
                {
                    WorkerJoin worker(_id, _attrliststr, &_coordinationClient, 
                        msg.params.substr(12)
                    );
                    LOG << "becoming a join worker for " 
                        << msg.params.substr(12) << ENDL;

                    worker.WorkerJoin::Initialize();
                    worker.WorkerGen::Initialize();

                    noSpecilization = false;
                } 

                // become a notification and monitoring handler
                else if (msg.params == "nomo")
                {
                    NoMo nomo(_id, _attrliststr, &_coordinationClient);

                    nomo.Initialize();

                    noSpecilization = false;
                }

                // unknown handler
                else
                {
                    std::cout << "No idea what a " << msg.params 
                        << " handler is.\n";
                }
            }

            // force shutdown
            else if (msg.cmd == CoordinatorGenP::shutdown() || 
                msg.cmd == "disconnected") 
            {
                LOG << "shutting down due to " << msg.cmd 
                    << " " << msg.params << ENDL;
                
                noSpecilization = false;
            } 

            // unknown command
            else 
            {
                LOG << "No handler for command " << msg.cmd << ENDL;
            }

        } else {
            LOG << "Message '" + msg.cmd + "' is invalid..." << ENDL;
        }   
    }

    LOG << "Ende" << ENDL;
    // Shutdown();
}

/*
1.4 Shutdown

Shut down the idle handler.

*/

void HandlerIdle::Shutdown() 
{
    LOG << "IdleHandler " << _id << " is shutting down ";
    _coordinationClient.Shutdown();
    LOG << "... _cC has shut down ";
    _coordinationClientThread.join();
    LOG << "... _cCT has joined. Finished! " << ENDL;
}

}