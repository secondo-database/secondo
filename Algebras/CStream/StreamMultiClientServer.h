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
For detailed information refer to ~StreamMultiClientServer.cpp~.

2 Defines and includes


*/

#ifndef _STREAM_MULTI_CLIENT_SERVER_H_
#define _STREAM_MULTI_CLIENT_SERVER_H_

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "SocketIO.h"

#include "Algebras/Distributed2/Server.h"

class Socket;

namespace cstream {

/*

1.1 StreamMultiClientServer

The StreamMultiClientServer can be used as superclass for servers in order to
enable them to handle multiple incoming client connections at once. Working
with threads.

*/

class StreamMultiClientServer : public distributed2::Server {
/*

1.1.1 Function Definitions

The functions provided by the StreamMultiClientServer class are explained
below.

1.1.1.1 Constructor

*/
public:
    explicit StreamMultiClientServer(
        const int port);
/*

1.1.1.2 Destructor

*/
    virtual ~StreamMultiClientServer();

/*

1.1.1.3  start

This function starts the server.

*/
    int start();

/*

1.1.1.4 stop

This function stop set the member stopped to true. The server stops
and terminates all threads and closes all connections.

*/
    void stop();

/*

1.1.1.5 run

This function starts the StreamMultiClientServer in a
separate thread.

*/
    void run();

/*

1.1.1.6 communicate

The communicate function needs to be overwritten by the respective subclass. It
shall provide the main functionality of the respective server. This function
includes the communication of the worker threads with the client.

*/
protected:
    virtual int communicate(std::iostream& io) = 0;

/*

1.1.1.7 handleCommunicationThread

This function is called within a new thread. It pops a socket from the queue
and processes it by calling the ~communicate~ function of the respective
subclass.

*/
    bool handleCommunicationThread(
        boost::thread_group & threads, boost::thread * thisThread);

/*

1.1.1.8 stopServer

This function stops the server and terminates all threads and
closes the connections and sockets.

*/
    void stopServer();

/*

1.1.1.9 startWorker

Starts a new worker thread. Thread using function handleCommunicationThread.

*/
    void startWorker(boost::thread_group &threads);

/*

1.1.2 Member Definitions

*/
private:

/*

1.1.2.1 runner

Stores a pointer to the created thread.

*/
    boost::thread* _runner;

/*

1.1.2.2 stopped

Stores a false if the server is running.

*/
    bool _stopped;


/*

1.1.2.3 socketBuffer

This queue stores the sockets of client connections to be popped for further
processing.

*/
    std::queue<Socket*> _socketBuffer;

/*

1.1.2.4 queueGuard

This mutex takes care that only one thread can access the socketBuffer
at a time.

*/
    boost::mutex _queueGuard;

/*

1.1.2.5 stoppedGuard

This mutex takes care that only one thread can change or read the variable
stopped at a time.

*/
    boost::mutex _stoppedGuard;

};

} /* namespace cstream */

#endif // _STREAM_MULTI_CLIENT_SERVER_H_