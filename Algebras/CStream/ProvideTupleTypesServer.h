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

For detailed information refer to ~ProvideTupleTypesServer.cpp~.

*/

#ifndef _PROVIDE_TUPLE_TYPES_SERVER_H_
#define _PROVIDE_TUPLE_TYPES_SERVER_H_

#include "StreamMultiClientServer.h"
#include "LRUCache.h"
#include "TupleDescr.h"

/*

1.1 Class Definition

This class is a subclass of ~StreamMultiClientServer~

*/

namespace cstream {

    class ProvideTupleTypesServer : public StreamMultiClientServer {

    public:

/*
3.1 Constructors and Destructor

*/
            ProvideTupleTypesServer(
                const int port);
            ProvideTupleTypesServer(
                const int port, const size_t cache);
            ~ProvideTupleTypesServer();

/*
3.2 functions

3.2.1 start

Starts the server.

*/
            int start();

/*
3.2.2 addTupleDescr

Add a new ~tupledescr~ to the cache.

*/
            void addTupleDescr(TupleDescr* td);

        protected:
/*
3.2.4 communicate

This function includes the communication of the worker threads with the client.

*/
            int communicate(std::iostream& io);
        
        private:
/*
3.3 Members

3.3.1 cache

This is the cache for the TupleDescr.

*/
            LRUCache* _cache;
    };
};

#endif // _PROVIDE_TUPLE_TYPES_SERVER_H_