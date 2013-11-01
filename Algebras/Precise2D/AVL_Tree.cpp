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
 //[->] [$\to$]

 [TOC]

0 Overview

1 Includes and defines

*/
#ifndef _AVL_Tree_CPP
#define _AVL_Tree_CPP

#include "AVL_Tree.h"
#include <cmath>
#include <vector>
#include <limits>

namespace p2d {

bool p2d_debug = false;

/*
1 Class AVLTree

*/

/*
1.1 Constructor and destructor

*/
AVLTree::AVLTree() {
 root = NULL;
}

AVLTree::~AVLTree() {
 delete root;
}

/*
1.1 ~insert~

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
1.1 ~removeGetNeighbor~

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
1.1 ~removeGetNeighbor2~

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
1.1 ~removeInvalidSegment~

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
1.1 ~invertSegments~

*/
void AVLTree::invertSegments(vector<AVLSegment*>& v, int gridX, mpq_class& pX,
  int gridY, mpq_class& pY, AVLSegment*& pred, size_t predIndex,
  AVLSegment*& suc, size_t sucIndex) {
 pred = NULL;
 suc = NULL;
 vector<AVLNode*> nodeVector;
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
    lastNode = nodeVector.back();
    nodeVector.pop_back();
    tempSeg1 = node->getElement();
    tempSeg2 = lastNode->getElement();
    node->setElement(&tempSeg2);
    lastNode->setElement(&tempSeg1);

    nodeVector.push_back(node);
    nodeVector.push_back(lastNode);
   } else {
    nodeVector.push_back(node);
    lastNode = node;
   }
  } else {
   cerr << "Element not found." << endl;
   cerr << "The tree:" << endl << endl;
   root->inorder();
   if (p2d_debug) {
    cerr << "The following segments have to be inverted:" << endl;
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

 for (size_t k = 0; k < ((nodeVector.size()) / 2); k++) {
  tempSeg1 = (nodeVector.at(k))->getElement();
  tempSeg2 = nodeVector.at(nodeVector.size() - k - 1)->getElement();

  nodeVector.at(k)->setElement(&tempSeg2);
  nodeVector.at(nodeVector.size() - k - 1)->setElement(&tempSeg1);
 }

}

/*
1.1 ~inorder~

*/
void AVLTree::inorder() {
 if (root != NULL) {
  root->inorder();
 }
}


/*
2 Class AVLNode

*/

/*
1.1 Constructor and destructor

*/
AVLNode::AVLNode(AVLSegment* elem) :
  elem(elem), left(NULL), right(NULL), height(0) {
}

AVLNode::AVLNode(const AVLNode& node) {
 elem = node.elem;
 if (node.left == NULL) {
  left = NULL;
 } else {
  left = node.left;//new AVLNode(*node.left);
 }
 if (node.right == NULL) {
  right = NULL;
 } else {
  right = node.right;//new AVLNode(*node.right);
 }

 height = node.height;
}

AVLNode::~AVLNode() {
 /*
 elem = NULL;
 if (left){
  delete left;
 }
 if (right){
  delete right;
 }
 left = NULL;
 right = NULL;
 */
}

/*
1.1 ~=~

*/
AVLNode& AVLNode::operator=(const AVLNode& node) {
 elem = node.elem;
 if (node.left == NULL) {
  left = NULL;
 } else {
  left = node.left;//new AVLNode(*node.left);
 }
 if (node.right == NULL) {
  right = NULL;
 } else {
  right = node.right;//new AVLNode(*node.right);
 }
 height = node.height;
 return *this;
}

/*
1.1 ~memberPlusNeighbor~

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
1.1 ~member~

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
1.1 ~insert~

*/
AVLNode* AVLNode::insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc) {

 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x,
   x->getGridXL());
 if (cmpSegments == 0) {
  mpq_class preciseX = x->getPreciseXL();
  cmpSegments = elem->compareInPosOrLeft(*x, x->getGridXL(), preciseX);
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
1.1 ~counterClockwiseRotation~

 This function rebalances the tree

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
1.1 ~clockwiseRotation~

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
1.1 ~setHeight~

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
1.1 ~balance~

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
1.1 ~removeGetNeighbor~

*/
AVLNode* AVLNode::removeGetNeighbor(AVLSegment* x, AVLSegment*& pred,
  AVLSegment*& suc) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x,
   x->getGridXR());
 if (cmpSegments == 0) {
  mpq_class preciseX = x->getPreciseXR();
  cmpSegments = elem->compareInPosOrRight(*x, x->getGridXR(), preciseX);
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
1.1 ~removeGetNeighbor2~

*/
AVLNode* AVLNode::removeGetNeighbor2(AVLSegment* x, int gridXPos,
  mpq_class preciseXPos, AVLSegment*& pred, AVLSegment*& suc) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x, gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosOrLeft(*x, gridXPos, preciseXPos);
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
 }
}

