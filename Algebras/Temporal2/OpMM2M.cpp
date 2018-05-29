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

#include "MMPoint.h"
#include "Algebras/Temporal/TemporalAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;
using namespace temporalalgebra;

namespace temporal2algebra{

struct MM2MInfo : OperatorInfo {
    MM2MInfo() : OperatorInfo() {
        name =      "mm2m";
        signature = "mpoint -> mmpoint";
        syntax =    "<MMPoint> mm2m;";
        meaning =   "Converts an <MMPoint> into an MPoint";
    }
};

ListExpr MM2M_tm( ListExpr args ) {
    if (!nl->HasLength(args,1)) {
        return listutils::typeError("expected 1 argument.");
    }

    if (!MMPoint::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + MMPoint::BasicType()
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
  MMPoint* mmpoint = static_cast<MMPoint*>(args[0].addr);

  result = qp->ResultStorage(s);
  MPoint* mpoint = static_cast<MPoint*>(result.addr);

  std::cout << "MM2M_vm got: "
          << mmpoint
          << std::endl;

  // logic from MPoint::CopyFrom(const Attribute* right)
  if (mmpoint->IsDefined()) {
      mpoint->Clear();
      mpoint->SetDefined(true);
      mpoint->StartBulkLoad();

      const std::vector<temporalalgebra::UPoint>& memUnits
          = mmpoint->memGet();
      std::vector<temporalalgebra::UPoint>::const_iterator it;
      for (it = memUnits.begin(); it != memUnits.end(); ++it) {
          mpoint->Add(*it);
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


