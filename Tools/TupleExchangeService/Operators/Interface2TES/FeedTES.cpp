/*

*/

#include <StandardTypes.h>
#include "Stream.h"
#include "FeedTES.h"
#include "../../TESClient.h"
#include "../../TESContext.h"
#include <ListUtils.h>
#include <vector>
#include <QueryProcessor.h>

namespace distributed3 {

/**
   tupleType vom TESContext zu beziehen.
   SetUsesArgsInTypeMapping() ist noetig, um auch auf 
   den Wert des ersten Parameters (eid) zugreifen zu koennen. 
     
*/
ListExpr FeedTES::typeMapping(ListExpr args){

  std::string err = "int x int expected";
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("two arguments expected");
    return nl->TypeError();
  }
 
  ListExpr leid    = nl->First(args);
  ListExpr lslot   = nl->Second(args);
  // Uses args in type mapping
  if (!nl->HasLength(leid, 2) || !nl->HasLength(lslot, 2) ) {
   return listutils::typeError("Internal Failure");
  }
  
  if(!listutils::isSymbol(nl->First(leid))  ||
     !listutils::isSymbol(nl->First(lslot))){
       return listutils::typeError(err);
  }
  
  //ListExpr eidValue = nl->Second(leid);
  //int eid = nl->IntValue(eidValue);
  int eid = nl->IntValue(nl->Second(leid));
  ListExpr tupleType;
  bool ok = nl->ReadFromString(
                  TESContext::get().getStringTupleType(eid), tupleType);
  if (!ok) { // can't happen in partitiondmap
    ErrorReporter::ReportError("No tupleType set yet!");
    return nl->TypeError();
  }
  ListExpr res = nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType()),
                           tupleType);
  return res;
}

  int FeedTES::valueMapping(Word* args, Word& result,
                   int message, Word& local, Supplier s) {
  
  switch (message)
  {
    case OPEN:
    {
      return 0;       
    }
    case REQUEST: // return the next stream element
    { 
      // eid und slot könnte auch in local.addr zwischengespeichert werden
      int eid = ((CcInt*)args[0].addr)->GetIntval();
      int slot  = ((CcInt*)args[1].addr)->GetIntval();
      Tuple* t;
      if ((t = TESClient::get().getTuple(eid,slot)) != 0) {
        result.setAddr(t);
        return YIELD;
      } else {
        return CANCEL;
      }
    }
    case CLOSE:
      return 0;
  }  
  std::cerr << "should never happen";
  return -1;
}

OperatorSpec FeedTES::operatorSpec(
  "int x int -> stream(tuple)",
  "feedtes(_,_)",
  "eid, slot",
  "query feedtes(5,6) count"
);


Operator FeedTES::feedTES(
  "feedtes",
  FeedTES::operatorSpec.getStr(),
  FeedTES::valueMapping,
  Operator::SimpleSelect,
  FeedTES::typeMapping
);

}
