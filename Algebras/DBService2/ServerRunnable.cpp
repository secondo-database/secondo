/*

1.1.1 Class Implementation

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
#include "Algebras/DBService2/CommunicationServer.hpp"
#include "Algebras/DBService2/ReplicationServer.hpp"
#include "Algebras/DBService2/ServerRunnable.hpp"

#include <loguru.hpp>

using namespace distributed2;

namespace DBService {

ServerRunnable::ServerRunnable(int serverPort)
: runner(0), port(serverPort)
{}

ServerRunnable::~ServerRunnable()
{
 /*   if(runner){
        runner->join();
        delete runner;
    }*/
}

template <typename T>
void ServerRunnable::createServer(int port)
{
    LOG_SCOPE_FUNCTION(INFO);
    T server(port);
    server.start();
}

template void ServerRunnable::createServer<CommunicationServer>(int port);
template void ServerRunnable::createServer<ReplicationServer>(int port);

template <typename T>
void ServerRunnable::run()
{
    LOG_SCOPE_FUNCTION(INFO);
    if(runner){
        LOG_F(INFO, "%s", "Waiting for runner to joind...");
        runner->join();
        LOG_F(INFO, "%s", "Runners has joined. Deleting it.");
        delete runner;
    }

    LOG_F(INFO, "%s", "Creating new Server thread...");
    runner = new boost::thread(boost::bind(
            &ServerRunnable::createServer<T>,
            this,
            port));
}

template void ServerRunnable::run<CommunicationServer>();
template void ServerRunnable::run<ReplicationServer>();



} /* namespace DBService */
