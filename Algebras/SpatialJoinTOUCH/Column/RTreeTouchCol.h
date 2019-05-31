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

#ifndef SECONDO_RTREETOUCHCOL_H
#define SECONDO_RTREETOUCHCOL_H

//#include "RTreeTouch.h"
#include <vector>
#include "BinaryTuple.h"
#include "nodeCol.h"

class TupleType;

namespace mmrtreetouch {

    //class RTreeTouch;

    class RTreeTouchCol {
    private:

        std::vector<std::pair<binaryTuple, binaryTuple >> matchings;

        int cellFactor;

        std::vector<std::vector<nodeCol *> > reGroupByConsideringFanout(
                std::vector<nodeCol * > sortedArray,
                int fanout
        );

        std::vector<nodeCol * > getNodesOfInnerNodeRecursive(
                nodeCol * node,
                std::vector<nodeCol *> nodes,
                bool justLeafNodes = true
        );

        std::pair<double, double> findAverageSizeOfTupleAs(
                std::vector<nodeCol*> leafNodes
        );

        void removeBsFromTree(nodeCol* node);

        void findMatchingsRecurvGrid(
                nodeCol * node
        );

        //##

        //bool checkIfOverlapping(Tuple* tupleA, Tuple* tupleB);

        //#

    public:

        int64_t remainingMem;

        RTreeTouchCol(
                TupleType *ttParam,
                int firstStreamWordIndex,
                int secondStreamWordIndex,
                int _cellFactor,
                int64_t &remainingMem
        );

        ~RTreeTouchCol();

        nodeCol * root = nullptr;

        nodeCol * constructTree(std::vector<nodeCol *> sortedArray, int fanout);

        int64_t assignTupleBs(binaryTuple bt);

        std::vector<std::pair<binaryTuple , binaryTuple >> findMatchings();

        int noLeaves();

        int noLeaves(nodeCol * someNode);

        std::string recursiveInfo(nodeCol * subRoot);

        void showSubTreeInfo(nodeCol * subRoot);
    };

}






#endif //SECONDO_RTREETOUCHCOL_H
