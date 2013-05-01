/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/
#ifndef _AVL_Tree_CPP
#define _AVL_Tree_CPP

#include "AVL_Tree.h"
#include <cmath>
#include <vector>

namespace p2d {

bool p2d_debug = false;

/*
 Constructor and Deconstructor

*/
AVLTree::AVLTree() {
 root = NULL;
}

AVLTree::~AVLTree() {
 delete root;
}

/*
 ~insert~

*/
void AVLTree::insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc) {
 pred = NULL;
 suc = NULL;
 if (root == NULL) {
  root = new AVLNode(x);
 } else {
  root = root->insert(x, pred, suc);
 }
}

/*
 ~removeGetNeighbor~

*/
void AVLTree::removeGetNeighbor(AVLSegment* x, AVLSegment*& pred,
  AVLSegment*& suc) {
 pred = NULL;
 suc = NULL;
 if (root != NULL) {
  root = root->removeGetNeighbor(x, pred, suc);
 }
}

/*
 ~removeGetNeighbor2~

*/
void AVLTree::removeGetNeighbor2(AVLSegment* x, int gridXPos,
  mpq_class& preciseXPos, AVLSegment*& pred, AVLSegment*& suc) {
 pred = NULL;
 suc = NULL;
 if (root != NULL) {
  root = root->removeGetNeighbor2(x, gridXPos, preciseXPos, pred, suc);
 }
}

/*
 ~removeInvalidSegment~

*/
void AVLTree::removeInvalidSegment(AVLSegment* x, int gridXPos,
  mpq_class& preciseXPos) {
 bool found = false;
 if (root != NULL) {
  root = root->removeInvalidSegment(x, gridXPos, preciseXPos, found);
  assert(found);
 }

}

/*
 ~invertSegments~

*/
void AVLTree::invertSegments(vector<AVLSegment*>& v, int gridX, mpq_class& pX,
  int gridY, mpq_class& pY, AVLSegment*& pred, size_t predIndex,
  AVLSegment*& suc, size_t sucIndex) {
 pred = NULL;
 suc = NULL;
 vector<AVLNode*>* nodeVector = new vector<AVLNode*>();
 bool found = false;
 AVLNode* node = NULL;
 AVLNode* lastNode = NULL;
 AVLSegment* tempSeg1;
 AVLSegment* tempSeg2;
 for (size_t j = 0; j < v.size(); j++) {
  if (j == predIndex) {
   node = root->memberPlusNeighbor(v[j], found, gridX, pX, gridY, pY, pred,
     true);
  } else {
   if (j == sucIndex) {
    node = root->memberPlusNeighbor(v[j], found, gridX, pX, gridY, pY, suc,
      false);
   } else {
    node = root->member(v[j], found, gridX, pX, gridY, pY);
   }
  }

  if (found) {
   if (j > 0 && lastNode->getElement()->isParallelTo(*(node->getElement()))) {
    //2 segments, which run on the same line have
    //to be in the same order after the inversion
    //as before. Now they will be switched in the
    //wrong order, and after the inversion they
    //once again have the right order.
    lastNode = nodeVector->back();
    nodeVector->pop_back();
    tempSeg1 = node->getElement();
    tempSeg2 = lastNode->getElement();
    node->setElement(&tempSeg2);
    lastNode->setElement(&tempSeg1);

    nodeVector->push_back(node);
    nodeVector->push_back(lastNode);
   } else {
    nodeVector->push_back(node);
    lastNode = node;
   }
  } else {
   cerr << "Element not found." << endl;
   cerr << "The tree:" << endl << endl;
   root->inorder();
   if (p2d_debug) {
    cerr << "The following segments has to be inverted:" << endl;
    size_t i = 0;
    while (i < v.size()) {
     v[i]->print();
     i++;
    }
   }
   cerr << "This segment could not be found: " << endl;
   v[j]->print();
   assert(false);
  }
 }

 for (size_t k = 0; k < ((nodeVector->size()) / 2); k++) {
  tempSeg1 = (nodeVector->at(k))->getElement();
  tempSeg2 = nodeVector->at(nodeVector->size() - k - 1)->getElement();

  nodeVector->at(k)->setElement(&tempSeg2);
  nodeVector->at(nodeVector->size() - k - 1)->setElement(&tempSeg1);
 }

}

/*
 ~inorder~

*/
void AVLTree::inorder() {
 if (root != NULL) {
  root->inorder();
 }
}

/*
 Constructor and Deconstructor

*/
AVLNode::AVLNode(AVLSegment* elem) :
  elem(elem), left(NULL), right(NULL), height(0) {
}

AVLNode::AVLNode(const AVLNode& node) {
 elem = node.elem;
 if (node.left == NULL) {
  left = NULL;
 } else {
  left = new AVLNode(*node.left);
 }
 if (node.right == NULL) {
  right = NULL;
 } else {
  right = new AVLNode(*node.right);
 }

 height = node.height;
}

AVLNode::~AVLNode() {
}

/*
 ~=~

*/
AVLNode& AVLNode::operator=(const AVLNode& node) {
 elem = node.elem;
 if (node.left == NULL) {
  left = NULL;
 } else {
  left = new AVLNode(*node.left);
 }
 if (node.right == NULL) {
  right = NULL;
 } else {
  right = new AVLNode(*node.right);
 }
 height = node.height;
 return *this;
}

/*
 ~memberPlusNeighbor~

*/
AVLNode* AVLNode::memberPlusNeighbor(AVLSegment* key, bool& result,
  int gridXPos, mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos,
  AVLSegment*& neighbor, bool pred) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*key,
   gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosForMember(*key, gridXPos, preciseXPos,
    gridYPos, preciseYPos);
 }
 if (cmpSegments == 0) {
  result = true;
  //search predecessor
  if (pred && left != NULL) {
   AVLNode* predNode = left;
   while (predNode->right != NULL) {
    predNode = predNode->right;
   }
   neighbor = &*(predNode->elem);
  }
  //search successor
  if (!pred && right != NULL) {
   AVLNode* sucNode = right;
   while (sucNode->left != NULL) {
    sucNode = sucNode->left;
   }
   neighbor = &*(sucNode->elem);
  }
  return this;
 } else {
  if (cmpSegments > 0) {
   if (left == 0) {
    result = false;
    return 0;
   } else {
    if (!pred) {
     neighbor = &*elem;
    }
    return left->memberPlusNeighbor(key, result, gridXPos, preciseXPos,
      gridYPos, preciseYPos, neighbor, pred);
   }
  } else {
   if (right == 0) {
    result = false;
    return 0;
   } else {
    if (pred) {
     neighbor = &*elem;
    }
    return right->memberPlusNeighbor(key, result, gridXPos, preciseXPos,
      gridYPos, preciseYPos, neighbor, pred);
   }
  }
 }
}

/*
 ~member~

*/
AVLNode* AVLNode::member(AVLSegment* key, bool& result, int gridXPos,
  mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos) {

 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*key,
   gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosForMember(*key, gridXPos, preciseXPos,
    gridYPos, preciseYPos);
 }
 if (cmpSegments == 0) {
  result = true;
  return this;
 } else {
  if (cmpSegments > 0) {
   if (left == 0) {
    result = false;
    return 0;
   } else {
    return left->member(key, result, gridXPos, preciseXPos, gridYPos,
      preciseYPos);
   }
  } else {
   if (right == 0) {
    result = false;
    return 0;
   } else {
    return right->member(key, result, gridXPos, preciseXPos, gridYPos,
      preciseYPos);
   }
  }
 }
}

void AVLNode::setElement(AVLSegment** seg) {
 elem = *seg;
}

/*

 ~insert~

*/
AVLNode* AVLNode::insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc) {

 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x,
   x->getGridXL());
 if (cmpSegments == 0) {
  mpq_class preciseX = x->getPreciseXL();
  cmpSegments = elem->compareInPosBackward(*x, x->getGridXL(), preciseX);
 }

 if (cmpSegments == 0) {
  if (elem->getOwner() == first) {
   elem->setSecondInsideAbove(x->getSecondInsideAbove());
  } else {
   elem->setFirstInsideAbove(x->getFirstInsideAbove());
  }
  elem->setConBelow(elem->getConBelow() + x->getConBelow());
  elem->setConAbove(elem->getConAbove() + x->getConAbove());
  assert((0 <= elem->getConBelow()) && (elem->getConBelow() <= 2));
  assert((0 <= elem->getConAbove()) && (elem->getConAbove() <= 2));
  elem->setOwner(both);
  *x = *elem;

  //search predecessor
  if (left != NULL) {
   AVLNode* predNode = left;
   while (predNode->right != NULL) {
    predNode = predNode->right;
   }
   pred = &*(predNode->elem);
  }
  //search successor
  if (right != NULL) {
   AVLNode* sucNode = right;
   while (sucNode->left != NULL) {
    sucNode = sucNode->left;
   }
   suc = &*(sucNode->elem);
  }

  return this;
 } else {
  if (cmpSegments > 0) {
   suc = &*elem;
   if (left == NULL) {
    left = new AVLNode(x);
    left->setHeight();
    return this;
   } else {
    left = left->insert(x, pred, suc);
    left->setHeight();
   }

  } else {
   pred = &*elem;
   if (right == NULL) {
    right = new AVLNode(x);
    right->setHeight();
    return this;
   } else {
    right = right->insert(x, pred, suc);
    right->setHeight();
   }
  }
 }

 setHeight();

 if (balance() == -2) {
  // the right son is out of balance
  return counterClockwiseRotation();
 }
 if (balance() == 2) {
  //the left son is out of balance
  return clockwiseRotation();
 }
 return this;
}

/*
 ~counterClockwiseRotation~

 This funcion rebalances the tree

*/
AVLNode* AVLNode::counterClockwiseRotation() {
 if (right->balance() != 1) {
  //the right son of right is higher than his left son or has equal height
  //a single rotation counterclockwise is necessary
  AVLNode* t1 = right;
  right = t1->left;
  t1->left = new AVLNode(*this);
  t1->left->setHeight();
  t1->setHeight();

  return t1;

 } else {
  //the left son of right is higher than his right son
  // a double rotation counterclockwise is necessary
  AVLNode *t1 = right;
  AVLNode *t2 = t1->left;
  t1->left = t2->right;
  t2->right = t1;
  right = t2->left;
  t2->left = new AVLNode(*this);
  (t2->left)->setHeight();
  (t2->right)->setHeight();
  t2->setHeight();
  return t2;
 }
}

/*
 ~clockwiseRotation~

 rebalances the tree

*/
AVLNode* AVLNode::clockwiseRotation() {
 if (left->balance() != -1) {
  //the left son of left is higher than his right son or has equal height
  //a single rotation clockwise is necessary
  AVLNode *t1 = left;
  left = t1->right;
  t1->right = new AVLNode(*this);
  t1->right->setHeight();

  t1->setHeight();
  return t1;

 } else {
  //the right son of left is higher than his left son
  // a double rotation clockwise is necessary
  AVLNode *t1 = left;
  AVLNode *t2 = t1->right;
  t1->right = t2->left;
  t2->left = t1;
  left = t2->right;
  t2->right = new AVLNode(*this);
  (t2->left)->setHeight();
  (t2->right)->setHeight();
  t2->setHeight();
  return t2;
 }
}

/*
 ~setHeight~

*/
void AVLNode::setHeight() {
 if (left == NULL && right == NULL) {
  height = 0;
 } else {
  if (left == NULL) {
   height = right->getHeight() + 1;
  } else {
   if (right == NULL) {
    height = left->getHeight() + 1;
   } else {
    if (left->getHeight() < right->getHeight()) {
     height = right->getHeight() + 1;
    } else {
     height = left->getHeight() + 1;
    }
   }
  }
 }
}

/*
 ~balance~

*/
int AVLNode::balance() {
 int h1, h2;
 if (left == NULL && right == NULL) {
  return 0;
 } else {
  if (left == NULL) {
   h1 = 0;
   h2 = right->getHeight() + 1;
  } else {
   if (right == NULL) {
    h2 = 0;
    h1 = left->getHeight() + 1;
   } else {
    h1 = left->getHeight() + 1;
    h2 = right->getHeight() + 1;
   }
  }
 }
 return h1 - h2;
}

/*
 ~removeGetNeighbor~

*/
AVLNode* AVLNode::removeGetNeighbor(AVLSegment* x, AVLSegment*& pred,
  AVLSegment*& suc) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x,
   x->getGridXR());
 if (cmpSegments == 0) {
  mpq_class preciseX = x->getPreciseXR();
  cmpSegments = elem->compareInPos(*x, x->getGridXR(), preciseX);
 }
 if (cmpSegments > 0) {
  if (left == NULL) {
   //Nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPos-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the left subtree." << endl;
   assert(false);
   return this;
  } else {
   suc = &*elem;
   left = left->removeGetNeighbor(x, pred, suc);
  }
  setHeight();
  if (right == NULL) {
   return this;
  }
  if (balance() == -2) {
   // the right son is higher than the left son
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   // the left son is higher than the right son
   return clockwiseRotation();
  }
  return this;
 } else if (cmpSegments < 0) {
  if (right == NULL) {
   // nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPos-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the right subtree." << endl;
   assert(false);
   return this;
  } else {
   pred = &*elem;
   right = right->removeGetNeighbor(x, pred, suc);
  }
  setHeight();
  if (left == NULL) {
   return this;
  }
  if (balance() == -2) {
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   return clockwiseRotation();
  }
  return this;
 } else {
  //search predecessor
  if (left != NULL) {
   AVLNode* predNode = left;
   while (predNode->right != NULL) {
    predNode = predNode->right;
   }
   pred = &*(predNode->elem);
  }
  //search successor
  if (right != NULL) {
   AVLNode* sucNode = right;
   while (sucNode->left != NULL) {
    sucNode = sucNode->left;
   }
   suc = &*(sucNode->elem);
  }
  if (isLeaf()) {
   delete this;
   return NULL;
  }
  if (left == NULL) {
   AVLNode *tr = right;
   elem = NULL;
   delete this;
   return tr;
  }
  if (right == NULL) {
   AVLNode *tl = left;
   elem = NULL;
   delete this;
   return tl;
  }
  elem = right->deletemin(right);

  if (right->elem == NULL) {
   AVLNode* tmp = right;
   right = right->right;
   if (right != NULL) {
    right->setHeight();
   }
   tmp->left = NULL;
   tmp->right = NULL;
   delete tmp;
  }
  setHeight();

  if (balance() == -2) {
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   return clockwiseRotation();
  }

  return this;
 }
}

/*
 ~removeGetNeighbor2~

*/
AVLNode* AVLNode::removeGetNeighbor2(AVLSegment* x, int gridXPos,
  mpq_class preciseXPos, AVLSegment*& pred, AVLSegment*& suc) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x, gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosBackward(*x, gridXPos, preciseXPos);
 }
 if (cmpSegments > 0) {
  if (left == NULL) {
   //Nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPosBackward-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the left subtree." << endl;
   assert(false);
   return this;
  } else {
   suc = &*elem;
   left = left->removeGetNeighbor2(x, gridXPos, preciseXPos, pred, suc);
  }
  setHeight();
  if (right == NULL) {
   return this;
  }
  if (balance() == -2) {
   // the right son is higher than the left son
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   // the left son is higher than the right son
   return clockwiseRotation();
  }
  return this;
 } else if (cmpSegments < 0) {
  if (right == NULL) {
   // nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPosBackward-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the right subtree." << endl;
   assert(false);
   return this;
  } else {
   pred = &*elem;
   right = right->removeGetNeighbor2(x, gridXPos, preciseXPos, pred, suc);
  }
  setHeight();
  if (left == NULL) {
   return this;
  }
  if (balance() == -2) {
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   return clockwiseRotation();
  }
  return this;
 } else {
  //search predecessor
  if (left != NULL) {
   AVLNode* predNode = left;
   while (predNode->right != NULL) {
    predNode = predNode->right;
   }
   pred = &*(predNode->elem);
  }
  //search successor
  if (right != NULL) {
   AVLNode* sucNode = right;
   while (sucNode->left != NULL) {
    sucNode = sucNode->left;
   }
   suc = &*(sucNode->elem);
  }
  if (isLeaf()) {
   delete this;
   return NULL;
  }
  if (left == NULL) {
   AVLNode *tr = right;
   elem = NULL;
   delete this;
   return tr;
  }
  if (right == NULL) {
   AVLNode *tl = left;
   elem = NULL;
   delete this;
   return tl;
  }
  elem = right->deletemin(right);
  if (right->elem == NULL) {
   AVLNode* tmp = right;
   right = right->right;
   if (right != NULL) {
    right->setHeight();
   }
   tmp->left = NULL;
   tmp->right = NULL;
   delete tmp;
  }
  setHeight();
  if (balance() == -2) {
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   return clockwiseRotation();
  }

  return this;
  return this;
 }
}

