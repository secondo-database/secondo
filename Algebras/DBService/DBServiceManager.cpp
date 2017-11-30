/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
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

*/
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/make_shared.hpp>

#include "Algebra.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoException.h"
#include "SecParser.h"
#include "StringUtils.h"

#include "Algebras/Distributed2/ConnectionInfo.h"

#include "Algebras/DBService/CommunicationServer.hpp"
#include "Algebras/DBService/DBServiceManager.hpp"
#include "Algebras/DBService/DBServicePersistenceAccessor.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/RelationInfo.hpp"
#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/SecondoUtilsRemote.hpp"
#include "Algebras/DBService/ServerRunnable.hpp"

using namespace std;
using namespace distributed2;

namespace DBService
{

DBServiceManager::DBServiceManager()
{
    printFunction("DBServiceManager::DBServiceManager");
    boost::lock_guard<boost::mutex> lock(managerMutex);
    restoreConfiguration();
    restoreReplicaInformation();
}

DBServiceManager::~DBServiceManager()
{
    printFunction("DBServiceManager::~DBServiceManager");
    DBServicePersistenceAccessor::persistAllLocations(connections);
    DBServicePersistenceAccessor::persistAllReplicas(relations, derivates);
}

void DBServiceManager::restoreConfiguration()
{
    printFunction("DBServiceManager::restoreConfiguration");
    string port;
    SecondoUtilsLocal::readFromConfigFile(
            port, "DBService","DBServicePort", "9989");
    ServerRunnable commServer(atoi(port.c_str()));
    commServer.run<CommunicationServer>();

    string replicaNumber;
    SecondoUtilsLocal::readFromConfigFile(
            replicaNumber, "DBService","ReplicaNumber", "1");
    replicaCount = atoi(replicaNumber.c_str());

    string faultToleranceMode;
    SecondoUtilsLocal::readFromConfigFile(
            faultToleranceMode, "DBService","FaultToleranceLevel", "2");
    mode = static_cast<FaultToleranceMode>(atoi(faultToleranceMode.c_str()));
}

void DBServiceManager::restoreReplicaInformation()
{
    printFunction("DBServiceManager::restoreReplicaInformation");
    map<ConnectionID, LocationInfo> locations;
    DBServicePersistenceAccessor::restoreLocationInfo(locations);
    for(map<ConnectionID, LocationInfo>::const_iterator it = locations.begin();
            it != locations.end(); it++)
    {
        ConnectionInfo* connectionInfo =
                ConnectionInfo::createConnection(
                            it->second.getHost(),
                            atoi(it->second.getPort().c_str()),
                            *(const_cast<string*>(&(it->second.getConfig()))));
        pair<LocationInfo, ConnectionInfo*> workerConnDetails(it->second,
                                                              connectionInfo);
        connections.insert(
                pair<size_t, pair<LocationInfo, ConnectionInfo*> >(
                        it->first, workerConnDetails));

        connectionInfo->switchDatabase(
                string("dbservice"),
                true /*createifnotexists*/,
                false /*showCommands*/,
                true /*forceExec*/);

        if(!startServersOnWorker(connectionInfo))
        {
            // TODO more descriptive error message (host, port, etc)
            throw new SecondoException("could not start file transfer server");
        }
    }

    DBServicePersistenceAccessor::restoreRelationInfo(relations);
    DBServicePersistenceAccessor::restoreDerivateInfo(derivates);

    queue<pair<string, pair<ConnectionID, bool> > > mapping;
    DBServicePersistenceAccessor::restoreLocationMapping(mapping);

    while(!mapping.empty())
    {
        const auto& currentMapping = mapping.front();
        string objectId = currentMapping.first;
        DBServiceRelations::iterator it1 = relations.find(objectId);
        if(it1 != relations.end()){
            it1->second.addNode(currentMapping.second.first,
                               currentMapping.second.second);
        } else {
           DBServiceDerivates::iterator it2 = derivates.find(objectId);
           if(it2!=derivates.end()){
              it2->second.addNode(
                    currentMapping.second.first,
                    currentMapping.second.second);
           } else {
              print("found location for object with id " + objectId + 
                    " but there is neigther a relation nor a derivate "
                    " stored with this id.");
           }
        }
        mapping.pop();
    }

    for(map<string, RelationInfo>::const_iterator it = relations.begin();
            it != relations.end(); it++)
    {
        print("RelationID: ", it->first);
        print("Number of Replicas: ",
                const_cast<RelationInfo*>(&(it->second))->getNodeCount());
    }
}

DBServiceManager* DBServiceManager::getInstance()
{
    printFunction("DBServiceManager::getInstance");
    if (!_instance)
    {
        _instance = new DBServiceManager();
    }
    active = true;
    return _instance;
}

bool DBServiceManager::isActive()
{
    return active;
}

bool DBServiceManager::isUsingIncrementalMetadataUpdate()
{
    return usesIncrementalMetadataUpdate;
}

void DBServiceManager::useIncrementalMetadataUpdate(bool use)
{
    usesIncrementalMetadataUpdate = use;
}


ConnectionID DBServiceManager::getNextFreeConnectionID()
{
    printFunction("DBServiceManager::getNextFreeConnectionID");
    return connections.size() + 1;
}

bool DBServiceManager::addNode(const string host,
        const int port,
        string config)
{
    printFunction("DBServiceManager::addNode");
    boost::lock_guard<boost::mutex> lock(managerMutex);

    for(const auto& connection : connections)
    {
        if(connection.second.first.isSameWorker(
                host, stringutils::int2str(port)))
        {
            return false;
        }
    }

    ConnectionInfo* connectionInfo =
            ConnectionInfo::createConnection(host, port, config);

    if(!connectionInfo)
    {
        print("Could not connect to remote server");
        return false;
    }

    connectionInfo->switchDatabase(
            string("dbservice"),
            true /*createifnotexists*/,
            false /*showCommands*/,
            true /*forceExec*/);

    // retrieve location parameters from worker
    string dir;
    if(!getConfigParamFromWorker(dir, connectionInfo,
            "Environment", "SecondoHome"))
    {
        print("could not retrieve SecondoHome of remote host");
        return false;
    }

    string commPort;
    if(!getConfigParamFromWorker(commPort, connectionInfo,
            "DBService", "CommunicationPort"))
    {
        print("could not retrieve CommunicationPort of remote host");
        return false;
    }

    string transferPort;
    if(!getConfigParamFromWorker(transferPort, connectionInfo,
            "DBService", "FileTransferPort"))
    {
        print("could not retrieve FileTransferPort of remote host");
        return false;
    }

    if(!startServersOnWorker(connectionInfo))
    {
        print("could not start servers on worker");
        return false;
    }

    LocationInfo location(host, stringutils::int2str(port), config, dir,
            commPort, transferPort);

    pair<LocationInfo, ConnectionInfo*> workerConnDetails(location,
                                                          connectionInfo);
    ConnectionID connID = getNextFreeConnectionID();
    connections.insert(
            pair<size_t, pair<LocationInfo, ConnectionInfo*> >(
                    connID, workerConnDetails));

    //DBServicePersistenceAccessor::persistLocationInfo(connID, location);

    if (!DBServicePersistenceAccessor::persistAllLocations(connections))
    {
        print("Could not persist DBService locations");
        connections.erase(connID);
        return false;
    }

    for(auto& replicaLocation : possibleReplicaLocations)
    {
        addToPossibleReplicaLocations(
                connID,
                location,
                replicaLocation.second,
                host,
                dir);
    }

    return true;
}

bool DBServiceManager::startServersOnWorker(
        distributed2::ConnectionInfo* connectionInfo)
{
    printFunction("DBServiceManager::startServersOnWorker");
    string queryInit("query initdbserviceworker()");
    print("queryInit", queryInit);

    return SecondoUtilsRemote::executeQuery(connectionInfo, queryInit);
}

bool DBServiceManager::getConfigParamFromWorker(string& result,
        distributed2::ConnectionInfo* connectionInfo, const char* section,
        const char* key)
{
    printFunction("DBServiceManager::getConfigParamFromWorker");
    string resultAsString;
    stringstream query;
    query << "query getconfigparam(\""
          << section
          << "\", \""
          << key
          << "\")";
    bool resultOk = SecondoUtilsRemote::executeQuery(
            connectionInfo,
            query.str(),
            resultAsString);
    print("resultAsString", resultAsString);

    if(!resultOk)
    {
        print("error during remote query execution");
        return false;
    }

    ListExpr resultAsNestedList;
    nl->ReadFromString(resultAsString, resultAsNestedList);
    result.assign(nl->StringValue(nl->Second(resultAsNestedList)));
    print("result", result);

    return result.size() != 0;
}

void DBServiceManager::determineReplicaLocations(const string& databaseName,
                                         const string& relationName,
                                         const string& host,
                                         const string& port,
                                         const string& disk)
{
    printFunction("DBServiceManager::determineReplicaLocations");
    boost::lock_guard<boost::mutex> lock(managerMutex);
    RelationInfo relationInfo(databaseName,
                              relationName,
                              host,
                              port,
                              disk);

    vector<ConnectionID> locations;
    getWorkerNodesForReplication(locations,
                                 host,
                                 disk);
    relationInfo.addNodes(locations);
    relations.insert(pair<string, RelationInfo>(relationInfo.toString(),
            relationInfo));
}



string DBServiceManager::determineDerivateLocations(
                                         const string& targetname,
                                         const string& relationId,
                                         const string& fundef)
{
    printFunction("DBServiceManager::determineDerivateLocations");
    boost::lock_guard<boost::mutex> lock(managerMutex);
    DerivateInfo derivateInfo(targetname, relationId, fundef);
    ReplicaLocations locations;
    getReplicaLocations(relationId, locations);
    vector<ConnectionID> v;
    for(const auto& loc : locations)
    {
       v.push_back(loc.first);
    }
    derivateInfo.addNodes(v);
    string id = derivateInfo.toString();
    derivates.insert( pair<string, DerivateInfo>(id, derivateInfo));
    return id;
}

void DBServiceManager::deleteReplicaLocations(const string& databaseName,
                                              const string& relationName)
{
    boost::lock_guard<boost::mutex> lock(managerMutex);
    relations.erase(RelationInfo::getIdentifier(databaseName, relationName));
}


void DBServiceManager::deleteDerivateLocations(const string& objectId)
{
    boost::lock_guard<boost::mutex> lock(managerMutex);
    derivates.erase(objectId);
}

void DBServiceManager::persistReplicaLocations(const string& databaseName,
                                               const string& relationName)
{
    boost::lock_guard<boost::mutex> lock(managerMutex);
    RelationInfo relationInfo =
            relations.at(
                    RelationInfo::getIdentifier(databaseName, relationName));
    //DBServicePersistenceAccessor::persistRelationInfo(relationInfo);
    if(!DBServicePersistenceAccessor::persistAllReplicas(relations, derivates))
    {
        print("Could not persist DBService relations");
    }
}

void DBServiceManager::persistDerivateLocations(const string& ObjectId)
{
    boost::lock_guard<boost::mutex> lock(managerMutex);
    DerivateInfo derivateInfo = derivates.at(ObjectId);
    //DBServicePersistenceAccessor::persistDerivateInfo(derivateInfo);
    if(!DBServicePersistenceAccessor::persistAllReplicas(relations,derivates))
    {
        print("Could not persist DBService derivates");
    }
}


void DBServiceManager::getReplicaLocations(
        const string& relationAsString,
        ReplicaLocations& ids)
{
    printFunction("DBServiceManager::getReplicaLocations");
    try
    {
        RelationInfo& relInfo = getRelationInfo(relationAsString);
        ids.insert(ids.begin(), relInfo.nodesBegin(), relInfo.nodesEnd());
    }catch(...)
    {
        print("RelationInfo does not exist");
    }
}


void DBServiceManager::getDerivateLocations(
        const string& objectId,
        ReplicaLocations& ids)
{
    printFunction("DBServiceManager::getDerivateLocations");
    try
    {
        DerivateInfo& derInfo = getDerivateInfo(objectId);
        ids.insert(ids.begin(), derInfo.nodesBegin(), derInfo.nodesEnd());
    }catch(...)
    {
        print("DerivateInfo does not exist");
    }
}



void DBServiceManager::addToPossibleReplicaLocations(
        const ConnectionID connectionID,
        const LocationInfo& location,
        vector<ConnectionID>& potentialReplicaLocations,
        const string& host,
        const string& disk)
{
    switch(mode)
    {
    case FaultToleranceMode::DISK:
        if(!location.isSameDisk(host, disk))
        {
            potentialReplicaLocations.push_back(connectionID);
        }
        break;
    case FaultToleranceMode::NODE:
        if(!location.isSameHost(host))
        {
            potentialReplicaLocations.push_back(connectionID);
        }
        break;
    case FaultToleranceMode::NONE:
    default:
        potentialReplicaLocations.push_back(connectionID);
        break;
    }
}

void DBServiceManager::getWorkerNodesForReplication(
        vector<ConnectionID>& nodes,
        const string& host,
        const string& disk)
{
    printFunction("DBServiceManager::getWorkerNodesForReplication");
    string locationID = LocationInfo::getIdentifier(host, disk);
    AlternativeLocations::const_iterator it =
            possibleReplicaLocations.find(locationID);
    if(it == possibleReplicaLocations.end())
    {
        vector<ConnectionID> potentialReplicaLocations;
        for(const auto& connection : connections)
        {
            const LocationInfo& location = connection.second.first;
            addToPossibleReplicaLocations(
                    connection.first,
                    location,
                    potentialReplicaLocations,
                    host,
                    disk);
        }
        possibleReplicaLocations.insert(
                pair<string, vector<ConnectionID> >(
                        locationID, potentialReplicaLocations));
        it = possibleReplicaLocations.find(locationID);
    }
    vector<ConnectionID>& connectionsForLocation =
            const_cast<vector<ConnectionID>& >(it->second);
    random_shuffle(
            connectionsForLocation.begin(),
            connectionsForLocation.end());

    nodes.insert(nodes.begin(),
            connectionsForLocation.begin(),
            connectionsForLocation.end());

    if(connectionsForLocation.size() > replicaCount)
    {
        nodes.resize(replicaCount);
    }
}

ConnectionInfo* DBServiceManager::getConnection(ConnectionID id)
{
    printFunction("DBServiceManager::getConnection");
    return connections.at(id).second;
}

LocationInfo& DBServiceManager::getLocation(ConnectionID id)
{
    printFunction("DBServiceManager::getLocation");
    return connections.at(id).first;
}

RelationInfo& DBServiceManager::getRelationInfo(const string& relationAsString)
{
    printFunction("DBServiceManager::getRelationInfo");
    print("relationAsString", relationAsString);
    return relations.at(relationAsString);
}

DerivateInfo& DBServiceManager::getDerivateInfo(const string& objectId)
{
    printFunction(__PRETTY_FUNCTION__);
    print("objectId", objectId);
    return derivates.at(objectId);
}

void DBServiceManager::printDerivates() const{
   printFunction(__PRETTY_FUNCTION__);
   print("available derivates:", stringutils::int2str(derivates.size()));
   for(auto& t : derivates){
      print(t.first);
      print(t.second);
   }
   print("---------------");
}


void DBServiceManager::maintainSuccessfulReplication(
        const string& relID,
        const string& replicaLocationHost,
        const string& replicaLocationPort)
{
    printFunction("DBServiceManager::maintainSuccessfulReplication");
    boost::lock_guard<boost::mutex> lock(managerMutex);
    print("relID", relID);
    print("replicaLocationHost", replicaLocationHost);
    print("replicaLocationPort", replicaLocationPort);

    try
    {
        RelationInfo& relInfo = getRelationInfo(relID);

        for(ReplicaLocations::const_iterator it
                = relInfo.nodesBegin(); it != relInfo.nodesEnd(); it++)
        {
            LocationInfo& location = getLocation(it->first);
            if(location.isSameWorker(replicaLocationHost, replicaLocationPort))
            {
                relInfo.updateReplicationStatus(it->first, true);
//                if(!DBServicePersistenceAccessor::updateLocationMapping(
//                        relID,
//                        it->first,
//                        true))
//                {
//                    print("Could not update location mapping");
//                }
                break;
            }
        }
    }catch(...)
    {
        print("RelationInfo does not exist");
    }
    if(!DBServicePersistenceAccessor::persistAllReplicas(relations, derivates))
    {
        print("Could not persist DBService relations");
    }
}


void DBServiceManager::maintainSuccessfulDerivation(
        const string& objectID,
        const string& replicaLocationHost,
        const string& replicaLocationPort)
{
    printFunction("DBServiceManager::maintainSuccessfulDerivation");
    boost::lock_guard<boost::mutex> lock(managerMutex);
    print("ObjectID", objectID);
    print("replicaLocationHost", replicaLocationHost);
    print("replicaLocationPort", replicaLocationPort);
    try
    {
        DerivateInfo& derInfo = getDerivateInfo(objectID);

        for(ReplicaLocations::const_iterator it
                = derInfo.nodesBegin(); it != derInfo.nodesEnd(); it++)
        {
            LocationInfo& location = getLocation(it->first);
            if(location.isSameWorker(replicaLocationHost, replicaLocationPort))
            {
                derInfo.updateReplicationStatus(it->first, true);
//                if(!DBServicePersistenceAccessor::updateLocationMapping(
//                        objectID,
//                        it->first,
//                        true))
//                {
//                    print("Could not update location mapping");
//                }
                break;
            }
        }
    }catch(...)
    {
        print("DerivateInfo does not exist");
    }
    if(!DBServicePersistenceAccessor::persistAllReplicas(relations,derivates))
    {
        print("Could not persist DBService derivates");
    }
}



void DBServiceManager::deleteReplicaMetadata(const string& database,
                                             const string& relation,
                                             const string& derivateName)
{
    printFunction("DBServiceManager::deleteReplicaMetadata");

    boost::lock_guard<boost::mutex> lock(managerMutex);
    try
    { 
  //        DBServicePersistenceAccessor::deleteRelationInfo(
  //                getRelationInfo(relID));
        if(derivateName.empty()){
            string relId = RelationInfo::getIdentifier(database, relation);
            RelationInfo relinfo = getRelationInfo(relId);
            DBServicePersistenceAccessor::deleteLocationMapping(relId,
                                     relinfo.nodesBegin(), relinfo.nodesEnd());
            relations.erase(relId);
            possibleReplicaLocations.erase(relId);
            vector<string> derivatestoremove;
            findDerivates(database, relation, derivatestoremove);
            for( auto& t : derivatestoremove){
               DerivateInfo derinfo = getDerivateInfo(t);
               DBServicePersistenceAccessor::deleteLocationMapping(t, 
                                        derinfo.nodesBegin(), 
                                        derinfo.nodesEnd());
               possibleReplicaLocations.erase(t);
               derivates.erase(t);
            }
        } else {
            string derId = DerivateInfo::getIdentifier(database, relation, 
                                                       derivateName);
            DerivateInfo derinfo = getDerivateInfo(derId);
            DBServicePersistenceAccessor::deleteLocationMapping(derId, 
                                         derinfo.nodesBegin(), 
                                         derinfo.nodesEnd());
            derivates.erase(derId);
            possibleReplicaLocations.erase(derId);
        }
    }catch(...)
    {
        print("RelationInfo does not exist");
    }
    if(!derivateName.empty())
    {
      if(!DBServicePersistenceAccessor::persistAllReplicas(relations,derivates))
      {
        print("Could not persist DBService relations");
      }
    }
    if(!DBServicePersistenceAccessor::persistAllReplicas(relations,derivates))
    {
        print("Could not persist DBService derivates");
    }
}



void DBServiceManager::printMetadata()
{
    cout << "********************************" << endl;
    cout << "* WORKER NODES                 *" << endl;
    cout << "********************************" << endl;
    if(connections.empty())
    {
        cout << "*** NONE ***" << endl;
    }
    for(const auto& connection : connections)
    {
        cout << "ConnectionID:\t" << connection.first << endl;
        printLocationInfo(connection.second.first);
        cout << endl;
    }
    cout << "********************************" << endl;
    cout << "* REPLICAS                     *" << endl;
    cout << "********************************" << endl;
    if(relations.empty())
    {
        cout << "*** NONE ***" << endl;
    }
    for(const auto& relation : relations)
    {
        cout << "RelationID:\t" << relation.first << endl;
        printRelationInfo(relation.second);
        cout << endl;
    }
    cout << "********************************" << endl;
    cout << "* DERIVATES                    *" << endl;
    cout << "********************************" << endl;
    if(derivates.empty())
    {
        cout << "*** NONE ***" << endl;
    }
    for(const auto& derivate : derivates)
    {
        cout << "ObjectID:\t" << derivate.first << endl;
        printDerivateInfo(derivate.second);
        cout << endl;
    }
}

bool DBServiceManager::replicaExists(
        const string& databaseName,
        const string& relationName)
{
    DBServiceRelations::const_iterator it =
            relations.find(RelationInfo::getIdentifier(
                    databaseName,
                    relationName));
    return it != relations.end();
}

bool DBServiceManager::derivateExists(const string& objectId)
{
    DBServiceDerivates::const_iterator it =
            derivates.find( objectId);
    return it != derivates.end();
}

bool DBServiceManager::locationExists(
                  const string& databaseName,
                  const string& relation,
                  const vector<string>& derivates){

    string relId = RelationInfo::getIdentifier(databaseName, relation);
    DBServiceRelations::const_iterator rit = relations.find(relId);
    if(rit==relations.end()){
        return false;
    }
    vector<int> locations;
    ReplicaLocations::const_iterator nit;
    for(nit=rit->second.nodesBegin(); nit!=rit->second.nodesEnd();nit++){
         if(nit->second){ // replicated flag set
            locations.push_back(nit->first);
         }
    }

    sort(locations.begin(), locations.end());
    
    for( auto& der : derivates){
      string derId = RelationInfo::getIdentifier(relId, der);
      DBServiceDerivates::const_iterator dit = this->derivates.find(derId);
      if(dit==this->derivates.end()){
         return false; // derived object not managed
      }
      // collect all replica locations of this derived object
      vector<int> dlocs;
      ReplicaLocations::const_iterator nit;
      for(nit = dit->second.nodesBegin(); nit!=dit->second.nodesEnd(); nit++){
          if(nit->second){
            dlocs.push_back(nit->first);
          }
      }
      sort(dlocs.begin(), dlocs.end());
      vector<int> tmp;
      set_intersection(locations.begin(), locations.end(),
                       dlocs.begin(), dlocs.end(),
                       back_inserter(tmp));
      if(tmp.empty()){
        return false;
      } 
      swap(locations,tmp);
    }
    return true;

}

void DBServiceManager::findDerivates(const string& relID,
                                     vector<string>& result){

  result.clear();
  for(auto& t : derivates){
        if(t.second.getSource()==relID){
           result.push_back(t.first);
        }
  }

}  

void DBServiceManager::setOriginalLocationTransferPort(
        const string& relID,
        const string& transferPort)
{
    DBServiceRelations::const_iterator it =
            relations.find(relID);
    if(it != relations.end())
    {
        RelationInfo& relInfo = getRelationInfo(relID);
        relInfo.setTransferPortOfOriginalLocation(
                *(const_cast<string*>(&transferPort)));
    }
}

DBServiceManager* DBServiceManager::_instance = nullptr;
bool DBServiceManager::active = false;
bool DBServiceManager::usesIncrementalMetadataUpdate = false;

} /* namespace DBService */
