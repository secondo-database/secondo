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

[toc]

1 CoordinatorJoin class implementation

*/

#include "CoordinatorJoin.h"

namespace continuousqueries {

CoordinatorJoin::CoordinatorJoin(int port, std::string attrlist): 
    CoordinatorGen::CoordinatorGen(port, attrlist)
{
    _type = "join";

    (void) ProtocolHelpers::getQueryAttributes(attrlist);
}

CoordinatorJoin::~CoordinatorJoin() {
}

/*
1.X setupNetwork

Decides what an idle worker should become. 
Current strategy: Greedy worker. After creating one NoMo, all new confirmed
idle handler will become a worker.
Will be called with the id of a new idle handler. If not, this is probably 
the place to implement the logic for restructuring the network.

*/

void CoordinatorJoin::setupNetwork(int newHandlerId)
{
    LOG << "in setupNetwork (Loop) " << newHandlerId << ENDL; 
    if (!newHandlerId) return;

    // create a nomo, if neccessary
    if (countHandlers(handlerType::nomo, handlerStatus::all) == 0)
    {
        LOG << "creating NoMo" << ENDL; 
        createNoMo(newHandlerId);
    } else {
        LOG << "creating Worker" << ENDL; 
        createWorker(newHandlerId, "join|S_eq");
    }

    if (_lifecyle == coordinatorStatus::initialze) 
    {
        if ((countHandlers(handlerType::worker, handlerStatus::active)>0) 
            and (countHandlers(
                handlerType::nomo, handlerStatus::active) > 0)
            and (countHandlers(
                    handlerType::streamsupplier, handlerStatus::active) > 0
                ))
        {
            _lifecyle = coordinatorStatus::run;
            std::cout << "All needed handler available."
                << " System is running. \n";
        }
    }
}

/*
1.X registerQuery

This function informs one worker and one nomo in the network about 
the new query.

*/

void CoordinatorJoin::registerQuery(queryStruct query)
{
    int w = selectWorker();
    if (!w) return;

    _queries[query.id].worker = w;
    _handlers[w].ownqueries.push_back(query.id);

    LOG << "Adding query " << query.function << " to worker " 
        << w << "." << ENDL;

    (void) _coordinationServer.Send(
        _handlers[w].socket,
        CoordinatorGenP::addquery(query.id, query.function, true)
    );

    int n = selectNoMo();
    if (!n) return;

    LOG << "Adding query " << query.function << " to nomo " 
        << n << "." << ENDL;

    (void) _coordinationServer.Send(
        _handlers[n].socket,
        CoordinatorGenP::addquery(
            query.id, 
            query.function,
            query.userhash,
            _users[query.userhash].email,
            true
        )
    );
}

/*
1.X selectWorker

Returns the id of the worker who should work a query, 0 if there is none.
Current strategy: the worker with the least amount of queries should get
a new query assigned.

*/

int CoordinatorJoin::selectWorker()
{
    int candidateId = 0;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::worker) and 
            (it->second.status == handlerStatus::active)) 
        {
            candidateId = it->first;
        }
    }

    return candidateId;
}

/*
1.X checkNewFunction

Checks if the provided function is a valid representation for this 
coordinator type. In this case, it has to be a valid function for the 
Secondo Query Processor.

*/

bool CoordinatorJoin::checkNewFunction(std::string function) 
{
    return true;
}

}