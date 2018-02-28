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
For detailed information refer to ~DistributeStreamServer.cpp~.

*/
#ifndef _DISTRIBUTE_STREAM_SERVER_H_
#define _DISTRIBUTE_STREAM_SERVER_H_

#include <iostream>
#include <queue>
#include <list>

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "SocketIO.h"

#include "Algebras/Distributed2/Server.h"
#include "DistributeStreamWorker.h"

class Socket;
class VTuple;
class Server;
class DistributeStreamWorker;

namespace cstream {

class DistributeStreamServer : public distributed2::Server {

    public:
        DistributeStreamServer( const int port, 
                                const bool providedformat);

        ~DistributeStreamServer();

        int start();

        void run();

        void pushTuple(VTuple* tuple);

    private:

    void startCommunication(Socket* serverConnection);
/*
2 Member Definitions

2.2 providedFormat

Stores the format that is provided by the server. The client can only request 
the provided format. True is the binary format and false is the text format.

*/
        const bool _providedFormat;

/*
2.3 runner

Stores a pointer to the created thread of the server to handle new client 
connections.

*/
        boost::thread* _runner;

/*
2.4 socketBuffer

This queue stores the sockets of client connections to be popped for further
processing.

*/
        std::queue<Socket*> _socketBuffer;

/*
2.5 queueGuard

This mutex takes care that only one thread can access the socketBuffer
at a time.

*/
        boost::mutex _workerListGuard;

/*
2.6 workerList

This is a list is a complete list of all workers. The workers handle the 
communication with the connected clients. This list is used to notify all 
workers when a new tuple is pushed.

*/
        std::list<DistributeStreamWorker*> _workerList;
};

} /* namespace cstream */

#endif // _DISTRIBUTE_STREAM_SERVER_H_