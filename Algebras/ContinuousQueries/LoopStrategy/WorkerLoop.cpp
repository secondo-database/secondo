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


WorkerLoop::WorkerLoop(int id, std::string tupledescr, 
    TcpClient* coordinationClient):
    _coordinationClient(coordinationClient),
    _id(id),
    _tupledescr(tupledescr),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id*10))
{
}

// Destroy
WorkerLoop::~WorkerLoop()
{
    Shutdown();
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
        WorkerGenP::confirmspecialize(true)
    );

    Run();
}

// Run
void WorkerLoop::Run()
{
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) 
    {
        std::unique_lock<std::mutex> lock(_coordinationClient->mqMutex);

        hasMsg = _coordinationClient->mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationClient->messages.empty();
        });

        // _coordinationClient->mqCondition.wait(lock, [this] {
        //     return !_coordinationClient->messages.empty();
        // });

        if (!_running) {
            LOG << "!_running-->continue" << ENDL;
            lock.unlock();
            continue;
        }
        
        if (hasMsg)
        {
            msg = ProtocolHelpers::decodeMessage(
                _coordinationClient->messages.front()
            );
            _coordinationClient->messages.pop();
            
        } else {
            msg.valid = false;
        }

        lock.unlock();

        if (hasMsg && msg.valid) {

            // get a new nomo
            if (msg.cmd == CoordinatorGenP::addhandler()) 
            {
                int nomoId = 0;
                std::string handlerType = "";
                std::string nomoAddress = "";

                std::vector<std::string> parts;

                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    nomoId = std::stoi(parts[0]);
                    handlerType = parts[1];
                    nomoAddress = parts[2];
                }
                catch(...)
                {
                    nomoId = 0;
                    std::cout << "failed to extract id or address \n";
                }

                if (nomoId && handlerType=="nomo") addNoMo(nomoId, nomoAddress);
            } else

            // get a query
            if (msg.cmd == CoordinatorGenP::addquery(0, "", false)) 
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
            if (msg.cmd == CoordinatorGenP::shutdown() || 
                msg.cmd == "disconnected") 
            {
                LOG << "shutting down due to " << msg.cmd 
                    << " " << msg.params << ENDL;
                
                _running = false;
            } 

            // unknown command
            else 
            {
                std::cout << "No handler for command " << msg.cmd << ".\n";
            }

        } else {
            if (hasMsg) 
            {
                std::cout << "Message '" + msg.cmd + "' is invalid... \n";
            } else {
                std::cout << "No Message. Timeout... \n";
            }
        }   
    }
}

void WorkerLoop::TightLoop() 
{
    // wait for new tuple
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) {
        std::unique_lock<std::mutex> lock(_tupleServer.mqMutex);

        hasMsg = _tupleServer.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_tupleServer.messages.empty();
        });

        if (!_running) {
            LOG << "TL: !_running-->continue" << ENDL;
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

            if (!tupleId) continue;

            // create tuple
            
            ListExpr attrlist;
            nl->ReadFromString(_tupledescr, attrlist);

            ListExpr resultTupleType = nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                attrlist
            );

            SecondoCatalog* sc = SecondoSystem::GetCatalog();
            ListExpr numResultTupleType = sc->NumericType(resultTupleType);

            TupleType* tt = new TupleType(numResultTupleType);
            Tuple* tuple  = new Tuple(tt);

            tuple->ReadFromBinStr(0, tupleString);

            // Beging creating the string representation of tuple
            // ListExpr _tType = nl->OneElemList(
            //     SecondoSystem::GetCatalog()->NumericType(
            //         nl->TwoElemList(
            //             nl->SymbolAtom(Tuple::BasicType()),
            //             attrlist
            //         )
            //     )
            // );
            // ListExpr tupleValue;
            // std::string message;
            // tupleValue = tuple->Out(_tType);
            // nl->WriteToString(message, tupleValue);
            // LOG << message << ENDL;

            // loop over Queries, check for hits
            std::string hitlist = "";
            bool anyhit = false;

            for (std::map<int, queryStruct>::iterator it = _queries.begin();
                it != _queries.end(); it++)
            {
                if (filterTuple(tuple, it->second.tree, it->second.funargs))
                {
                    anyhit = true;
                    hitlist += std::to_string(it->first);
                    hitlist += ",";
                }
            }

            tuple->DeleteIfAllowed();
            tt->DeleteIfAllowed();
            // delete sc;

            hitlist = hitlist.substr(0, hitlist.size()-1);

            LOG << "tID: " << tupleId << "hl: " << hitlist << ENDL;

            // notify all nomos
            if (anyhit) notifyAllNoMos(tupleId, tupleString, hitlist);
        }
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

    qp->EvalS(tree, result, REQUEST);

    if (((Attribute*)result.addr)->IsDefined()) {
        return ((CcBool*)result.addr)->GetBoolval();
    }

    return false;
}

void WorkerLoop::notifyAllNoMos(int tupleId, std::string tupleString, 
    std::string hitlist)
{
    for (std::map<int, nomoStruct>::iterator it = _nomos.begin(); 
        it != _nomos.end(); it++)
    {
        it->second.ptrClient->SendAsync(
            WorkerGenP::hit(tupleId, tupleString, hitlist, true)
        );
    }
}

// NoMo handling
void WorkerLoop::addNoMo(int id, std::string address)
{
    LOG << "Adding NoMo " << id << "|" << address << ":" 
        << _basePort + (id * 10) << ENDL;

    nomoStruct toAdd;
    toAdd.id = id;
    toAdd.port = _basePort + (id * 10);
    toAdd.address = address;
    toAdd.ptrClient = new TcpClient(address, _basePort + (id * 10));

    toAdd.ptrClient->Initialize();
    
    _nomoThreads.push_back(std::thread(
        &TcpClient::AsyncHandler, 
        toAdd.ptrClient
    ));

    _nomos.insert( std::pair<int, nomoStruct>(id, toAdd));
}

void WorkerLoop::deleteNoMo(int id)
{}

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

    // QueryProcessor* qqp = new QueryProcessor(nl,
    //     SecondoSystem::GetAlgebraManager());

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

// Shutdown
void WorkerLoop::Shutdown()
{
    _running = false;

    _tupleServer.Shutdown();
    _tupleServerThread.join();
    _tightLoopThread.join();
    
    for (std::map<int, nomoStruct>::iterator it = _nomos.begin(); 
        it != _nomos.end(); it++)
    {
        it->second.ptrClient->Shutdown();
    }

    for (unsigned i=0; i < _nomoThreads.size(); i++) {
        _nomoThreads[i].join();
    }
}

}