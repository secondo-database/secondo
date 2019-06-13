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

#ifndef SECONDO_SPATIALJOINROWLOCALINFO_H
#define SECONDO_SPATIALJOINROWLOCALINFO_H

class TupleType;

namespace mmrtreetouch {
    class RTreeTouch;
}

extern NestedList* nl;
extern QueryProcessor *qp;

// Row Version
class SpatialJoinRowLocalInfo
{
private:
    TupleType* tt;
    Word firstStreamWord;
    Word secondStreamWord;
    int _firstStreamWordIndex;
    int cellFactor;

    std::vector<Tuple*> getAllTuplesFromStream (const Word stream);


    std::vector<Tuple*> matchingVector;
    std::vector<Tuple*> tuplesA;
    std::vector<Tuple*> tuplesB;

    bool turnOn;

    void findMatchings();

public:
    mmrtreetouch::RTreeTouch * rtree;

    SpatialJoinRowLocalInfo(
            Word firstStreamWordParam,
            Word secondStreamWordParam,
            int firstStreamWordIndex,
            int secondStreamWordIndex,
            ListExpr ttl,
            int _fanout,
            int _numOfItemsInBucket,
            int _cellFactor
    );

    ~SpatialJoinRowLocalInfo();

    std::stringstream ss;

    void assignTuplesB(std::vector<Tuple*> tuplesB);

    Tuple* NextResultTuple ();

};

#endif //SECONDO_SPATIALJOINROWLOCALINFO_H
