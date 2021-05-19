/*

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "SetTupleType.h"
#include <boost/log/trivial.hpp>
#include "../../TESContext.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../MessageBroker/Message.h"

namespace distributed3 {
  
 ListExpr SetTupleType::typeMapping(ListExpr args) {
  nl->WriteListExpr(args);
  if (!nl->HasLength(args, 3)) {
   return listutils::typeError("You must provide 3 arguments.");
  }
  
  if (!CcInt::checkType(nl->First(args))) {
   return listutils::typeError("The first argument must be an int");
  }

  if (!Relation::checkType(nl->Second(args))) {
   return listutils::typeError("The second argument must be of type relation");
  }
  if (!CcInt::checkType(nl->Third(args))) {
   return listutils::typeError("The third argument must be an int");
  }
  
  return listutils::basicSymbol<CcBool>();
 }

 int SetTupleType::valueMapping(Word *args, Word &result, int,
                                           Word &, Supplier s) {
  int eid = ((CcInt*)args[0].addr)->GetIntval();
  ListExpr tupleTypeList = nl->Second(qp->GetType(qp->GetSon(s,1)));
  int numberOfSlots = ((CcInt*)args[2].addr)->GetIntval();
  TESContext::get().setTupleType(eid, tupleTypeList);
  TESContext::get().setNumberOfSlots(eid,numberOfSlots);
  MessageBroker::get().setNumberOfSlots(eid,numberOfSlots);
  result = qp->ResultStorage(s);
  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec SetTupleType::operatorSpec(
  "int x tuple -> bool",
  "# (_)",
  "This operator sets the tuple type."
  "and the (new) number of slots",
  "query setTupleType(7, Relation, 40);",
  ""
 );

 Operator SetTupleType::setTupleType(
  "setTupleType",
  SetTupleType::operatorSpec.getStr(),
  SetTupleType::valueMapping,
  Operator::SimpleSelect,
  SetTupleType::typeMapping
 );
}
