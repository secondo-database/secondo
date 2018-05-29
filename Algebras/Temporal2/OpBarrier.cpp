/*
OpBarrier.cpp
Created on: 08.04.2018
Author: simon

implements a barrier

*/

#include "OpBarrier.h"
#include "Barrier.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Algebras/FText/FTextAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace temporal2algebra{

struct BarrierInfo : OperatorInfo {
  BarrierInfo() : OperatorInfo() {
      name =      "barrier";
      signature = "text x int -> int";
      syntax =    "barrier ('<id>', <num_instances>)";
      meaning =   "Waits until <num_of_instances> processes "
              "have joined the barrier with <id>.\n"
              "Returns the number of joining instances.";
  }
};

ListExpr Barrier_tm( ListExpr args ) {
    if (!nl->HasLength(args,2)) {
        return listutils::typeError("expected 2 arguments, but got "
                + nl->ToString(args));
    }
    if (!FText::checkType(nl->First(args))) {
        return listutils::typeError("expected " + FText::BasicType()
            + " as first argument, but got "
            + nl->ToString(nl->First(args)));
    }
    if (!CcInt::checkType(nl->Second(args))) {
        return listutils::typeError("expected " + CcInt::BasicType()
        + " as second argument, but got "
        + nl->ToString(nl->Second(args)));
    }

    return NList(CcInt::BasicType()).listExpr();
}


int Barrier_sf( ListExpr args ) {
    return 0;
}

int Barrier_vm
    ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  cout << "Barrier_vm(): ";
  std::string barrier_id = (static_cast<FText*>(args[0].addr))->GetValue();
  cout << "barrier_id: '" << barrier_id << "', ";
  int num_procs  = (static_cast<CcInt*>(args[1].addr))->GetValue();
  cout << "num_procs: " << num_procs << endl;

  Barrier myBarrier(barrier_id, num_procs);
  int actual_num_of_procs = myBarrier.wait();

  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*>(result.addr);
  *res = CcInt(actual_num_of_procs);

  return 0;
}

ValueMapping Barrier_vms[] =
{
    Barrier_vm
};

Operator* getBarrierOpPtr() {
    return new Operator(
            BarrierInfo(),
            Barrier_vms,
            Barrier_sf,
            Barrier_tm
           );
}

} // end of namespace temporal2algebra


