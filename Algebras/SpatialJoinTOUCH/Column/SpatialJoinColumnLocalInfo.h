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

#ifndef SECONDO_SPATIALJOINCOLUMNLOCALINFO_H
#define SECONDO_SPATIALJOINCOLUMNLOCALINFO_H

#include "Algebras/CRel/TBlock.h"
#include "tupleBlockStr.h"


class TupleType;

namespace CRelAlgebra {
    class TBlockEntry;
    class TBlock;
}

namespace mmrtreetouch {
    class RTreeTouchCol;
}


extern NestedList* nl;
extern QueryProcessor *qp;


class SpatialJoinColumnLocalInfo
{
private:
    TupleType* tt;
    Word firstStreamWord;
    Word secondStreamWord;
    int _firstStreamIndex;
    int _secondStreamIndex;
    std::vector<const CRelAlgebra::TBlock*> fTBlockVector;
    std::vector<const CRelAlgebra::TBlock*> sTBlockVector;
    const CRelAlgebra::TBlockTI rTBlockTypeInfo;
    const CRelAlgebra::PTBlockInfo rTBlockInfo;
    uint64_t rTBlockSize;
    uint64_t maxRow;

    std::vector<mmrtreetouch::tupleBlockStr> bts;
    std::vector<mmrtreetouch::tupleBlockStr> btsB;

    CRelAlgebra::AttrArrayEntry* tuple;

    CRelAlgebra::TBlock* tempTBlock;

    void getAllTuplesFromStreamA(
            Word stream
    );

    void getAllTuplesFromStreamB(
            Word stream
    );

    std::vector<mmrtreetouch::tupleBlockStr> createTupleBlockStrVectorA(
            const uint64_t &joinIndex
    );

    std::vector<mmrtreetouch::tupleBlockStr> createTupleBlockStrVectorB(
            const uint64_t &joinIndex
    );

    std::vector<CRelAlgebra::TBlock *> matchingVector;

    void addtupleBlockStrToTBlock(
        std::pair<mmrtreetouch::tupleBlockStr ,
        mmrtreetouch::tupleBlockStr > btPair,
        const size_t fNumColumns,
        const size_t sNumColumns
    );

    void assignTuplesB(std::vector<mmrtreetouch::tupleBlockStr> BBTs);

    void findMatchings();

public:
    mmrtreetouch::RTreeTouchCol * rtree;

    SpatialJoinColumnLocalInfo(
            Word firstStreamWordParam,
            Word secondStreamWordParam,
            int fanout,
            int numOfItemsInBucket,
            int cellFactor,
            int firstStreamWordIndex,
            int secondStreamWordIndex,
            ListExpr ttl,
            Supplier s
    );

    ~SpatialJoinColumnLocalInfo();

    std::stringstream ss;

    CRelAlgebra::TBlock* NextResultTBlock();

};

#endif //SECONDO_SPATIALJOINCOLUMNLOCALINFO_H

