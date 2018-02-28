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

[1] Implementation of datatype StreamMultiClientServer and operators.

[toc]

1 StreamMultiClientServer class implementation

2 Defines and includes


*/
#include "StringUtils.h"

#include "StreamMultiClientServer.h"
#include "VTHelpers.h"

namespace cstream {

/*
1.1 StreamMultiClientServer

Creates a server with a given port number and a maximum number of worker
threads. The server is in the stopped status.

*/
StreamMultiClientServer::StreamMultiClientServer(
    const int port) :
        Server(port), _runner(0), _stopped(true) {

}

/*
1.2 ~StreamMultiClientServer~

Stoppes the server

*/
StreamMultiClientServer::~StreamMultiClientServer() {
    stopServer();
}

/*
1.3 run

Starts the server in a separate thread.

*/
void StreamMultiClientServer::run() {
    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    _stoppedGuard.lock();
    _stopped = false;
    _stoppedGuard.unlock();
    _runner = new boost::thread(boost::bind(
            &StreamMultiClientServer::start,
            this));
}

/*
1.4 run

This function runs in a separate thread and waits for connection requests.
If a client connect to the server the communication will be done in a
worker thread.

*/
int StreamMultiClientServer::start() {
    if (!listener->IsOk()) {
        return 1;
    }
    
    boost::thread_group threads;

    while(true) {
        // wait for incoming connections and pass them to a new worker thread
        LOG << "listener->Accept" << ENDL;
        Socket* serverConnection = listener->Accept();
        LOG << "wait for stop lock" << ENDL;
        _stoppedGuard.lock();
        if (_stopped || !serverConnection->IsOk()) {
            _stoppedGuard.unlock();

            LOG << "stop all threads" << ENDL;
            threads.join_all(); // wait for all threads to finish

            LOG << "interrupt all complete" << ENDL;
            if(listener)
                listener->ShutDown();
            return 2;
        }
        _stoppedGuard.unlock();
        boost::unique_lock<boost::mutex> lock(_queueGuard);
        _socketBuffer.push(serverConnection);

        startWorker(threads);
    }

    return 0;
}

/*
1.5 handleCommunicationThread

This function is used as a worker thread.

*/
bool StreamMultiClientServer::handleCommunicationThread(
    boost::thread_group & threads, boost::thread * thisThread) {

    boost::unique_lock<boost::mutex> lock(_queueGuard);

    assert(!_socketBuffer.empty());

    Socket* server = _socketBuffer.front();
    _socketBuffer.pop();

    lock.unlock();
    communicate(server->GetSocketStream());
    server->Close();
    LOG << "communication end" << ENDL;

    _stoppedGuard.lock();
    if(!_stopped)
        threads.remove_thread(thisThread);
    _stoppedGuard.unlock();

    return true;
}

/*
1.6 stopServer

This function stops the server.

*/
void StreamMultiClientServer::stopServer() {
    if(_runner) {
        LOG << "stop lock" << ENDL;
        _stoppedGuard.lock();
        _stopped = true;
        _stoppedGuard.unlock();
        LOG << "stop unlock" << ENDL;
        listener->ShutDown();
        LOG << "shutdown ok" << ENDL;
        LOG << "listener close" << ENDL;
        _runner->join();
        LOG << "runner join" << ENDL;
    }
    if(listener) {
        listener->Close();
    }
}

/*
1.7 startWorker

Starts a new worker thread.

*/
void StreamMultiClientServer::startWorker(boost::thread_group &threads) {
    // start a new thread
    boost::thread *worker = new boost::thread();
    // thread needs to now the group and the thread to remove the thread 
    // from the group after finisch
    *worker = boost::thread(
        boost::bind(
            &StreamMultiClientServer::handleCommunicationThread, this, 
            boost::ref(threads), worker));
    threads.add_thread(worker);
}

} /* namespace cstream */