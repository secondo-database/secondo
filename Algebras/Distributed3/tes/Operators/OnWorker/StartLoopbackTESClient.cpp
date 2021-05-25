/*

*/
#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartLoopbackTESClient.h"
#include "../../MessageBroker/MessageBroker.h"

namespace distributed3 {
 ListExpr StartLoopbackTESClient::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 1)) {
   return listutils::typeError("You must provide 1 argument.");
  }
  const ListExpr workerNr = nl->First(args);

  if (!CcInt::checkType(workerNr)) {
   return listutils::typeError(
    "The first argument must be an int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartLoopbackTESClient::valueMapping(Word *args, Word &result, int,
                                              Word &, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *workernrWrapper = (CcInt *) args[0].addr;

  PRECONDITION(workernrWrapper->IsDefined(), "workernr is undefined")
  int workernr = workernrWrapper->GetIntval();
  PRECONDITION(workernr >= 0, "workernr must not be negative");


  bool loopbackProxyStarted =
     MessageBroker::get().startLoopbackProxy(workernr);

  if (!loopbackProxyStarted) {
   BOOST_LOG_TRIVIAL(error) << "Couldn't start loopback proxy";
   ((CcBool *) result.addr)->Set(true, false);
   return 0;
  }

  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec StartLoopbackTESClient::operatorSpec(
  "int -> bool",
  "# (_)",
  "This operator is used analogous to 'startMessageClient(...)'."
  "It also starts a message client, although the proxy doesn't send messages "
 "to other hosts. It queues the messages directly into the input queue of the "
  "local message broker.",
  "query startLoopbackMessageClient(1);",
  "This operator belongs to the TES API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
  "CAUTION: This operator is used internally by the TES system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the TES system."
 );

 Operator StartLoopbackTESClient::startLoopbackTESClient(
  "startLoopbackTESClient",
  StartLoopbackTESClient::operatorSpec.getStr(),
  StartLoopbackTESClient::valueMapping,
  Operator::SimpleSelect,
  StartLoopbackTESClient::typeMapping
 );
}
