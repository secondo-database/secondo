/*

*/
#include <ListUtils.h>
#include <StandardTypes.h>
#include "ResetTES.h"
#include "../../TESContext.h"
#include "../../MessageBroker/MessageBroker.h"
#include <GenericTC.h> // QueryProcessor
#include "iostream"

namespace distributed3 {
 ListExpr ResetTES::typeMapping(ListExpr args) {
  if (!nl->IsEmpty(args)) {
   return listutils::typeError("You must provide no arguments.");
  }
  return listutils::basicSymbol<CcBool>();
 }

 int ResetTES::valueMapping(Word *args, Word &result, int message,
                               Word &local, Supplier s) {
  result = qp->ResultStorage(s);

  MessageBroker::get().reset();
  std::cout << "\nnach MessageBroker.reset()";
  TESContext::get().reset(); // könnte auch in dessen Destruktor?
  std::cout << "\nnach TESContext.reset()";
  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec ResetTES::operatorSpec(
  "() -> bool",
  "#",
  "This operator resets the runtime state of the Pregel system. It does so "
  "recursively for all connected workers.",
  "query resetPregel();",
  "This operator belongs to the Pregel API."
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
 );

 Operator ResetTES::resetTES(
  "resettes",
  ResetTES::operatorSpec.getStr(),
  ResetTES::valueMapping,
  Operator::SimpleSelect,
  ResetTES::typeMapping
 );
}
