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

CoordinatorJoin::CoordinatorJoin(int port, std::string attrliststr): 
    CoordinatorGen::CoordinatorGen(port, attrliststr)
{
    _type = "join";

    _attrlist = ProtocolHelpers::getQueryAttributes(_attrliststr);

    // build queryparts map
    int count = _attrlist.length();
    
    for(int i = 1; i <= count; i++)
    {
        querypartStruct toAdd;
        toAdd.type = _attrlist.elem(i).second().convertToString();
        toAdd.comp = _attrlist.elem(i).first().convertToString().substr(
            _attrlist.elem(i).first().convertToString().length()-2
        );

        if (toAdd.comp == "eq")
        {
            if (toAdd.type == "bool")
            {
                toAdd.group = 3;
            } else {
                toAdd.group = 1;
            }
        } else {
            toAdd.group = 2;
        }

        _queryparts[_attrlist.elem(i).first().convertToString()] = toAdd;
    }
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
    if (!newHandlerId) return;

    // create a nomo, if neccessary
    if (countHandlers(handlerType::nomo, handlerStatus::all) == 0)
    {
        createNoMo(newHandlerId);
    } else {
        std::string jc = selectWorkerJoinCondition();
        createWorker(
            newHandlerId, 
            "join|" + jc
        );
        _queryparts[jc].worker.push_back(newHandlerId);
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
            std::cout << "Minimum of handlers is available."
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
    int w = selectWorker(query.function);
    if (!w) return;
    LOG << w << ENDL;

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
1.X selectWorkerJoinCondition

Returns the Join Condition a new worker should use.
Current strategy: create one worker for every elligble join condition. If 
all of them exist, choose the conditione with the highest queries per 
worker ratio.

*/

std::string CoordinatorJoin::selectWorkerJoinCondition()
{
    std::string joincondition = "";
    
    std::map<std::string, int> priorities;

    int count = _attrlist.length();
    int i;
    std::string name = "";

    // calculate the priority
    for(i = 1; i <= count; i++)
    {
        name = _attrlist.elem(i).first().convertToString();
        if (_queryparts[name].type == "bool") continue;
        if (name=="QID") continue;

        if (_queryparts[name].worker.size() == 0)
        {
            priorities[name] = std::numeric_limits<int>::max()
                - _queryparts[name].group;
        } else {
            if (_queryparts[name].queries.size()) 
            {
                // no queries but already at least one worker
                // so the join condition with the least (inactive)
                // worker should get the next worker
                priorities[name] = 
                    _queryparts[name].worker.size() * -1;
            } else {
                priorities[name] = 
                    _queryparts[name].queries.size() 
                        / _queryparts[name].worker.size();
            }
        }
    }

    // find the highest priority
    int max = 0;
    for(i = count; i != 0; i--)
    {
        name = _attrlist.elem(i).first().convertToString();

        if (_queryparts[name].type == "bool") continue;
        if (name=="QID") continue;

        if (priorities[name] >= max)
        {
            max = priorities[name];
            joincondition = name;
        }
    }

    return joincondition;
}

/*
1.X selectWorker

Returns the id of the worker who should work a query, 0 if there is none.
Current strategy: the worker with the least amount of queries should get
a new query assigned.
The function was already checked, so one can be assured, that there is
at least one worker for at least one of the sub queries.

*/

int CoordinatorJoin::selectWorker(std::string function)
{
    ListExpr newfunc;
    nl->ReadFromString(function, newfunc);

    int length = nl->ListLength(newfunc);

    // initialize a vector to check if a name is in the query function
    std::vector<std::string> parts;
    parts.push_back(nl->ToString(nl->First(nl->First(newfunc))));
    if (length>1)
        parts.push_back(nl->ToString(nl->First(nl->Second(newfunc))));
    if (length>2)
        parts.push_back(nl->ToString(nl->First(nl->Third(newfunc))));

    int candidateId      = 0;
    int candidateQueries = std::numeric_limits<int>::max();
    int candidateGroup   = 5;
    std::string name = "";

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::worker) and 
            (it->second.status == handlerStatus::active)) 
        {
            // check if this worker has a query part as his join condition
            LOG << it->second.info << "|" << it->second.info.substr(5) << ENDL;
            name = it->second.info.substr(5);
            if (std::find(parts.begin(), parts.end(), name) != parts.end())
            {
                if ((int) it->second.ownqueries.size() <= candidateQueries)
                {
                    if (_queryparts[name].group <= candidateGroup)
                    {
                        candidateGroup = _queryparts[name].group;
                        candidateQueries = (int) it->second.ownqueries.size();
                        candidateId = it->first;
                    }
                }
            }
        }
    }

    return candidateId;
}

/*
1.X checkNewFunction

Checks if the provided function is a valid representation for this 
coordinator type. In this case, it has to be in the list format, with
a maximum of three sub-lists, it cannot only check bool values and the
coordinator must already have a worker for this type.

*/

bool CoordinatorJoin::checkNewFunction(std::string function, std::string &err)
{
    ListExpr newfunc;
    err = "";
    
    // has to be a valid list
    if (!nl->ReadFromString(function, newfunc)) 
    {
        err = "not a valid list";
        return false;
    }

    int length = nl->ListLength(newfunc);

    // three elements is the maximum
    if (length > 3) 
    {
        err = "three comparisons are the maximum";
        return false;
    }

    // empty list not allowed
    if (length == 0) 
    {
        err = "empty list is not enough";
        return false;
    }


    // pure bool comparisons are not allowed
    // a worker for one of the parts must already exist
    bool allBools = true;
    bool noWorker = true;

    // first list element has to have length = 2
    if (nl->ListLength(nl->First(newfunc)) != 2)
    {
        err = "first list element has not 2 elements... loop function?";
        return false;
    }

    std::string name = nl->ToString(nl->First(nl->First(newfunc)));
    if (_queryparts[name].type != "bool") allBools = false;
    if (_queryparts[name].worker.size() > 0) noWorker = false;

    if (length>1)
    {
        name = nl->ToString(nl->First(nl->Second(newfunc)));
        if (_queryparts[name].type != "bool") allBools = false;
        if (_queryparts[name].worker.size() > 0) noWorker = false;
    }

    if (length>2)
    {
        name = nl->ToString(nl->First(nl->Third(newfunc)));
        if (_queryparts[name].type != "bool") allBools = false;
        if (_queryparts[name].worker.size() > 0) noWorker = false;
    }

    if (allBools)
    {
        err = "can't just compare on bool";
        return false;
    }

    if (noWorker)
    {
        err = "currently there is no worker for this attribute available";
        return false;
    }
    
    return true;
}

}