/*
 ~removeInvalidSegment~

*/
AVLNode* AVLNode::removeInvalidSegment(AVLSegment* x, int gridXPos,
  mpq_class preciseXPos, bool& found) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x, gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosBackward(*x, gridXPos, preciseXPos);
 }
 if (cmpSegments > 0) {
  if (left == NULL) {
   //Nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPosBackward-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the left subtree." << endl;
   assert(false);
   return this;
  } else {
   left = left->removeInvalidSegment(x, gridXPos, preciseXPos, found);
  }
  setHeight();
  if (right == NULL) {
   return this;
  }
  if (balance() == -2) {
   // the right son is higher than the left son
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   // the left son is higher than the right son
   return clockwiseRotation();
  }
  return this;
 } else if (cmpSegments < 0) {
  if (right == NULL) {
   // nothing to do
   cerr << "Element not found:" << endl;
   x->print();
   cerr << "is not located on this path. "
     << "There might be an error in the compareInPosBackward-function" << endl;
   cerr << "The path ends at element:" << endl;
   elem->print();
   cerr << "and the searched element would lie in the right subtree." << endl;
   assert(false);
   return this;
  } else {
   right = right->removeInvalidSegment(x, gridXPos, preciseXPos, found);
  }
  setHeight();
  if (left == NULL) {
   return this;
  }
  if (balance() == -2) {
   return counterClockwiseRotation();
  }
  if (balance() == 2) {
   return clockwiseRotation();
  }
  return this;
 } else {
  if (!elem->isValid()) {
   // delete this
   found = true;

   if (isLeaf()) {
    delete this;
    return NULL;
   }
   if (left == NULL) {
    AVLNode *tr = right;
    elem = NULL;
    delete this;
    return tr;
   }
   if (right == NULL) {
    AVLNode *tl = left;
    elem = NULL;
    delete this;
    return tl;
   }
   elem = right->deletemin(right);
   if (right->elem == NULL) {
    AVLNode* tmp = right;
    right = right->right;
    if (right != NULL) {
     right->setHeight();
    }
    tmp->left = NULL;
    tmp->right = NULL;
    delete tmp;
   }
   setHeight();
   if (balance() == -2) {
    return counterClockwiseRotation();
   }
   if (balance() == 2) {
    return clockwiseRotation();
   }

   return this;
  } else {
   // ~this~ was inserted earlier then the invalid
   // segment ~x~ ~x~ lay below ~this~
   left = left->removeInvalidSegment(x, gridXPos, preciseXPos, found);

   setHeight();
   if (left == NULL) {
    return this;
   }
   if (balance() == -2) {
    // the right son is higher than the left son
    return counterClockwiseRotation();
   }
   if (balance() == 2) {
    // the left son is higher than the right son
    return clockwiseRotation();
   }
   return this;
   if (!found) {
    right = right->removeInvalidSegment(x, gridXPos, preciseXPos, found);
    setHeight();
    if (right == NULL) {
     return this;
    }
    if (balance() == -2) {
     // the right son is higher than the left son
     return counterClockwiseRotation();
    }
    if (balance() == 2) {
     // the left son is higher than the right son
     return clockwiseRotation();
    }
    return this;
   }
  }

 }
}

/*
 ~isLeaf~

*/
bool AVLNode::isLeaf() {
 return (left == NULL && right == NULL);
}

/*
 ~deletemin~

*/
AVLSegment* AVLNode::deletemin(AVLNode* node) {
 AVLSegment* result = 0;
 if (left == NULL) {
  result = elem;
  elem = 0;
 } else {
  result = left->deletemin(left);
  if (left->elem == 0) {
   left = left->right;
  }

  if (left != 0) {
   left->setHeight();
  }
  setHeight();

  if (balance() == -2) {
   *node = *counterClockwiseRotation();
  }
  if (balance() == 2) {
   *node = *clockwiseRotation();
  }

 }
 return result;
}

/*
 ~inorder~

*/
void AVLNode::inorder() {
 if (left != 0) {
  left->inorder();
 }
 cout << "aktueller Knoten:" << endl;
 elem->print();
 if (left != 0) {

  cout << "left son: ";
  left->elem->print();
 } else {
  cout << "left Son: 0" << endl << endl;
 }
 if (right != 0) {
  cout << "right son: ";
  right->elem->print();
 } else {
  cout << "right Son: 0" << endl << endl;
 }
 cout << endl << endl << endl;
 if (right != 0) {
  right->inorder();
 }
}

/*
 Constructors and deconstructor

*/
AVLSegment::AVLSegment() :
  flob(0), dbarray(0), originalData1(0), originalData2(0),
  pxl(0), pyl(0), pxr(0), pyr(0),
  valid(false), isNew(true), noOfChanges(0), conAbove(0), conBelow(0) {
}

AVLSegment::AVLSegment(const Flob* preciseData, SegmentData* sd, Owner o) :
  gridXL(sd->GetDomGridXCoord()), gridYL(sd->GetDomGridYCoord()), gridXR(
    sd->GetSecGridXCoord()), gridYR(sd->GetSecGridYCoord()),
    flob(preciseData), dbarray(0), originalData1(sd), originalData2(0),
    pxl(0), pyl(0), pxr(0), pyr(0), owner(o),
    valid(true), isNew(false), noOfChanges(0),
    firstInsideAbove(o == first ? sd->GetInsideAbove() : 0),
    secondInsideAbove(o == second ? sd->GetInsideAbove() : 0),
    conAbove(sd->GetInsideAbove() ? 1 : 0),
    conBelow(sd->GetInsideAbove() ? 0 : 1) {
}

AVLSegment::AVLSegment(const DbArray<int>* preciseData, GridHalfSegment& gs,
  PrecHalfSegment* ps, Owner o) :
  gridXL(gs.GetLeftPointX()), gridYL(gs.GetLeftPointY()), gridXR(
    gs.GetRightPointX()), gridYR(gs.GetRightPointY()),
    flob(0), dbarray(preciseData), originalData1(0), originalData2(ps),
    pxl(0), pyl(0), pxr(0), pyr(0),
    owner(o), valid(true), isNew(false), noOfChanges(0),
    firstInsideAbove(o == first ? gs.GetAttr().insideAbove : 0),
    secondInsideAbove(o == second ? gs.GetAttr().insideAbove : 0),
    conAbove(gs.attr.insideAbove ? 1 : 0),
    conBelow(gs.attr.insideAbove ? 0 : 1) {
}

AVLSegment::AVLSegment(const AVLSegment& s) :
  gridXL(s.getGridXL()), gridYL(s.getGridYL()), gridXR(s.getGridXR()), gridYR(
    s.getGridYR()), flob(s.flob), dbarray(s.dbarray), originalData1(
    s.originalData1), originalData2(s.originalData2),
    pxl(s.pxl), pyl(s.pyl), pxr(s.pxr), pyr(s.pyr),
    owner(s.owner), valid(s.isValid()), isNew(s.isNew),
    noOfChanges(s.noOfChanges), firstInsideAbove(s.getFirstInsideAbove()),
    secondInsideAbove(s.getSecondInsideAbove()),
    conAbove(s.conAbove), conBelow(s.conBelow) {
}

AVLSegment::AVLSegment(int gridX, mpq_class px, int gridY, mpq_class py) :
  gridXL(gridX), gridYL(gridY), gridXR(gridX), gridYR(gridY),
  flob(0), dbarray(0), originalData1(0), originalData2(0),
  pxl(px), pyl(py), pxr(px), pyr(py),
  owner(none), valid(true), isNew(true), noOfChanges(0),
  firstInsideAbove(0), secondInsideAbove(0), conAbove(0), conBelow(0) {
}

AVLSegment::AVLSegment(mpq_class xLeft, mpq_class yLeft, mpq_class xRight,
  mpq_class yRight, Owner o) {

 if (cmp(xLeft, 0) <= 0) {
  gridXL = 0;
  pxl = xLeft;
 } else {
  prepareData(gridXL, pxl, xLeft);
 }
 if (cmp(yLeft, 0) <= 0) {
  gridYL = 0;
  pyl = yLeft;
 } else {
  prepareData(gridYL, pyl, yLeft);
 }
 if (cmp(xRight, 0) <= 0) {
  gridXR = 0;
  pxr = xRight;
 } else {
  prepareData(gridXR, pxr, xRight);
 }
 if (cmp(yRight, 0) <= 0) {
  gridYR = 0;
  pyr = yRight;
 } else {
  prepareData(gridYR, pyr, yRight);
 }
 setOwner(o);
 flob = NULL;
 dbarray = NULL;
 originalData1 = NULL;
 originalData2 = NULL;
 noOfChanges = 0;
 isNew = true;
 valid = true;
 firstInsideAbove = false;
 secondInsideAbove = false;
 conAbove = 0;
 conBelow = 0;
}

AVLSegment::AVLSegment(int gxl, int gyl, int gxr, int gyr, mpq_class xLeft,
  mpq_class yLeft, mpq_class xRight, mpq_class yRight, Owner o) :
  gridXL(gxl), gridYL(gyl), gridXR(gxr), gridYR(gyr),
  flob(0), dbarray(0), originalData1(0), originalData2(0),
  pxl(xLeft), pyl(yLeft), pxr(xRight), pyr(yRight),
  owner(o), valid(true), isNew(true), noOfChanges(0),
  firstInsideAbove(0), secondInsideAbove(0), conAbove(0), conBelow(0) {
}

AVLSegment::~AVLSegment() {
 originalData1 = NULL;
 originalData2 = NULL;
 flob = NULL;
 dbarray = NULL;
}

/*
 ~set~

*/
void AVLSegment::set(mpq_class xl, mpq_class yl, mpq_class xr, mpq_class yr,
  Owner o) {
 prepareData(gridXL, pxl, xl);
 prepareData(gridYL, pyl, yl);
 prepareData(gridXR, pxr, xr);
 prepareData(gridYR, pyr, yr);

 owner = o;
 isNew = true;
 valid = true;
 originalData1 = NULL;
 originalData2 = NULL;
 flob = NULL;
 dbarray = NULL;
}

void AVLSegment::set(int gxl, int gyl, int gxr, int gyr, mpq_class xl,
  mpq_class yl, mpq_class xr, mpq_class yr, Owner o) {
 gridXL = gxl;
 gridYL = gyl;
 gridXR = gxr;
 gridYR = gyr;

 pxl = xl;
 pyl = yl;
 pxr = xr;
 pyr = yr;

 owner = o;
 isNew = true;
 valid = true;
 originalData1 = NULL;
 originalData2 = NULL;
 flob = NULL;
 dbarray = NULL;
}

/*
 ~prepareData~

 Extract the integer from ~value~.

*/
void AVLSegment::prepareData(int& resultGridX, mpq_class& resultPX,
  mpq_class& value) {
 mpz_class numerator = value.get_num();
 mpz_class denominator = value.get_den();
 mpz_class gridX = numerator / denominator;

 resultGridX = (int) gridX.get_d();
 int cmpValue = cmp(value, resultGridX);
 if (cmpValue != 0) {
  //value is not an integer
  if (cmpValue < 0) {
   //value is a rational number less 0
   //the grid value is the next integer less than value
   resultGridX--;
  }
  resultPX = value - resultGridX;
 } else {
  //value is an integer
  resultPX = 0;
 }
}

/*
 ~getPrecise...~

*/
mpq_class AVLSegment::getPreciseXL() const {
 if (isNew) {
  return pxl;
 } else {
  if (originalData1) {
   return originalData1->getPreciseLeftX(flob);
  } else {
   return originalData2->GetlPointx(dbarray);
  }
 }

}

mpq_class AVLSegment::getPreciseYL() const {
 if (isNew) {
  return pyl;
 } else {
  if (originalData1) {
   return originalData1->getPreciseLeftY(flob);
  } else {
   return originalData2->GetlPointy(dbarray);
  }
 }
}

mpq_class AVLSegment::getPreciseXR() const {
 if (isNew) {
  return pxr;
 } else {
  if (originalData1) {
   return originalData1->getPreciseRightX(flob);
  } else {
   return originalData2->GetrPointx(dbarray);
  }
 }

}

mpq_class AVLSegment::getPreciseYR() const {
 if (isNew) {
  return pyr;
 } else {
  if (originalData1) {
   return originalData1->getPreciseRightY(flob);
  } else {
   return originalData2->GetrPointy(dbarray);
  }
 }

}

/*
 ~getNumberOfChanges~

*/
int AVLSegment::getNumberOfChanges() const {
 return noOfChanges;
}

/*
 ~setNumberOfChanges~

*/
void AVLSegment::setNumberOfChanges(int i) {
 noOfChanges = i;
}

/*
 ~incrementNumberOfChanges~

*/
void AVLSegment::incrementNumberOfChanges() {
 noOfChanges++;
}

/*
 ~getFirstInsideAbove~

*/
bool AVLSegment::getFirstInsideAbove() const {
 return firstInsideAbove;
}

/*
 ~getSecondInsideAbove~

*/
bool AVLSegment::getSecondInsideAbove() const {
 return secondInsideAbove;
}

/*
 ~setFirstInsideAbove~

*/
void AVLSegment::setFirstInsideAbove(bool insideAbove) {
 firstInsideAbove = insideAbove;
}

/*
 ~setSecondInsideAbove~

*/
void AVLSegment::setSecondInsideAbove(bool insideAbove) {
 secondInsideAbove = insideAbove;
}

/*
 ~getConBelow~

*/
short int AVLSegment::getConBelow() {
 return conBelow;
}

/*
 ~getConAbove~

*/
short int AVLSegment::getConAbove() {
 return conAbove;
}

/*
 ~setConBelow~

*/
void AVLSegment::setConBelow(short int i) {
 conBelow = i;
}

/*
 ~setConAbove~

*/
void AVLSegment::setConAbove(short int i) {
 conAbove = i;
}

/*
 ~print~

*/
void AVLSegment::print() {

 cout << "( " << getGridXL() << " " << getPreciseXL() << ", " << getGridYL()
   << " " << getPreciseYL() << ", " << getGridXR() << " " << getPreciseXR()
   << ", " << getGridYR() << " " << getPreciseYR() << " )" << endl << "owner: "
   << owner << endl << "conAbove: " << conAbove << endl << "conBelow: "
   << conBelow << endl << "valid: " << valid << endl;

}

/*
 ~equal~

*/
bool AVLSegment::equal(AVLSegment& s) const {
 if (getGridXL() != s.getGridXL() || getGridYL() != s.getGridYL()
   || getGridXR() != s.getGridXR() || getGridYR() != s.getGridYR()) {
  return false;
 }
 if ((cmp(getPreciseXL(), s.getPreciseXL()) != 0)
   || (cmp(getPreciseYL(), s.getPreciseYL()) != 0)
   || (cmp(getPreciseXR(), s.getPreciseXR()) != 0)
   || (cmp(getPreciseYR(), s.getPreciseYR()) != 0)) {
  return false;
 }
 return true;
}

/*
 ~isPoint~

*/
bool AVLSegment::isPoint() const {
 if (getGridXL() != getGridXR() || getGridYL() != getGridYR()) {
  return false;
 }
 if ((cmp(getPreciseXL(), getPreciseXR()) != 0)
   || (cmp(getPreciseYL(), getPreciseYR()) != 0)) {
  return false;
 }
 return true;
}

bool AVLSegment::isEndpoint(int gx, int gy, mpq_class px, mpq_class py) {
 if (getGridXL() == gx && getGridYL() == gy) {
  if (cmp(getPreciseXL(), px) == 0 && cmp(getPreciseYL(), py) == 0) {
   return true;
  }
 }
 if (getGridXR() == gx && getGridYR() == gy) {
  if (cmp(getPreciseXR(), px) == 0 && cmp(getPreciseYR(), py) == 0) {
   return true;
  }
 }
 return false;
}

bool AVLSegment::startsAtPoint(int gx, int gy, mpq_class px, mpq_class py) {
 if (getGridXL() == gx && getGridYL() == gy) {
  if (cmp(getPreciseXL(), px) == 0 && cmp(getPreciseYL(), py) == 0) {
   return true;
  }
 }
 return false;
}

bool AVLSegment::endsInPoint(int gx, int gy, mpq_class px, mpq_class py) {
 if (getGridXR() == gx && getGridYR() == gy) {
  if (cmp(getPreciseXR(), px) == 0 && cmp(getPreciseYR(), py) == 0) {
   return true;
  }
 }
 return false;
}

bool AVLSegment::isLeftOf(Event& event) {
 if (getGridXR() < event.getGridX()) {
  return true;
 }
 int cmpXR = cmp(getPreciseXR(), event.getPreciseX());
 if ((getGridXR() == event.getGridX()) && (cmpXR < 0)) {
  return true;
 }
 if ((getGridXR() > event.getGridX())
   || ((getGridXR() == event.getGridX()) && cmpXR > 0)) {
  return false;
 }

 //equal x

 if (getGridYR() < event.getGridY()) {
  return true;
 }
 if (getGridYR() > event.getGridY()) {
  return false;
 }
 int cmpYR = cmp(getPreciseYR(), event.getPreciseY());
 if (cmpYR < 0) {
  return true;
 }
 if (cmpYR > 0) {
  return false;
 }

 //The right endpoint of the segment is equal to the eventpoint
 //If the event is a leftendpoint event or an intersection event,
 //then the segment lay right of the event, otherwise not.
 if (event.isRightEndpointEvent() || event.isIntersectionEvent()) {
  return true;
 } else {
  return false;
 }
}

bool AVLSegment::leftPointIsLeftOf(Event& event) {
 if (getGridXL() < event.getGridX()) {
  return true;
 }
 int cmpXL = cmp(getPreciseXL(), event.getPreciseX());
 if ((getGridXL() == event.getGridX()) && (cmpXL < 0)) {
  return true;
 }
 if ((getGridXL() > event.getGridX())
   || ((getGridXL() == event.getGridX()) && cmpXL > 0)) {
  return false;
 }
 //equal x
 if (getGridYL() < event.getGridY()) {
  return true;
 }
 if (getGridYL() > event.getGridY()) {
  return false;
 }
 int cmpYL = cmp(getPreciseYL(), event.getPreciseY());
 if (cmpYL < 0) {
  return true;
 }
 if (cmpYL > 0) {
  return false;
 }
 //The right endpoint of the segment is equal to the eventpoint
 //If the event is a leftendpoint event or an intersection event, then
 //the segment lay right of the event, otherwise not.
 if (event.isRightEndpointEvent()) {
  return true;
 } else {
  return false;
 }
}

bool AVLSegment::isRightPointOf(AVLSegment& s) {
 if (s.getGridXR() != getGridXL()) {
  return false;
 }
 if (s.getGridYR() != getGridXL()) {
  return false;
 }
 if (cmp(s.getPreciseXR(), getPreciseXL()) != 0) {
  return false;
 }
 if (cmp(s.getPreciseYR(), getPreciseYL()) != 0) {
  return false;
 }
 return true;
}

bool AVLSegment::isLeftPointOf(AVLSegment& s) {
 if (s.getGridXL() != getGridXL()) {
  return false;
 }
 if (s.getGridYL() != getGridYL()) {
  return false;
 }
 if (cmp(s.getPreciseXL(), getPreciseXL()) != 0) {
  return false;
 }
 if (cmp(s.getPreciseYL(), getPreciseYL()) != 0) {
  return false;
 }
 return true;
}

