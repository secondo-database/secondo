#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include "Algebras/DBService2/RecordManager.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"

#include <vector>

namespace DBService {

  //TODO Is NodeManager a good name? Maybe PersistentNodeVector?
  //TODO think about a generic RecordManager.

  /*
    A NodeManager is responsible for managing a set of Nodes.
    There can be several NodeManagers (the NM is not a singleton).
    NodeManagers are always responsible for a certain node type,  e.g.
    Node::nodeTypeDBService() (default) or NodeType::nodeTypeOriginal().

    By default the NodeManager is initialized for the NodeType ~DBService~.
  */
  class NodeManager : public RecordManager<DBService::Node, SecondoNodeAdapter> {

    protected:
    
    std::string nodeType;

    Query loadByNodeTypeQuery();

    public:
      
    NodeManager(std::string database);

    NodeManager(std::string database, std::string nodeType);

      
    std::shared_ptr<Node> findByHostnameAndPort(std::string hostname, int port);    

    std::vector<std::shared_ptr<Node> > getNodes();
  };
}
#endif