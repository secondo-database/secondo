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
// #include "CoordinationServer.h"
#include <boost/algorithm/string.hpp>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace continuousqueries {

/*
1.1 Constructor

Creates a new CoordinatorLoop object.

*/

CoordinatorLoop::CoordinatorLoop(int port, std::string tupledescr): 
    _lastId(0),
    _coordinationPort(port),
    _coordinationServer(port),
    _lastQueryId(0),
    _tupledescr(tupledescr)
{

}

/*
1.2 Destructor

Destroys the CoordinatorLoop object and sets all ressources free.

*/

CoordinatorLoop::~CoordinatorLoop() {
    Shutdown();
}

/*
1.3 Receive Coordination Message

Receives 

*/

void CoordinatorLoop::ReceiveCoordinationMessage(std::string message) {

}

/*
1.4 Run

The coordination works completely event driven. He reacts to incoming messages
according to his life cycle status:
 * initialize
 * run
 * shutdown

In the initializing mode, it waits for all neccessary handlers to connect and
then branches out to the Initialize function, where it waits for the answers
of the become messages. (Minimum configuration is 1 stream supplier, 1 worker 
and 1 nomo handler.)

The coordinationServer runs in a seperate thread and pushes incoming messages 
to its queue.

Outgoing messages are handled seriell. If the coordinator awaits a reaction 
to one of his messages, he stores this information within the handler struct.

*/

void CoordinatorLoop::Run() {
    // start the server, received messages will be pushed to _MessageQueque
    _coordinationServerThread = std::thread(
        &TcpServer::Run, 
        &_coordinationServer
    );
    
    // wait for the server to be started
    if (!_coordinationServer.IsRunning()) 
    {
        int count = 0;
        std::cout << "Waiting a maximum of 60 seconds for the server "
            << "to start... \n";

        while (!_coordinationServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_coordinationServer.IsRunning()) std::cout << " Done!\n";
    }

    if (!_coordinationServer.IsRunning()) return;

    std::cout << "Done! New handler have to connect to this host on port "
        << std::to_string(_coordinationServer.GetMasterPort()) << ". \n";

    // start the coordination loop in the initialze lifecycle
    _lifecyle = coordinatorStatus::initialze;

    while (_lifecyle != coordinatorStatus::shutdown) 
    {
        // wait for a new message
        std::unique_lock<std::mutex> lock(_coordinationServer.mqMutex);

        bool hasMsg = _coordinationServer.mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationServer.messages.empty();
        });

        if (hasMsg)
        {
            ProtocolHelpers::Message msg = ProtocolHelpers::decodeMessage(
                _coordinationServer.messages.front()
            );
            _coordinationServer.messages.pop();
            
            lock.unlock();

            ProtocolHelpers::printMessage(msg);

            if (msg.valid) {
                // add a new idle handler or stream supplier to the network
                // cmd is identical with hello from stream supplier
                if (msg.cmd == IdleGenP::hello()) {
                    doRegisterNewHandler(msg);
                } else

                // new handler confirms his id and the tupledescr
                if (msg.cmd == CoordinatorGenP::confirmhello()) {
                    doConfirmNewHandler(msg);
                } else

                // new handler confirms his role
                if (msg.cmd == IdleGenP::confirmspecialize()) {
                    doConfirmSpecialize(msg);
                } else

                // webinterface registers new user
                if (msg.cmd == CoordinatorGenP::userauth()) {
                    doUserAuth(msg);
                } else

                // webinterface requests all queries for a user
                if (msg.cmd == CoordinatorGenP::getqueries()) {
                    doGetQueries(msg);
                } else
                
                // webinterface adds a new query
                if (msg.cmd == CoordinatorGenP::addquery(0, "", false)) {
                    doAddQuery(msg);
                } else

                // a way to send remote commands to other handler
                if (msg.cmd == CoordinatorGenP::remote()) {
                    doRemote(msg.params);
                } else

                // a way to shut down the coordinator
                if (msg.cmd == CoordinatorGenP::shutdown()) {
                    if (msg.params == "474747")
                    {
                        std::cout << "Shutting down the coordinator... \n";
                        _lifecyle = coordinatorStatus::shutdown;
                    }
                } else

                // a way to get information about the networt
                if (msg.cmd == CoordinatorGenP::status()) {
                    showStatus();
                }

            } else {
                std::cout << "Message '" + msg.body + "' is invalid... \n";
            }
        } else {
            lock.unlock();
            std::cout << "mqCondition Timeout... no new messages"
                << " in the last 5 seconds. \n";
        }

        // build the minimal network if idle handlers exist.
        if (_lifecyle == coordinatorStatus::initialze) 
        {
            // create a worker, if neccessary
            if (countHandlers(handlerType::worker, handlerStatus::all) == 0)
                createWorker(firstIdleHandler());

            // create a nomo, if neccessary
            if (countHandlers(handlerType::nomo, handlerStatus::all) == 0)
                createNoMo(firstIdleHandler());

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

        // handle timeouts and reocurring tasks
        // loop over handler
        uint64_t now = ProtocolHelpers::getUnixTimestamp();

        for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
            it != _handlers.end(); it++)
        {
            // hello timeouts
            if (((it->second.type == handlerType::idle) or 
                 (it->second.type == handlerType::streamsupplier)) and 
                (it->second.status == handlerStatus::inactive) and 
                (now - it->second.wait_since > 15)) {
                    shutdownHandler(it->first, 
                        "hello: failed to respond within 15 seconds");
                }
            // specialize timeouts
            if (((it->second.type == handlerType::worker) or 
                 (it->second.type == handlerType::nomo)) and
                (it->second.status == handlerStatus::inactive) and 
                (now - it->second.wait_since > 15)) {
                    shutdownHandler(it->first, 
                        "specialize: failed to respond within 15 seconds");
                }

            // get statistics
        }

        // garbage collecting... 
        for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
            it != _handlers.end();)
        {
            if (it->second.must_delete)
            {
                it = _handlers.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// returns the id of the first idle and active worker, 0 if there is none
int CoordinatorLoop::firstIdleHandler()
{
    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::idle) and 
            (it->second.status == handlerStatus::active)) 
        return it->first;
    }

    return 0;
}

