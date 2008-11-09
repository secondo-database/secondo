/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/


#include "Algebra.h"
#include "NestedList.h"
#include "NList.h" 
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h" 
#include "StandardTypes.h"
#include <string>
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "MovingRegionAlgebra.h"
#include "SetOps.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace symbols;
using namespace mappings;
using namespace std;

namespace mregionops {

void Test() {
    
    Instant t1(instanttype);
    Instant t12(instanttype);
    Instant t2(instanttype);
    
    t1.ReadFrom(0.0);
    t12.ReadFrom(5000.0);
    t2.ReadFrom(10000.0);
    
    cout << "t1: " << t1 << endl;
    cout << "t12: " << t12 << endl;
    cout << "t2: " << t2 << endl;
    
    cout << (t12 - t1) / (t2 - t1) << endl;
    cout << (t12 - t1).ToDouble() / (t2 - t1).ToDouble() << endl;
}

/*

 Type Mapping Functions

*/

ListExpr MRMRMRTypeMap(ListExpr args) {

    NList type(args);
    const string errMsg = "Expecting two movingregions.";

    if (type.length() != 2)
        return NList::typeError(errMsg);

    // movingregion x movingregion -> movingregion
    if (type.first().isSymbol("movingregion") && 
        type.second().isSymbol("movingregion")) {

        return NList("movingregion").listExpr();
    }

    return NList::typeError(errMsg);
}

ListExpr MRMRMBTypeMap(ListExpr args) {

    NList type(args);
    const string errMsg = "Expecting two movingregions.";

    if (type.length() != 2)
        return NList::typeError(errMsg);

    // movingregion x movingregion -> mbool
    if (type.first().isSymbol("movingregion") && 
        type.second().isSymbol("movingregion")) {

        return NList("mbool").listExpr();
    }

    return NList::typeError(errMsg);
}

/*

 Value Mapping Functions

*/

int IntersectionValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion* mrA = static_cast<MRegion*>(args[0].addr );
    MRegion* mrB = static_cast<MRegion*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion* res = static_cast<MRegion*>(result.addr );

    SetOperator so(mrA, mrB, res);
    so.Intersection();

    return 0;
}

int UnionValueMap(Word* args, Word& result, int message, Word& local, 
        Supplier s) {

    MRegion* mrA = static_cast<MRegion*>(args[0].addr );
    MRegion* mrB = static_cast<MRegion*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion* res = static_cast<MRegion*>(result.addr );

    SetOperator so(mrA, mrB, res);
    so.Union();

    return 0;
}

int MinusValueMap(Word* args, Word& result, int message, Word& local, 
        Supplier s) {

    MRegion* mrA = static_cast<MRegion*>(args[0].addr );
    MRegion* mrB = static_cast<MRegion*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion* res = static_cast<MRegion*>(result.addr );

    SetOperator so(mrA, mrB, res);
    so.Minus();

    return 0;
}

int IntersectsValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion* mrA = static_cast<MRegion*>(args[0].addr );
    MRegion* mrB = static_cast<MRegion*>(args[1].addr );

    result = qp->ResultStorage(s);

    MBool* res = static_cast<MBool*>(result.addr );

    res->SetDefined(false);

    return 0;
}

int InsideValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion* mrA = static_cast<MRegion*>(args[0].addr );
    MRegion* mrB = static_cast<MRegion*>(args[1].addr );

    result = qp->ResultStorage(s);

    MBool* res = static_cast<MBool*>(result.addr );

    res->SetDefined(false);
    
    //Test();

    return 0;
}



/*

 Operator Descriptions

*/

struct IntersectionInfo : OperatorInfo {

    IntersectionInfo() {
        name = "intersection";
        signature = "movingregion x movingregion -> movingregion";
        syntax = "intersection(_, _)";
        meaning = "Intersection operation for two movingregions.";
    }
};

struct UnionInfo : OperatorInfo {

    UnionInfo() {
        name = "union";
        signature = "movingregion x movingregion -> movingregion";
        syntax = "_ union _";
        meaning = "Union operation for two movingregions.";
    }
};

struct MinusInfo : OperatorInfo {

    MinusInfo() {
        name = "minus";
        signature = "movingregion x movingregion -> movingregion";
        syntax = "_ minus _";
        meaning = "Minus operation for two movingregions.";
    }
};

struct IntersectsInfo : OperatorInfo {

    IntersectsInfo() {
        name = "intersects";
        signature = "movingregion x movingregion -> mbool";
        syntax = "_ intersects _";
        meaning = "Intersects predicate for two movingregions.";
    }
};

struct InsideInfo : OperatorInfo {

    InsideInfo() {
        name = "inside";
        signature = "movingregion x movingregion -> mbool";
        syntax = "_ inside _";
        meaning = "Inside predicate for two movingregions.";
    }
};

/*

 Implementation of the Algebra Class

*/

class MRegionOpsAlgebra : public Algebra {

public:

    MRegionOpsAlgebra() :
        Algebra() {

        AddOperator(IntersectionInfo(), IntersectionValueMap, MRMRMRTypeMap);
        AddOperator(UnionInfo(), UnionValueMap, MRMRMRTypeMap);
        AddOperator(MinusInfo(), MinusValueMap, MRMRMRTypeMap);
        //AddOperator(IntersectsInfo(), IntersectsValueMap, MRMRMBTypeMap);
        //AddOperator(InsideInfo(), InsideValueMap, MRMRMBTypeMap);
    }

    ~MRegionOpsAlgebra() {

    }
};

} // end of namespace mregionops

extern "C"Algebra*
InitializeMRegionOpsAlgebra( NestedList* nlRef,
        QueryProcessor* qpRef )
{
    // The C++ scope-operator :: must be used to qualify the full name 
    return new mregionops::MRegionOpsAlgebra();
}

