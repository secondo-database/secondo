/*

*/

#ifndef SECONDO_FEEDTES_H
#define SECONDO_FEEDTES_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>
#include <vector>
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


namespace distributed3 {
 class FeedTES {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator feedTES;
  
 };
}


#endif //SECONDO_FEEDTES_H