// returns the id of the worker who should work a query, 0 if there is none
int CoordinatorLoop::selectWorker()
{
    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::worker) and 
            (it->second.status == handlerStatus::active)) 
        return it->first;
    }

    return 0;
}

// returns the id of the nomo who should work a query, 0 if there is none
int CoordinatorLoop::selectNoMo()
{
    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if ((it->second.type == handlerType::nomo) and 
            (it->second.status == handlerStatus::active)) 
        return it->first;
    }

    return 0;
}

/*
1.X createWorker

This function is called when an idle handler should be specialized to become
an loopworker.

*/

void CoordinatorLoop::createWorker(int id) 
{
    if (id == 0) 
    {
        std::cout << "Can't create worker, no idle handler. \n";
        return;
    }

    if (_handlers.find(id) == _handlers.end()) 
    {
        std::cout << "Can't create worker, ID is unknow. \n";
        return;
    }

    // sent specialize
    int sendl = _coordinationServer.Send(
        _handlers[id].socket,
        CoordinatorGenP::specialize("worker", true)
    );

    if ((size_t)sendl == 
        CoordinatorGenP::specialize("worker", true).length())
    {
        _handlers[id].type = handlerType::worker;
        _handlers[id].status = handlerStatus::inactive;
        _handlers[id].wait_since = ProtocolHelpers::getUnixTimestamp();
    } else {
        std::cout << "worker: Error sending specializeHandler message. \n";
    }
} 

/*
1.X registerWorker

This function informs all other participients in the network about the new
worker.

*/

void CoordinatorLoop::registerWorker(int id)
{
    int sendl;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        // inform all StreamSupplier about the new worker
        if ((it->second.type == handlerType::streamsupplier) 
             and (it->second.status == handlerStatus::active))
        {
            sendl = _coordinationServer.Send(
                it->second.socket,
                CoordinatorGenP::addhandler(id, "worker", 
                    _handlers[id].address, true)
            );

            if ((size_t)sendl != CoordinatorGenP::addhandler(id, "worker", 
                    _handlers[id].address, true).length())
            {
                std::cout << "registerWorker: Error informing StSu " << 
                    it->first << ". \n";
            }
        } else

        // inform the new worker about all existing NoMos
        if ((it->second.type == handlerType::nomo) 
             and (it->second.status == handlerStatus::active))
        {
            sendl = _coordinationServer.Send(
                _handlers[id].socket,
                CoordinatorGenP::addhandler(it->first, "nomo", 
                    it->second.address, true)
            );

            if ((size_t)sendl != CoordinatorGenP::addhandler(it->first, 
                "nomo", it->second.address, true).length())
            {
                std::cout << "registerWorker: Error informing Worker " << 
                    it->first << ". \n";
            }
        }
    }
}

