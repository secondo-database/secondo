#ifndef SECONDO_RELATION_ADAPTER_H
#define SECONDO_RELATION_ADAPTER_H

#include "Algebras/DBService2/SecondoRecordAdapter.hpp"
#include "Algebras/DBService2/Relation.hpp"

namespace DBService {

  class Relation;

  class SecondoRelationAdapter : 
    public SecondoRecordAdapter<DBService::Relation, SecondoRelationAdapter> {

    public:

    static std::shared_ptr<DBService::Relation> buildObjectFromNestedList(std::string database, ListExpr recordAsNestedList);
  };

}

#endif