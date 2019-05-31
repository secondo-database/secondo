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

#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"
#include "Algebras/CRel/SpatialAttrArray.h"

#include "RTreeTouchCol.h"
#include "SpatialJoinColumnLocalInfo.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "tupleBlockStr.h"

class NestedList;
class QueryProcessor;
class nodeCol;

namespace CRelAlgebra {
    class TBlockEntry;
    class TBlock;
    class TBlockIterator;
}

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
using namespace mmrtreetouch;
using namespace CRelAlgebra;


namespace STRColumn {
    vector<mmrtreetouch::nodeCol *> createBuckets(
            vector<tupleBlockStr> tuples,
            int _numOfItemsInBucket,
            int64_t &remainingMem
    );
}


SpatialJoinColumnLocalInfo::~SpatialJoinColumnLocalInfo() {
    tt->DeleteIfAllowed();

    delete rtree;

    for (const TBlock* tb: fTBlockVector) {
        //delete tb;
        if(tb) {
            tb->DecRef();
        }
    }
    fTBlockVector.clear();

    for (const TBlock* tb: sTBlockVector) {
        //delete tb;
        if(tb) {
            tb->DecRef();
        }
    }
    sTBlockVector.clear();

    delete[] tuple;
};


SpatialJoinColumnLocalInfo::SpatialJoinColumnLocalInfo(
        Word firstStreamWordParam,
        Word secondStreamWordParam,
        Word _fanout,
        Word _numOfItemsInBucket,
        Word cellFactorWord,
        int firstStreamWordIndex,
        int secondStreamWordIndex,
        ListExpr ttl,
        Supplier s,
        int64_t _remainingMem
):rTBlockTypeInfo(CRelAlgebra::TBlockTI(qp->GetType(s), false)),
  rTBlockInfo(rTBlockTypeInfo.GetBlockInfo()),
  rTBlockSize(rTBlockTypeInfo.GetDesiredBlockSize()
              * CRelAlgebra::TBlockTI::blockSizeFactor),
  tuple(0),
  remainingMem(_remainingMem)
{
    tt = new ::TupleType(ttl);

    remainingMem -= sizeof(TupleType);

    firstStreamWord = firstStreamWordParam;
    secondStreamWord = secondStreamWordParam;

    int fanout = ((CcInt*)_fanout.addr)->GetIntval();
    int numOfItemsInBucket = ((CcInt*)_numOfItemsInBucket.addr)->GetIntval();
    int cellFactor = ((CcInt*)cellFactorWord.addr)->GetIntval();

    _firstStreamIndex = firstStreamWordIndex;
    _secondStreamIndex = secondStreamWordIndex;

    cout << "1 " << endl;

    bts = getAllTuplesFromStream(firstStreamWord, _firstStreamIndex);

    cout << "3 " << endl;

    if (remainingMem > 0) {

        btsB = getAllTuplesFromStream(secondStreamWord, _secondStreamIndex);

        cout << "btsB size: " << sizeof(tupleBlockStr)*btsB.size() << endl;
        cout << "btsA size: " << sizeof(tupleBlockStr)*bts.size() << endl;

        if (remainingMem > 0) {

            vector<mmrtreetouch::nodeCol * > buckets =
               STRColumn::createBuckets(bts, numOfItemsInBucket, remainingMem);

            cout << "create buckets " << endl;

            if (remainingMem > 0) {

                rtree = new RTreeTouchCol(
                        tt,
                        _firstStreamIndex,
                        _secondStreamIndex,
                        cellFactor,
                        remainingMem
                );

                remainingMem -= sizeof(RTreeTouchCol);

                rtree->constructTree(buckets, fanout);

                cout << "constructTree " << endl;


                assignTuplesB(btsB);

                cout << "assignTuplesB " << endl;

                findMatchings();

                cout << "findMatchings" << endl;

                remainingMem = rtree->remainingMem;
            }
        }
    }
}

void SpatialJoinColumnLocalInfo::assignTuplesB(vector<tupleBlockStr> BBTs)
{
    for (tupleBlockStr tupleB: BBTs) {
        rtree->assignTupleBs(tupleB);
    }
}

