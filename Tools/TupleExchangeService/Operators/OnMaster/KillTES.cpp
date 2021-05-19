/*

*/

#include <ListUtils.h>
#include <StandardTypes.h>
#include "KillTES.h"
#include "../../MessageBroker/MessageBroker.h"
#include "../../TESManager.h"
#include "../../Helpers/Commander.h"
#include "../../Helpers/WorkerConfig.h"
#include <GenericTC.h> // QueryProcessor
#include "iostream"

namespace distributed3 {
 ListExpr KillTES::typeMapping(ListExpr args) {
  if (!nl->IsEmpty(args)) {
   return listutils::typeError("You must provide no arguments.");
  }

  return listutils::basicSymbol<CcBool>();
 }

 int KillTES::valueMapping(Word *args, Word &result, int message,
                               Word &local, Supplier s) {
  result = qp->ResultStorage(s);
  TESManager::getInstance().reset();
  //clearWorkers();
  // the master does not have anything stored on the heap. 
  // So there is nothing to free on the master. 
  // The workers(WorkerConfig) of the TESManager are destroyed 
  // with the TESManager by entering quit.
  // the killed connection to the workers does not lead to calls of 
  // the destructors of their Objects.
  // TODO what about remoteMonitors ... stop ? 
  //std::cout << "\nTES teared down";
  ((CcBool *) result.addr)->Set(true, true);
  return 0;
 }

 OperatorSpec KillTES::operatorSpec(
  "() -> bool",
  "#",
  "This operator destroys the TES. It does so "
  "recursively for all connected workers.",
  "query killtes();",
  "It may require knowledge of the system to effectively understand and "
  "use all the operators that are provided."
 );

 Operator KillTES::killTES(
  "killtes",
  KillTES::operatorSpec.getStr(),
  KillTES::valueMapping,
  Operator::SimpleSelect,
  KillTES::typeMapping
 );
 // moved to TESManager
 void KillTES::clearWorkers() {
   auto workers = TESManager::getInstance().getWorkers();
   supplier<Runner> runners = [&workers]() -> Runner * {
     WorkerConfig *worker;
     if ((worker = workers()) != nullptr) {
      std::string query = "query resettes()"; 
      return new Runner(worker->connection, query);
     }
     return nullptr;
   };

    auto dummy = Commander::broadcast(runners, 
                                      Commander::throwWhenFalse, true);
    // ensure to delete the result store
    auto d1 = dummy();
    while(d1 != nullptr){
      d1 = dummy();
    }
 }
}
