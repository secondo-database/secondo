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

[1] Implementation of DistributeStreamServer.

[toc]

1 DistributeStreamServer class implementation

2 Includes


*/
#include "StringUtils.h"
#include "Algebras/DBService/CommunicationUtils.hpp"

#include "DistributeStreamProtocol.h"
#include "VTHelpers.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "DistributeStreamServer.h"

using namespace std;

namespace cstream {

/*
1.1 Constructor

Creates a server with a given port number and a given provided format. 
If provided format is set to true the binary format is provided. If 
the variable is false, the text format is provieded.

*/
DistributeStreamServer::DistributeStreamServer(
    const int port, const bool providedFormat) :
        Server(port), _providedFormat(providedFormat), _runner(0) {

}

/*
1.2 Destructor

Stoppes the server and all running workers.

*/
DistributeStreamServer::~DistributeStreamServer() {
    if(_runner) {
        _runner->interrupt();
        if(listener->IsOk()) {
            listener->ShutDown();
        }
        _runner->join();
    }

    boost::unique_lock<boost::mutex> lock(_workerListGuard);
    for(list<DistributeStreamWorker*>::iterator it=_workerList.begin() ; 
            it != _workerList.end() ; ++it)
        delete (*it);
}

/*
1.3 Function Definitions

1.3.1 run

Starts the server in a separate thread to wait for new client connections.
For a new communication with a client a worker thread will be started.

*/
void DistributeStreamServer::run() {
    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    _runner = new boost::thread(boost::bind(
            &DistributeStreamServer::start,
            this));
}

/*
1.3.2 start

This function runs in a separate thread and waits for connection requests.
If a client connect to the server the communication will be done in a
worker thread.

*/
int DistributeStreamServer::start() {
    if (!listener->IsOk()) {
        return 1;
    }

    while(true) {
        try {
            // wait for incoming connections and pass 
            // them to a new worker thread
            Socket* serverConnection = listener->Accept();
            if (!serverConnection || !serverConnection->IsOk()) {
                if(listener)
                    listener->ShutDown();
                return 2;
            }

            startCommunication(serverConnection);
        }
        catch (boost::thread_interrupted) {
            return 0;
        }
    }

    return 0;
}

/*
1.3.3 handleCommunicationThread

This function is used to start a new worker for a new client connection.

*/
void DistributeStreamServer::startCommunication(Socket* serverConnection) {
    DistributeStreamWorker* worker = 
        new DistributeStreamWorker(serverConnection, _providedFormat);
    if(worker->run()) {
        boost::unique_lock<boost::mutex> lock(_workerListGuard);
        _workerList.push_back(worker);
    }
}

/*
1.3.4 pushTuple

This function pushes a given vtuple to all connected workers.

*/
void DistributeStreamServer::pushTuple(VTuple* vt) {
    boost::unique_lock<boost::mutex> lock(_workerListGuard);
    queue<DistributeStreamWorker*> deadWorkers;
    for(list<DistributeStreamWorker*>::iterator it=_workerList.begin() ; 
            it != _workerList.end() ; ++it) {

        if((*it)->isConnected()) {
            vt->IncReference();
            (*it)->pushTuple(vt);
        }
        else {
            deadWorkers.push((*it));
        }
    }

    // delete workers without a connection
    while(!deadWorkers.empty()) {
        DistributeStreamWorker* deadWorker = deadWorkers.front();
        deadWorkers.pop();
        _workerList.remove(deadWorker);
        delete deadWorker;
    }

    vt->DeleteIfAllowed();
}

} /* namespace cstream */
