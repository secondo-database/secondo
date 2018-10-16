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


WorkerLoop::WorkerLoop(int id, TcpClient* coordinationClient): 
    _coordinationClient(coordinationClient),
    _id(id),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id*10))
{
}

// Destroy
WorkerLoop::~WorkerLoop()
{

}

// Initialize
void WorkerLoop::Initialize()
{
    // start tuple server thread
    _tupleServerThread = std::thread(
        &TcpServer::Run, 
        &_tupleServer
    );

    // wait for the server to be started
    if (!_tupleServer.IsRunning()) 
    {
        int count = 0;
        std::cout << "Waiting a maximum of 60 seconds for the tuple"
            << " receiving server to start... \n";

        while (!_tupleServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_tupleServer.IsRunning()) std::cout << " Done!\n";
    }

    if (!_tupleServer.IsRunning()) return;

    std::cout << "Done! Stream supplier have to push tuple "
        << "to this host on port "
        << std::to_string(_tupleServer.GetMasterPort()) << ". \n";

    _running = true;

    // start tight loop in thread
    _tightLoopThread = std::thread(
        &WorkerLoop::TightLoop,
        this
    );

    // confirm specialization
    (void) _coordinationClient->Send(
        CgenToHidleP::specializeHandler("worker|loop", true)
    );

    Run();
}

// Run
void WorkerLoop::Run()
{
    while (_running) {
        std::unique_lock<std::mutex> lock(_coordinationClient->mqMutex);

        _coordinationClient->mqCondition.wait(lock, [this] {
            return !_coordinationClient->messages.empty();
        });
        
        ProtocolHelpers::Message msg = 
        ProtocolHelpers::decodeMessage(
            _coordinationClient->messages.front()
        );
        _coordinationClient->messages.pop();
        
        lock.unlock();

        if (msg.valid) {

            // get a new nomo
            if (msg.cmd == CgenToWloopP::addNomo()) 
            {
                int nomoId = 0;
                std::string nomoAddress = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    nomoId = std::stoi(parts[0]);
                    nomoAddress = parts[1];
                }
                catch(...)
                {
                    nomoId = 0;
                    std::cout << "failed to extract id or address \n";
                }

                if (nomoId) addNoMo(nomoId, nomoAddress);
            } else

            // get a query
            if (msg.cmd == CgenToWloopP::addQuery()) 
            {
                int qId = 0;
                std::string function = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    qId = std::stoi(parts[0]);
                    function = parts[1];
                }
                catch(...)
                {
                    qId = 0;
                    std::cout << "failed to extract id or function \n";
                }

                if (qId) addQuery(qId, function);
            } else

            // force shutdown
            if (msg.cmd == CgenToHidleP::shutdownHandler() || 
                msg.cmd == "disconnected") 
            {
                std::cout << "shutting down due to " << msg.cmd 
                    << " " << msg.params << "\n";
                
                _running = false;
            } 

            // unknown command
            else 
            {
                std::cout << "No handler for command " << msg.cmd << ".\n";
            }

        } else {
            std::cout << "Message '" << msg.cmd << "' is invalid... \n";
        }   
    }
}

