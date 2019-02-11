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

#include "Algebras/RTree/RTreeAlgebra.h"
#include "RTreeTouch.h"
#include <typeinfo>

using namespace mmrtreetouch;
using namespace std;

NodeT::NodeT(long maxPar, bool isLeafPar) {
    max = maxPar;
    noChildren = 0;
    noObjects = 0;
    boolean_is_Leaf = isLeafPar;
}

bool NodeT::addChild(NodeT *child) {
    assert(noChildren <= max);
    children.push_back(child);

    assert(child->box.IsDefined());

    if(noChildren == 0){ // adding the first child
        this->box = child->box;
    } else {
        this->box = this->box.Union(child->box);
    }
    noChildren++;
    assert(this->box.IsDefined());

    return noChildren <= max;
}

void NodeT::addChildren(vector<NodeT*> childrenV) {
    assert(noChildren == 0);

    for (int i = 0; i < (int) childrenV.size(); i++) {
        addChild(childrenV.at(i));
    }
}

bool NodeT::isLeaf() {
    return boolean_is_Leaf;
}

bool NodeT::addObject(Tuple *t, int leftStreamWordIndex) {

    assert(noObjects <= max);
    objects.push_back(t);

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) t->GetAttribute(leftStreamWordIndex);

    assert(attr1->BoundingBox().IsDefined());


    if(noObjects == 0){
        this->box = attr1->BoundingBox();
    } else {
        this->box = this->box.Union(attr1->BoundingBox());
    }

    assert(this->box.IsDefined());

    noObjects++;

    return noObjects <= max;
}

bool NodeT::addObjectB(Tuple *t) {

    objectsB.push_back(t);

    noObjectsB++;

    return noObjectsB <= max;
}