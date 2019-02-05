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



#include <string.h>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <deque>
#include <utility>
#include <stack>
#include "NodeT.h"

#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace mmrtreetouch {


    class RTreeTouch {
    public:
        NodeT* root;

        RTreeTouch(
                TupleType* ttParam,
                int _firstStreamWordIndex,
                int _secondStreamWordIndex
                );

        ~RTreeTouch();

        NodeT* constructTree(std::deque<NodeT*> sortedArray, int fanout);

        deque<Tuple*> getTuplesOverlappingOnTreeWith(Tuple* objectB);

        int noLeaves();

        int noLeaves(NodeT* someNode);

        void showSubTreeInfo(NodeT* subRoot);

    private:
        TupleType* tt;

        int _firstStreamWordIndex;
        int _secondStreamWordIndex;

        bool outputOn;

        deque<deque<NodeT*> > reGroupByConsideringFanout(
                std::deque<NodeT*> sortedArray,
                int fanout
                );

        deque<Tuple*> joinPhase(NodeT* node, Tuple* objectB);

        deque<NodeT*> getNodesOfInnerNodeRecursive(
                NodeT* node,
                std::deque<NodeT*> leafNodes,
                bool justLeafNodes = true
                );

        deque<Tuple*> getMatchingConcatenatedTuples(
                Tuple*B,
                std::deque<NodeT*> leafNodes
                );

        string recursiveInfo(NodeT* subRoot);

    };
} // end of namespace

#endif
