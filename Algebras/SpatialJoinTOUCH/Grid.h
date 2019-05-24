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

#ifndef SECONDO_GRID_H
#define SECONDO_GRID_H

#include <vector>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stack>

#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "tchNode.h"

class TupleType;

namespace mmrtreetouch {

    class Grid {

    private:

        int fAttrIndex;
        int sAttrIndex;
        TupleType* tt;
        Rectangle<2> box;
        double minX;
        double maxX;
        double minY;
        double maxY;
        double xCellDim;
        double yCellDim;
        double xLength;
        double yLength;
        int64_t numOfXCells;
        int64_t numOfYCells;
        int64_t cellSize;
        std::vector<Tuple*> matchings;

        std::vector<std::vector<std::vector<Tuple*> > > grid;

        int64_t calculateIndexX(double coord);
        int64_t calculateIndexY(double coord);

        bool checkIfOverlapping(Tuple* tupleA, Tuple* tupleB);

        bool tuplesIntersectInCell(
                Tuple* fTuple, Tuple* sTuple, int64_t i, int64_t j);

        Tuple* concatenateTuples(Tuple* tupleA, Tuple* tupleB);
    public:
        Grid(
                tchNode* node,
                double _xCellDim,
                double _yCellDim,
                int _fAttrIndex,
                int _sAttrIndex,
                TupleType* tt
        );

        int numOfComp;

        ~Grid();

        void addTuple(Tuple* t, int attrIndex);

        std::pair<std::pair<int64_t, int64_t>, std::pair<int64_t, int64_t>>
        getGridCoordinatesOf(
                Tuple* t,
                int attrIndex
                );

        std::vector<Tuple*> getMatchings();

        void setMatchings(std::vector<Tuple*> matchings);

        void getTuplesOverlappingWith(
                Tuple* fTuple,
                int attrIndex
        );
    };
}

#endif //SECONDO_GRID_H
