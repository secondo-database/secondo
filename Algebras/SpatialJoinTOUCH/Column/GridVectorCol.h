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

#ifndef SECONDO_GRIDVECTORCOL_H
#define SECONDO_GRIDVECTORCOL_H

#include <vector>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stack>
#include "tupleBlockStr.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "nodeCol.h"

namespace mmrtreetouch {

    class GridVectorCol {

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
        int64_t numOfXCells;
        int64_t numOfYCells;
        int64_t cellSize;
        std::vector <std::pair<tupleBlockStr, tupleBlockStr>> matchings;

        std::vector <std::vector<std::vector < tupleBlockStr>> >
        gridVectorCol;

        int64_t calculateIndexX(double coord);
        int64_t calculateIndexY(double coord);

        bool tuplesIntersectInCell(
                tupleBlockStr fTuple,
                tupleBlockStr sTuple,
                int64_t i,
                int64_t j
                );


    public:
        GridVectorCol(
                nodeCol* node,
                double _xCellDim,
                double _yCellDim,
                int64_t remainingMem
        );

        ~GridVectorCol();

        int64_t remainingMem;

        //
        void addTuple(tupleBlockStr bt);

        std::pair <std::pair<int64_t, int64_t>, std::pair<int64_t, int64_t>>
        getGridCoordinatesOf(
                tupleBlockStr t
        );

        std::vector <std::pair<tupleBlockStr, tupleBlockStr>> getMatchings();

        void setMatchings(
                std::vector <std::pair<tupleBlockStr, tupleBlockStr>> matchings
        );

        void getTuplesOverlappingWith(
                tupleBlockStr fTuple
        );

    };
}

#endif //SECONDO_GRIDVECTORCOL_H
