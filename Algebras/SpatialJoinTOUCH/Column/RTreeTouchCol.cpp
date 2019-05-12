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

#include "RTreeTouchCol.h"
#include <vector>
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "nodeCol.h"
#include "GridVectorCol.h"


using namespace mmrtreetouch;
using namespace std;

RTreeTouchCol::RTreeTouchCol(
        TupleType* ttParam,
        int firstStreamWordIndex,
        int secondStreamWordIndex,
        int _cellFactor
):tt(ttParam),
  _firstStreamWordIndex(firstStreamWordIndex) ,
  _secondStreamWordIndex(secondStreamWordIndex),
  cellFactor(_cellFactor)
{}

RTreeTouchCol::~RTreeTouchCol() {

    vector<nodeCol*> nodes;

    if (root) {
        nodes = getNodesOfInnerNodeRecursive(root, nodes, false);
    }
    // the root is included in the vector

    for (nodeCol* node :nodes) {
        if (node) {
            node->objectsB.clear();
            node->objects.clear();
            node->children.clear();
            delete node;
            node = nullptr;
        }
    }

    nodes.clear();

    root = nullptr;

}

nodeCol * RTreeTouchCol::constructTree(
        vector<nodeCol * > sortedArray, int fanout) {

    bool firstTime = true;
    long levelCounter = 0;

    vector<vector<nodeCol * > > Pa =
            reGroupByConsideringFanout(sortedArray, fanout);

    vector<nodeCol * > nodes;

    int sizeOfPa = (int)Pa.size();
    while ( sizeOfPa > 1 || firstTime) {
        firstTime = false;

        levelCounter++;

        for (vector<nodeCol * > group : Pa) {

            nodeCol* parentNode = new nodeCol();
            parentNode->addChildren(group);
            parentNode->level = levelCounter;

            assert(parentNode->noChildren == (int)parentNode->children.size());

            nodes.push_back(parentNode);
        }

        Pa.clear();
        Pa = reGroupByConsideringFanout(nodes, fanout);
        sizeOfPa = (int)Pa.size();
        nodes.clear();

    }

    root = new nodeCol();

    if ((int)Pa.size() == 1 && (int)Pa.at(0).size() == 1) {
        delete root;
        root = Pa.front().front();
    } else if ((int)Pa.size() == 1) {
        root->level = levelCounter + 1;
        root->addChildren(Pa.front());

        assert(root->noChildren == (int)root->children.size());
    }

    return root;
}

