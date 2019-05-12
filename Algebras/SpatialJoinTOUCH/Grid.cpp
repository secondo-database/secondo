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


#include "Grid.h"
#include <algorithm>
#include "Algebras/RTree/RTreeAlgebra.h"

using namespace mmrtreetouch;
using namespace std;

Grid::Grid(
        tchNode* node,
        double _xCellDim,
        double _yCellDim,
        int _fAttrIndex,
        int _sAttrIndex,
        TupleType* _tt
        ):
        fAttrIndex(_fAttrIndex),
        sAttrIndex(_sAttrIndex),
        tt(_tt),
        box(node->box),
        minX((double) box.MinD(0)),
        maxX((double) box.MaxD(0)),
        minY((double) box.MinD(1)),
        maxY((double) box.MaxD(1)),

        xCellDim(_xCellDim == 0 ? 0.1: _xCellDim),
        yCellDim(_yCellDim == 0 ? 0.1: _yCellDim),
        xLength(maxX - minX),
        yLength(maxY - minY),
        numOfXCells((int) ceil(xLength / xCellDim)),
        numOfYCells((int) ceil(yLength / yCellDim)),
        cellSize((int)node->objectsB.size()),
        grid(
                numOfXCells,
                vector<vector<Tuple*> >(numOfYCells, vector<Tuple* >(0)))
{
    xCellDim = xLength / numOfXCells;
    yCellDim = yLength / numOfYCells;
}



Grid::~Grid() {
};

int Grid::calculateIndexX(double coord) {
    int index;
    int roundDown = (int) floor((coord-minX) / xCellDim);

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


int Grid::calculateIndexY(double coord) {
    int index;
    int roundDown = (int) floor((coord-minY) / yCellDim);

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



pair<pair<int, int>, pair<int, int>> Grid::getGridCoordinatesOf(
        Tuple* t,
        int attrIndex
        ) {
    Attribute* attr = t->GetAttribute(attrIndex);

    int tMinX = calculateIndexX(attr->getMinX());
    int tMaxX = calculateIndexX(attr->getMaxX());

    int tMinY = calculateIndexY(attr->getMinY());
    int tMaxY = calculateIndexY(attr->getMaxY());

    return make_pair(make_pair(tMinX, tMaxX), make_pair(tMinY, tMaxY));
}

void Grid::addTuple(Tuple* t, int attrIndex) {

    Attribute* attr = t->GetAttribute(attrIndex);

    int tMinX = calculateIndexX(attr->getMinX());
    int tMaxX = calculateIndexX(attr->getMaxX());

    int tMinY = calculateIndexY(attr->getMinY());
    int tMaxY = calculateIndexY(attr->getMaxY());
    
    for (int i = tMinX; i <= tMaxX; i++) {
        for (int j = tMinY; j <= tMaxY; j++) {
            grid.at(i).at(j).push_back(t);
        }
    }
}


bool Grid::checkIfOverlapping(Tuple* tupleA, Tuple* tupleB) {

    StandardSpatialAttribute<2> * attrA1 =
            (StandardSpatialAttribute<2>*) tupleA->GetAttribute(
                    fAttrIndex
            );

    Rectangle<2> boxA = attrA1->BoundingBox();

    StandardSpatialAttribute<2> * attrB1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    sAttrIndex
            );

    Rectangle<2> boxB = attrB1->BoundingBox();

    if (boxA.Intersects(boxB, 0)) {
        return true;
    }

    return false;
}

Tuple* Grid::concatenateTuples(Tuple* tupleA, Tuple* tupleB) {
    Tuple* result = new Tuple(tt);
    Concat(tupleA, tupleB, result);

    return result;
};

vector<Tuple*> Grid::getTuplesOverlappingWith(
        Tuple* TupleB,
        int attrIndex,
        vector<Tuple*> matchings
) {

    pair<pair<int, int>, pair<int, int>> indexes = getGridCoordinatesOf(
            TupleB,
            attrIndex
    );

    StandardSpatialAttribute<2> * attr2 =
            (StandardSpatialAttribute<2>*) TupleB->GetAttribute(sAttrIndex);

    Rectangle<2> tupleBBox = attr2->BoundingBox();

    pair<int, int> xPair = indexes.first;
    pair<int, int> yPair = indexes.second;

    for (int i = xPair.first; i <= xPair.second; i++) {
        for (int j = yPair.first; j <= yPair.second; j++) {
            vector<Tuple*> temp = grid[i][j];

            for (Tuple* TupleA: temp) {

                numOfComp++;

                StandardSpatialAttribute<2> * attr1 =
                    (StandardSpatialAttribute<2>*) TupleA->GetAttribute(
                            fAttrIndex
                            );


                Rectangle<2> intersectionBox =
                        attr1->BoundingBox().Intersection(tupleBBox);

                double minX = (double) intersectionBox.MinD(0);
                double minY = (double) intersectionBox.MinD(1);

                int intersectionIndexX = calculateIndexX(minX);
                int intersectionIndexY = calculateIndexY(minY);

                if (intersectionIndexX == i && intersectionIndexY == j) {
                    if (checkIfOverlapping(TupleA, TupleB)) {
                        Tuple* res = concatenateTuples(TupleA,TupleB);
                        matchings.push_back(res);
                    }
                }
            }
        }
    }

    return  matchings;
}

bool Grid::tuplesIntersectInCell(Tuple* TupleA, Tuple* TupleB, int i, int j) {

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) TupleA->GetAttribute(fAttrIndex);

    StandardSpatialAttribute<2> * attr2 =
            (StandardSpatialAttribute<2>*) TupleB->GetAttribute(sAttrIndex);

    Rectangle<2> intersectionBox =
                attr1->BoundingBox().Intersection(attr2->BoundingBox());

    double minX = (double) intersectionBox.MinD(0);
    double minY = (double) intersectionBox.MinD(1);

    int intersectionIndexX = calculateIndexX(minX);
    int intersectionIndexY = calculateIndexY(minY);

    if (intersectionIndexX == i && intersectionIndexY == j) {
        return true;
    }

    return false;
}
