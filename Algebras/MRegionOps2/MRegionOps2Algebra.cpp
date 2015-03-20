/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOps2Algebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

the additional Prädikat Inside and Intersect,
because movingregion2 [x] movingregion2 [->] bool not working,
it's a open problem to solve

[TOC]

1 Introduction

This file essentially contains the implementation of the algebra class and
the type- and value mapping functions of the three set operators
~intersection~, ~union~ and ~minus~ with the signature \\
movingregion2 [x] movingregion2 [->] movingregion2 \\
used in the MovingRegion Algebra.


2 Defines and Includes

*/

#include <string>

#include "Algebra.h"
#include "NestedList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "NList.h"
#include "MovingRegion2Algebra.h"
#include "SetOperator2.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace mappings;
using namespace std;

namespace mregionops2 {

/*

1 Type Mapping Functions

*/

ListExpr MRMRMRTypeMap(ListExpr args) {

    NList type(args);
    const string errMsg = "Expecting two movingregions.";

    if (type.length() != 2)
        return NList::typeError(errMsg);

    // movingregion2 x movingregion2 -> movingregion2
    if (type.first().isSymbol(MRegion2::BasicType()) &&
        type.second().isSymbol(MRegion2::BasicType())) {

        return NList(MRegion2::BasicType()).listExpr();
    }

    return NList::typeError(errMsg);
}

/*

1 Value Mapping Functions

*/


int IntersectionValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {


    MRegion2* mrA = static_cast<MRegion2*>(args[0].addr );
    MRegion2* mrB = static_cast<MRegion2*>(args[1].addr );

    result = qp->ResultStorage(s);
    MRegion2* res = static_cast<MRegion2*>(result.addr );

    SetOperator2 so(mrA, mrB, res);
    so.Intersection();

    return 0;

}

int UnionValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {


    MRegion2* mrA = static_cast<MRegion2*>(args[0].addr );
    MRegion2* mrB = static_cast<MRegion2*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion2* res = static_cast<MRegion2*>(result.addr );

    SetOperator2 so(mrA, mrB, res);
    so.Union();

    return 0;

}

int MinusValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion2* mrA = static_cast<MRegion2*>(args[0].addr );
    MRegion2* mrB = static_cast<MRegion2*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion2* res = static_cast<MRegion2*>(result.addr );

    SetOperator2 so(mrA, mrB, res);
    so.Minus();

   return 0;
}

int InsideValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion2* mrA = static_cast<MRegion2*>(args[0].addr );
    MRegion2* mrB = static_cast<MRegion2*>(args[1].addr );

    result = qp->ResultStorage(s);
    MRegion2* res = static_cast<MRegion2*>(result.addr );

    SetOperator2 so(mrA, mrB, res);
    so.Inside();
//  not working
//    bool* bRes = static_cast<bool*>(result.addr );
//  bRes = so.bRes;
//
   return 0;
}

int IntersectValueMap(Word* args, Word& result, int message, Word& local,
        Supplier s) {

    MRegion2* mrA = static_cast<MRegion2*>(args[0].addr );
    MRegion2* mrB = static_cast<MRegion2*>(args[1].addr );

    result = qp->ResultStorage(s);

    MRegion2* res = static_cast<MRegion2*>(result.addr );


    SetOperator2 so(mrA, mrB, res);
    so.Intersect();
//  not working
//    bool* bRes = static_cast<bool*>(result.addr );
//    bRes = so.bRes;
//
   return 0;
}

/*

1 Operator Descriptions

*/

struct IntersectionInfo : OperatorInfo {

    IntersectionInfo() {
        name = "intersection";
        signature = "mregion2 x mregion2 -> mregion2";
        syntax = "intersection(_, _)";
        meaning = "Intersection operation for two moving regions.";
    }
};

struct UnionInfo : OperatorInfo {

    UnionInfo() {
        name = "union";
        signature = "mregion2 x mregion2 -> mregion2";
        syntax = "_ union _";
        meaning = "Union operation for two moving regions.";
    }
};

struct MinusInfo : OperatorInfo {

    MinusInfo() {
        name = "minus";
        signature = "mregion2 x mregion2 -> mregion2";
        syntax = "_ minus _";
        meaning = "Minus operation for two moving regions.";
    }
};

struct InsideInfo : OperatorInfo {

    InsideInfo() {
        name = "inside";
        signature = "mregion2 x mregion2 -> " + CcBool::BasicType();
        syntax = "_ inside _";
        meaning = "inside operation for two moving regions.";
    }
};

struct IntersectInfo : OperatorInfo {

    IntersectInfo() {
        name = "intersect";
        signature = "mregion2 x mregion2 -> " + CcBool::BasicType();
        syntax = "_ intersect _";
        meaning = "Intersect operation for two moving regions.";
    }
};
/*

1 Implementation of the Algebra Class

*/

class MRegionOps2Algebra : public Algebra {

public:

    MRegionOps2Algebra() :
        Algebra() {

        AddOperator(IntersectionInfo(), IntersectionValueMap, MRMRMRTypeMap);
        AddOperator(UnionInfo(), UnionValueMap, MRMRMRTypeMap);
        AddOperator(MinusInfo(), MinusValueMap, MRMRMRTypeMap);
        AddOperator(IntersectInfo(), IntersectValueMap, MRMRMRTypeMap);
        AddOperator(InsideInfo(), InsideValueMap, MRMRMRTypeMap);

    }

    ~MRegionOps2Algebra() {

    }
};

}    // end of namespace mregionops2

extern "C"Algebra*
InitializeMRegionOps2Algebra( NestedList* nlRef,
        QueryProcessor* qpRef ) {

    // The C++ scope-operator :: must be used to qualify the full name
    return new mregionops2::MRegionOps2Algebra();
}
