#ifndef SECONDO_REPLICA_ADAPTER_H
#define SECONDO_REPLICA_ADAPTER_H

#include "Algebras/DBService2/SecondoRecordAdapter.hpp"
#include "Algebras/DBService2/Replica.hpp"

namespace DBService {

  class Replica;

  class SecondoReplicaAdapter : 
    public SecondoRecordAdapter<DBService::Replica, SecondoReplicaAdapter> {

    public:

    static std::shared_ptr<DBService::Replica> buildObjectFromNestedList(
      std::string database, ListExpr recordAsNestedList,
      int resultListOffset = 0);
  };

}

#endif