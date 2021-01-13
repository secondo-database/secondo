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

#include "Algebras/DBService2/NodeManager.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"

#include <thread>
#include <chrono>

using namespace DBService;
using namespace std;

TEST_CASE("Constructing DBService::NodeManagers")
{

  //TODO define the test_db_name centrally for all tests!
  const string test_db_name = "dbservice_test";

  SECTION("Can't construct a NodeManager with an empty string as database name")
  {
    REQUIRE_THROWS_WITH(new DBService::NodeManager(""), 
      "Can't setDatabase on Query given an empty database string!");
  }

  SECTION("Constructing a simple NodeManager")
  {
    DBService::NodeManager nodeManager(test_db_name);

    REQUIRE(nodeManager.empty() == true);
    REQUIRE(nodeManager.size() == 0);
  }

  SECTION("Adding an empty node should lead to an exception")
  {
    shared_ptr<DBService::Node> node = make_shared<DBService::Node>();
    REQUIRE(node->getIsNew() == true);

    DBService::NodeManager nodeManager(test_db_name);

    REQUIRE_THROWS_WITH(nodeManager.add(node), "Cannot add empty record.");
  }

  SECTION("Adding a node should lead to a non-empty list of size equal to 1")
  {
    shared_ptr<DBService::Node> node = make_shared<DBService::Node>("localhost",
      1244, "", "/home/doesnt_exist/secondo", 9941, 9942);
    DBService::NodeManager nodeManager(test_db_name);

    REQUIRE(node->getIsNew() == true);

    nodeManager.add(node);

    REQUIRE(nodeManager.empty() == false);
    REQUIRE(nodeManager.size() == 1);
  }

  SECTION("Adding the same node twice should lead to a non-empty list of size \
    equal to 1") {
    shared_ptr<DBService::Node> node = make_shared<DBService::Node>("localhost",
    1244, "", "/home/doesnt_exist/secondo", 9941, 9942);
    DBService::NodeManager nodeManager(test_db_name);

    nodeManager.add(node);
    nodeManager.add(node);

    REQUIRE(nodeManager.empty() == false);
    REQUIRE(nodeManager.size() == 1);
  }

  SECTION("Adding the same node and trying to find it by node should be \
    successful")
  {
    shared_ptr<DBService::Node> node = make_shared<DBService::Node>("localhost",
      1244, "", "/home/doesnt_exist/secondo", 9941, 9942);

    DBService::NodeManager nodeManager(test_db_name);

    nodeManager.add(node);
    shared_ptr<DBService::Node> foundNode = nodeManager.find(node);  

    REQUIRE(foundNode->empty() == false);
    REQUIRE(*foundNode == *node);
  }

  SECTION("Adding the same node and trying to find it by hostname and port \
    should be successful")
  {
    shared_ptr<DBService::Node> node = make_shared<DBService::Node>("localhost",
      1244, "", "/home/doesnt_exist/secondo", 9941, 9942);
    DBService::NodeManager nodeManager(test_db_name);

    nodeManager.add(node);
    shared_ptr<DBService::Node> foundNode = nodeManager.findByHostnameAndPort(
      "localhost", 1244);

    REQUIRE(foundNode->empty() == false);
    REQUIRE(*foundNode == *node);
  }

  SECTION("Loading Nodes") {
    SECTION("Loading all Nodes") {
      DBService::NodeManager nodeManager(test_db_name);

      //TODO Fixtures. For this test we need a defined set of Nodes in the db.
      REQUIRE_NOTHROW( nodeManager.load() );

      REQUIRE(nodeManager.size() > 0);

      shared_ptr<DBService::Node> node = nodeManager.back(); 

      //TODO Create fixtures to have a defined state before the test starts!
      // This makes assumptions about the records in the test db although
      // these are artefacts of prior tests instead of fixtures. 
      REQUIRE(node->getHost().getHostname() == "localhost");
      REQUIRE(node->getConfig() == "");
      REQUIRE(node->getDiskPath() == "/home/doesnt_exist/secondo");
      REQUIRE(node->getComPort() == 9941);
      REQUIRE(node->getTransferPort() == 9942);      
      REQUIRE(node->getDatabase() == test_db_name);
      REQUIRE(node->getDatabaseAdapter() != nullptr);
    }    
  }

  SECTION("Saving nodes") {
    SECTION("Saving a single new node") {
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();
      
      // TODO Implement Record::truncate
      // dbAdapter->executeQueryWithoutResult(test_db_name, 
      //   DBService::Node::deleteAllQuery().str());      
      DBService::Node::deleteAll(test_db_name);

      DBService::NodeManager nodeManager(test_db_name);

      // Verify that the nodes relation is empty
      REQUIRE_NOTHROW( nodeManager.load() );      
      REQUIRE(nodeManager.size() == 0);

      shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
        "localhost", 4711, "", "/home/doesnt_exist/secondo", 9941, 9942);
      
      nodeManager.add(node);
      REQUIRE_NOTHROW( nodeManager.save() );

      // Verify with a 2nd node manager that the node has been stored.
      DBService::NodeManager nodeManager2(test_db_name);
      nodeManager2.load();
      shared_ptr<DBService::Node> loadedNode = nodeManager2.back();
      REQUIRE(loadedNode->getPort() == 4711);
    }
  }

  SECTION("Node Types") {
    SECTION("A node manager of type ORIGINAL should not load non-ORIGINALS") {
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      //TODO The test should not make assumptions about data from other tests
      //  instead fixtures should be used.

      // NodeManager responsible for original nodes.
      DBService::NodeManager nodeManager(test_db_name, 
        DBService::Node::nodeTypeOriginal());
      
      REQUIRE_NOTHROW( nodeManager.load() );

      // Although there are records, the size should be 0 as none
      // is of the type "original".
      REQUIRE(nodeManager.size() == 0);
    }

    SECTION("A node manager of type ORIGINAL should load ORIGINALS") {
      shared_ptr<DatabaseAdapter> dbAdapter = DatabaseAdapter::getInstance();

      // Create an original node
      shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
        "localhost", 4715, "", "/home/doesnt_exist/secondo", 9941, 9942);
      node->setType(DBService::Node::nodeTypeOriginal());
      node->setDatabaseAdapter(dbAdapter);
      node->setDatabase(test_db_name);
      REQUIRE(node->save() == true);

      // NodeManager responsible for original nodes.
      DBService::NodeManager nodeManager(test_db_name, 
        DBService::Node::nodeTypeOriginal());

      REQUIRE_NOTHROW( nodeManager.load() );
      REQUIRE(nodeManager.size() == 1);
      shared_ptr<DBService::Node> loadedNode = nodeManager.back();
      REQUIRE(loadedNode->getPort() == 4715);
    }
  }
}