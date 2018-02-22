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

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file essentially contains the implementation of the algebra class and
the type- and value mapping functions of the three set operators
~intersection~, ~union~ and ~minus~ with the signature \\
movingregion [x] movingregion [->] movingregion \\
used in the MovingRegion Algebra.

For more detailed information please see the files ~SetOps.h~ and ~SetOps.cpp~.

2 Defines and Includes

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
#include "Algebras/MovingRegion/MovingRegionAlgebra.h"
#include "SetOps.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace mappings;
using namespace std;

namespace temporalalgebra{

namespace mregionops {

/*

3 Type Mapping Functions

*/

ListExpr MRMRMRTypeMap(ListExpr args) {

    NList type(args);
    const string errMsg = "Expecting two movingregions.";

    if (type.length() != 2)
        return NList::typeError(errMsg);

    // movingregion x movingregion -> movingregion
    if (type.first().isSymbol(MRegion::BasicType()) &&
        type.second().isSymbol(MRegion::BasicType())) {

        return NList(MRegion::BasicType()).listExpr();
    }

    return NList::typeError(errMsg);
}

/*

4 Value Mapping Functions

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

/*

5 Operator Descriptions

*/

struct IntersectionInfo : OperatorInfo {

    IntersectionInfo() {
        name = "intersection";
        signature = "mregion x mregion -> mregion";
        syntax = "intersection(_, _)";
        meaning = "Intersection operation for two moving regions.";
    }
};

struct UnionInfo : OperatorInfo {

    UnionInfo() {
        name = "union";
        signature = "mregion x mregion -> mregion";
        syntax = "_ union _";
        meaning = "Union operation for two moving regions.";
    }
};

struct MinusInfo : OperatorInfo {

    MinusInfo() {
        name = "minus";
        signature = "mregion x mregion -> mregion";
        syntax = "_ minus _";
        meaning = "Minus operation for two moving regions.";
    }
};

/*

6 Implementation of the Algebra Class

*/

class MRegionOpsAlgebra : public Algebra {

public:

    MRegionOpsAlgebra() :
        Algebra() {

        AddOperator(IntersectionInfo(), IntersectionValueMap, MRMRMRTypeMap);
        AddOperator(UnionInfo(), UnionValueMap, MRMRMRTypeMap);
        AddOperator(MinusInfo(), MinusValueMap, MRMRMRTypeMap);
    }

    ~MRegionOpsAlgebra() {

    }
};

} // end of namespace mregionops

} // end of namespace temporalalgebra

extern "C"Algebra*
InitializeMRegionOpsAlgebra( NestedList* nlRef,
        QueryProcessor* qpRef ) {

    // The C++ scope-operator :: must be used to qualify the full name
    return new temporalalgebra::mregionops::MRegionOpsAlgebra();
}

