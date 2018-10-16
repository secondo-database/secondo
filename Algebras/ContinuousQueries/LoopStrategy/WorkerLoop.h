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

[1] Implementation of the Worker type: Loop

The Worker receives the queries from the Coordinator and tuples from the
stream supplier. After checking all queries with the tuple, the nomo handler
is informed.

[toc]

1 WorkerLoop class implementation
see WorkerLoop.cpp for details.

*/

#ifndef __WORKERLOOP_H__
#define __WORKERLOOP_H__

#include "../Tcp/TcpClient.h"
#include "../Tcp/TcpServer.h"
#include "../Protocols.h"

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace continuousqueries {

class WorkerLoop {

public:
    // Create
    WorkerLoop(int id, TcpClient* coordinationClient);

    // Destroy
    ~WorkerLoop();

    struct nomoStruct {
        int id;
        std::string address;
        int port;
        TcpClient* ptrClient;
        // std::thread asyncThread;
    };

    struct queryStruct {
        int id;
        std::string funText;
        ArgVectorPointer funargs;
        OpTree tree;
        QueryProcessor* qp;
    };

    // Initialize
    void Initialize();

    // Run
    void Run();

    // The Loop
    void TightLoop();
    bool filterTuple(Tuple* tuple, OpTree& tree, 
        ArgVectorPointer& funargs, QueryProcessor* qp);

    // NoMo handling
    void addNoMo(int id, std::string address);
    void deleteNoMo(int id);
    void notifyAllNoMos(std::string tuple, std::string hitlist);

    // Query handling
    void addQuery(int id, std::string function);

    // Shutdown
    void Shutdown();

private:
    TcpClient* _coordinationClient;
    int _id;
    int _basePort;

    bool _running;

    QueryProcessor* _qp;

    TcpServer _tupleServer;
    std::thread _tupleServerThread;
    std::thread _tightLoopThread;

    std::map<int, nomoStruct> _nomos;
    std::map<int, queryStruct> _queries;
};

}
#endif