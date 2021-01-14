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
#include <random>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>

#include <boost/make_shared.hpp>
#include <boost/asio.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <loguru.cc>

#include "Algebra.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoException.h"
#include "SecParser.h"
#include "StringUtils.h"

#include "Algebras/Distributed2/ConnectionInfo.h"

#include "Algebras/DBService2/CommunicationServer.hpp"
#include "Algebras/DBService2/DBServiceManager.hpp"
#include "Algebras/DBService2/DBServicePersistenceAccessor.hpp"
#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/RelationInfo.hpp"
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/SecondoUtilsLocal.hpp"
#include "Algebras/DBService2/SecondoUtilsRemote.hpp"
#include "Algebras/DBService2/ServerRunnable.hpp"
#include "Algebras/DBService2/ReplicationUtils.hpp"

#include "Algebras/DBService2/DatabaseSchema.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#include "Algebras/DBService2/CreateDerivateRunnable.hpp"


using namespace std;
using namespace distributed2;

namespace DBService
{

    DBServiceManager::DBServiceManager()
    {
        printFunction("DBServiceManager::DBServiceManager", std::cout);
        boost::lock_guard<boost::mutex> lock(managerMutex);

        restoreConfiguration();

        //TODO Make configurable
        database = DatabaseEnvironment::production;

        //TODO Create the DBService database if that's not the current database.

        DatabaseSchema::migrate(database);

        nodeManager = make_unique<NodeManager>(database);
        nodeManager->load();

        relationManager = make_unique<RelationManager>(database);
        relationManager->load();

        restoreReplicaPlacementStrategyConfig();

        // TODO restoreReplicaInformation used to start worker nodes
        //  Where is this done now?

        // restoreReplicaInformation();

        //TODOMove to function "initializeLogger"
        char* argv[] = { (char*)"DBService", NULL };
        int argc = sizeof(argv) / sizeof(char*) - 1;

        loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
        loguru::init(argc, argv);

        loguru::add_file("dbservice.log", loguru::Append, 
            loguru::Verbosity_MAX);
    }

    DBServiceManager::~DBServiceManager()
    {
        printFunction("DBServiceManager::~DBServiceManager", std::cout);
    }

    void DBServiceManager::restoreConfiguration()
    {
        printFunction("DBServiceManager::restoreConfiguration", std::cout);
        string port;
        SecondoUtilsLocal::readFromConfigFile(
            port, "DBService", "DBServicePort", "9989");
        ServerRunnable commServer(atoi(port.c_str()));
        commServer.run<CommunicationServer>();

        string replicaNumber;
        SecondoUtilsLocal::readFromConfigFile(
            replicaNumber, "DBService", "ReplicaNumber", "1");
        replicaCount = atoi(replicaNumber.c_str());

        string faultToleranceMode;
        SecondoUtilsLocal::readFromConfigFile(
            faultToleranceMode, "DBService", "FaultToleranceLevel", "2");
        mode = static_cast<FaultToleranceMode>(atoi(
            faultToleranceMode.c_str()));
    }

    void DBServiceManager::restoreReplicaPlacementStrategyConfig() {

        PlacementPolicy placementPolicy = { 
            mode, static_cast<int>(replicaCount) 
        };

        replicaPlacementStrategy = make_shared<ReplicaPlacementStrategy>(
            placementPolicy, nodeManager->getNodes());
    }

    DBServiceManager* DBServiceManager::getInstance()
    {
        printFunction("DBServiceManager::getInstance", std::cout);
        if(!_instance)
        {
            try {
                _instance = new DBServiceManager();
                active = true;
            }
            catch(...) {
                _instance = 0;
                active = false;
            }
        }
        return _instance;
    }

    bool DBServiceManager::isActive()
    {
        //TODO es würde genügen _instance auf == 0 zu prüfen.
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
        printFunction("DBServiceManager::getNextFreeConnectionID", std::cout);
        return connections.size() + 1;
    }

