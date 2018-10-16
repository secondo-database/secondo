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
        std::cout << "Waiting a maximum of 60 seconds for Stream Supplier"
            << " and at least one worker.";
        
        while ((!_id) and (count < (60*1000)) and (_workers.size()==0)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << ".";
            count = count + 100;
        }
        if (_id) std::cout << " Done!\n";
    }
}


void StreamSupplier::Run() {
    _running = false;

    // start the client, received messages will be pushed to _MessageQueque
    _coordinationClient.Initialize();

    _coordinationClientThread = std::thread(
        &TcpClient::Receive, 
        &_coordinationClient
    );
    _coordinationClientThread.detach();
    
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
        if (_coordinationClient.IsRunning()) std::cout << " Done!\n";
    }

    if (!_coordinationClient.IsRunning()) return;

    // send Hello message to the Coordinator
    size_t sendl;
    do
    {
        sendl = _coordinationClient.Send(
            CgenToHidleP::registerHandler("streamsupplier", true)
        );

        if (sendl != CgenToHidleP::registerHandler("streamsupplier", true)
            .length())
            std::cout << "error while sending registerHandler message \n";
    } while (sendl != CgenToHidleP::registerHandler("streamsupplier", true)
        .length());

    // wait for the id
    bool hasId = false;
    _running = true;

    while (_running) {
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
            if ((msg.cmd == CgenToHidleP::confirmHandler()) and (!hasId)) 
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
                    hasId = true;
                } else {
                    std::cout << "error while sending "
                        << "confirmHandler message \n";
                }
            }

            // get a new worker
            else if ((msg.cmd == CgenToStSuP::addWorker()) and (hasId)) 
            {
                int workerId = 0;
                std::string workerAddress = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, 
                    boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
                );
                
                try
                {
                    workerId = std::stoi(parts[0]);
                    workerAddress = parts[1];
                }
                catch(...)
                {
                    workerId = 0;
                    std::cout << "failed to extract id or address \n";
                }

                if (workerId) addWorker(workerId, workerAddress);
            }

            // force shutdown
            else if (msg.cmd == CgenToHidleP::shutdownHandler() || 
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
            std::cout << "Message '" + msg.cmd + "' is invalid... \n";
        }   
    }

    Shutdown();
}

void StreamSupplier::pushTuple(Tuple* t)
{
    if (_workers.size() == 0) 
        std::cout << "No worker connected. Tuple is lost. \n";
    
    std::string msg = StSuToWgenP::tuple(
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

    std::thread t = std::thread(
        &TcpClient::AsyncHandler, 
        toAdd.ptrClient
    );

    t.detach();

    _workers.insert( std::pair<int, workerStruct>(id, toAdd));
}

void StreamSupplier::Shutdown() {
    // TODO: Verbindungen lösen, etc.
    _running = false;
    
    _coordinationClient.Shutdown();

    _ownThread.join();
}

}