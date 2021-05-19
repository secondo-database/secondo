/*

*/

#ifndef SECONDO_RESETTESWORKER_H
#define SECONDO_RESETTESWORKER_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>

namespace distributed3 {
 class ResetTES {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator resetTES;
 };
}


#endif //SECONDO_RESETTESWORKER_H