/*
1.X createNoMo

This function is called when an idle handler should be specialized to become
an nomo handler.

*/

void CoordinatorLoop::createNoMo(int id) 
{
    if (id == 0) 
    {
        std::cout << "Can't create nomo, no idle handler. \n";
        return;
    }

    if (_handlers.find(id) == _handlers.end()) 
    {
        std::cout << "Can't create nomo, ID is unknow. \n";
        return;
    }

    // sent become
    int sendl = _coordinationServer.Send(
        _handlers[id].socket,
        CoordinatorGenP::specialize("nomo", true)
    );

    if ((size_t)sendl == 
        CoordinatorGenP::specialize("nomo", true).length())
    {
        _handlers[id].type = handlerType::nomo;
        _handlers[id].status = handlerStatus::inactive;
        _handlers[id].wait_since = ProtocolHelpers::getUnixTimestamp();
    } else {
        std::cout << "nomo: Error sending specializeHandler message. \n";
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
1.X registerNoMo

This function informs all existing worker in the network about the new 
nomo.

*/

void CoordinatorLoop::registerNoMo(int id)
{
    int sendl;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        // inform all worker about the new NoMo
        if ((it->second.type == handlerType::worker) 
             and (it->second.status == handlerStatus::active))
        {
            sendl = _coordinationServer.Send(
                it->second.socket,
                CoordinatorGenP::addhandler(id, "nomo", 
                    _handlers[id].address, true)
            );

            if ((size_t)sendl != CoordinatorGenP::addhandler(id, "nomo", 
                    _handlers[id].address, true).length())
            {
                std::cout << "registerNoMo: Error informing Worker " << 
                    it->first << ". \n";
            }
        }
    }
}

/*
1.X registerStreamSupplier

This function informs the new stream supplier about all existing worker.

*/

void CoordinatorLoop::registerStreamSupplier(int id)
{
    int sendl;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        // inform the stream supplier about all worker
        if ((it->second.type == handlerType::worker) 
             and (it->second.status == handlerStatus::active))
        {
            sendl = _coordinationServer.Send(
                _handlers[id].socket,
                CoordinatorGenP::addhandler(it->first, "worker",
                    it->second.address, true)
            );

            if ((size_t)sendl != CoordinatorGenP::addhandler(it->first, 
                "worker", it->second.address, true).length())
            {
                LOG << "registerStreamSupplier: Error informing " 
                    << "about Worker " << it->first << "." << ENDL;
            }
        }
    }
}

void CoordinatorLoop::doRemote(std::string cmd)
{
    size_t cmdPos = cmd.find("|");

    if (cmdPos == std::string::npos) 
    {
        std::cout << "remote: no pipe. \n";
        return;
    }

    std::string sId  = boost::algorithm::trim_copy(cmd.substr(0, cmdPos));
    std::string sCmd = boost::algorithm::trim_copy(
        cmd.substr(cmdPos + 1, cmd.size() - cmdPos));
    
    int id = 0;

    try {
        id = std::stoi(sId);
    } catch(...) {
        std::cout << "remote: stoi failed. \n";
        return;
    }

    if (_handlers.find(id) == _handlers.end())
    {
        std::cout << "remote: id unknow. \n";
        return;
    }

    (void) _coordinationServer.Send(
        _handlers[id].socket,
        sCmd
    );
}

/*
1.X doRegisterNewHandler

This function is called when ever a new handler wants to connect to the 
network. The Coordinator reacts with a confirmHandler message including the
unique id of the new handler.

*/