void SpatialJoinColumnLocalInfo::findMatchings()
{
    vector<pair<tupleBlockStr, tupleBlockStr >> matchings =
            rtree->findMatchings();

    cout << "after tree findMatchings" << endl;

    const size_t fNumColumns = fTBlockVector[0]->GetColumnCount();
    const size_t sNumColumns = sTBlockVector[0]->GetColumnCount();

    if (remainingMem-sizeof(AttrArrayEntry) <= 0) {
        cout << "Memory is not enough 1" << endl;
        remainingMem -= sizeof(AttrArrayEntry);
        return;
    }

    tuple = new AttrArrayEntry[fNumColumns + sNumColumns];

    remainingMem -= sizeof(AttrArrayEntry);

    if (remainingMem-sizeof(tempTBlock) <= 0) {
        cout << "Memory is not enough 2" << endl;
        remainingMem -= sizeof(tempTBlock);
        return;
    }

    tempTBlock = new TBlock(rTBlockInfo, 0, 0);

    remainingMem -= sizeof(tempTBlock);

    for (pair<tupleBlockStr, tupleBlockStr> btPair: matchings) {

        addtupleBlockStrToTBlock(
                btPair,
                tuple,
                fNumColumns,
                sNumColumns);
    }

    if (tempTBlock->GetRowCount() > 0) {
        matchingVector.push_back(tempTBlock);
        tempTBlock = 0;
    }
}

TBlock* SpatialJoinColumnLocalInfo::NextResultTBlock () {
    if (remainingMem <= 0) {
        return 0;
    }

    if (!matchingVector.empty()) {
        TBlock *tBlockToReturn = matchingVector.back();
        matchingVector.pop_back();

        return tBlockToReturn;
    } else {
        return 0;
    }
}



// TODO: use second stream
void SpatialJoinColumnLocalInfo::addtupleBlockStrToTBlock(
        pair<tupleBlockStr, tupleBlockStr> btPair,
        AttrArrayEntry* tuple,
        const size_t fNumColumns,
        const size_t sNumColumns
        )
{
    tupleBlockStr fBT = btPair.first;
    tupleBlockStr sBT = btPair.second;

    const CRelAlgebra::TBlockEntry &fTuple = CRelAlgebra::TBlockEntry(
            fTBlockVector[fBT.blockNum],
            fBT.row);

    const CRelAlgebra::TBlockEntry &sTuple = CRelAlgebra::TBlockEntry(
            sTBlockVector[sBT.blockNum],
            sBT.row
    );

    for(uint64_t i = 0; i < fNumColumns; i++) {
        tuple[i] = fTuple[i];
    }

    for(uint64_t i = 0; i < sNumColumns; i++) {
        tuple[fNumColumns + i] = sTuple[i];
    }

    tempTBlock->Append(tuple);

    if(tempTBlock->GetSize() >= rTBlockSize) { // if tempTBlock is full
        matchingVector.push_back(tempTBlock);

        if (remainingMem-sizeof(tempTBlock) <= 0) {
            cout << "Memory is not enough" << endl;
            remainingMem -= sizeof(TBlock);
            return;
        }

        tempTBlock = new TBlock(rTBlockInfo, 0, 0);

        remainingMem -= sizeof(TBlock);
    }
}

vector<mmrtreetouch::tupleBlockStr>
        SpatialJoinColumnLocalInfo::getAllTuplesFromStream(
        const Word stream,
        const uint64_t joinIndex
        )
{
    Word streamTBlockWord;

    qp->Open (stream.addr);
    qp->Request (stream.addr, streamTBlockWord);

    vector<tupleBlockStr> BT;
    uint64_t tBlockNum = 0;
    tupleBlockStr temp;

    //cout << "A-1" << endl;

    while (qp->Received (stream.addr) )
    {
        TBlock* tBlock = (TBlock*) streamTBlockWord.addr;

        //cout << "A-2" << endl;


        CRelAlgebra::TBlockIterator tBlockIter = tBlock->GetIterator();
        uint64_t row = 0;

        while(tBlockIter.IsValid()) {

            if (remainingMem-sizeof(tupleBlockStr) <= 0) {
                cout << "Memory is not enough 4" << endl;
                remainingMem -= sizeof(tupleBlockStr);
                return BT;
            }

            const CRelAlgebra::TBlockEntry &tuple = tBlockIter.Get();

            temp.blockNum = tBlockNum;
            temp.row = row;

            //cout << "A-3" << endl;

            CRelAlgebra::SpatialAttrArrayEntry<2> attribute = tuple[joinIndex];

            //cout << "A-3 1" << endl;

            const Rectangle<2> &rec = attribute.GetBoundingBox();

            //cout << "A-3 2" << endl;

            temp.xMin = rec.MinD(0);
            temp.xMax = rec.MaxD(0);
            temp.yMin = rec.MinD(1);
            temp.yMax = rec.MaxD(1);

            //cout << "A-3 3" << endl;

            BT.push_back(temp);



            //cout << "A-3 4" << endl;
            ++row;
            //cout << "A-3 5" << endl;
            tBlockIter.MoveToNext();

            remainingMem -= sizeof(tupleBlockStr);

            //cout << "A-4" << endl;
        }

        ++tBlockNum;

        qp->Request ( stream.addr, streamTBlockWord);
    }

    qp->Close (stream.addr);

    return BT;
}

