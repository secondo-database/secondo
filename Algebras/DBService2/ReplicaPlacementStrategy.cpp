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
#include "Algebras/DBService2/ReplicaPlacementStrategy.hpp"

#include <random>
#include <algorithm>

using namespace std;

namespace DBService {

  ReplicaPlacementStrategy::ReplicaPlacementStrategy(PlacementPolicy newPolicy, 
      std::vector<shared_ptr<DBService::Node> > newNodes) {

      setPolicy(newPolicy);
      setNodes(newNodes);
  }

  string ReplicaPlacementStrategy::getMessage() const {
    return message.str();
  }

  void ReplicaPlacementStrategy::setPolicy(PlacementPolicy newPolicy) {
    policy = newPolicy;
  }

  void ReplicaPlacementStrategy::setNodes(
    std::vector<shared_ptr<DBService::Node> > newNodes) {
    nodes = newNodes;
  }
  

  /*
    Assigns replicas to available DBService Nodes according to the given 
    ~ReplicaPlacementPolicy~.

    In case of a successful placement, Replicas will be created and
    added to the given Relation. Neither the Replicas nor the Relation will
    be saved and thus will be "dirty".

    In case of a failed placement, e.g. due to missing DBService Nodes in
    relation to the requires number of replicas, the Relation remains unchanged.
  */
  bool ReplicaPlacementStrategy::doPlacement(shared_ptr<Relation> relation) {

    // Nodes selected for the placement
    vector<shared_ptr<Node> > selectedNodes;

    // Shuffle nodes to create a simple load balancing.
    std::random_device rd;
    std::mt19937 rng(rd());    
    vector<shared_ptr<Node> > shuffledNodes = nodes;    
    std::shuffle(shuffledNodes.begin(), shuffledNodes.end(), rng);

    for(const auto& node : shuffledNodes) {
      if (isNodeCompliant(node, relation)) {        
        message << "Node " << node->getHost().getHostname() << ", ";
        message << node->getPort() << " is compliant.";        
        selectedNodes.push_back(node);
      } else {
        message << "Node " << node->getHost().getHostname() << ", ";
        message << node->getPort() << " is NOT compliant.";
      }
      message << endl;

      // Avoid creating too many Replicas
      if (selectedNodes.size() >= policy.minimumNrOfReplicas) {
        break;
      }
    }

    message << "The placement has selected " << selectedNodes.size();
    message << " nodes for placing replicas. " << policy.minimumNrOfReplicas;
    message << " are required." << endl;

    // Verify that enough replicas have been created.
    if (!isPlacementCompliant(selectedNodes)) {
      
      message << "The placement of replicas for the relation " << endl;
      
      //TODO Print the selected replicas
      message << *relation << endl << " is not compliant with the given ";
      message << "policy." << endl;            

      return false;
    }

    // The placement was successful, create replicas for selected nodes
    for (const auto &node : selectedNodes) {
      relation->addReplica(node);
    }

    message << endl << "The placement was successful." << endl;

    message << "Relation after the placement: " << endl;
    message << *relation << endl;

    return true;
  }

  bool ReplicaPlacementStrategy::isNodeCompliant(
    shared_ptr<DBService::Node> node, 
    shared_ptr<DBService::Relation> relation) {

    bool equal = true;

    // A node is disqualified as a target node if ...
    switch (policy.faultToleranceMode) {
      case FaultToleranceMode::DISK:        
        // DISK mode is on and originalNode and target node are equal in host, 
        // port and diskpath
        if (*relation->getOriginalNode() == *node && 
          relation->getOriginalNode()->getDiskPath() == node->getDiskPath()) {

          message << "The designated target Node " << node->str();
          message << " is disk-equal with the Relation's original Node.";
          message << endl;
          equal = false;
        }
        break;
      case FaultToleranceMode::NODE:
        // NODE mode is on and originalNode and target node are equal in host, 
        //  port
        if (*relation->getOriginalNode() == *node) {          
          message << "The designated target Node " << node->str();
          message << " is node-equal with the Relation's original Node.";
          message << endl; 
          equal = false;
        }
        break;
      case FaultToleranceMode::NONE:
        message << "For the designate target Node " << node->str();
        message << " node-equality with the Relation's original Node has not \
been considered.";

        message << endl;
        
        equal = true;
        break;
      default:
        throw "Unknown placement policy / fault tolerance mode!"; 
        break;
    }

    return equal;
  }

  bool ReplicaPlacementStrategy::isPlacementCompliant(
    vector<shared_ptr<Node> > selectedNodes) {

    // TODO Implement relation->getReplicaCount()
    return (selectedNodes.size() >= policy.minimumNrOfReplicas);
  }
}