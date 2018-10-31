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

[1] Implementation of the Coordinator (Worker type: Loop)

The Coordinator coordinates all handlers. He has to accept new handlers,
give them an task and save information about them to optimize (and recover) 
the network.

He also receives all queries, saves them and assignes them to workers.

[toc]

1 CoordinatorLoop class implementation
see CoordinatorLoop.cpp for details.

*/

#ifndef __COORDINATORLOOP_H__
#define __COORDINATORLOOP_H__

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

class CoordinatorLoop {

public:
    // Create
    CoordinatorLoop();
    CoordinatorLoop(int port, std::string tupledescr);

    // Destroy
    ~CoordinatorLoop();

    enum class coordinatorStatus {initialze, run, shutdown};
    enum class handlerType {idle, worker, streamsupplier, nomo, all };
    enum class handlerStatus {inactive, active, unknown, all };

    struct handlerStruct {
        int socket;
        int id;
        std::string address;
        int port;
        handlerType type;
        handlerStatus status;
        bool awaitConfirmation; // TODO unneccessary, it's within type&status
        uint64_t wait_since;
        bool must_delete=false;
    };

    struct userStruct {
        std::string hash;
        std::string email;
        std::vector<int> ownqueries;
    };

    struct queryStruct {
        int id;
        std::string userhash;
        std::string function;
    };

    // Initialize
    void Initialize();

    // Run
    void Run();

    // ReceiveCoordinationMessage
    void ReceiveCoordinationMessage(std::string);

    // reactions to messages
    void doRegisterNewHandler(ProtocolHelpers::Message msg);
    void doConfirmNewHandler(ProtocolHelpers::Message msg);
    void doConfirmSpecialize(ProtocolHelpers::Message msg);

    void doRemote(std::string cmd);

    void createWorker(int id);
    void createNoMo(int id);

    void registerWorker(int id);
    void registerNoMo(int id);
    void registerStreamSupplier(int id);
    void registerQuery(queryStruct query);

    void doUserAuth(ProtocolHelpers::Message msg);
    void doGetQueries(ProtocolHelpers::Message msg);
    void doAddQuery(ProtocolHelpers::Message msg);

    int selectWorker();
    int selectNoMo();

    void shutdownHandler(int id, std::string reason="");
    void showStatus();

    void Shutdown();

    // helper functions
    int firstIdleHandler();
    int countHandlers(handlerType _type, handlerStatus _status);
    std::string getHashFromEmail(std::string email);
    int getIdFromSocket(int _socket);
    bool confirmMessageIntegrity(ProtocolHelpers::Message msg, int testId);
    bool checkNewFunction(std::string function);

private:
    int _lastId;
    int _coordinationPort;

    TcpServer _coordinationServer;
    std::thread _coordinationServerThread;
    coordinatorStatus _lifecyle;

    std::map<int, handlerStruct> _handlers;
    std::mutex _protectHandlers;

    std::map<std::string, userStruct> _users;

    int _lastQueryId;
    std::map<int, queryStruct> _queries;

    std::string _tupledescr;
};

}
#endif