void WorkerLoop::TightLoop() 
{
    // wait for new tuple

    while (_running) {
        std::unique_lock<std::mutex> lock(_tupleServer.mqMutex);

        _tupleServer.mqCondition.wait(lock, [this] {
            return !_tupleServer.messages.empty();
        });
        
        ProtocolHelpers::Message msg = 
            ProtocolHelpers::decodeMessage(
                _tupleServer.messages.front()
            );
        _tupleServer.messages.pop();
        
        lock.unlock();

        if ((msg.valid) and (msg.cmd=="tuple")) {
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
                std::cout << "failed to extract id or tuple \n";
            }

            if (!tupleId) continue;

            // create tuple
            TupleType* tt;
            
            ListExpr attrlist;
            std::string attrstring = "((No int))";
            nl->ReadFromString(attrstring, attrlist);

            
            ListExpr resultTupleType = nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                attrlist
            );

            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numResultTupleType = sc->NumericType(resultTupleType);
            tt = new TupleType(numResultTupleType);
            

            Tuple* tuple;
            tuple = new Tuple(tt);

            tuple->ReadFromBinStr(0, tupleString);

            // loop durch Queries, prüfe auf hit
            std::string hitlist = "";
            bool anyhit = false;

            for (std::map<int, queryStruct>::iterator it = _queries.begin();
                it != _queries.end(); it++)
            {
                std::cout << "inWhile" << "\n";

                if (filterTuple(
                    tuple, it->second.tree, 
                    it->second.funargs, 
                    it->second.qp)
                )
                {
                    std::cout << "inHit" << "\n";

                    anyhit = true;
                    hitlist += std::to_string(it->first);
                    hitlist += ",";
                }
            }

            hitlist = hitlist.substr(0, hitlist.size()-1);

            // notify all nomos
            if (anyhit) notifyAllNoMos(msg.params, hitlist);
        }
    }
}

/*
1.3.5 filterTuple

This function checks wether the filterfunction is true for a given tuple.
True is returned when the result of the function is true.

*/
bool WorkerLoop::filterTuple(Tuple* tuple, OpTree& tree, 
    ArgVectorPointer& funargs, QueryProcessor* qqp) 
{
    (*funargs)[0] = tuple;
    Word result;

    qp->EvalS(tree, result, REQUEST);

    if (((Attribute*)result.addr)->IsDefined()) {
        return ((CcBool*)result.addr)->GetBoolval();
    }
    return false;
}

void WorkerLoop::notifyAllNoMos(std::string tuple, std::string hitlist)
{
    std::cout << "notify " << hitlist << "\n";

    for (std::map<int, nomoStruct>::iterator it = _nomos.begin(); 
        it != _nomos.end(); it++)
    {
        std::cout << "sent to " << it->first << "\n";

        it->second.ptrClient->SendAsync(
            tuple + 
            std::string(1, ProtocolHelpers::seperator) +
            hitlist
        );
    }
}

// NoMo handling
void WorkerLoop::addNoMo(int id, std::string address)
{
    std::cout << id << "|" << address << ":" << _basePort + (id * 10) << "\n";
    nomoStruct toAdd;
    toAdd.id = id;
    toAdd.port = _basePort + (id * 10);
    toAdd.address = address;
    toAdd.ptrClient = new TcpClient(address, _basePort + (id * 10));

    toAdd.ptrClient->Initialize();

    std::thread t = std::thread(
        &TcpClient::AsyncHandler, 
        toAdd.ptrClient
    );

    t.detach();

    _nomos.insert( std::pair<int, nomoStruct>(id, toAdd));
}

void WorkerLoop::deleteNoMo(int id)
{

}

// Query handling
void WorkerLoop::addQuery(int id, std::string function)
{
    std::cout << "adding " << id << "|" << function << "\n";
    queryStruct toAdd;
    toAdd.id = id;
    toAdd.funText = function;
    
    // build funList
    ListExpr funList;
    if( !nl->ReadFromString(function, funList)) {
        std::cout << "Error building funList" << "\n";
        return;
    }

    QueryProcessor* qqp = new QueryProcessor(nl,
        SecondoSystem::GetAlgebraManager());

    // build the tree
    OpTree tree = 0;
    ListExpr resultType;
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;

    try {
        qqp->Construct(
            funList,
            correct,
            evaluable,
            defined,
            isFunction,
            tree,
            resultType );
    }
    catch(SI_Error ERR_IN_QUERY_EXPR) {
        std::cout << "Error building tree" << "\n";
        return;
    }

    toAdd.tree = tree;
    toAdd.qp = qqp;

    // get the funargs
    toAdd.funargs = qp->Argument(tree);

    _queries.insert( std::pair<int, queryStruct>(id, toAdd));
}

// Shutdown
void WorkerLoop::Shutdown()
{
    _tupleServer.Shutdown();
    _tupleServerThread.join();
}

}