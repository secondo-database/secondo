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

[toc]

1 NoMo class implementation

*/

#include "NoMo.h"
#include <boost/algorithm/string.hpp>

namespace continuousqueries {

/*
1.1 Constructor

Creates a new NoMo object.

*/


NoMo::NoMo(int id, std::string tupledescr, TcpClient* coordinationClient): 
    _coordinationClient(coordinationClient),
    _id(id),
    _tupledescr(tupledescr),
    _running(false),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id*10))
{
}

// Destroy
NoMo::~NoMo()
{
    Shutdown();
}

// Initialize
void NoMo::Initialize()
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
        LOG << "Waiting a maximum of 60 seconds for the tuple"
            << " receiving server to start... " << ENDL;

        while (!_tupleServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_tupleServer.IsRunning()) LOG << " Done!" << ENDL;
    }

    if (!_tupleServer.IsRunning()) return;

    std::cout << "Done! Worker have to push hits "
        << "to this host on port "
        << std::to_string(_tupleServer.GetMasterPort()) << ". \n";

    // confirm specialization
    (void) _coordinationClient->Send(
        NoMoGenP::confirmspecialize(true)
    );

    // start tight loop in thread
    _notificationLoopThread = std::thread(
        &NoMo::NotificationLoop,
        this
    );

    _running = true;

    Run();
}

void NoMo::NotificationLoop()
{
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
            msg.valid=false;
        }
        
        lock.unlock();

        if (hasMsg && msg.valid && msg.cmd==WorkerGenP::hit()) {
            int tupleId = 0;
            std::string tupleString = "";
            std::string hitlist = "";

            std::vector<std::string> parts;
            boost::split(parts, msg.params, boost::is_any_of(
                std::string(1, ProtocolHelpers::seperator)
            ));
            
            try
            {
                tupleId = std::stoi(parts[0]);
                tupleString = parts[1];
                hitlist = parts[2];
            }
            catch(...)
            {
                tupleId = 0;
                LOG << "failed to extract tupleId, "
                    << "tupleString or hitlist" << ENDL;
            }

            LOG << "Data:" << tupleId << ", " 
                << tupleString << ". " << hitlist << ENDL;
            

            if (tupleId) handleHit(tupleId, tupleString, hitlist);
        }
    
    }
}

// Run
void NoMo::Run()
{
    bool hasMsg = false;
    ProtocolHelpers::Message msg;

    while (_running) {
        std::unique_lock<std::mutex> lock(_coordinationClient->mqMutex);

        hasMsg = _coordinationClient->mqCondition.wait_for(
            lock, 
            std::chrono::milliseconds(5000),
            [this] {
            return !_coordinationClient->messages.empty();
        });

        if (!_running) {
            LOG << "TL: !_running-->continue" << ENDL;
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
            msg.valid=false;
        }

        lock.unlock();
        
        if (hasMsg && msg.valid) {
            // get a new user query
            if (msg.cmd == CoordinatorGenP::addquery(0, "", "", "", false)) 
            {
                int queryId = 0;
                std::string function = "";
                std::string userhash = "";
                std::string email = "";

                std::vector<std::string> parts;
                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    queryId  = std::stoi(parts[0]);
                    function = parts[1];
                    userhash = parts[2];
                    email    = parts[3];
                }
                catch(...)
                {
                    queryId = 0;
                    LOG << "failed to extract id, query or email" << ENDL;
                }

                if (queryId) addUserQuery(queryId, function, userhash, email);
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
                std::cout << "Message '" << msg.cmd << "' is invalid... \n";
            } else {
                std::cout << "No Message. Timeout... \n";
            }
        }   
    }
}

void NoMo::handleHit(int tupleId, std::string tupleString, 
    std::string hitlist) 
{
    // create tuple
    LOG << tupleId << " hit: " << tupleString << ": " << hitlist << ENDL;
    LOG << tupleBinaryStringToRealString(tupleString) << ENDL;
}

std::string NoMo::tupleBinaryStringToRealString(std::string tupleString)
{
    LOG << "In tBSTRS" << ENDL;

    ListExpr resulttype;
    nl->ReadFromString(_tupledescr, resulttype);

    ListExpr _tupleType = nl->OneElemList(
        SecondoSystem::GetCatalog()->NumericType(
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                resulttype
            )
        )
    );

    ListExpr resultTupleType = nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
        resulttype
    );

    ListExpr numResultTupleType = SecondoSystem::GetCatalog()
        ->NumericType(resultTupleType);

    TupleType* tt = new TupleType(numResultTupleType);
    Tuple* tuple  = new Tuple(tt);

    tuple->ReadFromBinStr(0, tupleString);

    ListExpr tupleValue;
    std::string message;

    tupleValue = tuple->Out(_tupleType);
    nl->WriteToString(message, tupleValue);

    tt->DeleteIfAllowed();
    tuple->DeleteIfAllowed();

    return message;
}

void NoMo::addUserQuery(int queryId, std::string query, 
    std::string userhash, std::string email)
{
    // check if query already exists
    if (_queries.find(queryId) == _queries.end()) 
    {
        // create query
        queryStruct toAdd;
        toAdd.id = queryId;
        toAdd.query = query;

        _queries.insert(std::pair<int, queryStruct>(queryId, toAdd));
    }

    // add email to list
    _queries[queryId].emails.push_back(email);

    std::cout << email << " and " << _queries[queryId].emails.size()-1 
        << " others will be informed when Query " 
        << queryId << " was hit." << "\n";
}

// Shutdown
void NoMo::Shutdown()
{
    _running = false;

    _tupleServer.Shutdown();
    _tupleServerThread.join();
    _notificationLoopThread.join();
}

}