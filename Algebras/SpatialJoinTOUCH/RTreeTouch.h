/*
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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

#ifndef SECONDO_RTREE_H
#define SECONDO_RTREE_H

#include "tchNode.h"
#include <vector>
#include "Column/BinaryTuple.h"


class TupleType;
class Tuple;

namespace mmrtreetouch {


    class RTreeTouch {

    private:
        TupleType* tt;

        int _firstStreamWordIndex;
        int _secondStreamWordIndex;

        bool outputOn;
        bool isM;

        std::vector<std::vector<tchNode *> > reGroupByConsideringFanout(
                std::vector<tchNode * > sortedArray,
                int fanout
        );

        std::vector<Tuple*> joinPhase(tchNode * node, Tuple* objectB);

        std::vector<tchNode *> getNodesOfInnerNodeRecursive(
                tchNode * node,
                std::vector<mmrtreetouch::tchNode *> leafNodes,
                bool justLeafNodes = true
        );

        std::vector<Tuple*> getMatchingConcatenatedTuples(
                Tuple*B,
                std::vector<tchNode * > leafNodes
        );

        std::string recursiveInfo(tchNode * subRoot);

        Tuple* concatenateTuples(Tuple* tupleA, Tuple* tupleB);

        bool checkIfOverlapping(Tuple* tupleA, Tuple* tupleB);

        std::vector<Tuple*> findMatchingsTopToBottomRecurs(
                tchNode * node,
                std::vector<Tuple*> matchings
                );

        std::vector<Tuple*> findMatchingsTopToBottomRecursWithGridFirstLeaves(
                tchNode * node,
                std::vector<Tuple*> matchings
        );

        std::pair<double, double> findAverageSize(std::vector<Tuple*> tuples);
        std::pair<double, double> findAverageSizeOfTupleAs(
                std::vector<tchNode*> leafNodes
                );

    protected:
        int cellFactor;


    public:
        tchNode * root;

        RTreeTouch(
                TupleType* ttParam,
                int _firstStreamWordIndex,
                int _secondStreamWordIndex,
                int _cellFactor,
                bool _isM
                );

        virtual ~RTreeTouch();

        tchNode * constructTree(
                std::vector<tchNode * > sortedArray,
                int fanout
                );

        std::vector<Tuple*> getTuplesOverlappingOnTreeWith(Tuple* objectB);

        int64_t noLeaves();

        int64_t noLeaves(tchNode * someNode);

        void showSubTreeInfo(tchNode * subRoot);


        int64_t assignTupleB(Tuple* objectB);

        std::vector<Tuple*> findMatchings();



    };
} // end of namespace

#endif  // SECONDO_RTREE_H