void CoordinatorLoop::doRegisterNewHandler(ProtocolHelpers::Message msg) 
{
    std::lock_guard<std::mutex> guard(_protectHandlers);

    handlerStruct handler;
    handler.id = ++_lastId;
    handler.socket = msg.socket;
    handler.address = _coordinationServer.GetIpFromSocket(msg.socket);
    handler.port = _coordinationServer.GetPortFromSocket(msg.socket);
    handler.status = handlerStatus::inactive;
    handler.awaitConfirmation = true;
    handler.wait_since = ProtocolHelpers::getUnixTimestamp();

    if (msg.params.substr(0, 14) == "streamsupplier") {
        handler.type = handlerType::streamsupplier;
    } else {
        handler.type = handlerType::idle;
    }

    int sendl = _coordinationServer.Send(
        msg.socket,
        CoordinatorGenP::confirmhello(handler.id, _tupledescr, true)
    );

    if ((size_t)sendl == 
        CoordinatorGenP::confirmhello(handler.id, _tupledescr, true).length())
    {
        _handlers.insert( std::pair<int, handlerStruct>(handler.id, handler));
    } else {
        std::cout << "Error while sending confirmHandler message."
            << " Handler ignored. \n";
    }
}

/*
1.X doConfirmNewHandler

This function is called when ever a new handler responds to the confirm message
by mirroring the assigned id. If he fails to do that or if the coordinator is 
not awaiting an confirmation message from this handler, the coordinator shuts
the handler down.

*/

void CoordinatorLoop::doConfirmNewHandler(ProtocolHelpers::Message msg) 
{
    // generate the received Id from the message
    int receivedId=0;
    try {
        receivedId = std::stoi(msg.params);
    } catch(...) {
        (void) _coordinationServer.Send(
            msg.socket,
            CoordinatorGenP::shutdown(
                "cannot cast the id to int", 
                true
            )
        );
        return;
    }

    if (confirmMessageIntegrity(msg, receivedId)) 
    {
        if (_handlers[receivedId].awaitConfirmation)
        {
            _handlers[receivedId].status = handlerStatus::active;
            _handlers[receivedId].wait_since = 0;

            if (_handlers[receivedId].type == handlerType::streamsupplier)
                registerStreamSupplier(receivedId);

            std::cout <<"Handshake for handler "<<receivedId<< " confirmed.\n";
        } else {
            shutdownHandler(receivedId, "not awaiting a handshake");
        }

    } else {
        shutdownHandler(getIdFromSocket(msg.socket), 
            "failed to respond with correct id");
    }
}

/*
1.X doConfirmSpecialize

This function is called when ever a new handler confirms that he has 
successfully specialized to a worker or nomo handler. His state is changed
to active.

Only possible for inactive worker and nomo handler.

*/

void CoordinatorLoop::doConfirmSpecialize(ProtocolHelpers::Message msg) 
{
    int id = getIdFromSocket(msg.socket);
    
    if (id) 
    {
        if ((_handlers[id].status == handlerStatus::inactive) and 
           ((_handlers[id].type == handlerType::worker) or 
            (_handlers[id].type == handlerType::nomo)))
        {
            if (((msg.params == "worker") and 
                 (_handlers[id].type == handlerType::worker)) or
                ((msg.params == "nomo") and 
                 (_handlers[id].type == handlerType::nomo)))
            {
                // this happens if the everything is correct
                _handlers[id].status = handlerStatus::active;
                _handlers[id].wait_since = 0;
                LOG << "Handler " <<id<< " successfully specialized." << ENDL;

                // inform the other handler about the new participient
                if (msg.params == "worker") registerWorker(id);
                if (msg.params == "nomo") registerNoMo(id);
            } else {
                shutdownHandler(id, "wrong specialization");
            }
        } else {
            shutdownHandler(id, "not expected to specialize");
        }

    } else {
        std::cout << "There is no handler with ID " << id << ". \n";
    }
}


void CoordinatorLoop::doUserAuth(ProtocolHelpers::Message msg)
{
    std::string hash  = "";
    std::string email = "";
    std::string type  = "";

    std::vector<std::string> parts;

    boost::split(parts, msg.params, 
        boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
    );

    try
    {        
        hash  = parts[0];
        email = parts[1];
        type  = parts[2];
    }
    catch(...)
    {
        LOG << "error reveicing needed data for a new user" << ENDL;
        (void) _coordinationServer.Send(
            msg.socket,
            "error|Wrong Parameters! \n\n"
        );
        return;
    }

    // register new user
    if (type == "register") 
    {
        if (getHashFromEmail(email) == "") 
        {

        userStruct toAdd;
        toAdd.hash  = hash;
        toAdd.email = email;
        _users.insert( std::pair<std::string, userStruct>(toAdd.hash, toAdd));

        LOG << "Added user " << hash << " with email " << email << "." << ENDL;

        (void) _coordinationServer.Send(
            msg.socket,
            CoordinatorGenP::userauth("register", _tupledescr, true) + "\n\n"
        );
        } else {
            (void) _coordinationServer.Send(
                msg.socket,
                "error|email already in use \n\n"
            );
        }
    } else

    // login existing user
    if (type == "login")
    {
        if (getHashFromEmail(email) == hash)
        {
            LOG << "Logged user " << hash << " with email " 
                << email << " in." << ENDL;

            (void) _coordinationServer.Send(
                msg.socket,
                CoordinatorGenP::userauth("login", _tupledescr, true) + "\n\n"
            );
        } else {
            (void) _coordinationServer.Send(
                msg.socket,
                "error|wrong password \n\n"
            );
        }
    }
}

