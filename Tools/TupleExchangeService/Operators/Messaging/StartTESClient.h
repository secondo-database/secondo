/*

*/

#ifndef SECONDO_STARTTESCLIENT_H
#define SECONDO_STARTTESCLIENT_H

#include <Operator.h>

namespace distributed3 {
 class StartTESClient {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  static OperatorSpec operatorSpec;

  static Operator startTESClient;
 };
}


#endif //SECONDO_STARTMESSAGECLIENT_H
