/*

*/
#ifndef SECONDO_SETUPTES_H
#define SECONDO_SETUPTES_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>
#include <vector>
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "../../Helpers/RemoteEndpoint.h"
#include "../../Helpers/WorkerConfig.h"

namespace distributed3 {
 class SetupTES {
 public:
  static ListExpr typeMapping(ListExpr args);

  static int valueMapping(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          void* s);

  static OperatorSpec operatorSpec;

  static Operator setupTES;

 private:
  //static void addWorkers(Word* args);
  static void resetTES();
  //static void startMessageServersAndClients();// noexcept(false);
  static void startLoopbackClients();// noexcept(false);
  static void startTESClients();// noexcept(false);
  static void startTESServers();// noexcept(false);

  static WorkerConfig workerFromTuple(Tuple*, int, int,
                                      int, int,
                                      std::string&,
                                      int);// noexcept(false);
 };
}


#endif //SECONDO_INITTES_H