bool AVLSegment::hasEqualLeftEndpointAs(AVLSegment& s) {
 if (s.getGridXL() != getGridXL()) {
  return false;
 }
 if (s.getGridYL() != getGridYL()) {
  return false;
 }
 if (cmp(s.getPreciseXL(), getPreciseXL()) != 0) {
  return false;
 }
 if (cmp(s.getPreciseYL(), getPreciseYL()) != 0) {
  return false;
 }
 return true;
}

/*
 ~isValid~

*/
bool AVLSegment::isValid() const {
 return valid;
}

/*
 ~changeValidity~

*/
void AVLSegment::changeValidity(bool v) {
 valid = v;
}

/*
 ~=~

*/
AVLSegment& AVLSegment::operator=(const AVLSegment& s) {
 gridXL = s.getGridXL();
 gridYL = s.getGridYL();
 gridXR = s.getGridXR();
 gridYR = s.getGridYR();
 flob = s.flob;
 originalData1 = s.originalData1;
 originalData2 = s.originalData2;
 owner = s.getOwner();
 valid = s.isValid();
 isNew = s.isNew;
 noOfChanges = s.getNumberOfChanges();
 firstInsideAbove = s.firstInsideAbove;
 secondInsideAbove = s.secondInsideAbove;
 conAbove = s.conAbove;
 conBelow = s.conBelow;

 if (isNew) {
  pxl = s.getPreciseXL();
  pyl = s.getPreciseYL();
  pxr = s.getPreciseXR();
  pyr = s.getPreciseYR();
 }
 return *this;
}

/*
 ~intervalIsVertical~

*/
bool AVLSegment::intervalIsVertical() {
 return gridXL == gridXR;
}

/*
 ~computeBeginOfIntersectionInterval~

*/
int AVLSegment::computeBeginOfIntersectionInterval(int pos) {
 if (intervalIsVertical()) {
  if (gridYL <= gridYR) {
   return gridYL;
  } else {
   return gridYR;
  }
 } else {
  if (((gridYR - gridYL) > 0 && (gridXR - gridXL) > 0)
    || ((gridYR - gridYL) < 0 && (gridXR - gridXL) < 0)) {
   //positive slope
   return (((pos * (gridYR - gridYL)) + (gridYL * (gridXR - gridXL))
     - ((gridXL + 1) * (gridYR - gridYL))) / (gridXR - gridXL));
  } else {
   return ((((pos + 1) * (gridYR - gridYL)) + (gridYL * (gridXR - gridXL))
     - (gridXL * (gridYR - gridYL))) / (gridXR - gridXL));
  }
 }
}

/*
 ~computeEndOfIntersectionInterval~

*/
int AVLSegment::computeEndOfIntersectionInterval(int pos) {
 if (intervalIsVertical()) {
  if (gridYL >= gridYR) {
   return gridYL + 1;
  } else {
   return gridYR + 1;
  }
 } else {
  if (((gridYR - gridYL) > 0 && (gridXR - gridXL) > 0)
    || ((gridYR - gridYL) < 0 && (gridXR - gridXL) < 0)) {
   //positive slope
   return ((((pos + 1) * (gridYR - gridYL)) + ((gridYL + 1) * (gridXR - gridXL))
     - ((gridXL) * (gridYR - gridYL))) / (gridXR - gridXL)) + 1;
  } else {
   return (((pos * (gridYR - gridYL)) + ((gridYL + 1) * (gridXR - gridXL))
     - ((gridXL + 1) * (gridYR - gridYL))) / (gridXR - gridXL)) + 1;
  }
 }
}

/*
 ~computePreciseYCoordInPos~

*/
bool AVLSegment::computePreciseYCoordInPos(mpq_class& pos, mpq_class& result) {
 pos.canonicalize();

 mpq_class x1 = getPreciseXL() + getGridXL();
 mpq_class x2 = getPreciseXR() + getGridXR();

 assert(cmp(x1, pos) <= 0 && cmp(pos, x2) <= 0);
 if (cmp(x2, x1) != 0) {

  mpq_class y1 = getPreciseYL() + getGridYL();
  mpq_class y2 = getPreciseYR() + getGridYR();

  mpq_class m = (y2 - y1) / (x2 - x1);

  result = (m * pos) + y1 - (m * x1);

  return true;
 } else {
  // the AVLSegment is vertical
  return false;
 }

}

int AVLSegment::compareIntersectionintervalWithSweepline(AVLSegment& s,
  int gridXPos) {
 assert(
   (gridXL <= gridXPos && gridXPos <= gridXR && s.getGridXL() <= gridXPos
     && gridXPos <= s.getGridXR()));
 int endInterval, beginIntervalOfS;
 endInterval = computeEndOfIntersectionInterval(gridXPos);
 beginIntervalOfS = s.computeBeginOfIntersectionInterval(gridXPos);

 if (endInterval < beginIntervalOfS) {
  // the interval of this segment in ~pos~ is below
  // the interval of ~s~ in ~pos~. So the y-value of this in ~pos~
  // is less than the y-value of ~s~ in ~pos~.
  return -1;
 }
 int beginInterval, endIntervalOfS;
 beginInterval = computeBeginOfIntersectionInterval(gridXPos);
 endIntervalOfS = s.computeEndOfIntersectionInterval(gridXPos);
 if (endIntervalOfS < beginInterval) {
  // the interval of this segment in ~pos~ is above
  // the interval of ~s~ in ~pos~. So the y-value of this in ~pos~
  // is greater than the y-value of ~s~ in ~pos~.
  return 1;
 }
 return 0;
}

/*
 ~compareInPos~

*/
int AVLSegment::compareInPos(AVLSegment& s, int gridXPos,
  mpq_class& preciseXPos) {
 mpq_class thisY, sY;
 mpq_class preciseXPosition = preciseXPos + gridXPos;

 // compute the slope of both segments
 bool thisIsNotVertical = false;
 bool sIsNotVertical = false;
 mpq_class thisSlope(0);
 mpq_class sSlope(0);

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisXR = getPreciseXR() + getGridXR();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class sXL = s.getPreciseXL() + s.getGridXL();
 mpq_class sYL = s.getPreciseYL() + s.getGridYL();
 mpq_class sXR = s.getPreciseXR() + s.getGridXR();
 mpq_class sYR = s.getPreciseYR() + s.getGridYR();

 if (cmp(thisXR, thisXL) != 0) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if (cmp(sXR, sXL) != 0) {

  sSlope = (sYR - sYL) / (sXR - sXL);

  sIsNotVertical = true;
 }

 if (thisIsNotVertical && sIsNotVertical) {
  mpq_class thisY = ((thisSlope * preciseXPosition) + thisYL
    - (thisSlope * thisXL));
  mpq_class sY = ((sSlope * preciseXPosition) + sYL - (sSlope * sXL));
  int cmpYinPos = cmp(thisY, sY);

  if (cmpYinPos != 0) {
   return cmpYinPos;
  } else {
   //both segments intersect in ~pos~
   if ((gridYL == gridYR) && (s.getGridYL() == s.getGridYR())
     && (cmp(getPreciseYL(), getPreciseYR()) == 0)
     && (cmp(s.getPreciseYL(), s.getPreciseYR()) == 0)) {
    //both segments are horizontal, compare left points
    //the left one is the smaller one
    if (gridXL != s.getGridXL()) {
     if (gridXL < s.getGridXL()) {
      return -1;
     } else {
      return 1;
     }
    }
    int cmpXL = cmp(getPreciseXL(), s.getPreciseXL());
    if (cmpXL != 0) {
     return cmpXL;
    }
    //both left points are equal, compare right points,
    //the left one is the smaller one
    if (gridXR != s.getGridXR()) {
     if (gridXR < s.getGridXR()) {
      return -1;
     } else {
      return 1;
     }
    }
    return cmp(getPreciseXR(), s.getPreciseXR());
   } else {
    int cmpSlope = cmp(sSlope, thisSlope);
    if (cmpSlope != 0) {
     if (cmpSlope < 0) {
      return 1;
     } else {
      return -1;
     }

    } else {
     //both segments run on the same line
     //the left one is smaller
     //int cmpSlope0 = cmp(thisSlope, 0);
     //if (cmpSlope0 >= 0) {
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      return cmp(thisXR, sXR);
     }

    }

   }
  }
 }

 else {
  if (thisIsNotVertical) {
   //~s~ is vertical
   mpq_class thisY = ((thisSlope * sXL) + thisYL - (thisXL * thisSlope));

   int cmpYinSXL1 = cmp(thisY, sYR);
   if (cmpYinSXL1 <= 0) {
    //~this~ runs below ~s~
    return -1;
   } else {
    return 1;
   }

  }
  if (sIsNotVertical) {
   //~this~ is vertical
   mpq_class sY = ((sSlope * thisXL) + sYL - (sXL * sSlope));

   int cmpYinSXL2 = cmp(sY, thisYR);
   if (cmpYinSXL2 <= 0) {
    //~s~ runs below ~this~
    return 1;
   } else {
    return -1;
   }
  }

  // both are vertical
  int cmpY1 = cmp(sYR, thisYL);
  if (cmpY1 < 0) {
   // this is above s
   return 1;
  } else {
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   int cmpY2 = cmp(thisYR, sYL);
   if (cmpY2 < 0) {
    // s above this
    return -1;
   }
   int cmpYL = cmp(thisYL, sYL);
   int cmpYR = cmp(thisYR, sYR);
   if (cmpYL != 0) {
    return cmpYL;
   } else {
    return cmpYR;
   }
  }
 }

}

/*
 ~compareInPosBackward~

*/
int AVLSegment::compareInPosBackward(AVLSegment& s, int gridXPos,
  mpq_class& preciseXPos) {

 mpq_class thisY, sY;
 mpq_class preciseXPosition = preciseXPos + gridXPos;

 // compute the slope of both segments
 bool thisIsNotVertical = false;
 bool sIsNotVertical = false;
 mpq_class thisSlope(0);
 mpq_class sSlope(0);

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisXR = getPreciseXR() + getGridXR();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class sXL = s.getPreciseXL() + s.getGridXL();
 mpq_class sYL = s.getPreciseYL() + s.getGridYL();
 mpq_class sXR = s.getPreciseXR() + s.getGridXR();
 mpq_class sYR = s.getPreciseYR() + s.getGridYR();

 if (cmp(thisXR, thisXL) != 0) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if (cmp(sXR, sXL) != 0) {

  sSlope = (sYR - sYL) / (sXR - sXL);

  sIsNotVertical = true;
 }

 if (thisIsNotVertical && sIsNotVertical) {
  mpq_class thisY = ((thisSlope * preciseXPosition) + thisYL
    - (thisSlope * thisXL));
  mpq_class sY = ((sSlope * preciseXPosition) + sYL - (sSlope * sXL));
  int cmpYinPos = cmp(thisY, sY);
  if (cmpYinPos != 0) {
   return cmpYinPos;
  } else {

   //both segments intersect in ~pos~
   if ((gridYL == gridYR) && (s.getGridYL() == s.getGridYR())
     && (cmp(getPreciseYL(), getPreciseYR()) == 0)
     && (cmp(s.getPreciseYL(), s.getPreciseYR()) == 0)) {
    //both segments are horizontal, compare left points
    //the left one is the smaller one
    if (gridXL != s.getGridXL()) {
     if (gridXL < s.getGridXL()) {
      return -1;
     } else {
      return 1;
     }
    }
    int cmpXL = cmp(getPreciseXL(), s.getPreciseXL());
    if (cmpXL != 0) {
     return cmpXL;
    }
    //both left points are equal, compare right points,
    //the left one is the smaller one
    if (gridXR != s.getGridXR()) {
     if (gridXR < s.getGridXR()) {
      return -1;
     } else {
      return 1;
     }
    }
    return cmp(getPreciseXR(), s.getPreciseXR());
   } else {
    int cmpSlope = cmp(sSlope, thisSlope);
    if (cmpSlope != 0) {
     return cmpSlope;

    } else {
     //both segments run on the same line
     //the left one is smaller
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      return cmp(thisXR, sXR);
     }

    }

   }
  }
 }

 else {
  if (thisIsNotVertical) {
   //~s~ is vertical
   mpq_class thisY = ((thisSlope * sXL) + thisYL - (thisXL * thisSlope));

   int cmpYinSXL1 = cmp(thisY, sYL);
   if (cmpYinSXL1 < 0) {
    //~this~ runs below ~s~
    return -1;
   } else {
    return 1;
   }
  }
  if (sIsNotVertical) {
   //~this~ is vertical
   mpq_class sY = ((sSlope * thisXL) + sYL - (sXL * sSlope));

   int cmpYinSXL2 = cmp(sY, thisYL);
   if (cmpYinSXL2 < 0) {
    //~s~ runs below ~this~
    return 1;
   } else {
    return -1;
   }
  }

  // both are vertical
  int cmpY1 = cmp(sYR, thisYL);
  if (cmpY1 < 0) {
   // this is above s
   return 1;
  } else {
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   int cmpY2 = cmp(thisYR, sYL);
   if (cmpY2 < 0) {
    // s above this
    return -1;
   }
   int cmpYL = cmp(thisYL, sYL);
   int cmpYR = cmp(thisYR, sYR);
   if (cmpYL != 0) {
    return cmpYL;
   } else {
    return cmpYR;
   }
  }
 }

}

/*
 ~compareInPosForMember~

*/
int AVLSegment::compareInPosForMember(AVLSegment& s, int gridXPos,
  mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos) {

 mpq_class thisY, sY;
 mpq_class preciseXPosition = preciseXPos + gridXPos;

 // compute the slope of both segments
 bool thisIsNotVertical = false;
 bool sIsNotVertical = false;
 mpq_class thisSlope(0);
 mpq_class sSlope(0);

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisXR = getPreciseXR() + getGridXR();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class sXL = s.getPreciseXL() + s.getGridXL();
 mpq_class sYL = s.getPreciseYL() + s.getGridYL();
 mpq_class sXR = s.getPreciseXR() + s.getGridXR();
 mpq_class sYR = s.getPreciseYR() + s.getGridYR();

 if (cmp(thisXR, thisXL) != 0) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if (cmp(sXR, sXL) != 0) {

  sSlope = (sYR - sYL) / (sXR - sXL);

  sIsNotVertical = true;
 }

 if (thisIsNotVertical && sIsNotVertical) {
  mpq_class thisY = ((thisSlope * preciseXPosition) + thisYL
    - (thisSlope * thisXL));
  mpq_class sY = ((sSlope * preciseXPosition) + sYL - (sSlope * sXL));
  int cmpYinPos = cmp(thisY, sY);
  if (cmpYinPos != 0) {
   return cmpYinPos;
  } else {
   //both segments intersect in ~pos~
   if ((gridYL == gridYR) && (s.getGridYL() == s.getGridYR())
     && (cmp(getPreciseYL(), getPreciseYR()) == 0)
     && (cmp(s.getPreciseYL(), s.getPreciseYR()) == 0)) {
    //both segments are horizontal, compare left points
    //the left one is the smaller one
    if (gridXL != s.getGridXL()) {
     if (gridXL < s.getGridXL()) {
      return -1;
     } else {
      return 1;
     }
    }
    int cmpXL = cmp(getPreciseXL(), s.getPreciseXL());
    if (cmpXL != 0) {
     return cmpXL;
    }
    //both left points are equal, compare right points,
    //the left one is the smaller one
    if (gridXR != s.getGridXR()) {
     if (gridXR < s.getGridXR()) {
      return -1;
     } else {
      return 1;
     }
    }
    return cmp(getPreciseXR(), s.getPreciseXR());
   } else {
    int cmpSlope = cmp(sSlope, thisSlope);
    if (cmpSlope != 0) {
     return cmpSlope;

    } else {
     //both segments run on the same line
     //the left one is smaller
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      return cmp(thisXR, sXR);
     }

    }

   }
  }
 }

 else {
  if (thisIsNotVertical) {
   //~s~ is vertical

   AVLSegment* overlappingSegment = new AVLSegment();
   if (this->intersect(s, *overlappingSegment)) {
    if (overlappingSegment->getGridYL() < gridYPos
      || (overlappingSegment->getGridYL() == gridYPos
        && cmp(overlappingSegment->getPreciseYL(), preciseYPos) < 0)) {
     //~this~ and ~s~ have been inverted before.

     mpq_class thisY = ((thisSlope * sXL) + thisYL - (thisXL * thisSlope));
     int cmpYinSXL1 = cmp(thisY, sYL);
     if (cmpYinSXL1 < 0) {
      //~this~ runs below ~s~
      return 1;
     } else {
      return -1;
     }
    }
   }
   mpq_class thisY = ((thisSlope * sXL) + thisYL - (thisXL * thisSlope));

   int cmpYinSXL1 = cmp(thisY, sYL);
   if (cmpYinSXL1 < 0) {
    //~this~ runs below ~s~
    return -1;
   } else {
    return 1;
   }
  }
  if (sIsNotVertical) {
   //~this~ is vertical
   AVLSegment* overlappingSegment = new AVLSegment();
   if (this->intersect(s, *overlappingSegment)) {
    if (overlappingSegment->getGridYL() < gridYPos
      || (overlappingSegment->getGridYL() == gridYPos
        && cmp(overlappingSegment->getPreciseYL(), preciseYPos) < 0)) {
     //~this~ and ~s~ have been inverted before.
     mpq_class sY = ((sSlope * thisXL) + sYL - (sXL * sSlope));

     int cmpYinSXL2 = cmp(sY, thisYL);
     if (cmpYinSXL2 < 0) {
      //~s~ runs below ~this~
      return -1;
     } else {
      return 1;
     }
    }
   }
   mpq_class sY = ((sSlope * thisXL) + sYL - (sXL * sSlope));

   int cmpYinSXL2 = cmp(sY, thisYL);
   if (cmpYinSXL2 < 0) {
    //~s~ runs below ~this~
    return 1;
   } else {
    return -1;
   }
  }

  // both are vertical
  int cmpY1 = cmp(sYR, thisYL);
  if (cmpY1 < 0) {
   // this is above s
   return 1;
  } else {
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   int cmpY2 = cmp(thisYR, sYL);
   if (cmpY2 < 0) {
    // s above this
    return -1;
   }
   int cmpYL = cmp(thisYL, sYL);
   int cmpYR = cmp(thisYR, sYR);
   if (cmpYL != 0) {
    return cmpYL;
   } else {
    return cmpYR;
   }
  }
 }

}

