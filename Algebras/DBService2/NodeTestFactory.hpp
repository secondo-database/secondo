#ifndef DBS_NODE_TEST_FACTORY_H
#define DBS_NODE_TEST_FACTORY_H

#include "Algebras/DBService2/Node.hpp"

namespace DBServiceTest {

  class NodeTestFactory {

    // static int nextNodePort;

    public:

    /*
      Returns a saved (non-new) targetNode.

      If ~save~ is set to ~false~ then the targetNode will be returned
      unsaved.
    */
    static std::shared_ptr<DBService::Node> buildTargetNode(
        std::string recordDatabase,
        int nodePort, 
        bool save = true 
      ) {

      std::shared_ptr<DBService::Node> targetNode = buildNode(
        recordDatabase,
        DBService::Node::nodeTypeDBService(),
        nodePort,
        false
      );

      targetNode->setType(DBService::Node::nodeTypeDBService());    

      if (save == true)
        targetNode->save();

      return targetNode;
    }

    /*
      Creates the given number of nodes.
    */
    static std::vector<std::shared_ptr<DBService::Node> > buildNodes(
        std::string recordDatabase,
        std::string nodeType,
        int lowestNodePort, 
        int numberOfNodes,
        bool save = true 
      ) {
      
      std::vector<std::shared_ptr<DBService::Node> > nodes;
      std::shared_ptr<DBService::Node> node;

      int currentNodePort = lowestNodePort;

      for(int i=0; i<numberOfNodes; i++) {
        
        node = buildNode(
          recordDatabase,
          nodeType,
          currentNodePort,
          save
        );

        currentNodePort++;

        nodes.push_back(node);
      }
      

      return nodes;
    }

    static std::shared_ptr<DBService::Node> buildNode(
        std::string recordDatabase,
        std::string nodeType,
        int nodePort, 
        bool save = true 
      ) {
      std::shared_ptr<DBService::Node> targetNode = 
        std::make_shared<DBService::Node>(
          "localhost", nodePort, "", 
          "/home/doesnt_exist/secondo", 9941, 9942);

      targetNode->setDatabase(recordDatabase);
      targetNode->setType(nodeType);

      if (save == true)
        targetNode->save();

      return targetNode;
    }

    

    //TODO Using statics in header files causes linker errors. Thank you c++! :)
    // static int getUniqueNodePort() {
    //   if (NodeTestFactory::nextNodePort == 0)
    //     NodeTestFactory::nextNodePort = 4740;

    //   // 4740, 4741, ...
    //   return NodeTestFactory::nextNodePort++;
    // }
  };
}

#endif