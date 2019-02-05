/*
----
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
----

*/

#include "Algebras/RTree/RTreeAlgebra.h"
#include "RTreeTouch.h"
#include "NodeT.h"
#include <typeinfo>


using namespace mmrtreetouch;


RTreeTouch::RTreeTouch(
        TupleType* ttParam,
        int firstStreamWordIndex,
        int secondStreamWordIndex
        ) {
    tt = ttParam;
    outputOn = false;
    _firstStreamWordIndex = firstStreamWordIndex;
    _secondStreamWordIndex = secondStreamWordIndex;
}

RTreeTouch::~RTreeTouch() {

    // delete all Nodes
    deque<NodeT*> nodes;
    nodes = getNodesOfInnerNodeRecursive(root, nodes, false);

    for (NodeT* node :nodes) {
        delete node;
    }

    delete root;
}

NodeT* RTreeTouch::constructTree(deque<NodeT*> sortedArray, int fanout) {
    // sortedArray has the BucketNodes or Leaf Nodes

    bool firstTime = true;
    long levelCounter = 0;

    // here sortedArray is an Array with LeafNodes
    deque<deque<NodeT*> > Pa = reGroupByConsideringFanout(sortedArray, fanout);

    // create from the bottom to the top the tree
    while ((int)Pa.size() > 1 || firstTime) {
        firstTime = false;

        deque<NodeT*> nodes;
        levelCounter++;

        for (deque<NodeT *> group : Pa) {

            NodeT* parentNode = new NodeT();
            parentNode->addChildren(group);
            parentNode->level = levelCounter;

            assert(parentNode->noChildren == (int)parentNode->children.size());

            nodes.push_back(parentNode);
        }

        Pa = reGroupByConsideringFanout(nodes, fanout);
    }

    NodeT* rootNode = new NodeT();

    if ((int)Pa.size() == 1 && (int)Pa.at(0).size() == 1) {
        rootNode = Pa.front().front();
    } else if ((int)Pa.size() == 1) {
        rootNode->level = levelCounter + 1;
        rootNode->addChildren(Pa.front());

        assert(rootNode->noChildren == (int)rootNode->children.size());
    }

    root = rootNode; // root is a class property

    return rootNode;
}

// Returns as vector all concatenated Tuples of ObjectA and ObjectB that overlap
// In particular, it finds all leaf-Nodes (Objects of A) that overlap with given
// ObjectB.
deque<Tuple*> RTreeTouch::getTuplesOverlappingOnTreeWith(Tuple* tupleB) {

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    _secondStreamWordIndex
                    );

    Rectangle<2> boxB = attr1->BoundingBox();
    NodeT * parent = NULL, * temp = NULL;
    bool overlap;
    long numOfChildren;
    NodeT* p = root;
    deque<Tuple*> emptyVector;

    if (outputOn) {
        cout << "# tree level and num of Children: ";
        cout << (int) p->level;
        cout << " -- " << (int)p->children.size() << endl;
    }

    while (!p->isLeaf()) {
        overlap = false;
        numOfChildren = 0;

        for (NodeT* child : p->children) {
            //child = p->children.at(i);

            numOfChildren++;

            child->box.SetDefined(true);
            /*
             * danger, SetDefined(true) should not be added here,
             * fix the problem differently
             */

            assert(child->box.IsDefined());

            if (boxB.Intersects(child->box, 0)) {
                if (outputOn) {
                    cout <<
                    "##### current tree node is intersecting with TupleB"
                    << endl;
                    cout << "child i: " << " - level: " <<
                    child->level << endl;
                }

                if (overlap) {
                    if (parent != NULL) {
                        if (outputOn) {
                            cout << "##### tree 5" << endl;
                        }

                        parent->addObjectB(tupleB); /// this is the place
                        return joinPhase(parent, tupleB);
                    }
                    return emptyVector;
                } else {
                    if (outputOn) {
                        cout << "##### tree 6" << endl;
                    }

                    overlap = true;

                    temp = child;

                    parent = p;
                }

                if ((temp != NULL) && (numOfChildren == p->noChildren)) {
                    // in case only last child intersects
                    p = temp;
                    break;
                }

            } else if (
                    (temp != NULL) && overlap &&
                    (numOfChildren == p->noChildren)
                    )
            { // in case some of the first but not the last child intersects
                p = temp;
                break;
            }
        }

        if (!overlap) {
            return emptyVector;
        }
    }

    p->addObjectB(tupleB);
    return joinPhase(p, tupleB);
}

