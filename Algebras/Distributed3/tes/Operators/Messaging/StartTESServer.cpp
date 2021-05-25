/*

*/
#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartTESServer.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../TESContext.h"


namespace distributed3 {
 ListExpr StartTESServer::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr portNo = nl->First(args);

  if (!CcInt::checkType(portNo)) {
   return listutils::typeError(
    "The second argument must be of type int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartTESServer::valueMapping(Word *args, Word &result, int message,
                                      Word &local, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *portNo = (CcInt *) args[0].addr;

  PRECONDITION(portNo->IsDefined(), "portNo is undefined");
  int port = portNo->GetValue();
  PRECONDITION(port > 0, "port must be greater than 0");

  MessageBroker& broker = MessageBroker::get();

  TESContext::get().setMessageServerPort(port);
  if (broker.tcpListenerRunning()) {
   ((CcBool *) result.addr)->Set(true, false);
  } else {
   bool success = broker.startTcpListener(port);
   ((CcBool *) result.addr)->Set(true, success);
  }

  return 0;
 }

 OperatorSpec StartTESServer::operatorSpec(
  "int -> bool",
  "# (_)",
  "This operator starts a message server locally as component of the "
  "Pregel messaging system."
  "It accepts connects over tcp on the specified port to a "
  "(possibly) remote host that connects as a message client as counterpart."
  "The server acts as a receiver for Pregel messages from other workers.",
  "query startTESServer(9001);",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
  "CAUTION: This operator is used internally by the Pregel system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the Pregel system."
 );

 Operator StartTESServer::startTESServer(
  "startTESServer",
  StartTESServer::operatorSpec.getStr(),
  StartTESServer::valueMapping,
  Operator::SimpleSelect,
  StartTESServer::typeMapping
 );
}
