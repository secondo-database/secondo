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
#include <sstream>
#include <boost/make_shared.hpp>

#include "SecondoException.h"
#include "SecParser.h"
#include "StringUtils.h"
#include "NestedList.h"
#include "Algebra.h"
#include "Operator.h"
#include "Algebras/Distributed2/ConnectionInfo.h"

#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/RelationInfo.hpp"
#include "Algebras/DBService/Replicator.hpp"
#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/SecondoUtils.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"


using namespace std;
using namespace distributed2;

namespace DBService
{

DBServiceManager::DBServiceManager()
{
    string port;
    SecondoUtils::readFromConfigFile(
            port, "DBService","DBServicePort", "9989");
    ServerRunnable commServer(atoi(port.c_str()));
    commServer.run<CommunicationServer>();

    string replicaNumber;
    SecondoUtils::readFromConfigFile(
            replicaNumber, "DBService","ReplicaNumber", "1");
    replicaCount = atoi(replicaNumber.c_str());
}

DBServiceManager* DBServiceManager::getInstance()
{
    if (!_instance)
    {
        _instance = new DBServiceManager();
    }
    return _instance;
}

ConnectionID DBServiceManager::getNextFreeConnectionID()
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

    SecondoUtils::createDatabaseOnRemoteServer(connectionInfo,
            "dbservice");

    SecondoUtils::openDatabaseOnRemoteServer(connectionInfo,
                                                   "dbservice");

    // retrieve location info from worker
    string dir;
    getConfigParamFromWorker(dir, connectionInfo,
            "Environment", "SecondoHome");
    string commPort;
    getConfigParamFromWorker(commPort, connectionInfo,
            "DBService", "CommunicationPort");
    string transferPort;
    getConfigParamFromWorker(transferPort, connectionInfo,
            "DBService", "FileTransferPort");
    LocationInfo location(host, stringutils::int2str(port), dir,
            commPort, transferPort);

    pair<LocationInfo, ConnectionInfo*> workerConnDetails(location,
                                                          connectionInfo);
    connections.insert(
            pair<size_t, pair<LocationInfo, ConnectionInfo*> >(
                    getNextFreeConnectionID(), workerConnDetails));

    if(!startServersOnWorker(connectionInfo))
    {
        // TODO more descriptive error message (host, port, etc)
        throw new SecondoException("could not start file transfer server");
    }
}

bool DBServiceManager::startServersOnWorker(
        distributed2::ConnectionInfo* connectionInfo)
{
    string queryInit("query initdbserviceworker()");
    print(queryInit);

    return SecondoUtils::executeQueryOnRemoteServer(connectionInfo,
            queryInit);
}

bool DBServiceManager::getConfigParamFromWorker(string& result,
        distributed2::ConnectionInfo* connectionInfo, const char* section,
        const char* key)
{
    string resultAsString;
    stringstream query;
    query << "query getconfigparam(\""
          << section
          << "\", \""
          << key
          << "\")";
    bool resultOk = SecondoUtils::executeQueryOnRemoteServer(connectionInfo,
            query.str(), resultAsString);
    print(resultAsString);

    ListExpr resultAsNestedList;
    nl->ReadFromString(resultAsString, resultAsNestedList);
    result.assign(nl->StringValue(nl->Second(resultAsNestedList)));
    print(result);

    return resultOk && result.size() != 0;
}

void DBServiceManager::storeRelationInfo(const string& databaseName,
                                         const string& relationName,
                                         const string& host,
                                         const string& port,
                                         const string& disk)
{
    RelationInfo relationInfo(databaseName,
                              relationName,
                              host,
                              port,
                              disk);
    // TODO handle more than one location (getWorkerNodesForReplication)
    relationInfo.addNode(determineReplicaLocation());
    replicaLocations.insert(pair<string, RelationInfo>(relationInfo.toString(),
            relationInfo));
}

void DBServiceManager::getReplicaLocations(const string& relationAsString,
                                           vector<ConnectionID>& ids)
{
    RelationInfo& relInfo = getRelationInfo(relationAsString);
    ids.insert(ids.begin(), relInfo.nodesBegin(), relInfo.nodesEnd());
}

void DBServiceManager::getWorkerNodesForReplication(
        vector<ConnectionID>& nodes)
{
    if (connections.size() < replicaCount)
    {
        throw new SecondoException("not enough DBService worker nodes");
    }
    while(nodes.size() < replicaCount)
    {
        nodes.push_back(determineReplicaLocation());
    }
}

ConnectionID DBServiceManager::determineReplicaLocation()
{
    // TODO consider fault tolerance mode etc
    // maybe introduce a helper structure to store all possible locations
    // for each node
    return rand() % connections.size() + 1;
}

ConnectionInfo* DBServiceManager::getConnection(ConnectionID id)
{
    return connections.at(id).second;
}

LocationInfo& DBServiceManager::getLocation(ConnectionID id)
{
    return connections.at(id).first;
}

RelationInfo& DBServiceManager::getRelationInfo(const string& relationAsString)
{
    return replicaLocations.at(relationAsString);
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
