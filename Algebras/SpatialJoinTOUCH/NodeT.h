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

#ifndef SECONDO_NODET_H
#define SECONDO_NODET_H

#include <string.h>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <stack>

#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace mmrtreetouch {

    class NodeT {
    private:
        bool boolean_is_Leaf;

    public:
        Rectangle<2> box;    // the bounding box
        long level;         // level of node in tree
        long max;            // maximum number of children  
        long noChildren;     // current number of children
        long noObjects;      // current number of objects
        long noObjectsB;      // current number of objects
        std::vector<NodeT*> children;    // array of children
        std::vector<Tuple*> objects;     // array of objects
        std::vector<Tuple*> objectsB;     // array of objects

        NodeT(long maxPar = 99999, bool isLeafPar=false);

        ~NodeT() {}

        bool addChild(NodeT *child);

        void addChildren(std::vector<NodeT*> childrenV);

        bool isLeaf();

        bool addObject(Tuple *t, int leftStreamWordIndex);

        bool addObjectB(Tuple *t);
    };

}

#endif //SECONDO_NODET_H
