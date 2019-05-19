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
#include "tchNode.h"
#include <typeinfo>

using namespace mmrtreetouch;
using namespace std;

tchNode::tchNode(bool isLeafNode):
    is_Leaf(isLeafNode),
    noChildren(0),
    noObjects(0),
    noObjectsB(0)
    {}

tchNode::~tchNode() {
}

void tchNode::addChild(tchNode *child) {
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

void tchNode::addChildren(vector<tchNode*> childrenV) {
    assert(noChildren == 0);

    int64_t vectorSize = (int64_t) childrenV.size();
    for (int64_t i = 0; i < vectorSize; i++) {
        addChild(childrenV.at(i));
    }
}

bool tchNode::isLeaf() {
    return is_Leaf;
}

void tchNode::addObject(Tuple *t, int leftStreamWordIndex) {

    objects.push_back(t);

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) t->GetAttribute(leftStreamWordIndex);

    if(noObjects == 0){
        this->box = attr1->BoundingBox();
    } else {
        this->box = this->box.Union(attr1->BoundingBox());
    }

    assert(this->box.IsDefined());

    noObjects++;
}

void tchNode::addObjectB(Tuple *t) {

    objectsB.push_back(t);

    noObjectsB++;
}