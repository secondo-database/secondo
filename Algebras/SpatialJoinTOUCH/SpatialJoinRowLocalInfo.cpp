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
#include <vector>
#include "RTreeTouch.h"
#include "STR.h"
#include "SpatialJoinRowLocalInfo.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mmrtreetouch;

SpatialJoinRowLocalInfo::~SpatialJoinRowLocalInfo() {
    tt->DeleteIfAllowed();

    delete rtree;
};


SpatialJoinRowLocalInfo::SpatialJoinRowLocalInfo(
        Word firstStreamWordParam,
        Word secondStreamWordParam,
        int firstStreamWordIndex,
        int secondStreamWordIndex,
        ListExpr ttl
)
{
    turnOn = false;

    tt = new TupleType(ttl);
    _firstStreamWordIndex = firstStreamWordIndex;

    firstStreamWord = firstStreamWordParam;
    secondStreamWord = secondStreamWordParam;

    vector<Tuple*> tuples = getAllTuplesFromStream(firstStreamWord);

    vector<NodeT*> buckets = STR::createBuckets(tuples, _firstStreamWordIndex);

    rtree = new RTreeTouch(tt, firstStreamWordIndex, secondStreamWordIndex);

    int fanout = 6;
    rtree->constructTree(buckets, fanout);
};


//    Tuple* SpatialJoinLocalInfo::NextResultTuple (Tuple* Btuple)
Tuple* SpatialJoinRowLocalInfo::NextResultTuple ()
{
    if (turnOn) {
        cout << "##### Starting new NextResultTuple" << endl;
        cout << "size of matchingVector: " <<
             (int) matchingVector.size() << endl;
    }

    if (matchingVector.empty()) {

        Word BTupleWord;
        qp->Request(secondStreamWord.addr, BTupleWord);

        if (qp->Received ( secondStreamWord.addr)) {

            Tuple* Btuple = (Tuple*) BTupleWord.addr;

            // find matching for given Tuple
            // (overlappings between TupleB and Dataset A)
            matchingVector = rtree->getTuplesOverlappingOnTreeWith(Btuple);

            if (turnOn) {
                cout << "+++ Fresh matching Vector- Its size: " <<
                     (int) matchingVector.size() << endl;
            }

            if (!matchingVector.empty()) {
                // return last item of vector (and remove from vector)
                Tuple* tupleToReturn = matchingVector.back();
                matchingVector.pop_back();

                if (turnOn) {
                    cout << " MatchingVector was not empty, one less: " <<
                         (int) matchingVector.size() << endl;
                }
                return tupleToReturn;
            } else {
                /*
                 if there was no match between TupleB and Dataset A -
                 call function again, to request next Tuple Stream
                */
                if (turnOn) {
                    cout << "Call recursively NextResultTuple" << endl;
                }
                return NextResultTuple();
            }
        } else {
            // There are no more Tuples (Dataset B) first in the stream
            if (turnOn) {
                cout << "there are no more left of Dataset B" << endl;
            }
            return 0;
        }

    } else {

        Tuple* tupleToReturn = matchingVector.back();
        matchingVector.pop_back();

        return tupleToReturn;
    }

    cout << "NextResultTuple returns 0" << endl;
    return 0;
};


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