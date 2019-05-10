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

#include "GridVectorCol.h"
#include <algorithm>
#include "Algebras/RTree/RTreeAlgebra.h"
#include <memory>
#include "BinaryTuple.h"

using namespace mmrtreetouch;
using namespace std;

typedef long long int ullong;

GridVectorCol::GridVectorCol(
        nodeCol* node,
        double _xCellDim,
        double _yCellDim
) {
    Rectangle<2> box = node->box;

    minX = (double) box.MinD(0);
    maxX = (double) box.MaxD(0);
    minY = (double) box.MinD(1);
    maxY = (double) box.MaxD(1);

    assert(minX < maxX);
    assert(minY < maxY);

    xLength = (maxX - minX) < 1 ? 1 : (maxX - minX);
    yLength = (maxY - minY) < 1 ? 1 : (maxY - minY);

    xCellDim = _xCellDim;
    yCellDim = _yCellDim;
    assert(xLength >= 0);
    assert(yLength >= 0);
    assert(xCellDim > 0);
    assert(yCellDim > 0);

    numOfXCells = (ullong) ceil(xLength / xCellDim);
    numOfYCells = (ullong) ceil(yLength / yCellDim);


    vector<vector<vector<binaryTuple> > > gridVectorCopy (
            numOfXCells,
            vector<vector<binaryTuple> >(numOfYCells, vector<binaryTuple >(0)));
    gridVectorCol = gridVectorCopy;

    assert(numOfXCells >= 1);
    assert(numOfYCells >= 1);


}

GridVectorCol::~GridVectorCol() {};

int GridVectorCol::calculateIndexX(double coord) {

    ullong index = 0; // could become negative
    double numRes = ((coord-minX) * numOfXCells) / xLength;
    ullong roundDown = (ullong) floor(numRes);
    index = roundDown;

    if (roundDown == numOfXCells) {
        index = roundDown - 1;
    }

    if (index < 0) {
        index = 0;
    } else if (index > numOfXCells) {
        index = numOfXCells - 1;
    }

    assert(index >= 0);
    assert(index < numOfXCells);

    return index;
}

int GridVectorCol::calculateIndexY(double coord) {

    ullong index;
    double numRes = ((coord-minY) * numOfYCells) / yLength;
    ullong roundDown = (ullong) floor(numRes);
    index = roundDown;

    if (roundDown == numOfYCells) {
        index = roundDown - 1;
    }

    if (index < 0) {
        index = 0;
    } else if (index > numOfYCells) {
        index = numOfYCells - 1;
    }

    assert(index >= 0);
    assert(index < numOfYCells);

    return index;
}

pair<pair<int, int>, pair<int, int>> GridVectorCol::getGridCoordinatesOf(
        binaryTuple bt
) {

    int tMinX = calculateIndexX(bt.xMin);
    int tMaxX = calculateIndexX(bt.xMax);

    int tMinY = calculateIndexY(bt.yMin);
    int tMaxY = calculateIndexY(bt.yMax);

    return make_pair(make_pair(tMinX, tMaxX), make_pair(tMinY, tMaxY));
}

vector<pair<binaryTuple, binaryTuple>> GridVectorCol::getTuplesOverlappingWith(
        binaryTuple * tbB,
        vector<pair<binaryTuple, binaryTuple>> matchings
) {

    pair<pair<int, int>, pair<int, int>> indexes = getGridCoordinatesOf(
            *tbB
    );

    pair<int, int> xPair = indexes.first;
    pair<int, int> yPair = indexes.second;

    for (int i = xPair.first; i <= xPair.second; i++) {
        for (int j = yPair.first; j <= yPair.second; j++) {

            vector<binaryTuple> temp = gridVectorCol[i][j];

            for (binaryTuple tbA: temp) {

                if (tuplesIntersectInCell(tbA, *tbB, i, j)) {

                    pair<binaryTuple, binaryTuple> res  = make_pair(tbA, *tbB);

                    matchings.push_back(res);
                }
            }

        }
    }

    return  matchings;
}

bool GridVectorCol::tuplesIntersectInCell(
        binaryTuple btA, binaryTuple btB, int i, int j) {

    double min[2];
    double max[2];
    min[0] = btA.xMin;
    min[1] = btA.yMin;
    max[0] = btA.xMax;
    max[1] = btA.yMax;

    Rectangle<2>* boxA = new Rectangle<2>(true, min, max);

    min[0] = btB.xMin;
    min[1] = btB.yMin;
    max[0] = btB.xMax;
    max[1] = btB.yMax;

    Rectangle<2>* boxB = new Rectangle<2>(true, min, max);

    // if the lower left edge of the intersection of the two boxes
    // is in the same cell then true, else false

    Rectangle<2> intersectionBox = boxA->Intersection(*boxB);

    bool boxesIntersect = boxA->Intersects(*boxB, 0);

    delete boxA;
    delete boxB;

    double minX = (double) intersectionBox.MinD(0);
    double minY = (double) intersectionBox.MinD(1);

    int intersectionIndexX = calculateIndexX(minX);
    int intersectionIndexY = calculateIndexY(minY);

    if (intersectionIndexX == i && intersectionIndexY == j) {
        if (boxesIntersect) {
            return true;
        }
    }

    return false;
}

void GridVectorCol::addTuple(binaryTuple * bt) {

    int tMinX = calculateIndexX(bt->xMin);
    int tMaxX = calculateIndexX(bt->xMax);

    int tMinY = calculateIndexY(bt->yMin);
    int tMaxY = calculateIndexY(bt->yMax);

    for (int i = tMinX; i <= tMaxX; i++) {
        for (int j = tMinY; j <= tMaxY; j++) {
            gridVectorCol[i][j].push_back(*bt);
        }
    }
}

