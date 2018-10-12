/*
implementation of operator streamvalve

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
                "expected at least 1 argument (incoming stream), but got none");
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
        // undocumented: let everything pass if we get no valve-name
        //    required for selftest
        cout << "No StreamValve specified, using default: "
                << OpenValveId
                << "StreamValve\n";

        return nl->ThreeElemList (
                nl->SymbolAtom ( Symbols::APPEND ()) ,
                nl->TwoElemList (
                        nl->TextAtom("dummy"),
                        nl->TextAtom(OpenValveId)) ,
                        streamType);
    }

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

    return nl->ThreeElemList (
            nl->SymbolAtom ( Symbols::APPEND ()) ,
            nl->OneElemList (nl->TextAtom(RemoteValveId)) ,
            streamType);
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
        FText* valveId = static_cast<FText*>(args[2].addr);
        const std::string valve_id = valveId->GetValue();
        StreamValvePtr valvePtr;
        if (valve_id == OpenValveId) {
            valvePtr = StreamValvePtr(new OpenStreamValve() );
        } else if (valve_id == RemoteValveId) {
            FText* remote_valve = static_cast<FText*>(args[1].addr);
            valvePtr = StreamValvePtr (
                    RemoteStreamValve::create(remote_valve->GetValue()) );
        } else {
            //shouldn't happen: we set valveId in TypeMapping
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