void CoordinatorLoop::doGetQueries(ProtocolHelpers::Message msg)
{
    // check if a valid user is asking for his queries
    if (_users.find(msg.params) == _users.end()) 
    {
        LOG << "No user with hash " << msg.params << " found." << ENDL;

        (void) _coordinationServer.Send(
            msg.socket,
            "error|User not found! \n\n"
        );

        return;
    }

    // check if there are any queries
    // if (_users.find(msg.params)->second.ownqueries.size() == 0) 
    // {
    //     LOG << "User with hash " << msg.params << " has no queries."<<ENDL;

    //     (void) _coordinationServer.Send(
    //         msg.socket,
    //         "error|No queries! \n\n"
    //     );

    //     return;
    // }

    // send all queries
    for (unsigned i=0; i < _users.find(msg.params)
            ->second.ownqueries.size(); i++) {
        (void) _coordinationServer.Send(
            msg.socket,
            CoordinatorGenP::getqueries(
                _queries[_users.find(msg.params)->second.ownqueries[i]].id, 
                _queries[_users.find(msg.params)
                    ->second.ownqueries[i]].function,
                true
            ) + "\n"
        );
    }

    // end transmission
    (void) _coordinationServer.Send(
        msg.socket,
        "\n\n"
    );
}

void CoordinatorLoop::doAddQuery(ProtocolHelpers::Message msg)
{
    LOG << "Adding query " << msg.params << ENDL;
    
    std::string hash = "";
    std::string func = "";

    std::vector<std::string> parts;

    boost::split(parts, msg.params, 
        boost::is_any_of(std::string(1,ProtocolHelpers::seperator))
    );

    try
    {        
        hash = parts[0];
        func = parts[1];
    }
    catch(...)
    {
        LOG << "error reveicing needed data for a new query" << ENDL;
        (void) _coordinationServer.Send(
            msg.socket,
            "error|Wrong Parameters! \n\n"
        );
        return;
    }
    
    // check if user exists
    if (_users.find(hash) == _users.end()) 
    {
        LOG << "No user with hash " << hash << " found." << ENDL;

        (void) _coordinationServer.Send(
            msg.socket,
            "error|User not found! \n\n"
        );

        return;
    }

    // add query
    // check if the function is ok
    if (!checkNewFunction(func)) {
        LOG << "Error in provided function!" << ENDL;
        // msg an client
        
        (void) _coordinationServer.Send(
            msg.socket,
            CoordinatorGenP::addquery(0, "", false) + "|error \n\n"
        );
        return;
    }

    queryStruct toAdd;
    toAdd.id = ++_lastQueryId;
    toAdd.userhash = hash;
    toAdd.function = func;

    // add to _queries
    _queries.insert( std::pair<int, queryStruct>(toAdd.id, toAdd));
    
    // add to _user[hash]
    _users[hash].ownqueries.push_back(toAdd.id);

    // confirm success
    (void) _coordinationServer.Send(
        msg.socket,
        CoordinatorGenP::addquery(0, "", false) + "|success \n\n"
    );

    registerQuery(toAdd);
}

bool CoordinatorLoop::checkNewFunction(std::string function) 
{
    // build funList
    ListExpr funList;
    if( !nl->ReadFromString(function, funList)) {
        LOG << "Error building funList" << ENDL;
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
        LOG << "Error building tree" << ENDL;
        delete tqp;
        return false;
    }

    delete tqp;

    LOG << "correct: " << correct << " && " 
        << "evaluable: " << evaluable << " && " 
        << "defined: " << defined << " && " 
        << "isFunction: " << isFunction << ENDL;

    return (correct && defined && isFunction);
}

