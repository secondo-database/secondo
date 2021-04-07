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
#include "catch.hh" // https://github.com/catchorg/Catch2


#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/Replica.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#include "Algebras/DBService2/DatabaseAdapter.hpp"

using namespace DBService;
using namespace std;

using Catch::Matchers::Contains;

TEST_CASE("Constructing DBService::Replicas")
{
  DBService::Derivative::disableCache();
  DBService::Relation::disableCache();
  DBService::Replica::disableCache();
  DBService::Node::disableCache();
  
  const string test_db_name = DatabaseEnvironment::test;

  // Creating the replica relation is needed for testing relations already

  SECTION("Creating and Loading") {

    SECTION("Attempting to load Replicas without existing Replicas") {      
      // It shouldn't throw an exception if no Replicas exist.

      // Implicit assumption: there will never be a Relation in the test db
      //  with id 9283.
      int nonExistingRelationId = 9283;

      vector<shared_ptr<Replica> > replicas;

      REQUIRE_NOTHROW( replicas = Replica::findByRelationId(
        test_db_name, nonExistingRelationId)
      );

      REQUIRE( replicas.empty() == true);
    }

    SECTION("Equality and Unequality of Replicas") {
      SECTION("Replicas should be equal if target Nodes and Relations are \
equal") {
  
        // Create a relation 1
        shared_ptr<DBService::Relation> relation = DBService::Relation::build(
          test_db_name, "equalrelation");
        relation->setDatabase(test_db_name);

        shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
          test_db_name, "equalrelation");
        relation2->setDatabase(test_db_name);

        // And their original nodes - identical values but different objects
        shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
          "localhost", 4719, "", "/home/doesnt_exist/secondo", 9941, 9942);    
        node->setDatabase(test_db_name);
        node->setType(DBService::Node::nodeTypeOriginal());    
        relation->setOriginalNode(node);

        shared_ptr<DBService::Node> nodeDuplicate = 
          make_shared<DBService::Node>(
            "localhost", 4719, "", "/home/doesnt_exist/secondo", 9941, 9942);
        nodeDuplicate->setDatabase(test_db_name);
        nodeDuplicate->setType(DBService::Node::nodeTypeOriginal());    
        relation2->setOriginalNode(nodeDuplicate);

        REQUIRE(*relation == *relation2);

        //TODO There is no comparison of replicas. Add it.
      }

      // equal if both targetNode and relation are equal
      // uneqal targetNodes equal but relations different
      // unequal targetNodes different but relations equal
      // unequal targetNodes and relations different

      // unequal if of different types
    }


    SECTION("Creating and saving a Replica") {

      SECTION("Failing to save a Replica without a target node") {
        //TODO Implement
      }

      SECTION("Create and Save a Replica successfully") {
        
        // Create a relation
        shared_ptr<DBService::Relation> relation = DBService::Relation::build(
          test_db_name, "rel832repl");
        relation->setDatabase(test_db_name);

        // And its original node
        shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
          "localhost", 4712, "", "/home/doesnt_exist/secondo", 9941, 9942);
        node->setDatabase(test_db_name);
        node->setType(DBService::Node::nodeTypeOriginal());    
        relation->setOriginalNode(node);
        
        // Save the relation
        REQUIRE( relation->save() == true );

        // Create a target node
        shared_ptr<DBService::Node> targetNode = make_shared<DBService::Node>(
          "localhost", 4722, "", "/home/doesnt_exist/secondo", 9941, 9942);
        targetNode->setDatabase(test_db_name);
        targetNode->setType(DBService::Node::nodeTypeDBService());
        targetNode->save();

        Replica replica = Replica();
        replica.setDatabase(test_db_name);
        replica.setTargetNode(targetNode);
        replica.setRelation(relation);
        REQUIRE_NOTHROW(replica.str());
        REQUIRE( replica.save() == true);  

        // Copy
        // Replica replicaCopy = replica;
        // REQUIRE(replicaCopy.getTargetNode() == targetNode);
        // REQUIRE(replicaCopy.getRelation() == relation);
      }
    }

    SECTION("Loading existing Replicas") {
      int testRelationId = -1;

      //TODO Remove assumption about RelationID using fixtures.
      // Assuming there is only one Relation with port 4713 and this is the 
      // one with a replica.
      Query query = DBService::Relation::query(
        test_db_name).feed().addid().filter(
          ".Name = ?", "rel832repl").consume();

      shared_ptr<DBService::Relation> relation = DBService::Relation::findOne(
        test_db_name, query);

      testRelationId = relation->getId();

      REQUIRE(testRelationId > -1);
 
      vector<shared_ptr<Replica> > replicas = Replica::findByRelationId(
        test_db_name, testRelationId);

      shared_ptr<Replica> replica = replicas.back();

      REQUIRE(replica->getStatus() == Replica::statusWaiting);
      REQUIRE_NOTHROW(replica->str());
    }
  }

  /*
    Tests of Derivative Replicas can be found in DerivativeTest.cpp
  */
}