/*

*/

#ifndef SECONDO_SETTUPLETYPE_H
#define SECONDO_SETTUPLETYPE_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>

namespace distributed3 {

 class SetTupleType {
 public:

  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);


  static OperatorSpec operatorSpec;

  static Operator setTupleType;
 
 };
}


#endif //SECONDO_SETTUPLETYPE_H
