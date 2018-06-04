/*
OpM2MM.cpp
Created on: 08.04.2018
Author: simon

Limitations and ToDos:
- Refactor to remove vm-function array
- c.f. StreamValve.cpp for further ToDos/Limitations

*/
#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "ListUtils.h"

#include "MPoint2.h"
#include "Algebras/Temporal/TemporalAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;
using namespace temporalalgebra;

namespace temporal2algebra{

struct M2MMInfo : OperatorInfo {
    M2MMInfo() : OperatorInfo() {
        name =      "m2mm";
        signature = "mpoint -> mpoint2";
        syntax =    "<MPoint> m2mm;";
        meaning =   "Converts an <MPoint> into an MPoint2";
    }
};

ListExpr M2MM_tm( ListExpr args ) {
    if (!nl->HasLength(args,1)) {
        return listutils::typeError("expected 1 argument.");
    }

    if (!MPoint::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + MPoint::BasicType()
                + " as first argument, but got "
                + nl->ToString(nl->First(args)));
    }

    return NList(MPoint2::BasicType()).listExpr();
}


int M2MM_sf( ListExpr args ) {
  return 0;
}

int M2MM_vm( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  //FText* valveId = static_cast<FText*>(args[0].addr);
  MPoint* mpoint = static_cast<MPoint*>(args[0].addr);

  result = qp->ResultStorage(s);
  MPoint2* mpoint2 = static_cast<MPoint2*>(result.addr);

  std::cout << "M2MM_vm got: "
          << *mpoint
          << std::endl;

  // logic from MPoint::CopyFrom(const Attribute* right)
  if (mpoint->IsDefined()) {
      UPoint unit(false);
      for ( int i = 0; i < mpoint->GetNoComponents(); i++) {
          mpoint->Get(i, unit);
          mpoint2->memAppend(unit);
      }
      mpoint2->SetDefined(true);
  } else {
      mpoint2->SetDefined(false);
  }
  return 0;
}

ValueMapping M2MM_vms[] =
{
  M2MM_vm
};

Operator* getM2MMOpPtr() {
    return new Operator(
            M2MMInfo(),
            M2MM_vms,
            M2MM_sf,
            M2MM_tm
           );
}

} // end of namespace temporal2algebra


