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

class NestedList;
class QueryProcessor;
class tchNode;
#include "../RTreeTouch.h"
#include "SpatialJoinRowLocalInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"





extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mmrtreetouch;


namespace STR {
vector<mmrtreetouch::tchNode *> createBuckets(
        vector<::Tuple*> tuples,
        int firstStreamWordIndex,
        int _numOfItemsInBucket
);
}


SpatialJoinRowLocalInfo::~SpatialJoinRowLocalInfo() {
    for (Tuple * tuple:tuplesA) {
        delete tuple;
    }

    for (Tuple * tupleB:tuplesB) {
        delete tupleB;
    }

    delete rtree;

    tt->DeleteIfAllowed();
};


SpatialJoinRowLocalInfo::SpatialJoinRowLocalInfo(
        Word firstStreamWordParam,
        Word secondStreamWordParam,
        int firstStreamWordIndex,
        int secondStreamWordIndex,
        ListExpr ttl,
        int _fanout,
        int _numOfItemsInBucket,
        int _cellFactor
)
{
    // Row Version
    turnOn = false;

    tt = new ::TupleType(ttl);
    _firstStreamWordIndex = firstStreamWordIndex;
    cellFactor = _cellFactor;

    firstStreamWord = firstStreamWordParam;
    secondStreamWord = secondStreamWordParam;

    tuplesA = getAllTuplesFromStream(firstStreamWord);
    tuplesB = getAllTuplesFromStream(secondStreamWord);

    vector<mmrtreetouch::tchNode * > buckets = STR::createBuckets(
            tuplesA,
            _firstStreamWordIndex,
            _numOfItemsInBucket
            );

    rtree = new RTreeTouch(
            tt,
            firstStreamWordIndex,
            secondStreamWordIndex,
            cellFactor,
            false
            );

    int fanout = _fanout;

    rtree->constructTree(buckets, fanout);

    assignTuplesB(tuplesB);

    findMatchings();
};

void SpatialJoinRowLocalInfo::assignTuplesB(vector<Tuple*> tuplesB)
{
    for (Tuple* tupleB: tuplesB) {
        rtree->assignTupleB(tupleB);
    }
}

void SpatialJoinRowLocalInfo::findMatchings()
{
    matchingVector = rtree->findMatchings();
}

Tuple* SpatialJoinRowLocalInfo::NextResultTuple () {
    if (!matchingVector.empty()) {
        Tuple *tupleToReturn = matchingVector.back();
        matchingVector.pop_back();

        return tupleToReturn;
    } else {
        return 0;
    }
}

vector<Tuple*> SpatialJoinRowLocalInfo::getAllTuplesFromStream(
        const Word stream)
{
    Word streamTupleWord;
    vector<Tuple*> tuples;

    qp->Open (stream.addr);
    qp->Request (stream.addr, streamTupleWord);

    while (qp->Received (stream.addr) )
    {
        Tuple* t = (Tuple*) streamTupleWord.addr;

        tuples.push_back(t);

        qp->Request ( stream.addr, streamTupleWord);
    }

    qp->Close (stream.addr);

    return tuples;
}