/*

*/

#include <StandardTypes.h>
#include "Stream.h"
#include "Distribute2TES.h"
#include "../../TESClient.h"
#include <ListUtils.h>
#include <vector>
#include <QueryProcessor.h>

namespace distributed3 {

 class distribute2tesInfo{
  public:
  distribute2tesInfo(Word _stream, CcInt* _eid, Supplier _fun, 
  CcInt* _numberOfSlots, CcInt* _numberOfWorkers/*,
                   ListExpr _relType*/): stream(_stream), fun(_fun) {
    eid = _eid->GetValue(); 
    numberOfSlots = _numberOfSlots->GetValue();
    numberOfWorkers = _numberOfWorkers->GetValue();
    //relType = _relType;
    // server starten würde hier nicht funktionieren, da ja gerade 
    // die Server auf den anderen Workern gestartet werden müssten.
       
    stream.open();
     
    funArgs = qp->Argument(fun);
  }

  ~distribute2tesInfo(){
      stream.close();
   }

   Tuple* next(){
     Tuple* t = stream.request();
     /* wenn der Strom erschöpft ist, muss an alle Worker eine 
        Finish-Nachricht geschickt werden.
        Wenn TESClient nullptr erhielte, müsste jedesmal auf nullptr 
        geprüft werden und
        an die Worker, an die keine Tupel geschickt worden sind, müsste 
        die Finish-Nachricht separat verschickt werden.
        
        Wenn der Strom erschöpft ist, kann auch eine separate 
        Funktion aufgerufen werden:
        TESClient::get().endOfTupleStreamFor(eid);
     */
     if(!t /*|| numberOfSlots <=0*/) {
       TESClient::get().endOfTupleStreamFor(eid);
        //return t;
        return nullptr;
     }
     t->IncReference(); // tuple wird von count gelöscht 
     (*funArgs)[0] = t;
     Word r = qp->Request(fun);
     CcInt* res = (CcInt*) r.addr;
     int resi = res->IsDefined()?res->GetValue():0;
     int slot = resi % numberOfSlots;
     int workerNumber = slot % numberOfWorkers;
     TESClient::get().putTuple(eid,slot,workerNumber,t);
     return t;
   }

  private:
     Stream<Tuple> stream;
     Supplier fun;
     ArgVectorPointer funArgs;
     int eid;
     int numberOfWorkers;
     int numberOfSlots;
     //ListExpr relType;
    
};   

 ListExpr Distribute2TES::typeMapping(ListExpr args) {

  std::string err = "stream(tuple) x int x (tuple->int) x int x int expected";
  if(!nl->HasLength(args,5)){
    return listutils::typeError(err + " (wrong number of arguments)");
  }
  if(    !Stream<Tuple>::checkType(nl->First(args) )
      || !CcInt::checkType(nl->Second(args))
      || !listutils::isMap<1>(nl->Third(args)) 
      || !CcInt::checkType(nl->Fourth(args))
      || !CcInt::checkType(nl->Fifth(args))){
    return listutils::typeError(err);
  }
  
  if(!nl->Equal(nl->Second(nl->First(args)),
                nl->Second(nl->Third(args)))){
    return listutils::typeError("type mismatch between tuple type "
                                "and function arg");
  }
  if(!CcInt::checkType(nl->Third(nl->Third(args)))) {
    return listutils::typeError("function does not result in an integer");
  }
  return nl->First(args);
 }
 
 int Distribute2TES::valueMapping(Word* args, Word& result, int message,
                                     Word& local, Supplier s ) {
  distribute2tesInfo* li = (distribute2tesInfo*) local.addr;
  switch(message){
    case OPEN :{
         if(li){
           delete li;
         }
         local.addr = new distribute2tesInfo(
                            args[0],               // stream
                            (CcInt*) args[1].addr, // eid
                            qp->GetSon(s,2),       // Supplier 
                            //für partitionfunText
                            (CcInt*) args[3].addr, // numberOfSlots
                            (CcInt*) args[4].addr/*, // numberOfWorkers
                            nl->TwoElemList(
                               listutils::basicSymbol<Relation>(),
                               nl->Second(qp->GetType(s))) */);
         return 0;
    }
    case REQUEST:
       result.addr = li?li->next():nullptr;
       return result.addr?YIELD:CANCEL;
    case CLOSE:
        if(li){
          delete li;
          local.addr = 0;
        }
        return 0;
  }
  return -1;
 }
 OperatorSpec Distribute2TES::operatorSpec(
   " stream(tuple) x int x (tuple->int) x int x int -> stream(tuple)",
     "_ distribute2tes[eid, tuple2intfun, numberOfSlots, numberOfWorkers]",
     "Distributes a tuple stream via the worker's TESClient. " 
     "The tuples are distributed according a function given "
     "by the user. The target slot for a tuple is the result of this "
     "function modulo numberOfSlots. And the target worker is "
     "the result of the target slot modulo numberOfWorkers. ",
"query strassen feed distribute2tes[77, hashvalue(.Name, 9997), 12, 5] count"
 );
 Operator Distribute2TES::distribute2TES(
   "distribute2tes",
   Distribute2TES::operatorSpec.getStr(),
   Distribute2TES::valueMapping,
   Operator::SimpleSelect,
   Distribute2TES::typeMapping
 );
               
}
