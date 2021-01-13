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
#include "Algebras/DBService2/NodeManager.hpp"
#include "Algebras/DBService2/DatabaseAdapter.hpp"
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"

#include "NestedList.h"

#include <exception>
#include <algorithm>

using namespace std;

namespace DBService {

  NodeManager::NodeManager(string newDatabase) : RecordManager(newDatabase) { 

    // if (newDatabase == "")
    //   throw "Can't use the empty string as a database name!";
    
    // database = newDatabase;
    nodeType = DBService::Node::nodeTypeDBService();

    setLoadQuery(loadByNodeTypeQuery());
    setDatabaseAdapter(DatabaseAdapter::getInstance());
  }

  NodeManager::NodeManager(string newDatabase, string newNodeType) : 
    RecordManager(newDatabase) { 

    if (newNodeType == "")
      throw "Can't use empty string as the node type!";

    nodeType = newNodeType;

    setLoadQuery(loadByNodeTypeQuery());
    setDatabaseAdapter(DatabaseAdapter::getInstance());
  }

  /*
    Hiding the default ~loadQuery~ implementation to filter Nodes by their
    node type.
  */
  Query NodeManager::loadByNodeTypeQuery() {
    return Node::query(database).feed().filter(".Type = ?", 
      nodeType).addid().consume();    
  }

  shared_ptr<DBService::Node> NodeManager::findByHostnameAndPort(
    string hostname, int port) {    

    shared_ptr<Node> nodeToFind = make_shared<Node>(
      hostname, port, "", "", 0, 0);

    return find(nodeToFind);
  }

  // string NodeManager::getDatabase() const {
  //   return database;
  // }

  //TODO Refactor uses of getNodes() to use getAll() instead.
  vector<shared_ptr<Node> > NodeManager::getNodes() {
    return getAll();
  }
}