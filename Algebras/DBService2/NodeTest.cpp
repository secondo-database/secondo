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

#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#include <iomanip> // for std::quoted

using namespace DBService;
using namespace std;

using Catch::Matchers::Contains;

TEST_CASE("Constructing DBService::Nodes")
{

  const string test_db_name = DatabaseEnvironment::test;

  // DBService::Derivative::disableCache();
  // DBService::Relation::disableCache();
  // DBService::Replica::disableCache();
  DBService::Node::disableCache();

  //TODO Check if test database is selected, create and select it, if not.

  // Creating the nodes relation so that persistency tests will work.

  SECTION("Creating the Node relation") {
    DBService::Node node;
    node.setDatabase(test_db_name);

    shared_ptr<DatabaseAdapter> adapter = DatabaseAdapter::getInstance();
    
    bool doesRelationExist = adapter->doesRelationExist(
      test_db_name, DBService::Node::getRelationName());

    REQUIRE(doesRelationExist == false);

    REQUIRE_NOTHROW(
      adapter->createRelation(
        test_db_name,
        DBService::Node::getRelationName(),
        DBService::Node::createRelationStatement()
      )
    );
    
    REQUIRE(adapter->doesRelationExist(
      test_db_name, DBService::Node::getRelationName()) == true);
  }

  // This is actually testing Record functionality.
  //TODO mark record tests by moving them to a RECORD section or even
  // separated test file.
  SECTION("Handling databases") {
    SECTION("A DBService::Node should use the default database") {
      DBService::Node node;
      REQUIRE(node.getDatabase() == "dbservicedefaultdb");
    }

    SECTION("Setting a database should be possible on a new record")
    {
      DBService::Node node;
      REQUIRE(node.getDatabase() == "dbservicedefaultdb");

      node.setDatabase(test_db_name);

      REQUIRE(node.getDatabase() == test_db_name);
    }
  }

  SECTION("Constructing DBService::Node records") {
    SECTION("Constructing a simple DBService::Node")
    {
      DBService::Node node;
      node.setDatabase(test_db_name);

      REQUIRE(node.getId() == -1);

      // There should be a Host object
      REQUIRE(node.getHost().getHostname() == "");
      REQUIRE(node.getPort() == 0);
      REQUIRE(node.getConfig() == "");
      REQUIRE(node.getDiskPath() == "");
      REQUIRE(node.getComPort() == 0);
      REQUIRE(node.getTransferPort() == 0);    
      REQUIRE(node.getType() == DBService::Node::nodeTypeDBService());    

      REQUIRE(!(node.getHost().getHostname() != ""));
    }

    SECTION("A DBService::Node constructed from the standard constructor \
should be empty") {

      DBService::Node node;
      node.setDatabase(test_db_name);

      // There should be a Host object
      REQUIRE(node.empty() == true);    
    }    
  }

  SECTION("DBService::Node comparisons") {
    SECTION("Two nodes with equal ports and equal hosts should be \
considered equal") {

      DBService::Node node1("localhost", 1244, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      DBService::Node node2("localhost", 1244, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node2.setDatabase(test_db_name);

      REQUIRE(node1 == node2);
    }

    SECTION("Two nodes with unequal ports and equal hosts should be \
considered unequal") {

      DBService::Node node1("localhost", 1244, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      DBService::Node node2("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node2.setDatabase(test_db_name);

      REQUIRE(node1 != node2);
    }

    SECTION("Two nodes with equal ports and unequal hosts should be \
considered unequal") {

      DBService::Node node1("localhost", 1244, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      DBService::Node node2("example.com", 1244, "", 
        "/home/doesnt_exist/secondo", 9941, 9942);

      node2.setDatabase(test_db_name);

      REQUIRE(node1 != node2);
    }

    SECTION("Two nodes with unequal ports and unequal hosts should be \
considered unequal") {
  
      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);
        
      node1.setDatabase(test_db_name);

      DBService::Node node2("google.com", 1244, "", 
        "/home/doesnt_exist/secondo", 9941, 9942);

      node2.setDatabase(test_db_name);

      REQUIRE(node1 != node2);
    }
  }

  SECTION("DBService::Node saving and dirty checking") {

    // TODO How to deal with record behavior in a separate, more generic way?
    SECTION("An newly created DBService::Node should be a dirty, new record") {
      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      REQUIRE(node1.getIsNew() == true);
      REQUIRE(node1.getIsDirty() == true);
    }

    SECTION("A recently saved DBService::Node should be clean and not-new")
    {    

      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      REQUIRE(node1.getIsNew() == true);
      REQUIRE(node1.getIsDirty() == true);

      node1.save();

      REQUIRE(node1.getIsNew() == false);
      REQUIRE(node1.getIsDirty() == false);

      //TODO Truncate relation
    }

    SECTION("A recently saved DBService::Node should now have an ID")
    {
      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      REQUIRE(node1.getId() == -1);

      node1.save();

      REQUIRE(node1.getId() > 0);
    }

    SECTION("A recently saved previously saved Node should be clean") {

      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      REQUIRE(node1.getIsNew() == true);
      REQUIRE(node1.getIsDirty() == true);

      node1.save();

      REQUIRE(node1.getIsNew() == false);
      REQUIRE(node1.getIsDirty() == false);

      node1.setHost("google.com");

      REQUIRE(node1.getIsDirty() == true);

      node1.save();

      REQUIRE(node1.getIsDirty() == false);
    }

    SECTION("Setting a database should not be possible on a non-new record")
    {
      DBService::Node node1("localhost", 1245, "", "/home/doesnt_exist/secondo",
        9941, 9942);

      node1.setDatabase(test_db_name);

      REQUIRE(node1.getIsNew() == true);
      node1.save();
      REQUIRE(node1.getIsNew() == false);

      // Once a record is stored to a database it will stick with the database.
      REQUIRE_THROWS_WITH(node1.setDatabase("another_database"), 
        Contains("Can't change the database of a non-new record."));
    }
  }

  SECTION("Query generation") {
    SECTION("A non-empty DBService::Node should create its own create \
statement") {

      DBService::Node node1("sec-w-0.sec-ws.secondo.svc.cluster.local", 1244, 
        "/database/config/SecondoConfig.ini", "/database/secondo-databases", 
        9941, 9942);

      node1.setDatabase(test_db_name); // the db doesn't matter here

      REQUIRE(node1.createStatement() == "query dbs_nodes \
inserttuple[totext(\"sec-w-0.sec-ws.secondo.svc.cluster.local\"), \
1244, totext(\"/database/config/SecondoConfig.ini\"), \
totext(\"/database/secondo-databases\"), 9941, 9942, \"dbservice\"] \
consume");
    }

    SECTION("A non-empty DBService::Node should create its own create \
      relation statement")
    {
      DBService::Node node1("sec-w-0.sec-ws.secondo.svc.cluster.local", 1244, 
        "/database/config/SecondoConfig.ini", "/database/secondo-databases", 
        9941, 9942);
      node1.setDatabase(test_db_name); // the db doesn't matter here

      REQUIRE(node1.createRelationStatement() == "let dbs_nodes = [const \
rel(tuple([Host: text, Port: int, Config: text, DiskPath: text, \
ComPort: int, TransferPort: int, Type: string])) value ()]");

    }
  }

  SECTION("Deleting a node") {
    DBService::Node node1("localhost", 1246, "", "/home/doesnt_exist/secondo", 
      9941, 9942);
    node1.setDatabase(test_db_name);

    REQUIRE(node1.getIsNew() == true);
    node1.save();

    int id = node1.getId();
    LOG_F(INFO, "The node id is %d id", id);

    REQUIRE(id > 0);
    REQUIRE(node1.getIsNew() == false);

    REQUIRE_NOTHROW(node1.destroy());

    LOG_F(INFO, "The node id is now %d id", id);
    auto node12 = DBService::Node::findByTid(test_db_name, id);
    REQUIRE(node12 == nullptr);
  }
}