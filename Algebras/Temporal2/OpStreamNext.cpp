/*
implementation of operator streamnext

*/

#include "OpStreamNext.h"

#include "Operator.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "ListUtils.h"
#include "Algebras/FText/FTextAlgebra.h"


#include "StreamValve.h"


extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace temporal2algebra{

struct StreamNextInfo : OperatorInfo {
    StreamNextInfo() : OperatorInfo() {
        name =      "streamnext";
        signature = "text x int -> int";
        syntax =    "streamnext(<MyValveId>, <count>);";
        meaning =   "Instructs the RemoteStreamValve with Id <MyValveId>"
                " to let the next <count> elements pass.\n"
                "Returns the number of elements that actually passed\n"
                "or -1 if there is no StreamValve with Id <MyValveId>";
    }
};

ListExpr StreamNext_tm( ListExpr args ) {
    if (!nl->HasLength(args,2)) {
        return listutils::typeError("expected 2 arguments.");
    }

    if (!FText::checkType(nl->First(args))) {
        return listutils::typeError("expected "
                + FText::BasicType()
        + " as first argument, but got "
        + nl->ToString(nl->First(args)));
    }

    if (!CcInt::checkType(nl->Second(args))) {
        return listutils::typeError("expected "
                + CcInt::BasicType()
        + " as second argument, but got "
        + nl->ToString(nl->Second(args)));
    }

    return NList(CcInt::BasicType()).listExpr();
}


int StreamNext_sf( ListExpr args ) {
    return 0;
}

int StreamNext_vm( Word* args, Word& result, int message,
        Word& local, Supplier s )
{
    std::string valveId = (static_cast<FText*>(args[0].addr))->GetValue();
    int advanceCount = (static_cast<CcInt*>(args[1].addr))->GetValue();

    result = qp->ResultStorage(s);
    CcInt* resAdvancedActual = static_cast<CcInt*>(result.addr);

    std::cout << "StreamNext_vm got: "
            << valveId << ", " << advanceCount
            << std::endl;

    int num_read = RemoteStreamValve::advance(valveId, advanceCount);

    resAdvancedActual->Set(true, num_read);
    return 0;
}

ValueMapping StreamNext_vms[] =
{
        StreamNext_vm
};

Operator* getStreamNextOpPtr() {
    return new Operator(
            StreamNextInfo(),
            StreamNext_vms,
            StreamNext_sf,
            StreamNext_tm
    );
}

} // end of namespace temporal2algebra


