/*
This file is part of SECONDO.

Copyright (C) 2018,
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

#ifndef ITNode_H_
#define ITNode_H_

#include "BinaryTuple.h"
#include "sort.h"
#include <iostream>
#include <vector>

using namespace std;

namespace csj {

  // structure to reprsent a node in IntSTree
  struct ITNode {
    vector<binaryTuple> vtR;
    vector<binaryTuple> vtL;
    vector<binaryTuple> outL;
    vector<binaryTuple> outR;
    double xLeft;
    double xRight;
    double xMedian;
    int height;
    ITNode *left;
    ITNode *right;
    bool rootIsDone;
    bool leftIsDone;
    bool rightIsDone;
  };

  // function deletes a tree
  void deleteTree(ITNode *root) {

    if(root == nullptr) {
      return;
    }
    
    deleteTree(root->left);
    deleteTree(root->right);

    root->vtL.clear();
    root->vtR.clear();
    root->outL.clear();
    root->outR.clear();
    delete root;
    root = nullptr;
  }

  // function to get height of the tree rooted with root
  int getHeight(ITNode *root) {

    if(root == nullptr) {
      return 0;
    }
    
    return root->height;
  }

  // function to get maximum of two integers
  int max(int x, int y) {
    return x > y ? x : y;
  }

  // function to create a new ITNode
  ITNode *newNode(double xLeft_, double xRight_) {
    
    ITNode *node = new ITNode;

    node->xLeft = xLeft_;
    node->xRight = xRight_;
    node->xMedian = (xRight_ + xLeft_) / 2; 
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    node->rootIsDone = false;
    node->leftIsDone = false;
    node->rightIsDone = false;

    return node;    
  }

  // function to right rotate subtree rooted with root
  ITNode *rightRotate(ITNode *root) {
    
    ITNode *temp1 = root->left;
    ITNode *temp2 = temp1->right;

    // perform rotation
    temp1->right = root;
    root->left = temp2;

    // update heights
    root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
    temp1->height = max(getHeight(temp1->left), getHeight(temp1->right)) + 1;

    // return new root
    return temp1;
  }

  // function to left rotate subtree rooted with root
  ITNode *leftRotate(ITNode *root) {

    ITNode *temp1 = root->right;
    ITNode *temp2 = temp1->left;

    // perform rotation
    temp1->left = root;
    root->right = temp2;

    // update heights
    root->height = max(getHeight(root->left), getHeight(root->right)) + 1;
    temp1->height = max(getHeight(temp1->left), getHeight(temp1->right)) + 1;

    // return new root
    return temp1;
  }

  // function to get balance of node
  int getBalance(ITNode *node) {

    if(node == nullptr) {
      return 0;
    }
    
    return getHeight(node->left) - getHeight(node->right);
  }

  // function to insert a interval in the subtree rooted with root
  ITNode *insert(ITNode *root, double xLeft_, double xRight_) {

    if(root == nullptr) {
      return newNode(xLeft_, xRight_);
    }

    if(xLeft_ < root->xLeft) {
      root->left = insert(root->left, xLeft_, xRight_);
    }
    else
      if(xLeft_ > root->xLeft) {
        root->right = insert(root->right, xLeft_, xRight_);
      }
      else {
        return root;
      }

    // update height
    root->height = 1 + max(getHeight(root->left), getHeight(root->right));

    int balance = getBalance(root);

    // left left case
    if((balance > 1) && (xLeft_ < root->left->xLeft))
      return rightRotate(root);

    // right right case
    if((balance < -1) && (xLeft_ > root->right->xLeft))
      return leftRotate(root);

    // left right case
    if((balance > 1) && (xLeft_ > root->left->xLeft)) {
      root->left = leftRotate(root->left);
      return rightRotate(root);
    }

    // right left case
    if((balance < -1) && (xLeft_ < root->right->xLeft)) {
      root->right = rightRotate(root->right);
      return leftRotate(root);
    }

    return root;
  } // end of insert

  // function to create static interval tree
  ITNode *createITree(ITNode *root, double xMin, double xMax,
                      uint64_t numOfIntervals) {

    double tempXLeft;
    double tempXRight;

    for(uint64_t i = 0; i < numOfIntervals; i++) {
      tempXLeft = xMin + (xMax - xMin)/numOfIntervals*i;
      tempXRight = xMin + (xMax - xMin)/numOfIntervals*(i + 1);

      root = insert(root, tempXLeft, tempXRight);        
    }

    return root;
  }

  // function to insert binary tuple
  void sweepPush(ITNode *root, binaryTuple bt) {

    if(root == nullptr) {
      return;
    }

    if(bt.xMin <= root->xMedian && bt.xMax >= root->xMedian) {
      root->vtL.push_back(bt);
      root->vtR.push_back(bt);
      MergeSortXLeft(root->vtL);
      MergeSortXRight(root->vtR);
      return;
    }
    else {
      if(bt.xMax < root->xMedian) {
        if(root->left != nullptr) {
          sweepPush(root->left, bt);
          return;
        }
        else {
          root->outL.push_back(bt);
          //MergeSortNode(root->outL);
          return;
        }
      }
      if(bt.xMin > root->xMedian) {
        if(root->right != nullptr) {
          sweepPush(root->right, bt);
          return;
        }
        else {
          root->outR.push_back(bt);
         // MergeSortNode(root->outR);
          return;
        }
      }
    }
  } // end of sweepPush

  void removeFromOutL(ITNode *node, binaryTuple bt) {

    vector<binaryTuple> temp;

    for(uint64_t i = 0; i < node->outL.size(); i++) {
      if(!tuplesAreEqual(bt, node->outL[i])) {
        temp.push_back(node->outL[i]);
      }
    }
    node->outL.clear();
    node->outL = temp;
    temp.clear();

  } // end of removeFRomOutL

  void removeFromOutR(ITNode *node, binaryTuple bt) {

    vector<binaryTuple> temp;

    for(uint64_t i = 0; i < node->outR.size(); i++) {
      if(!tuplesAreEqual(bt, node->outR[i])) {
        temp.push_back(node->outR[i]);
      }
    }
    node->outR.clear();
    node->outR = temp;
    temp.clear();

  } // end of removeFRomOutR

  void removeFromLR(ITNode *node, binaryTuple bt) {

    vector<binaryTuple> temp;

    for(uint64_t i = 0; i < node->vtL.size(); i++) {
      if(!tuplesAreEqual(bt, node->vtL[i])) {
        temp.push_back(node->vtL[i]);
      }
    }
    node->vtL.clear();
    node->vtL = temp;
    temp.clear();

    for(uint64_t i = 0; i < node->vtR.size(); i++) {
      if(!tuplesAreEqual(bt, node->vtR[i])) {
        temp.push_back(node->vtR[i]);
      }
    }
    node->vtR.clear();
    node->vtR = temp;
    temp.clear();
    
  } // end of removeFromLR

  // function to remove binary tuple from tree
  void sweepRemove(ITNode *root, binaryTuple bt) {

    if(root == nullptr) {
      return;
    }

    // tuple cut the median by actually root
    if(bt.xMin <= root->xMedian && bt.xMax >= root->xMedian) {
      removeFromLR(root, bt);
      return;
    }
    else {
      // search tuple lies on the left from median
      if(bt.xMax < root->xMedian) {
        if(root->left != nullptr) {
          sweepRemove(root->left, bt);
          return;
        }
        else {
          removeFromOutL(root, bt);
          return;
        }
      } // end of left
      
      // search tuple lies on the right from median
      if(bt.xMin > root->xMedian) {
        if(root->right != nullptr) {
          sweepRemove(root->right, bt);
          return;
        }
        else {
          removeFromOutR(root, bt);
          return;
        }
      } // end of right
    } // end of else

  } // end of sweepRemove

  // display tree from left to right (root at left)
  void display(ITNode *node, int tab) {
    
    if(node != nullptr) {
      display(node->right, tab + 4);
      
      for(int i=0; i<tab; i++) {
        cout<<"  ";
      }
      
      cout<<node->xMedian<<endl;
      
      display(node->left, tab + 4);
    }
  } // end of display

  void stateCleaning(ITNode *node) {

    if(node == nullptr) {
      return;
    }

    node->rootIsDone = false;
    node->leftIsDone = false;
    node->rightIsDone = false;

    stateCleaning(node->left);
    stateCleaning(node->right);
  } // end of stateCleaning
  
} // end of namespace csj

#endif // ITNode_H_
