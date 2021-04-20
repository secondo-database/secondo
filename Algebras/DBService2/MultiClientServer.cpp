/*
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
#include "StringUtils.h"

#include "Algebras/DBService2/MultiClientServer.hpp"

#include <loguru.hpp>

namespace DBService {


MultiClientServer::MultiClientServer(int port) :
        Server(port)
{}

MultiClientServer::~MultiClientServer()
{}

int MultiClientServer::start()
{
    LOG_SCOPE_FUNCTION(INFO);

    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    if (!listener->IsOk())
    {
        return 1;
    }

    const size_t MAX_THREADS = 256; // 32; // 256
    LOG_F(INFO, "%s", "Building thread pool...");

    // Build a thread pool to respond to incoming communication requests
    boost::thread_group threads;
    for(size_t i = 0; i < MAX_THREADS; i++)
    {
        // LOG_F(INFO, "Creating thread nr %d...", i);
        threads.create_thread(boost::bind(
                &MultiClientServer::handleCommunicationThread, this));
    }

    LOG_F(INFO, "%s", 
        "Waiting for incoming connections to pass them to workers...");

    while(true)
    {
        // wait for incoming connections and pass them to the worker threads
        Socket* serverConnection = listener->Accept();
        if (!serverConnection->IsOk())
        {
            return 2;
        }

        LOG_F(INFO, "%s", "Acquiring MultiClientServer lock...");
        boost::unique_lock<boost::mutex> lock(queueGuard);
        LOG_F(INFO, "%s", "Successfully acquired MultiClientServer lock.");

        socketBuffer.push(serverConnection);

        LOG_F(INFO, "%s", "queueIndicator.notify_one()...");
        queueIndicator.notify_one();
        LOG_F(INFO, "%s", "Done: queueIndicator.notify_one();");

        boost::this_thread::sleep_for(boost::chrono::milliseconds(5));
    }

    LOG_F(INFO, "%s", "Interrupting all threads a queueGaurd.wait()...");

    threads.interrupt_all(); // interrupt the threads (at queueGuard.wait())

    LOG_F(INFO, "%s", "Joining all threads. Waiting for them to finish...");
    threads.join_all(); // wait for all threads to finish

    LOG_F(INFO, "%s", "Closing listener...");

    listener->Close();

    LOG_F(INFO, "%s", "Done.");
    return 0;
}

bool MultiClientServer::handleCommunicationThread()
{
    LOG_SCOPE_FUNCTION(INFO);

    while(true)
    {
        //boost::this_thread::get_id();
        try
        {
            LOG_F(INFO, "%s", "Acquiring MultiClientServer lock...");
            boost::unique_lock<boost::mutex> lock(queueGuard);
            LOG_F(INFO, "%s", "Successfully acquired MultiClientServer lock.");
            LOG_F(INFO, "%s", "Now waiting for the lock...");
            queueIndicator.wait(lock);
            LOG_F(INFO, "%s", "Done waiting for the lock.");
        }
        catch (boost::thread_interrupted)
        {
            LOG_F(ERROR, "%s", "Thread has been interrupted!");
            return false;
        }

        LOG_F(INFO, "%s", "Acquiring MultiClientServer lock again...");
        boost::unique_lock<boost::mutex> lock(queueGuard);

        assert(!socketBuffer.empty());

        Socket* server = socketBuffer.front();
        socketBuffer.pop();

        LOG_F(INFO, "%s", "Unlocking MultiClientServer...");
        lock.unlock();

        LOG_F(INFO, "%s", "Handling the communication request...");
        communicate(server->GetSocketStream());

    }
}


} /* namespace DBService */
