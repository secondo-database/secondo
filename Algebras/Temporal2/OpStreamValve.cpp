/*
OpStreamValve.cpp
Created on: 08.04.2018
Author: simon

Limitations and ToDos:
- Handle failures for RemoteStreamValve in a sensible way
    (Default to NoneStremValve? "throw" secondo error?)
- Refactoring: Move TypeMapping Checks and ValueMapping to StreamValve
- Refactoring: Inroduce Factory for StreamValves

*/

#include "OpStreamValve.h"
#include "StreamValve.h"
#include <tr1/memory>

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Stream/Stream.h"
#include "Algebras/FText/FTextAlgebra.h"

#include "TypeMapUtils.h"
#include "Symbols.h"
#include "ListUtils.h"

extern NestedList* nl;
extern QueryProcessor* qp;


using namespace std;
using std::tr1::shared_ptr;
namespace temporal2algebra{

struct StreamValveInfo : OperatorInfo {
  StreamValveInfo() : OperatorInfo() {
    name =      "streamvalve";
    signature = "stream(alpha) x string -> stream(alpha)";
    syntax =    "<incoming_stream> streamvalve [<MyValveId>]";
    meaning =   "Creates a remotely controlled barrier with Id <MyValveId>"
                " for a stream of any type.\n"
                "Upon calls to streamnext operator elements of the incoming "
                "stream will be copied to the outgoing stream.\n"
                "Returns a stream of the same type as the incoming stream.";
  }
};

ListExpr StreamValve_tm( ListExpr args ) {
    cout << "StreamValve_tm(" << nl->ToString(args) << ")\n";
    ListExpr rest = args;
    // will never match - operator spec requires at least on prefix-arg
    if (nl->IsEmpty(rest)) {
        return listutils::typeError(
                "expected at 1 argument (incoming stream), but got none");
    }

    ListExpr current = nl->First(rest);
    rest = nl->Rest(rest);

    ListExpr streamType = nl->First(current);

    if (!listutils::isStream(streamType)) {
        return listutils::typeError(
                "expected stream as first argument (incoming stream), but got "
                + nl->ToString(streamType));
    }

    if (nl->IsEmpty(rest)) {
        cout << "No StreamValve specified, using default: "
                << OpenValveId
                << "StreamValve\n";

        return nl->ThreeElemList (
                nl->SymbolAtom ( Symbols::APPEND ()) ,
                nl->OneElemList ( nl->TextAtom(OpenValveId)) ,
                streamType);
    }

    current = nl->First(rest);
    rest = nl->Rest(rest);

    ListExpr valveNameType = nl->First(current);
    if (!FText::checkType(valveNameType)) {
            return listutils::typeError("expected " + FText::BasicType()
            + " as second argument (make of StreamValve), but got "
            + nl->ToString(valveNameType));
        }

    ListExpr valveName = nl->Second(current);
    if (nl->AtomType(valveName) != TextType) {
        return listutils::typeError(
                "expected TextType contents as in second Argument, but got "
                + nl->ToString(valveName));
    }
    std::string valve_id = nl->Text2String(valveName);

    if ( valve_id == OpenValveId) {
        if (!nl->IsEmpty(rest)) {
            return listutils::typeError(
                    "OpenValve does not take additional parameters, but got "
                    + nl->ToString(rest));
        }
        return streamType;
    }

    if (valve_id == RandomValveId) {
        if (nl->IsEmpty(rest)) { // No args provided, use default
            cout << "No args provided for RandomValve, will use default\n";
            return nl->ThreeElemList (
                            nl->SymbolAtom ( Symbols::APPEND ()) ,
                            nl->TwoElemList ( nl->IntAtom(1),
                                    nl->IntAtom(3)) ,
                            streamType);
        }
        if (!nl->HasLength(rest, 2)) {
            return listutils::typeError(
                    "RandomValve expects 2 int parameters, but got "
                    + nl->ToString(rest));
        }

        ListExpr min_value_type = nl->First(nl->First(rest));
        if (!CcInt::checkType(min_value_type)) {
            return listutils::typeError(
                    "RandomValve expects int type as first argument"
                    "(min_value), but got  " + nl->ToString(min_value_type));
        }

        ListExpr max_value_type = nl->First(nl->Second(rest));
        if (!CcInt::checkType(max_value_type)) {
            return listutils::typeError(
                    "RandomValve expects int type as second argument"
                    " (max_value), but got " + nl->ToString(max_value_type));
        }

        return streamType;
    }

    if (valve_id == RemoteValveId) {
        if (!nl->HasLength(rest, 1)) {
            return listutils::typeError(
                    "RemoteValve expects 1 text parameters, but got "
                    + nl->ToString(rest));
        }
        ListExpr remoteValveId = nl->First(nl->First(rest));
        if (!FText::checkType(remoteValveId)) {
                return listutils::typeError("RemoteValve expected "
                        + FText::BasicType()
                        + " as argument (Id for Valve), but got "
                        + nl->ToString(remoteValveId));
            }
        return streamType;
    }

    return listutils::typeError("Unknown StreamValve make " + valve_id);

}


int StreamValve_sf( ListExpr args ) {
    cout << "StreamValve_sf(" << nl->ToString(args) << ")\n";
  return 0;
}

int StreamValve_vm( Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s ) {

      struct StreamValve_LocalInfo{
        StreamValve_LocalInfo(StreamValvePtr valve): pValve(valve) {
        };
        StreamValvePtr pValve;
      };

      Word elem;
      StreamValve_LocalInfo *localInfo =
              static_cast<StreamValve_LocalInfo*>(local.addr);

      switch( message ){
        case OPEN:{
          FText* valveId = static_cast<FText*>(args[1].addr);
          const std::string valve_id = valveId->GetValue();
          StreamValvePtr valvePtr;
          if (valve_id == OpenValveId) {
              valvePtr = StreamValvePtr(new OpenStreamValve() );
          } else if (valve_id == RandomValveId) {
              CcInt* min_wait = static_cast<CcInt*>(args[2].addr);
              CcInt* max_wait = static_cast<CcInt*>(args[3].addr);
              valvePtr = StreamValvePtr (
                 new RandomStreamValve(min_wait->GetValue(),
                              max_wait->GetValue()) );
          } else if (valve_id == RemoteValveId) {
              FText* remote_valve = static_cast<FText*>(args[2].addr);
              valvePtr = StreamValvePtr (
                 RemoteStreamValve::create(remote_valve->GetValue()) );
          } else {
              cout << "Unknown valveId: " << valve_id << endl;
              assert(false);
          }

          localInfo = new StreamValve_LocalInfo(valvePtr);

          local.setAddr(localInfo);
          qp->Open(args[0].addr);
          return 0;
        }
        case REQUEST:{
          if(!localInfo){ return CANCEL; }
          localInfo->pValve->waitForSignal();
          qp->Request(args[0].addr, elem);
          while ( qp->Received(args[0].addr) ) {
            result = elem;
            localInfo->pValve->sendHasReadSucceeded(true);
            return YIELD;
          }
          localInfo->pValve->sendHasReadSucceeded(false);
          return CANCEL;
        }
        case CLOSE:{
          qp->Close(args[0].addr);
          if(localInfo){
            delete localInfo;
            local.setAddr(0);
          }
          return 0;
        }
      } // switch
      assert (false); // should not happen: unhandled message type
      return -1;
}


ValueMapping StreamValve_vms[] =
{
  StreamValve_vm
};

Operator* getStreamValveOpPtr() {
    Operator* op =  new Operator(
            StreamValveInfo(),
            StreamValve_vms,
            StreamValve_sf,
            StreamValve_tm
           );
    op->SetUsesArgsInTypeMapping();
    return op;
}

} // end of namespace temporal2algebra


