/*

*/

#ifndef SECONDO_DISTRIBUTE2TES_H
#define SECONDO_DISTRIBUTE2TES_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>
#include <vector>
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"


namespace distributed3 {
 class Distribute2TES {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator distribute2TES;
  
 };
}


#endif //SECONDO_DISTRIBUTE2TES_H
