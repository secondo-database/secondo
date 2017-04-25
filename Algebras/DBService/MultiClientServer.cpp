/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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


//[$][\$]
//[_][\_]


*/
#include "StringUtils.h"

#include "Algebras/DBService/MultiClientServer.hpp"

namespace DBService {


MultiClientServer::MultiClientServer(int port) :
        Server(port)
{}

MultiClientServer::~MultiClientServer()
{}

int MultiClientServer::start()
{
    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    if (!listener->IsOk())
    {
        return 1;
    }

    const size_t MAX_THREADS = 4;

    boost::thread_group threads;
    for(size_t i = 0; i < MAX_THREADS; i++)
    {
        threads.create_thread(boost::bind(
                &MultiClientServer::handleCommunicationThread, this));
    }

    while(true)
    {
        // wait for incoming connections and pass them to the worker threads
        Socket* serverConnection = listener->Accept();
        if (!serverConnection->IsOk())
        {
            return 2;
        }
        boost::unique_lock<boost::mutex> lock(queueGuard);
        socketBuffer.push(serverConnection);
        queueIndicator.notify_one();
    }

    threads.interrupt_all(); // interrupt the threads (at queueGuard.wait())
    threads.join_all(); // wait for all threads to finish

    listener->Close();
    return 0;
}

bool MultiClientServer::handleCommunicationThread()
{
    while(true)
    {
        try
        {
            boost::unique_lock<boost::mutex> lock(queueGuard);
            queueIndicator.wait(lock);
        }
        catch (boost::thread_interrupted)
        {
            return false;
        }

        boost::unique_lock<boost::mutex> lock(queueGuard);

        assert(!socketBuffer.empty());

        Socket* server = socketBuffer.front();
        socketBuffer.pop();

        lock.unlock();

        communicate(server->GetSocketStream());
    }
}


} /* namespace DBService */