/*
 ~isVertical~

*/
bool AVLSegment::isVertical() const {
 if (getGridXL() != getGridXR()) {
  return false;
 } else {
  return (cmp(getPreciseXL(), getPreciseXR()) == 0);
 }
}

/*
 ~isParallelTo~

*/
bool AVLSegment::isParallelTo(const AVLSegment& s) const {

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisXR = getPreciseXR() + getGridXR();

 mpq_class sXL = s.getPreciseXL() + s.getGridXL();
 mpq_class sXR = s.getPreciseXR() + s.getGridXR();
 if (cmp(thisXL, thisXR) == 0) {
  //this is vertical
  if (cmp(sXL, sXR) == 0) {
   //s is vertical
   return true;
  } else {
   return false;
  }
 }

 if (cmp(sXL, sXR) == 0) {
  //s is vertical
  return false;
 }

 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class sYL = s.getPreciseYL() + s.getGridYL();
 mpq_class sYR = s.getPreciseYR() + s.getGridYR();

 mpq_class thisSlope = (thisYR - thisYL) / (thisXR - thisXL);
 mpq_class sSlope = (sYR - sYL) / (sXR - sXL);

 if (cmp(thisSlope, sSlope) == 0) {
  return true;
 }
 return false;
}

/*
 ~getSlope~

*/
mpq_class AVLSegment::getSlope() const {
 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisXR = getPreciseXR() + getGridXR();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class m = (thisYR - thisYL) / (thisXR - thisXL);

 return m;
}

/*

 ~mightIntersect~

*/
bool AVLSegment::mightIntersect(AVLSegment& seg) {
 BoundingSegments* thisBS = new BoundingSegments(gridXL, gridYL, gridXR,
   gridYR);
 BoundingSegments* segBS = new BoundingSegments(seg.getGridXL(),
   seg.getGridYL(), seg.getGridXR(), seg.getGridYR());
 bool result = thisBS->intersect(*segBS);
 delete thisBS;
 delete segBS;
 return result;
}

/*

 ~intersect~

*/
bool AVLSegment::intersect(AVLSegment& seg, AVLSegment& result) {

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisYL = getPreciseYL() + getGridYL();
 mpq_class thisXR = getPreciseXR() + getGridXR();
 mpq_class thisYR = getPreciseYR() + getGridYR();

 mpq_class segXL = seg.getPreciseXL() + seg.getGridXL();
 mpq_class segYL = seg.getPreciseYL() + seg.getGridYL();
 mpq_class segXR = seg.getPreciseXR() + seg.getGridXR();
 mpq_class segYR = seg.getPreciseYR() + seg.getGridYR();

 if (cmp(thisXL, thisXR) == 0 && cmp(segXL, segXR) == 0) {
  //both are vertical
  if (cmp(thisXL, segXL) == 0) {
   //both segments run on one line
   if (cmp(segYR, thisYL) < 0 || cmp(thisYR, segYL) < 0) {
    //this passes below or above ~seg~
    return false;
   } else {
    //determine the endpoints
    if (cmp(thisYL, segYL) <= 0) {
     //overlapping segment starts in (segXL, segYL)
     if (cmp(segYR, thisYR) <= 0) {
      //overlapping segment ends in (segXL, segYR)
      result.set(segXL, segYL, segXL, segYR, both);
      return true;
     } else {
      //overlapping segment ends in (segXL, thisYR)
      result.set(seg.getGridXL(), seg.getGridYL(), seg.getGridXL(), getGridYR(),
        seg.getPreciseXL(), seg.getPreciseYL(), seg.getPreciseXL(),
        getPreciseYR(), both);
      return true;
     }
    } else {
     //overlapping segment starts in (thisXL, thisYL)
     if (cmp(segYR, thisYR) <= 0) {
      //overlapping segment ends in (segXL, segYR)
      result.set(getGridXL(), getGridYL(), getGridXL(), seg.getGridYR(),
        getPreciseXL(), getPreciseYL(), getPreciseXL(), seg.getPreciseYR(),
        both);
      return true;
     } else {
      //overlapping segment ends in (segXL, thisYR)
      result.set(getGridXL(), getGridYL(), getGridXL(), getGridYR(),
        getPreciseXL(), getPreciseYL(), getPreciseXL(), getPreciseYR(), both);
      return true;
     }
    }

   }
  }

 }

 if (cmp(thisXL, thisXR) == 0) {
  //this segments is vertical
  if (cmp(segXL, thisXL) <= 0 && cmp(thisXL, segXR) <= 0) {
   //this runs through segXL and segXR
   mpq_class segSlope = (segYR - segYL) / (segXR - segXL);
   mpq_class segB = segYL - segSlope * segXL;
   mpq_class yValue = segSlope * thisXL + segB;

   if ((cmp(thisYL, yValue) <= 0 && cmp(yValue, thisYR) <= 0)
     || (cmp(thisYR, yValue) <= 0 && cmp(yValue, thisYL) <= 0)) {
    result.set(thisXL, yValue, thisXL, yValue, both);
    return true;
   } else {
    return false;
   }
  } else {
   //this runs more left/right than seg
   return false;
  }
 }

 if (cmp(segXL, segXR) == 0) {
  //seg is vertical
  if (cmp(thisXL, segXL) <= 0 && cmp(segXL, thisXR) <= 0) {
   //seg runs through thisXL and thisXR
   mpq_class thisSlope = (thisYR - thisYL) / (thisXR - thisXL);
   mpq_class thisB = thisYL - thisSlope * thisXL;
   mpq_class yValue = thisSlope * segXL + thisB;

   if ((cmp(segYL, yValue) <= 0 && cmp(yValue, segYR) <= 0)
     || (cmp(segYR, yValue) <= 0 && cmp(yValue, segYL) <= 0)) {
    result.set(segXL, yValue, segXL, yValue, both);
    return true;
   } else {
    return false;
   }
  } else {
   //this runs more left/right than seg
   return false;
  }
 }

 mpq_class thisSlope = (thisYR - thisYL) / (thisXR - thisXL);
 mpq_class segSlope = (segYR - segYL) / (segXR - segXL);
 mpq_class thisB = thisYL - thisSlope * thisXL;
 mpq_class segB = segYL - segSlope * segXL;

 if (cmp(thisSlope, segSlope) != 0) {
  mpq_class xValue = (segB - thisB) / (thisSlope - segSlope);
  if (cmp(segXL, xValue) <= 0 && cmp(xValue, segXR) <= 0
    && cmp(thisXL, xValue) <= 0 && cmp(xValue, thisXR) <= 0) {
   mpq_class yValue = segSlope * xValue + segB;
   yValue.canonicalize();
   result.set(xValue, yValue, xValue, yValue, both);
   return true;
  } else {
   return false;
  }
 } else {
  // the segments are parallel
  if (cmp(thisB, segB) == 0) {
   //both segments runs on a line
   if (cmp(segXR, thisXL) < 0 || cmp(thisXR, segXL) < 0) {
    // seg runs more left/right than this
    return false;
   } else {
    if (cmp(thisXL, segXL) < 0) {
     if (cmp(thisXR, segXR) < 0) {
      result.set(segXL, segYL, thisXR, thisYR, both);
      return true;
     } else {
      result.set(segXL, segYL, segXR, segYR, both);
      return true;
     }
    } else {
     if (cmp(thisXR, segXR) < 0) {
      result.set(thisXL, thisYL, thisXR, thisYR, both);
      return true;
     } else {
      result.set(thisXL, thisYL, segXR, segYR, both);
      return true;
     }
    }

   }
  } else {
   return false;
  }
 }
 assert(false);
 return false;
}

/*
 SimpleSegment and BoundingSegments are used in mightIntersect.
 An object of type BoundingSegments contains the segments which
 form the envelope of a real segment.

*/

/*
 Constructors and Deconstructor

*/
SimpleSegment::SimpleSegment(int gxl, int gyl, int gxr, int gyr, Position gp) :
  xl(gxl), yl(gyl), xr(gxr), yr(gyr), p(gp) {
}

BoundingSegments::BoundingSegments(int gxl, int gyl, int gxr, int gyr) {
 if (gxl == gxr) {
  //the segment is vertical
  numSeg = 4;
  createBoundingSegments(vertical, gxl, gyl, gxr, gyr);
 } else {
  if (gyl == gyr) {
   //the segment is horizontal
   numSeg = 4;
   createBoundingSegments(horizontal, gxl, gyl, gxr, gyr);
  } else {
   int numerator = gyr - gyl;
   int denumerator = gxr - gxl;
   numSeg = 6;
   if ((numerator > 0 && denumerator > 0)
     || (numerator < 0 && denumerator < 0)) {
    //the segment has a positiv slope
    createBoundingSegments(positivSlope, gxl, gyl, gxr, gyr);
   } else {
    //the segment has a negativ slope
    createBoundingSegments(negativSlope, gxl, gyl, gxr, gyr);
   }
  }
 }
}

BoundingSegments::~BoundingSegments() {

}

void BoundingSegments::createBoundingSegments(Slope s, int gxl, int gyl,
  int gxr, int gyr) {
 switch (s) {
 case vertical: {
  segments = new SimpleSegment[numSeg];
  segments[0] = SimpleSegment(gxl, gyl, gxr, gyr + 1, pLeft);
  segments[1] = SimpleSegment(gxr, gyr + 1, gxr + 1, gyr + 1, top);
  segments[2] = SimpleSegment(gxl + 1, gyl, gxr + 1, gyr + 1, pRight);
  segments[3] = SimpleSegment(gxl, gyl, gxl + 1, gyl, bottom);
  break;
 }
 case horizontal: {
  segments = new SimpleSegment[numSeg];
  segments[0] = SimpleSegment(gxl, gyl, gxl, gyl + 1, pLeft);
  segments[1] = SimpleSegment(gxl, gyl + 1, gxr + 1, gyr + 1, top);
  segments[2] = SimpleSegment(gxr + 1, gyr, gxr + 1, gyr + 1, pRight);
  segments[3] = SimpleSegment(gxl, gyl, gxr + 1, gyr, bottom);
  break;
 }
 case positivSlope: {
  segments = new SimpleSegment[numSeg];
  segments[0] = SimpleSegment(gxl, gyl, gxl, gyl + 1, pLeft);
  segments[1] = SimpleSegment(gxl, gyl + 1, gxr, gyr + 1, top);
  segments[2] = SimpleSegment(gxr, gyr + 1, gxr + 1, gyr + 1, top);
  segments[3] = SimpleSegment(gxr + 1, gyr, gxr + 1, gyr + 1, pRight);
  segments[4] = SimpleSegment(gxl + 1, gyl, gxr + 1, gyr, bottom);
  segments[5] = SimpleSegment(gxl, gyl, gxl + 1, gyl, bottom);
  break;
 }
 case negativSlope: {
  segments = new SimpleSegment[numSeg];
  segments[0] = SimpleSegment(gxl, gyl, gxl, gyl + 1, pLeft);
  segments[1] = SimpleSegment(gxl, gyl + 1, gxl + 1, gyl + 1, top);
  segments[2] = SimpleSegment(gxl + 1, gyl + 1, gxr + 1, gyr + 1, top);
  segments[3] = SimpleSegment(gxr + 1, gyr, gxr + 1, gyr + 1, pRight);
  segments[4] = SimpleSegment(gxr, gyr, gxr + 1, gyr, bottom);
  segments[5] = SimpleSegment(gxl, gyl, gxr, gyr, bottom);
  break;
 }
 default: {
  assert(false);
  break;
 }
 }
}

/*
 ~isVertical~

*/
bool BoundingSegments::isVertical(int i) {
 assert(0 <= i && i < numSeg);
 return (segments[i].getXR() == segments[i].getXL());
}

/*
 ~isHorizontal~

*/
bool BoundingSegments::isHorizontal(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getYR() == segments[i].getYL());
}

/*
 ~isLeft~

*/
bool BoundingSegments::isLeft(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == pLeft);
}

/*
 ~isRight~

*/
bool BoundingSegments::isRight(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == pRight);
}

/*
 ~isTop~

*/
bool BoundingSegments::isTop(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == top);
}

/*
 ~isBottom~

*/
bool BoundingSegments::isBottom(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == bottom);
}

/*
 ~intersect~

*/
bool BoundingSegments::intersect(BoundingSegments& bs) {
 int i = 0;
 bool result = false;
 while (i < numSeg && !result) {
  int j = 0;
  while (j < bs.numSeg && !result) {
   int thisNum = segments[i].getYR() - segments[i].getYL();
   int thisDenom = segments[i].getXR() - segments[i].getXL();

   int bsNum = bs.segments[j].getYR() - bs.segments[j].getYL();
   int bsDenom = bs.segments[j].getXR() - bs.segments[j].getXL();
   if (thisDenom == 0 && bsDenom == 0) {
    // both segments are vertical
    if (((isRight(i) && bs.isRight(j)) || (isLeft(i) && isLeft(j)))
      && ((segments[i].getXL() == bs.segments[j].getXL())
        && !(segments[i].getYR() <= bs.segments[j].getYL()
          || segments[i].getYL() >= bs.segments[j].getYR()))) {
     //If one segment is the left border of the real segment
     //and one segment is the right border of the real segment,
     //intersection of both is not relevant.
     //An intersection is relevant if both are from the
     //same side, both run through the same x-value and
     //they overlap.
     return true;
    }
   } else { //max. one segment is vertical
    if (thisDenom == 0 || bsDenom == 0) {
     SimpleSegment vertical;
     SimpleSegment second;
     if (isVertical(i)) {
      vertical = segments[i];
      second = bs.segments[j];
     } else {
      vertical = bs.segments[j];
      second = segments[i];
     }
     if (second.getXL() <= vertical.getXL()
       && vertical.getXL() <= second.getXR()) {
      // the vertical segment lay between XL and XR
      int numerator = second.getYR() - second.getYL();
      int denominator = second.getXR() - second.getXL();
      if ((numerator > 0 && denominator > 0)
        || (numerator < 0 && denominator < 0)) {
       /* second has a positiv slope
        *  To prevent wrong values by using double we compute
        *  only with integers. If we have a function like
        *  f(x)=m*x+b, we take: m=numerator/denumerator
        *  and b=second.getGridYL()-m*second.getGridXL().
        *  By avoiding double-values we get:
        *  y=(numerator*vertical.getGridXL()
        *  + (second.getGridYL()+1)*denumerator
        *  - numerator*second.getGridXL()) / denumerator.
        *  Then y is the grid-value. If yValue is in the
        *  interval formed by vertical, both segments might
        *  intersect.
       */
       if (!((second.getPosition() == top && vertical.getPosition() == pRight
         && vertical.getXL() == second.getXL())
         || (second.getPosition() == bottom && vertical.getPosition() == pLeft
           && vertical.getXL() == second.getXR()))) {
        int yValue = (numerator * vertical.getXL()
          + second.getYL() * denominator - numerator * second.getXL())
          / denominator;
        if (((vertical.getYL() <= yValue && yValue < vertical.getYR())
          || (vertical.getYR() <= yValue && yValue <= vertical.getYL()))
          && !(second.getPosition() == top && vertical.getPosition() == pLeft
            && vertical.getXL() == second.getXR()
            && yValue == vertical.getYL())) {
         return true;
        }
       }
      } else {
       if ((numerator > 0 && denominator < 0)
         || (numerator < 0 && denominator > 0)) {
        //second has a negativ slope
        if (!((second.getPosition() == top && vertical.getPosition() == pLeft
          && vertical.getXL() == second.getXR())
          || (second.getPosition() == bottom && vertical.getPosition() == pRight
            && vertical.getXL() == second.getXL()))) {

         int yValue = (numerator * vertical.getXL()
           + second.getYL() * denominator - numerator * second.getXL())
           / denominator;
         if (((vertical.getYL() <= yValue && yValue <= vertical.getYR())
           || (vertical.getYR() <= yValue && yValue <= vertical.getYL()))
           && !(second.getPosition() == top && vertical.getPosition() == pRight
             && vertical.getXL() == second.getXL()
             && yValue == vertical.getYL())) {
          return true;
         }

        }
       } else { // second is horizonal
        if (vertical.getPosition() == pLeft) {
         if (second.getXL() <= vertical.getXL()
           && vertical.getXL() < second.getXR()
           && vertical.getYL() < second.getYL()
           && second.getYL() < vertical.getYR()) {
          return true;
         }
        } else { //vertical is right
         if (second.getXL() < vertical.getXL()
           && vertical.getXL() <= second.getXR()
           && vertical.getYL() < second.getYL()
           && second.getYL() < vertical.getYR()) {
          return true;
         }
        }
       }
      }
     }
    } else { //there is no vertical segment
     if (thisDenom * bsNum == thisNum * bsDenom) {
      //the segments are parallel
      if ((isBottom(i) && bs.isBottom(j)) || (isTop(i) && bs.isTop(j))) {
       //compare the intersection with the y-coordinate
       //(~thisB~ and ~segB~). If they are equal, both
       //segments run on the same line and intersect if they
       //overlap
       int thisB = segments[i].getYL() * thisDenom
         - segments[i].getXL() * thisNum;
       int segB = bs.segments[j].getYL() * thisDenom
         - bs.segments[j].getXL() * thisNum;
       if (thisB == segB) {
        if (!(segments[i].getXR() <= bs.segments[j].getXL()
          || bs.segments[j].getXR() <= segments[i].getXL())) {
         return true;
        }
       }
      }
     } else { //Segemente are not parallel or vertical
      int xNumerator = (thisDenom * bsDenom
        * (bs.segments[j].getYL() - segments[i].getYL()))
        + (segments[i].getXL() * bsDenom * thisNum)
        - (bs.segments[j].getXL() * thisDenom * bsNum);
      int xDenominator = (thisNum * bsDenom - thisDenom * bsNum);
      int gridX = xNumerator / xDenominator;
      if ((segments[i].getXL() <= gridX && gridX < segments[i].getXR())
        &&
        (bs.segments[j].getXL() <= gridX && gridX < bs.segments[j].getXR())) {
       //the x-value of the intersection point lays in
       //the x-interval of both segments
       if (thisNum == 0) {
        //this is a horizontal segment
        if ((bsNum < 0 && bsDenom < 0) || (bsNum > 0 && bsDenom > 0)) {
         //the segment j of bs has a positiv slope
         if (!(segments[i].getPosition() == bottom
           && bs.segments[j].getPosition() == top
           && segments[i].getXL() == bs.segments[j].getXR())
           && !(segments[i].getPosition() == top
             && bs.segments[j].getPosition() == bottom
             && segments[i].getXR() == bs.segments[j].getXL())) {
          if (bs.segments[j].getYL() <= segments[i].getYL()
            && segments[i].getYL() <= bs.segments[j].getYR()) {
           return true;
          }
         }
        } else {
         if (!(segments[i].getPosition() == bottom
           && bs.segments[j].getPosition() == top
           && segments[i].getXR() == bs.segments[j].getXL())
           && !(segments[i].getPosition() == top
             && bs.segments[j].getPosition() == bottom
             && segments[i].getXL() == bs.segments[j].getXR())) {
          if (bs.segments[j].getYR() <= segments[i].getYL()
            && segments[i].getYL() <= bs.segments[j].getYL()) {
           return true;
          }
         }
        }

       } else {
        if (bsNum == 0) {
         if ((thisNum < 0 && thisDenom < 0) || (thisNum > 0 && thisDenom > 0)) {
          //the segment i of this has a positiv slope
          if (!(bs.segments[j].getPosition() == bottom
            && segments[i].getPosition() == top
            && bs.segments[j].getXL() == segments[i].getXR())
            && !(bs.segments[j].getPosition() == top
              && segments[i].getPosition() == bottom
              && bs.segments[j].getXR() == segments[i].getXL())) {
           if (segments[i].getYL() <= bs.segments[j].getYL()
             && bs.segments[j].getYL() <= segments[i].getYR()) {
            return true;
           }
          }
         } else {
          if ((!(bs.segments[j].getPosition() == bottom
            && segments[i].getPosition() == top
            && bs.segments[j].getXR() == segments[i].getXL()))
            && (!(bs.segments[j].getPosition() == top
              && segments[i].getPosition() == bottom
              && bs.segments[j].getXL() == segments[i].getXR()))) {
           if (segments[i].getYR() <= bs.segments[j].getYL()
             && bs.segments[j].getYL() <= segments[i].getYL()) {
            return true;
           }
          }
         }
        } else {
         //both segments are not horizonatal
         int gridY = ((thisNum * gridX) + (segments[i].getYL() * thisDenom)
           - (segments[i].getXL() * thisNum)) / thisDenom;
         int thisYMin = min(segments[i].getYL(), segments[i].getYR());
         int thisYMax = max(segments[i].getYL(), segments[i].getYR());
         int bsYMin = min(bs.segments[j].getYL(), bs.segments[j].getYR());
         int bsYMax = max(bs.segments[j].getYL(), bs.segments[j].getYR());
         if ((thisYMin <= gridY && gridY <= thisYMax)
           && (bsYMin <= gridY && gridY <= bsYMax)) {
          return true;
         }
        }
       }
      }
     }
    }
   }
   j++;
  }
  i++;
 }
 return result;
}

