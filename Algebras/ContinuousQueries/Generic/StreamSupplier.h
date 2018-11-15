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

[1] Implementation of the Stream Supplier

The Stream Supplier pushes all tuple to the workers.
The Coordinator tells him which worker exist.

[toc]

1 StreamSupplier class implementation
see StreamSupplier.cpp for details.

*/

#ifndef __STREAMSUPPLIER_H__
#define __STREAMSUPPLIER_H__

#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "../Tcp/TcpClient.h"
#include "../Protocols.h"

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

namespace continuousqueries {

class StreamSupplier {

public:
    // Create
    StreamSupplier(std::string address, int port);

    // Destroy
    ~StreamSupplier();

    struct workerStruct {
        int socket;
        int id;
        std::string address;
        int port;
        TcpClient* ptrClient;
        bool active;
    };

    // Initialize
    void Initialize();

    // Run
    void Run();

    // Shutdown
    void Shutdown();

    void addWorker(int id, std::string address);

    void pushTuple(Tuple* t);

private:
    std::string _coordinatorAddress;
    int _coordinatorPort;
    int _id;

    TcpClient _coordinationClient;
    std::thread _coordinationClientThread;

    std::thread _ownThread;

    int _activeWorker;
    int _lastTupleId;

    bool _running;
    std::map<int, workerStruct> _workers;
    std::vector<std::thread> _workerThreads;
    std::string _tupledescr;
};

}
#endif