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

[1] Implementation of the general worker.

[toc]

1 WorkerGen class implementation

*/

#include "WorkerGen.h"
#include <boost/algorithm/string.hpp>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace continuousqueries {

/*
1.1 Constructor

Creates a new WorkerGen object.

*/

WorkerGen::WorkerGen(int id, std::string attrliststr, 
    TcpClient* coordinationClient):
    _coordinationClient(coordinationClient),
    _id(id),
    _attrliststr(attrliststr),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id))
{
}

// Destroy
WorkerGen::~WorkerGen()
{
    Shutdown();
}

// Initialize
void WorkerGen::Initialize()
{
    // start tuple server thread
    _tupleServerThread = std::thread(
        &TcpServer::Run, 
        &_tupleServer
    );

    // wait for the server to be started
    if (!_tupleServer.IsRunning()) 
    {
        int count = 0;
        LOG << "Waiting a maximum of 60 seconds for the tuple"
            << " receiving server to start... " << ENDL;

        while (!_tupleServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_tupleServer.IsRunning()) LOG << " Done!" << ENDL;
    }

    if (!_tupleServer.IsRunning()) return;

    std::cout << "Done! Stream supplier have to push tuple "
        << "to this host on port "
        << std::to_string(_tupleServer.GetMasterPort()) << ". \n";

    _running = true;

    // start tight loop in thread
    _tightLoopThread = std::thread(
        &WorkerGen::TightLoop,
        this
    );

    // confirm specialization
    (void) _coordinationClient->Send(
        WorkerGenP::confirmspecialize(true)
    );

    Run();
}

// Run
void WorkerGen::Run()
{
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) 
    {
        std::unique_lock<std::mutex> lock(_coordinationClient->mqMutex);

        hasMsg = _coordinationClient->mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationClient->messages.empty();
        });

        if (!_running) {
            lock.unlock();
            continue;
        }
        
        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                _coordinationClient->messages.front()
            );
            _coordinationClient->messages.pop();
            
        } else {
            msg.valid = false;
        }

        lock.unlock();

        if (hasMsg && msg.valid) {
            // get a new nomo
            if (msg.cmd == CoordinatorGenP::addhandler()) 
            {
                int nomoId = 0;
                std::string handlerType = "";
                std::string nomoAddress = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    nomoId = std::stoi(parts[0]);
                    handlerType = parts[1];
                    nomoAddress = parts[2];
                }
                catch(...)
                {
                    nomoId = 0;
                    std::cout << "failed to extract id or address" << endl;
                }

                if (nomoId && handlerType=="nomo") addNoMo(nomoId, nomoAddress);
            } else

            // get a query
            if (msg.cmd == CoordinatorGenP::addquery(0, "", false)) 
            {
                int qId = 0;
                std::string function = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    qId = std::stoi(parts[0]);
                    function = parts[1];
                }
                catch(...)
                {
                    qId = 0;
                    std::cout << "failed to extract id or function" << endl;
                }

                if (qId) addQuery(qId, function);
            } else

            // show status
            if (msg.cmd == CoordinatorGenP::status())
            {
                showStatus();
            } else

            // force shutdown
            if (msg.cmd == CoordinatorGenP::shutdown() || 
                msg.cmd == "disconnected") 
            {
                std::cout << "shutting down due to " << msg.cmd 
                    << " " << msg.params << endl;
                
                _running = false;
            } 

            // unknown command
            else 
            {
                LOG << "No handler for command " << msg.cmd << "." << ENDL;
            }

        } else {
            if (hasMsg) 
            {
                LOG << "Message '" + msg.cmd + "' is invalid... " << ENDL;
            } else {
                // Timeout
            }
        }   
    }
}

void WorkerGen::notifyAllNoMos(int tupleId, std::string tupleString, 
    std::string hitlist)
{
    for (std::map<int, nomoStruct>::iterator it = _nomos.begin(); 
        it != _nomos.end(); it++)
    {
        it->second.ptrClient->SendAsync(
            WorkerGenP::hit(tupleId, tupleString, hitlist, true)
        );
    }
}

// NoMo handling
void WorkerGen::addNoMo(int id, std::string address)
{
    LOG << "Adding NoMo " << id << "|" << address << ":" 
        << _basePort + (id) << ENDL;

    nomoStruct toAdd;
    toAdd.id = id;
    toAdd.port = _basePort + (id);
    toAdd.address = address;
    toAdd.ptrClient = new TcpClient(address, _basePort + (id));

    toAdd.ptrClient->Initialize();
    
    _nomoThreads.push_back(std::thread(
        &TcpClient::AsyncHandler, 
        toAdd.ptrClient
    ));

    _nomos.insert( std::pair<int, nomoStruct>(id, toAdd));
}

void WorkerGen::deleteNoMo(int id)
{}

// Shutdown
void WorkerGen::Shutdown()
{
    _running = false;

    _tupleServer.Shutdown();
    _tupleServerThread.join();
    _tightLoopThread.join();
    
    for (std::map<int, nomoStruct>::iterator it = _nomos.begin(); 
        it != _nomos.end(); it++)
    {
        it->second.ptrClient->Shutdown();
    }

    for (unsigned i=0; i < _nomoThreads.size(); i++) {
        _nomoThreads[i].join();
    }
}

}