/*
 Event

*/

/*
 Constructors and Deconstructor

*/
Event::Event(KindOfEvent k, AVLSegment* s) :
  kind(k), gridX(k == leftEndpoint ? s->getGridXL() : s->getGridXR()), gridY(
    k == leftEndpoint ? s->getGridYL() : s->getGridYR()), preciseX(
    k == leftEndpoint ? s->getPreciseXL() : s->getPreciseXR()), preciseY(
    k == leftEndpoint ? s->getPreciseYL() : s->getPreciseYR()),
    seg(s), lSeg(0), gSeg(0), noOfChangesSeg(s->getNumberOfChanges()) {
}

Event::Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2) :
  kind(k), gridX(s1->getGridXR()), gridY(s1->getGridYR()), preciseX(
    s1->getPreciseXR()), preciseY(s1->getPreciseYR()),
    seg(NULL), lSeg(s1), gSeg(s2),
    noOfChangesSeg((s1->getNumberOfChanges() + s2->getNumberOfChanges())) {
}

Event::Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2, AVLSegment* s3) :
  kind(k), gridX(s3->getGridXL()), gridY(s3->getGridYL()), preciseX(
    s3->getPreciseXL()), preciseY(s3->getPreciseYL()),
    seg(NULL), lSeg(s1), gSeg(s2),
    noOfChangesSeg((s1->getNumberOfChanges() + s2->getNumberOfChanges())) {
}

Event::Event(const Event& e) :
  kind(e.kind), gridX(e.gridX), gridY(e.gridY), preciseX(e.preciseX), preciseY(
    e.preciseY), seg(e.seg), lSeg(e.lSeg), gSeg(e.gSeg), noOfChangesSeg(
    e.getNoOfChanges()) {
}

Event::~Event() {
}

/*
 ~getGridX~

*/
int Event::getGridX() const {
 return gridX;
}

/*
 ~getPreciseX~

*/
mpq_class Event::getPreciseX() const {
 return preciseX;
}

/*
 ~getGridY~

*/
int Event::getGridY() const {
 return gridY;
}

/*
 ~getPreciseX~

*/
mpq_class Event::getPreciseY() const {
 return preciseY;
}

/*
 ~getNoOfChanges~

*/
int Event::getNoOfChanges() const {
 return noOfChangesSeg;
}

/*
 ~=~

*/
Event& Event::operator=(const Event& e) {
 kind = e.kind;
 gridX = e.gridX;
 gridY = e.gridY;
 preciseX = e.preciseX;
 preciseY = e.preciseY;
 if (kind == intersectionPoint) {
  seg = 0;
  lSeg = e.lSeg;
  gSeg = e.gSeg;
 } else {
  seg = e.seg;
  lSeg = 0;
  gSeg = 0;
 }
 noOfChangesSeg = e.noOfChangesSeg;
 return *this;
}

/*
 ~isValid~
 An Event becomes invalid, if the associated segment(s) have been
 changed between the time where the eventis put in the
 priority\_queue and the time where it has to be precessed.

*/
bool Event::isValid() const {
 if (this->isIntersectionEvent()) {
  if (lSeg->isValid() && gSeg->isValid()) {
   return true;
  } else {
   return false;
  }
 }
 if (seg->isValid()) {
  if (this->isRightEndpointEvent()) {
   if (seg->getNumberOfChanges() == noOfChangesSeg) {
    return true;
   } else {
    return false;
   }
  } else {
   return true;
  }
 } else {
  return false;
 }
}

/*
 ~isLeftEndpointEvent~

*/
bool Event::isLeftEndpointEvent() const {
 return (kind == leftEndpoint);
}

/*
 ~isRightEndpointEvent~

*/
bool Event::isRightEndpointEvent() const {
 return (kind == rightEndpoint);
}

/*
 ~isIntersectionEvent~

*/
bool Event::isIntersectionEvent() const {
 if (kind == intersectionPoint) {
  return true;
 }
 return false;
}

/*
 ~getSegment~

*/
AVLSegment* Event::getSegment() const {
 return seg;
}

/*
 ~getLeftSegment~

*/
AVLSegment* Event::getLeftSegment() const {
 return lSeg;
}

/*
 ~getRightSegment~

*/
AVLSegment* Event::getRightSegment() const {
 return gSeg;
}

/*
 ~>~

*/
bool Event::operator>(const Event& e) const {
 if (getGridX() != e.getGridX()) {
  if (getGridX() > e.getGridX()) {
   return true;
  } else {
   return false;
  }
 }

 int cmpX = cmp(getPreciseX(), e.getPreciseX());
 if (cmpX != 0) {
  if (cmpX > 0) {
   return true;
  } else {
   return false;
  }
 }
 //equal x
 if (getGridY() != e.getGridY()) {
  if (getGridY() > e.getGridY()) {
   return true;
  } else {
   return false;
  }
 }

 int cmpY = cmp(getPreciseY(), e.getPreciseY());
 if (cmpY != 0) {
  if (cmpY > 0) {
   return true;
  } else {
   return false;
  }
 }

 //equal y
 if (isRightEndpointEvent()) {
  if (e.isRightEndpointEvent()) {
   mpq_class p = getPreciseX();
   return (seg->compareInPosBackward(*e.seg, getGridX(), p)) == 1;
  } else {
   return true;
  }
 }
 if (isIntersectionEvent() && e.isLeftEndpointEvent()) {
  return true;
 }
 if (isIntersectionEvent() && e.isIntersectionEvent()) {
  mpq_class p = getPreciseX();
  int cmpLSeg = lSeg->compareInPosBackward(*e.lSeg, getGridX(), p);
  if (cmpLSeg != 0) {
   if (cmpLSeg > 0) {
    return true;
   } else {
    return false;
   }
  } else {

   int cmpGSeg = gSeg->compareInPosBackward(*e.gSeg, getGridX(), p);
   if (cmpGSeg > 0) {
    return true;
   } else {
    return false;
   }

  }
 }
 if (isLeftEndpointEvent() && e.isLeftEndpointEvent()) {
  mpq_class p = getPreciseX();
  return (seg->compareInPosBackward(*e.seg, getGridX(), p)) == 1;
 } else {
  return false;
 }
 assert(false);
 return false;
}

void Event::print() const {
 if (!isValid()) {
  cout << "invalid ";
 }
 switch (kind) {
 case 0:
  cout << "leftendpoint-event in (" << getGridX() << " " << getPreciseX()
    << ", " << getGridY() << " " << getPreciseY() << ")" << endl;
  cout << "with segment: " << endl;
  seg->print();
  break;

 case 1:
  cout << "rightendpoint-event in (" << getGridX() << " " << getPreciseX()
    << ", " << getGridY() << " " << getPreciseY() << ")" << endl;
  cout << "with segment: " << endl;
  seg->print();
  break;

 case 2:
  cout << "intersection-event in (" << getGridX() << " " << getPreciseX()
    << ", " << getGridY() << " " << getPreciseY() << ")" << endl;
  cout << "with segments: " << endl;
  lSeg->print();
  gSeg->print();
  break;
 default:
  assert(false);

 }
}

