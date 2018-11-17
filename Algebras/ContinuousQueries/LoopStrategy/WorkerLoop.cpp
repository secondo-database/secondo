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

[1] Implementation of the loop worker.

[toc]

1 WorkerLoop class implementation

*/

#include "WorkerLoop.h"
#include <boost/algorithm/string.hpp>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace continuousqueries {

/*
1.1 Constructor

Creates a new WorkerLoop object.

*/


WorkerLoop::WorkerLoop(int id, std::string attrliststr, 
    TcpClient* coordinationClient):
    WorkerGen::WorkerGen(id, attrliststr, coordinationClient)
{
    _type = "loop";

    _monitor = new Monitor(id, _type, "", coordinationClient, 
        0.5 * 60 * 1000, 100);

    // Build TupleType once to speed up Tuple creation
    nl->ReadFromString(attrliststr, _attrlist);

    ListExpr resultTupleType = nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        _attrlist
    );

    ListExpr numResultTupleType = SecondoSystem::GetCatalog()->
        NumericType(resultTupleType);

    _tt = new TupleType(numResultTupleType);
    
}

WorkerLoop::~WorkerLoop() {
    _tt->DeleteIfAllowed();
}

// Query handling
void WorkerLoop::addQuery(int id, std::string function)
{
    LOG << "Adding Query " << id << "|" << function << ENDL;
    queryStruct toAdd;
    toAdd.id = id;
    toAdd.funText = function;
    
    // build funList
    ListExpr funList;
    if( !nl->ReadFromString(function, funList)) {
        LOG << "Error building funList" << ENDL;
        return;
    }

    // build the tree
    OpTree tree = 0;
    ListExpr resultType;
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    try {
        qp->Construct(
            funList,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );
    }
    catch(SI_Error ERR_IN_QUERY_EXPR) {
        LOG << "Error building tree" << ENDL;
        return;
    }

    toAdd.tree = tree;

    // get the funargs
    toAdd.funargs = qp->Argument(tree);

    _queries.insert( std::pair<int, queryStruct>(id, toAdd));
}


void WorkerLoop::TightLoop() 
{
    // wait for new tuple
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    _monitor->startBatch();

    while (_running) {
        std::unique_lock<std::mutex> lock(_tupleServer.mqMutex);

        hasMsg = _tupleServer.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_tupleServer.messages.empty();
        });

        if (!_running) {
            _monitor->checkBatch();
            lock.unlock();
            continue;
        }
        
        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                    _tupleServer.messages.front()
                );
            _tupleServer.messages.pop();
        } else {
            msg.valid = false;
        }
        
        lock.unlock();

        if (hasMsg && msg.valid && msg.cmd==StSuGenP::tuple()) {

            _monitor->startWorkRound();

            // extract informations
            int tupleId = 0;
            std::string tupleString = "";

            std::vector<std::string> parts;

            boost::split(parts, msg.params, boost::is_any_of(
                std::string(1, ProtocolHelpers::seperator)
            ));
            
            try
            {
                tupleId     = std::stoi(parts[0]);
                tupleString = parts[1];
            }
            catch(...)
            {
                tupleId = 0;
                LOG << "failed to extract id or tuple" << ENDL;
            }

            if (!tupleId) {
                _monitor->endWorkRound(0, 0, 0);
                continue;
            }
            // create tuple
            Tuple* tuple  = new Tuple(_tt);

            tuple->ReadFromBinStr(0, tupleString);

            // loop over Queries, check for hits
            std::string hitlist = "";
            int hits = 0;
            int queries = 0;

            for (std::map<int, queryStruct>::iterator it = _queries.begin();
                it != _queries.end(); it++)
            {
                queries++;
                if (filterTuple(tuple, it->second.tree, it->second.funargs))
                {
                    hits++;
                    hitlist += std::to_string(it->first);
                    hitlist += ",";
                }
            }

            tuple->DeleteIfAllowed();
            // delete sc;

            hitlist = hitlist.substr(0, hitlist.size()-1);

            LOG << "tID: " << tupleId << "hl: " << hitlist << ENDL;

            // notify all nomos
            if (hits) notifyAllNoMos(tupleId, tupleString, hitlist);

            _monitor->endWorkRound(1, queries, hits);
        }

        _monitor->checkBatch();
    }
}


/*
1.3.5 filterTuple

This function checks wether the filterfunction is true for a given tuple.
True is returned when the result of the function is true.

*/

bool WorkerLoop::filterTuple(Tuple* tuple, OpTree& tree, 
    ArgVectorPointer& funargs) 
{
    (*funargs)[0] = tuple;
    Word result;

    try
    {
        qp->EvalS(tree, result, REQUEST);

        if ((Attribute*)result.addr&&((Attribute*)result.addr)->IsDefined()) {
            return ((CcBool*)result.addr)->GetBoolval();
        }
    }
    catch(const std::exception& e)
    {
        return false;
    }


    return false;
}


void WorkerLoop::showStatus()
{
    LOG << "**************************************************" << ENDL;
    LOG << "WorkerLoop::Status" << ENDL << ENDL;
    LOG << "Number of Queries: " << _queries.size() << ENDL;
    LOG << "**************************************************" << ENDL;
}

}