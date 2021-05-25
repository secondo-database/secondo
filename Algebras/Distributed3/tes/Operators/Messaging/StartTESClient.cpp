/*

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "StartTESClient.h"
#include "../../Helpers/RemoteEndpoint.h"
#include "../../MessageBroker/MessageBroker.h"

namespace distributed3 {
 ListExpr StartTESClient::typeMapping(ListExpr args) {
  if (!nl->HasLength(args, 3)) {
   return listutils::typeError("You must provide 3 arguments.");
  }
  const ListExpr worker = nl->First(args);
  const ListExpr host = nl->Second(args);
  const ListExpr port = nl->Third(args);

  if (!CcInt::checkType(worker)) {
   return listutils::typeError(
    "The first argument (worker) must be of type int");
  }
  if (!CcString::checkType(host)) {
   return listutils::typeError(
    "The second argument (host) must be of type string");
  }
  if (!CcInt::checkType(port)) {
   return listutils::typeError(
    "The second argument (port) must be of type int");
  }

  return nl->SymbolAtom(CcBool::BasicType());
 }

 int StartTESClient::valueMapping(Word *args, Word &result, int message,
                                      Word &local, Supplier s) {
  result = qp->ResultStorage(s);
  CcInt *workerNoObj = (CcInt *) args[0].addr;
  CcString *hostObj = (CcString *) args[1].addr;
  CcInt *portWrapper = (CcInt *) args[2].addr;

  PRECONDITION(workerNoObj->IsDefined(), "worker is undefined");
  PRECONDITION(hostObj->IsDefined(), "host is undefined");
  PRECONDITION(portWrapper->IsDefined(), "port is undefined");
  const int worker = workerNoObj->GetValue();
  const std::string &hostName = hostObj->GetValue();
  const int port = portWrapper->GetValue();
  PRECONDITION(port > 0, "port must be grater than 0")
  PRECONDITION(worker >= 0, "worker must be grater or equal to 0")
  PRECONDITION(!hostName.empty(), "host must not be empty")

  const RemoteEndpoint host(hostName, port);
  MessageBroker &broker = MessageBroker::get();
  bool clientStarted = broker.startClient(worker, host);

  ((CcBool *) result.addr)->Set(true, clientStarted);
  return 0;
 }

 OperatorSpec StartTESClient::operatorSpec(
  "int x string x int -> bool",
  "# (_,_,_)",
  "This operator starts a message client locally as component of the "
  "Pregel messaging system."
  "It connects to a (possibly) remote host that runs a message server "
  "as counterpart."
  "The client acts as a sender for Pregel messages to other workers."
  "It has an index, which designates the logical address of the connected "
  "message server."
  "Arguments are (<index>, <host>, <port>)",
  "query startTESClient(2, \"localhost\", 8001);",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and use "
  "all the operators that are provided."
  "CAUTION: This operator is used internally by the Pregel system. "
  "Hence you must not use it in queries yourself."
  "Doing so may lead to inconsistent states of the Pregel system."
 );

 Operator StartTESClient::startTESClient(
  "startTESClient",
  StartTESClient::operatorSpec.getStr(),
  StartTESClient::valueMapping,
  Operator::SimpleSelect,
  StartTESClient::typeMapping
 );
}
