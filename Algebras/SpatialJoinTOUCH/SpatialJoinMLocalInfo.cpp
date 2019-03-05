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
#include <deque>
#include "RTreeTouch.h"
#include "STR.h"
#include "SpatialJoinRowLocalInfo.h"

#include "Algebras/MainMemory2/MainMemoryExt.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mm2algebra;


SpatialJoinMLocalInfo::~SpatialJoinMLocalInfo() {
    delete rtree;
}

SpatialJoinMLocalInfo::SpatialJoinMLocalInfo(
        vector<Tuple*> _vA,
        vector<Tuple*> _vB,
        int vAIndex,
        int vBIndex,
        TupleType* _tt
) {
    vA = _vA;
    vB = _vB;
    tt = _tt;

    vector<NodeT*> buckets = STR::createBuckets(_vA, vAIndex);

    rtree = new RTreeTouch(_tt, vAIndex, vBIndex);

    int fanout = 6;
    rtree->constructTree(buckets, fanout);
}

MemoryRelObject* SpatialJoinMLocalInfo::getMatchings(vector<Tuple*> vB) {

    MemoryRelObject* matchings = new MemoryRelObject();

    for (Tuple* Btuple: vB) {
        for (Tuple* matchTuple :rtree->getTuplesOverlappingOnTreeWith(Btuple)) {
            matchings->addTuple(matchTuple);
        }
    }

    return matchings;
}