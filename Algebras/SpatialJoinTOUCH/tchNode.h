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

#ifndef SECONDO_tchNode_H
#define SECONDO_tchNode_H

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

    class tchNode {
    private:
        bool is_Leaf;

    public:
        int level;
        Rectangle<2> box;
        long noChildren;
        long noObjects;
        long noObjectsB;
        std::vector<tchNode*> children;
        std::vector<Tuple*> objects;
        std::vector<Tuple*> objectsB;

        tchNode(bool isLeafNode=false);

        ~tchNode();

        void addChild(tchNode *child);

        void addChildren(std::vector<tchNode*> childrenV);

        bool isLeaf();

        void addObject(Tuple *t, int leftStreamWordIndex);

        void addObjectB(Tuple *t);
    };

}

#endif //SECONDO_tchNode_H
