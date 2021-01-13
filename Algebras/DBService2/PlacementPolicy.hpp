#ifndef DBS_PLACEMENT_POLICY_H
#define DBS_PLACEMENT_POLICY_H

#include "Algebras/DBService2/FaultToleranceMode.hpp"

namespace DBService {
  struct PlacementPolicy {
    FaultToleranceMode faultToleranceMode;
    int minimumNrOfReplicas;
  };
}

#endif