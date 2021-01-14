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
#include "catch.hh"

#include "Algebras/DBService2/RelationManager.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"

using namespace DBService;
using namespace std;

TEST_CASE("Constructing DBService::RelationManagers") {
  
  //TODO define the test_db_name centrally for all tests!
  const string test_db_name = "dbservice_test";

  SECTION("Can't construct a RelationManager with an empty string as database \
name") {

    REQUIRE_THROWS_WITH(new DBService::RelationManager(""), "Can't setDatabase \
on Query given an empty database string!");

  }

  SECTION("Constructing a simple RelationManager")
  {
    DBService::RelationManager manager(test_db_name);

    REQUIRE(manager.empty() == true);
    REQUIRE(manager.size() == 0);
  }

  SECTION("Adding an empty relation should lead to an exception")
  {
    shared_ptr<DBService::Relation> relation = DBService::Relation::build();
    REQUIRE(relation->getIsNew() == true);

    DBService::RelationManager manager(test_db_name);

    REQUIRE_THROWS_WITH(manager.add(relation), "Cannot add empty record.");
  }

  // Some tests are being skipped as they'd only test RecordManager behavior
  // which is covered in NodeManagerTest in greater detail.

  SECTION("Loading Relations") {
    DBService::RelationManager manager(test_db_name);

    REQUIRE_NOTHROW( manager.load() );

    // Assuming there already relations to load
    //TODO Use fixtures
    REQUIRE(manager.size() > 0);

    shared_ptr<DBService::Relation> relation = manager.back(); 

    REQUIRE(relation->getName().empty() == false);
  }

  SECTION("doesRelationHaveReplicas") {

    SECTION("doesRelationHaveReplicas should return false for a non-existing \
database") {

      DBService::RelationManager manager(test_db_name);

      // the test_db_name db exists but has no replicas
      REQUIRE(manager.doesRelationHaveReplicas("nonexistingdb", 
        "nonexistingrelation") == false);
    }

    SECTION("doesRelationHaveReplicas should return false for a non-existing \
relation") {
  
      DBService::RelationManager manager(test_db_name);

      // the test_db_name db exists but has no replicas
      REQUIRE(manager.doesRelationHaveReplicas(test_db_name, 
        "nonexistingrelation") == false);
    }

    // false for a relation without replicas (covered below)
    // true for a relation with replicas
  }

  SECTION("Saving Relations") {
    SECTION("Saving a single new Relation") {
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      // TODO Implement Record::truncate
      // dbAdapter->executeQueryWithoutResult(test_db_name, 
      //   DBService::Relation::deleteAllStatement());

      DBService::Relation::deleteAll(test_db_name);
      DBService::RelationManager manager(test_db_name);

      REQUIRE(manager.getDatabase() == test_db_name);
      
       // Verify that the "dbs_relations" relation is empty
      REQUIRE_NOTHROW( manager.load() );      
      REQUIRE(manager.size() == 0);

      // Building a relation
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
      "somedb", "relation1", 
      "localhost", 4723, "/home/doesnt_exist/secondo");      
      
      // The manager will set the Record database 
      
      manager.add(relation1);

      //TODO here are too many implicit tests of functions that
      // should be tested separately
      REQUIRE(manager.findByDatabaseAndName("somedb", 
        "relation1") == relation1);
      REQUIRE(manager.doesRelationHaveReplicas("somedb", "relation1") == false);

      // Verify that the manager did indeed set the Record's database
      REQUIRE( manager.back()->getDatabase() == "dbservice_test");

      REQUIRE_NOTHROW( manager.save() );
      
      // Verify with a 2nd manager that the Relation has been stored.
      DBService::RelationManager manager2(test_db_name);
      manager2.load();
      shared_ptr<DBService::Relation> loadedRelation = manager2.back();
      REQUIRE(loadedRelation->getName() == "relation1");      
    }

    SECTION("Two Relations should share an identical original Node") {
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      DBService::Relation::deleteAll(test_db_name);
      DBService::RelationManager manager(test_db_name);

      // Building a relation
      shared_ptr<DBService::Relation> relation1 = DBService::Relation::build(
      "somedb", "relation1", 
      "localhost", 4723, "/home/doesnt_exist/secondo");      
      
      manager.add(relation1);
      REQUIRE_NOTHROW( manager.save() );

      // Create a 2nd relation from the same original Host
      shared_ptr<DBService::Relation> relation2 = DBService::Relation::build(
      "somedb", "relation2", 
      "localhost", 4723, "/home/doesnt_exist/secondo");

      // If the recordDatabase of relation2 was different from relations1
      // the originalNode couldn't be assigned as it resulted into a 
      // "Can't change database for non-new record"-error.
      relation2->setDatabase(test_db_name);

      // See whether the original already exists
      shared_ptr<DBService::Node> originalNode = 
        manager.findOriginalNode(relation2->getOriginalNode());
      
      // To avoid a duplicate originalNode, reuse the existing one.
      if (originalNode != nullptr) {
        relation2->setOriginalNode(originalNode);
      }

      manager.add(relation2);
      REQUIRE_NOTHROW( manager.save() );      

      // Verify with a 2nd manager that the Relation has been stored.
      DBService::RelationManager manager2(test_db_name);
      manager2.load();

      vector<shared_ptr<DBService::Relation> > relations = manager2.getAll();
      shared_ptr<DBService::Relation> loadedRelation1 = relations.front();
      shared_ptr<DBService::Relation> loadedRelation2 = relations.back();


      REQUIRE(loadedRelation1->getName() == "relation1");      
      REQUIRE(loadedRelation2->getName() == "relation2");      

      // Both relations should refer to the same Node (Record).
      REQUIRE(loadedRelation1->getOriginalNode()->getId() == 
        loadedRelation2->getOriginalNode()->getId());
    }
    //TODO Add test for doesRelationHaveReplicas > true
    
  }
}