vector<vector<nodeCol * > > RTreeTouchCol::reGroupByConsideringFanout(
        vector<nodeCol * > sortedArray,
        int fanout
)
{
    vector<vector<nodeCol * > > reGroupPartitions;
    reGroupPartitions.reserve(fanout);

    int counter = 0;
    vector<nodeCol * > innerContainer;

    for (nodeCol * item: sortedArray) {
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

void RTreeTouchCol::removeBsFromTree(nodeCol * node) {

    node->objectsB.clear();

    for (nodeCol * child: node->children) {
        removeBsFromTree(child);
    }
}


int RTreeTouchCol::assignTupleBs(binaryTuple * bt) {

    double min[2];
    double max[2];
    min[0] = bt->xMin;
    min[1] = bt->yMin;
    max[0] = bt->xMax;
    max[1] = bt->yMax;

    Rectangle<2>* boxB = new Rectangle<2>(true, min, max);

    nodeCol * parent = NULL, * temp = NULL;
    bool overlap;
    long numOfChildren;
    nodeCol * p = root;

    while (!p->isLeaf()) {

        overlap = false;
        numOfChildren = 0;

        for (nodeCol * child : p->children) {


            numOfChildren++;

            assert(child->box.IsDefined());
            assert(boxB->IsDefined());

            if (boxB->Intersects(child->box, 0)) {
                if (overlap) {
                    if (parent != NULL) {
                        parent->addObjectB(bt);
                        delete boxB;
                        return 0;
                    }
                    delete boxB;
                    return 0;
                } else {

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
            delete boxB;
            return 0;
        }
    }

    p->addObjectB(bt);
    delete boxB;
    return 0;
}

vector<pair<binaryTuple , binaryTuple >> RTreeTouchCol::findMatchings() {

    vector<pair<binaryTuple , binaryTuple >> matchings;
    matchings = findMatchingsRecurvGrid(root, matchings);

    return matchings;
}

vector<pair<binaryTuple, binaryTuple>>
RTreeTouchCol::findMatchingsRecurvGrid(
        nodeCol * node,
        vector<pair<binaryTuple, binaryTuple>> matchings
        ) {

    if (node->noChildren > 0) {
        for (nodeCol * child: node->children) {
            matchings = findMatchingsRecurvGrid(child, matchings);
        }
    }

    if (node->noObjectsB > 0) {
        vector<binaryTuple *> Bs = node->objectsB;

        vector<nodeCol *> leafNodes;
        leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

        pair<double, double> cellDimension =
                findAverageSizeOfTupleAs(leafNodes);

        GridVectorCol* grid = new GridVectorCol(
                node,
                cellDimension.first * cellFactor,
                cellDimension.second * cellFactor
        );

        for (nodeCol* leafNode: leafNodes) {
            for (binaryTuple * btA: leafNode->objects) {
                grid->addTuple(btA);
            }
        }

        for (binaryTuple * btB: Bs) {
            matchings = grid->getTuplesOverlappingWith(btB, matchings);
        }

        leafNodes.clear();

        delete grid;
    }



    return matchings;
}

vector<nodeCol * > RTreeTouchCol::getNodesOfInnerNodeRecursive(
        nodeCol * node,
        vector<nodeCol *> nodes,
        bool justLeafNodes
)
{
    if (node->isLeaf()) {
        nodes.push_back(node);

        return nodes;
    }

    for (nodeCol * child: node->children) {
        nodes = getNodesOfInnerNodeRecursive(child , nodes, justLeafNodes);
    }

    if (!justLeafNodes) {
        nodes.push_back(node);
    }

    return nodes;
}

pair<double, double>
        RTreeTouchCol::findAverageSizeOfTupleAs(vector<nodeCol*> leafNodes) {

    double totalXCellDim = 0;
    double totalYCellDim = 0;
    long counter = 0;

    for (nodeCol* leafNode:leafNodes ) {

        for (binaryTuple * bt: leafNode->objects) {

            double xCellDim = bt->xMax - bt->xMin;
            double yCellDim = bt->yMax - bt->yMin;

            totalXCellDim += xCellDim;
            totalYCellDim += yCellDim;

            counter++;
        }
    }

    long totalNum = counter;

    assert(totalNum > 0);

    double avgXCellDim = totalXCellDim / totalNum;
    double avgYCellDim = totalYCellDim / totalNum;


    return make_pair(avgXCellDim, avgYCellDim);
}

int RTreeTouchCol::noLeaves() {
    return noLeaves(root);
}

int RTreeTouchCol::noLeaves(nodeCol * someNode) {

    if(!someNode){
        return 0;
    }
    if(someNode->isLeaf()){

        return 1;
    }else {
        int sum = 0;

        int sizeOfChildren = (int)someNode->children.size();
        for(int i=0;i < sizeOfChildren; i++){
            sum += noLeaves(someNode->children.at(i));
        }

        return sum;
    }
}

void RTreeTouchCol::showSubTreeInfo(nodeCol * subRoot) {

    string infoStrOut;

    infoStrOut = recursiveInfo(subRoot);

    cout << infoStrOut << endl;
}

string RTreeTouchCol::recursiveInfo(nodeCol * subRoot) {

    stringstream info;
    string infoStrOut;

    string type = "nodeCol";
    if (subRoot->isLeaf()) {
        type = "LeafNode";
    }


    info << "(#" << type << "# - Level: " << subRoot->level <<
         " - NumChildren: "  << (int)subRoot->children.size() <<
         " - Num Tuples B : "  << (int)subRoot->noObjectsB <<
         " - Num Tuples A : " << (int)subRoot->noObjects <<
         " - MBR of Node : xMin: " << (int)subRoot->box.MinD(0) <<
         "- xMax: " <<  (int)subRoot->box.MaxD(0) << "- yMin: " <<
         (int)subRoot->box.MinD(1) << "- yMax: " <<  (int)subRoot->box.MaxD(1);

    if ((int)subRoot->children.size() > 0) {

        info << " [ ";

        int sizeOfChildren = (int) subRoot->children.size();
        for (int i=0; i < sizeOfChildren; i++) {


            infoStrOut = recursiveInfo(subRoot->children.at(i));

            info << infoStrOut;

        }

        info << " ]";

    }

    info <<  ")";

    return info.str();;
}
