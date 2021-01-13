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
#include "Algebras/DBService2/RelationManager.hpp"

#include <string>

using namespace std;

namespace DBService {
  
  RelationManager::RelationManager(string newDatabase) : 
    RecordManager(newDatabase) { 
  }
  
  shared_ptr<DBService::Relation> RelationManager::findByDatabaseAndName(
      string relationDatabase, 
      string relationName) {

    shared_ptr<DBService::Relation> relationToFind = DBService::Relation::build(
      relationDatabase,
      relationName
    );

    return find(relationToFind);
  }

  shared_ptr<DBService::Node> RelationManager::findOriginalNode(
    shared_ptr<DBService::Node> nodeToFind) {

    shared_ptr<DBService::Node> currentNode;

    for (auto& relation : records) {
      currentNode = relation->getOriginalNode();

      if (currentNode == nullptr)
        continue;

      if (*currentNode == *nodeToFind)
        return currentNode;
    }

    return nullptr;
  }

  bool RelationManager::doesRelationHaveReplicas(
      string relationDatabase, 
      string relationName) {

    shared_ptr<DBService::Relation> relation = findByDatabaseAndName(
      relationDatabase, relationName);
    
    if (relation == nullptr)
      return false;

    return (relation->getReplicaCount() > 0);
  }

  shared_ptr<Replica> RelationManager::getRandomReplica(
    string relationDatabase, string relationName) {
    
    auto relation = findByDatabaseAndName(relationDatabase, relationName);
    
    if (relation == nullptr)
      return nullptr;    

    return relation->getRandomReplica();
  }

  bool RelationManager::doesDerivativeExist(string derivativeName) {
    for (auto& relation : records) {
      if (relation->doesDerivativeExist(derivativeName))
        return true;
    }

    return false;
  }

  void RelationManager::deleteRelationsByRelationDatabase(
    string relationDatabase) {

    LOG_F(INFO, "%lu Relations to be checked for deletion.", records.size());

    records.erase(
      std::remove_if(
        records.begin(), records.end(),

        // Capture &relationName to pass it into the lambda
        [&relationDatabase](const shared_ptr<Relation>& relation) {
          if(relation->getRelationDatabase() == relationDatabase) {

            /* Destroy the Relation -> removing the entire structure will all 
             depent records from the db.
             */
            relation->destroy();

            // Mark the record to be erased from the vector.
            return true;
          } else {
            return false;
          }
        }),
      records.end()
    );


    LOG_F(INFO, "Successfully checked %lu Relations for deletion.", 
      records.size());
  }

  void RelationManager::deleteRelationByDatabaseAndName(string relationDatabase,
    string relationName) {

    auto relation = findByDatabaseAndName(relationDatabase, relationName);

    if (relation != nullptr) {
      relation->destroy();

      // Also remove relation from records
      remove(relation);
    } else {
      LOG_SCOPE_FUNCTION(INFO);
      LOG_F(INFO, "Didn't find Relation and thus couldn't it.");
      LOG_F(INFO, "RelationDB: %s", relationDatabase.c_str());
      LOG_F(INFO, "RelationName: %s", relationName.c_str());
    }
  }

  void RelationManager::deleteDerivativeByName(string relationDatabase,
    std::string relationName, std::string derivativeName) {

    
    auto relation = findByDatabaseAndName(relationDatabase, relationName);

    if(relation != nullptr) {
      relation->deleteDerivative(derivativeName);
    } else {      
      LOG_SCOPE_FUNCTION(INFO);
      LOG_F(INFO, "Didn't find Relation and thus couldn't delete Derivative.");
      LOG_F(INFO, "RelationDB: %s", relationDatabase.c_str());
      LOG_F(INFO, "RelationName: %s", relationName.c_str());
      LOG_F(INFO, "DerivativeName: %s", derivativeName.c_str());
    }
  }
}