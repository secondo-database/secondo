/*
---- 
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
----

*/
#include "GraphAlgebra.h"
#include "binNode.cpp"

/*
This class stores edges in an AVL tree. To shorten the sourcecode this class
can only insert and give back edges or the cost of an edge. It have a special
destructor, which delete all nodes and the tree himself. It is used in the graphalgebra by the shortestpath operator

*/

class BinTree {

    public:

        BinTree();
        ~BinTree();

        bool insertElem(Edge elem);

        float getCostOfEdge(int at);

        Edge getElemAT(int at);

    private:

        BinNode* root;
};

BinTree::BinTree() { root = NULL; }

BinTree::~BinTree() {
    if (root != NULL) {
        root->delNode();
        delete root;
    }
}

bool BinTree::insertElem(Edge elem) {
    if (root == NULL) {
        root = new BinNode(NULL, NULL, elem);
        return true;
    } else
        return root->insertElem(elem);
}

float BinTree::getCostOfEdge(int index) {
    if (root == NULL)
        return -1.0f;
    else
        return root->getCostOfEdge(index);
}

Edge BinTree::getElemAT(int at) {
    if (root == NULL)
        return new Edge(false);
    else
        return root->getElemAt(at);
}
