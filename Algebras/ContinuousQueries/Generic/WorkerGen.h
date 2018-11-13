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

[1] Implementation of the general Worker

The Worker receives the queries from the Coordinator and tuples from the
stream supplier. After checking all queries with the tuple, the nomo handler
is informed.

[toc]

1 WorkerGen class implementation
see WorkerGen.cpp for details.

*/

#ifndef __WORKERGEN_H__
#define __WORKERGEN_H__

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

class WorkerGen {

  public:
    // Create
    WorkerGen(int id, std::string attrliststr, TcpClient* coordinationClient);

    // Destroy
    ~WorkerGen();

    struct nomoStruct {
        int id;
        std::string address;
        int port;
        TcpClient* ptrClient;
    };

    // Initialize
    virtual void Initialize();

    // Run
    void Run();

        // The Loop
        virtual void TightLoop() = 0;

        // Query handling
        virtual void addQuery(int id, std::string function) = 0;

        // Show status
        virtual void showStatus() = 0;

    // NoMo handling
    void addNoMo(int id, std::string address);
    void deleteNoMo(int id);
    void notifyAllNoMos(int tupleId, std::string tupleString, 
        std::string hitlist);

    // Shutdown
    void Shutdown();

  protected:
    TcpClient* _coordinationClient;
    int _id;
    std::string _attrliststr;
    std::string _type;
    int _basePort;

    bool _running;

    QueryProcessor* _qp;

    TcpServer _tupleServer;
    std::thread _tupleServerThread;
    std::thread _tightLoopThread;

    std::map<int, nomoStruct> _nomos;
    std::vector<std::thread> _nomoThreads;
};

}
#endif