int RTreeTouch::noLeaves() {
    return noLeaves(root);
}

int RTreeTouch::noLeaves(NodeT* someNode) {

    if(!someNode){
        return 0;
    }
    if(someNode->isLeaf()){

        return 1;
    }else {
        int sum = 0;

        for(int i=0;i < (int)someNode->children.size(); i++){
            sum += noLeaves(someNode->children.at(i));
        }

        return sum;
    }
}


// Finds and returns as vector all leaf-Nodes that overlap with TupleB
deque<Tuple*> RTreeTouch::joinPhase(NodeT* node,Tuple* tupleB) {

    deque<NodeT*> leafNodes;
    leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

    return getMatchingConcatenatedTuples(tupleB, leafNodes);
}

deque<Tuple*> RTreeTouch::getMatchingConcatenatedTuples(
        Tuple* tupleB,
        deque<NodeT*> leafNodes
        )
    {

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    _secondStreamWordIndex
                    );
    Rectangle<2> boxB = attr1->BoundingBox();

    deque<Tuple*> matchingConcatenatedTuplesVector;

    for (NodeT* leafNode: leafNodes) {

        deque<Tuple*> objectsA = leafNode->objects;

        for (Tuple* tupleA: objectsA) {

            StandardSpatialAttribute<2> * attr1 =
                    (StandardSpatialAttribute<2>*) tupleA->GetAttribute(
                            _firstStreamWordIndex
                            );
            Rectangle<2> boxA = attr1->BoundingBox();

            if (boxA.Intersects(boxB, 0)) {

                Tuple* result = new Tuple(tt);

                Concat(tupleA, tupleB, result);

                if (outputOn) {
                    cout << " RTreeTouch.cpp: intersection exists with TupleA: "
                    << tupleA->GetAttribute(0)->toText()
                    << endl;
                }
                matchingConcatenatedTuplesVector.push_back(result);
            }

        }
    }

    return matchingConcatenatedTuplesVector;
}



deque<NodeT*> RTreeTouch::getNodesOfInnerNodeRecursive(
        NodeT* node,
        deque<NodeT*> nodes,
        bool justLeafNodes
        )
    {

        if (node->isLeaf()) {
            nodes.push_back(node);

            return nodes;
        }

        for (NodeT* child: node->children) {
            //NodeT* child = node->children.at(i);

            if (!justLeafNodes) {
                nodes.push_back(child);
            }

            nodes = getNodesOfInnerNodeRecursive(child , nodes);
        }

        return nodes;
}

deque<deque<NodeT*> > RTreeTouch::reGroupByConsideringFanout(
        deque<NodeT*> sortedArray,
        int fanout
        )
        { //you could change the name to ::groupNodesByConsideringFanout()
            deque<deque<NodeT*> > reGroupPartitions;

            int counter = 0;
            deque<NodeT*> innerContainer;

            for (NodeT* item: sortedArray) {
                counter++;
                innerContainer.push_back(item);

                if ((counter > 0) && (counter % fanout == 0)) {
                    reGroupPartitions.push_back(innerContainer);
                    innerContainer.clear();
                }

            }

            if (innerContainer.size() > 0) {
                reGroupPartitions.push_back(innerContainer);
            }

            return reGroupPartitions;
}

void RTreeTouch::showSubTreeInfo(NodeT* subRoot) {

    string infoStrOut;

    infoStrOut = recursiveInfo(subRoot);

    cout << infoStrOut << endl;
}

string RTreeTouch::recursiveInfo(NodeT* subRoot) {

    stringstream info;
    string infoStrOut;

    string type = "Node";
    if (subRoot->isLeaf()) {
        type = "LeafNode";
    }

    info << "(#" << type << "# - Level: " << subRoot->level <<
    " - NumChildren: "  << (int)subRoot->children.size() <<
    " - NumOfObjects: " << (int)subRoot->noObjects;

    if ((int)subRoot->children.size() > 0) {

        info << " [ ";

        for (int i=0; i < (int) subRoot->children.size(); i++) {


            infoStrOut = recursiveInfo(subRoot->children.at(i));

            info << infoStrOut;

        }

        info << " ]";

    }

    info <<  ")";

    return info.str();;
}