/*
 ~selectNext~

*/
template<class C1, class C2>
Owner selectNext(const C1& l1, int& pos1, const C2& l2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {
 Coordinate* values[3];
 SegmentData* sd1 = new SegmentData();
 SegmentData* sd2 = new SegmentData();
 Event e1, e2;
 Coordinate c0, c1, c2;
 bool found = false;
 int number = 0; // number of available values

 while (pos1 < l1.Size() && !found) {
  l1.get(pos1, *sd1);
  if (sd1->IsLeftDomPoint()) {
   c0.gx = l1.getLeftGridX(pos1);
   c0.px = l1.getPreciseLeftX(pos1);
   c0.gy = l1.getLeftGridY(pos1);
   c0.py = l1.getPreciseLeftY(pos1);
   values[0] = &c0;
   number++;
   found = true;
  }
  pos1++;
 }
 if (!found) {
  values[0] = 0;
 }
 found = false;
 while (pos2 < l2.Size() && !found) {
  l2.get(pos2, *sd2);

  if (sd2->IsLeftDomPoint()) {
   c1.gx = l2.getLeftGridX(pos2);
   c1.px = l2.getPreciseLeftX(pos2);
   c1.gy = l2.getLeftGridY(pos2);
   c1.py = l2.getPreciseLeftY(pos2);
   values[1] = &c1;
   number++;
   found = true;
  }
  pos2++;
 }
 if (!found) {
  values[1] = 0;
 }
 if (q.empty()) {
  values[2] = 0;
 } else {
  e1 = q.top();
  while (!e1.isValid()) {
   q.pop();
   e1 = q.top();
  }
  c2.gx = e1.getGridX();
  c2.px = e1.getPreciseX();
  c2.gy = e1.getGridY();
  c2.py = e1.getPreciseY();
  values[2] = &c2;
  number++;
 }

 if (number == 0) {
  // no halfsegments found
  return none;
 }

// search for the minimum.
 int index = -1;
 for (int i = 0; i < 3; i++) {
  if (values[i]) {
   if (index < 0 || (*values[index] > *values[i])) {
    index = i;
   }
  }
 }
 switch (index) {
 case 0: {
  if (values[1] != 0) {
   pos2--;
  }
  AVLSegment* seg1 = new AVLSegment(l1.getPreciseCoordinates(), sd1, first);
  Event* result1 = new Event(leftEndpoint, seg1);
  event = *result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(l2.getPreciseCoordinates(), sd2, second);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  return second;
 }
 case 2: {
  if (values[0] != 0) {
   pos1--;
  }
  if (values[1] != 0) {
   pos2--;
  }
  if (q.empty()) {
  } else {
   q.pop();
  }
  event = e1;
  if (e1.isIntersectionEvent()) {
   return both;
  } else {
   return e1.getSegment()->getOwner();
  }
 }
 default:
  assert(false);
 }
 return none;
}

Owner selectNext(const Line2& l, int& pos1, const Line2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {
 return selectNext<Line2, Line2>(l, pos1, r, pos2, q, event);
}

Owner selectNext(const Region2& r1, int& pos1, const Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {

 Coordinate* values[3];

 GridHalfSegment gs1;
 GridHalfSegment gs2;
 PrecHalfSegment ps1;
 PrecHalfSegment ps2;
 Event e1, e2;
 Coordinate c0, c1, c2;
 bool found = false;
 int number = 0; // number of available values

 while (pos1 < r1.Size() && !found) {
  r1.getgridCoordinates()->Get(pos1, gs1);
  r1.getprecCoordinates()->Get(pos1, ps1);

  if (gs1.IsLeftDomPoint()) {
   c0.gx = gs1.GetLeftPointX();
   c0.px = ps1.GetlPointx(r1.getpreciseCoordinates());
   c0.gy = gs1.GetLeftPointY();
   c0.py = ps1.GetlPointy(r1.getpreciseCoordinates());
   values[0] = &c0;
   number++;
   found = true;
  }
  pos1++;
 }
 if (!found) {
  values[0] = 0;
 }
 found = false;
 while (pos2 < r2.Size() && !found) {
  r2.getgridCoordinates()->Get(pos2, gs2);
  r2.getprecCoordinates()->Get(pos2, ps2);
  if (gs2.IsLeftDomPoint()) {
   c1.gx = gs2.GetLeftPointX();
   c1.px = ps2.GetlPointx(r2.getpreciseCoordinates());
   c1.gy = gs2.GetLeftPointY();
   c1.py = ps2.GetlPointy(r2.getpreciseCoordinates());
   values[1] = &c1;
   number++;
   found = true;
  }
  pos2++;
 }
 if (!found) {
  values[1] = 0;
 }
 if (q.empty()) {
  values[2] = 0;
 } else {
  e1 = q.top();
  while (!e1.isValid()) {
   q.pop();
   e1 = q.top();
  }
  c2.gx = e1.getGridX();
  c2.px = e1.getPreciseX();
  c2.gy = e1.getGridY();
  c2.py = e1.getPreciseY();
  values[2] = &c2;
  number++;
 }

 if (number == 0) {
  // no halfsegments found
  return none;
 }

// search for the minimum.
 int index = -1;
 for (int i = 0; i < 3; i++) {
  if (values[i]) {
   if (index < 0 || (*values[index] > *values[i])) {
    index = i;
   }
  }
 }
 switch (index) {
 case 0: {
  if (values[1] != 0) {
   pos2--;
  }
  AVLSegment* seg1 = new AVLSegment(r1.getpreciseCoordinates(), gs1,
    new PrecHalfSegment(ps1), first);
  Event* result1 = new Event(leftEndpoint, seg1);
  event = *result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(r2.getpreciseCoordinates(), gs2,
    new PrecHalfSegment(ps2), second);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  return second;
 }
 case 2: {
  if (values[0] != 0) {
   pos1--;
  }
  if (values[1] != 0) {
   pos2--;
  }
  if (q.empty()) {
  } else {
   q.pop();
  }
  event = e1;
  if (e1.isIntersectionEvent()) {
   return both;
  } else {
   return e1.getSegment()->getOwner();
  }
 }
 default:
  assert(false);
 }
 return none;
}

Owner selectNext(const Line2& l, int& pos,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {
 return selectNext<Line2>(l, pos, q, event);
}

Owner selectNext(const SimpleLine2& l, int& pos,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {
 return selectNext<SimpleLine2>(l, pos, q, event);
}

/*
 ~selectNext~

*/
template<class C>
Owner selectNext(const C& l, int& pos,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {
 Coordinate* values[2];
 Event e1;
 Coordinate c0, c1;
 SegmentData* sd1 = new SegmentData();
 int number = 0; // number of available values

 bool found = false;
 values[0] = 0;
 while (pos < l.Size() && !found) {
  l.get(pos, *sd1);

  if (sd1->IsLeftDomPoint()) {
   c0.gx = l.getLeftGridX(pos);
   c0.px = l.getPreciseLeftX(pos);
   c0.gy = l.getLeftGridY(pos);
   c0.py = l.getPreciseLeftY(pos);
   values[0] = &c0;
   number++;
   found = true;
  }
  pos++;
 }

 if (q.empty()) {
  values[1] = 0;
 } else {
  e1 = q.top();
  c1.gx = e1.getGridX();
  c1.px = e1.getPreciseX();
  c1.gy = e1.getGridY();
  c1.py = e1.getPreciseY();
  values[1] = &c1;
  number++;
 }

 if (number == 0) {
  // no halfsegments found
  return none;
 }

// search for the minimum.
 int index = -1;
 for (int i = 0; i < 2; i++) {
  if (values[i]) {
   if (index < 0 || (*values[index] > *values[i])) {
    index = i;
   }
  }
 }

 switch (index) {
 case 0: {
  AVLSegment* result = new AVLSegment(l.getPreciseCoordinates(), sd1, first);
  Event* res = new Event(leftEndpoint, result);
  event = *res;
  return first;
 }
 case 1: {
  pos--;
  q.pop();
  event = e1;
  return first;
 }
 default: {
  assert(false);
 }
 }
 return none;
}

/*
 ~mergeNeighbors~

*/
bool mergeNeighbors(AVLSegment* current, AVLSegment* neighbor) {

 current->changeValidity(false);
 if (neighbor->getGridXR() > current->getGridXR()
   || (neighbor->getGridXR() == current->getGridXR()
     && cmp(neighbor->getPreciseXR(), current->getPreciseXR()) > 0)
   || ((neighbor->getGridXR() == current->getGridXR())
     && (cmp(neighbor->getPreciseXR(), current->getPreciseXR()) == 0)
     && (neighbor->getGridYR() > current->getGridYR()))
   || ((neighbor->getGridXR() == current->getGridXR()
     && cmp(neighbor->getPreciseXR(), current->getPreciseXR()) == 0)
     && ((neighbor->getGridYR() == current->getGridYR())
       && cmp(neighbor->getPreciseYR(), current->getPreciseYR()) > 0))) {
  //neighbor is longer -> nothing to do
  return false;

 } else {
  AVLSegment* newNeighbor = new AVLSegment(neighbor->getGridXL(),
    neighbor->getGridYL(), current->getGridXR(), current->getGridYR(),
    neighbor->getPreciseXL(), neighbor->getPreciseYL(), current->getPreciseXR(),
    current->getPreciseYR(), neighbor->getOwner());

  newNeighbor->setNumberOfChanges(neighbor->getNumberOfChanges() + 1);
  *neighbor = *newNeighbor;
  delete newNeighbor;

  return true;
 }

}

/*
 ~splitNeighbors~

*/
void splitNeighbors(AVLSegment* current, AVLSegment* neighbor,
  AVLSegment* overlappingSegment, AVLSegment* right) {

 //r starts at he end of overlappingSegment. If neighbor is longer
 //than current, r ends in the right endpoint of neighbor
 AVLSegment* r;
 if (neighbor->getGridXR() > current->getGridXR()
   || (neighbor->getGridXR() == current->getGridXR()
     && cmp(neighbor->getPreciseXR(), current->getPreciseXR()) > 0)
   || ((neighbor->getGridXR() == current->getGridXR())
     && (cmp(neighbor->getPreciseXR(), current->getPreciseXR()) == 0)
     && (neighbor->getGridYR() > current->getGridYR()))
   || ((neighbor->getGridXR() == current->getGridXR()
     && cmp(neighbor->getPreciseXR(), current->getPreciseXR()) == 0)
     && ((neighbor->getGridYR() == current->getGridYR())
       && cmp(neighbor->getPreciseYR(), current->getPreciseYR()) > 0))) {
  //neighbor is longer
  r = new AVLSegment(overlappingSegment->getGridXR(),
    overlappingSegment->getGridYR(), neighbor->getGridXR(),
    neighbor->getGridYR(), overlappingSegment->getPreciseXR(),
    overlappingSegment->getPreciseYR(), neighbor->getPreciseXR(),
    neighbor->getPreciseYR(), neighbor->getOwner());

  r->setConAbove(neighbor->getConAbove());
  r->setConBelow(neighbor->getConBelow());
  r->setFirstInsideAbove(neighbor->getFirstInsideAbove());
  r->setSecondInsideAbove(neighbor->getSecondInsideAbove());

 } else {
  r = new AVLSegment(overlappingSegment->getGridXR(),
    overlappingSegment->getGridYR(), current->getGridXR(), current->getGridYR(),
    overlappingSegment->getPreciseXR(), overlappingSegment->getPreciseYR(),
    current->getPreciseXR(), current->getPreciseYR(), current->getOwner());

  r->setConAbove(current->getConAbove());
  r->setConBelow(current->getConBelow());
  r->setFirstInsideAbove(current->getFirstInsideAbove());
  r->setSecondInsideAbove(current->getSecondInsideAbove());

 }
 *right = *r;
 delete r;
 if (overlappingSegment->hasEqualLeftEndpointAs(*neighbor)) {
  // the overlapping part starts at the left endpoint
  // of ~neighbor~. ~neighbor~ is truncated to the
  // length of the overlapping part and ~current~ is
  // set to invalid, and will be removed later.

  current->changeValidity(false);

  AVLSegment newNeighbor(overlappingSegment->getGridXL(),
    overlappingSegment->getGridYL(), overlappingSegment->getGridXR(),
    overlappingSegment->getGridYR(), overlappingSegment->getPreciseXL(),
    overlappingSegment->getPreciseYL(), overlappingSegment->getPreciseXR(),
    overlappingSegment->getPreciseYR(), both);

  if (neighbor->getOwner() == first) {
   newNeighbor.setFirstInsideAbove(neighbor->getFirstInsideAbove());
   newNeighbor.setSecondInsideAbove(current->getSecondInsideAbove());
   newNeighbor.setConAbove(
     neighbor->getFirstInsideAbove() + current->getSecondInsideAbove());
   newNeighbor.setConBelow(
     (!neighbor->getFirstInsideAbove()) + (!current->getSecondInsideAbove()));
  } else {
   newNeighbor.setFirstInsideAbove(current->getFirstInsideAbove());
   newNeighbor.setSecondInsideAbove(neighbor->getSecondInsideAbove());
   newNeighbor.setConAbove(
     neighbor->getSecondInsideAbove() + current->getFirstInsideAbove());
   newNeighbor.setConBelow(
     (!neighbor->getSecondInsideAbove()) + (!current->getFirstInsideAbove()));
  }

  newNeighbor.setNumberOfChanges(neighbor->getNumberOfChanges() + 1);
  *neighbor = newNeighbor;
 } else {
  AVLSegment newCurrent(overlappingSegment->getGridXL(),
    overlappingSegment->getGridYL(), overlappingSegment->getGridXR(),
    overlappingSegment->getGridYR(), overlappingSegment->getPreciseXL(),
    overlappingSegment->getPreciseYL(), overlappingSegment->getPreciseXR(),
    overlappingSegment->getPreciseYR(), both);

  if (neighbor->getOwner() == first) {
   newCurrent.setFirstInsideAbove(neighbor->getFirstInsideAbove());
   newCurrent.setSecondInsideAbove(current->getSecondInsideAbove());
   newCurrent.setConAbove(
     neighbor->getFirstInsideAbove() + current->getSecondInsideAbove());
   newCurrent.setConBelow(
     (!neighbor->getFirstInsideAbove()) + (!current->getSecondInsideAbove()));
  } else {
   newCurrent.setFirstInsideAbove(current->getFirstInsideAbove());
   newCurrent.setSecondInsideAbove(neighbor->getSecondInsideAbove());
   newCurrent.setConAbove(
     neighbor->getSecondInsideAbove() + current->getFirstInsideAbove());
   newCurrent.setConBelow(
     (!neighbor->getSecondInsideAbove()) + (!current->getFirstInsideAbove()));
  }

  *current = newCurrent;

  AVLSegment newNeighbor(neighbor->getGridXL(), neighbor->getGridYL(),
    overlappingSegment->getGridXL(), overlappingSegment->getGridYL(),
    neighbor->getPreciseXL(), neighbor->getPreciseYL(),
    overlappingSegment->getPreciseXL(), overlappingSegment->getPreciseYL(),
    neighbor->getOwner());

  newNeighbor.setConAbove(neighbor->getConAbove());
  newNeighbor.setConBelow(neighbor->getConBelow());
  if (neighbor->getOwner() == first) {
   newNeighbor.setFirstInsideAbove(neighbor->getFirstInsideAbove());
   newNeighbor.setSecondInsideAbove(current->getSecondInsideAbove());
  } else {
   newNeighbor.setFirstInsideAbove(current->getFirstInsideAbove());
   newNeighbor.setSecondInsideAbove(neighbor->getSecondInsideAbove());
  }

  newNeighbor.setNumberOfChanges(neighbor->getNumberOfChanges() + 1);
  *neighbor = newNeighbor;
 }

}

/*
 ~intersectionTestForRealminize~

*/
void intersectionTestForRealminize(AVLSegment* left, AVLSegment* right,
  Event* event, priority_queue<Event, vector<Event>, greater<Event> >& q,
  bool leftIsSmaller) {

 if (left->mightIntersect(*right)) {
  AVLSegment* overlappingSegment = new AVLSegment();
  if (left->intersect(*right, *overlappingSegment)) {
   if (p2d_debug) {
    cout << "s1: ";
    left->print();
    cout << "s2: ";
    right->print();
    cout << "Schnittpunkt :";
    overlappingSegment->print();
   }
   if (overlappingSegment->isPoint()) {
    if (!overlappingSegment->isLeftOf(*event)
      && (!(overlappingSegment->isLeftPointOf(*left)
        && overlappingSegment->isRightPointOf(*right)))) {
     if (leftIsSmaller) {
      Event ie(intersectionPoint, left, right, overlappingSegment);
      q.push(ie);
     } else {
      Event ie(intersectionPoint, right, left, overlappingSegment);
      q.push(ie);
     }
    }
   } else {
    bool newNeighbor = mergeNeighbors(left, right);
    if (newNeighbor) {
     Event e(rightEndpoint, right);
     q.push(e);
    }
   }
  }
  delete overlappingSegment;
 }
}

/*
 ~intersectionTestForSetOp~

*/
bool intersectionTestForSetOp(AVLSegment* s1, AVLSegment* s2, Event* event,
  priority_queue<Event, vector<Event>, greater<Event> >& q,
  bool leftIsSmaller) {

 if (s1->mightIntersect(*s2)) {
  AVLSegment* overlappingSegment = new AVLSegment();
  if (s1->intersect(*s2, *overlappingSegment)) {
   if (p2d_debug) {
    cout << "s1: ";
    s1->print();
    cout << "s2: ";
    s2->print();
    cout << "Schnittpunkt :";
    overlappingSegment->print();
   }
   if (overlappingSegment->isPoint()) {
    if (!overlappingSegment->isLeftOf(*event)) {
     if (leftIsSmaller) {
      Event ie(intersectionPoint, s1, s2, overlappingSegment);
      q.push(ie);
     } else {
      Event ie(intersectionPoint, s2, s1, overlappingSegment);
      q.push(ie);
     }
    }
   } else {
    AVLSegment* right = new AVLSegment();
    splitNeighbors(s1, s2, overlappingSegment, right);

    if (s1->getOwner() == both) {

     Event e1(rightEndpoint, s1);
     q.push(e1);
    }
    if (leftIsSmaller) {
     Event ie2(intersectionPoint, s1, s2, overlappingSegment);
     q.push(ie2);
    } else {
     Event ie2(intersectionPoint, s2, s1, overlappingSegment);
     q.push(ie2);
    }

    Event e2(rightEndpoint, s2);
    q.push(e2);

    if (!right->isPoint()) {
     Event e3(leftEndpoint, right);
     q.push(e3);
    } else {

     delete right;
    }
   }
   delete overlappingSegment;
   return true;
  }
  delete overlappingSegment;
 }
 return false;
}

/*
 ~collectSegmentsForInverting~

*/
void collectSegmentsForInverting(vector<AVLSegment*>& segmentVector,
  Event& event, priority_queue<Event, vector<Event>, greater<Event> >& q,
  size_t& predIndex, size_t& sucIndex, bool& inversionNecessary) {
 mpq_class v = event.getPreciseX();

 AVLSegment* seg = event.getLeftSegment();
 AVLSegment* r = event.getRightSegment();
 segmentVector.push_back(seg);
 Event e = q.top();

 while (e.isIntersectionEvent() && e.getGridX() == event.getGridX()
   && e.getGridY() == event.getGridY()
   && cmp(e.getPreciseX(), event.getPreciseX()) == 0
   && cmp(e.getPreciseY(), event.getPreciseY()) == 0) {
  if (e.isValid()) {

   if (seg->compareInPosBackward(*(e.getLeftSegment()), event.getGridX(), v)
     < 0) {
    if (segmentVector.back()->isParallelTo(*e.getLeftSegment())) {

     if (segmentVector.size() == 1) {
      predIndex = 1;
     }
     segmentVector.pop_back();
     if ((segmentVector.size() == 0)
       || (segmentVector.size() > 0
         && !(segmentVector.back()->equal(*e.getLeftSegment())))) {

      segmentVector.push_back(e.getLeftSegment());
     }
     segmentVector.push_back(seg);
     r = e.getRightSegment();

    } else {
     inversionNecessary = true;
     seg = e.getLeftSegment();
     r = e.getRightSegment();
     segmentVector.push_back(seg);
    }

   }
  }
  q.pop();
  e = q.top();
 }
 if (segmentVector.back()->compareInPosBackward(*r, event.getGridX(), v) < 0) {
  if (segmentVector.back()->isParallelTo(*r)) {
   segmentVector.pop_back();
   segmentVector.push_back(r);
   segmentVector.push_back(seg);
   sucIndex = segmentVector.size() - 2;
  } else {
   inversionNecessary = true;
   segmentVector.push_back(r);
   sucIndex = segmentVector.size() - 1;
  }
 }
}

/*
 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, Line2& result, int& edgeno,
  SetOperation op) {
 switch (op) {
 case union_op: {
  if (p2d_debug) {
   cout << "neues Segment in Line einfuegen" << endl;

   s.print();
  }
  result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
    s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
    s.getPreciseYR(), edgeno);
  result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
    s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
    s.getPreciseYR(), edgeno);
  edgeno++;
  break;
 }
 case intersection_op: {
  if (s.getOwner() == both) {
   if (p2d_debug) {
    cout << "neues Segment in Line einfuegen" << endl;

    s.print();
   }
   result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
     s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
     s.getPreciseYR(), edgeno);

   result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
     s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
     s.getPreciseYR(), edgeno);

   edgeno++;
  }
  break;
 }
 case difference_op: {
  if (s.getOwner() == first) {
   if (p2d_debug) {
    cout << "neues Segment in Line einfuegen" << endl;
    s.print();
   }
   result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
     s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
     s.getPreciseYR(), edgeno);
   result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
     s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
     s.getPreciseYR(), edgeno);
   edgeno++;
  }
  break;
 }
 default: {
  assert(false);
 }
 }
}

/*
 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  Line2& result, int& edgeno, SetOperation op) {
 for (size_t i = 0; i < segmentVector.size(); i++) {
  mpq_class px = event.getPreciseX();
  mpq_class py = event.getPreciseY();
  bool isEndpoint = segmentVector.at(i)->isEndpoint(event.getGridX(),
    event.getGridY(), px, py);
  if (!isEndpoint
    && (op == union_op
      || (op == intersection_op && segmentVector.at(i)->getOwner() == both)
      || (op == difference_op && segmentVector.at(i)->getOwner() == first))) {
   if (p2d_debug) {
    cout << "neues Segment in Line einfuegen" << endl;

    cout << segmentVector.at(i)->getGridXL() << " "
      << segmentVector.at(i)->getGridYL() << " " << event.getGridX() << " "
      << event.getGridY() << " " << segmentVector.at(i)->getPreciseXL() << " "
      << segmentVector.at(i)->getPreciseYL() << " " << event.getPreciseX()
      << " " << event.getPreciseY() << endl;
   }
   result.addSegment(true, segmentVector.at(i)->getGridXL(),
     segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
     segmentVector.at(i)->getPreciseXL(), segmentVector.at(i)->getPreciseYL(),
     event.getPreciseX(), event.getPreciseY(), edgeno);
   result.addSegment(false, segmentVector.at(i)->getGridXL(),
     segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
     segmentVector.at(i)->getPreciseXL(), segmentVector.at(i)->getPreciseYL(),
     event.getPreciseX(), event.getPreciseY(), edgeno);
   edgeno++;
   AVLSegment* newLeft = new AVLSegment(event.getGridX(), event.getGridY(),
     segmentVector.at(i)->getGridXR(), segmentVector.at(i)->getGridYR(),
     event.getPreciseX(), event.getPreciseY(),
     segmentVector.at(i)->getPreciseXR(), segmentVector.at(i)->getPreciseYR(),
     segmentVector.at(i)->getOwner());
   *(segmentVector.at(i)) = *newLeft;
  }
 }
}

/*
 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno, SetOperation op) {
 size_t size = segmentVector.size();

 AVLSegment* next = successor;
 for (size_t i = 0; i < size; i++) {
  AVLSegment* s = segmentVector.at(i);
  mpq_class px = event.getPreciseX();
  mpq_class py = event.getPreciseY();
  bool startsAtEventpoint = s->startsAtPoint(event.getGridX(), event.getGridY(),
    px, py);
  bool endsInEventpoint = s->endsInPoint(event.getGridX(), event.getGridY(), px,
    py);
  if ((!startsAtEventpoint) && (!endsInEventpoint)) {
   switch (op) {
   case union_op: {
    if ((s->getConAbove() == 0) || (s->getConBelow() == 0)) {
     if (p2d_debug) {
      cout << "speichere Segment" << endl;
      s->print();
     }
     PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
       s->getGridYL());
     PrecisePoint p2(event.getPreciseX(), event.getGridX(), event.getPreciseY(),
       event.getGridY());
     PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (s->getConBelow() == 0);
     result += hs;
     hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
     result += hs;
     edgeno++;
    }
    break;
   }
   case intersection_op: {

    if (s->getConAbove() == 2 || s->getConBelow() == 2) {
     if (p2d_debug) {
      cout << "speichere Segment" << endl;
      s->print();
     }
     PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
       s->getGridYL());
     PrecisePoint p2(event.getPreciseX(), event.getGridX(), event.getPreciseY(),
       event.getGridY());
     PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (s->getConAbove() == 2);
     result += hs;
     hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
     result += hs;
     edgeno++;
    }
    break;
   }
   case difference_op: {
    switch (s->getOwner()) {
    case first: {
     if (s->getConAbove() + s->getConBelow() == 1) {
      if (p2d_debug) {
       cout << "speichere Segment" << endl;
       s->print();
      }
      PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL());
      PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY());
      PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
      hs.attr.edgeno = edgeno;
      hs.attr.insideAbove = s->getFirstInsideAbove();
      result += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      result += hs;
      edgeno++;
     }
     break;
    }
    case second: {
     if ((s->getConAbove() == 1 && s->getConBelow() == 2)
       || (s->getConBelow() == 1 && s->getConAbove() == 2)) {
      if (p2d_debug) {
       cout << "speichere Segment" << endl;
       s->print();
      }
      PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL());
      PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY());
      PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
      hs.attr.edgeno = edgeno;
      hs.attr.insideAbove = s->getFirstInsideAbove();
      result += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      result += hs;
      edgeno++;
     }
     break;
    }
    case both: {
     if ((s->getConAbove() == 1) && (s->getConBelow() == 1)) {
      if (p2d_debug) {
       cout << "speichere Segment" << endl;
       s->print();
      }
      PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL());
      PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY());
      PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
      hs.attr.edgeno = edgeno;
      hs.attr.insideAbove = s->getFirstInsideAbove();
      result += hs;
      hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
      result += hs;
      edgeno++;
     }
     break;
    }
    default:
     assert(false);
    } // switch (s->getOwner)
    break;
   } // case difference
   default:
    assert(false);
   } //switch(op)
   AVLSegment newS(event.getGridX(), event.getGridY(), s->getGridXR(),
     s->getGridYR(), event.getPreciseX(), event.getPreciseY(),
     s->getPreciseXR(), s->getPreciseYR(), s->getOwner());
   newS.setConAbove(s->getConAbove());
   newS.setConBelow(s->getConBelow());
   newS.setFirstInsideAbove(s->getFirstInsideAbove());
   newS.setSecondInsideAbove(s->getSecondInsideAbove());
   *s = newS;
  } // (!startsAtEventpoint) && (!endsInEventpoint)
  if (!endsInEventpoint) {

   if (s->getOwner() != both) {
    if (!next) {
     s->setConAbove(0);
    } else {
     s->setConAbove(next->getConBelow());
    }
    if ((s->getOwner() == first)) {
     if (s->getFirstInsideAbove()) {
      s->setConBelow(s->getConAbove() - 1);
     } else {
      s->setConBelow(s->getConAbove() + 1);
     }
    }
    if (s->getOwner() == second) {
     if (s->getSecondInsideAbove()) {
      s->setConBelow(s->getConAbove() - 1);
     } else {
      s->setConBelow(s->getConAbove() + 1);
     }
    }

   }
   next = s;
   assert((0 <= s->getConAbove()) && (s->getConAbove() <= 2));
   assert((0 <= s->getConBelow()) && (s->getConBelow() <= 2));
  }
 } //end for
} //end intersects region x region -> bool

/*
 ~checkSegments~

*/
void checkSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, bool& result, RelationshipOperation op) {
 size_t size = segmentVector.size();

 AVLSegment* next = successor;
 for (size_t i = 0; i < size; i++) {
  AVLSegment* s = segmentVector.at(i);
  mpq_class px = event.getPreciseX();
  mpq_class py = event.getPreciseY();
  bool startsAtEventpoint = s->startsAtPoint(event.getGridX(), event.getGridY(),
    px, py);
  bool endsInEventpoint = s->endsInPoint(event.getGridX(), event.getGridY(), px,
    py);
  if ((!startsAtEventpoint) && (!endsInEventpoint)) {
   checkSegment(*s, result, op);
   AVLSegment newS(event.getGridX(), event.getGridY(), s->getGridXR(),
     s->getGridYR(), event.getPreciseX(), event.getPreciseY(),
     s->getPreciseXR(), s->getPreciseYR(), s->getOwner());
   newS.setConAbove(s->getConAbove());
   newS.setConBelow(s->getConBelow());
   newS.setFirstInsideAbove(s->getFirstInsideAbove());
   newS.setSecondInsideAbove(s->getSecondInsideAbove());
   *s = newS;
  } // (!startsAtEventpoint) && (!endsInEventpoint)
  if (!endsInEventpoint) {

   if (s->getOwner() != both) {
    if (!next) {
     s->setConAbove(0);
    } else {
     s->setConAbove(next->getConBelow());
    }
    if ((s->getOwner() == first)) {
     if (s->getFirstInsideAbove()) {
      s->setConBelow(s->getConAbove() - 1);
     } else {
      s->setConBelow(s->getConAbove() + 1);
     }
    }
    if (s->getOwner() == second) {
     if (s->getSecondInsideAbove()) {
      s->setConBelow(s->getConAbove() - 1);
     } else {
      s->setConBelow(s->getConAbove() + 1);
     }
    }

   }
   next = s;
   assert((0 <= s->getConAbove()) && (s->getConAbove() <= 2));
   assert((0 <= s->getConBelow()) && (s->getConBelow() <= 2));
  }
 }
}

/*
 ~checkSegment~

*/
void checkSegment(AVLSegment& seg, bool& result, RelationshipOperation op) {
 seg.print();
 switch (op) {
 case intersects_op: {
  if ((seg.getConAbove() == 2) || (seg.getConBelow() == 2)) {
   if (p2d_debug) {
    cout << "the regions intersect." << endl;
    seg.print();
   }
   result = true;
  }
  break;
 } //case: intersects_op
 case inside_op: {
  switch (seg.getOwner()) {
  case first: {
   if (seg.getConAbove() + seg.getConBelow() != 3) {
    if (p2d_debug) {
     cout << "r1 is not within r2" << endl;
     seg.print();
    }
    result = false;
   }
   break;
  }
  case second: {
   if ((seg.getConAbove() + seg.getConBelow()) != 1) {
    if (p2d_debug) {
     cout << "r1 is not within r2" << endl;
     seg.print();
    }
    result = false;
   }
   break;
  }
  case both: {
   if (!(((seg.getConAbove() == 2) && (seg.getConBelow() == 0))
     || ((seg.getConAbove() == 0) && (seg.getConBelow() == 2)))) {
    if (p2d_debug) {
     cout << "r1 is not within r2" << endl;
     seg.print();
    }
    result = false;
   }
   break;
  }
  default:
   assert(false);
  } // switch (seg.getOwner)
  break;
 } // case:inside_op

 default:
  assert(false);
 } //switch(op)
}

/*
 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  SetOperation op) {
 switch (op) {
 case union_op: {
  if ((s.getConAbove() == 0) || (s.getConBelow() == 0)) {
   if (p2d_debug) {
    cout << "neues Segment in Region einfuegen" << endl;
    s.print();
   }
   PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
     s.getGridYL());
   PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
     s.getGridYR());
   PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
   hs.attr.edgeno = edgeno;
   hs.attr.insideAbove = (s.getConBelow() == 0);
   result += hs;
   hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
   result += hs;
   edgeno++;
  }
  break;
 }
 case intersection_op: {
  if (s.getConAbove() == 2 || s.getConBelow() == 2) {
   if (p2d_debug) {
    cout << "neues Segment in Region einfuegen" << endl;
    s.print();
   }
   PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
     s.getGridYL());
   PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
     s.getGridYR());
   PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
   hs.attr.edgeno = edgeno;
   hs.attr.insideAbove = (s.getConAbove() == 2);
   result += hs;
   hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
   result += hs;
   edgeno++;
  }
  break;
 }
 case difference_op: {
  switch (s.getOwner()) {
  case first: {
   if (s.getConAbove() + s.getConBelow() == 1) {
    if (p2d_debug) {
     cout << "neues Segment in Region einfuegen" << endl;
     s.print();
    }
    PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL());
    PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR());
    PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
    hs.attr.edgeno = edgeno;
    hs.attr.insideAbove = s.getFirstInsideAbove();
    result += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    result += hs;
    edgeno++;
   }
   break;
  }
  case second: {
   if ((s.getConAbove() == 1 && s.getConBelow() == 2)
     || (s.getConBelow() == 1 && s.getConAbove() == 2)) {
    if (p2d_debug){
    cout << "speichere Segment" << endl;
    s.print();
    }
    PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL());
    PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR());
    PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
    hs.attr.edgeno = edgeno;
    hs.attr.insideAbove = s.getFirstInsideAbove();
    result += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    result += hs;
    edgeno++;
   }
   break;
  }
  case both: {
   if ((s.getConAbove() == 1) && (s.getConBelow() == 1)) {
    if (p2d_debug){
    cout << "speichere Segment" << endl;
    s.print();
    }
    PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL());
    PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR());
    PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
    hs.attr.edgeno = edgeno;
    hs.attr.insideAbove = s.getFirstInsideAbove();
    result += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    result += hs;
    edgeno++;
   }
   break;
  }
  default:
   assert(false);
  } // switch (s.getOwner)
  break;
 }
 default: {
  assert(false);
 }
 }
}

/*
 ~Realminize~

*/
void Realminize(const Line2& src, Line2& result, const bool forceThrow) {

 result.Clear();
 if (!src.IsDefined()) {
  result.SetDefined(false);
  return;
 }
 result.SetDefined(true);
 if (src.Size() <= 1) {
  // empty line, nothing to realminize
  return;
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;

 int pos = 0;

 AVLSegment* current = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;

 result.StartBulkLoad();
 int edgeno = 0;

 Event* event = new Event();

 while (selectNext(src, pos, q, *event) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForRealminize(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForRealminize(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      if (p2d_debug) {
       cout << "neues Segment in Line einfuegen" << endl;

       current->print();
      }
      result.addSegment(true, current->getGridXL(), current->getGridYL(),
        current->getGridXR(), current->getGridYR(), current->getPreciseXL(),
        current->getPreciseYL(), current->getPreciseXR(),
        current->getPreciseYR(), edgeno);
      result.addSegment(false, current->getGridXL(), current->getGridYL(),
        current->getGridXR(), current->getGridYR(), current->getPreciseXL(),
        current->getPreciseYL(), current->getPreciseXR(),
        current->getPreciseYR(), edgeno);
      edgeno++;
     }

     mpq_class v1 = current->getPreciseXR();
     sss.removeGetNeighbor(current, pred, suc);
     current->changeValidity(false);
     if (pred && suc) {
      intersectionTestForRealminize(pred, suc, event, q, true);
     }

    }
   }

   else {
    //intersection Event
    if (p2d_debug) {
     event->print();
    }
    mpq_class v1 = event->getPreciseX();
    mpq_class v2 = event->getPreciseY();
    //segmentVector stores all segments which intersect in the
    //same event-point
    vector<AVLSegment*> segmentVector;

    AVLSegment* seg = event->getLeftSegment();
    AVLSegment* r = event->getRightSegment();

    segmentVector.push_back(seg);
    Event e = q.top();
    size_t predIndex = 0;
    size_t sucIndex = 0;

    //if there are only 2 intersecting segments,
    //which are parallel (and the right endpoint of the first
    //segment is the left endpoint of the second segment),
    //the segments don't have to be inverted.
    bool inversionNecessary = false;

    while (e.isIntersectionEvent() && e.getGridX() == event->getGridX()
      && e.getGridY() == event->getGridY()
      && cmp(e.getPreciseX(), event->getPreciseX()) == 0
      && cmp(e.getPreciseY(), event->getPreciseY()) == 0) {
     if (seg->compareInPosBackward(*(e.getLeftSegment()), event->getGridX(), v1)
       < 0) {
      if (seg->isParallelTo(*e.getLeftSegment())) {
       if (segmentVector.size() == 1) {
        predIndex = 1;
       }
       segmentVector.pop_back();
       segmentVector.push_back(e.getLeftSegment());
       segmentVector.push_back(seg);

      } else {
       inversionNecessary = true;
       seg = e.getLeftSegment();
       r = e.getRightSegment();
       segmentVector.push_back(seg);
      }

     }
     q.pop();
     e = q.top();
    }

    if (segmentVector.back()->compareInPosBackward(*r, event->getGridX(), v1)
      < 0) {
     if (segmentVector.back()->isParallelTo(*r)) {
      segmentVector.pop_back();
      segmentVector.push_back(r);
      segmentVector.push_back(seg);
      sucIndex = segmentVector.size() - 2;
     } else {
      inversionNecessary = true;
      segmentVector.push_back(r);
      sucIndex = segmentVector.size() - 1;
     }

    }

    if (inversionNecessary) {

     left = segmentVector.at(0);
     right = segmentVector.back();
     sss.invertSegments(segmentVector, event->getGridX(), v1, event->getGridY(),
       v2, pred, predIndex, suc, sucIndex);

     if (p2d_debug && pred) {
      cout << "intersection-test for";
      right->print();
      cout << " and " << endl;
      pred->print();
     }
     if (pred) {
      intersectionTestForRealminize(pred, right, event, q, true);
     }
     if (p2d_debug && suc) {
      cout << "intersection-test for" << endl;
      left->print();
      cout << " and " << endl;
      suc->print();
     }
     if (suc) {
      intersectionTestForRealminize(left, suc, event, q, true);
     }
     mpq_class px = event->getPreciseX();
     mpq_class py = event->getPreciseY();
     for (size_t i = 0; i < segmentVector.size(); i++) {
      bool isEndpoint = segmentVector.at(i)->isEndpoint(event->getGridX(),
        event->getGridY(), px, py);
      if (!isEndpoint) {
       if (p2d_debug) {
        cout << "neues Segment in Line einfuegen" << endl;

        cout << segmentVector.at(i)->getGridXL() << " "
          << segmentVector.at(i)->getGridYL() << " " << event->getGridX() << " "
          << event->getGridY() << " "
          << segmentVector.at(i)->getPreciseXL().get_d() << " "
          << segmentVector.at(i)->getPreciseYL().get_d() << " "
          << event->getPreciseX().get_d() << " " << event->getPreciseY().get_d()
          << endl;
       }
       result.addSegment(true, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event->getGridX(), event->getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event->getPreciseX(),
         event->getPreciseY(), edgeno);
       result.addSegment(false, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event->getGridX(), event->getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event->getPreciseX(),
         event->getPreciseY(), edgeno);
       edgeno++;

       AVLSegment* newLeft = new AVLSegment(event->getGridX(),
         event->getGridY(), segmentVector.at(i)->getGridXR(),
         segmentVector.at(i)->getGridYR(), event->getPreciseX(),
         event->getPreciseY(), segmentVector.at(i)->getPreciseXR(),
         segmentVector.at(i)->getPreciseYR(), segmentVector.at(i)->getOwner());

       *(segmentVector.at(i)) = *newLeft;

      }
     }

    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 delete event;
 result.EndBulkLoad(false, false, false);
}				// end Realminize

/*
 ~Realminize~

*/
void Realminize(const SimpleLine2& src, SimpleLine2& result,
  const bool forceThrow) {

 result.Clear();
 if (!src.IsDefined()) {
  result.SetDefined(false);
  return;
 }

 if (src.Size() <= 1) {
  // empty line, nothing to realminize
  result.SetDefined(true);
  return;
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;

 int pos = 0;

 AVLSegment* current = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;

 result.StartBulkLoad();
 int edgeno = 0;

 Event* event = new Event();

 while (selectNext(src, pos, q, *event) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForRealminize(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForRealminize(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      if (p2d_debug) {
       cout << "neues Segment in Line einfuegen" << endl;

       current->print();
      }
      result.addSegment(true, current->getGridXL(), current->getGridYL(),
        current->getGridXR(), current->getGridYR(), current->getPreciseXL(),
        current->getPreciseYL(), current->getPreciseXR(),
        current->getPreciseYR(), edgeno);
      result.addSegment(false, current->getGridXL(), current->getGridYL(),
        current->getGridXR(), current->getGridYR(), current->getPreciseXL(),
        current->getPreciseYL(), current->getPreciseXR(),
        current->getPreciseYR(), edgeno);
      edgeno++;
     }

     mpq_class v1 = current->getPreciseXR();
     sss.removeGetNeighbor(current, pred, suc);
     current->changeValidity(false);
     if (pred && suc) {
      intersectionTestForRealminize(pred, suc, event, q, true);
     }

    }
   }

   else {
    //intersection Event
    if (p2d_debug) {
     event->print();
    }
    mpq_class v1 = event->getPreciseX();
    mpq_class v2 = event->getPreciseY();
    //segmentVector stores all segments which intersect in the
    //same event-point
    vector<AVLSegment*> segmentVector;

    AVLSegment* seg = event->getLeftSegment();
    AVLSegment* r = event->getRightSegment();

    segmentVector.push_back(seg);
    Event e = q.top();
    size_t predIndex = 0;
    size_t sucIndex = 0;

    //if there are only 2 intersecting segments,
    //which are parallel (and the right endpoint of the first
    //segment is the left endpoint of the second segment),
    //the segments don't have to be inverted.
    bool inversionNecessary = false;

    while (e.isIntersectionEvent() && e.getGridX() == event->getGridX()
      && e.getGridY() == event->getGridY()
      && cmp(e.getPreciseX(), event->getPreciseX()) == 0
      && cmp(e.getPreciseY(), event->getPreciseY()) == 0) {
     if (seg->compareInPosBackward(*(e.getLeftSegment()), event->getGridX(), v1)
       < 0) {
      if (seg->isParallelTo(*e.getLeftSegment())) {
       if (segmentVector.size() == 1) {
        predIndex = 1;
       }
       segmentVector.pop_back();
       segmentVector.push_back(e.getLeftSegment());
       segmentVector.push_back(seg);

      } else {
       inversionNecessary = true;
       seg = e.getLeftSegment();
       r = e.getRightSegment();
       segmentVector.push_back(seg);
      }

     }
     q.pop();
     e = q.top();
    }

    if (segmentVector.back()->compareInPosBackward(*r, event->getGridX(), v1)
      < 0) {
     if (segmentVector.back()->isParallelTo(*r)) {
      segmentVector.pop_back();
      segmentVector.push_back(r);
      segmentVector.push_back(seg);
      sucIndex = segmentVector.size() - 2;
     } else {
      inversionNecessary = true;
      segmentVector.push_back(r);
      sucIndex = segmentVector.size() - 1;
     }

    }

    if (inversionNecessary) {

     left = segmentVector.at(0);
     right = segmentVector.back();
     sss.invertSegments(segmentVector, event->getGridX(), v1, event->getGridY(),
       v2, pred, predIndex, suc, sucIndex);

     if (p2d_debug && pred) {
      cout << "intersection-Test for" << endl;
      right->print();
      cout << " and " << endl;
      pred->print();
     }
     if (pred) {
      intersectionTestForRealminize(pred, right, event, q, true);
     }
     if (p2d_debug && suc) {
      cout << "intersection-Test for" << endl;
      left->print();
      cout << " and " << endl;
      suc->print();
     }
     if (suc) {
      intersectionTestForRealminize(left, suc, event, q, true);
     }
     mpq_class px = event->getPreciseX();
     mpq_class py = event->getPreciseY();
     for (size_t i = 0; i < segmentVector.size(); i++) {
      bool isEndpoint = segmentVector.at(i)->isEndpoint(event->getGridX(),
        event->getGridY(), px, py);
      if (!isEndpoint) {
       if (p2d_debug) {
        cout << "neues Segment in Line einfuegen" << endl;

        cout << segmentVector.at(i)->getGridXL() << " "
          << segmentVector.at(i)->getGridYL() << " " << event->getGridX() << " "
          << event->getGridY() << " "
          << segmentVector.at(i)->getPreciseXL().get_d() << " "
          << segmentVector.at(i)->getPreciseYL().get_d() << " "
          << event->getPreciseX().get_d() << " " << event->getPreciseY().get_d()
          << endl;
       }
       result.addSegment(true, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event->getGridX(), event->getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event->getPreciseX(),
         event->getPreciseY(), edgeno);
       result.addSegment(false, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event->getGridX(), event->getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event->getPreciseX(),
         event->getPreciseY(), edgeno);
       edgeno++;

       AVLSegment* newLeft = new AVLSegment(event->getGridX(),
         event->getGridY(), segmentVector.at(i)->getGridXR(),
         segmentVector.at(i)->getGridYR(), event->getPreciseX(),
         event->getPreciseY(), segmentVector.at(i)->getPreciseXR(),
         segmentVector.at(i)->getPreciseYR(), segmentVector.at(i)->getOwner());

       *(segmentVector.at(i)) = *newLeft;

      }
     }

    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 delete event;
 if (!result.EndBulkLoad(true, false, false)) {
  result.Clear();
  result.SetDefined(false);
 } else {
  result.setStartSmaller(src.startsSmaller());
 }
}    // end Realminize

/*
 ~SetOp~

*/
void SetOp(const Line2& line1, const Line2& line2, Line2& result,
  SetOperation op, const Geoid* geoid/*=0*/) {

 result.Clear();

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!line1.IsDefined() || !line2.IsDefined()
   || (geoid && !geoid->IsDefined())) {
  result.SetDefined(false);
  return;
 }
 result.SetDefined(true);
 if (line1.Size() == 0) {
  switch (op) {
  case union_op:
   result = line2;
   return;
  case intersection_op:
   return; // empty line
  case difference_op:
   return; // empty line
  default:
   assert(false);
  }
 }
 if (line2.Size() == 0) {
  switch (op) {
  case union_op:
   result = line1;
   return;
  case intersection_op:
   return;
  case difference_op:
   result = line1;
   return;
  default:
   assert(false);
  }
 }

 if (!line1.BoundingBox().Intersects(line2.BoundingBox(), geoid)) {
  switch (op) {
  case avlseg::union_op: {
   result.StartBulkLoad();
   int edgeno = 0;
   int s = line1.Size();
   SegmentData sd;
   const Flob* flob = line1.getPreciseCoordinates();
   for (int i = 0; i < s; i++) {
    line1.get(i, sd);
    if (sd.IsLeftDomPoint()) {
     result.addSegment(true, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(flob), sd.getPreciseLeftY(flob),
       sd.getPreciseRightX(flob), sd.getPreciseRightY(flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(flob), sd.getPreciseLeftY(flob),
       sd.getPreciseRightX(flob), sd.getPreciseRightY(flob), edgeno);
     edgeno++;
    }

   }
   s = line2.Size();
   for (int i = 0; i < s; i++) {
    line2.get(i, sd);
    if (sd.IsLeftDomPoint()) {
     result.addSegment(true, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(flob), sd.getPreciseLeftY(flob),
       sd.getPreciseRightX(flob), sd.getPreciseRightY(flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(flob), sd.getPreciseLeftY(flob),
       sd.getPreciseRightX(flob), sd.getPreciseRightY(flob), edgeno);
     edgeno++;
    }

   }
   result.EndBulkLoad();
   return;
  }
  case avlseg::difference_op: {
   result = line1;
   return;
  }
  case avlseg::intersection_op: {
   return;
  }
  default:
   assert(false);
  }
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;
 AVLSegment nextHs;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 int edgeno = 0;

 Event* event = new Event();

 result.StartBulkLoad();
 while ((selectNext(line1, pos1, line2, pos2, q, *event)) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      createNewSegments(*current, result, edgeno, op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);

      }

     } else {
      mpq_class v1 = event->getPreciseX();
      sss.removeGetNeighbor2(current, event->getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    }
    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event->isValid()) {
     if (p2d_debug) {
      event->print();
     }
     mpq_class v1 = event->getPreciseX();
     mpq_class v2 = event->getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event->getGridX(), v1,
        event->getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test with" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       intersectionTestForSetOp(pred, right, event, q, true);
      }
      if (p2d_debug && suc) {
       cout << "intersection-test with" << endl;
       left->print();
       cout << " and " << endl;
       suc->print();
      }
      if (suc) {
       intersectionTestForSetOp(left, suc, event, q, true);
      }
      mpq_class px = event->getPreciseX();
      mpq_class py = event->getPreciseY();

      createNewSegments(segmentVector, *event, result, edgeno, op);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 delete event;
 result.EndBulkLoad(true, false);
} // setop line2 x line2 -> line2

/*
 ~intersects~

*/
bool intersects(const Line2& line1, const Line2& line2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!line1.IsDefined() || !line2.IsDefined()
   || (geoid && !geoid->IsDefined())) {
  return false;
 }

 if (line1.Size() == 0) {
  return false;
 }
 if (line2.Size() == 0) {
  return false;
 }

 if (!line1.BoundingBox().Intersects(line2.BoundingBox(), geoid)) {
  return false;
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current = NULL;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 Event* event = new Event();

 bool intersect = false;

 while ((!intersect
   && (selectNext(line1, pos1, line2, pos2, q, *event)) != none)) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    if (intersectionTestForSetOp(current, pred, event, q, false)) {
     if ((pred->getOwner() != current->getOwner())) {
      intersect = true;
     }
    }
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     if (intersectionTestForSetOp(current, suc, event, q, true)) {
      if ((suc->getOwner() != current->getOwner())) {
       intersect = true;
      }
     }
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       if (intersectionTestForSetOp(pred, suc, event, q, true)) {
        if ((pred->getOwner() != suc->getOwner())) {
         intersect = true;
        }
       }

      }

     } else {
      mpq_class v1 = event->getPreciseX();
      sss.removeGetNeighbor2(current, event->getGridX(), v1, pred, suc);
      if (pred && suc) {
       if (intersectionTestForSetOp(pred, suc, event, q, true)) {
        if (pred->getOwner() != suc->getOwner()) {
         intersect = true;
        }
       }
      }
     }
    }
    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event->isValid()) {
     if (p2d_debug) {
      event->print();
     }
     mpq_class v1 = event->getPreciseX();
     mpq_class v2 = event->getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event->getGridX(), v1,
        event->getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       if (intersectionTestForSetOp(pred, right, event, q, true)) {
        if (pred->getOwner() != right->getOwner()) {
         intersect = true;
        }
       }
      }
      if (p2d_debug && suc) {
       cout << "intersection-test for" << endl;
       left->print();
       cout << " and " << endl;
       suc->print();
      }
      if (suc) {
       if (intersectionTestForSetOp(left, suc, event, q, true)) {
        if (suc->getOwner() != left->getOwner()) {
         intersect = true;
        }
       }
      }
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 delete event;
 return intersect;
} // intersects line2 x line2 -> bool

/*

 ~crossings~

*/
void crossings(const Line2& line1, const Line2& line2, Points2& result,
  const Geoid* geoid/*=0*/) {
 result.Clear();

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!line1.IsDefined() || !line2.IsDefined()
   || (geoid && !geoid->IsDefined())) {
  result.SetDefined(false);
  return;
 }

 if (line1.Size() == 0) {
  return;
 }
 if (line2.Size() == 0) {
  return;
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 Event* event = new Event();

 while (selectNext(line1, pos1, line2, pos2, q, *event) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();

   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }
   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     } else {

     }
    }
   }

   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {

     mpq_class v1 = current->getPreciseXR();
     sss.removeGetNeighbor(current, pred, suc);
     current->changeValidity(false);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }

    }

    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (p2d_debug) {
     event->print();
    }

    mpq_class v1 = event->getPreciseX();
    mpq_class v2 = event->getPreciseY();
    vector<AVLSegment*> segmentVector;

    size_t predIndex = 0;
    size_t sucIndex = 0;
    bool inversionNecessary = false;

    collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
      inversionNecessary);

    bool hasSegOfArg1 = false;
    bool hasSegOfArg2 = false;
    size_t i = 0;
    size_t size = segmentVector.size();

    while (!(hasSegOfArg1 && hasSegOfArg2) && i < size) {
     if (segmentVector.at(i)->getOwner() == first) {
      hasSegOfArg1 = true;
     }
     if (segmentVector.at(i)->getOwner() == second) {
      hasSegOfArg2 = true;
     }
     i++;
    }

    if (hasSegOfArg1 && hasSegOfArg2) {
     result.addPoint(
       new Point2(true, event->getGridX(), event->getGridY(),
         event->getPreciseX(), event->getPreciseY()));
    }

    if (inversionNecessary) {
     left = segmentVector.at(0);
     right = segmentVector.back();
     sss.invertSegments(segmentVector, event->getGridX(), v1, event->getGridY(),
       v2, pred, predIndex, suc, sucIndex);

     if (p2d_debug && pred) {
      cout << "intersection-test for";
      right->print();
      cout << " and " << endl;
      pred->print();

     }
     if (pred) {
      intersectionTestForSetOp(pred, right, event, q, true);
     }
     if (p2d_debug && suc) {
      cout << "intersection-test for";
      left->print();
      cout << " and " << endl;
      suc->print();
     }
     if (suc) {
      intersectionTestForSetOp(left, suc, event, q, true);
     }
     mpq_class px = event->getPreciseX();
     mpq_class py = event->getPreciseY();

    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 delete event;
} // crossings line2 x line2 -> points2

/*
 ~SetOp~

*/
void SetOp(const Region2& reg1, const Region2& reg2, Region2& result,
  SetOperation op, const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }
 result.Clear();
 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())) {
  result.SetDefined(false);
  return;
 }
 result.SetDefined(true);
 if (reg1.Size() == 0) {
  switch (op) {
  case avlseg::union_op:
   result = reg2;
   return;
  case avlseg::intersection_op:
   return; // empty region
  case avlseg::difference_op:
   return; // empty region
  default:
   assert(false);
  }
 }
 if (reg2.Size() == 0) {
  switch (op) {
  case avlseg::union_op:
   result = reg1;
   return;
  case avlseg::intersection_op:
   return;
  case avlseg::difference_op:
   result = reg1;
   return;
  default:
   assert(false);
  }
 }

 if (!reg1.BoundingBox().Intersects(reg2.BoundingBox(), geoid)) {
  switch (op) {
  case avlseg::union_op: {
   result.StartBulkLoad();
   int edgeno = 0;
   int s = reg1.Size();
   GridHalfSegment gs;
   PreciseHalfSegment ps;
   for (int i = 0; i < s; i++) {
    reg1.getgridCoordinates()->Get(i, gs);
    reg1.Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     PrecisePoint p1(ps.GetLeftPoint().x, gs.GetLeftPointX(),
       ps.GetLeftPoint().y, gs.GetLeftPointY());
     PrecisePoint p2(ps.GetRightPoint().x, gs.GetRightPointX(),
       ps.GetRightPoint().y, gs.GetRightPointY());
     PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(!gs.IsLeftDomPoint());
     result += hs;
     edgeno++;
    }
   }
   s = reg2.Size();
   for (int i = 0; i < s; i++) {
    reg2.getgridCoordinates()->Get(i, gs);
    reg2.Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     PrecisePoint p1(ps.GetLeftPoint().x, gs.GetLeftPointX(),
       ps.GetLeftPoint().y, gs.GetLeftPointY());
     PrecisePoint p2(ps.GetRightPoint().x, gs.GetRightPointX(),
       ps.GetRightPoint().y, gs.GetRightPointY());
     PreciseHalfSegment hs = PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(!gs.IsLeftDomPoint());
     result += hs;
     edgeno++;
    }
   }
   result.EndBulkLoad();
   return;
  }
  case avlseg::difference_op: {
   result = reg1;
   return;
  }
  case avlseg::intersection_op: {
   return;
  }
  default:
   assert(false);
  }
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;

 int pos1 = 0;
 int pos2 = 0;

 result.StartBulkLoad();
 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 int edgeno = 0;

 Event* event = new Event();

 result.StartBulkLoad();

 while ((selectNext(reg1, pos1, reg2, pos2, q, *event)) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      createNewSegments(*current, result, edgeno, op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);

      }

     } else {
      mpq_class v1 = event->getPreciseX();
      sss.removeGetNeighbor2(current, event->getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    }
    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }

   } else {
    //intersection Event
    if (event->isValid()) {
     if (p2d_debug) {
      event->print();
     }
     mpq_class v1 = event->getPreciseX();
     mpq_class v2 = event->getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event->getGridX(), v1,
        event->getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       intersectionTestForSetOp(pred, right, event, q, true);
      }
      if (p2d_debug && suc) {
       cout << "intersection-test for" << endl;
       left->print();
       cout << " and " << endl;
       suc->print();
      }
      if (suc) {
       intersectionTestForSetOp(left, suc, event, q, true);
      }
      mpq_class px = event->getPreciseX();
      mpq_class py = event->getPreciseY();

      createNewSegments(segmentVector, *event, suc, result, edgeno, op);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 delete event;
 result.EndBulkLoad();

} // setOP region2 x region2 -> region2

