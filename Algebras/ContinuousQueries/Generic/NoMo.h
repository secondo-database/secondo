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
//[&] [\&]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of the Notification [&] Monitoring handler

It informs users when there is a query-tuple hit.

[toc]

1 NoMo class implementation
see NoMo.cpp for details.

*/

#ifndef __NOMO_H__
#define __NOMO_H__

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

class NoMo {

public:

    // Create
    NoMo(int id, std::string tupledescr, TcpClient* coordinationClient);

    // Destroy
    ~NoMo();

    struct queryStruct {
        int id;
        std::string query;
        std::vector<std::string> emails;
    };

    // Initialize
    void Initialize();

    // Run
    void Run();

    // Handle hits
    void NotificationLoop();
    void handleHit(int queryId, std::string tupleString, std::string hitlist);
    std::string tupleBinaryStringToRealString(std::string tupleString);
    
    // NoMo handling
    void addUserQuery(int id, std::string query, 
        std::string userhash, std::string email);

    // Shutdown
    void Shutdown();

private:
    TcpClient* _coordinationClient;
    int _id;
    std::string _tupledescr;
    bool _running;
    int _basePort;

    TcpServer _tupleServer;
    std::thread _tupleServerThread;
    std::thread _notificationLoopThread;

    std::map<int, queryStruct> _queries;
};

}
#endif