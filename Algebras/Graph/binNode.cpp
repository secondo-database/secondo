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

BinNode::BinNode() {}

BinNode::~BinNode() {}

BinNode::BinNode(BinNode* nleft, BinNode* nright, Edge nelem) {
    right = nright;
    left = nleft;
    elem = nelem;
    depth = 1;
}

/* 
Insert a new Edge in the tree, by walking throw the tree to find the right place.
Return true if the edge could be inserted

*/
bool BinNode::insertElem(Edge nelem) {
    // Identical Edge found, store in actual node only if cost are less
    if (elem.GetTarget() == nelem.GetTarget()) {
        if (elem.GetCost() > nelem.GetCost()) {
            elem = nelem;
            return true;
        }
        else
            return false;
    }
    // If edge not in actual node search right and left
    // for the right insert position

    // Try to insert at left son
    if (elem.GetTarget() > nelem.GetTarget())
        if (left == NULL) {
            left = new BinNode(NULL, NULL, nelem);
            if (right == NULL)
                depth++;
            return true;
        } else { // if actual left is in use, search deeper in the tree
            bool ret = (left)->insertElem(nelem);
            depth = max(depth, (left)->depth + 1);
            balance();
            return ret;
        }

    // Try to insert at right son
    else
        if (right == NULL) { 
            right = new BinNode(NULL, NULL, nelem);
            if (left == NULL)
                depth++;
            return true;
        }
        else { // if actual right is in use, search deeper in the tree
            bool ret = right->insertElem(nelem);
            depth = max(depth, right->depth + 1);
            balance();
            return ret;
        }
}

/*
Search for a given index of an edge to find, if so it return the cost of the edge,
otherwise it gives -1.0 back.
Used by the shortespath operator of the graph class this cost means the cost from
the start vertex to the end of this edge

*/
float BinNode::getCostOfEdge(int index) {
    if (elem.GetTarget() == index)
        return elem.GetCost();
    if (elem.GetTarget() > index)
        if (left == NULL)
            return -1.0f;
        else
            return left->getCostOfEdge(index);
    else
        if (right == NULL)
            return -1.0f;
        else
            return right->getCostOfEdge(index);
}

/*
Search for a given index of an edge to find and give it back, if it doesn't exist
an undefined edge will be given back.
Used by the shortespath operator of the graph class this cost means the cost from
the start vertex to the end of this edge

*/
Edge BinNode::getElemAt(int index) {
    if (elem.GetTarget() == index)
        return elem;
    if (elem.GetTarget() > index)
        if (left == NULL)
            return new Edge(false);
        else
            return left->getElemAt(index);
    else
        if (right == NULL)
            return new Edge(false);
        else
            return right->getElemAt(index);
}

/*
Main balance method, controls after a new node is inserted, if the tree must be 
rebalanced and if which kind of the following 4 balancekind must be used.

*/
void BinNode::balance() {
    int ri, le, riri, lele;
    ri = (right == NULL) ?  0 : right->depth;
    le = (left == NULL) ?  0 : left->depth;
    if (abs(ri - le) < 2) return;
    if (ri - le == 2) {
        riri = (right->right == NULL) ?  0 : (right->right)->depth;
        if (ri - riri == 1) balanceLeft();
        else balanceInnerRight();
    } else {
        lele = (left->left == NULL) ?  0 : (left->left)->depth;
        if (le - lele == 1) balanceRight();
        else balanceInnerLeft();
    }
}

void BinNode::balanceRight() {
    BinNode* node = new BinNode(left->right, right, elem);
    elem = left->elem;
    BinNode* del = left;
    left = left->left;
    delete del;
    right = node;
    deepen();
}

void BinNode::balanceInnerLeft() {
    BinNode* node = new BinNode(left->right->right, right, elem);
    elem = left->right->elem;
    BinNode* del = left->right;
    left->right = left->right->left;
    delete del;
    right = node;
    deepen();
}

void BinNode::balanceLeft() {
    BinNode* node = new BinNode(left, right->left,  elem);
    elem = right->elem;
    BinNode* del = right;
    right = right->right;
    delete del;
    left = node;
    deepen();
}

void BinNode::balanceInnerRight() {
    BinNode* node = new BinNode(left, right->left->left, elem);
    elem = right->left->elem;
    BinNode* del = right->left;
    right->left = right->left->right;
    delete del;
    left = node;
    deepen();
}

// Used after balancing keeping deep of nodes at actual state
void BinNode::deepen() {
    depth--;
    right->depth = depth - 1;
    left->depth = depth - 1;
}

// Used to destruct the tree from the leaves to the root
void BinNode::delNode() {
    if (right != NULL) {
        right->delNode();
        delete right;
    }
    if (left != NULL) {
        left->delNode();
        delete left;
    }
}
