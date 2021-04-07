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

#include "Algebras/DBService2/Derivative.hpp"
#include "Algebras/DBService2/DatabaseEnvironment.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/DatabaseSchema.hpp"
#include "Algebras/DBService2/RelationTestFactory.hpp"
#include "Algebras/DBService2/Relation.hpp"

using namespace DBService;
using namespace DBServiceTest;
using namespace std;

using Catch::Matchers::Equals;

TEST_CASE("Constructing DBService::Derivatives")
{

  const string test_db_name = DatabaseEnvironment::test;

  DBService::Derivative::disableCache();
  DBService::Relation::disableCache();
  DBService::Replica::disableCache();
  DBService::Node::disableCache();

  SECTION("Building, Saving and Loading") {

    const string relationName = "Cities";
    const string derivativeName = "CitiesCount";
    const string function = ". count";

    SECTION("Creating a Derivative") {
      
      /* 
        In order to construct a derivate, a relation with original node is 
        required
      */
   
      auto relation = RelationTestFactory::buildRelationWithOriginalNode(
        test_db_name, relationName);
   
      shared_ptr<Derivative> derivative = DBService::Derivative::build(
        derivativeName, function, relation);
        derivative->setDatabase(test_db_name);

      REQUIRE_NOTHROW(derivative->save());
    }

    SECTION("Loading a Derivative") {
      

      auto derivatives = DBService::Derivative::findAll(test_db_name);

      REQUIRE(derivatives.size() > 0);

      shared_ptr<Derivative> derivative = derivatives.front();

      REQUIRE_THAT(derivative->getName(), Equals(derivativeName));
      REQUIRE_THAT(derivative->getFunction(), Equals(function));
    }

    SECTION("Creating a Derivative using a Relation") {
      auto relation = RelationTestFactory::buildRelationWithOriginalNode(
        test_db_name, "relationWithDerivative");

      REQUIRE(relation->getIsNew() == false);
      REQUIRE(relation->getIsDirty() == false);
          
      relation->addDerivative(derivativeName, function);

      REQUIRE(relation->getIsDirty() == true);

      relation->save();
      
      REQUIRE(relation->getIsDirty() == false);
      REQUIRE(relation->getDerivativeCount() == 1);

      auto derivative = relation->getDerivatives().front();

      REQUIRE(derivative->getIsDirty() == false);
      REQUIRE_THAT(derivative->getFunction(), Equals(function));
      REQUIRE_THAT(derivative->getName(), Equals(derivativeName));
      
    }

    SECTION("Loading a Relation with a Derivative") {      
      Query query = DBService::Relation::queryByDatabaseAndName(test_db_name, 
        test_db_name, "relationWithDerivative");  
      
      shared_ptr<DBService::Relation> loadedRelation = 
        DBService::Relation::findOne(test_db_name, query);


      REQUIRE(loadedRelation->getDerivativeCount() == 1);

      auto derivative = loadedRelation->getDerivatives().front();
      REQUIRE_THAT(derivative->getName(), Equals(derivativeName));
      REQUIRE_THAT(derivative->getFunction(), Equals(function));
    }
  }

  SECTION("Derivative Replicas") {
    SECTION("Create and Save") {      
      auto relation = RelationTestFactory::buildRelationWithReplica(
          test_db_name, "Foh4Taer");

      relation->addDerivative("derivative1", ". count");

      REQUIRE( relation->getDerivativeCount() == 1);

      auto derivative = relation->getDerivatives().front();

      REQUIRE_THAT(derivative->getName(), Equals("derivative1"));
      REQUIRE_THAT(derivative->getFunction(), Equals(". count"));
      REQUIRE(derivative->getReplicaCount() == relation->getReplicaCount());
      REQUIRE_NOTHROW(derivative->save());
    }

    SECTION("Loading a Derivative with Replicas") {      
      
      Query query = DBService::Relation::queryByDatabaseAndName(
        test_db_name, test_db_name, "Foh4Taer");      
      shared_ptr<DBService::Relation> relation = DBService::Relation::findOne(
        test_db_name, query);

      REQUIRE( relation->getReplicaCount() == 1);
      REQUIRE( relation->getDerivativeCount() == 1);

      auto derivative = relation->getDerivatives().front();

      REQUIRE_THAT(derivative->getName(), Equals("derivative1"));
      REQUIRE_THAT(derivative->getFunction(), Equals(". count"));
      REQUIRE(derivative->getReplicaCount() == relation->getReplicaCount());
    }

    SECTION("Update Derivative Replica Status") {
      Query query = DBService::Relation::queryByDatabaseAndName(test_db_name, 
        test_db_name, "Foh4Taer");

      shared_ptr<DBService::Relation> relation = DBService::Relation::findOne(
        test_db_name, query);
      
      REQUIRE( relation->getReplicaCount() == 1);
      
      REQUIRE( relation->getDerivativeCount() == 1);

      auto derivative = relation->getDerivatives().front();

      // Known due to the usage of the RelationTestFactory.
      int targetNodePort = 4713;      

      relation->updateDerivativeReplicaStatus(
        derivative->getName(),
        "localhost",
        targetNodePort,
        Replica::statusReplicated
      );
    }
  }
}