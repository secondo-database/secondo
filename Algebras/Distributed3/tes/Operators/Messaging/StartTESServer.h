/*

*/

#ifndef SECONDO_STARTTESSERVER_H
#define SECONDO_STARTTESSERVER_H

#include <Operator.h>

namespace distributed3 {
 class StartTESServer {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator startTESServer;
 };
}


#endif //SECONDO_STARTTESSERVER_H
