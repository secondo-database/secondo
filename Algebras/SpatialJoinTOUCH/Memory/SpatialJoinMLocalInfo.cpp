/*
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Symbols.h"
#include "Algebras/Stream/Stream.h"
#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include <string>

#include <iostream>
#include <typeinfo>
#include "../STR.h"
#include "SpatialJoinMLocalInfo.h"
#include "../tchNode.h"
#include "../RTreeTouch.h"

#include "Algebras/MainMemory2/MainMemoryExt.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mm2algebra;
using namespace mmrtreetouch;


SpatialJoinMLocalInfo::~SpatialJoinMLocalInfo() {
    delete rtree;
    vLeft.clear();
    vRight.clear();
}

SpatialJoinMLocalInfo::SpatialJoinMLocalInfo(
        vector<Tuple*> vLeftIn,
        vector<Tuple*> vRightIn,
        int vLeftIndexIn,
        int vRightIndexIn,
        TupleType* ttIn,
        int fanoutIn,
        int numOfItemsInBucket,
        int cellFactor
) {
    vLeft = vLeftIn;
    vRight = vRightIn;
    tt = ttIn;

    vector<tchNode*> buckets = STR::createBuckets(
            vLeft,
            vLeftIndexIn,
            numOfItemsInBucket
            );

    rtree = new RTreeTouch(tt, vLeftIndexIn, vRightIndexIn, cellFactor);

    int fanout = fanoutIn;

    rtree->constructTree(buckets, fanout);

}

MemoryRelObject* SpatialJoinMLocalInfo::getMatchings(vector<Tuple*> vRight) {

    MemoryRelObject* matchings = new MemoryRelObject();

    for (Tuple* Btuple: vRight) {
        rtree->assignTupleB(Btuple);
    }

    vector<Tuple*> matchingsVector = rtree->findMatchings();

    for (Tuple* match: matchingsVector) {
        matchings->addTuple(match);
    }

    return matchings;
}
