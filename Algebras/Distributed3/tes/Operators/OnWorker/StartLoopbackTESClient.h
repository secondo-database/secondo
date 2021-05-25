/*

*/

#ifndef SECONDO_INITTESWORKER_H
#define SECONDO_INITTESWORKER_H

#include <Operator.h>

namespace distributed3 {
class StartLoopbackTESClient {
public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator startLoopbackTESClient;
};
}


#endif //SECONDO_INITTESWORKER_H
