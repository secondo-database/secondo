#ifndef SECONDO_DERIVATIVE_ADAPTER_H
#define SECONDO_DERIVATIVE_ADAPTER_H

#include "Algebras/DBService2/SecondoRecordAdapter.hpp"
#include "Algebras/DBService2/Derivative.hpp"

namespace DBService {

  class Derivative;

  class SecondoDerivativeAdapter : 
    public SecondoRecordAdapter<DBService::Derivative, SecondoDerivativeAdapter> {

    public:

    static std::shared_ptr<DBService::Derivative> buildObjectFromNestedList(std::string database, ListExpr recordAsNestedList);
  };

}

#endif