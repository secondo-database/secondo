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

#include "Algebras/DBService2/SecondoDatabaseAdapter.hpp"


using namespace DBService;
using namespace std;

using Catch::Matchers::Contains;

TEST_CASE("DBService::SecondoDatabaseAdapter")
{

  const string test_db_name = "dbservice_test";
  shared_ptr<DatabaseAdapter> adapter = SecondoDatabaseAdapter::getInstance();

  SECTION("It should confirm that an non-existing database doesn't exists")
  {
    REQUIRE(adapter->doesDatabaseExist("nonexistenddatabase") == false);
  }

  SECTION("It should confirm that an existing database exists")
  {
    REQUIRE(adapter->doesDatabaseExist(test_db_name) == true);
  }

  SECTION("A database should be open as the test operator requires an open \
database to be invoked") {

    REQUIRE(adapter->isDatabaseOpen() == true);
  }

  SECTION("Given an open database it should be possible to close the database")
  {
    REQUIRE(adapter->isDatabaseOpen() == true);
    adapter->closeDatabase();
    REQUIRE(adapter->isDatabaseOpen() == false);

    // Now there's no database open anymore!
  }

  SECTION("Without an open database, the attempt to close it should succeed \
silently") {

    REQUIRE(adapter->isDatabaseOpen() == false);
    adapter->closeDatabase();
    REQUIRE(adapter->isDatabaseOpen() == false);
  }

  SECTION("Without an open database, it should be possible to open the \
database") {

    REQUIRE(adapter->isDatabaseOpen() == false);
    adapter->openDatabase(test_db_name);
    REQUIRE(adapter->isDatabaseOpen() == true);
  }

  SECTION("Given an open database, it should success to open the \
database again") {

    REQUIRE(adapter->isDatabaseOpen() == true);
    adapter->openDatabase(test_db_name);
    REQUIRE(adapter->isDatabaseOpen() == true);
  }

  SECTION("Given an open database, it should success to open the \
database again") {

    REQUIRE(adapter->isDatabaseOpen() == true);
    adapter->openDatabase(test_db_name);
    REQUIRE(adapter->isDatabaseOpen() == true);
  }

  SECTION("Given an open database, it should success to open the database \
again") {

    REQUIRE(adapter->isDatabaseOpen() == true);
    adapter->openDatabase(test_db_name);
    REQUIRE(adapter->isDatabaseOpen() == true);
  }

// The following test has been disabled as the check has been 
//  disabled for performance reasons.

//   SECTION("It should raise an exception attempting to open a non-existing \
// database") {
//     REQUIRE_THROWS_WITH(adapter->openDatabase("nonexistenddatabase"),
//       Contains("doesn't exist"));
//   }

  SECTION("Database lifecycle tests") {

    // exoticdatabase73829 -> crash
    string exoticDatabaseName = "db73829";

    SECTION("Creating a database") {
      REQUIRE(adapter->doesDatabaseExist(exoticDatabaseName) == false);
      REQUIRE_NOTHROW(adapter->createDatabase(exoticDatabaseName));
      REQUIRE(adapter->doesDatabaseExist(exoticDatabaseName) == true);
    }

    SECTION("Deleting a database") {
      REQUIRE(adapter->doesDatabaseExist(exoticDatabaseName) == true);
      REQUIRE_NOTHROW(adapter->deleteDatabase(exoticDatabaseName));
      REQUIRE(adapter->doesDatabaseExist(exoticDatabaseName) == false);
    }
  }
}