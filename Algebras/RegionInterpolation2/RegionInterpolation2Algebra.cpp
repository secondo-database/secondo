/*
*/
#include "Algebra.h"        //always needed in Algebras
#include "MovingRegionAlgebra.h"

static const string interpolatespec =
 "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
 "  ( <text>region x instant x region x instant [ x real ] -> mregion</text--->"
 "<text>interpolate( _ , _ , _ , _ [ , _ ] )</text--->"
 "<text>Interpolate two regions to an interval region</text--->"
 "<text>interpolate( region1, instant1, region2, instant2, mode )</text--->) )";

int interpolatevalmap(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s);

ListExpr interpolatetypemap(ListExpr args) {
    string err = "region x instant x region x instant [x real] expected";
    int len = nl->ListLength(args);
    if ((len != 4) && (len != 5)) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!Region::checkType(nl->First(args))) {
        return listutils::typeError(err);
    }
    if (!Instant::checkType(nl->Second(args))) {
        return listutils::typeError(err);
    }
    if (!Region::checkType(nl->Third(args))) {
        return listutils::typeError(err);
    }
    if (!Instant::checkType(nl->Fourth(args))) {
        return listutils::typeError(err);
    }
    if (len == 4) {
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->IntAtom(0)),
                nl->SymbolAtom(MRegion::BasicType()));
    }
    if (!CcReal::checkType(nl->Fifth(args))) {
        return listutils::typeError(err);
    }
    return nl->SymbolAtom(MRegion::BasicType());
}

Operator interpolate("interpolate",
        interpolatespec,
        interpolatevalmap,
        Operator::SimpleSelect,
        interpolatetypemap);

class RegionInterpolation2Algebra : public Algebra {
public:
  RegionInterpolation2Algebra() : Algebra() {
    AddOperator(&interpolate);
  }
    ~RegionInterpolation2Algebra() {}
};


extern "C"
Algebra* InitializeRegionInterpolation2Algebra(NestedList* nlRef,
                                       QueryProcessor *qpRef) {
    nl = nlRef;
    qp = qpRef;
    return new RegionInterpolation2Algebra();
}