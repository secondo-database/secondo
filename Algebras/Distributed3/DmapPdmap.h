/*

*/

#ifndef SECONDO_DMAPPDMAP_H
#define SECONDO_DMAPPDMAP_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>
#include <vector>
#include "Algebras/Distributed2/fsrel.h"
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Distributed3/Distributed3Algebra.h"


namespace distributed3 {
 class DmapPdmap {
 public:
  static ListExpr typeMapping(ListExpr args);
  
  template<class A>
  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);
                          
  static ValueMapping dmapPdmapVM[];
  static int dmapPdmapSelect(ListExpr args);
  static OperatorSpec operatorSpec;

  static Operator dmapPdmapOp;
  
 };
}


#endif //SECONDO_DMAPPDMAP_H
