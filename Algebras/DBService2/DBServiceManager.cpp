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
#include "StandardTypes.h"
#include "FileSystem.h"

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
        LOG_SCOPE_FUNCTION(INFO);

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

        loguru::set_thread_name("DBServiceManager");

    }

    DBServiceManager::~DBServiceManager()
    {
        printFunction("DBServiceManager::~DBServiceManager", std::cout);
        LOG_SCOPE_FUNCTION(INFO);
    }

    void DBServiceManager::restoreConfiguration()
    {
        printFunction("DBServiceManager::restoreConfiguration", std::cout);
        LOG_SCOPE_FUNCTION(INFO);
        
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
        LOG_SCOPE_FUNCTION(INFO);

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


    bool DBServiceManager::addNode(const string host,
        const int port,
        string configPath)
    {
        printFunction("DBServiceManager::addNode", std::cout);
        LOG_SCOPE_FUNCTION(INFO);

        boost::lock_guard<boost::mutex> lock(managerMutex);

        if (!FileSystem::FileOrFolderExists(configPath)) {
            print("The given DBService config file does not exist.", std::cout);
            LOG_F(ERROR, "The given DBService config file does not exist.");
            
            return 0;
        }

        CcString config = CcString(true, configPath);

        // Establishing a connection to the remote node to retrieve 
        // config values

        print("Establishing a connection to worker node", std::cout);
        print("\tHost", host, std::cout);
        print("\tPort", port, std::cout);
        LOG_F(INFO, "Establishing a connection to a worker node");
        LOG_F(INFO, "Node host: %s, port: %d", host.c_str(), port);


        shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
            host, port, config.getCsvStr());

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

        print("The DBServiceManager is done adding the node.", std::cout);
        LOG_F(INFO, "The DBServiceManager is done adding the node.");

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
        LOG_SCOPE_FUNCTION(INFO);

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
                
        LOG_F(INFO, "Placement is done. Placement message: %s.",
            replicaPlacementStrategy->getMessage().c_str());

        if(success) {
            relationManager->add(relation);

            stringstream msg;
            msg << *relation;

            print("Relation after adding to relationManager: ", 
                msg.str(), std::cout);

            LOG_F(INFO, "Relation after adding to relationManager: %s",
                msg.str().c_str());

            // Resetting the stringstream
            msg.str("");
            msg.clear();

            msg << relationManager->str();
            print("RelationManager bevore saving: ", msg.str(), std::cout);
            LOG_F(INFO, "RelationManager bevore saving: %s", msg.str().c_str());

            relationManager->save();
        }

        return success;
    }

    shared_ptr<DBService::Relation> DBServiceManager::getRelation(
        string relationDatabase, string relationName) {        

        return relationManager->findByDatabaseAndName(
            relationDatabase, relationName);
    }

    void DBServiceManager::printDerivates(std::ostream& out) const {
        printFunction(__PRETTY_FUNCTION__, out);
        LOG_SCOPE_FUNCTION(INFO);

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

        LOG_SCOPE_FUNCTION(INFO);

        boost::lock_guard<boost::mutex> lock(managerMutex);
        print("relID", relID, std::cout);
        LOG_F(INFO, "reldID: %s", relID.c_str());

        print("replicaLocationHost", replicaTargetNodeHost, std::cout);
        print("replicaLocationPort", replicaTargetNodePort, std::cout);
        LOG_F(INFO, "Replica target node: (%s, %s)", 
            replicaTargetNodeHost.c_str(), replicaTargetNodePort.c_str());

        string relationDatabase;
        string relationName;

        //TODO Refactor -> Eliminate dependency to RelationInfo
        RelationInfo::parseIdentifier(relID, relationDatabase, relationName);


        shared_ptr<DBService::Relation> relation = getRelation(relationDatabase,
            relationName);

        if(relation == nullptr) {
            print("Relation does not exist", std::cout);
            LOG_F(WARNING, "Relation does not exist.");
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
        LOG_SCOPE_FUNCTION(INFO);

        boost::lock_guard<boost::mutex> lock(managerMutex);

        print("ObjectID", objectID, std::cout);
        print("replicaLocationHost", replicaLocationHost, std::cout);
        print("replicaLocationPort", replicaLocationPort, std::cout);

        LOG_F(INFO, "ObjectID: %s", objectID.c_str());
        LOG_F(INFO, "Replica target node: (%s, %s)",
            replicaLocationHost.c_str(), replicaLocationPort.c_str());

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
        LOG_SCOPE_FUNCTION(INFO);

        print("Database", database, std::cout);
        print("Relation", relation, std::cout);
        print("Derivative", derivateName, std::cout);
        LOG_F(INFO, "Database: %s", database.c_str());
        LOG_F(INFO, "Relation: %s", relation.c_str());
        LOG_F(INFO, "Derivative name: %s", derivateName.c_str());

        boost::lock_guard<boost::mutex> lock(managerMutex);

        try
        {
            if(relation.empty()) {
                //JF: If no relation is provided, delete the entire db

                print("deleteRelationsByRelationDatabase", std::cout);
                LOG_F(INFO, "deleteRelationsByRelationDatabase");

                relationManager->deleteRelationsByRelationDatabase(database);

            }
            else if(derivateName.empty()) {
                //JF: If a relation is provided but no derivativeName,
                //  delete the entire relation

                print("deleteRelationByDatabaseAndName", std::cout);
                LOG_F(INFO, "deleteRelationByDatabaseAndName");

                relationManager->deleteRelationByDatabaseAndName(database,
                    relation);                        
            }
            else {

                print("deleteDerivativeByName", std::cout);
                LOG_F(INFO, "deleteDerivativeByName");
                
                relationManager->deleteDerivativeByName(database, relation,
                    derivateName);
            }
        }        
catch(...)
        {
            print("Object to remove not found", std::cout);
            LOG_F(ERROR, "Object to remove not found!");
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
        bool doesExist = false;

        doesExist = relationManager->doesDerivativeExist(derivativeName);

        return doesExist;
    }

    void DBServiceManager::addDerivative(std::string relationDatabase,
        std::string relationName, std::string derivativeName,
        std::string derivativeFunction) {
    
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

    shared_ptr<DBService::Node> DBServiceManager::getRandomNodeWithReplica(
        string relationDatabase, string relationName) {

        printFunction("DBServiceManager::getRandomNodeWithReplica", std::cout);
        LOG_SCOPE_FUNCTION(INFO);

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
        LOG_F(INFO, "Replica: %s", replica->str().c_str());

        int targetNodeId = replica->getTargetNodeId();

        print("TargetNodeId: ", targetNodeId, std::cout);
        LOG_F(INFO, "TargetNodeId: %d", targetNodeId);

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
        LOG_F(INFO, "Target node: %s", targetNode->str().c_str());

        return targetNode;
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
