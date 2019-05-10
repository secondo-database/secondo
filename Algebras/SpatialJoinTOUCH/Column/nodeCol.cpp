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

#include "nodeCol.h"
#include "Algebras/CRel/SpatialAttrArray.h"
#include "Algebras/CRel/TBlock.h"
#include <typeinfo>
#include "BinaryTuple.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"


namespace CRelAlgebra {
    class TBlockEntry;
    class TBlock;
}

using namespace std;
using namespace mmrtreetouch;
using namespace CRelAlgebra;

void nodeCol::addObject(binaryTuple * t) {
    objects.push_back(t);

    double min[2];
    double max[2];
    min[0] = t->xMin;
    min[1] = t->yMin;
    max[0] = t->xMax;
    max[1] = t->yMax;

    Rectangle<2>* boxbT = new Rectangle<2>(true, min, max);

    if(noObjects == 0){
        this->box = *boxbT;
    } else {
        this->box = this->box.Union(*boxbT);
    }

    delete boxbT;

    assert(this->box.IsDefined());

    noObjects++;

}

void nodeCol::addObjectB(binaryTuple * t) {

    objectsB.push_back(t);

    noObjectsB++;
}

void nodeCol::addChild(nodeCol *child) {
    children.push_back(child);

    assert(child->box.IsDefined());

    if(noChildren == 0){
        this->box = child->box;
    } else {
        this->box = this->box.Union(child->box);
    }
    noChildren++;
    assert(this->box.IsDefined());
}

void nodeCol::addChildren(vector<nodeCol*> childrenV) {
    assert(noChildren == 0);

    int vectorSize = (int) childrenV.size();
    for (int i = 0; i < vectorSize; i++) {
        addChild(childrenV.at(i));
    }
}
