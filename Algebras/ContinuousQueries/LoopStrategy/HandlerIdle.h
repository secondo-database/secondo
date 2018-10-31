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
//[&] [\&]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of Idle Handler

The Idle Handler awaits a specialization. It can either become a worker or
a notification [&] monitoring handler.

[toc]

1 HandlerIdle class implementation
see HandlerIdle.cpp for details.

*/

#ifndef __HANDLERIDLE_H__
#define __HANDLERIDLE_H__

#include "../Tcp/TcpClient.h"
#include "../Protocols.h"
#include "WorkerLoop.h"
#include "NoMo.h"

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

namespace continuousqueries {

class HandlerIdle {

public:
    // Create
    // HandlerIdle();
    HandlerIdle(std::string address, int port);

    // Destroy
    ~HandlerIdle();

    // Initialize
    void Initialize();

    // Shutdown
    void Shutdown();

private:
    std::string _coordinatorAddress;
    int _coordinatorPort;
    int _id;

    TcpClient _coordinationClient;
    std::thread _coordinationClientThread;
    std::string _tupledescr;
};

}
#endif