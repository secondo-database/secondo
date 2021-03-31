#ifndef SECONDO_NODE_ADAPTER_H
#define SECONDO_NODE_ADAPTER_H

#include "Algebras/DBService2/Node.hpp"
#include "Algebras/DBService2/SecondoRecordAdapter.hpp"

#include "NestedList.h"

namespace DBService
{

  class Node;

  /* Provides conversion functions from Secondo Nested Lists
   * to construct Node objects.
   * The goal is to encapsulate all Secondo dependencies in a central place.
   * 
   * TODO However, this is an application specific 
   *  adapter and thus should be shipped with the application rather than the 
   *  ORM.
   *  Or with an Algebra.
   * 
   */
  class SecondoNodeAdapter : public SecondoRecordAdapter<DBService::Node, 
    SecondoNodeAdapter> {

    public:

      /** Input is a nested list.
       * Returns a newly constructed Node object.
       */
      static std::shared_ptr<Node> buildObjectFromNestedList(
        std::string database, ListExpr recordAsNestedList, 
        int resultListOffset = 0);
  };
} // namespace DBService

#endif