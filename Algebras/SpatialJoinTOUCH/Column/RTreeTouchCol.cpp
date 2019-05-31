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
        int _cellFactor,
        int64_t &_remainingMem
):
  cellFactor(_cellFactor),
  remainingMem(_remainingMem)
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

    int64_t sizeOfPa = (int64_t)Pa.size();
    while ( sizeOfPa > 1 || firstTime) {
        firstTime = false;

        levelCounter++;

        for (vector<nodeCol * > group : Pa) {

            if (remainingMem-sizeof(nodeCol) <= 0 ) {
                cout << "Memory is not enough 7" << endl;
                remainingMem -= sizeof(nodeCol);
                return nullptr;
            }

            nodeCol* parentNode = new nodeCol();
            parentNode->addChildren(group);
            parentNode->level = levelCounter;

            remainingMem -= sizeof(nodeCol);

            nodes.push_back(parentNode);
        }

        Pa.clear();
        Pa = reGroupByConsideringFanout(nodes, fanout);
        sizeOfPa = (int64_t)Pa.size();
        nodes.clear();

    }

    if (remainingMem-sizeof(nodeCol) <= 0 ) {
        cout << "Memory is not enough 8" << endl;
        return 0;
    }

    root = new nodeCol();

    remainingMem -= sizeof(nodeCol);

    if ((int64_t)Pa.size() == 1 && (int64_t)Pa.at(0).size() == 1) {
        delete root;
        root = Pa.front().front();
    } else if ((int64_t)Pa.size() == 1) {
        root->level = levelCounter + 1;
        root->addChildren(Pa.front());

        assert(root->noChildren == (int64_t)root->children.size());
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

    int64_t counter = 0;
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


int64_t RTreeTouchCol::assignTupleBs(tupleBlockStr bt) {

    double min[2];
    double max[2];
    min[0] = bt.xMin;
    min[1] = bt.yMin;
    max[0] = bt.xMax;
    max[1] = bt.yMax;

    if (remainingMem-sizeof(Rectangle<2>) <= 0 ) {
        cout << "Memory is not enough 9" << endl;
        remainingMem -= sizeof(Rectangle<2>);
        return 0;
    }

    Rectangle<2>* boxB = new Rectangle<2>(true, min, max);

    remainingMem -= sizeof(Rectangle<2>);

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

vector<pair<tupleBlockStr , tupleBlockStr >> RTreeTouchCol::findMatchings() {

    findMatchingsRecurvGrid(root);

    return matchings;
}

void RTreeTouchCol::findMatchingsRecurvGrid(
        nodeCol * node
        ) {

    if (remainingMem <= 0) {
        //return matchings;
    }

    if (node->noObjectsB > 0) {
        vector<tupleBlockStr> Bs = node->objectsB;

        vector<nodeCol *> leafNodes;
        leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

        pair<double, double> cellDimension =
                findAverageSizeOfTupleAs(leafNodes);

        if (remainingMem-sizeof(GridVectorCol) <= 0 ) {
            cout << "Memory is not enough 6" << endl;
            remainingMem -= sizeof(GridVectorCol);
            //return matchings;
        }

        GridVectorCol* grid = new GridVectorCol(
                node,
                cellDimension.first * cellFactor,
                cellDimension.second * cellFactor,
                remainingMem
        );

        remainingMem -= sizeof(GridVectorCol);

        for (nodeCol* leafNode: leafNodes) {
            for (tupleBlockStr btA: leafNode->objects) {
                grid->addTuple(btA);
            }
        }

        grid->setMatchings(matchings);

        for (tupleBlockStr btB: Bs) {
            grid->getTuplesOverlappingWith(btB);

            remainingMem = grid->remainingMem;

            if (remainingMem <= 0) {
                cout << "getTuplesOverlappingWith" << endl;
                //return matchings;
            }
        }

        matchings = grid->getMatchings();

        delete grid;

        leafNodes.clear();
    }

    for (nodeCol * child: node->children) {
        findMatchingsRecurvGrid(child);
    }

    //return matchings;
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
        int64_t incr = leafNode->noObjects * 0.1;
        incr = incr == 0 ? 1: incr;
        for (int64_t i = 0; i < leafNode->noObjects; i += incr) {
            tupleBlockStr bt = leafNode->objects[i];

            double xCellDim = bt.xMax - bt.xMin;
            double yCellDim = bt.yMax - bt.yMin;

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
