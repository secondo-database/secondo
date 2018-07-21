/*
OpAppendTo.cpp
Created on: 08.04.2018
Author: simon

Limitations and Todos:
- only naive implementation
    (optimization for fast concurrent updates is topic of bachelor thesis)
- only MPoint implemented so far

*/

#include "OpAppendTo.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "MPoint2.h"
#include "MovingCalculations.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace temporalalgebra;

namespace temporal2algebra{

struct AppendToInfo : OperatorInfo {
  AppendToInfo() : OperatorInfo() {
      name =      "appendto";
      signature = "stream(i(alpha)) x m(alpha) -> m(alpha), alpha in {point}"
              "i(alpha) x m(alpha) -> m(alpha), alpha in {point}"
              "- {bool, string, int, real} not implemented yet";
      syntax =    "<stream(IPoint)> appendto <MPoint>"
              "<IPoint> appendto <MPoint>";
      meaning =   "Returns the MPoint extended by the (Stream of) IPoints";
  }
};

ListExpr AppendTo_tm( ListExpr args ) {
    if (!nl->HasLength(args,2)) {
        return listutils::typeError("expected 2 arguments, but got "
                + nl->ToString(args));
    }

    if (!Stream<IPoint>::checkType(nl->First(args))
            && !IPoint::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + Stream<IPoint>::BasicType()
                + "(" + IPoint::BasicType()
                + ") or " + IPoint::BasicType()
                + " as first argument, but got "
                + nl->ToString(nl->First(args))
                );
    }

    if (MPoint::checkType(nl->Second(args))) {
        return NList(MPoint::BasicType()).listExpr();
    }

    if (MPoint2::checkType(nl->Second(args))) {
        return NList(MPoint2::BasicType()).listExpr();
    }

    return listutils::typeError("expected "
        + MPoint::BasicType() + " or "
        + MPoint2::BasicType() + " as second argument, but got "
        + nl->ToString(nl->Second(args)));

}


int AppendTo_sf( ListExpr args ) {
    bool isMemory;
    if (MPoint::checkType(nl->Second(args))) {
        isMemory = false;
        cout << "AppendTo_sf(..) MPoint: memory = false\n";
    } else if (MPoint2::checkType(nl->Second(args))){
        isMemory = true;
        cout << "AppendTo_sf(..) MPoint2: memory = true\n";
    } else {
        assert(false);
    }

    bool isStream;
    if (IPoint::checkType(nl->First(args))) {
        isStream = false;
        cout << "AppendTo_sf(..) IPoint: stream = false\n";
    } else if (Stream<IPoint>::checkType(nl->First(args))) {
        isStream = true;
        cout << "AppendTo_sf(..) stream(IPoint): stream = true\n";
    } else {
        assert(false);
    }

    int selection = 0;
    selection += isMemory?1:0;
    selection += isStream?2:0;

    cout << "AppendTo_sf(..) selection = "<< selection << "\n";
    return selection;
}

// return copy of original object with extension
template<class TmAlpha, class TiAlpha, class TuAlpha>
int AppendToStreamRegular_vm
    ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  cout << "int AppendToStreamRegular_vm\n";
  TmAlpha* arg_mAlpha = static_cast<TmAlpha*>(args[1].addr);
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

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  while ( qp->Received(args[0].addr) ){
      TiAlpha* arg_iAlpha = static_cast<TiAlpha*>(elem.addr);
      CreateExtensionUnit(&unit, arg_iAlpha, &unit);
      if (unit.IsDefined()) {
        res_mAlpha->Add(unit);
      }
      ((Attribute*)elem.addr)->DeleteIfAllowed();
      qp->Request(args[0].addr, elem);
  }

  res_mAlpha->EndBulkLoad(false);

  return 0;
}

// return copy of original object with extension
template<class TmAlpha, class TiAlpha, class TuAlpha>
int AppendToStreamMemory_vm
    ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  cout << "int AppendToStreamMemory_vm\n";
  TmAlpha* arg_mAlpha = static_cast<TmAlpha*>(args[1].addr);
  // Stream<TiAlpha>* arg_siAlpha = static_cast<Stream<TiAlpha>*>(args[1].addr);

  result = qp->ResultStorage(s);
  TmAlpha* res_mAlpha = static_cast<TmAlpha*>(result.addr);
  res_mAlpha->CopyFrom(arg_mAlpha);
  Word elem(Address(0));

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);

  TuAlpha unit;
  if (res_mAlpha->get().empty()) {
      unit.SetDefined(false);
  } else {
      unit = res_mAlpha->get().back();
  }

  while ( qp->Received(args[0].addr) ){
      TiAlpha* arg_iAlpha = static_cast<TiAlpha*>(elem.addr);
      CreateExtensionUnit(&unit, arg_iAlpha, &unit);
      if (unit.IsDefined()) {
        res_mAlpha->append(unit);
      }
      ((Attribute*)elem.addr)->DeleteIfAllowed();
      qp->Request(args[0].addr, elem);
  }
  return 0;
}



// return copy of original object with extension
template<class TmAlpha, class TiAlpha, class TuAlpha>
int AppendToUnitRegular_vm
    ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  cout << "int AppendToUnitRegular_vm\n";
  TmAlpha* arg_mAlpha = static_cast<TmAlpha*>(args[1].addr);
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

  TiAlpha* arg_iAlpha = static_cast<TiAlpha*>(args[0].addr);
  CreateExtensionUnit(&unit, arg_iAlpha, &unit);
  if (unit.IsDefined()) {
     res_mAlpha->Add(unit);
  }

  res_mAlpha->EndBulkLoad(false);

  return 0;
}

//// return copy of original object with extension
//template<class TmAlpha, class TiAlpha, class TuAlpha>
//int AppendToUnitMemory_vm
//    ( Word* args, Word& result, int message, Word& local, Supplier s )
//{
//  cout << "int AppendToUnitMemory_vm\n";
//  TmAlpha* arg_mAlpha = static_cast<TmAlpha*>(args[1].addr);
//  // Stream<TiAlpha>* arg_siAlpha =
// static_cast<Stream<TiAlpha>*>(args[1].addr);
//
//  result = qp->ResultStorage(s);
//  TmAlpha* res_mAlpha = static_cast<TmAlpha*>(result.addr);
//  res_mAlpha->CopyFrom(arg_mAlpha);
//
//  TuAlpha unit;
//  if (res_mAlpha->get().empty()) {
//      unit.SetDefined(false);
//  } else {
//      unit = res_mAlpha->get().back();
//  }
//
//  TiAlpha* arg_iAlpha = static_cast<TiAlpha*>(args[0].addr);
//  CreateExtensionUnit(&unit, arg_iAlpha, &unit);
//  if (unit.IsDefined()) {
//     res_mAlpha->append(unit);
//  }
//
//  return 0;
//}

ValueMapping AppendTo_vms[] =
{
    AppendToUnitRegular_vm<MPoint, IPoint, UPoint>,
    AppendToUnitRegular_vm<MPoint2, IPoint, UPoint>,
    AppendToStreamRegular_vm<MPoint, IPoint, UPoint>,
    AppendToStreamRegular_vm<MPoint2, IPoint, UPoint>

  //  AppendTo_vm<MBool, IBool>,
  //  AppendTo_vm<MString, IString>,
  //  AppendTo_vm<MInt, IInt>
  //  AppendTo_vm<MReal, IReal>,
};

Operator* getAppendToOpPtr() {
    return new Operator(
            AppendToInfo(),
            AppendTo_vms,
            AppendTo_sf,
            AppendTo_tm
           );
}

} // end of namespace temporal2algebra


