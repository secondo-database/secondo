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
#include "Algebras/DBService2/Derivative.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#include "Algebras/DBService2/SecondoRelationAdapter.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include "Algebras/DBService2/RelationTestFactory.hpp"
#include "Algebras/DBService2/NodeTestFactory.hpp"

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

using namespace DBService;
using namespace DBServiceTest;
using namespace std;

using Catch::Matchers::Contains;
using Catch::Matchers::Equals;

TEST_CASE("Constructing DBService::Relations")
{

  DBService::Derivative::disableCache();
  DBService::Relation::disableCache();
  DBService::Replica::disableCache();
  DBService::Node::disableCache();

  const string test_db_name = DatabaseEnvironment::test;

  SECTION("Creating the Replica Relation required for Relations") {
    shared_ptr<DatabaseAdapter> adapter = DatabaseAdapter::getInstance();

    bool doesRelationExist = adapter->doesRelationExist(
      test_db_name, DBService::Replica::getRelationName()
    );

    REQUIRE(doesRelationExist == false);

    REQUIRE_NOTHROW(
      adapter->createRelation(
        test_db_name,
        DBService::Replica::getRelationName(),
        DBService::Replica::createRelationStatement()
      )
    );
    
    REQUIRE(adapter->doesRelationExist(
      test_db_name, 
      DBService::Replica::getRelationName()) == true);
  }

  SECTION("Creating the Derivatives Relation requires for Relations") {
    shared_ptr<DatabaseAdapter> adapter = DatabaseAdapter::getInstance();

    bool doesRelationExist = adapter->doesRelationExist(
      test_db_name, DBService::Derivative::getRelationName()
    );

    REQUIRE(doesRelationExist == false);

    REQUIRE_NOTHROW(
      adapter->createRelation(
        test_db_name,
        DBService::Derivative::getRelationName(),
        DBService::Derivative::createRelationStatement()
      )
    );
    
    REQUIRE(adapter->doesRelationExist(
      test_db_name, 
      DBService::Derivative::getRelationName()) == true);
  }

  SECTION("Creating the Relation relation (table)") {
    shared_ptr<DBService::Relation> relation = DBService::Relation::build();
    relation->setDatabase(test_db_name);

    shared_ptr<DatabaseAdapter> adapter = DatabaseAdapter::getInstance();

    bool doesRelationExist = adapter->doesRelationExist(
      test_db_name, DBService::Relation::getRelationName());
    
    REQUIRE(doesRelationExist == false);
    
    REQUIRE_NOTHROW(
      adapter->createRelation(
        test_db_name,
        DBService::Relation::getRelationName(),
        DBService::Relation::createRelationStatement()
      )
    );
    
    REQUIRE(adapter->doesRelationExist(
      test_db_name, 
      DBService::Relation::getRelationName()) == true);
  }

  SECTION("A DBService::Relation comparisons") {
    SECTION("Relations are different if there databases are different") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
        "db2", "relation1");
      
      REQUIRE( !(relation1 == relation2) );    
      REQUIRE( relation1 != relation2 );    
    }

    SECTION("Relations are different if there relation names are different") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
        "db1", "relation2");
      
      REQUIRE( !(*relation1 == *relation2) );    
      REQUIRE( *relation1 != *relation2 );    
    }

    SECTION("Relations are equal if db and names are equal") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
        "db1", "relation1");      
      
      REQUIRE( *relation1 == *relation2 );    
      REQUIRE( !(*relation1 != *relation2) );    
    }
  }
  
  SECTION("Create a Relation along with its original Node") {
    shared_ptr<DBService::Relation> relWithOriginal 
      = DBService::Relation::build(
        test_db_name, "relation_with_original_node", 
        "localhost", 4714, "/home/doesnt_exist/secondo"
      );
    
    relWithOriginal->setDatabase(test_db_name);
    
    REQUIRE( relWithOriginal->getOriginalNode()->getHost().getHostname() 
      == "localhost" );
    REQUIRE( relWithOriginal->getOriginalNode()->getPort() == 4714 );
    REQUIRE( relWithOriginal->getOriginalNode()->getDiskPath() 
      == "/home/doesnt_exist/secondo" );
    REQUIRE( relWithOriginal->getOriginalNode()->getType() 
      == DBService::Node::nodeTypeOriginal() );
  }

  SECTION("In-Memory Handling of Replicas") {
    SECTION("In-Memory adding a Replica") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);

      shared_ptr<Replica> replica = make_shared<Replica>();
      
      REQUIRE_NOTHROW(relation1->addReplica(replica));

      REQUIRE(relation1->getReplicaCount() == 1);
      REQUIRE(relation1->doesReplicaExist(replica) == true);

      // Verify that the replica has been adapted to the relation
      REQUIRE(replica->getDatabase() == relation1->getDatabase());
      REQUIRE(replica->getRelation() == relation1);
    }

    SECTION("In-Memory adding a Replica twice shouldn't be possible") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);

      REQUIRE(relation1->getIsNew() == true);

      shared_ptr<Replica> replica = make_shared<Replica>();
      
      REQUIRE(relation1->doesReplicaExist(replica) == false);
      REQUIRE_NOTHROW(relation1->addReplica(replica));
      
      REQUIRE(relation1->doesReplicaExist(replica) == true);
      REQUIRE_THROWS_WITH(relation1->addReplica(replica),
        Contains(
          "Can't add replica. Replica already exists!"
         )
      );
      
      REQUIRE(relation1->getReplicaCount() == 1);
      REQUIRE(relation1->getIsDirty() == true);

      //TODO Also test for peristet relations!
      REQUIRE_NOTHROW(relation1->resetReplicas());

      REQUIRE(relation1->getReplicaCount() == 0);
      REQUIRE(relation1->getIsDirty() == true);
    }

    SECTION("Resetting a Relation with two in-memory (new) Replicas") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);

      REQUIRE(relation1->getReplicaCount() == 0);
      
      shared_ptr<Replica> replica = make_shared<Replica>();
      REQUIRE_NOTHROW(relation1->addReplica(replica));

      REQUIRE(relation1->getReplicaCount() == 1);

      shared_ptr<Replica> replica2 = make_shared<Replica>();
      REQUIRE_NOTHROW(relation1->addReplica(replica2));

      REQUIRE(relation1->getReplicaCount() == 2);

      relation1->resetReplicas();
    }

    SECTION("In-Memory adding a Replica for a given Target Node") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);

      // Create a target node
  
      // shared_ptr<DBService::Node> targetNode = make_shared<DBService::Node>(
      //   "localhost", 4716, "", "/home/doesnt_exist/secondo", 9941, 9942);   
      // targetNode->setDatabase(test_db_name);
      // targetNode->setType(DBService::Node::nodeTypeDBService());

      shared_ptr<DBService::Node> targetNode = NodeTestFactory::buildTargetNode(
        test_db_name, 4716, false // do not save
      );
      
      // Build and add a Replica for the given target node
      REQUIRE_NOTHROW(relation1->addReplica(targetNode));

      REQUIRE(relation1->getReplicaCount() == 1);          
    }
  }

  SECTION("Saving Relation records") {
    SECTION("A new Relation can't be saved without an original Node") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);
      
      REQUIRE_THROWS_WITH( relation1->save(), 
        Contains(
          "Can't execute Relation::createStatement() without original Node!"
         )
      );      
    }

    SECTION("A new Relation record can be saved") {
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
        "db1", "relation1");
      relation1->setDatabase(test_db_name);

      shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
        "localhost", 4718, "", "/home/doesnt_exist/secondo", 9941, 9942);
      
      node->setDatabase(test_db_name);
      node->setType(DBService::Node::nodeTypeOriginal());

      REQUIRE( node->save() == true );
      
      relation1->setOriginalNode(node);
      
      REQUIRE( relation1->save() == true );      
    }

    SECTION("A new Relation also saves a new OriginalNode") {
      shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
        "db1", "relation2");
      relation2->setDatabase(test_db_name);

      shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
        "localhost", 4712, "", "/home/doesnt_exist/secondo", 9941, 9942);
      
      node->setDatabase(test_db_name);
      node->setType(DBService::Node::nodeTypeOriginal());
      
      relation2->setOriginalNode(node);
      
      REQUIRE( relation2->save() == true );      

      REQUIRE( relation2->getOriginalNode()->getIsNew() == false);
    }

    SECTION("A new Relation with Replicas") {

      shared_ptr<DBService::Relation> relationWithOriginalNode = 
        DBServiceTest::RelationTestFactory::buildRelationWithOriginalNode(
          test_db_name, "relationWithOriginalNode", 4712);

      // Target node
      shared_ptr<DBService::Node> targetNode = NodeTestFactory::buildTargetNode(
        test_db_name, 4713, true // do save
      );

      // /* 
      //   TODO More elegant way to create Replica for a given Relation
      //   Also this Replica is not part of the in-memory relation as it is not
      //   contained in the relation's replicas-vector. 
      // */
      Replica replica = Replica();
      replica.setDatabase(test_db_name);
      replica.setTargetNode(targetNode);
      replica.setRelation(relationWithOriginalNode);

      REQUIRE( replica.save() == true);
    }

    SECTION("Updating a Relation") {
      
    }

    // relation->save -> after_save -> replicas.save()

    SECTION("Saving a Relation with new Replicas using a save cascade") {
      shared_ptr<DBService::Relation> relationWithReplica = 
        DBServiceTest::RelationTestFactory::buildRelationWithOriginalNode(
          test_db_name, "relationWithReplica", 4712, true); // save the relation
      
      // Target node -      
      shared_ptr<DBService::Node> targetNode = NodeTestFactory::buildTargetNode(
        test_db_name, 4724, true // target Node will be saved
      );

      REQUIRE( relationWithReplica->getIsDirty() == false);

      relationWithReplica->addReplica(targetNode);

      REQUIRE( relationWithReplica->getIsDirty() == true);

      relationWithReplica->save();
            
      REQUIRE( relationWithReplica->getIsDirty() == false);
      
      // recorddb, relationDb, relationName
      Query query = DBService::Relation::queryByDatabaseAndName(test_db_name,
        test_db_name, "relationWithReplica");      

      shared_ptr<DBService::Relation> loadedRelation = 
        DBService::Relation::findOne(test_db_name, query);

      REQUIRE(loadedRelation != nullptr);
      REQUIRE(loadedRelation->getReplicaCount() == 
        relationWithReplica->getReplicaCount());
    }   
  }

  SECTION("Relation Queries") {
    string relationName = "relation1";

    SECTION("It should generate a Query for a given Relation") {    
      Query query = DBService::Relation::query(test_db_name);

      REQUIRE(query.str() == "query dbs_relations");
    }

    SECTION("It should generate a filter condition") {
      Query query = DBService::Relation::query(test_db_name);

      Query filteredQuery = query.filter(".Name = ?", relationName);

      REQUIRE(filteredQuery.str() == "query dbs_relations \
filter[.Name = \"relation1\"]");

    }

    SECTION("It should be able to create query train racks") {

      Query query = DBService::Relation::query(test_db_name).filter(
        ".Name = ?", relationName);

      REQUIRE(query.str() == "query dbs_relations \
filter[.Name = \"relation1\"]");

    }

    SECTION("Generate a query for all Relations using feed consume") {
      Query query = DBService::Relation::query(
        test_db_name).feed().addid().consume();

      REQUIRE(query.str() == "query dbs_relations feed addid consume");
    }

    SECTION("Generate a query for all Relations using findAllStatement") {
      string query = DBService::Relation::findAllStatement(test_db_name);

      REQUIRE(query == "query dbs_relations feed addid consume");

      /* Result:
                Name : relation1
            Database : dbservice_test
      OriginalNodeId : 7
                TID  : 1

      */
    }

    SECTION("Query a Relation by RelationId") {
      Query query = DBService::Relation::query(test_db_name).feed().filter(
        ".RelationId = ?", 1).addid().consume();

      REQUIRE(query.str() == "query dbs_relations feed \
filter[.RelationId = 1] addid consume");

    }
  }

  SECTION("Loading Relations") {
    SECTION("Load all Relations") {

      //TODO Delete all relations, Create a relation

      //TODO Move this load functionality to an apropriate class
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      dbAdapter->openDatabase(test_db_name);
      vector<shared_ptr<DBService::Relation> > relations = 
        DBService::Relation::findAll(test_db_name);

      //TODO Create fixtures to decouple from previous test cases.
      shared_ptr<DBService::Relation> relation = relations.back();
      
      REQUIRE(relation->getId() > 0);
      REQUIRE(relation->getDatabase() == test_db_name);

      // Case in-sensitive comparison
      REQUIRE_THAT(relation->getRelationDatabase(), Equals(test_db_name, 
        Catch::CaseSensitive::No));

      //TODO Remove assumption about ordinality of records
      REQUIRE(relation->getName() == "relationWithReplica");

      // Testing whether the original Node has been loaded successfully
      shared_ptr<DBService::Node> originalNode = relation->getOriginalNode();
      REQUIRE(originalNode->getHost().getHostname() == "localhost");
      REQUIRE(originalNode->getType() == DBService::Node::nodeTypeOriginal());
    }

    SECTION("Load Relation with Replica") {

      Replica::invalidateCache();

      //TODO Remove dependency to prio test cast. Use fixture.
      string relationNameWithReplica = "relationWithReplica";
      
      // query(test_db_name).feed().filter(".Database = ?", 
      //  boost::to_upper_copy(test_db_name)).filter(".Name = ?", 
      //  relationNameWithReplica).addid().consume();            
      Query query = DBService::Relation::queryByDatabaseAndName(test_db_name, 
        boost::to_upper_copy(test_db_name), relationNameWithReplica);
      shared_ptr<DBService::Relation> relationWithReplica = 
        DBService::Relation::findOne(test_db_name, query);

      REQUIRE(relationWithReplica != nullptr);

      relationWithReplica->setDatabase(test_db_name);

      // Case in-sensitive comparison
      REQUIRE_THAT(relationWithReplica->getRelationDatabase(), 
        Equals(test_db_name, Catch::CaseSensitive::No));

      REQUIRE(relationWithReplica->getName() == relationNameWithReplica);

      vector<shared_ptr<Replica> > replicas = 
        relationWithReplica->getReplicas();

      REQUIRE(replicas.size() > 0);

      // Acquire a shared_ptr from shared_ptr
      shared_ptr<Replica> replica = replicas.back();

      REQUIRE(replica->getStatus() == Replica::statusWaiting);
    }

    // Load relation by db and relation name;
  }

  // Set name for new node
  // Set name for non-new node
  // Set originalNode for new node
  // Set originalNode for non-new node 
}