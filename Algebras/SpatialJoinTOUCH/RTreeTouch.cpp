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
#include "Grid.h"
#include "RTreeTouch.h"

using namespace mmrtreetouch;
using namespace std;


RTreeTouch::RTreeTouch(
        TupleType* ttParam,
        int firstStreamWordIndex,
        int secondStreamWordIndex,
        int _cellFactor,
        bool _isM
):tt(ttParam),
  _firstStreamWordIndex(firstStreamWordIndex) ,
  _secondStreamWordIndex(secondStreamWordIndex),
  isM(_isM),
  cellFactor(_cellFactor)
{ outputOn = false; }

RTreeTouch::~RTreeTouch() {

    vector<tchNode*> nodes;

    if (root) {
        nodes = getNodesOfInnerNodeRecursive(root, nodes, false);
    }

    for (tchNode* node :nodes) {
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

tchNode * RTreeTouch::constructTree(
        vector<tchNode * > sortedArray,
        int fanout
        ) {

    bool firstTime = true;
    long levelCounter = 0;

    vector<vector<tchNode * > > Pa =
            reGroupByConsideringFanout(sortedArray, fanout);

    vector<tchNode * > nodes;

    // create from the bottom to the top the tree
    while ((int64_t)Pa.size() > 1 || firstTime) {
        firstTime = false;

        levelCounter++;

        for (vector<tchNode * > group : Pa) {

            tchNode* parentNode = new tchNode();
            parentNode->addChildren(group);
            parentNode->level = levelCounter;

            nodes.push_back(parentNode);
        }

        Pa.clear();
        Pa = reGroupByConsideringFanout(nodes, fanout);
        nodes.clear();
    }

    root = new tchNode();

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

vector<Tuple*> RTreeTouch::getTuplesOverlappingOnTreeWith(Tuple* tupleB) {

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    _secondStreamWordIndex
            );

    Rectangle<2> boxB = attr1->BoundingBox();
    tchNode * parent = NULL, * temp = NULL;
    bool overlap;
    long numOfChildren;
    tchNode * p = root;
    vector<Tuple*> emptyVector;

    while (!p->isLeaf()) {
        overlap = false;
        numOfChildren = 0;

        for (tchNode * child : p->children) {

            numOfChildren++;

            assert(child->box.IsDefined());

            if (boxB.Intersects(child->box, 0)) {

                if (overlap) {
                    if (parent != NULL) {

                        parent->addObjectB(tupleB);

                        return joinPhase(parent, tupleB);
                    }
                    return emptyVector;
                } else {
                    overlap = true;

                    temp = child;

                    parent = p;
                }

                if ((temp != NULL) && (numOfChildren == p->noChildren)) {
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

int64_t RTreeTouch::noLeaves() {
    return noLeaves(root);
}

int64_t RTreeTouch::noLeaves(tchNode * someNode) {

    if(!someNode){
        return 0;
    }
    if(someNode->isLeaf()){

        return 1;
    }else {
        int64_t sum = 0;

        for(int64_t i=0;i < (int64_t)someNode->children.size(); i++){
            sum += noLeaves(someNode->children.at(i));
        }

        return sum;
    }
}

vector<Tuple*> RTreeTouch::joinPhase(tchNode * node,Tuple* tupleB) {

    vector<tchNode * > leafNodes;
    leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

    return getMatchingConcatenatedTuples(tupleB, leafNodes);
}

vector<Tuple*> RTreeTouch::getMatchingConcatenatedTuples(
        Tuple* tupleB,
        vector<tchNode * > leafNodes
)
{

    vector<Tuple*> matchingConcatenatedTuplesVector;

    for (tchNode * leafNode: leafNodes) {

        vector<Tuple*> objectsA = leafNode->objects;

        for (Tuple* tupleA: objectsA) {

            if (checkIfOverlapping(tupleA, tupleB)) {

                Tuple* result = concatenateTuples(tupleA, tupleB);

                matchingConcatenatedTuplesVector.push_back(result);
            }

        }
    }

    return matchingConcatenatedTuplesVector;
}

vector<tchNode * > RTreeTouch::getNodesOfInnerNodeRecursive(
        tchNode * node,
        vector<tchNode *> nodes,
        bool justLeafNodes
)
{
    if (node->isLeaf()) {
        nodes.push_back(node);

        return nodes;
    }

    for (tchNode * child: node->children) {
        nodes = getNodesOfInnerNodeRecursive(child , nodes, justLeafNodes);
    }

    if (!justLeafNodes) {
        nodes.push_back(node);
    }

    return nodes;
}

vector<vector<tchNode * > > RTreeTouch::reGroupByConsideringFanout(
        vector<tchNode * > sortedArray,
        int fanout
)
{
    vector<vector<tchNode * > > reGroupPartitions;
    reGroupPartitions.reserve(fanout);

    int64_t counter = 0;
    vector<tchNode * > innerContainer;

    for (tchNode * item: sortedArray) {
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

void RTreeTouch::showSubTreeInfo(tchNode * subRoot) {

    string infoStrOut;

    infoStrOut = recursiveInfo(subRoot);

    cout << infoStrOut << endl;
}

string RTreeTouch::recursiveInfo(tchNode * subRoot) {

    stringstream info;
    string infoStrOut;

    string type = "tchNode";
    if (subRoot->isLeaf()) {
        type = "LeafNode";
    }


    info << "(#" << type << "# - Level: " << subRoot->level <<
         " - NumChildren: "  << (int64_t)subRoot->children.size() <<
         " - Num Tuples B : "  << (int64_t)subRoot->noObjectsB <<
         " - Num Tuples A : " << (int64_t)subRoot->noObjects;

    if ((int64_t)subRoot->children.size() > 0) {

        info << " [ ";

        for (int64_t i=0; i < (int64_t) subRoot->children.size(); i++) {


            infoStrOut = recursiveInfo(subRoot->children.at(i));

            info << infoStrOut;

        }

        info << " ]";

    }

    info <<  ")";

    return info.str();;
}

int64_t RTreeTouch::assignTupleB(Tuple* tupleB) {

    StandardSpatialAttribute<2> * attr1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    _secondStreamWordIndex
            );

    Rectangle<2> boxB = attr1->BoundingBox();
    tchNode * parent = NULL, * temp = NULL;
    bool overlap;
    long numOfChildren;
    tchNode * p = root;
    vector<Tuple*> emptyVector;

    while (!p->isLeaf()) {
        overlap = false;
        numOfChildren = 0;

        for (tchNode * child : p->children) {

            numOfChildren++;

            assert(child->box.IsDefined());
            assert(boxB.IsDefined());

            if (boxB.Intersects(child->box, 0)) {

                if (overlap) {
                    if (parent != NULL) {

                        parent->addObjectB(tupleB);
                        return 0;
                    }

                    return 0;
                } else {

                    overlap = true;

                    temp = child;

                    parent = p;
                }

                if ((temp != NULL) && (numOfChildren == p->noChildren)) {
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
            // in case that in one level there is no overlap
            return 0;
        }
    }

    p->addObjectB(tupleB);

    return 0;
}


vector<Tuple*> RTreeTouch::findMatchings() {

    vector<Tuple*> matchings;

    matchings =
            findMatchingsTopToBottomRecursWithGridFirstLeaves(root, matchings);

    return matchings;
}


vector<Tuple*> RTreeTouch::findMatchingsTopToBottomRecurs(
        tchNode * node,
        vector<Tuple*> matchings
        ) {

    if (node->noChildren > 0) {
        for (tchNode * child: node->children) {
            matchings = findMatchingsTopToBottomRecurs(child, matchings);
        }
    }

    if (node->noObjectsB > 0) {
        vector<Tuple*> Bs = node->objectsB;

        vector<tchNode * > leafNodes;
        leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

        for (Tuple* B: Bs) {
            for (tchNode * leafNode: leafNodes) {
                for (Tuple* A: leafNode->objects) {
                    if (checkIfOverlapping(A,B)) {
                        Tuple* res = concatenateTuples(A,B);
                        matchings.push_back(res);
                    }
                }
            }
        }
    }

    return matchings;
}



vector<Tuple*> RTreeTouch::findMatchingsTopToBottomRecursWithGridFirstLeaves(
        tchNode * node,
        vector<Tuple*> matchings
        ) {

    if (node->noObjectsB > 0) {
        vector<Tuple*> Bs = node->objectsB;

        vector<tchNode * > leafNodes;
        leafNodes = getNodesOfInnerNodeRecursive(node, leafNodes);

        pair<double, double> cellDimension
                                    = findAverageSizeOfTupleAs(leafNodes);

        Grid *grid = new Grid(
                node,
                cellDimension.first * cellFactor,
                cellDimension.second * cellFactor,
                _firstStreamWordIndex,
                _secondStreamWordIndex,
                tt
        );

        for (tchNode *leafNode: leafNodes) {
            for (Tuple *TupleA: leafNode->objects) {
                grid->addTuple(TupleA, _firstStreamWordIndex);
            }
        }

        grid->setMatchings(matchings);

        for (Tuple *tupleB: Bs) {
            grid->getTuplesOverlappingWith(
                    tupleB,
                    _secondStreamWordIndex
            );
        }

        matchings = grid->getMatchings();

        delete grid;

        leafNodes.clear();
    }

    for (tchNode * child: node->children) {

        matchings =
                findMatchingsTopToBottomRecursWithGridFirstLeaves(
                        child,
                        matchings
                );
    }

    return matchings;
}


pair<double, double> RTreeTouch::findAverageSize(vector<Tuple*> tuples) {

    double totalXCellDim = 0;
    double totalYCellDim = 0;

    for (Tuple* tuple: tuples) {
        Attribute* attr = tuple->GetAttribute(_secondStreamWordIndex);

        double xCellDim = attr->getMaxX() - attr->getMinX();
        double yCellDim = attr->getMaxY() - attr->getMinY();

        totalXCellDim += xCellDim;
        totalYCellDim += yCellDim;
    }

    int64_t totalNum = tuples.size();

    assert(totalNum != 0);

    double avgXCellDim = totalXCellDim / totalNum;
    double avgYCellDim = totalYCellDim / totalNum;

    return make_pair(avgXCellDim, avgYCellDim);
}

pair<double, double> RTreeTouch::findAverageSizeOfTupleAs(
                                        vector<tchNode*> leafNodes
                                        ) {

    double totalXCellDim = 0;
    double totalYCellDim = 0;
    long counter = 0;

    for (tchNode* leafNode:leafNodes ) {
        int64_t incr = leafNode->noObjects * 0.1;
        incr = incr == 0 ? 1: incr;
        for (int64_t i = 0; i < leafNode->noObjects; i += incr) {

            Attribute* attr = leafNode->objects[i]->GetAttribute(
                                                        _firstStreamWordIndex
                                                        );

            double xCellDim = attr->getMaxX() - attr->getMinX();
            double yCellDim = attr->getMaxY() - attr->getMinY();

            totalXCellDim += xCellDim;
            totalYCellDim += yCellDim;

            counter++;
        }
    }

    long totalNum = counter;

    assert(totalNum != 0);

    double avgXCellDim = totalXCellDim / totalNum;
    double avgYCellDim = totalYCellDim / totalNum;


    return make_pair(avgXCellDim, avgYCellDim);
}

bool RTreeTouch::checkIfOverlapping(Tuple* tupleA, Tuple* tupleB) {

    StandardSpatialAttribute<2> * attrA1 =
            (StandardSpatialAttribute<2>*) tupleA->GetAttribute(
                    _firstStreamWordIndex
            );

    Rectangle<2> boxA = attrA1->BoundingBox();

    StandardSpatialAttribute<2> * attrB1 =
            (StandardSpatialAttribute<2>*) tupleB->GetAttribute(
                    _secondStreamWordIndex
            );

    Rectangle<2> boxB = attrB1->BoundingBox();

    if (boxA.Intersects(boxB, 0)) {
        return true;
    }

    return false;
}

Tuple* RTreeTouch::concatenateTuples(Tuple* tupleA, Tuple* tupleB) {
    Tuple* result = new Tuple(tt);
    Concat(tupleA, tupleB, result);

    return result;
};
