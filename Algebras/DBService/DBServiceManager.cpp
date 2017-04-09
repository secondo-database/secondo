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
#include <string>

#include <boost/make_shared.hpp>

#include "DBServiceManager.hpp"
#include "RelationInfo.hpp"
#include "Replicator.hpp"
#include "DBServiceCommunicationServer.hpp"
#include "DBServiceUtils.hpp"
#include "DebugOutput.hpp"
#include "ServerRunnable.hpp"

#include "SecondoException.h"
#include "ConnectionInfo.h"
#include "SecParser.h"
#include "StringUtils.h"
#include "NestedList.h"
#include "Algebra.h"
#include "Operator.h"

using namespace std;
using namespace distributed2;

namespace DBService
{

DBServiceManager::DBServiceManager()
{
    string port;
    DBServiceUtils::readFromConfigFile(
            port, "DBService","DBServicePort", "9989");
    ServerRunnable commServer(atoi(port.c_str()));
    commServer.run<DBServiceCommunicationServer>();

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
    // TODO check that connection does not already exist!
    cout << "Adding connection: "
            << host << ":" << port << " -> " << config << endl;
    ConnectionInfo* connectionInfo =
            ConnectionInfo::createConnection(host, port, config);

    // retrieve information on SecondoHome (disk where data is stored on worker)
    string dir;
    retrieveSecondoHomeOnWorker(dir, connectionInfo);
    LocationInfo location(host, stringutils::int2str(port), dir);

    pair<LocationInfo, ConnectionInfo*> workerConnDetails(location,
                                                          connectionInfo);
    connections.insert(
            pair<size_t, pair<LocationInfo, ConnectionInfo*> >(
                    getNextConnectionID(), workerConnDetails));

    if(!startServersOnWorker(connectionInfo))
    {
        // TODO more descriptive error message (host, port, etc)
        throw new SecondoException("could not start file transfer server");
    }
}

bool DBServiceManager::startServersOnWorker(
        distributed2::ConnectionInfo* connectionInfo)
{
    //    query << "create database dbservice";
    //    try
    //    {
    //    DBServiceUtils::executeQueryOnRemoteServer(
    //connectionInfo, query.str());
    //    } catch(const SecondoException& e)
    //    {
    //    // result can be ignored, as error means that database already exists
    //    }

    DBServiceUtils::openDatabaseOnRemoteServer(connectionInfo,
                                               "dbservice");

    string queryInit("query initdbserviceworker()");
    print(queryInit);

    return DBServiceUtils::executeQueryOnRemoteServer(connectionInfo,
            queryInit);
}

bool DBServiceManager::retrieveSecondoHomeOnWorker(string& dir,
        distributed2::ConnectionInfo* connectionInfo)
{
    string resultAsString;
    DBServiceUtils::openDatabaseOnRemoteServer(connectionInfo,
                                               "dbservice");
    string querySecondoHome(
            "query getconfigparam(\"Environment\", \"SecondoHome\")");
    bool resultOk = DBServiceUtils::executeQueryOnRemoteServer(connectionInfo,
            querySecondoHome, resultAsString);
    print(resultAsString);

    dir.assign(resultAsString);
    // TODO retrieve result from nested list
//    ListExpr resultAsNestedList,
//    nl->ReadFromString(resultAsString, resultExpr);
//    dir.assign(nl->ToString(nl->Second(resultAsNestedList)));

    return resultOk && dir.size() != 0;
}

/*bool DBServiceManager::replicateRelation(const std::string& relationName)
{
    boost::shared_ptr<RelationInfo> replicaInfo(
            new RelationInfo(relationName));
    vector<ConnectionID> nodes;
    getWorkerNodesForReplication(nodes);
    replicaInfo->addNodes(nodes);
    replicaLocations.push_back(replicaInfo);
    Replicator replicator(".txt");
    replicator.replicateRelation(*replicaInfo);
    return false;
}*/

void DBServiceManager::getWorkerNodesForReplication(
        vector<ConnectionID>& nodes)
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
    return connections.at(id).second;
}

//TODO
bool DBServiceManager::persistLocationInformation()
{
    string query = "query ten feed head[3] consume";
    SecParser mySecParser;
    string nl_query_str;
    if(!mySecParser.Text2List(query,nl_query_str)!=0){
        // fehlerbehandlung
    } else {
        ListExpr nl_query;
        if(!nl->ReadFromString(nl_query_str,nl_query)){
            // sollte nicht auftreten
        } else {
            //QueryProcessor::ExecuteQuery(nl_query,...);
        }
    }
    return true;
}

DBServiceManager* DBServiceManager::_instance = NULL;
map<ConnectionID, pair<LocationInfo,
                       ConnectionInfo*> > DBServiceManager::connections;

} /* namespace DBService */