    bool DBServiceManager::addNode(const string host,
        const int port,
        string config)
    {
        printFunction("DBServiceManager::addNode", std::cout);
        boost::lock_guard<boost::mutex> lock(managerMutex);

        // Establishing a connection to the remote node to retrieve 
        // config values

        print("Establishing a connection to worker node", std::cout);
        print("\tHost", host, std::cout);
        print("\tPort", port, std::cout);

        shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
            host, port, config);

        /* Connect to the node and obtain missing params such as
         * comPort
         * transferPort and
         * diskPath
         */
        node->connectAndConfigure();

        // Adding the node to the in-memory store.
        // This also checks if the node has already been added.
        // Does silently ignore duplicate nodes.
        nodeManager->add(node);

        // Starting the DBService Worker.
        node->startWorker();

        // Persisting the new node
        nodeManager->save();

        //TODO What is any of the above step fails? 
        //  Add Exception handling, Retry or Removing of a failed node, ...

        print("The DBServiceManager is done adding node.", std::cout);

        return true;
    }

    bool DBServiceManager::determineReplicaLocations(const string& databaseName,
        const string& relationName,
        const string& host,
        const string& port,
        const string& disk)
    {
        bool success = false;

        printFunction("DBServiceManager::determineReplicaLocations", std::cout);
        boost::lock_guard<boost::mutex> lock(managerMutex);
    
        shared_ptr<DBService::Relation> relation = DBService::Relation::build(
            string(databaseName), string(relationName),
            string(host), stoi(port), string(disk));

        relation->setDatabase(database);

        // See whether the original already exists
        shared_ptr<DBService::Node> originalNode =
            relationManager->findOriginalNode(relation->getOriginalNode());

        // To avoid a duplicate originalNode, reuse the existing one.
        if(originalNode != nullptr) {
            relation->setOriginalNode(originalNode);
        }

        /*
            The original node's information is incomplete.
            Config, diskPath, comPort and transferPort are missing.
            It must be retrieved.
        */
        relation->getOriginalNode()->connectAndConfigure();

        /*
         TODO Either add nodes to the signature of doPlacement or initalize
         the strategy with a shared_pointer to the node manager.

         The list of nodes must be up to date as it may change of time, e.g.
         due to addNode invocations.
         */
        replicaPlacementStrategy->setNodes(nodeManager->getNodes());

        success = replicaPlacementStrategy->doPlacement(relation);

        print("Placement done.\nPlacement message:\n", 
            replicaPlacementStrategy->getMessage(), std::cout);

        if(success) {
            relationManager->add(relation);

            stringstream msg;
            msg << *relation;

            print("Relation after adding to relationManager: ", 
                msg.str(), std::cout);

            // Resetting the stringstream
            msg.str("");
            msg.clear();

            msg << relationManager->str();
            print("RelationManager bevore saving: ", msg.str(), std::cout);

            relationManager->save();
        }

        return success;
    }



    string DBServiceManager::determineDerivateLocations(
        const string& targetname,
        const string& relationId,
        const string& fundef)
    {
        printFunction("DBServiceManager::determineDerivateLocations", 
            std::cout);
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
        derivates.insert(pair<string, DerivateInfo>(id, derivateInfo));
        return id;
    }

    //TODO Remove. Depricated.
    void DBServiceManager::deleteRelationLocations(const string& databaseName,
        const string& relationName)
    {
        //TODO JF This deletes from the vector, only. 
        // It doesn't delete from the database. 
        // Change the dbService function accordingly.
        boost::lock_guard<boost::mutex> lock(managerMutex);
        relations.erase(RelationInfo::getIdentifier(databaseName, 
            relationName));
    }


    void DBServiceManager::deleteDerivateLocations(const string& objectId)
    {
        boost::lock_guard<boost::mutex> lock(managerMutex);
        derivates.erase(objectId);
    }

    //TODO Remove. Depricated.
    void DBServiceManager::persistReplicaLocations(const string& databaseName,
        const string& relationName)
    {
        boost::lock_guard<boost::mutex> lock(managerMutex);
        RelationInfo relationInfo =
            relations.at(
                RelationInfo::getIdentifier(databaseName, relationName));

        // Store the relation
        DBServicePersistenceAccessor::persistRelationInfo(relationInfo);

        /*

            JF:
            the RelationInfo already exists. Where and when has it been created?

            RelationInfo(const std::string& dbName,
                     const std::string& relName,
                     const std::string& host,
                     const std::string& port,
                     const std::string& disk);

            ReplicaLocations nodes;

            LocationInfo originalLocation;

        */


        // Store the replica

        //TODO Change to persist only the single replica that is being handled 
        // here.
        //  --> There could be multiple replicas for a given database&relation
        // depending 
        //      on the number of replicas set in the configuration
        // -> Implement persistReplica(...)
        if(!DBServicePersistenceAccessor::persistAllReplicas(relations, 
            derivates))
        {
            print("Could not persist DBService relations", std::cout);
        }
    }

    //TODO Remove. Depricated.
    void DBServiceManager::persistDerivateLocations(const string& ObjectId)
    {
        // boost::lock_guard<boost::mutex> lock(managerMutex);
        // DerivateInfo derivateInfo = derivates.at(ObjectId);
        // //DBServicePersistenceAccessor::persistDerivateInfo(derivateInfo);
        // if(!DBServicePersistenceAccessor::persistAllReplicas(relations, 
        // derivates))
        // {
        //     print("Could not persist DBService derivates", std::cout);
        // }
    }


    void DBServiceManager::getReplicaLocations(
        const string& relationAsString,
        ReplicaLocations& ids)
    {
        printFunction("DBServiceManager::getReplicaLocations", std::cout);
        try
        {
            RelationInfo& relInfo = getRelationInfo(relationAsString);
            ids.insert(ids.begin(), relInfo.nodesBegin(), relInfo.nodesEnd());
        }        
catch(...)
        {
            print("RelationInfo does not exist", std::cout);
        }
    }


    void DBServiceManager::getDerivateLocations(
        const string& objectId,
        ReplicaLocations& ids)
    {
        printFunction("DBServiceManager::getDerivateLocations", std::cout);
        try
        {
            DerivateInfo& derInfo = getDerivateInfo(objectId);
            ids.insert(ids.begin(), derInfo.nodesBegin(), derInfo.nodesEnd());
        }        
catch(...)
        {
            print("DerivateInfo does not exist", std::cout);
        }
    }

    /*
        TODO Refactor and move!
        Preserve the semantics of the FaultToleranceMode values.

    */
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

    //TODO Remove. Depricated.
    void DBServiceManager::getWorkerNodesForReplication(
        vector<ConnectionID>& nodes,
        const string& host,
        const string& disk)
    {
        /* Given is an empty set of worker nodes meant to store the results of 
        * this operation.
        * The host and disk identify the host holding the original relation 
        * to be replicated.
        */
        printFunction("DBServiceManager::getWorkerNodesForReplication", 
            std::cout);
        
        string locationID = LocationInfo::getIdentifier(host, disk);

        // A location is considered a possible replica location if it has been
        // added during the addNode operation.
        AlternativeLocations::const_iterator it =
            possibleReplicaLocations.find(locationID);

        // If there is only one result ...
        if(it == possibleReplicaLocations.end())
        {
            // PotentialReplicaLocations are locations for which there 
            // is a connection.
            vector<ConnectionID> potentialReplicaLocations;

            // Connections is a vector of DBServiceLocations
            // DBServiceLocations is a map of ConnectionID -> {LocationInfo, 
            // ConnectionInfo}
            for(const auto& connection : connections)
            {
                // connection.second > pair LocationInfo
                // pair.first > ConnectionID
                const LocationInfo& location = connection.second.first;

                // this method modifies the potentialReplicaLocations vector
                addToPossibleReplicaLocations(
                    connection.first, // ConnectionID
                    location, // LocationInfo
                    potentialReplicaLocations, // vector
                    host,
                    disk);
            }

            // And here the vector is modifed too
            possibleReplicaLocations.insert(
                pair<string, vector<ConnectionID> >(
                    locationID, potentialReplicaLocations));

            it = possibleReplicaLocations.find(locationID);
        } // What if not?

        vector<ConnectionID>& connectionsForLocation =
            const_cast<vector<ConnectionID>&>(it->second);

        std::random_device rd;
        std::mt19937 g(rd());

        // Randomly realign elements of the connectionsForLocation vector
        // This will distribute replicas randomly across potential replica
        //  locations.
        std::shuffle(
            connectionsForLocation.begin(),
            connectionsForLocation.end(), g);

        // ReplicaLocations nodes;
        nodes.insert(nodes.begin(),
            connectionsForLocation.begin(),
            connectionsForLocation.end());

        // Resize the nodes vector if 0
        if(connectionsForLocation.size() > replicaCount)
        {
            nodes.resize(replicaCount);
        }
    }

    ConnectionInfo* DBServiceManager::getConnection(ConnectionID id)
    {
        printFunction("DBServiceManager::getConnection", std::cout);
        return connections.at(id).second;
    }

    LocationInfo& DBServiceManager::getLocation(ConnectionID id)
    {
        printFunction("DBServiceManager::getLocation", std::cout);
        return connections.at(id).first;
    }

    RelationInfo& DBServiceManager::getRelationInfo(
        const string& relationAsString)
    {
        printFunction("DBServiceManager::getRelationInfo", std::cout);
        print("relationAsString", relationAsString, std::cout);
        return relations.at(relationAsString);
    }

    shared_ptr<DBService::Relation> DBServiceManager::getRelation(
        string relationDatabase, string relationName) {
        return relationManager->findByDatabaseAndName(
            relationDatabase, relationName);
    }

    DerivateInfo& DBServiceManager::getDerivateInfo(const string& objectId)
    {
        printFunction(__PRETTY_FUNCTION__, std::cout);
        print("objectId", objectId, std::cout);
        return derivates.at(objectId);
    }

    void DBServiceManager::printDerivates(std::ostream& out) const {
        printFunction(__PRETTY_FUNCTION__, out);
        print("available derivates:", stringutils::int2str(
            derivates.size()), out);
        for(auto& t : derivates) {
            print(t.first, out);
            print(t.second, out);
        }
        print("---------------", out);
    }


    void DBServiceManager::maintainSuccessfulReplication(
        const string& relID,
        const string& replicaTargetNodeHost,
        const string& replicaTargetNodePort)
    {
        printFunction("DBServiceManager::maintainSuccessfulReplication", 
            std::cout);
        boost::lock_guard<boost::mutex> lock(managerMutex);
        print("relID", relID, std::cout);

        print("replicaLocationHost", replicaTargetNodeHost, std::cout);
        print("replicaLocationPort", replicaTargetNodePort, std::cout);


        string relationDatabase;
        string relationName;

        //TODO Refactor -> Eliminate dependency to RelationInfo
        RelationInfo::parseIdentifier(relID, relationDatabase, relationName);


        shared_ptr<DBService::Relation> relation = getRelation(relationDatabase,
            relationName);

        if(relation == nullptr) {
            print("Relation does not exist", std::cout);
            return;
        }

        relation->updateReplicaStatus(
            replicaTargetNodeHost, stoi(replicaTargetNodePort),
            Replica::statusReplicated);
    }


    void DBServiceManager::maintainSuccessfulDerivation(
        const string& objectID,
        const string& replicaLocationHost,
        const string& replicaLocationPort)
    {
        printFunction("DBServiceManager::maintainSuccessfulDerivation", 
            std::cout);

        boost::lock_guard<boost::mutex> lock(managerMutex);
        print("ObjectID", objectID, std::cout);
        print("replicaLocationHost", replicaLocationHost, std::cout);
        print("replicaLocationPort", replicaLocationPort, std::cout);

        // ObjectID example:
        // DBSTESTxDBSxCitiesR2xDBSxCitiesR2_count
        // relationID xDBSx derivativeID

        //TODO Move to utility function    
        std::vector<std::string> fragments;
        boost::regex separator("xDBSx");
        boost::algorithm::split_regex(fragments, objectID, separator);

        string relationDatabase = fragments[0];
        string relationName = fragments[1];
        string derivativeName = fragments[2];

        auto relation = relationManager->findByDatabaseAndName(
            relationDatabase, relationName);

        relation->updateDerivativeReplicaStatus(
            derivativeName,
            replicaLocationHost, stoi(replicaLocationPort),
            Replica::statusReplicated);
    }



    void DBServiceManager::deleteReplicaMetadata(const string& database,
        const string& relation,
        const string& derivateName)
    {
        printFunction("DBServiceManager::deleteReplicaMetadata", std::cout);
        print("Database", database, std::cout);
        print("Relation", relation, std::cout);
        print("Derivative", derivateName, std::cout);

        boost::lock_guard<boost::mutex> lock(managerMutex);
        try
        {
            if(relation.empty()) {
                //JF: If no relation is provided, delete the entire db

                print("deleteRelationsByRelationDatabase", std::cout);
                relationManager->deleteRelationsByRelationDatabase(database);

            }
            else if(derivateName.empty()) {
                //JF: If a relation is provided but no derivativeName,
                //  delete the entire relation

                print("deleteRelationByDatabaseAndName", std::cout);
                relationManager->deleteRelationByDatabaseAndName(database,
                    relation);                        
            }
            else {

                print("deleteDerivativeByName", std::cout);
                relationManager->deleteDerivativeByName(database, relation,
                    derivateName);
            }
        }        
catch(...)
        {
            print("Object to remove not found", std::cout);
        }
    }



    void DBServiceManager::printMetadata(std::ostream& out)
    {

        out << "**************************************" << endl;
        out << "* DBSERVICE WORKER NODES             *" << endl;
        out << "**************************************" << endl;

        if(nodeManager->empty())
        {
            out << "*** NONE ***" << endl;
        }

        for(const auto& node : nodeManager->getAll())
        {
            out << node->str() << endl;
        }

        out << "**************************************" << endl;
        out << "* RELATIONS & REPLICAS & DERIVATIVES *" << endl;
        out << "**************************************" << endl;

        if(relationManager->empty())
        {
            out << "*** NONE ***" << endl;
        }

        for(const auto& relation : relationManager->getAll())
        {
            out << "Relation:" << endl;
            out << *relation << endl;
        }
    }

    bool DBServiceManager::replicaExists(
        const string& databaseName,
        const string& relationName)
    {
        return relationManager->doesRelationHaveReplicas(databaseName, 
            relationName);
    }

    bool DBServiceManager::derivateExists(const string& derivativeName)
    {
        // DBServiceDerivates::const_iterator it =
        //         derivates.find( objectId);
        // return it != derivates.end();

        bool doesExist = false;

        doesExist = relationManager->doesDerivativeExist(derivativeName);

        return doesExist;
    }

    void DBServiceManager::addDerivative(std::string relationDatabase,
        std::string relationName, std::string derivativeName,
        std::string derivativeFunction) {

        // TODO Move to DBServiceManager where a relationManager is present
        auto relation = relationManager->findByDatabaseAndName(
            relationDatabase, relationName);

        auto derivative = relation->addDerivative(derivativeName, 
            derivativeFunction);

        relation->save();

        //TODO make the creation of the processes a control-loop

        for(auto& replica : derivative->getReplicas()) {
            auto targetNode = replica->getTargetNode();

            CreateDerivateRunnable cdr(
                targetNode->getHost().getHostname(),
                targetNode->getComPort(),
                relationDatabase,
                derivativeName,
                relationName,
                derivativeFunction);
            cdr.run();
        }
    }

    //TODO Depricated. Remove
    bool DBServiceManager::locationExists(
        const string& databaseName,
        const string& relation,
        const vector<string>& derivates) {

        string relId = RelationInfo::getIdentifier(databaseName, relation);
        DBServiceRelations::const_iterator rit = relations.find(relId);
        if(rit == relations.end()) {
            return false;
        }
        vector<int> locations;
        ReplicaLocations::const_iterator nit;
        for(nit = rit->second.nodesBegin(); 
            nit != rit->second.nodesEnd(); nit++) {
            if(nit->second) { // replicated flag set
                locations.push_back(nit->first);
            }
        }

        sort(locations.begin(), locations.end());

        for(auto& der : derivates) {
            string derId = RelationInfo::getIdentifier(relId, der);
            DBServiceDerivates::const_iterator dit = 
                this->derivates.find(derId);
            if(dit == this->derivates.end()) {
                return false; // derived object not managed
            }
            // collect all replica locations of this derived object
            vector<int> dlocs;
            ReplicaLocations::const_iterator nit;
            for(nit = dit->second.nodesBegin(); 
                nit != dit->second.nodesEnd(); nit++) {
                if(nit->second) {
                    dlocs.push_back(nit->first);
                }
            }
            sort(dlocs.begin(), dlocs.end());
            vector<int> tmp;
            set_intersection(locations.begin(), locations.end(),
                dlocs.begin(), dlocs.end(),
                back_inserter(tmp));
            if(tmp.empty()) {
                return false;
            }
            swap(locations, tmp);
        }
        return true;

    }

    shared_ptr<DBService::Node> DBServiceManager::getRandomNodeWithReplica(
        string relationDatabase, string relationName) {

        printFunction("DBServiceManager::getRandomNodeWithReplica", std::cout);

        auto replica = relationManager->getRandomReplica(
            relationDatabase, relationName);

        if(replica == nullptr) {
            LOG_F(WARNING, "Didn't find Replica for db (%s) and relation (%s)",
                relationDatabase.c_str(),
                relationName.c_str()
            );
            return nullptr;
        }

        print("Replica: ", replica->str(), std::cout);

        int targetNodeId = replica->getTargetNodeId();

        print("TargetNodeId: ", targetNodeId, std::cout);

        // The goal of using the nodeManager is to avoid
        // loading nodes multiple times and thus producing in-memory duplicates
        // In this case an in-mem dup wouldn't do any harm, though.
        auto targetNode = nodeManager->findById(targetNodeId);

        if(targetNode == nullptr) {
            LOG_F(WARNING, "Didn't find target node for replica of db(%s) / \
relation (%s)",
                relationDatabase.c_str(),
                relationName.c_str()
            );
            return nullptr;
        }

        print("TargetNode: ", targetNode->str(), std::cout);

        return targetNode;
    }


    void DBServiceManager::findRelations(const string& databaseName,
        vector<string>& result) {


        result.clear();
        string start = ReplicationUtils::getDBStart(databaseName);
        DBServiceRelations::const_iterator it = relations.lower_bound(start);
        while(it != relations.end() && it->first.find(start) == 0) {
            result.push_back(it->first);
            it++;
        }

    }


    void DBServiceManager::findDerivates(const string& relID,
        vector<string>& result) {

        result.clear();
        for(auto& t : derivates) {
            if(t.second.getSource() == relID) {
                result.push_back(t.first);
            }
        }

    }

    void DBServiceManager::findDerivatesInDatabase(
        const std::string& databaseName,
        std::vector<std::string>& result) {
        result.clear();
        string start = ReplicationUtils::getDBStart(databaseName);
        DBServiceDerivates::const_iterator it = derivates.lower_bound(start);
        while(it != derivates.end() && it->first.find(start) == 0) {
            result.push_back(it->first);
            it++;
        }

    }

    string DBServiceManager::getMessages() {
        if(replicaPlacementStrategy != nullptr) {
            return replicaPlacementStrategy->getMessage();
        }
        return "";
    }

    DBServiceManager* DBServiceManager::_instance = nullptr;
    bool DBServiceManager::active = false;
    bool DBServiceManager::usesIncrementalMetadataUpdate = false;

} /* namespace DBService */
