#ifndef DBS_REPLICA_PLACEMENT_STRATEGY_H
#define DBS_REPLICA_PLACEMENT_STRATEGY_H

#include "Algebras/DBService2/Relation.hpp"
#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/FaultToleranceMode.hpp"
#include "Algebras/DBService2/PlacementPolicy.hpp"

#include <vector>

namespace DBService {

  /*
    The ~ReplicaPlacementStrategy~ - as the name suggests - encapsulates the
    logic of selecting Nodes for the placement of Replicas for a given 
    Relation.

    The ~getMessage()~ function provides a human readable explanation of
    the placement decisions made by the stratgy. 
    The idea is to make the decision process transparent so that in the event
    of unexpected behavior the engineer or operator can investigate the 
    placement decision easily. 
    
    This increases the observability of the ~DBService~ which is a major
    contribution to improved operability. Both qualities are critical 
    successfactors for the adoption of distributed systems.
  */
  class ReplicaPlacementStrategy {
    private:

    PlacementPolicy policy;    
    std::vector<std::shared_ptr<DBService::Node> > nodes;
    
    std::stringstream message;
    
    public:

    ReplicaPlacementStrategy(PlacementPolicy newPolicy, 
      std::vector<std::shared_ptr<DBService::Node> > newNodes);

    /*
      Returns a description of the placement decision and potential errors.
    */
    std::string getMessage() const;

    /*
      Sets the PlacementPolicy describing constraints the placement strategy
      has to incorporate when making placement decisions.
    */
    void setPolicy(PlacementPolicy newPolicy);

    void setNodes(std::vector<std::shared_ptr<DBService::Node> > newNodes);

    /*
      Determines the placement of Replicas of a given Relation among
      available DBService Nodes.

      The result of the placement procedure is stored in the Relation by
      adding Replica objects to it.

      After executing the strategy, the Relation is unsaved (dirty).

      relation: Relation to be replicated.
    */
    bool doPlacement(std::shared_ptr<Relation> relation);

    /*
      Checks whether the given Node is compliant to the given
      PlacementPolicy and thus qualifies as a target Node to
      place a Replica.
    */
    bool isNodeCompliant(std::shared_ptr<Node> node, 
      std::shared_ptr<Relation> relation);

    /*
      Verifies whether the placement is compliant to the fault tolerance level
      specified by the PlacementPolicy.
    */
    bool isPlacementCompliant(
      std::vector<std::shared_ptr<Node> > selectedNodes);
  };
}
#endif