/*
1.1 ~removeInvalidSegment~

*/
AVLNode* AVLNode::removeInvalidSegment(AVLSegment* x, int gridXPos,
  mpq_class preciseXPos, bool& found) {
 int cmpSegments = elem->compareIntersectionintervalWithSweepline(*x, gridXPos);
 if (cmpSegments == 0) {
  cmpSegments = elem->compareInPosOrLeft(*x, gridXPos, preciseXPos);
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
1.1 ~isLeaf~

*/
bool AVLNode::isLeaf() {
 return (left == NULL && right == NULL);
}

/*
1.1 ~deletemin~

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
1.1 ~inorder~

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
3 Class AVLSegment

*/

/*
1.1 Constructors and destructor

*/
AVLSegment::AVLSegment() :
  flob(0), dbarray(0), originalData1(0), originalData2(0),
  pxl(0), pyl(0), pxr(0), pyr(0),
  valid(false), isNew(true), noOfChanges(0),
  conAbove(0), conBelow(0) {
}

AVLSegment::AVLSegment(const Flob* preciseData, SegmentData* sd, Owner o) :
  gridXL(sd->GetDomGridXCoord()), gridYL(sd->GetDomGridYCoord()), gridXR(
    sd->GetSecGridXCoord()), gridYR(sd->GetSecGridYCoord()),
    flob(preciseData), dbarray(0),
    originalData1(sd), originalData2(0), pxl(0), pyl(0), pxr(0), pyr(0), owner(
    o), valid(true), isNew(false), noOfChanges(0), firstInsideAbove(
    o == first ? sd->GetInsideAbove() : 0), secondInsideAbove(
    o == second ? sd->GetInsideAbove() : 0), conAbove(
    sd->GetInsideAbove() ? 1 : 0), conBelow(sd->GetInsideAbove() ? 0 : 1) {
}

AVLSegment::AVLSegment(const Flob* preciseData, p2d::SegmentData* sd, Owner o,
  mpq_class& scalefactor) :
  gridXL(0), gridYL(0), gridXR(0), gridYR(0),
    flob(0), dbarray(0), originalData1(0), originalData2(0),
    pxl(0), pyl(0), pxr(0), pyr(0), owner(o),
    valid(true), isNew(true), noOfChanges(0),
    firstInsideAbove(o == first ? sd->GetInsideAbove() : 0),
    secondInsideAbove(o == second ? sd->GetInsideAbove() : 0),
    conAbove(sd->GetInsideAbove() ? 1 : 0),
    conBelow(sd->GetInsideAbove() ? 0 : 1) {
 p2d::prepareData(gridXL, pxl,
   ((sd->GetDomGridXCoord()+
     sd->GetDomPreciseXCoord(*preciseData))*scalefactor));
 p2d::prepareData(gridYL, pyl,
   ((sd->GetDomGridYCoord()+
     sd->GetDomPreciseYCoord(*preciseData))*scalefactor));
 p2d::prepareData(gridXR, pxr,
   ((sd->GetSecGridXCoord()+
     sd->GetSecPreciseXCoord(*preciseData))*scalefactor));
 p2d::prepareData(gridYR, pyr,
   ((sd->GetSecGridYCoord()+
     sd->GetSecPreciseYCoord(*preciseData))*scalefactor));
}

AVLSegment::AVLSegment(const DbArray<unsigned int>* preciseData,
  Reg2GridHalfSegment& gs, Reg2PrecHalfSegment* ps, Owner o) :
  gridXL(gs.GetLeftPointX()), gridYL(gs.GetLeftPointY()), gridXR(
    gs.GetRightPointX()), gridYR(gs.GetRightPointY()),
    flob(0), dbarray(
    preciseData), originalData1(0), originalData2(ps),
    pxl(0), pyl(0), pxr(0), pyr(0),
    owner(o), valid(true), isNew(false), noOfChanges(0), firstInsideAbove(
    o == first ? gs.GetAttr().insideAbove : 0), secondInsideAbove(
    o == second ? gs.GetAttr().insideAbove : 0), conAbove(
    gs.attr.insideAbove ? 1 : 0), conBelow(gs.attr.insideAbove ? 0 : 1) {
}

AVLSegment::AVLSegment(const DbArray<unsigned int>* preciseData,
  Reg2GridHalfSegment& gs, Reg2PrecHalfSegment& ps, Owner o,
  mpq_class& scalefactor) :
  gridXL(0), gridYL(0), gridXR(0), gridYR(0),
    flob(0), dbarray(0), originalData1(0), originalData2(0),
    pxl(0), pyl(0), pxr(0), pyr(0),
    owner(o), valid(true), isNew(true), noOfChanges(0), firstInsideAbove(
    o == first ? gs.GetAttr().insideAbove : 0), secondInsideAbove(
    o == second ? gs.GetAttr().insideAbove : 0), conAbove(
    gs.attr.insideAbove ? 1 : 0), conBelow(gs.attr.insideAbove ? 0 : 1) {

 p2d::prepareData(gridXL, pxl,
   ((gs.GetLeftPointX()+ps.GetlPointx(preciseData))*scalefactor));
 p2d::prepareData(gridYL, pyl,
   ((gs.GetLeftPointY()+ps.GetlPointy(preciseData))*scalefactor));
 p2d::prepareData(gridXR, pxr,
   ((gs.GetRightPointX()+ps.GetrPointx(preciseData))*scalefactor));
 p2d::prepareData(gridYR, pyr,
   ((gs.GetRightPointY()+ps.GetrPointy(preciseData))*scalefactor));
}

AVLSegment::AVLSegment(const AVLSegment& s) :
  gridXL(s.getGridXL()), gridYL(s.getGridYL()), gridXR(s.getGridXR()), gridYR(
    s.getGridYR()), flob(s.flob), dbarray(s.dbarray), originalData1(
    s.originalData1), originalData2(s.originalData2),
    pxl(s.pxl), pyl(s.pyl), pxr(s.pxr), pyr(s.pyr), owner(s.owner),
    valid(s.isValid()), isNew(s.isNew), noOfChanges(s.noOfChanges),
    firstInsideAbove(s.getFirstInsideAbove()), secondInsideAbove(
    s.getSecondInsideAbove()), conAbove(s.conAbove), conBelow(s.conBelow) {
}

AVLSegment::AVLSegment(int gridX, mpq_class px, int gridY, mpq_class py) :
  gridXL(gridX), gridYL(gridY), gridXR(gridX), gridYR(gridY), flob(0), dbarray(
    0), originalData1(0), originalData2(0), pxl(px), pyl(py), pxr(px), pyr(py),
    owner(none), valid(true), isNew(true), noOfChanges(0), firstInsideAbove(0),
    secondInsideAbove(0), conAbove(0), conBelow(0) {
}

AVLSegment::AVLSegment(int gxl, int gyl, int gxr, int gyr, mpq_class xLeft,
  mpq_class yLeft, mpq_class xRight, mpq_class yRight, Owner o) :
  gridXL(gxl), gridYL(gyl), gridXR(gxr), gridYR(gyr), flob(0), dbarray(0),
  originalData1(0), originalData2(0),
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
1.1 ~set~

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
1.1 ~prepareData~

 Extract the integer from ~value~.

*/
void AVLSegment::prepareData(int& resultGridX, mpq_class& resultPX,
  mpq_class& value) {
 mpz_class numerator = value.get_num();
 mpz_class denominator = value.get_den();
 mpz_class gridX = numerator / denominator;
 if ((cmp(gridX, numeric_limits<int>::min())<0)
   ||(cmp(numeric_limits<int>::max(), gridX)<0)){
  cerr <<"The grid-value "<<gridX
       <<"don't fit in a variable of type int."<<endl;
  assert(false);
 }
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
1.1 ~getPrecise...~

*/
mpq_class AVLSegment::getPreciseXL() const {
 if (isNew) {
  return pxl;
 } else {
  if (originalData1) {
   return originalData1->getPreciseLeftX(*flob);
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
   return originalData1->getPreciseLeftY(*flob);
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
   return originalData1->getPreciseRightX(*flob);
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
   return originalData1->getPreciseRightY(*flob);
  } else {
   return originalData2->GetrPointy(dbarray);
  }
 }

}

/*
1.1 ~getNumberOfChanges~

*/
int AVLSegment::getNumberOfChanges() const {
 return noOfChanges;
}

/*
1.1 ~setNumberOfChanges~

*/
void AVLSegment::setNumberOfChanges(int i) {
 noOfChanges = i;
}

/*
1.1 ~incrementNumberOfChanges~

*/
void AVLSegment::incrementNumberOfChanges() {
 noOfChanges++;
}

/*
1.1 ~getFirstInsideAbove~

*/
bool AVLSegment::getFirstInsideAbove() const {
 return firstInsideAbove;
}

/*
1.1 ~getSecondInsideAbove~

*/
bool AVLSegment::getSecondInsideAbove() const {
 return secondInsideAbove;
}

/*
1.1 ~setFirstInsideAbove~

*/
void AVLSegment::setFirstInsideAbove(bool insideAbove) {
 firstInsideAbove = insideAbove;
}

/*
1.1 ~setSecondInsideAbove~

*/
void AVLSegment::setSecondInsideAbove(bool insideAbove) {
 secondInsideAbove = insideAbove;
}

/*
1.1 ~getConBelow~

*/
short int AVLSegment::getConBelow() {
 return conBelow;
}

/*
1.1 ~getConAbove~

*/
short int AVLSegment::getConAbove() {
 return conAbove;
}

/*
1.1 ~setConBelow~

*/
void AVLSegment::setConBelow(short int i) {
 conBelow = i;
}

/*
1.1 ~setConAbove~

*/
void AVLSegment::setConAbove(short int i) {
 conAbove = i;
}

/*
1.1 ~print~

*/
void AVLSegment::print() {

 cout << "( " << getGridXL() << " " << getPreciseXL() << ", " << getGridYL()
   << " " << getPreciseYL() << ", " << getGridXR() << " " << getPreciseXR()
   << ", " << getGridYR() << " " << getPreciseYR() << " )" << endl << "owner: "
   << owner << endl << "conAbove: " << conAbove << endl << "conBelow: "
   << conBelow << endl << "valid: " << valid << endl
   << "firstInsideAbove: " << firstInsideAbove<<
   endl<<"secondInsideAbove: " <<secondInsideAbove<<endl;

}

/*
1.1 test-functions

*/

/*
1.1.1 ~equal~

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
1.1.1 ~isPoint~

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

/*
1.1.1 ~isEndpoint~

*/
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

/*
1.1.1 ~startsAtPoint~

*/
bool AVLSegment::startsAtPoint(int gx, int gy, mpq_class px, mpq_class py) {
 if (getGridXL() == gx && getGridYL() == gy) {
  if (cmp(getPreciseXL(), px) == 0 && cmp(getPreciseYL(), py) == 0) {
   return true;
  }
 }
 return false;
}

/*
1.1.1 ~endsInPoint~

*/
bool AVLSegment::endsInPoint(int gx, int gy, mpq_class px, mpq_class py) {
 if (getGridXR() == gx && getGridYR() == gy) {
  if (cmp(getPreciseXR(), px) == 0 && cmp(getPreciseYR(), py) == 0) {
   return true;
  }
 }
 return false;
}

/*
1.1.1 ~isLeftOf~

*/
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
 //If the event is a left endpoint event or an intersection event,
 //then the segment lay right of the event, otherwise not.
 if (event.isRightEndpointEvent() || event.isIntersectionEvent()) {
  return true;
 } else {
  return false;
 }
}

/*
1.1.1 ~leftPointIsLeftOf~

*/
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
 //If the event is a left endpoint event or an intersection event, then
 //the segment lay right of the event, otherwise not.
 if (event.isRightEndpointEvent()) {
  return true;
 } else {
  return false;
 }
}

/*
1.1.1 ~isRightPointOf~

*/
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

/*
1.1.1 ~isLeftPointOf~

*/
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

/*
1.1.1 ~hasEqualLeftEndPointAs~

*/
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
1.1.1 ~isValid~

*/
bool AVLSegment::isValid() const {
 return valid;
}

/*
1.1 ~changeValidity~

*/
void AVLSegment::changeValidity(bool v) {
 valid = v;
}

/*
1.1  ~=~

*/
AVLSegment& AVLSegment::operator=(const AVLSegment& s) {
 gridXL = s.getGridXL();
 gridYL = s.getGridYL();
 gridXR = s.getGridXR();
 gridYR = s.getGridYR();
 flob = s.flob;
 dbarray = s.dbarray;
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
1.1 ~intervalIsVertical~

*/
bool AVLSegment::intervalIsVertical() {
 return gridXL == gridXR;
}

/*
1.1 ~computeBeginOfIntersectionInterval~

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
   //mpq_class value = (((ppos * (gridYR - gridYL))
   //+ (gridYL * (gridXR - pgridXL))
   //  - ((pgridXL + 1) * (gridYR - gridYL))) / (gridXR - pgridXL));
   //if (cmp(value, numeric_limits<int>::max())>0){
   // cerr<< "wert zu groß"<<endl;
   // assert(false);
   //}
   long long yryl = gridYR - gridYL;
   long long xrxl = gridXR - gridXL;
   long long lPos = (long long) pos;
   long long lGridXL = (long long) gridXL;
   long long lGridYL = (long long) gridYL;
   long long num = ((lPos * yryl) + (lGridYL * xrxl)
     - ((lGridXL + 1) * yryl));
   return ((int)(num/xrxl));
   //return (((pos * (gridYR - gridYL)) + (gridYL * (gridXR - gridXL))
   //  - ((gridXL + 1) * (gridYR - gridYL))) / (gridXR - gridXL));
  } else {
   //mpq_class value = ((((ppos + 1) * (gridYR - gridYL))
   //+ (gridYL * (gridXR - pgridXL))
   //  - (pgridXL * (gridYR - gridYL))) / (gridXR - pgridXL));
   //if (cmp(value, numeric_limits<int>::max())>0){
   // cerr<< "wert zu groß"<<endl;
   // assert(false);
   //}
   long long yryl = gridYR - gridYL;
   long long xrxl = gridXR - gridXL;
   long long lPos = (long long) pos;
   long long lGridXL = (long long) gridXL;
   long long lGridYL = (long long) gridYL;
   long long num = (((lPos + 1) * yryl) + (lGridYL * xrxl)
     - (lGridXL * yryl));
   return ((int)(num/xrxl));
   //return ((((pos + 1) * (gridYR - gridYL)) + (gridYL * (gridXR - gridXL))
   //  - (gridXL * (gridYR - gridYL))) / (gridXR - gridXL));
  }
 }
}

/*
1.1 ~computeEndOfIntersectionInterval~

*/
int AVLSegment::computeEndOfIntersectionInterval(int pos) {
 if (intervalIsVertical()) {
  if (gridYL >= gridYR) {
   return gridYL + 1;
  } else {
   return gridYR + 1;
  }
 } else {
  //mpq_class pgridXL = gridXL;
  //mpq_class ppos = pos;
  if (((gridYR - gridYL) > 0 && (gridXR - gridXL) > 0)
    || ((gridYR - gridYL) < 0 && (gridXR - gridXL) < 0)) {
   //positive slope
   long long yryl = gridYR - gridYL;
   long long xrxl = gridXR - gridXL;
   long long lPos = (long long) pos;
   long long lGridXL = (long long) gridXL;
   long long lGridYL = (long long) gridYL;
   long long num = (((lPos + 1) * yryl) + ((lGridYL + 1) * xrxl)
       - ((lGridXL) * yryl));
   return ((int)((num/(xrxl))+1));
  } else {
   long long yryl = gridYR - gridYL;
   long long xrxl = gridXR - gridXL;
   long long lPos = (long long) pos;
   long long lGridXL = (long long) gridXL;
   long long lGridYL = (long long) gridYL;
   long long num = ((lPos * yryl) + ((lGridYL + 1) * xrxl)
     - ((lGridXL + 1) * yryl));
   return ((int)((num/xrxl)+1));
  }
 }
}

/*
1.1 ~compareIntersectionIntervallWithSweepline~

*/
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
1.1 ~compareInPos~

*/
int AVLSegment::compareInPosOrRight(AVLSegment& s, int gridXPos,
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

 if ((getGridXR() != getGridXL()) || (cmp(thisXR, thisXL) != 0)) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if ((s.getGridXR() != s.getGridXL()) || (cmp(sXR, sXL) != 0)) {

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
     if (getGridXL() != s.getGridXL()) {
      if (getGridXL() < s.getGridXL()) {
       return -1;
      } else {
       return 1;
      }
     }
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      if (getGridXR() != s.getGridXR()) {
       if (getGridXR() < s.getGridXR()) {
        return -1;
       } else {
        return 1;
       }
      }
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

  if ((s.getGridYR() < this->getGridYL())
    || ((s.getGridYR() == this->getGridYL()) && (cmp(sYR, thisYL) < 0))) {
   // this is above s
   return 1;
  } else {
   if (getGridYR() < s.getGridYL()) {
    return -1;
   }
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   if ((getGridYR() == s.getGridYL()) && (cmp(thisYR, sYL) < 0)) {
    // s above this
    return -1;
   }
   if (getGridYL() != s.getGridYL()) {
    if (getGridYL() < s.getGridYL()) {
     return -1;
    } else {
     return 1;
    }
   }
   int cmpYL = cmp(thisYL, sYL);
   if (cmpYL != 0) {
    return cmpYL;
   } else {
    if (getGridYR() != s.getGridYR()) {
     if (getGridYR() < s.getGridYR()) {
      return -1;
     } else {
      return 1;
     }
    }
    int cmpYR = cmp(thisYR, sYR);
    return cmpYR;
   }
  }
 }

}

/*
1.1 ~compareInPosOrLeft~

*/
int AVLSegment::compareInPosOrLeft(AVLSegment& s, int gridXPos,
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

 if ((getGridXR() != getGridXL()) || (cmp(thisXR, thisXL) != 0)) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if ((s.getGridXR() != s.getGridXL()) || (cmp(sXR, sXL) != 0)) {

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
     if (getGridXL() != s.getGridXL()) {
      if (getGridXL() < s.getGridXL()) {
       return -1;
      } else {
       return 1;
      }
     }
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      if (getGridXR() != s.getGridXR()) {
       if (getGridXR() < s.getGridXR()) {
        return -1;
       } else {
        return 1;
       }
      }
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
  if ((s.getGridYR() < getGridYL())
    || ((s.getGridYR() == getGridYL()) && cmp(sYR, thisYL) < 0)) {
   // this is above s
   return 1;
  } else {
   if (getGridYR() < s.getGridYL()) {
    return -1;
   }
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   if ((getGridYR() == s.getGridYL()) && (cmp(thisYR, sYL) < 0)) {
    // s above this
    return -1;
   }
   if (getGridYL() != s.getGridYL()) {
    if (getGridYL() < s.getGridYL()) {
     return -1;
    } else {
     return 1;
    }
   }
   int cmpYL = cmp(thisYL, sYL);
   if (cmpYL != 0) {
    return cmpYL;
   } else {
    if (getGridYR() != s.getGridYR()) {
     if (getGridYR() < s.getGridYR()) {
      return -1;
     } else {
      return 1;
     }
    }
    return cmp(thisYR, sYR);
   }
  }
 }

}

/*
1.1 ~compareInPosForMember~

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

 if ((getGridXR() != getGridXL()) || (cmp(thisXR, thisXL) != 0)) {

  thisSlope = (thisYR - thisYL) / (thisXR - thisXL);

  thisIsNotVertical = true;
 }

 if ((s.getGridXR() != s.getGridXL()) || (cmp(sXR, sXL) != 0)) {

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
     if (getGridXL() != s.getGridXL()) {
      if (getGridXL() < s.getGridXL()) {
       return -1;
      } else {
       return 1;
      }
     }
     int cmpXL = cmp(thisXL, sXL);
     if (cmpXL != 0) {
      return cmpXL;
     } else {
      //both segments start in the same XL
      if (getGridXR() != s.getGridXR()) {
       if (getGridXR() < s.getGridXR()) {
        return -1;
       } else {
        return 1;
       }
      }
      return cmp(thisXR, sXR);
     }

    }

   }
  }
 }

 else {
  if (thisIsNotVertical) {
   //~s~ is vertical
   if ((this->getGridXL() == s.getGridXL())
     && (this->getGridYL() == s.getGridYL())
     && (cmp(this->getPreciseXL(), s.getPreciseXL()) == 0)
     && (cmp(this->getPreciseYL(), s.getPreciseYL()) == 0)
     && ((this->getGridYL() < gridYPos)
       || ((this->getGridYL() == gridYPos)
         && (cmp(this->getPreciseYL(), preciseYPos) < 0)))) {
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
   if ((this->getGridXL() == s.getGridXL())
     && (this->getGridYL() == s.getGridYL())
     && (cmp(this->getPreciseXL(), s.getPreciseXL()) == 0)
     && (cmp(this->getPreciseYL(), s.getPreciseYL()) == 0)
     && ((this->getGridYL() < gridYPos)
       || ((this->getGridYL() == gridYPos)
         && (cmp(this->getPreciseYL(), preciseYPos) < 0)))) {
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
  if ((s.getGridYR() < getGridYL())
    || ((s.getGridYR() == getGridYL()) && (cmp(sYR, thisYL) < 0))) {
   // this is above s
   return 1;
  } else {
   if (getGridYR() < s.getGridYL()) {
    return -1;
   }
   mpq_class thisYR = getPreciseYR() + getGridYR();
   mpq_class sYL = s.getPreciseYL() + s.getGridYL();

   if ((getGridYR() == s.getGridYL()) && (cmp(thisYR, sYL) < 0)) {
    // s above this
    return -1;
   }
   if (getGridYL() != s.getGridYL()) {
    if (getGridYL() < s.getGridYL()) {
     return -1;
    } else {
     return 1;
    }
   }
   int cmpYL = cmp(thisYL, sYL);

   if (cmpYL != 0) {
    return cmpYL;
   } else {
    if (getGridYR() != s.getGridYR()) {
     if (getGridYR() < s.getGridYR()) {
      return -1;
     } else {
      return 1;
     }
    }
    return cmp(thisYR, sYR);
   }
  }
 }

}

/*
1.1 ~isVertical~

*/
bool AVLSegment::isVertical() const {
 if (getGridXL() != getGridXR()) {
  return false;
 } else {
  return (cmp(getPreciseXL(), getPreciseXR()) == 0);
 }
}

/*
1.1 ~isParallelTo~

*/
bool AVLSegment::isParallelTo(const AVLSegment& s) const {

 mpq_class thisXL = getPreciseXL() + getGridXL();
 mpq_class thisXR = getPreciseXR() + getGridXR();

 mpq_class sXL = s.getPreciseXL() + s.getGridXL();
 mpq_class sXR = s.getPreciseXR() + s.getGridXR();

 if ((getGridXL() == getGridXR()) && (cmp(thisXL, thisXR) == 0)) {
  //this is vertical
  if ((s.getGridXL() == s.getGridXR()) && (cmp(sXL, sXR) == 0)) {
   //s is vertical
   return true;
  } else {
   return false;
  }
 }

 if ((s.getGridXL() == s.getGridXR()) && (cmp(sXL, sXR) == 0)) {
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

1.1 ~mightIntersect~

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

1.1 ~intersect~

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

 if ((getGridXL() == getGridXR()) && (seg.getGridXL() == seg.getGridXR())
   && (cmp(thisXL, thisXR) == 0) && (cmp(segXL, segXR) == 0)) {
  //both are vertical
  if ((getGridXL() == seg.getGridXL()) && (cmp(thisXL, segXL) == 0)) {
   //both segments run on one line
   if ((seg.getGridYR() < getGridYL())
     || ((seg.getGridYR() == getGridYL()) && (cmp(segYR, thisYL) < 0))
     || (getGridYR() < seg.getGridYL())
     || ((getGridYR() == seg.getGridYL()) && (cmp(thisYR, segYL) < 0))) {
    //this passes below or above ~seg~
    return false;
   } else {
    //determine the endpoints
    if ((getGridYL() < seg.getGridYL())
      || ((getGridYL() == seg.getGridYL()) && (cmp(thisYL, segYL) <= 0))) {
     //overlapping segment starts in (segXL, segYL)
     if ((seg.getGridYR() < getGridYR())
       || ((seg.getGridYR() == getGridYR()) && (cmp(segYR, thisYR) <= 0))) {
      //overlapping segment ends in (segXL, segYR)
      result.set(segXL, segYL, segXL, segYR, both);
      return true;
     } else {
      //overlapping segment ends in (segXL, thisYR)
      result.set(seg.getGridXL(), seg.getGridYL(),
        seg.getGridXL(), getGridYR(),
        seg.getPreciseXL(), seg.getPreciseYL(), seg.getPreciseXL(),
        getPreciseYR(), both);
      return true;
     }
    } else {
     //overlapping segment starts in (thisXL, thisYL)
     if ((seg.getGridYR() < getGridYR())
       || ((seg.getGridYR() == getGridYR()) && (cmp(segYR, thisYR) <= 0))) {
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
  } else {
   return false;
  }

 }

 if ((getGridXL() == getGridXR()) && (cmp(thisXL, thisXR) == 0)) {
  //this segments is vertical
  if (((seg.getGridXL() < getGridXL())
    || ((seg.getGridXL() == getGridXL()) && (cmp(segXL, thisXL) <= 0)))
    && ((getGridXL() < seg.getGridXR())
      || ((getGridXL() == seg.getGridXR()) && (cmp(thisXL, segXR) <= 0)))) {
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

 if ((seg.getGridXL() == seg.getGridXR()) && (cmp(segXL, segXR) == 0)) {
  //seg is vertical
  if (((getGridXL() < seg.getGridXL())
    || ((getGridXL() == seg.getGridXL()) && (cmp(thisXL, segXL) <= 0)))
    && ((seg.getGridXL() < getGridXR())
      || ((seg.getGridXL() == getGridXR()) && (cmp(segXL, thisXR) <= 0)))) {
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
   if ((seg.getGridXR() < getGridXL()) || (getGridXR() < seg.getGridXL())
     || ((seg.getGridXR() == getGridXL()) && (cmp(segXR, thisXL) < 0))
     || ((getGridXR() == seg.getGridXL()) && (cmp(thisXR, segXL) < 0))) {
    // seg runs more left/right than this
    return false;
   } else {
    if ((getGridXL() < seg.getGridXL())
      || ((getGridXL() == seg.getGridXL()) && (cmp(thisXL, segXL) < 0))) {
     if ((getGridXR() < seg.getGridXR())
       || ((getGridXR() == seg.getGridXR()) && (cmp(thisXR, segXR) < 0))) {
      result.set(segXL, segYL, thisXR, thisYR, both);
      return true;
     } else {
      result.set(segXL, segYL, segXR, segYR, both);
      return true;
     }
    } else {
     if ((getGridXR() < seg.getGridXR())
       || ((getGridXR() == seg.getGridXR()) && (cmp(thisXR, segXR) < 0))) {
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
1 Class SimpleSegment and BoundingSegments

 SimpleSegment and BoundingSegments are used in mightIntersect.
 An object of type BoundingSegments contains the segments which
 form the envelope of a real segment.

*/

/*
1.1 Constructors and destructor

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

/*
1.1 ~createBoundingSegments~

*/
void BoundingSegments::createBoundingSegments(Slope s, int gxl, int gyl,
  int gxr, int gyr) {
 switch (s) {
 case vertical: {
  int y1 = gyl<gyr?gyl:gyr;
  int y2 = gyl<gyr?gyr:gyl;
  segments = new SimpleSegment[numSeg];
  segments[0] = SimpleSegment(gxl, y1, gxr, y2 + 1, pLeft);
  segments[1] = SimpleSegment(gxr, y2 + 1, gxr + 1, y2 + 1, top);
  segments[2] = SimpleSegment(gxl + 1, y1, gxr + 1, y2 + 1, pRight);
  segments[3] = SimpleSegment(gxl, y1, gxl + 1, y1, bottom);
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
1.1 ~isVertical~

*/
bool BoundingSegments::isVertical(int i) {
 assert(0 <= i && i < numSeg);
 return (segments[i].getXR() == segments[i].getXL());
}

/*
1.1 ~isHorizontal~

*/
bool BoundingSegments::isHorizontal(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getYR() == segments[i].getYL());
}

/*
1.1 ~isLeft~

*/
bool BoundingSegments::isLeft(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == pLeft);
}

/*
1.1 ~isRight~

*/
bool BoundingSegments::isRight(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == pRight);
}

/*
1.1 ~isTop~

*/
bool BoundingSegments::isTop(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == top);
}

/*
1.1 ~isBottom~

*/
bool BoundingSegments::isBottom(int i) {
 assert(0 <= i && i <= numSeg);
 return (segments[i].getPosition() == bottom);
}

/*
1.1 ~intersect~

*/
bool BoundingSegments::intersect(BoundingSegments& bs) {
 int i = 0;
 bool result = false;
 while (i < numSeg && !result) {
  int j = 0;
  while (j < bs.numSeg && !result) {
   long long thisNum = segments[i].getYR() - segments[i].getYL();
   long long thisDenom = segments[i].getXR() - segments[i].getXL();

   long long bsNum = bs.segments[j].getYR() - bs.segments[j].getYL();
   long long bsDenom = bs.segments[j].getXR() - bs.segments[j].getXL();
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
      long long numerator = second.getYR() - second.getYL();
      long long denominator = second.getXR() - second.getXL();
      if ((numerator > 0 && denominator > 0)
        || (numerator < 0 && denominator < 0)) {
       /* second has a positive slope
        *  To prevent wrong values by using double we compute
        *  only with integers. If we have a function like
        *  f(x)=m*x+b, we take: m=numerator/denominator
        *  and b=second.getGridYL()-m*second.getGridXL().
        *  By avoiding double-values we get:
        *  y=(numerator*vertical.getGridXL()
        *  + (second.getGridYL()+1)*denominator
        *  - numerator*second.getGridXL()) / denominator.
        *  Then y is the grid-value. If yValue is in the
        *  interval formed by vertical, both segments might
        *  intersect.
       */
       if (!((second.getPosition() == top && vertical.getPosition() == pRight
         && vertical.getXL() == second.getXL())
         || (second.getPosition() == bottom && vertical.getPosition() == pLeft
           && vertical.getXL() == second.getXR()))) {
        long long yValue = (numerator * ((long long)vertical.getXL())
          + ((long long)second.getYL()) * denominator
          - numerator * ((long long)second.getXL()))
          / denominator;
        /*mpz_class pValue = (numerator * vertical.getXL()
          + second.getYL() * denominator - numerator * second.getXL())
          / denominator;
        if ((cmp(pValue, numeric_limits<int>::min())<0)
          ||(cmp(numeric_limits<int>::max(), pValue)<0)){
         cerr <<"The grid-value "<<pValue
              <<"don't fit in a variable of type int."<<endl;
         return true;
        }
        int yValue = (int) pValue.get_d();*/
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
          || (second.getPosition() == bottom
            && vertical.getPosition() == pRight
            && vertical.getXL() == second.getXL()))) {

         long long yValue = (numerator * ((long long)vertical.getXL())
           + ((long long)second.getYL()) * denominator
           - numerator * ((long long)second.getXL()))
           / denominator;
         /*mpz_class pYValue = (numerator * vertical.getXL()
             + second.getYL() * denominator - numerator * second.getXL())
             / denominator;
         if ((cmp(pYValue, numeric_limits<int>::min())<0)
           ||(cmp(numeric_limits<int>::max(), pYValue)<0)){
          cerr <<"The grid-value "<<pYValue
               <<"don't fit in a variable of type int."<<endl;
          return true;
         }
         int yValue = (int) pYValue.get_d();*/
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
       long long thisB =
         ((long long)segments[i].getYL()) * ((long long)thisDenom)
         - ((long long)segments[i].getXL()) * thisNum;
       long long segB = ((long long)bs.segments[j].getYL()) * thisDenom
         - ((long long)bs.segments[j].getXL()) * thisNum;
       if (thisB == segB) {
        if (!(segments[i].getXR() <= bs.segments[j].getXL()
          || bs.segments[j].getXR() <= segments[i].getXL())) {
         return true;
        }
       }
      }
     } else { //the segements are not parallel or vertical
      long long xNumerator = (thisDenom * bsDenom
        * (((long long)bs.segments[j].getYL()) -
          ((long long)segments[i].getYL())))
        + (((long long)segments[i].getXL()) * bsDenom * thisNum)
        - (((long long)bs.segments[j].getXL()) * thisDenom * bsNum);
      long long xDenominator = (thisNum * bsDenom - thisDenom * bsNum);
      long long gridX = (xNumerator / xDenominator);

      /*mpz_class pThisDenom = thisDenom;
      mpz_class pBsDenom = bsDenom;
      mpz_class pXNumerator = (pThisDenom * pBsDenom
        * (bs.segments[j].getYL() - segments[i].getYL()))
        + (segments[i].getXL() * pBsDenom * thisNum)
        - (bs.segments[j].getXL() * pThisDenom * bsNum);
      mpz_class pXDenominator = (thisNum * pBsDenom - pThisDenom * bsNum);
      mpz_class pGridX = pXNumerator / pXDenominator;
      if ((cmp(pGridX, numeric_limits<int>::min())<0)
        ||(cmp(numeric_limits<int>::max(), pGridX)<0)){
       cerr <<"The grid-value "<<pGridX
            <<"don't fit in a variable of type int."<<endl;
       return true;
      }
      int gridX = (int) pGridX.get_d();
      cout <<"gridX: "<<gridX<<endl;*/
      if ((segments[i].getXL() <= gridX && gridX < segments[i].getXR())
        && (bs.segments[j].getXL() <= gridX
          && gridX < bs.segments[j].getXR())) {
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
         //both segments are not horizontal
         long long gridY = ((thisNum * gridX)
           + (segments[i].getYL() * thisDenom)
           - (segments[i].getXL() * thisNum)) / thisDenom;
         /*mpz_class pGridY = ((thisNum * gridX)
           + (segments[i].getYL() * thisDenom)
           - (segments[i].getXL() * thisNum)) / thisDenom;
         if ((cmp(pGridY, numeric_limits<int>::min())<0)
           ||(cmp(numeric_limits<int>::max(), pGridY)<0)){
          cerr <<"The grid-value "<<pGridY
               <<"don't fit in a variable of type int."<<endl;
          return true;
         }
         int gridY = (int) pGridY.get_d();
         cout <<"gridY: "<<gridY<<endl;*/
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
1 Class Event

*/

/*
1.1 Constructors and destructor

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
1.1 ~getGridX~

*/
int Event::getGridX() const {
 return gridX;
}

/*
1.1 ~getPreciseX~

*/
mpq_class Event::getPreciseX() const {
 return preciseX;
}

/*
1.1 ~getGridY~

*/
int Event::getGridY() const {
 return gridY;
}

/*
1.1 ~getPreciseX~

*/
mpq_class Event::getPreciseY() const {
 return preciseY;
}

/*
1.1 ~getNoOfChanges~

*/
int Event::getNoOfChanges() const {
 return noOfChangesSeg;
}

/*
1.1 ~=~

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
1.1 ~isValid~

 An Event becomes invalid, if the associated segment(s) have been
 changed between the time where the event is put in the
 priority\_queue and the time where it has to be processed.

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
1.1 ~isLeftEndpointEvent~

*/
bool Event::isLeftEndpointEvent() const {
 return (kind == leftEndpoint);
}

/*
1.1 ~isRightEndpointEvent~

*/
bool Event::isRightEndpointEvent() const {
 return (kind == rightEndpoint);
}

/*
1.1 ~isIntersectionEvent~

*/
bool Event::isIntersectionEvent() const {
 if (kind == intersectionPoint) {
  return true;
 }
 return false;
}

/*
1.1 ~getSegment~

*/
AVLSegment* Event::getSegment() const {
 return seg;
}

/*
1.1 ~getLeftSegment~

*/
AVLSegment* Event::getLeftSegment() const {
 return lSeg;
}

/*
1.1 ~getRightSegment~

*/
AVLSegment* Event::getRightSegment() const {
 return gSeg;
}

/*
1.1 ~$>$~

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
   return (seg->compareInPosOrLeft(*e.seg, getGridX(), p)) == 1;
  } else {
   return true;
  }
 }
 if (isIntersectionEvent() && e.isLeftEndpointEvent()) {
  return true;
 }
 if (isIntersectionEvent() && e.isIntersectionEvent()) {
  mpq_class p = getPreciseX();
  int cmpLSeg = lSeg->compareInPosOrLeft(*e.lSeg, getGridX(), p);
  if (cmpLSeg != 0) {
   if (cmpLSeg > 0) {
    return true;
   } else {
    return false;
   }
  } else {

   int cmpGSeg = gSeg->compareInPosOrLeft(*e.gSeg, getGridX(), p);
   if (cmpGSeg > 0) {
    return true;
   } else {
    return false;
   }

  }
 }
 if (isLeftEndpointEvent() && e.isLeftEndpointEvent()) {
  mpq_class p = getPreciseX();
  return (seg->compareInPosOrLeft(*e.seg, getGridX(), p)) == 1;
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
1 ~selectNext~

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
  delete result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(l2.getPreciseCoordinates(), sd2, second);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  delete result2;
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

template<class C1, class C2>
Owner selectNext(const C1& l1, int& pos1, const C2& l2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  mpq_class& internalScalefactor) {
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
   prepareData(c0.gx, c0.px,
      ((l1.getLeftGridX(pos1)+l1.getPreciseLeftX(pos1))*internalScalefactor));
   prepareData(c0.gy, c0.py,
      ((l1.getLeftGridY(pos1)+l1.getPreciseLeftY(pos1))*internalScalefactor));
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
   prepareData(c1.gx, c1.px,
      ((l2.getLeftGridX(pos2)+l2.getPreciseLeftX(pos2))*internalScalefactor));
   prepareData(c1.gy, c1.py,
      ((l2.getLeftGridY(pos2)+l2.getPreciseLeftY(pos2))*internalScalefactor));
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
  prepareData(c2.gx, c2.px,
     (e1.getGridX()+e1.getPreciseX()));
  prepareData(c2.gy, c2.py,
     (e1.getGridY()+e1.getPreciseY()));
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
  AVLSegment* seg1 = new AVLSegment(l1.getPreciseCoordinates(), sd1,
    first, internalScalefactor);
  //AVLSegment* seg1 = new AVLSegment(l1.getPreciseCoordinates(), sd1, first);
  Event* result1 = new Event(leftEndpoint, seg1);
  event = *result1;
  delete result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(l2.getPreciseCoordinates(), sd2,
    second, internalScalefactor);
  //AVLSegment* seg2 = new AVLSegment(l2.getPreciseCoordinates(), sd2, second);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  delete result2;
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
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  mpq_class& internalScalefactor) {
 return selectNext<Line2, Line2>(l, pos1, r, pos2, q, event,
   internalScalefactor);
}

Owner selectNext(/*const*/Region2& r1, int& pos1,
/*const*/Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event) {

 Coordinate* values[3];

 Reg2GridHalfSegment gs1;
 Reg2GridHalfSegment gs2;
 Reg2PrecHalfSegment ps1;
 Reg2PrecHalfSegment ps2;
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
    new Reg2PrecHalfSegment(ps1), first);
  Event* result1 = new Event(leftEndpoint, seg1);
  event = *result1;
  delete result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(r2.getpreciseCoordinates(), gs2,
    new Reg2PrecHalfSegment(ps2), second);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  delete result2;
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

Owner selectNext(/*const*/Region2& r1, int& pos1,
/*const*/Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  mpq_class& internalScalefactor) {


 Coordinate* values[3];

 Reg2GridHalfSegment gs1;
 Reg2GridHalfSegment gs2;
 Reg2PrecHalfSegment ps1;
 Reg2PrecHalfSegment ps2;
 Event e1, e2;
 Coordinate c0, c1, c2;
 bool found = false;
 int number = 0; // number of available values

 while (pos1 < r1.Size() && !found) {
  r1.getgridCoordinates()->Get(pos1, gs1);
  r1.getprecCoordinates()->Get(pos1, ps1);
  if (gs1.IsLeftDomPoint()) {
   //c0.gx = gs1.GetLeftPointX();
   //c0.px = ps1.GetlPointx(r1.getpreciseCoordinates());
   //c0.gy = gs1.GetLeftPointY();
   //c0.py = ps1.GetlPointy(r1.getpreciseCoordinates());
   prepareData(c0.gx, c0.px,
     ((gs1.GetLeftPointX()
       +ps1.GetlPointx(r1.getpreciseCoordinates()))*internalScalefactor));
   prepareData(c0.gy, c0.py,
      ((gs1.GetLeftPointY()
        +ps1.GetlPointy(r1.getpreciseCoordinates()))*internalScalefactor));
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
   //c1.gx = gs2.GetLeftPointX();
   //c1.px = ps2.GetlPointx(r2.getpreciseCoordinates());
   //c1.gy = gs2.GetLeftPointY();
   //c1.py = ps2.GetlPointy(r2.getpreciseCoordinates());
   prepareData(c1.gx, c1.px,
     ((gs2.GetLeftPointX()
       +ps2.GetlPointx(r2.getpreciseCoordinates()))*internalScalefactor));
   prepareData(c1.gy, c1.py,
      ((gs2.GetLeftPointY()
        +ps2.GetlPointy(r2.getpreciseCoordinates()))*internalScalefactor));

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
  //c2.gx = e1.getGridX();
  //c2.px = e1.getPreciseX();
  //c2.gy = e1.getGridY();
  //c2.py = e1.getPreciseY();
  prepareData(c2.gx, c2.px,
    (e1.getGridX()+e1.getPreciseX()));
  prepareData(c2.gy, c2.py,
     (e1.getGridY()+e1.getPreciseY()));
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
    ps1, first, internalScalefactor);
  Event* result1 = new Event(leftEndpoint, seg1);
  event = *result1;
  delete result1;
  return first;
 }
 case 1: {
  if (values[0] != 0) {
   pos1--;
  }
  AVLSegment* seg2 = new AVLSegment(r2.getpreciseCoordinates(), gs2,
    ps2, second, internalScalefactor);
  Event* result2 = new Event(leftEndpoint, seg2);
  event = *result2;
  delete result2;
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
  delete res;
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
1 ~mergeNeighbors~

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
    neighbor->getPreciseXL(), neighbor->getPreciseYL(),
    current->getPreciseXR(),
    current->getPreciseYR(), neighbor->getOwner());

  newNeighbor->setNumberOfChanges(neighbor->getNumberOfChanges() + 1);
  *neighbor = *newNeighbor;
  delete newNeighbor;

  return true;
 }

}

/*
1 ~splitNeighbors~

*/
void splitNeighbors(AVLSegment* current, AVLSegment* neighbor,
  AVLSegment* overlappingSegment, AVLSegment* right) {

 //r starts at the end of overlappingSegment. If neighbor is longer
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
1 ~intersectionTestForRealminize~

*/
void intersectionTestForRealminize(AVLSegment* left, AVLSegment* right,
  Event event, priority_queue<Event, vector<Event>, greater<Event> >& q,
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
    if (!overlappingSegment->isLeftOf(event)
      /*&& (!(overlappingSegment->isLeftPointOf(*left)
        && overlappingSegment->isRightPointOf(*right)))*/) {
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
1 ~intersectionTestForSetOp~

*/
bool intersectionTestForSetOp(AVLSegment* s1, AVLSegment* s2, Event& event,
  priority_queue<Event, vector<Event>, greater<Event> >& q,
  bool leftIsSmaller) {

 if (s1->mightIntersect(*s2)) {
  AVLSegment* overlappingSegment = new AVLSegment();
  if (s1->intersect(*s2, *overlappingSegment)) {
   if (p2d_debug) {
    cout << endl << endl << "s1: ";
    s1->print();
    cout << "s2: ";
    s2->print();
    cout << "Schnittpunkt :";
    overlappingSegment->print();
    cout << endl << endl;
   }
   if (overlappingSegment->isPoint()) {
    if (!overlappingSegment->isLeftOf(event)) {
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
1 ~collectSegmentsForInverting~

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

   if (seg->compareInPosOrLeft(*(e.getLeftSegment()), event.getGridX(), v)
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
 if (segmentVector.back()->compareInPosOrLeft(*r, event.getGridX(), v) < 0) {
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
1 ~createNewSegments~

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

void createNewSegments(AVLSegment& s, Line2& result, int& edgeno,
  SetOperation op, mpq_class& internalScalefactor) {
 switch (op) {
 case union_op: {
  if (p2d_debug) {
   cout << "neues Segment in Line einfuegen" << endl;

   s.print();
  }
  int gxl, gyl, gxr, gyr;
  mpq_class pxl, pyl, pxr, pyr;
  p2d::prepareData(gxl, pxl,
    ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
  p2d::prepareData(gyl, pyl,
    ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
  p2d::prepareData(gxr, pxr,
    ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
  p2d::prepareData(gyr, pyr,
    ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));

  result.addSegment(true, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
  result.addSegment(false, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
  //result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
  //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
  //  s.getPreciseYR(), edgeno);
  //result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
  //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
  //  s.getPreciseYR(), edgeno);
  edgeno++;
  break;
 }
 case intersection_op: {
  if (s.getOwner() == both) {
   if (p2d_debug) {
    cout << "neues Segment in Line einfuegen" << endl;

    s.print();
   }
   int gxl, gyl, gxr, gyr;
   mpq_class pxl, pyl, pxr, pyr;
   p2d::prepareData(gxl, pxl,
     ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
   p2d::prepareData(gyl, pyl,
     ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
   p2d::prepareData(gxr, pxr,
     ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
   p2d::prepareData(gyr, pyr,
     ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));

   result.addSegment(true, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
   result.addSegment(false, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
   //result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
   //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
   //  s.getPreciseYR(), edgeno);
   //result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
   //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
   //  s.getPreciseYR(), edgeno);

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
   int gxl, gyl, gxr, gyr;
   mpq_class pxl, pyl, pxr, pyr;
   p2d::prepareData(gxl, pxl,
     ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
   p2d::prepareData(gyl, pyl,
     ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
   p2d::prepareData(gxr, pxr,
     ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
   p2d::prepareData(gyr, pyr,
     ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));

   result.addSegment(true, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
   result.addSegment(false, gxl, gyl, gxr, gyr, pxl, pyl, pxr, pyr, edgeno);
   //result.addSegment(true, s.getGridXL(), s.getGridYL(), s.getGridXR(),
   //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
   //  s.getPreciseYR(), edgeno);
   //result.addSegment(false, s.getGridXL(), s.getGridYL(), s.getGridXR(),
   //  s.getGridYR(), s.getPreciseXL(), s.getPreciseYL(), s.getPreciseXR(),
   //  s.getPreciseYR(), edgeno);
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
1 ~createNewSegments~

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
1 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  Line2& result, int& edgeno, SetOperation op, mpq_class& internalScalefactor) {
 for (size_t i = 0; i < segmentVector.size(); i++) {
  mpq_class px = event.getPreciseX();
  mpq_class py = event.getPreciseY();
  bool isEndpoint = segmentVector.at(i)->isEndpoint(event.getGridX(),
    event.getGridY(), px, py);
  if (!isEndpoint
    && (op == union_op
      || (op == intersection_op && segmentVector.at(i)->getOwner() == both)
      || (op == difference_op && segmentVector.at(i)->getOwner() == first))) {
   int gxl, gyl, gxr, gyr;
   mpq_class pxl, pyl, pxr, pyr;
   p2d::prepareData(gxl, pxl,
     ((segmentVector.at(i)->getGridXL()+
       segmentVector.at(i)->getPreciseXL())/internalScalefactor));
   p2d::prepareData(gyl, pyl,
     ((segmentVector.at(i)->getGridYL()+
       segmentVector.at(i)->getPreciseYL())/internalScalefactor));
   p2d::prepareData(gxr, pxr,
     ((event.getGridX()+
       event.getPreciseX())/internalScalefactor));
   p2d::prepareData(gyr, pyr,
     ((event.getGridY()+
       event.getPreciseY())/internalScalefactor));
   if (p2d_debug) {
    cout << "neues Segment in Line einfuegen" << endl;

    cout << gxl << " " << gyl << " " << gxr << " "  << gyr
      << " " << pxl << " " << pyl << " " << pxr << " " << pyr << endl;
   }
   result.addSegment(true, gxl, gyl, gxr, gyr,
     pxl, pyl, pxr, pyr, edgeno);
   result.addSegment(false, gxl, gyl, gxr, gyr,
     pxl, pyl, pxr, pyr, edgeno);
   //result.addSegment(true, segmentVector.at(i)->getGridXL(),
   //  segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
   //  segmentVector.at(i)->getPreciseXL(), segmentVector.at(i)->getPreciseYL(),
   //  event.getPreciseX(), event.getPreciseY(), edgeno);
   //result.addSegment(false, segmentVector.at(i)->getGridXL(),
   //  segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
   //  segmentVector.at(i)->getPreciseXL(), segmentVector.at(i)->getPreciseYL(),
   //  event.getPreciseX(), event.getPreciseY(), edgeno);
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
1 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno,
  int scalefactor, SetOperation op) {
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
     Reg2PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
       s->getGridYL(), scalefactor);
     Reg2PrecisePoint p2(event.getPreciseX(), event.getGridX(),
       event.getPreciseY(), event.getGridY(), scalefactor);
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
     Reg2PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
       s->getGridYL(), scalefactor);
     Reg2PrecisePoint p2(event.getPreciseX(), event.getGridX(),
       event.getPreciseY(), event.getGridY(), scalefactor);
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
      Reg2PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL(), scalefactor);
      Reg2PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY(), scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
      Reg2PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL(), scalefactor);
      Reg2PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY(), scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
      hs.attr.edgeno = edgeno;
      hs.attr.insideAbove = !(s->getSecondInsideAbove());
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
      Reg2PrecisePoint p1(s->getPreciseXL(), s->getGridXL(), s->getPreciseYL(),
        s->getGridYL(), scalefactor);
      Reg2PrecisePoint p2(event.getPreciseX(), event.getGridX(),
        event.getPreciseY(), event.getGridY(), scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
}

/*
1 ~setInsideAbove~

 For buildRegion
*/
void setInsideAbove(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor) {
 size_t size = segmentVector.size();

 AVLSegment* next = successor;
 for (size_t i = 0; i < size; i++) {
  AVLSegment* s = segmentVector.at(i);
  mpq_class px = event.getPreciseX();
  mpq_class py = event.getPreciseY();
  bool endsInEventpoint = s->endsInPoint(event.getGridX(), event.getGridY(), px,
    py);

  if (!endsInEventpoint) {

   if (s->getOwner() != both) {
    if (!next) {
     s->setFirstInsideAbove(false);
    } else {
     s->setFirstInsideAbove(!next->getFirstInsideAbove());
    }
   }
   next = s;
  }
 } //end for
} // end setInsideAbove


/*
1 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno, int scalefactor,
  SetOperation op, mpq_class& internalScalefactor) {
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

     int x, y;
     mpq_class px, py;
     prepareData(x, px,
       ((s->getGridXL()+s->getPreciseXL())/internalScalefactor));
     prepareData(y, py,
       ((s->getGridYL()+s->getPreciseYL())/internalScalefactor));
     Reg2PrecisePoint p1(px, x, py, y, scalefactor);

     prepareData(x, px,
       ((event.getGridX()+event.getPreciseX())/internalScalefactor));
     prepareData(y, py,
       ((event.getGridY()+event.getPreciseY())/internalScalefactor));
     Reg2PrecisePoint p2(px, x, py, y, scalefactor);
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
     int x, y;
     mpq_class px, py;
     prepareData(x, px,
       ((s->getGridXL()+s->getPreciseXL())/internalScalefactor));
     prepareData(y, py,
       ((s->getGridYL()+s->getPreciseYL())/internalScalefactor));
     Reg2PrecisePoint p1(px, x, py, y, scalefactor);

     prepareData(x, px,
       ((event.getGridX()+event.getPreciseX())/internalScalefactor));
     prepareData(y, py,
       ((event.getGridY()+event.getPreciseY())/internalScalefactor));
     Reg2PrecisePoint p2(px, x, py, y, scalefactor);
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
      int x, y;
      mpq_class px, py;
      prepareData(x, px,
        ((s->getGridXL()+s->getPreciseXL())/internalScalefactor));
      prepareData(y, py,
        ((s->getGridYL()+s->getPreciseYL())/internalScalefactor));
      Reg2PrecisePoint p1(px, x, py, y, scalefactor);

      prepareData(x, px,
        ((event.getGridX()+event.getPreciseX())/internalScalefactor));
      prepareData(y, py,
        ((event.getGridY()+event.getPreciseY())/internalScalefactor));
      Reg2PrecisePoint p2(px, x, py, y, scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
      int x, y;
      mpq_class px, py;
      prepareData(x, px,
        ((s->getGridXL()+s->getPreciseXL())/internalScalefactor));
      prepareData(y, py,
        ((s->getGridYL()+s->getPreciseYL())/internalScalefactor));
      Reg2PrecisePoint p1(px, x, py, y, scalefactor);

      prepareData(x, px,
        ((event.getGridX()+event.getPreciseX())/internalScalefactor));
      prepareData(y, py,
        ((event.getGridY()+event.getPreciseY())/internalScalefactor));
      Reg2PrecisePoint p2(px, x, py, y, scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
      hs.attr.edgeno = edgeno;
      hs.attr.insideAbove = !(s->getSecondInsideAbove());
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
      int x, y;
      mpq_class px, py;
      prepareData(x, px,
        ((s->getGridXL()+s->getPreciseXL())/internalScalefactor));
      prepareData(y, py,
        ((s->getGridYL()+s->getPreciseYL())/internalScalefactor));
      Reg2PrecisePoint p1(px, x, py, y, scalefactor);

      prepareData(x, px,
        ((event.getGridX()+event.getPreciseX())/internalScalefactor));
      prepareData(y, py,
        ((event.getGridY()+event.getPreciseY())/internalScalefactor));
      Reg2PrecisePoint p2(px, x, py, y, scalefactor);
      Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
}

/*
1 ~checkSegments~

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
1 ~checkSegment~

*/
void checkSegment(AVLSegment& seg, bool& result, RelationshipOperation op) {
 switch (op) {
 case (intersects_op): {
  if ((seg.getConAbove() == 2) || (seg.getConBelow() == 2)) {
   if (p2d_debug) {
    cout << "the regions intersect." << endl;
    seg.print();
   }
   result = true;
  }
  break;
 }
 case overlaps_op: {
  if ((seg.getConAbove() == 2) || (seg.getConBelow() == 2)) {
   if (p2d_debug) {
    cout << "the regions overlap." << endl;
    seg.print();
   }
   result = true;
  }
  break;
 } //case: overlaps_op
 case inside_op: {
  switch (seg.getOwner()) {
  case first: {
   if (seg.getConAbove() + seg.getConBelow() != 3) {
    if (p2d_debug) {
     cout << "r1 is not within r2-first" << endl;
     seg.print();
    }
    result = false;
   }
   break;
  }
  case second: {
   if ((seg.getConAbove() + seg.getConBelow()) != 1) {
    if (p2d_debug) {
     cout << "r1 is not within r2-second" << endl;
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
     cout << "r1 is not within r2-both" << endl;
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
1 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  int scalefactor, SetOperation op) {
 switch (op) {
 case union_op: {
  if ((s.getConAbove() == 0) || (s.getConBelow() == 0)) {
   if (p2d_debug) {
    cout << "neues Segment in Region einfuegen" << endl;
    s.print();
   }
   Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
     s.getGridYL(), scalefactor);
   Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
     s.getGridYR(), scalefactor);
   Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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

   Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
     s.getGridYL(), scalefactor);
   Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
     s.getGridYR(), scalefactor);
   Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
    Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL(), scalefactor);
    Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR(), scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
    if (p2d_debug) {
     cout << "speichere Segment" << endl;
     s.print();
    }
    Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL(), scalefactor);
    Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR(), scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
    hs.attr.edgeno = edgeno;
    hs.attr.insideAbove = !(s.getSecondInsideAbove());
    result += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    result += hs;
    edgeno++;
   }
   break;
  }
  case both: {
   if ((s.getConAbove() == 1) && (s.getConBelow() == 1)) {
    if (p2d_debug) {
     cout << "speichere Segment" << endl;
     s.print();
    }
    Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
      s.getGridYL(), scalefactor);
    Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
      s.getGridYR(), scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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

1 ~createNewSegments~

 for buildRegion

*/
void createNewSegments(Region2& result, AVLSegment& s,
  int& edgeno, int scalefactor){
   if (p2d_debug) {
    cout << "neues Segment in Region einfuegen" << endl;
    s.print();
   }
   Reg2PrecisePoint p1(s.getPreciseXL(), s.getGridXL(), s.getPreciseYL(),
     s.getGridYL(), scalefactor);
   Reg2PrecisePoint p2(s.getPreciseXR(), s.getGridXR(), s.getPreciseYR(),
     s.getGridYR(), scalefactor);
   Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
   hs.attr.edgeno = edgeno;
   hs.attr.insideAbove = (s.getFirstInsideAbove());
   result += hs;
   hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
   result += hs;
   edgeno++;
}


/*
1 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  int scalefactor, SetOperation op, mpq_class& internalScalefactor) {
 switch (op) {
 case union_op: {
  if ((s.getConAbove() == 0) || (s.getConBelow() == 0)) {
   if (p2d_debug) {
    cout << "neues Segment in Region einfuegen" << endl;
    s.print();
   }
   int x, y;
   mpq_class px, py;
   prepareData(x, px,
     ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
   prepareData(y, py,
     ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
   Reg2PrecisePoint p1(px, x, py, y, scalefactor);
      prepareData(x, px,
     ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
   prepareData(y, py,
     ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));
   Reg2PrecisePoint p2(px, x, py, y, scalefactor);
   Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
   int x, y;
   mpq_class px, py;
   prepareData(x, px,
     ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
   prepareData(y, py,
     ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
   Reg2PrecisePoint p1(px, x, py, y, scalefactor);
   prepareData(x, px,
     ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
   prepareData(y, py,
     ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));
   Reg2PrecisePoint p2(px, x, py, y, scalefactor);
   Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
    int x, y;
    mpq_class px, py;
    prepareData(x, px,
      ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
    Reg2PrecisePoint p1(px, x, py, y, scalefactor);
    prepareData(x, px,
      ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));
    Reg2PrecisePoint p2(px, x, py, y, scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
    if (p2d_debug) {
     cout << "speichere Segment" << endl;
     s.print();
    }
    int x, y;
    mpq_class px, py;
    prepareData(x, px,
      ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
    Reg2PrecisePoint p1(px, x, py, y, scalefactor);
    prepareData(x, px,
      ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));
    Reg2PrecisePoint p2(px, x, py, y, scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
    hs.attr.edgeno = edgeno;
    hs.attr.insideAbove = !(s.getSecondInsideAbove());
    result += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    result += hs;
    edgeno++;
   }
   break;
  }
  case both: {
   if ((s.getConAbove() == 1) && (s.getConBelow() == 1)) {
    if (p2d_debug) {
     cout << "speichere Segment" << endl;
     s.print();
    }
    int x, y;
    mpq_class px, py;
    prepareData(x, px,
      ((s.getGridXL()+s.getPreciseXL())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYL()+s.getPreciseYL())/internalScalefactor));
    Reg2PrecisePoint p1(px, x, py, y, scalefactor);
    prepareData(x, px,
      ((s.getGridXR()+s.getPreciseXR())/internalScalefactor));
    prepareData(y, py,
      ((s.getGridYR()+s.getPreciseYR())/internalScalefactor));
    Reg2PrecisePoint p2(px, x, py, y, scalefactor);
    Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
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
1 ~Realminize~

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

 Event event;

 while (selectNext(src, pos, q, event) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForRealminize(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForRealminize(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (event.isValid()) {
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
     event.print();
    }
    mpq_class v1 = event.getPreciseX();
    mpq_class v2 = event.getPreciseY();
    //segmentVector stores all segments which intersect in the
    //same event-point
    vector<AVLSegment*> segmentVector;

    AVLSegment* seg = event.getLeftSegment();
    AVLSegment* r = event.getRightSegment();

    segmentVector.push_back(seg);
    Event e = q.top();
    size_t predIndex = 0;
    size_t sucIndex = 0;

    //if there are only 2 intersecting segments,
    //which are parallel (and the right endpoint of the first
    //segment is the left endpoint of the second segment),
    //the segments don't have to be inverted.
    bool inversionNecessary = false;

    while (e.isIntersectionEvent() && e.getGridX() == event.getGridX()
      && e.getGridY() == event.getGridY()
      && cmp(e.getPreciseX(), event.getPreciseX()) == 0
      && cmp(e.getPreciseY(), event.getPreciseY()) == 0) {
     if (seg->compareInPosOrLeft(*(e.getLeftSegment()), event.getGridX(), v1)
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

    if (segmentVector.back()->compareInPosOrLeft(*r, event.getGridX(), v1)
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
     sss.invertSegments(segmentVector, event.getGridX(), v1, event.getGridY(),
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
     mpq_class px = event.getPreciseX();
     mpq_class py = event.getPreciseY();
     for (size_t i = 0; i < segmentVector.size(); i++) {
      bool isEndpoint = segmentVector.at(i)->isEndpoint(event.getGridX(),
        event.getGridY(), px, py);
      if (!isEndpoint) {
       if (p2d_debug) {
        cout << "neues Segment in Line einfuegen" << endl;

        cout << segmentVector.at(i)->getGridXL() << " "
          << segmentVector.at(i)->getGridYL() << " "
          << event.getGridX() << " "
          << event.getGridY() << " "
          << segmentVector.at(i)->getPreciseXL().get_d() << " "
          << segmentVector.at(i)->getPreciseYL().get_d() << " "
          << event.getPreciseX().get_d() << " "
          << event.getPreciseY().get_d()
          << endl;
       }
       result.addSegment(true, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event.getPreciseX(),
         event.getPreciseY(), edgeno);
       result.addSegment(false, segmentVector.at(i)->getGridXL(),
         segmentVector.at(i)->getGridYL(), event.getGridX(), event.getGridY(),
         segmentVector.at(i)->getPreciseXL(),
         segmentVector.at(i)->getPreciseYL(), event.getPreciseX(),
         event.getPreciseY(), edgeno);
       edgeno++;

       AVLSegment* newLeft = new AVLSegment(event.getGridX(),
         event.getGridY(), segmentVector.at(i)->getGridXR(),
         segmentVector.at(i)->getGridYR(), event.getPreciseX(),
         event.getPreciseY(), segmentVector.at(i)->getPreciseXR(),
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

 result.EndBulkLoad(false, false, false);
} // end Realminize


/*
1 ~BuildRegion~

*/
void BuildRegion(/*const*/Line2& line, Region2& result, int scalefactor) {
 result.Clear();
 if (!line.IsDefined() ) {
  result.SetDefined(false);
  return;
 }
 result.SetDefined(true);

 if (line.Size() == 0) {
  return;
 }

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;

 int pos1 = 0;

 result.SetScaleFactor(scalefactor);

 result.StartBulkLoad();
 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 int edgeno = 0;

 Event event;

 while ((selectNext(line, pos1, q, event)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }
   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      createNewSegments(result, *current, edgeno, scalefactor);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }

   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      setInsideAbove(segmentVector, event, suc);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad();


} // buildRegion line2 [->] region2
/*
1 ~SetOp~

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
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     edgeno++;
    }

   }
   s = line2.Size();
   for (int i = 0; i < s; i++) {
    line2.get(i, sd);
    if (sd.IsLeftDomPoint()) {
     result.addSegment(true, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
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

 Event event;

 result.StartBulkLoad();
 while ((selectNext(line1, pos1, line2, pos2, q, event)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()) {
      createNewSegments(*current, result, edgeno, op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      createNewSegments(segmentVector, event, result, edgeno, op);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad(true, false);
} // setop line2 x line2 [->] line2

/*
1 ~SetOp~

*/
void SetOpWithScaling(const Line2& line1, const Line2& line2, Line2& result,
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
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     edgeno++;
    }

   }
   s = line2.Size();
   for (int i = 0; i < s; i++) {
    line2.get(i, sd);
    if (sd.IsLeftDomPoint()) {
     result.addSegment(true, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
     result.addSegment(false, sd.getLeftX(), sd.getLeftY(), sd.getRightX(),
       sd.getRightY(), sd.getPreciseLeftX(*flob), sd.getPreciseLeftY(*flob),
       sd.getPreciseRightX(*flob), sd.getPreciseRightY(*flob), edgeno);
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

 mpq_class internalScalefactor = computeScalefactor(line1, line2);

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

 Event event;

 result.StartBulkLoad();
 while ((selectNext(line1, pos1, line2, pos2, q,
   event, internalScalefactor)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()) {
      createNewSegments(*current, result, edgeno, op, internalScalefactor);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      createNewSegments(segmentVector, event, result, edgeno, op,
        internalScalefactor);
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad(true, false);
} // setopWithScaling line2 x line2 [->] line2

/*
1 ~intersects~

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

 Event event;

 bool intersect = false;

 while ((!intersect
   && (selectNext(line1, pos1, line2, pos2, q, event)) != none)) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();

   sss.insert(current, pred, suc);

   if (current->getOwner() == both) {
    intersect = true;
   }
   mpq_class v = current->getPreciseXL();
   if (pred) {
    if (intersectionTestForSetOp(current, pred, event, q, false)) {
     if ((pred->getOwner() != current->getOwner())
       || (current->getOwner() == both)) {
      intersect = true;
     }
    }
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     if (intersectionTestForSetOp(current, suc, event, q, true)) {
      if ((suc->getOwner() != current->getOwner())
        || (current->getOwner() == both)) {
       intersect = true;
      }
     }
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       if (intersectionTestForSetOp(pred, suc, event, q, true)) {
        if ((pred->getOwner() != suc->getOwner())||(pred->getOwner()==both)) {
         intersect = true;
        }
       }
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      if (intersectionTestForSetOp(pred, suc, event, q, true)) {
       if ((pred->getOwner() != suc->getOwner())||(pred->getOwner()==both)) {
        intersect = true;
       }
      }
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       if (intersectionTestForSetOp(pred, right, event, q, true)) {
        if ((pred->getOwner() != right->getOwner())||(pred->getOwner()==both)) {
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
        if ((suc->getOwner() != left->getOwner())||(suc->getOwner()==both)) {
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
 return intersect;
} // intersects line2 x line2 [->] bool

/*
1 ~intersects~

*/
bool intersectsWithScaling(const Line2& line1, const Line2& line2,
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

 mpq_class internalScalefactor = computeScalefactor(line1, line2);

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current = NULL;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 Event event;

 bool intersect = false;

 while ((!intersect
   && (selectNext(line1, pos1, line2, pos2, q,
     event, internalScalefactor)) != none)) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();

   sss.insert(current, pred, suc);

   if (current->getOwner() == both) {
    intersect = true;
   }
   mpq_class v = current->getPreciseXL();
   if (pred) {
    if (intersectionTestForSetOp(current, pred, event, q, false)) {
     if ((pred->getOwner() != current->getOwner())
       || (current->getOwner() == both)) {
      intersect = true;
     }
    }
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     if (intersectionTestForSetOp(current, suc, event, q, true)) {
      if ((suc->getOwner() != current->getOwner())
        || (current->getOwner() == both)) {
       intersect = true;
      }
     }
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {

    if (p2d_debug) {
     event.print();
     sss.inorder();
    }
    current = event.getSegment();
    if (current->isValid()){
      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       if (intersectionTestForSetOp(pred, suc, event, q, true)) {
        if ((pred->getOwner() != suc->getOwner())||(pred->getOwner()==both)) {
         intersect = true;
        }
       }
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      if (intersectionTestForSetOp(pred, suc, event, q, true)) {
       if ((pred->getOwner() != suc->getOwner())||(pred->getOwner()==both)) {
        intersect = true;
       }
      }
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       if (intersectionTestForSetOp(pred, right, event, q, true)) {
        if ((pred->getOwner() != right->getOwner())||(pred->getOwner()==both)) {
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
        if ((suc->getOwner() != left->getOwner())||(suc->getOwner()==both)) {
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
 return intersect;
} // intersectsWithScaling line2 x line2 [->] bool

/*
1 ~crossings~

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

 if (!line1.BoundingBox().Intersects(line2.BoundingBox(), geoid)) {
  return;
 }

 result.StartBulkLoad();

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 Event event;

 while (selectNext(line1, pos1, line2, pos2, q, event) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();

   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }
   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }

   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
     mpq_class v1 = current->getPreciseXR();
     sss.removeGetNeighbor(current, pred, suc);
     current->changeValidity(false);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    } else {
      mpq_class v1 = event.getPreciseX();
      sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (p2d_debug) {
     event.print();
    }

    mpq_class v1 = event.getPreciseX();
    mpq_class v2 = event.getPreciseY();
    vector<AVLSegment*> segmentVector;

    size_t predIndex = 0;
    size_t sucIndex = 0;
    bool inversionNecessary = false;

    collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
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
     Point2 p(true, event.getGridX(), event.getGridY(),
       event.getPreciseX(), event.getPreciseY());
     result.addPoint( p );
    }

    if (inversionNecessary) {
     left = segmentVector.at(0);
     right = segmentVector.back();
     sss.invertSegments(segmentVector, event.getGridX(), v1, event.getGridY(),
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
     mpq_class px = event.getPreciseX();
     mpq_class py = event.getPreciseY();

    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }

 }

 result.EndBulkLoad();
} // crossings line2 x line2 [->] points2

/*
1 ~crossings~

*/
void crossingsWithScaling(const Line2& line1, const Line2& line2,
  Points2& result, const Geoid* geoid/*=0*/) {
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

 if (!line1.BoundingBox().Intersects(line2.BoundingBox(), geoid)) {
  return;
 }
 result.StartBulkLoad();

 mpq_class internalScalefactor = p2d::computeScalefactor(line1, line2);

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* left = NULL;
 AVLSegment* right = NULL;

 Event event;

 while (selectNext(line1, pos1, line2, pos2, q,
   event, internalScalefactor) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();

   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }
   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }

   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
     mpq_class v1 = current->getPreciseXR();
     sss.removeGetNeighbor(current, pred, suc);
     current->changeValidity(false);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    } else {
      mpq_class v1 = event.getPreciseX();
      sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
     }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (p2d_debug) {
     event.print();
    }

    mpq_class v1 = event.getPreciseX();
    mpq_class v2 = event.getPreciseY();
    vector<AVLSegment*> segmentVector;

    size_t predIndex = 0;
    size_t sucIndex = 0;
    bool inversionNecessary = false;

    collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
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
     int gx, gy;
     mpq_class px, py;
     p2d::prepareData(gx, px,
       ((event.getGridX()+event.getPreciseX())/internalScalefactor));
     p2d::prepareData(gy, py,
       ((event.getGridY()+event.getPreciseY())/internalScalefactor));
     Point2 p(true, gx, gy, px, py);
     result.addPoint( p );
    }

    if (inversionNecessary) {
     left = segmentVector.at(0);
     right = segmentVector.back();
     sss.invertSegments(segmentVector, event.getGridX(), v1, event.getGridY(),
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
     mpq_class px = event.getPreciseX();
     mpq_class py = event.getPreciseY();

    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad();

} // crossingsWithScaling line2 x line2 [->] points2

/*
1 ~SetOp~

*/
void SetOp(/*const*/Region2& reg1, /*const*/Region2& reg2, Region2& result,
  SetOperation op, const Geoid* geoid/*=0*/) {
 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }
 result.Clear();
 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 result.SetScaleFactor(reg1.GetScaleFactor());

 if (!reg1.BoundingBox().Intersects(reg2.BoundingBox(), geoid)) {
  switch (op) {
  case avlseg::union_op: {
   result.StartBulkLoad();
   int edgeno = 0;
   int s = reg1.Size();
   Reg2GridHalfSegment gs;
   Reg2PrecHalfSegment ps;
   for (int i = 0; i < s; i++) {
    reg1.getgridCoordinates()->Get(i, gs);
    reg1.getprecCoordinates()->Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     Reg2PrecisePoint p1(ps.GetlPointx(reg1.getpreciseCoordinates()),
       gs.GetLeftPointX(), ps.GetlPointy(reg1.getpreciseCoordinates()),
       gs.GetLeftPointY(), reg1.GetScaleFactor());
     Reg2PrecisePoint p2(ps.GetrPointx(reg1.getpreciseCoordinates()),
       gs.GetRightPointX(), ps.GetrPointy(reg1.getpreciseCoordinates()),
       gs.GetRightPointY(), reg1.GetScaleFactor());
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(false);
     result += hs;
     edgeno++;
    }
   }
   s = reg2.Size();
   for (int i = 0; i < s; i++) {
    reg2.getgridCoordinates()->Get(i, gs);
    reg2.getprecCoordinates()->Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     Reg2PrecisePoint p1(ps.GetlPointx(reg2.getpreciseCoordinates()),
       gs.GetLeftPointX(), ps.GetlPointy(reg2.getpreciseCoordinates()),
       gs.GetLeftPointY(), reg1.GetScaleFactor());
     Reg2PrecisePoint p2(ps.GetrPointx(reg2.getpreciseCoordinates()),
       gs.GetRightPointX(), ps.GetrPointy(reg2.getpreciseCoordinates()),
       gs.GetRightPointY(), reg1.GetScaleFactor());
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(false);
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

 Event event;

 result.StartBulkLoad();
 while ((selectNext(reg1, pos1, reg2, pos2, q, event)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
   } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }

    current = event.getSegment();

    if (current->isValid()){
      createNewSegments(*current, result, edgeno, reg1.GetScaleFactor(), op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }

   } else {
    //intersection Event
    if (event.isValid()) {

     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      createNewSegments(segmentVector, event, suc, result, edgeno,
        reg1.GetScaleFactor(), op);

      } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad();
} // setOP region2 x region2 [->] region2

/*
1 ~intersects~

*/
bool intersects(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 Event event;

 bool intersect = false;

 while (!intersect && (selectNext(reg1, pos1, reg2, pos2, q, event)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
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
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     if (intersectionTestForSetOp(current, suc, event, q, true)) {
      if ((suc->getOwner() != current->getOwner())) {
       intersect = true;
      }
     }
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, intersect, intersects_op);

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
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      if(intersectionTestForSetOp(pred, suc, event, q, true)){
        if ((pred->getOwner() != suc->getOwner())) {
         intersect = true;
        }
      }
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       if (intersectionTestForSetOp(pred, right, event, q, true)) {
        if ((pred->getOwner() != right->getOwner())) {
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
        if ((suc->getOwner() != left->getOwner())) {
         intersect = true;
        }
       }
      }
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, intersect, intersects_op);
     }  else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 return intersect;
} // intersects region2 x region2 [->] bool

/*
1 ~overlaps~

*/
bool overlaps(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 Event event;

 bool overlaps = false;

 while (!overlaps && (selectNext(reg1, pos1, reg2, pos2, q, event)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, overlaps, overlaps_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, overlaps, overlaps_op);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 return overlaps;
} // overlaps region2 x region2 [->] bool

/*
1 ~inside~

*/
bool inside(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {
 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 Event event;

 bool inside = true;
 while (inside && (selectNext(reg1, pos1, reg2, pos2, q, event)) != none) {

  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, inside, inside_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, inside, inside_op);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 return inside;
} // inside region2 x region2 [->] bool

/*
1 ~SetOp~

*/
void SetOpWithScaling(/*const*/Region2& reg1, /*const*/Region2& reg2,
  Region2& result, SetOperation op, const Geoid* geoid/*=0*/) {
 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }
 result.Clear();
 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())) {
  result.SetDefined(false);
  return;
 }

 if (reg1.IsDefined() && reg2.IsDefined() &&
   (reg1.GetScaleFactor()!=reg2.GetScaleFactor())){
  cout <<"The regions have different scalefactors."<<endl;
  result.SetDefined(false);
  return;
 }

 result.SetScaleFactor(reg1.GetScaleFactor());

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
   Reg2GridHalfSegment gs;
   Reg2PrecHalfSegment ps;
   for (int i = 0; i < s; i++) {
    reg1.getgridCoordinates()->Get(i, gs);
    reg1.getprecCoordinates()->Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     Reg2PrecisePoint p1(ps.GetlPointx(reg1.getpreciseCoordinates()),
       gs.GetLeftPointX(), ps.GetlPointy(reg1.getpreciseCoordinates()),
       gs.GetLeftPointY(), reg1.GetScaleFactor());
     Reg2PrecisePoint p2(ps.GetrPointx(reg1.getpreciseCoordinates()),
       gs.GetRightPointX(), ps.GetrPointy(reg1.getpreciseCoordinates()),
       gs.GetRightPointY(), reg1.GetScaleFactor());
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(false);
     result += hs;
     edgeno++;
    }
   }
   s = reg2.Size();
   for (int i = 0; i < s; i++) {
    reg2.getgridCoordinates()->Get(i, gs);
    reg2.getprecCoordinates()->Get(i, ps);
    if (gs.IsLeftDomPoint()) {
     Reg2PrecisePoint p1(ps.GetlPointx(reg2.getpreciseCoordinates()),
       gs.GetLeftPointX(), ps.GetlPointy(reg2.getpreciseCoordinates()),
       gs.GetLeftPointY(), reg1.GetScaleFactor());
     Reg2PrecisePoint p2(ps.GetrPointx(reg2.getpreciseCoordinates()),
       gs.GetRightPointX(), ps.GetrPointy(reg2.getpreciseCoordinates()),
       gs.GetRightPointY(), reg1.GetScaleFactor());
     Reg2PreciseHalfSegment hs = Reg2PreciseHalfSegment(true, p1, p2);
     hs.attr.edgeno = edgeno;
     hs.attr.insideAbove = (gs.attr.insideAbove);
     result += hs;
     hs.SetLeftDomPoint(false);
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

 mpq_class internalScalefactor = computeScalefactor(reg1, reg2);

 //start plane sweep
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

 Event event;

 result.StartBulkLoad();
 while ((selectNext(reg1, pos1, reg2, pos2, q,
   event, internalScalefactor)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }

  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      createNewSegments(*current, result, edgeno, reg1.GetScaleFactor(), op,
        internalScalefactor);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }

   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      createNewSegments(segmentVector, event, suc, result, edgeno,
        reg1.GetScaleFactor(), op, internalScalefactor);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 result.EndBulkLoad();

} // setOPWithScalefactor region2 x region2 [->] region2

/*
1 ~intersects~

*/
bool intersectsWithScaling(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || reg1.GetScaleFactor()!=reg2.GetScaleFactor()) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 mpq_class internalScalefactor = computeScalefactor(reg1, reg2);

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* right = NULL;
 AVLSegment* left = NULL;

 Event event;

 bool intersect = false;

 while (!intersect
   && (selectNext(reg1, pos1, reg2, pos2, q,
     event, internalScalefactor)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
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
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     if (intersectionTestForSetOp(current, suc, event, q, true)) {
      if ((suc->getOwner() != current->getOwner())) {
       intersect = true;
      }
     }
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, intersect, intersects_op);

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
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      if(intersectionTestForSetOp(pred, suc, event, q, true)){
        if ((pred->getOwner() != suc->getOwner())) {
         intersect = true;
        }
      }
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

      if (p2d_debug && pred) {
       cout << "intersection-test for" << endl;
       right->print();
       cout << " and " << endl;
       pred->print();
      }
      if (pred) {
       if (intersectionTestForSetOp(pred, right, event, q, true)) {
        if ((pred->getOwner() != right->getOwner())) {
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
        if ((suc->getOwner() != left->getOwner())) {
         intersect = true;
        }
       }
      }
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, intersect, intersects_op);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 return intersect;
} // intersectsWithScalefactor region2 x region2 [->] bool

/*
1 ~overlaps~

*/
bool overlapsWithScaling(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {

 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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

 mpq_class internalScalefactor = computeScalefactor(reg1, reg2);

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* right = NULL;
 AVLSegment* left = NULL;

 Event event;

 bool overlaps = false;

 while (!overlaps
   && (selectNext(reg1, pos1, reg2, pos2, q,
     event, internalScalefactor)) != none) {
  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);

   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, overlaps, overlaps_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, overlaps, overlaps_op);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }

 return overlaps;
} // overlapsWithScalefactor region2 x region2 [->] bool

/*
1 ~inside~

*/
bool insideWithScaling(/*const*/Region2& reg1, /*const*/Region2& reg2,
  const Geoid* geoid/*=0*/) {
 if (geoid) {
  cerr << ": Spherical geometry not implemented." << endl;
  assert(false);
 }

 if (!reg1.IsDefined() || !reg2.IsDefined() || (geoid && !geoid->IsDefined())
   || (reg1.GetScaleFactor()!=reg2.GetScaleFactor())) {
  if (reg1.GetScaleFactor()!=reg2.GetScaleFactor()){
   cout <<"The regions have different scalefactors."<<endl;
  }
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
 cout <<"PS noetig"<<endl;
 assert(reg1.IsOrdered());
 assert(reg2.IsOrdered());

 mpq_class internalScalefactor = computeScalefactor(reg1, reg2);

 priority_queue<Event, vector<Event>, greater<Event> > q;
 AVLTree sss;
 int pos1 = 0;
 int pos2 = 0;

 AVLSegment* current;
 AVLSegment* pred = NULL;
 AVLSegment* suc = NULL;
 AVLSegment* right = NULL;
 AVLSegment* left = NULL;

 Event event;

 bool inside = true;
 while (inside
   && (selectNext(reg1, pos1, reg2, pos2, q,
     event, internalScalefactor)) != none) {

  if (event.isLeftEndpointEvent()) {
   if (p2d_debug) {
    event.print();
   }
   current = event.getSegment();
   sss.insert(current, pred, suc);
   mpq_class v = current->getPreciseXL();
   if (pred) {
    intersectionTestForSetOp(current, pred, event, q, false);
   }

   if (!current->isValid()) {
    sss.removeInvalidSegment(current, event.getGridX(), v);
   } else {
    if (suc) {
     intersectionTestForSetOp(current, suc, event, q, true);
     if (!current->isValid()) {
      sss.removeInvalidSegment(current, event.getGridX(), v);
     }
    }
   }
   if (current->isValid() && !(current->getOwner() == both)) {
    Event re(rightEndpoint, current);
    q.push(re);
   }
  } else {
   if (event.isRightEndpointEvent()) {
    if (p2d_debug) {
     event.print();
    }
    current = event.getSegment();
    if (current->isValid()){
      checkSegment(*current, inside, inside_op);

      sss.removeGetNeighbor(current, pred, suc);
      current->changeValidity(false);
      if (pred && suc) {
       intersectionTestForSetOp(pred, suc, event, q, true);
      }
    } else {
     mpq_class v1 = event.getPreciseX();
     sss.removeGetNeighbor2(current, event.getGridX(), v1, pred, suc);
     if (pred && suc) {
      intersectionTestForSetOp(pred, suc, event, q, true);
     }
    }
    if (event.getNoOfChanges() == 0) {
     //this is the last event with ~current~
     delete current;
    }
   } else {
    //intersection Event
    if (event.isValid()) {
     if (p2d_debug) {
      event.print();
     }
     mpq_class v1 = event.getPreciseX();
     mpq_class v2 = event.getPreciseY();
     vector<AVLSegment*> segmentVector;

     size_t predIndex = 0;
     size_t sucIndex = 0;
     bool inversionNecessary = false;

     collectSegmentsForInverting(segmentVector, event, q, predIndex, sucIndex,
       inversionNecessary);

     if (inversionNecessary) {
      left = segmentVector.at(0);
      right = segmentVector.back();
      sss.invertSegments(segmentVector, event.getGridX(), v1,
        event.getGridY(), v2, pred, predIndex, suc, sucIndex);

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
      mpq_class px = event.getPreciseX();
      mpq_class py = event.getPreciseY();

      checkSegments(segmentVector, event, suc, inside, inside_op);
     } else {
      AVLSegment* higherSeg = segmentVector.at(0);
      AVLSegment* lowerSeg = segmentVector.at(1);
      higherSeg->setConAbove(lowerSeg->getConAbove());
      higherSeg->setConBelow(lowerSeg->getConBelow());
     }
    }
   }
  }
  if (p2d_debug) {
   cout << "the tree:" << endl;
   sss.inorder();
  }
 }
 return inside;
} // insideWithScalefactor region2 x region2 [->] bool
} //end of p2d

#endif /* AVL_TREE_CPP_*/
