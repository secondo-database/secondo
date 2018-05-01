/*
OpExtendWith.cpp
Created on: 08.04.2018
Author: simon

Limitations and Todos:
- only naive implementation
    (optimization for fast concurrent updates is topic of bachelor thesis)
- only MPoint implemented so far
- only Streams (but no single IPoints) working

*/

#include "OpExtendWith.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace temporalalgebra;

namespace temporal2algebra{

struct ExtendWithInfo : OperatorInfo {
  ExtendWithInfo() : OperatorInfo() {
      name =      "extendwith";
      signature = "m(alpha) x stream(i(alpha)) -> m(alpha), alpha in {point}"
              "- {bool, string, int, real} not implemented yet";
      syntax =    "<MPoint> extendwith <stream(IPoint)>";
      meaning =   "Returns the MPoint extended by the Stream of IPoints";
  }
};

ListExpr ExtendWith_tm( ListExpr args ) {
    if (!nl->HasLength(args,2)) {
        return listutils::typeError("expected 2 arguments.");
    }

    if (!MPoint::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + MPoint::BasicType()
                + " as first argument");
    }

    if (!Stream<IPoint>::checkType(nl->Second(args))) {
        return listutils::typeError("expected "
                + Stream<IPoint>::BasicType()
                + "(" + IPoint::BasicType()
                + ") as second argument");
    }

    return NList(MPoint::BasicType()).listExpr();
}


int ExtendWith_sf( ListExpr args ) {
  return 0;
}

template<class TiAlpha, class TuAlpha>
void CreateStartExtensionUnit(const TiAlpha* iAlpha, TuAlpha* result) {
    Instant instant = iAlpha->instant;
    Interval<Instant> interval(instant, instant, true, true);
    TuAlpha resUnit(interval, iAlpha->value, iAlpha->value);
    *result = resUnit;
    return;
}

template<class TuAlpha, class TiAlpha>
void CreateRegularExtensionUnit(const TiAlpha* final_intime_mAlpha,
        const TiAlpha* iAlpha,
        TuAlpha* result ) {

    // TODO: corner case: previous unit is right open but same instant.
    if (iAlpha->instant > final_intime_mAlpha->instant) {
        Interval<Instant> res_interval(
                final_intime_mAlpha->instant, iAlpha->instant, false, true);
        TuAlpha extension_unit(
                res_interval, final_intime_mAlpha->value, iAlpha->value);
        *result = extension_unit;
        return;
    }
    result->SetDefined(false);
    return;
}


// calculate the next Unit based on the an m(alpha) for a given i(alpha)
template<class TiAlpha, class TuAlpha>
void CreateExtensionUnit(
        const TuAlpha* uAlpha,
        const TiAlpha* iAlpha,
        TuAlpha* res_uAlpha) {

    if (!iAlpha->IsDefined()) {
        // nothing to append => nothing to do
        *res_uAlpha = TuAlpha(false);
        return;
    }

    if (!uAlpha->IsDefined()) {
        // we have no "starting point" => create a single "point" unit
        TuAlpha result;
        CreateStartExtensionUnit(iAlpha, &result);
        *res_uAlpha = result;
        return;
    }

    TiAlpha final_instant(true);
    uAlpha->TemporalFunction(
            uAlpha->timeInterval.end,
            final_instant.value, true );
    final_instant.instant.CopyFrom( &uAlpha->timeInterval.end );

    TuAlpha result;
    CreateRegularExtensionUnit(&final_instant, iAlpha, &result );
    *res_uAlpha = result;
    return;
}

// return copy of original object with extension
template<class TmAlpha, class TiAlpha, class TuAlpha>
int ExtendWith_vm( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  TmAlpha* arg_mAlpha = static_cast<TmAlpha*>(args[0].addr);
  // Stream<TiAlpha>* arg_siAlpha = static_cast<Stream<TiAlpha>*>(args[1].addr);

  result = qp->ResultStorage(s);
  TmAlpha* res_mAlpha = static_cast<TmAlpha*>(result.addr);

  // logic from MPoint::CopyFrom(const Attribute* right)
  res_mAlpha->Clear();
  res_mAlpha->SetDefined(true);
  res_mAlpha->StartBulkLoad();
  TuAlpha unit(false);
  if (arg_mAlpha->IsDefined()) {
      for ( int i = 0; i < arg_mAlpha->GetNoComponents(); i++) {
          arg_mAlpha->Get(i, unit);
          res_mAlpha->Add(unit);
      }
  }

  Word elem(Address(0));

  qp->Open(args[1].addr);
  qp->Request(args[1].addr, elem);
  while ( qp->Received(args[1].addr) ){
      TiAlpha* arg_iAlpha = static_cast<TiAlpha*>(elem.addr);
      CreateExtensionUnit(&unit, arg_iAlpha, &unit);
      if (unit.IsDefined()) {
        res_mAlpha->Add(unit);
      }
      ((Attribute*)elem.addr)->DeleteIfAllowed();
      qp->Request(args[1].addr, elem);
  }

  res_mAlpha->EndBulkLoad(false);

  return 0;
}

ValueMapping ExtendWith_vms[] =
{
//  ExtendWith_vm<MBool, IBool>,
//  ExtendWith_vm<MString, IString>,
//  ExtendWith_vm<MInt, IInt>
//  ExtendWith_vm<MReal, IReal>,
  ExtendWith_vm<MPoint, IPoint, UPoint>
};

Operator* getExtendWithOpPtr() {
    return new Operator(
            ExtendWithInfo(),
            ExtendWith_vms,
            ExtendWith_sf,
            ExtendWith_tm
           );
}

} // end of namespace temporal2algebra


