/*
 1 RegionInterpolation2Algebra
 
 Specifies a new Secondo-algebra for interpolating two regions.
 This algebra defines a new operator ~interpolate2~, which takes two regions
 and instants and interpolates an mregion from them.
 
*/

#include "Algebra.h"
#include "MovingRegionAlgebra.h"

#include <librip.h>

static const string interpolate2spec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
" (<text>region x instant x region x instant [ x string ] -> mregion</text--->"
"<text>interpolate2( _ , _ , _ , _ [ , _ ] )</text--->"
"<text>Interpolates two regions to a moving region</text--->"
"<text>interpolate2( region1, instant1, region2, instant2, args )</text--->) )";

static ListExpr Rl2ListExpr (RList l);
static RList ListExpr2Rl (ListExpr l);

// InMRegion is defined in the MovingRegionAlgebra and converts a NestedList-
// expression to an mregion
Word InMRegion(const ListExpr typeInfo,
	       const ListExpr instance,
	       const int errorPos,
	       ListExpr& errorInfo,
	       bool& correct);
                                 


// Configure the fallbacks with their arguments here, in case the interpolation
// failed for some reason.
string fallbacks[] = {
   "mw",
   "null",
   "FALLBACK"
};
#define nrfallbacks (sizeof(fallbacks)/sizeof(fallbacks[0]))


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

/*
 1.2 interpolate2valmap is the interface to the Secondo-Algebra
 RegionInterpolation2Algebra and called for the database function interpolate2.
 The result is an mregion.
 
*/
int interpolate2valmap(Word* args,
        Word& result,
        int message,
        Word& local,
        Supplier s) {
    result = qp->ResultStorage(s);

    Instant* ti1 = static_cast<Instant*> (args[1].addr);
    Instant* ti2 = static_cast<Instant*> (args[3].addr);
    CcString* arg = static_cast<CcString*> (args[4].addr);

    Interval<Instant> iv(*ti1, *ti2, true, true);

    // We create the Face-objects from the RList-representation of the
    // regions, this interface seems most stable.
    ListExpr _sregs = OutRegion(nl->Empty(), args[0]);
    ListExpr _dregs = OutRegion(nl->Empty(), args[2]);

    RList sr = ListExpr2Rl(_sregs);
    RList dr = ListExpr2Rl(_dregs);
   
    // Create the interpolation from the lists of faces
    RList res = regioninterpolate(sr, dr, ti1->ToString(),
				  ti2->ToString(), arg->GetValue());
    bool correct = false;
    ListExpr err;
    Word w = InMRegion(nl->Empty(), Rl2ListExpr(res), 0, err, correct);
    
    for (unsigned int i = 0; !correct && (i < nrfallbacks); i++) {
        // Import failed, try the configured fallbacks...
        RList res = regioninterpolate(sr, dr, ti1->ToString(),
				      ti2->ToString(), fallbacks[i]);
        w = InMRegion(nl->Empty(), Rl2ListExpr(res), 0, err, correct);
    }
    
    if (correct)
        result.setAddr(w.addr);
    else {
       // Yield an error message here.
       ErrorReporter::ReportError("interpolate2 internal error");
       MRegion *res = new MRegion(0);
       result.setAddr(res);
    }
    
    return 0;
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





/* Functions to convert Secondo-NestedLists to librip-RLists and vice-versa */

static RList ListExpr2Rl (ListExpr le) {
   RList r;
   
   while (le != nl->Empty()) {
      ListExpr l = nl->First(le);
      if (nl->IsAtom(l)) {
	 if (nl->AtomType(l) == IntType) {
	    r.append((double)nl->IntValue(l));
	 } else if (nl->AtomType(l) == RealType) {
	    r.append((double)nl->RealValue(l));
	 } else if (nl->AtomType(l) == StringType) {
	    r.append(nl->StringValue(l));
	 } else if (nl->AtomType(l) == BoolType) {
	    r.append(nl->BoolValue(l));
	 } else {
	    r.append("UNDEF");
	 }
      } else {
	 r.append(ListExpr2Rl(l));
      }
      le = nl->Rest(le);
   }
   
   return r;
}

// Helper function, which appends a list-item to the end of a list.
static void Append(ListExpr &head, ListExpr l) {
   if (l == nl->Empty())
     return;
   if (head == nl->Empty()) {
      head = nl->OneElemList(l);
   } else {
      nl->Append(nl->End(head), l);
   }
}

static ListExpr Rl2ListExpr (RList l) {
   ListExpr r = nl->Empty();
   
   for (unsigned int i = 0; i < l.items.size(); i++) {
      if (l.items[i].getType() == NL_DOUBLE) {
	 Append(r, nl->RealAtom(l.items[i].getNr()));
      } else if (l.items[i].getType() == NL_STRING) {
	 Append(r, nl->StringAtom(l.items[i].getString()));
      } else if (l.items[i].getType() == NL_BOOL) {
	 Append(r, nl->BoolAtom(l.items[i].getBool()));
      } else {
	 Append(r, Rl2ListExpr(l.items[i]));
      }
   }
   
   return r;
}
