/*
OpStorageAction.cpp
Created on: 08.04.2018
Author: simon

Limitations and ToDos:
- Refactor to remove vm-function array
- c.f. StreamValve.cpp for further ToDos/Limitations

*/

#include "OpStorageAction.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "ListUtils.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "MemStorageManager.h"


#include "StreamValve.h"


extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace temporal2algebra{
const std::string pushMem = "pushMem";
const std::string printLog = "printLog";
const std::string printMem = "printMem";

struct StorageActionInfo : OperatorInfo {
    StorageActionInfo() : OperatorInfo() {
        name =      "storageaction";
        signature = "text -> int";
        syntax =    "storageaction('<Action>');";
        meaning =   "Performs <Action> on MemStorage\n"
                "Returns the number of affected Items";
    }
};

ListExpr StorageAction_tm( ListExpr args ) {
    if (!nl->HasLength(args,1)) {
        return listutils::typeError("expected 1 arguments.");
    }

    if (!FText::checkType(nl->First(nl->First(args)))) {
        return listutils::typeError("expected "
                + FText::BasicType()
                + " as first argument, but got "
                + nl->ToString(nl->First(nl->First(args))));
    }

    ListExpr action = nl->Second(nl->First(args));
    if (nl->AtomType(action) != TextType) {
        return listutils::typeError(
                "expected TextType contents in second Argument, but got "
                + nl->ToString(action));
    }
    std::string action_id = nl->Text2String(action);

    ListExpr result_type = NList(CcInt::BasicType()).listExpr();

    if (action_id == pushMem) {
        return result_type;
    } else if (action_id == printLog) {
        return result_type;
    } else if (action_id == printMem) {
        return result_type;
    }

    return listutils::typeError(
                    "unknown Action '"
                    + action_id
                    + "'");
}


int StorageAction_sf( ListExpr args ) {
  return 0;
}

int StorageAction_vm( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  //FText* valveId = static_cast<FText*>(args[0].addr);
  std::string action = (static_cast<FText*>(args[0].addr))->GetValue();

  result = qp->ResultStorage(s);
  CcInt* resCount = static_cast<CcInt*>(result.addr);

  std::cout << "StorageAction_vm got: "
            << action << std::endl;

  int resVal = 0;
  if (action == pushMem) {
      MemStorageManager* storage = MemStorageManager::getInstance();
      resVal = storage->pushToFlobs();
  } else if (action == printLog) {
      MemStorageManager* storage = MemStorageManager::getInstance();
      resVal = storage->printLog();
  } else if (action == printMem) {
      MemStorageManager* storage = MemStorageManager::getInstance();
      resVal = storage->printMem();
  } else {
      std::cout << "unknown action" << action << std::endl;
      return -1;
  }
  resCount->Set(true, resVal);
  return 0;
}

ValueMapping StorageAction_vms[] =
{
  StorageAction_vm
};

Operator* getStorageActionOpPtr() {
    Operator* op = new Operator(
            StorageActionInfo(),
            StorageAction_vms,
            StorageAction_sf,
            StorageAction_tm
           );
    op->SetUsesArgsInTypeMapping();
    return op;
}

} // end of namespace temporal2algebra


