/*
 1 RegionInterpolation2Algebra
 
 Specifies a new Secondo-algebra for interpolating two regions.
 This algebra defines a new operator ~interpolate2~, which takes two regions
 and instants, and creates an interpolated mregion.
 
*/

#include "Algebra.h"
#include "MovingRegionAlgebra.h"

static const string interpolate2spec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
" (<text>region x instant x region x instant [ x string ] -> mregion</text--->"
"<text>interpolate2( _ , _ , _ , _ [ , _ ] )</text--->"
"<text>Interpolate2 two regions to an interval region</text--->"
"<text>interpolate2( region1, instant1, region2, instant2, args )</text--->) )";

int interpolate2valmap(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s);

/*
 1.1 interpolate2typemap checks, if the number and type of arguments is
 correct. The function takes exactly two region-objects and two instants and
 an optional parameter-string.
 
*/
ListExpr interpolate2typemap(ListExpr args) {
    string err = "region x instant x region x instant [x string] expected";
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
    if (len == 4) { // By default, the parameter string is empty
        return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                nl->OneElemList(nl->StringAtom("", true)),
                nl->SymbolAtom(MRegion::BasicType()));
    }
    if (!CcString::checkType(nl->Fifth(args))) {
        return listutils::typeError(err);
    }
    return nl->SymbolAtom(MRegion::BasicType());
}

Operator interpolate2("interpolate2",
        interpolate2spec,
        interpolate2valmap,
        Operator::SimpleSelect,
        interpolate2typemap);

class RegionInterpolation2Algebra : public Algebra {
public:
  RegionInterpolation2Algebra() : Algebra() {
    AddOperator(&interpolate2);
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
