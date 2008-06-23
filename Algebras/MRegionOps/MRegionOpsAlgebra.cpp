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

extern NestedList* nl;
extern QueryProcessor *qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

#include "MovingRegionAlgebra.h"
#include "SetOpUtil.h"

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;

namespace mregionops {


void Intersection(MRegion& mrA, MRegion& mrB, MRegion& mrResult) {
	
	// Precondition (for testing):
	// mrA and mrB contains just one unit
	// over the same interval.
	
	const DBArray<MSegmentData>* aArray = mrA.GetMSegmentData();
	const DBArray<MSegmentData>* bArray = mrB.GetMSegmentData();
	const URegionEmb* a;
	const URegionEmb* b;
	
	mrA.Get(0, a);
	mrB.Get(0, b);
	
	SetOperator so = SetOperator(a, aArray, b, bArray);
	so.Intersection();
	
	/*
	//Compute the refinement partition:
	RefinementPartition<
	        MRegion,
	        MRegion,
	        URegionEmb,
	        URegionEmb> rp(*this, mr);
	
	// Get the number of region units from the refinement partition:
	const unsigned int nOfUnits = rp.Size();
	
	const URegionEmb* aa;
	const URegionEmb* bb;
	const URegionEmb* a;
	const URegionEmb* b;
	PUnitPair* unitPair;
	vector<URegion>* resultUnits;
	Interval<Instant>* interval;
	int aPos, bPos;
	
	const DBArray<MSegmentData>* aArray = this->GetMSegmentData();
	const DBArray<MSegmentData>* bArray = mr.GetMSegmentData();

	for (unsigned int i = 0; i < nOfUnits; i++) {
		
		// Get the interval of region unit i 
		// from the refinement partition:
		rp.Get(i, interval, aPos, bPos);
		
		// Skip this region unit, 
		// if one of the movingregions is undefined:
		if (aPos == -1 || bPos == -1)
			continue;
		
		this->Get(aPos, aa);
		mr.Get(bPos, bb);
		
		//aa->Restrict(interval, a);
		//bb->Restrict(interval, b);
		a = aa;
		b = bb;
		
		unitPair = new PUnitPair(a, aArray, b, bArray);
		resultUnits = unitPair->Intersection();
		
		vector<URegion>::iterator it;
		for (it = resultUnits->begin(); it != resultUnits->end(); it++)
			res.AddURegion(*it);
		
		delete unitPair;
		
	}
	*/
}

void Union(MRegion& mrA, MRegion& mrB, MRegion& mrResult) {
	
	// Precondition (for testing):
	// mrA and mrB contains just one unit
	// over the same interval.
	
	const DBArray<MSegmentData>* aArray = mrA.GetMSegmentData();
	const DBArray<MSegmentData>* bArray = mrB.GetMSegmentData();
	const URegionEmb* a;
	const URegionEmb* b;
	
	mrA.Get(0, a);
	mrB.Get(0, b);
	
	SetOperator so = SetOperator(a, aArray, b, bArray);
	so.Union();
}

void Minus(MRegion& mrA, MRegion& mrB, MRegion& mrResult) {
	
	// Precondition (for testing):
	// mrA and mrB contains just one unit
	// over the same interval.
	
	const DBArray<MSegmentData>* aArray = mrA.GetMSegmentData();
	const DBArray<MSegmentData>* bArray = mrB.GetMSegmentData();
	const URegionEmb* a;
	const URegionEmb* b;
	
	mrA.Get(0, a);
	mrB.Get(0, b);
	
	SetOperator so = SetOperator(a, aArray, b, bArray);
	so.Minus();
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


/*

 Value Mapping Functions

*/

int IntersectionValueMap(Word* args, Word& result, int message, 
		Word& local, Supplier s) {
	
	MRegion* mrA = static_cast<MRegion*>(args[0].addr );
	MRegion* mrB = static_cast<MRegion*>(args[1].addr );

	result = qp->ResultStorage(s);

	MRegion* res = static_cast<MRegion*>(result.addr );

	MRegion dummy(0);
	Intersection(*mrA, *mrB, dummy);

	res->SetDefined(false);

	return 0;
}

int UnionValueMap(Word* args, Word& result, int message, Word& local,
		Supplier s) {
	MRegion* mrA = static_cast<MRegion*>(args[0].addr );
	MRegion* mrB = static_cast<MRegion*>(args[1].addr );

	result = qp->ResultStorage(s);

	MRegion* res = static_cast<MRegion*>(result.addr );

	MRegion dummy(0);
	Union(*mrA, *mrB, dummy);

	res->SetDefined(false);

	return 0;
}

int MinusValueMap(Word* args, Word& result, int message, Word& local,
		Supplier s) {
	MRegion* mrA = static_cast<MRegion*>(args[0].addr );
	MRegion* mrB = static_cast<MRegion*>(args[1].addr );

	result = qp->ResultStorage(s);

	MRegion* res = static_cast<MRegion*>(result.addr );

	MRegion dummy(0);
	Minus(*mrA, *mrB, dummy);

	res->SetDefined(false);

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


/*

 Implementation of the Algebra Class

*/

class MRegionOpsAlgebra : public Algebra {
	
public:
	
	MRegionOpsAlgebra() :
		Algebra() {

		AddOperator(IntersectionInfo(), IntersectionValueMap, 
					MRMRMRTypeMap);
		AddOperator(UnionInfo(), UnionValueMap, MRMRMRTypeMap);
		AddOperator(MinusInfo(), MinusValueMap, MRMRMRTypeMap);
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
	return new mregionops::MRegionOpsAlgebra;
}