/*
 ~intersects~

*/
bool intersects(const Region2& reg1, const Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())) {
  return false;
 }

 if (reg1.Size() == 0) {
  return false;
 }
 if (reg2.Size() == 0) {
  return false;
 }

 if (!reg1.BoundingBox().Intersects(reg2.BoundingBox(), geoid)) {
  return false;
 }

 assert(reg1.IsOrdered());
 assert(reg2.IsOrdered());

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* right = NULL;
 AVLSegment* left = NULL;

 Event* event = new Event();

 bool intersect = false;

 while (!intersect && (selectNext(reg1, pos1, reg2, pos2, q, *event)) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      checkSegment(*current, intersect, intersects_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }

     } else {
      mpq_class v1 = event->getPreciseX();
      sss.removeGetNeighbor2(current, event->getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    }
    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event->isValid()) {
     if (p2d_debug) {
      event->print();
     }
     mpq_class v1 = event->getPreciseX();
     mpq_class v2 = event->getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event->getGridX(), v1,
        event->getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       intersectionTestForSetOp(pred, right, event, q, true);
      }
      if (p2d_debug && suc) {
       cout << "intersection-test for" << endl;
       left->print();
       cout << " and " << endl;
       suc->print();
      }
      if (suc) {
       intersectionTestForSetOp(left, suc, event, q, true);
      }
      mpq_class px = event->getPreciseX();
      mpq_class py = event->getPreciseY();

      checkSegments(segmentVector, *event, suc, intersect, intersects_op);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 delete event;

 return intersect;
} // intersect region2 x region2 -> bool

