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

#include "NestedList.h"
#include "StandardTypes.h"
#include "QueryProcessor.h"

#include "Algebras/DBService2/DebugOutput.hpp"
#include "Algebras/DBService2/OperatorTestDBService.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hh"

#include "Algebras/DBService2/SecondoDatabaseAdapterTest.cpp"
#include "Algebras/DBService2/HostTest.cpp"
#include "Algebras/DBService2/NodeTest.cpp"
#include "Algebras/DBService2/NodeManagerTest.cpp"
#include "Algebras/DBService2/RelationTest.cpp"
#include "Algebras/DBService2/ReplicaTest.cpp"
#include "Algebras/DBService2/RelationManagerTest.cpp"
#include "Algebras/DBService2/DBServiceClientTest.cpp"
#include "Algebras/DBService2/DBServiceManagerTest.cpp"
#include "Algebras/DBService2/DerivativeTest.cpp"

#include <loguru.hpp>

/*

This test creates all DBS relations and truncates all tables.
Therefore it is run last as other tests create relations and assume
they do not exist, yet.

TODO Refactor tests and reduce their assumptions/dependencies about prior
  tests and their particular order. 

*/

#include "Algebras/DBService2/ReplicaPlacementStrategyTest.cpp"

extern boost::recursive_mutex nlparsemtx;

namespace DBService
{

  ListExpr OperatorTestDBService::mapType(ListExpr nestedList)
  {
    printFunction("OperatorTestDBService::mapType", std::cout);
    print(nestedList, std::cout);

    LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
    boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);
    LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");

    if (!nl->HasLength(nestedList, 0))
    {
      ErrorReporter::ReportError(
          "expected signature: (empty signature)");
      return nl->TypeError();
    }

    return listutils::basicSymbol<CcBool>();
  }

  /*
    Executes the test suite of the DBService.
    Requires that there is not database called "dbservice_test".

    Creates the database "dbservice_test" and executes the test suite.
    Does not delete the test database after the test execution.

    This implies that running this operator twice will result in a crash.

    TODO Make operator idempotent to prevent it from crashing Secondo.
  */
  int OperatorTestDBService::mapValue(Word *args,
                                      Word &result,
                                      int message,
                                      Word &local,
                                      Supplier s)
  {
    printFunction("OperatorTestDBService::mapValue", std::cout);

    bool success = true;
    char *argv[] = {(char*)"DBService TestSuite", NULL};
    int argc = sizeof(argv) / sizeof(char *) - 1;

    // Ensure the test database exists.
    DatabaseAdapter::getInstance()->createDatabase(DatabaseEnvironment::test);

    int returnCode = Catch::Session().run(argc, argv);

    if (returnCode != 0) {
      success = false;
    }

    //TODO find out if there's a more elegant way...
    /* The test suite will commit many transactions but Secondo will attempt to 
      close
      the transaction for the given operator. Therefore, here a transaction 
      will be opened
      to satisfy this requirement for a successful operator execution.
     */
    SecondoSystem::BeginTransaction();

    result = qp->ResultStorage(s);
    static_cast<CcBool *>(result.addr)->Set(true, success);
    return 0;
  }

} /* namespace DBService */
