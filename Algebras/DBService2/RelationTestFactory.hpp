#ifndef DBS_SRELATION_TEST_FACTORY_H
#define DBS_SRELATION_TEST_FACTORY_H

#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/NodeTestFactory.hpp"

namespace DBServiceTest {

  //TODO Some of these factory methods - without their default values - could
  //  be moved to Relation.
  class RelationTestFactory {
    public: 

    static std::shared_ptr<DBService::Relation> buildRelationWithOriginalNode(
      std::string recordDatabase, 
      std::string relationName = "relationWithOriginalNode",
      int originalNodePort = 4712,
      bool save = true) {
      
      // Relation
      std::shared_ptr<DBService::Relation> relation = DBService::Relation::build(recordDatabase, relationName);
      relation->setDatabase(recordDatabase);

      // Original Node
      shared_ptr<DBService::Node> node = make_shared<DBService::Node>(
        "localhost", originalNodePort, "", "/home/doesnt_exist/secondo", 9941, 9942);      
      node->setDatabase(recordDatabase);
      node->setType(DBService::Node::nodeTypeOriginal());      
      relation->setOriginalNode(node); 

      if (save == true)
        relation->save();

      return relation;
    }

    /*
      Builds an Relation with an original Node, a targetNode and a Replica.      
    */
    static std::shared_ptr<DBService::Relation> buildRelationWithReplica(
      std::string recordDatabase, 
      std::string relationName = "relationWithReplica",
      int originalNodePort = 4712,
      int targetNodePort = 4713, 
      bool save = true) {

      std::shared_ptr<DBService::Relation> relationWithReplica = 
        buildRelationWithOriginalNode(
          recordDatabase, relationName, originalNodePort, false);      

      // Target Node
      shared_ptr<DBService::Node> targetNode = NodeTestFactory::buildTargetNode(recordDatabase, targetNodePort);

      //TODO Remove
      // shared_ptr<DBService::Node> targetNode = make_shared<DBService::Node>(
      //   "localhost", targetNodePort, "", 
      //   "/home/doesnt_exist/secondo", 9941, 9942);            
      // targetNode->setDatabase(recordDatabase);
      // targetNode->setType(DBService::Node::nodeTypeDBService());    
      // targetNode->save();

      // Replica
      relationWithReplica->addReplica(targetNode);

      // Save the replica by saving the relation
      if (save == true)
        relationWithReplica->save();

      return relationWithReplica;
    }
  };
}

#endif