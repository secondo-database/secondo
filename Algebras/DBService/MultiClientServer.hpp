/*

1 Inter-Node Communication and File Transfer

----
This file is part of SECONDO.

Copyright (C) 2017,
Faculty of Mathematics and Computer Science,
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

*/
#ifndef ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_
#define ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "SocketIO.h"

#include "Algebras/Distributed2/Server.h"

#include "Algebras/DBService/TraceWriter.hpp"

class Socket;

namespace DBService {

/*

1.1 \textit{MultiClientServer}

The \textit{MultiClientServer} can be used as superclass for servers in order to
enable them to handle multiple incoming client connections at once.

*/

class MultiClientServer : public distributed2::Server {
/*

1.1.1 Function Definitions

The functions provided by the \textit{MultiClientServer} class are explained
below.

1.1.1.1 Constructor

*/
public:
    explicit MultiClientServer(int port);
/*

1.1.1.1 Destructor

*/
    virtual ~MultiClientServer();

/*

1.1.1.1 \textit{start}

This function starts the \textit{MultiClientServer}.

*/
    int start();
/*

1.1.1.1 \textit{communicate}

The communicate function needs to be overwritten by the respective subclass. It
shall provide the main functionality of the respective server.

*/
protected:
    virtual int communicate(std::iostream& io) = 0;

/*

1.1.1.1 \textit{handleCommunicationThread}

This function is called within a new thread. It pops a socket from the queue
and processes it by calling the \textit{communicate} function of the respective
subclass.

*/
    bool handleCommunicationThread();

/*

1.1.1.1 \textit{traceWriter}

As the \textit{DBService} acts in a highly distributed environment, it
is important to provide comprehensive tracing in order to be able to understand
the behaviour of the individual components which is important for potential
error analysis. Therefore, each server holds its own instance of
a \textit{TraceWriter} object which documents all important events in a trace
file.

*/
    std::unique_ptr<TraceWriter> traceWriter;

/*

1.1.1 Member Definitions

1.1.1.1 \textit{socketBuffer}

This queue stores the sockets of client connections to be popped for further
processing.

*/
private:
    std::queue<Socket*> socketBuffer;

/*

1.1.1.1 \textit{queueGuard}

This mutex takes care that only one thread can access the \textit{socketBuffer}
at a time.

*/
    boost::mutex queueGuard;

/*

1.1.1.1 \textit{queueIndicator}

This semaphore is used to give a notification when a new client socket has been
added to the queue.

*/
    boost::condition_variable queueIndicator;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_MULTICLIENTSERVER_HPP_ */