/*
 ~inside~

*/
bool inside(const Region2& reg1, const Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())) {
  return false;
 }

 if (reg1.Size() == 0) {
  return true;
 }
 if (reg2.Size() == 0) {
  return false;
 }

 if (!reg2.BoundingBox().Contains(reg1.BoundingBox(), geoid)) {
  return false;
 }

 assert(reg1.IsOrdered());
 assert(reg2.IsOrdered());

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* right = NULL;
 AVLSegment* left = NULL;

 Event* event = new Event();

 bool inside = true;

 while (inside && (selectNext(reg1, pos1, reg2, pos2, q, *event)) != none) {
  if (event->isLeftEndpointEvent()) {
   if (p2d_debug) {
    event->print();
   }
   current = event->getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event->getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event->getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event->isRightEndpointEvent()) {
    if (p2d_debug) {
     event->print();
    }
    current = event->getSegment();
    if (event->isValid()) {
     if (current->isValid()) {
      checkSegment(*current, inside, inside_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }

     } else {
      mpq_class v1 = event->getPreciseX();
      sss.removeGetNeighbor2(current, event->getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    }
    if (event->getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event->isValid()) {
     if (p2d_debug) {
      event->print();
     }
     mpq_class v1 = event->getPreciseX();
     mpq_class v2 = event->getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, *event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event->getGridX(), v1,
        event->getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       intersectionTestForSetOp(pred, right, event, q, true);
      }
      if (p2d_debug && suc) {
       cout << "intersection-test for" << endl;
       left->print();
       cout << " and " << endl;
       suc->print();
      }
      if (suc) {
       intersectionTestForSetOp(left, suc, event, q, true);
      }
      mpq_class px = event->getPreciseX();
      mpq_class py = event->getPreciseY();

      checkSegments(segmentVector, *event, suc, inside, inside_op);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 delete event;

 return inside;
} // inside region2 x region2 -> bool
} //end of p2d

#endif /* AVL_TREE_CPP_*/
