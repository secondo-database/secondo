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
#include "cmath"

using namespace std; 

/*
This class contains the nodes for the AVL Tree BinTree for the shortestpath
operator. It have the same operators like the BinTree. To be able to balance
the tree it uses 5 private methods to balance the tree and a method to keep
the depth of the nodes actual. At final destruction a method delNode runs 
throw the tree and deletes the nodes from the leaves up to the root.

*/
class BinNode {

    public:

        BinNode();
        ~BinNode();
        BinNode(BinNode* left, BinNode* right, Edge elem);

        bool insertElem(Edge elem);
        float getCostOfEdge(int at);
        Edge getElemAt(int at);
        void delNode();

    private:

        BinNode* right;
        BinNode* left;
        Edge elem;
        int depth;

        void deepen();

        void balance();
        void balanceRight();
        void balanceInnerRight();
        void balanceLeft();
        void balanceInnerLeft();
};

