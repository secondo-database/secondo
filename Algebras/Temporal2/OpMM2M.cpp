/*
OpMM2M.cpp
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

struct MM2MInfo : OperatorInfo {
    MM2MInfo() : OperatorInfo() {
        name =      "mm2m";
        signature = "mpoint -> mpoint2";
        syntax =    "<MPoint2> mm2m;";
        meaning =   "Converts an <MPoint2> into an MPoint";
    }
};

ListExpr MM2M_tm( ListExpr args ) {
    if (!nl->HasLength(args,1)) {
        return listutils::typeError("expected 1 argument.");
    }

    if (!MPoint2::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + MPoint2::BasicType()
                + " as first argument, but got "
                + nl->ToString(nl->First(args)));
    }

    return NList(MPoint::BasicType()).listExpr();
}


int MM2M_sf( ListExpr args ) {
  return 0;
}

int MM2M_vm( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  MPoint2* mpoint2 = static_cast<MPoint2*>(args[0].addr);

  result = qp->ResultStorage(s);
  MPoint* mpoint = static_cast<MPoint*>(result.addr);

  std::cout << "MM2M_vm got: "
          << mpoint2
          << std::endl;

  // logic from MPoint::CopyFrom(const Attribute* right)
  if (mpoint2->IsDefined()) {
      mpoint->Clear();
      mpoint->SetDefined(true);
      mpoint->StartBulkLoad();

      UPoint unit(false);
      for ( int i = 0; i < mpoint2->GetNoComponents(); i++) {
          mpoint2->Get(i, unit);
          mpoint->Add(unit);
      }

      mpoint->EndBulkLoad(false);
  } else {
      mpoint->Clear();
      mpoint->SetDefined(false);
  }
  return 0;
}

ValueMapping MM2M_vms[] =
{
  MM2M_vm
};

Operator* getMM2MOpPtr() {
    return new Operator(
            MM2MInfo(),
            MM2M_vms,
            MM2M_sf,
            MM2M_tm
           );
}

} // end of namespace temporal2algebra


