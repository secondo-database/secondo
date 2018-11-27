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

1 CoordinatorLoop class implementation

*/

#include "CoordinatorLoop.h"

namespace continuousqueries {

CoordinatorLoop::CoordinatorLoop(int port, std::string tupledescr): 
    CoordinatorGen::CoordinatorGen(port, tupledescr)
{
    _type = "loop";
}

CoordinatorLoop::~CoordinatorLoop() {
}

/*
1.X setupNetwork

Decides what an idle worker should become. 
Current strategy: Greedy worker. After creating one NoMo, all new confirmed
idle handler will become a worker.
Will be called with the id of a new idle handler. If not, this is probably 
the place to implement the logic for restructuring the network.

*/

void CoordinatorLoop::setupNetwork(int newHandlerId)
{
    if (!newHandlerId) return;

    // create a nomo, if neccessary
    if (countHandlers(handlerType::nomo, handlerStatus::all) == 0)
    {
        createNoMo(newHandlerId);
    } else {
        createWorker(newHandlerId, "loop");
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

void CoordinatorLoop::registerQuery(queryStruct query)
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

int CoordinatorLoop::selectWorker()
{
    int candidateId = 0;
    int candidateQueries = 0;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::worker) and 
            (it->second.status == handlerStatus::active)) 
        {
            if ((candidateId == 0) || ((int) it->second.ownqueries.size() 
                < candidateQueries)) {
                candidateQueries = (int) it->second.ownqueries.size();
                candidateId = it->first;
            }
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

bool CoordinatorLoop::checkNewFunction(std::string function, std::string &err)
{
    err = "???";

    // build funList
    ListExpr funList;
    if( !nl->ReadFromString(function, funList)) {
        std::cout << "Error building funList" << endl;
        err = "Error building funList";
        return false;
    }

    QueryProcessor* tqp = new QueryProcessor(nl,
        SecondoSystem::GetAlgebraManager());

    // build the tree
    OpTree tree = 0;
    ListExpr resultType;
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    try {
        tqp->Construct(
            funList,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );
    }
    catch(SI_Error ERR_IN_QUERY_EXPR) {
        std::cout << "Error building tree" << endl;
        err = "Error building tree";
        delete tqp;
        return false;
    }

    delete tqp;

    return (correct && defined && isFunction);
}

}