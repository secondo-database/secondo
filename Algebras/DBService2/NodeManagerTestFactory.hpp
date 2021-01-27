#ifndef DBS_NODE_MANAGER_TEST_FACTORY_H
#define DBS_NODE_MANAGER_TEST_FACTORY_H

#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/NodeManager.hpp"
#include "Algebras/DBService2/NodeTestFactory.hpp"

namespace DBServiceTest {
  class NodeManagerTestFactory {
    
    public:

    static std::shared_ptr<DBService::NodeManager> buildNodeManagerWithNodes(
      std::string recordDatabase,      
      std::string nodeType,
      int lowestNodePort,
      int nrOfNodes,
      bool save = true
    ) {

      std::shared_ptr<DBService::NodeManager> nodeManager =
        std::make_shared<DBService::NodeManager>(recordDatabase);

      // Build nrOrNodes Nodes      
      std::vector<std::shared_ptr<DBService::Node> > nodes = 
        NodeTestFactory::buildNodes(
          recordDatabase, nodeType, lowestNodePort, nrOfNodes, false
        );

      for (auto& node : nodes) {
        nodeManager->add(node);
      }

      nodeManager->save();

      return nodeManager;
    }
  };
}
#endif