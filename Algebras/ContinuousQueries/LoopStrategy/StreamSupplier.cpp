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

[1] Implementation of the stream supplier.

[toc]

1 StreamSupplier class implementation

*/

#include "StreamSupplier.h"
#include <boost/algorithm/string.hpp>

namespace continuousqueries {

/*
1.1 Constructor

Creates a new StreamSupplier object.

*/

StreamSupplier::StreamSupplier(std::string address, int port): 
    _coordinatorAddress(address),
    _coordinatorPort(port),
    _id(0),
    _coordinationClient(address, port),
    _lastTupleId(0)
{
}

/*
1.2 Destructor

Destroys the StreamSupplier object and sets all ressources free.

*/

StreamSupplier::~StreamSupplier() {
    Shutdown();
    // TODO: Verbindungen lösen, etc.
}

/*
1.4 Initialize

Connects to the Coordinator to receive instructions about specilization.

The coordinationClient runs in a seperate thread and keeps running after the 
initialitation is done. It pushes incoming messages to its queue.

*/

void StreamSupplier::Initialize()
{
    _ownThread = std::thread(
        &StreamSupplier::Run, 
        this
    );

    // wait for the connection to be established
    if (!_id)
    {
        int count = 0;
        std::cout << "Waiting a maximum of 60 seconds for ID.";
        
        while ((!_id) && (count < (60*1000))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << ".";
            count = count + 100;
        }
        if (_id)
        {
            std::cout << " Done!\n";
        } else {
            Shutdown();
        }
    }
}


void StreamSupplier::Run() 
{
    _running = false;

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
        while ((!_coordinationClient.IsRunning()) and (count < (60*1000))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << ".";
            count = count + 100;
        }
        if (_coordinationClient.IsRunning()) LOG << " Done!" << ENDL;
    }

    if (!_coordinationClient.IsRunning()) return;

    // send Hello message to the Coordinator
    size_t sendl;
    do
    {
        sendl = _coordinationClient.Send(
            StSuGenP::hello(true)
        );

        if (sendl != StSuGenP::hello(true).length())
            LOG << "error while sending registerHandler message" << ENDL;
    } while (sendl != StSuGenP::hello(true).length());

    // wait for the id
    bool hasId = false;
    bool hasMsg = false;
    _running = true;

    ProtocolHelpers::Message msg;

    while (_running) {
        LOG << "BEFORE LOCK _cC" << ENDL;
        std::unique_lock<std::mutex> lock(_coordinationClient.mqMutex);

        hasMsg = _coordinationClient.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationClient.messages.empty();
        });
        LOG << "AFTER LOCK _cC: " << hasMsg << "!" << ENDL;

        if (!_running) {
            LOG << "!_running-->continue" << ENDL;
            lock.unlock();
            continue;
        }

        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                _coordinationClient.messages.front()
            );
            _coordinationClient.messages.pop();
        } else {
            msg.valid = false;
        }
        
        lock.unlock();

        if (hasMsg && msg.valid) {
            // get the id and tupledescr from the coordinator and confirm it
            if (msg.cmd == CoordinatorGenP::confirmhello()) 
            {
                if (hasId) 
                {
                    LOG << "ID already set!" << ENDL;
                    continue;
                }

                int handlerId = 0;
                std::string tupledescr;
                
                std::vector<std::string> parts;

                boost::split(parts, msg.params, 
                    boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
                );

                try
                {
                    handlerId = std::stoi(parts[0]);
                    tupledescr= parts[1];
                }
                catch(...)
                {
                    LOG << "failed to convert id to int" << ENDL;
                }
                
                sendl = _coordinationClient.Send(
                    CoordinatorGenP::confirmhello(handlerId, tupledescr, true)
                );

                if (
                    sendl == CoordinatorGenP::confirmhello(handlerId, 
                        tupledescr, true).length() && handlerId) 
                {
                    std::cout << "Set my ID to " << handlerId << ". \n";
                    _id = handlerId;
                    hasId = true;
                } else {
                    std::cout << "error while sending "
                        << "confirmHandler message \n";
                }
            }

            // get a new worker
            else if ((msg.cmd == CoordinatorGenP::addhandler()) and (hasId)) 
            {
                int workerId = 0;
                std::string handlerType = "";
                std::string workerAddress = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, 
                    boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
                );
                
                try
                {
                    workerId = std::stoi(parts[0]);
                    handlerType = parts[1];
                    workerAddress = parts[2];
                }
                catch(...)
                {
                    workerId = 0;
                    std::cout << "failed to extract id or address \n";
                }

                if (workerId && handlerType=="worker") 
                    addWorker(workerId, workerAddress);
            }

            // force shutdown
            else if (msg.cmd == CoordinatorGenP::shutdown() || 
                msg.cmd == "disconnected") 
            {
                std::cout << "shutting down due to " << msg.cmd 
                    << " " << msg.params << "\n";
                
                _running = false;
            } 

            // unknown command
            else 
            {
                std::cout << "Waiting for ID or no handler " 
                    << " for command " << msg.cmd << ".\n";
            }

        } else {
            if (hasMsg) 
            {
                std::cout << "Message '" + msg.cmd + "' is invalid... \n";
            } else {
                std::cout << "No Message. Timeout... \n";
            }
        }   
    }
}

void StreamSupplier::pushTuple(Tuple* t)
{
    if (_workers.size() == 0) 
        std::cout << "No worker connected. Tuple is lost. \n";
    
    std::string msg = StSuGenP::tuple(
        ++_lastTupleId, 
        t->WriteToBinStr(), 
        true
    );

    for (std::map<int, workerStruct>::iterator it = _workers.begin(); 
        it != _workers.end(); it++)
    {   
        std::cout << "Sending to worker with ID " << it->first << ". \n";

        it->second.ptrClient->SendAsync(
            msg
        );
    }
}

void StreamSupplier::addWorker(int id, std::string address)
{
    std::cout << id << "|" << address 
              << ":" << _coordinatorPort+(id*10) << "\n";

    workerStruct toAdd;
    toAdd.id = id;
    toAdd.port = _coordinatorPort + (id * 10);
    toAdd.address = address;
    toAdd.ptrClient = new TcpClient(address, _coordinatorPort + (id * 10));

    toAdd.ptrClient->Initialize();

    _workerThreads.push_back(std::thread(
        &TcpClient::AsyncHandler, 
        toAdd.ptrClient
    ));

    _workers.insert( std::pair<int, workerStruct>(id, toAdd));
}

void StreamSupplier::Shutdown() {
    _running = false;
    
    _coordinationClient.Shutdown();
    _coordinationClientThread.join();
    _ownThread.join();

    for (std::map<int, workerStruct>::iterator it = _workers.begin(); 
        it != _workers.end(); it++)
    {
        it->second.ptrClient->Shutdown();
    }

    for (unsigned i=0; i < _workerThreads.size(); i++) {
        _workerThreads[i].join();
    }
}

}