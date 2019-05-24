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

#ifndef SECONDO_GRIDVECTORCOL_H
#define SECONDO_GRIDVECTORCOL_H

#include <vector>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stack>
#include "BinaryTuple.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "nodeCol.h"

class TupleType;

namespace mmrtreetouch {

    class GridVectorCol {


    public:
        GridVectorCol(
                nodeCol *node,
                double _xCellDim,
                double _yCellDim
        );

        ~GridVectorCol();

        void addTuple(binaryTuple *bt);

        std::pair <std::pair<int, int>, std::pair<int, int>>
        getGridCoordinatesOf(
                binaryTuple t
        );

        void
        getTuplesOverlappingWith(
                binaryTuple *fTuple
        );

        void setMatchings(
                std::vector <std::pair<binaryTuple, binaryTuple>> matchings
                );

        std::vector <std::pair<binaryTuple, binaryTuple>> getMatchings();

    private:

        Rectangle<2> box;
        double minX;
        double maxX;
        double minY;
        double maxY;
        double xCellDim;
        double yCellDim;
        double xLength;
        double yLength;
        long long int numOfXCells;
        long long int numOfYCells;
        long long int cellSize;

        std::vector <std::pair<binaryTuple, binaryTuple>> matchings;

        std::vector <std::vector<std::vector < binaryTuple>> >
        gridVectorCol;

        int calculateIndexX(double coord);

        int calculateIndexY(double coord);

        void initializeGrid();

        void initializeGridArray(
                long long int nRows,
                long long int nCols
        );

        bool tuplesIntersectInCell(
                binaryTuple fTuple,
                binaryTuple sTuple,
                int i,
                int j
        );
    };
}


#endif //SECONDO_GRIDVECTORCOL_H
