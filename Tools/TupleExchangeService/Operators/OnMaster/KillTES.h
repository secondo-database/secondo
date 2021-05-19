/*

*/

#ifndef SECONDO_KILLTES_H
#define SECONDO_KILLTES_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>

namespace distributed3 {
 class KillTES {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator killTES;
  
  static void clearWorkers();
 };
}


#endif //SECONDO_KILLTES_H