void CoordinatorLoop::showStatus()
{
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n";
    std::cout << "**************************************************" << "\n";
    std::cout << "***                 STATUS                     ***" << "\n";
    std::cout << "**************************************************" << "\n";
    std::cout << "Coordinator Status: {initialze, run, shutdown}"     << "\n";
    std::cout << "Status:             " << (int) _lifecyle << "\n";

    std::cout << "\n\n\n";

    std::cout << "**************************************************" << "\n";
    std::cout << "***                 HANDLER                    ***" << "\n";
    std::cout << "**************************************************" << "\n";
    std::cout << "Type:   {idle, worker, streamsupplier, nomo, all }" << "\n";
    std::cout << "Status: {inactive, active, unknown, all }"          << "\n";

    std::cout << "\n";

    std::cout << "Number of connected handlers: " << _handlers.size() << "\n";
    std::cout << "\n\n\n";

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        std::cout <<"**************************************************"<<"\n";
        std::cout << "ID:      " << it->first << "\n";
        std::cout << "Type:    " << (int) it->second.type << "\n";
        std::cout << "Status:  " << (int) it->second.status << "\n";
        std::cout << "Address: " << it->second.address << "\n";
        std::cout << "Socket:  " << it->second.socket << "\n";

        if (it->second.type == handlerType::worker)
        std::cout << "Receive new tuple: " << it->second.address << ":"
                  << _coordinationPort + (it->first*10) << "\n";

        if (it->second.type == handlerType::nomo)
        std::cout << "Receive new hitd:  " << it->second.address << ":"
                  << _coordinationPort + (it->first*10) << "\n";
    }

    std::cout << "**************************************************" << "\n";
    std::cout << "\n\n";

}

void CoordinatorLoop::shutdownHandler(int id, std::string reason)
{
    if (_handlers.find(id) == _handlers.end()) 
    {
        std::cout << "Can't shutdown handler, ID doesn't exist.  \n";
        return;
    }

    (void) _coordinationServer.Send(
        _handlers.find(id)->second.socket,
            CoordinatorGenP::shutdown(reason, true)
    );

    _handlers[id].must_delete = true;

    std::cout << "Shutting down ID: " <<id<< ". Reason: '" << reason << "'\n";
    // todo close socket in server
}

/*
1.X confirmMessageIntegrity

Tests if the given handler exists and returns true if the sockets are 
identically. Unclear if this is really needed.

*/

bool CoordinatorLoop::confirmMessageIntegrity(
    ProtocolHelpers::Message msg, int testId)
{
    // test if an handler with this id exits
    if (_handlers.find(testId) == _handlers.end()) return false;

    // saved and received socket should be identical
    return (msg.socket == _handlers[testId].socket);
}

/*
1.X countHandlers

Returns the number of specified handlers.

*/

int CoordinatorLoop::countHandlers(handlerType _type, handlerStatus _status)
{
    int count=0;

    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if (((it->second.type == _type) and (it->second.status == _status))
        or ((handlerType::all == _type) and (it->second.status == _status))
        or ((it->second.type == _type) and (handlerStatus::all == _status)))
        count++;
    }

    return count;
}


/*
1.X getHashFromEmail

Returns the id of the handler with the corresponding socket. 
Or 0, if there is none.

*/

std::string CoordinatorLoop::getHashFromEmail(std::string email) 
{
    for (std::map<std::string, userStruct>::iterator it = _users.begin(); 
        it != _users.end(); it++)
    {
        if (it->second.email == email)
            return it->first;
    }

    return "";
}

/*
1.X getIdFromSocket

Returns the id of the handler with the corresponding socket. 
Or 0, if there is none.

*/

int CoordinatorLoop::getIdFromSocket(int _socket) 
{
    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        if (it->second.socket == _socket)
            return it->first;
    }

    return 0;
}

void CoordinatorLoop::Shutdown()
{
    // Shutting all connected handlers down
    for (std::map<int, handlerStruct>::iterator it = _handlers.begin(); 
        it != _handlers.end(); it++)
    {
        LOG << "Shutting down " << it->second.id << ENDL;
        shutdownHandler(it->second.id, "Coordinator is shutting down.");
    }

    _coordinationServer.Shutdown();
    _coordinationServerThread.join();
}

}