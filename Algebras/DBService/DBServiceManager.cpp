/*
----
This file is part of SECONDO.

Copyright (C) 2016,
Faculty of Mathematics and Computer Science,
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


//[$][\$]
//[_][\_]

*/
#include <iostream>
#include <cstdlib>

#include "DBServiceManager.hpp"
#include "RelationInfo.hpp"
#include "Replicator.hpp"

#include "SecondoException.h"


using namespace std;
using namespace distributed2;

namespace DBService
{

DBServiceManager::DBServiceManager()
{
    std::vector<distributed2::ConnectionInfo*> workers;
}

DBServiceManager* DBServiceManager::getInstance()
{
    if (!_instance)
    {
        _instance = new DBServiceManager();
    }
    return _instance;
}

ConnectionID DBServiceManager::getNextConnectionID()
{
    return connections.size() + 1;
}

void DBServiceManager::addNode(const string host,
                               const int port,
                               string config)
{
    cout << "Adding connection: "
         << host << ":" << port << " -> " << config << endl;
    ConnectionInfo* connectionInfo =
    ConnectionInfo::createConnection(host, port, config);
    connections.insert(
            pair<size_t, ConnectionInfo*>(
                    getNextConnectionID(), connectionInfo));
}

bool DBServiceManager::replicateRelation(const std::string& relationName)
{
    shared_ptr<RelationInfo> replicaInfo(new RelationInfo(relationName));
    vector<ConnectionID> nodes;
    getWorkerNodesForReplication(nodes);
    replicaInfo->addNodes(nodes);
    replicaLocations.push_back(replicaInfo);
    Replicator replicator(".txt");
    replicator.replicateRelation(*replicaInfo);
    return false;
}

void DBServiceManager::getWorkerNodesForReplication(vector<ConnectionID>& nodes)
{
    size_t numberOfReplicas = 3;

    if(getNextConnectionID() <= numberOfReplicas)
    {
        for(ConnectionID id = 1; id <= getNextConnectionID(); ++id)
        {
            nodes.push_back(id);
        }
        if(nodes.size() < numberOfReplicas)
        {
            //TODO warning
        }
        return;
    }

    while(nodes.size() < numberOfReplicas)
    {
        ConnectionID id = rand() % getNextConnectionID() + 1;

        if(find(nodes.begin(), nodes.end(), id) == nodes.end()) {
            nodes.push_back(id);
        }
    }
}

ConnectionInfo* DBServiceManager::getConnection(ConnectionID id)
{
    return connections.at(id);
}

DBServiceManager* DBServiceManager::_instance = NULL;
map<ConnectionID, ConnectionInfo*> DBServiceManager::connections;

} /* namespace DBService */
