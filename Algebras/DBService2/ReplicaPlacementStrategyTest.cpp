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

// Avoid conflict with loguru.hpp
//#undef INFO

#include "catch.hh" // https://github.com/catchorg/Catch2

#include "Algebras/DBService2/ReplicaPlacementStrategy.hpp"
#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/NodeManager.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#include "Algebras/DBService2/SecondoRelationAdapter.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/FaultToleranceMode.hpp"
#include "Algebras/DBService2/PlacementPolicy.hpp"
#include "Algebras/DBService2/DatabaseSchema.hpp"
#include "Algebras/DBService2/NodeManagerTestFactory.hpp"
#include "Algebras/DBService2/RelationTestFactory.hpp"

#include <string>
#include <vector>

using namespace DBService;
using namespace DBServiceTest;
using namespace std;

using Catch::Matchers::Contains;

TEST_CASE("Testing ReplicaPlacementStrategy") {
  
  const string test_db_name = DatabaseEnvironment::test;   
  DatabaseSchema::migrate(test_db_name);

  SECTION("Fault Tolerance Mode NODE") {
    SECTION("Simulate a successful 3 Node placement in Fault Tolerance Mode \
NODE") {

      // Create NodeManager and Nodes
      // The nodeManager will load dbservice nodes, only.
      NodeManager nodeManager(test_db_name, 
        DBService::Node::nodeTypeDBService());
      nodeManager.load();

      REQUIRE( nodeManager.size() >= 3);
      
      vector<shared_ptr<DBService::Node> > nodes = nodeManager.getNodes();

      // Create PlacementPolicy
      PlacementPolicy threeNodePolicy = {FaultToleranceMode::NODE, 3};

      // Create PlacementStrategy
      ReplicaPlacementStrategy placementStrategy(threeNodePolicy, nodes);

      // Create Relation and OriginalNode
      shared_ptr<DBService::Relation> relation = DBService::Relation::build(
        test_db_name, "relation_with_original_node", 
        "localhost", 4721, "/home/doesnt_exist/secondo");
      relation->setDatabase(test_db_name);

      REQUIRE(relation->getReplicas().empty() == true);

      bool success = placementStrategy.doPlacement(relation);

      REQUIRE_THAT( placementStrategy.getMessage(), Contains("The placement \
was successful.") );

      REQUIRE( success == true );
      REQUIRE( relation->getReplicas().size() == 3);
    }

    SECTION("Simulate a failed 3 Node placement with only 2 Nodes in Fault \
Tolerance Mode NODE") {

      /*
        For this test the following is required:

        - 2 DBS Nodes
        - PlacementPolicy with FaultToleranceMode::NODE and 3 Replicas.
        - A Relation with an original Node without Replicas

        Plan:
        - Implement DatabaseSchema::truncate
        - Implement a NodeMangerTestFactory
          - to create a NodeManager with 2 Nodes being loaded.
      */

      /* 
        Whipe the test database clean by erasing all records
        but preserve the schema.
      */
      DatabaseSchema::truncate(test_db_name);

      shared_ptr<NodeManager> nodeManager = 
        NodeManagerTestFactory::buildNodeManagerWithNodes(
          test_db_name,      
          DBService::Node::nodeTypeDBService(),
          4740, // lowest port nr
          2, // nr of nodes
          true // save the nodes
        );
      
      vector<shared_ptr<DBService::Node>> nodes = nodeManager->getNodes();

      REQUIRE( nodeManager->size() == 2);
      REQUIRE( nodes.size() == 2);

      // Now we are 1 Node short to comply with the policy!
  
      // Create PlacementPolicy
      PlacementPolicy threeNodePolicy = {FaultToleranceMode::NODE, 3};

      // Create PlacementStrategy
      ReplicaPlacementStrategy placementStrategy(threeNodePolicy, nodes);

      // Create Relation and OriginalNode
      shared_ptr<DBService::Relation> relation = 
        DBServiceTest::RelationTestFactory::buildRelationWithOriginalNode(
          test_db_name, "relationWithOriginalNode", 4714);

      // Starting with 0 replicas
      REQUIRE(relation->getReplicas().empty() == true);
      REQUIRE(relation->getReplicaCount() == 0);

      bool success = placementStrategy.doPlacement(relation);    

      REQUIRE_THAT( placementStrategy.getMessage(), Contains("The placement \
has selected 2 nodes for placing replicas. 3 are required."));

      REQUIRE_THAT( placementStrategy.getMessage(), Contains("not compliant \
with the given") );
      REQUIRE( success == false );
      //INFO(placementStrategy.getMessage());    
      
      //INFO("getReplicaCount1: " << relation->getReplicaCount());

      // A failed placement shall not change the relation -> 0 replicas
      REQUIRE( relation->getReplicaCount() == 0);    
    }
  }

  //TODO Add tests for FaultToleranceMode Disk
  //TODO Add tests for FaultToleranceMode None
}