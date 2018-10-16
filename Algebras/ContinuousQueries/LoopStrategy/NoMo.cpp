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


NoMo::NoMo(int id, TcpClient* coordinationClient): 
    _coordinationClient(coordinationClient),
    _id(id),
    _running(false),
    _basePort(coordinationClient->GetServerPort()),
    _tupleServer(coordinationClient->GetServerPort() + (id*10))
{
}

// Destroy
NoMo::~NoMo()
{
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
        std::cout << "Waiting a maximum of 60 seconds for the tuple"
            << " receiving server to start... \n";

        while (!_tupleServer.IsRunning() && count < (60*1000)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            count = count + 100;
        }
        if (_tupleServer.IsRunning()) std::cout << " Done!\n";
    }

    if (!_tupleServer.IsRunning()) return;

    std::cout << "Done! Worker have to push hits "
        << "to this host on port "
        << std::to_string(_tupleServer.GetMasterPort()) << ". \n";

    _running = true;

    // confirm specialization
    (void) _coordinationClient->Send(
        CgenToHidleP::specializeHandler("nomo", true)
    );

    _running = true;
    Run();
}

// Run
void NoMo::Run()
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
            
            // get a hit
            if (msg.cmd == WgenToNoMoP::hit())
            {
                std::cout << "HIT! '" << msg.params << ". \n";
            } else

            // get a new user/query combination
            if (msg.cmd == CgenToNoMoP::addUserQuery()) 
            {
                int queryId = 0;
                std::string query = "";
                std::string email = "";

                std::vector<std::string> parts;
                boost::split(parts, msg.params, boost::is_any_of(
                    std::string(1, ProtocolHelpers::seperator)
                ));
                
                try
                {
                    queryId = std::stoi(parts[0]);
                    query = parts[1];
                    email = parts[2];
                }
                catch(...)
                {
                    queryId = 0;
                    std::cout << "failed to extract id, query or email \n";
                }

                if (queryId) addUserQuery(queryId, query, email);
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

void NoMo::addUserQuery(int queryId, std::string query, std::string email)
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

    std::cout << email << "and " << _queries[queryId].emails.size() << " others"
        << " will be informed when Query " << queryId << " is a hit. \n";
}

// Shutdown
void NoMo::Shutdown()
{
}

}