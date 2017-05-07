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


//[$][\$]
//[_][\_]

*/
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/ReplicationServer.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

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
    T server(port);
    server.start();
}

template void ServerRunnable::createServer<CommunicationServer>(int port);
template void ServerRunnable::createServer<ReplicationServer>(int port);

template <typename T>
void ServerRunnable::run()
{
    if(runner){
       runner->join();
       delete runner;
    }
    runner = new boost::thread(boost::bind(
            &ServerRunnable::createServer<T>,
            this,
            port));
}

template void ServerRunnable::run<CommunicationServer>();
template void ServerRunnable::run<ReplicationServer>();



} /* namespace DBService */
