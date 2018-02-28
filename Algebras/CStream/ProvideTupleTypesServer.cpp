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

[1] Implementation of class ProvideTupleTypesServer.

[toc]

1 ProvideTupleTypesServer implementation

*/
#include <iostream>
#include <string>
#include "Algebras/DBService/CommunicationUtils.hpp"

#include "ProvideTupleTypesServer.h"
#include "ProvideTupleTypesProtocol.h"
#include "VTHelpers.h"

namespace cstream {

/*
1.1 Constructor
With default cache size 1.

*/
ProvideTupleTypesServer::ProvideTupleTypesServer
    (const int port) :
        StreamMultiClientServer(port) {

    LOG << "ProvideTupleTypesServer::ProvideTupleTypesServer" << ENDL;
    LOG << "Initializing ProvideTupleTypesServer" << ENDL;
    LOG << port << ENDL;
    _cache = new LRUCache(1);
}

/*
1.2 Constructor
With a given cache size.

*/
ProvideTupleTypesServer::ProvideTupleTypesServer
    (const int port, const size_t cache) :
        StreamMultiClientServer(port) {

    LOG << "ProvideTupleTypesServer::ProvideTupleTypesServer" << ENDL;
    LOG << "Initializing ProvideTupleTypesServer" << ENDL;
    LOG << port << ENDL;
    _cache = new LRUCache(cache);
}

/*
1.2 Destructor

*/
ProvideTupleTypesServer::~ProvideTupleTypesServer() {
    LOG << "ProvideTupleTypesServer::~ProvideTupleTypesServer" << ENDL;
    stopServer();
    delete _cache;
}

/*
2 functions

2.1 start
Starts the server.

*/
int ProvideTupleTypesServer::start() {
    LOG << "ProvideTupleTypesServer::start" << ENDL;
    return StreamMultiClientServer::start();
}

/*
2.2 communicate
Communication of the worker threads with the connected clients.

*/
int ProvideTupleTypesServer::communicate(std::iostream& io) {
    LOG << "ProvideTupleTypesServer::communicate" << ENDL;
    //const boost::thread::id tid = boost::this_thread::get_id();
    int numOfTuples;
    std::string request;

    DBService::CommunicationUtils::receiveLine(io, request);
    LOG << "Query to the server: " << ENDL;
    LOG << request << ENDL;
    
    if(ProvideTupleTypesProtocol::requestTupleTypes(request, numOfTuples)) {
        LOG << "Request OK" << ENDL;
        LOG << "Number of Tuples requested:" << ENDL;
        LOG << numOfTuples << ENDL;

        _cache->sendCache(io, numOfTuples);

        DBService::CommunicationUtils::sendLine(io, 
                    ProvideTupleTypesProtocol::requestDone());
    }
    else {
        LOG << "Request not OK" << ENDL;
    }
    return 0;
}

/*
2.3 addTupleDescr
Add a ~tupledescr~ to the cache.

*/
void ProvideTupleTypesServer::addTupleDescr(TupleDescr* td) {
    _cache->insert(td->GetString());
}

};