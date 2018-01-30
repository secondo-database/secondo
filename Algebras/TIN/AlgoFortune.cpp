/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004-2007, University in Hagen, Department of Computer Science,
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

#include "AlgoFortune.h"
#include "TinHelper.h"
#include "string"
#include <stack>
#include <typeinfo>
#include <initializer_list>
extern "C" {
#include "quadmath.h"
}

#include "gmpxx.h"

#ifdef FORTUNE_NO_LOGGING
#define LOGGING_SWITCH_OFF
#endif
#include <TinLogging.h>

namespace tin {
BeachLineNode BeachLineNode::nullnode = BeachLineNode();
//int VoronoiEdge::instanceCounter = 0;
const VERTEX_COORDINATE BeachLineNode::BREAK_X_UNSET =
  -std::numeric_limits<VERTEX_COORDINATE>::max();
const __float128 BeachLineNode::PRECALC_UNSET = -std::numeric_limits<
  VERTEX_COORDINATE>::max();
BeachLine::BeachLine() :
  root(BeachLineNode()) {
 ySweepLine = -std::numeric_limits<VERTEX_COORDINATE>::min();

}

BeachLine::~BeachLine() {

}
void BeachLine::rescueVertices(VertexContainerSet* rescueSet) {

 BeachLineNode * current, *lbp, *rbp;
 const Vertex * tmp;
 bool newornot;

 current = root.getLeftMost();

 while (current) {
  tmp = rescueSet->insertVertex(current->site1, newornot);
  current->site1 = tmp;
  lbp = current->getLeftBreakPoint();
  if (lbp)
   lbp->site2 = tmp;
  rbp = current->getRightBreakPoint();
  if (rbp)
   rbp->site1 = tmp;

  current = current->getRightNeighbor();
 }

}
void BeachLine::print(std::ostream & os) const {
 root.print(os, 1);
 os << "-------Beachline left to right from sweepline---\n";
 BeachLineNode * current = 0;

 current = root.getRightMost();
 while (current != 0) {
  current->print(os);
  current = current->getLeftNeighbor();
 }

 os << "\n Sweepline y: " << ySweepLine << " \n";
}
void BeachLine::print_small(std::ostream & os) const {
 os << "-------Beachline left to right from sweepline---\n";
 BeachLineNode * current = 0;

 current = root.getRightMost();
 while (current != 0) {
  current->print(os);
  current = current->getLeftNeighbor();
 }

 os << "\n Sweepline y: " << ySweepLine << " \n";
}
BeachLineNode * BeachLine::insert(const Vertex * psite,
  BeachLineNode* pParabolaAbove) {
 if (psite == 0)
  throw std::invalid_argument(E_BEACHLINE_INSERT1);
 if (psite->getY() < ySweepLine)
  throw std::invalid_argument(E_BEACHLINE_INSERT);

 if (pParabolaAbove == &root) {
  root.pleftSon = new BeachLineNode(psite, &root);
  root.site1 = psite;
  root.site2 = 0;
  return &root;
 } else {
  return pParabolaAbove->insert(psite);
 }
}

void BeachLine::setSweepLine(VERTEX_COORDINATE iySweepLine) {
 ySweepLine = iySweepLine;
}
BeachLineNode* BeachLine::find(const VERTEX_COORDINATE x) const {

 return root.find(x, ySweepLine);
}
BeachLineNode::BeachLineNode() //construction of root
{
 site1 = 0;
 site2 = 0;
 pleftSon = 0;
 prightSon = 0;
 parent = 0;
 red = false; //root is always black
 validIterator = false;
 edge = 0;
 precalc_circle = PRECALC_UNSET;
 break_x = BREAK_X_UNSET;
}

BeachLineNode::BeachLineNode(BeachLineNode * iparent, bool leftins,
  BeachLineNode* left, BeachLineNode* right) //construction of inner node
  {
 if (left == 0 || right == 0)
  throw std::invalid_argument(E_BEACHLINENODE_CONSTRUCTOR);

 edge = 0;
 validIterator = false;
 red = true; // inner node is red initially

 pleftSon = left;
 prightSon = right;
 prightSon->parent = this;
 pleftSon->parent = this;

 site1 = left->getRightMost()->site1;
 site2 = right->getLeftMost()->site1;

 break_x = BREAK_X_UNSET;
 precalc_circle = PRECALC_UNSET;

 parent = iparent;

 if (leftins) {
  parent->pleftSon = this;
  parent->site1 = this->getRightMost()->getSite();
 } else {
  parent->prightSon = this;
  parent->site2 = this->getLeftMost()->getSite();
 }

 BeachLineNode * lbp = this->getLeftBreakPoint();
 if (lbp) {
  lbp->setSite2(this->getLeftMost()->getSite());
  lbp->resetBreakX();
 }
 BeachLineNode * rbp = this->getRightBreakPoint();

 if (rbp) {
  rbp->setSite1(this->getRightMost()->getSite());
  rbp->resetBreakX();
 }

}
BeachLineNode::BeachLineNode(const Vertex* pV1, BeachLineNode * iparent) {
 if (pV1 == 0)
  throw std::invalid_argument(E_BEACHLINENODE_CONSTRUCTOR1);
 validIterator = false;
 site1 = pV1;
 site2 = 0;
 pleftSon = 0;
 prightSon = 0;
 parent = iparent;
 red = false; //leaf is always black
 edge = 0;

 break_x = BREAK_X_UNSET;
 precalc_circle = PRECALC_UNSET;
}

BeachLineNode::~BeachLineNode() {
 if (pleftSon != 0 && pleftSon != &nullnode) {
  delete pleftSon;
  pleftSon = 0;
 }
 if (prightSon != 0 && prightSon != &nullnode) {
  delete prightSon;
  prightSon = 0;
 }

 if (edge)
  edge->destroy();

}
BeachLineNode* BeachLineNode::insert_no_split(const Vertex* pV,
  const VERTEX_COORDINATE sweepy) {

 if (pV == 0)
  throw std::invalid_argument(E_BEACHLINENODE_INSERT);

 if (*pV == *site1)
  throw std::invalid_argument(E_BEACHLINENODE_INSERT1);

 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_INSERT2);

 BeachLineNode * innerNode, *newarc, *lbp, *rbp;
 POINT_COORDINATE bpleft, bpright;

 lbp = this->getLeftBreakPoint();
 rbp = this->getRightBreakPoint();

 if (lbp)
  bpleft = lbp->getParabolaIntersection_sec(sweepy);
 if (rbp)
  bpright = rbp->getParabolaIntersection_sec(sweepy);

 newarc = new BeachLineNode(pV, &nullnode);

 if ((!rbp && lbp)
   || (rbp && lbp
     && absolute(bpright - pV->getX()) > absolute(bpleft - pV->getX()))) {
//here the left breakpoints edge has to be rescued and overwritten

  innerNode = new BeachLineNode(this->parent, this->isLeftChild(), newarc,
    this);
  innerNode->balanceTree();

  newarc->rescueVoronoiEdge(lbp->edge);
  lbp->edge = 0;
  lbp->precalc_circle = BeachLineNode::PRECALC_UNSET;
//for SiteCircleEvents both breakpoints represent new edges

  VoronoiEdge::getInstance(innerNode);
  if (lbp)
   VoronoiEdge::getInstance(lbp);

  return newarc;

 } else //insert left seen from the sweep line
 {
//here the right breakpoints edge has to be rescued and overwritten
  innerNode = new BeachLineNode(this->parent, this->isLeftChild(), this,
    newarc);
  innerNode->balanceTree();

  newarc->rescueVoronoiEdge(rbp->edge);
  rbp->edge = 0;
  rbp->precalc_circle = BeachLineNode::PRECALC_UNSET;
//for SiteCircleEvents both breakpoints represent new edge
  VoronoiEdge::getInstance(innerNode);
  if (rbp)
   VoronoiEdge::getInstance(rbp);

  return newarc;
 }

}
bool BeachLineNode::isVirtualRoot() const {

 if (!isLeaf() && !isRoot() && parent->isRoot())
  return true;

 return false;

}
/*
 The Method balanceTree balances the red black tree after insertion.

*/
void BeachLineNode::balanceTree() {
 if (isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_BALANCETREE);
 if (isRoot()) {
  return;
 }
 if (isVirtualRoot()) {
  red = false;
  return;
 }
 if (!parent->red)
  return;

 if (getUncle() && getUncle()->red) {
  parent->red = false;
  getUncle()->red = false;

  getGrandParent()->red = true;

  getGrandParent()->balanceTree();
  return;
 }

 if (isLeftChild() && !parent->isLeftChild()) {
  parent->rotateLeft();

  prightSon->getGrandParent()->red = true;
  prightSon->parent->red = false;
  prightSon->getGrandParent()->rotateRight();
  return;
 } else if (!isLeftChild() && parent->isLeftChild()) {
  parent->rotateRight();

  pleftSon->getGrandParent()->red = true;
  pleftSon->parent->red = false;
  pleftSon->getGrandParent()->rotateLeft();
  return;

 }

 if (!isLeftChild() && !parent->isLeftChild()) {
  getGrandParent()->red = true;
  parent->red = false;
  getGrandParent()->rotateRight();
  return;
 }

 if (isLeftChild() && parent->isLeftChild()) {
  getGrandParent()->red = true;
  parent->red = false;
  getGrandParent()->rotateLeft();
  return;
 }

 throw std::runtime_error(E_BEACHLINENODE_BALANCETREE1);

}
void BeachLineNode::balanceTreeDeletion() {
 BeachLineNode * bro, *brol, *bror;

 if (isRoot()) {
  return;
 }
 if (isVirtualRoot()) {
  return;
 }

 bro = getBrother();
 if (!bro)
  return;

 bror = bro->getRight();
 brol = bro->getLeft();

 if (!bror || !brol)
  return;

 if (!parent->red) {
  if (bro->red) {
   parent->red = true;
   bro->red = false;

   if (!isLeftChild()) {
    parent->rotateLeft();
    this->balanceTreeDeletion();
    return;
   } else {
    parent->rotateRight();
    this->balanceTreeDeletion();
    return;
   }
  } else {
   if (!bror->red && !brol->red) {
    bro->red = true;
    parent->balanceTreeDeletion();
    return;
   }
  }

 } else {
  if (!bro->red && !bror->red && !brol->red) {
   parent->red = false;
   bro->red = true;
   return;
  }
 }

 if (bro->isLeftChild() && !bro->red && bror->red && !brol->red) {
  bror->red = false;
  bro->red = true;
  bro->rotateRight();
  bro = this->getBrother();
  bror = bro->getRight();
  brol = bro->getLeft();
 } else if (!bro->isLeftChild() && !bro->red && !bror->red && brol->red) {
  brol->red = false;
  bro->red = true;
  bro->rotateLeft();
  bro = this->getBrother();
  bror = bro->getRight();
  brol = bro->getLeft();
 }

 if (!bror || !brol)
  return;
 if (bro->isLeaf())
  return;

 if (bro->isLeftChild()) {
  if (bro->getLeft()->isLeaf())
   return;

  bro->red = parent->red;
  parent->red = false;
  brol->red = false;
  bro->rotateLeft();
  return;
 } else {
  if (bro->getRight()->isLeaf())
   return;

  bro->red = parent->red;
  parent->red = false;
  bror->red = false;
  bro->rotateRight();
  return;
 }

 throw std::runtime_error(E_BEACHLINENODE_BALANCETREE1);

}
void BeachLineNode::rotateLeft() {
 if (isRoot())
  return;

 if (isLeaf() || !pleftSon || pleftSon->isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_ROTATELEFT);

 BeachLineNode *switchNode;

 if (isLeftChild()) {
  parent->setLeft(pleftSon);
  pleftSon->parent = parent;
 } else {
  parent->setRight(pleftSon);
  pleftSon->parent = parent;
 }

 switchNode = pleftSon->getRight();
 pleftSon->setRight(this);
 this->parent = pleftSon;

 this->setLeft(switchNode);
 switchNode->parent = this;
}
void BeachLineNode::rotateRight() {
 if (isRoot())
  return;

 if (isLeaf() || !prightSon || prightSon->isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_ROTATELEFT);

 BeachLineNode *switchNode;

 if (isLeftChild()) {
  parent->setLeft(prightSon);
  prightSon->parent = parent;
 } else {
  parent->setRight(prightSon);
  prightSon->parent = parent;
 }

 switchNode = prightSon->getLeft();
 prightSon->setLeft(this);
 this->parent = prightSon;

 this->setRight(switchNode);
 switchNode->parent = this;
}
VoronoiEdge* BeachLineNode::getVoronoiEdge() const {
 return edge;
}
void BeachLineNode::setVoronoiEdge(VoronoiEdge *e) {

 edge = e;
}
void BeachLineNode::resetVoronoiEdge() {
 if (isLeaf())
  throw std::runtime_error("(BeachLineNode::setVoronoiEdge)");

 if (edge)
  edge->destroyHard();

 edge = 0;

}
void BeachLineNode::resetRescuedVoronoiEdge() {
 edge = 0;
}
void BeachLineNode::rescueVoronoiEdge(VoronoiEdge *e) {
 if (edge)
  throw std::runtime_error(
    "The VoronoiEdge is already set.(BeachLineNode::rescueVoronoiEdge)");

 edge = e;
}
BeachLineNode* BeachLineNode::insert(const Vertex* pV) {
 if (pV == 0)
  throw std::invalid_argument(E_BEACHLINENODE_INSERT);

 if (*pV == *site1)
  throw std::invalid_argument(E_BEACHLINENODE_INSERT1);

 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_INSERT2);

 BeachLineNode * innerNode, *newarc, *removededgebp, *lbp, *rbp;

 newarc = new BeachLineNode(pV, &nullnode);

 if (site1->getY() == pV->getY()) {
  if (pV->getX() > site1->getX()) {
//on the right side seen from the sweep line

   removededgebp = this->getLeftBreakPoint();
   if (removededgebp)
    removededgebp->resetVoronoiEdge();
//VoronoiEdge was wrong so remove it

   innerNode = new BeachLineNode(this->parent, this->isLeftChild(), newarc,
     this);

   lbp = newarc->getLeftBreakPoint();
   rbp = newarc->getRightBreakPoint();

   innerNode->balanceTree();

//here a new edge is assigned to the (possible) two new breakpoints
   if (rbp)
    VoronoiEdge::getInstance(rbp)->addEnd(VORONOI_OPEN_END);
   if (lbp)
    VoronoiEdge::getInstance(lbp)->addEnd(VORONOI_OPEN_END);

   return newarc;

  } else {

   removededgebp = this->getRightBreakPoint();
   if (removededgebp)
    removededgebp->resetVoronoiEdge();
//VoronoiEdge was wrong so remove it

   innerNode = new BeachLineNode(this->parent, this->isLeftChild(), this,
     newarc);

   lbp = newarc->getLeftBreakPoint();
   rbp = newarc->getRightBreakPoint();

   innerNode->balanceTree();
//here a new edge is assigned to the (possible) two new breakpoints
   if (rbp)
    VoronoiEdge::getInstance(rbp)->addEnd(VORONOI_OPEN_END);
   if (lbp)
    VoronoiEdge::getInstance(lbp)->addEnd(VORONOI_OPEN_END);

   return newarc;
  }
 } else //arc split
 {
  prightSon = new BeachLineNode(site1, this);
  site2 = site1;
  innerNode = new BeachLineNode(this, true,
    new BeachLineNode(site1, &nullnode), newarc);

  this->red = true;
  this->balanceTree();
  innerNode->balanceTree();

//here a new edge is assigned to the two new breakpoints
  VoronoiEdge::getInstance(this, innerNode);

//this makes circle calculation faster
  __float128 x1 = site2->getX();
  __float128 y1 = site2->getY();
  __float128 x2 = site1->getX();
  __float128 y2 = site1->getY();

  this->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;
  innerNode->precalc_circle = -this->precalc_circle;

  return newarc;
 }

}
BeachLineNode * BeachLineNode::getLeftBreakPoint() const {
 const BeachLineNode * current;
 current = this;

 while (current && current->isLeftChild()) {
  current = current->getParent();
 }

 if (!current)
  return 0;

 return current->getParent();

}
void BeachLineNode::setSite2(const Vertex * psite2) {
 site2 = psite2;

}
void BeachLineNode::setSite1(const Vertex * psite1) {
 site1 = psite1;

}

BeachLineNode * BeachLineNode::getRightBreakPoint() const {
 const BeachLineNode * current;
 current = this;

 while (current && !current->isLeftChild()) {
  current = current->getParent();
 }

 if (!current)
  return 0;
 if (current->getParent()->isRoot() && current->getParent()->isLeaf())
  return 0;

 return current->getParent();

}

BeachLineNode * BeachLineNode::getRightMost() const {
 const BeachLineNode * current;
 current = this;

 if (isRoot() && isNull())
  return 0;

 if (isRoot() && isLeaf())
  current = current->pleftSon;

 while (!current->isLeaf()) {
  current = current->getRight();
 }
 return const_cast<BeachLineNode*>(current);
}
BeachLineNode * BeachLineNode::getLeftMost() const {
 const BeachLineNode * current;
 current = this;

 if (isRoot() && isNull())
  return 0;

 if (isRoot() && isLeaf())
  current = current->pleftSon;

 while (!current->isLeaf()) {
  current = current->getLeft();
 }

 return const_cast<BeachLineNode*>(current);

}
BeachLineNode * BeachLineNode::getBrother() const {
 if (parent == 0)
  return 0;

 if (parent->getLeft() == this)
  return parent->getRight();
 else
  return parent->getLeft();
}
BeachLineNode * BeachLineNode::remove(Triangle * t) {
 LOGP
 LOG(this)
 BeachLineNode * leftbp;
 BeachLineNode * rightbp;
 BeachLineNode * brother;
 BeachLineNode * changedBp=0;

 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_REMOVE1);

 if (parent->isRoot()) {
  if (isLeftChild()) {
   parent->site1 = 0;
   parent->pleftSon = 0;
  } else {
   parent->site2 = 0;
   parent->prightSon = 0;
  }

  delete this;
  return parent;
 }

 parent->edge->addEnd(t);

 parent->edge = 0;

 brother = getBrother();
 brother->parent = parent->parent;

 if (parent->isLeftChild()) {
  parent->parent->pleftSon = brother;
 } else {
  parent->parent->prightSon = brother;

 }

 leftbp = brother->getLeftBreakPoint();
 rightbp = brother->getRightBreakPoint();
 if (leftbp) {
  if (brother->isLeaf()) {

   if (leftbp->site2 != brother->site1) {
    leftbp->site2 = brother->site1;

    leftbp->edge->addEnd(t);
    leftbp->edge = 0;

    leftbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = leftbp->site2->getX();
    __float128 y1 = leftbp->site2->getY();
    __float128 x2 = leftbp->site1->getX();
    __float128 y2 = leftbp->site1->getY();

    leftbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(leftbp)->addEnd(t);
    changedBp = leftbp;
   }
  } else {

   if (leftbp->site2 != brother->getLeftMost()->site1) {
    leftbp->site2 = brother->getLeftMost()->site1;

    leftbp->edge->addEnd(t);
    leftbp->edge = 0;
    leftbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = leftbp->site2->getX();
    __float128 y1 = leftbp->site2->getY();
    __float128 x2 = leftbp->site1->getX();
    __float128 y2 = leftbp->site1->getY();

    leftbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(leftbp)->addEnd(t);
    changedBp = leftbp;
   }

  }
 }

 if (rightbp) {
  if (brother->isLeaf()) {
   if (rightbp->site1 != brother->site1) {
    rightbp->site1 = brother->site1;
    rightbp->edge->addEnd(t);
    rightbp->edge = 0;

    rightbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = rightbp->site2->getX();
    __float128 y1 = rightbp->site2->getY();
    __float128 x2 = rightbp->site1->getX();
    __float128 y2 = rightbp->site1->getY();

    rightbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(rightbp)->addEnd(t);
    changedBp = rightbp;
   }
  }

  else {
   if (rightbp->site1 != brother->getRightMost()->site1) {
    rightbp->site1 = brother->getRightMost()->site1;
    rightbp->edge->addEnd(t);
    rightbp->edge = 0;

    rightbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = rightbp->site2->getX();
    __float128 y1 = rightbp->site2->getY();
    __float128 x2 = rightbp->site1->getX();
    __float128 y2 = rightbp->site1->getY();

    rightbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(rightbp)->addEnd(t);
    changedBp = rightbp;
   }

  }
 }

 parent->pleftSon = 0;
 parent->prightSon = 0;
//balancing only if a black node was deleted otherwise not necessary
 if (!parent->red)
  brother->balanceTreeDeletion();

 delete parent;
 delete this;
 return changedBp;

}
BeachLineNode * BeachLineNode::removeMultiEvent(Triangle** t1,
  Triangle** t2) {
 LOGP
 LOG(this)
 BeachLineNode * leftbp;
 BeachLineNode * rightbp;
 BeachLineNode * brother;
 BeachLineNode * changedBp=0;

 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_REMOVE1);

 if (parent->isRoot()) {
  if (isLeftChild()) {
   parent->site1 = 0;
   parent->pleftSon = 0;
  } else {
   parent->site2 = 0;
   parent->prightSon = 0;
  }

  delete this;
  return parent;
 }

 *t1 = parent->edge->getOnlyEnd();
 parent->edge->addEnd(VORONOI_OPEN_END);
 parent->edge = 0;

 brother = getBrother();
 brother->parent = parent->parent;

 if (parent->isLeftChild()) {
  parent->parent->pleftSon = brother;
 } else {
  parent->parent->prightSon = brother;

 }

 leftbp = brother->getLeftBreakPoint();
 rightbp = brother->getRightBreakPoint();
//find "new" breakpoint -> node is kept and changed
 if (leftbp) {
  if (brother->isLeaf()) {

   if (leftbp->site2 != brother->site1) {
    leftbp->site2 = brother->site1;

    *t2 = leftbp->edge->getOnlyEnd();
    leftbp->edge->addEnd(VORONOI_OPEN_END);
    leftbp->edge = 0;

    leftbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = leftbp->site2->getX();
    __float128 y1 = leftbp->site2->getY();
    __float128 x2 = leftbp->site1->getX();
    __float128 y2 = leftbp->site1->getY();

    leftbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(leftbp)->addEnd(VORONOI_OPEN_END);
    changedBp = leftbp;
   }
  } else {

   if (leftbp->site2 != brother->getLeftMost()->site1) {
    leftbp->site2 = brother->getLeftMost()->site1;

    *t2 = leftbp->edge->getOnlyEnd();
    leftbp->edge->addEnd(VORONOI_OPEN_END);
    leftbp->edge = 0;
    leftbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = leftbp->site2->getX();
    __float128 y1 = leftbp->site2->getY();
    __float128 x2 = leftbp->site1->getX();
    __float128 y2 = leftbp->site1->getY();

    leftbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(leftbp)->addEnd(VORONOI_OPEN_END);
    changedBp = leftbp;
   }

  }
 }

 if (rightbp) {
  if (brother->isLeaf()) {
   if (rightbp->site1 != brother->site1) {
    rightbp->site1 = brother->site1;

    *t2 = rightbp->edge->getOnlyEnd();

    rightbp->edge->addEnd(VORONOI_OPEN_END);
    rightbp->edge = 0;

    rightbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = rightbp->site2->getX();
    __float128 y1 = rightbp->site2->getY();
    __float128 x2 = rightbp->site1->getX();
    __float128 y2 = rightbp->site1->getY();

    rightbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(rightbp)->addEnd(VORONOI_OPEN_END);
    changedBp = rightbp;
   }
  }

  else {
   if (rightbp->site1 != brother->getRightMost()->site1) {
    rightbp->site1 = brother->getRightMost()->site1;
    *t2 = rightbp->edge->getOnlyEnd();
    rightbp->edge->addEnd(VORONOI_OPEN_END);
    rightbp->edge = 0;

    rightbp->resetBreakX();

//this makes circle calculation faster
    __float128 x1 = rightbp->site2->getX();
    __float128 y1 = rightbp->site2->getY();
    __float128 x2 = rightbp->site1->getX();
    __float128 y2 = rightbp->site1->getY();

    rightbp->precalc_circle = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

    VoronoiEdge::getInstance(rightbp)->addEnd(VORONOI_OPEN_END);
    changedBp = rightbp;
   }

  }
 }

 parent->pleftSon = 0;
 parent->prightSon = 0;
//balancing only if a black node was deleted otherwise not necessary
 if (!parent->red)
  brother->balanceTreeDeletion();

 delete parent;
 delete this;
 return changedBp;

}

BeachLineNode* BeachLineNode::find(const VERTEX_COORDINATE x,
  const VERTEX_COORDINATE ySweepLine) const {
 bool goLeft;

 if (isNull())
  return const_cast<BeachLineNode*>(this);
 if (isRoot() && site1 != 0 && site2 == 0)
  return pleftSon->find(x, ySweepLine);
 if (isRoot() && site2 != 0 && site1 == 0)
  throw std::runtime_error("should not occur");

 if (isLeaf())
  return const_cast<BeachLineNode*>(this);

//try to use value of the last calculation
 if (break_x != BREAK_X_UNSET) {
  BREAK_DIRECTION dir = getBreakXDirection();
  if (dir == BREAK_X_LEFT && x < break_x)
   return prightSon->find(x, ySweepLine);
  if (dir == BREAK_X_RIGHT && x > break_x)
   return pleftSon->find(x, ySweepLine);
  if (dir == BREAK_X_VERTICAL && x < break_x)
   return prightSon->find(x, ySweepLine);
  if (dir == BREAK_X_VERTICAL && x > break_x)
   return pleftSon->find(x, ySweepLine);
 }

//no decision possible based on direction so calculate anew
 break_x = this->getParabolaIntersection(ySweepLine);

 goLeft = ((x)) > break_x;

 if (goLeft) {
  return pleftSon->find(x, ySweepLine);
 } else {
  return prightSon->find(x, ySweepLine);
 }
}
bool BeachLineNode::isNull() const {
 return (site1 == 0 ? true : false);
}

bool BeachLineNode::isLeftChild() const {
 if (parent != 0)
  return (parent->pleftSon == this);
 return false;
}

bool BeachLineNode::isLeaf() const {

 return (site1 != 0 && site2 == 0 ? true : false);
}
bool BeachLineNode::isRoot() const {
 return (parent == 0);
}
/*
 Method determineBreakXDirection returns the x direction of
 the movement of the break point defined by parameters
 ~vsite1~ and ~vsite2~. ATTENTION vsite1 has to be site1
 and vsite2 has to be site2 !

*/
BREAK_DIRECTION BeachLineNode::determineBreakXDirection(
  const Vertex* vsite1, const Vertex* vsite2) const {
//direction seen from the beach
 if (vsite1->getY() < vsite2->getY()) //Parabola site1 on top
   {
  return BREAK_X_RIGHT;
 } else if (vsite1->getY() > vsite2->getY()) //Parabola site2 on top
   {
  return BREAK_X_LEFT;
 } else {
  return BREAK_X_VERTICAL;
 }

}
BREAK_DIRECTION BeachLineNode::determineBreakYDirection(
  const Vertex* vsite1, const Vertex* vsite2) const {
//direction seen from the beach
 if (vsite1->getX() < vsite2->getX()) {
  return BREAK_Y_UP;
 } else if (vsite1->getX() > vsite2->getX()) {
  return BREAK_Y_DOWN;
 } else {
  return BREAK_Y_HORIZONTAL;
 }

}
BeachLineNode* BeachLineNode::getLeft() const {
 return pleftSon;
}

BeachLineNode* BeachLineNode::getRight() const {
 return prightSon;
}

void BeachLineNode::setLeft(BeachLineNode* nl) {
 pleftSon = nl;
}

void BeachLineNode::setRight(BeachLineNode* nr) {
 prightSon = nr;
}
void BeachLineNode::print_simple(std::ostream& os) const {

os  << (isLeaf() ? "Leaf" : "") << "* BeachLineNode - "

   << (isLeaf() ? "Leaf" : "InnerNode") << "\t" << this << " color red: "
   << red << " VoronoiEdge: " << edge << "\n";
 if (validIterator)
  os << "Connected Event: " << (*event).second << "\n";

 os << " site1: ";
 if (site1 != 0)
  site1->print(os);

 os << "\t site2: ";
 if (site2 != 0)
  site2->print(os);
 if (parent != 0) {
  os << "\n" << " parent site: ";
  if (parent->site1 != 0)
   parent->site1->print();
  if (parent->site2 != 0)
   parent->site2->print();
 }
}
void BeachLineNode::print(std::ostream& os, int tab) const {
 std::string str;
 str.append(tab, '\t');
os  << (isLeaf() ? "Leaf" : "") << str << "* BeachLineNode - "

   << (isLeaf() ? "Leaf" : "InnerNode") << "\t" << this << " color red: "
   << red << " VoronoiEdge: " << edge << "\n";
 if (validIterator)
  os << "Connected Event: " << (*event).second << "\n";

 os << str << " site1: ";
 if (site1 != 0)
  site1->print(os);

 os << "\t site2: ";
 if (site2 != 0)
  site2->print(os);

 if (parent != 0) {
  os << "\n" << str << " parent site: ";
  if (parent->site1 != 0)
   parent->site1->print();
  if (parent->site2 != 0)
   parent->site2->print();
 }
 os << "\n";
 tab++;
 if (pleftSon != 0)
  pleftSon->print(os, tab);

 if (prightSon != 0)
  prightSon->print(os, tab);
}
BeachLineNode* BeachLineNode::getLeftNeighbor() const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;

 current = getLeftBreakPoint();

 if (current == 0)
  return 0;

 current = current->getLeft();
 if (current == 0)
  return 0;
 while (!current->isLeaf())
  current = current->getRight();

 return current;

}
BeachLineNode* BeachLineNode::getLeftNeighbor(BeachLineNode** bp) const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;

 current = getLeftBreakPoint();
 (*bp) = current;

 if (current == 0)
  return 0;

 current = current->getLeft();
 if (current == 0)
  return 0;
 while (!current->isLeaf())
  current = current->getRight();

 return current;

}
BeachLineNode* BeachLineNode::getLeftNeighbor(POINT_COORDINATE& breakpoint,
  const VERTEX_COORDINATE sweepliney) const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;

 current = getLeftBreakPoint();

 if (current == 0)
  return 0;

 breakpoint = current->getParabolaIntersection_sec(sweepliney);

 current = current->getLeft();
 if (current == 0)
  return 0;

 while (!current->isLeaf())
  current = current->getRight();

 return current;

}
void BeachLineNode::check() const {
 if (isRoot() && isLeaf()) {
  pleftSon->check();
 }
 if (!isLeaf() && !isNull()) {
  if (pleftSon->getRightMost()->site1 != site1
    || prightSon->getLeftMost()->site1 != site2) {
   LOG(this)
   throw std::runtime_error("Wrong");

  }

  pleftSon->check();
  prightSon->check();
 }

}
void BeachLine::checkTree() const {
 root.check();
}
BeachLineNode * BeachLineNode::getParent() const {
 return parent;
}

BeachLineNode * BeachLineNode::getGrandParent() const {
 if (parent)
  return parent->parent;

 return 0;
}
BeachLineNode * BeachLineNode::getUncle() const {
 BeachLineNode * p;
 BeachLineNode * gp;

 p = getParent();
 if (!p)
  return 0;

 gp = getGrandParent();
 if (!gp)
  return 0;

 if (p->isLeftChild()) {
  return gp->getRight();
 } else
  return gp->getLeft();
}

BeachLineNode* BeachLineNode::getRightNeighbor() const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;

 current = getRightBreakPoint();

 if (current == 0)
  return 0;
 current = current->getRight();
 if (current == 0)
  return 0;

 while (!current->isLeaf())
  current = current->getLeft();

 return current;
}
BeachLineNode* BeachLineNode::getRightNeighbor(BeachLineNode** bp) const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;

 current = getRightBreakPoint();

 (*bp) = current;

 if (current == 0)
  return 0;
 current = current->getRight();
 if (current == 0)
  return 0;

 while (!current->isLeaf())
  current = current->getLeft();

 return current;
}
BeachLineNode* BeachLineNode::getRightNeighbor(
  POINT_COORDINATE& breakpoint, const VERTEX_COORDINATE sweepliney) const {
 if (!isLeaf())
  throw std::runtime_error(E_BEACHLINENODE_GETNEIGHBOR);

 BeachLineNode * current;
 current = getRightBreakPoint();

 if (current == 0)
  return 0;

 breakpoint = current->getParabolaIntersection_sec(sweepliney);

 current = current->getRight();
 if (current == 0)
  return 0;

 while (!current->isLeaf())
  current = current->getLeft();

 return current;
}

void BeachLineNode::setParent(BeachLineNode* p) {
 parent = p;
}
POINT_COORDINATE BeachLineNode::getParabolaIntersection_sec(
  const VERTEX_COORDINATE ySweepLine) const {
 PreciseDouble result, c01, c02, c1, c2, a, b, c, sol, sol1, sol2;
 if ((site1->getY() - ySweepLine) <= 0.000001) //TODO make accurate
  return site1->getX();
 if ((site2->getY() - ySweepLine) <= 0.000001)
  return site2->getX();

 SecureOperator::startSecureCalc();

 c1 = (PreciseDouble) ((1))
   / ((PreciseDouble) (((site1->getY() - ySweepLine))) * 2);
 c2 = (PreciseDouble) ((1))
   / ((PreciseDouble) (((site2->getY() - ySweepLine))) * 2);
 c01 = c1
   * ((PreciseDouble) ((site1->getX())) * (PreciseDouble) ((site1->getX()))
     - (PreciseDouble) ((ySweepLine)) * (PreciseDouble) ((ySweepLine))
     + (PreciseDouble) ((site1->getY()))
       * (PreciseDouble) ((site1->getY())));
 c02 = c2
   * ((PreciseDouble) ((site2->getX())) * (PreciseDouble) ((site2->getX()))
     - (PreciseDouble) ((ySweepLine)) * (PreciseDouble) ((ySweepLine))
     + (PreciseDouble) ((site2->getY()))
       * (PreciseDouble) ((site2->getY())));
 a = c1 - c2;
 b = (c2 * (PreciseDouble) ((site2->getX()))
   - c1 * (PreciseDouble) ((site1->getX()))) * 2;
 c = c01 - c02;

 if (a == 0) {
  result = -c / b;

 } else {
  PreciseArithmetic::error_sqrt(b * b - a * 4 * c, sol);

  sol1 = (-b + sol) / (a * 2);
  sol2 = (-b - sol) / (a * 2);
//TODO SC same Y?
//site2 on top of site1 (from sweep line)
  if (site1->getY() > site2->getY())
   result = (sol1 > sol2 ? sol1 : sol2); //bigger solution
  else
//site1 is on top
   result = (sol1 > sol2 ? sol2 : sol1); //smaller solution

 }

 return result;
}
VERTEX_COORDINATE BeachLineNode::getParabolaIntersection(
  const VERTEX_COORDINATE ySweepLine) const {
 VERTEX_COORDINATE result, c01, c02, c1, c2, a, b, c, sol, sol1, sol2;

 if (((site1->getY() - ySweepLine) <= 0.000001)
   && site1->getY() == site2->getY())
  return (site1->getX() + site2->getX()) / 2;
 if ((site1->getY() - ySweepLine) <= 0.000001)
  return site1->getX();
 if ((site2->getY() - ySweepLine) <= 0.000001)
  return site2->getX();

 SecureOperator::startSecureCalc();

 c1 = 1 / (((site1->getY() - ySweepLine)) * 2);
 c2 = 1 / (((site2->getY() - ySweepLine)) * 2);
 c01 = c1
   * (site1->getX() * ((site1->getX())) - ((ySweepLine)) * ((ySweepLine))
     + ((site1->getY())) * ((site1->getY())));
 c02 = c2
   * (((site2->getX())) * ((site2->getX()))
     - ((ySweepLine)) * ((ySweepLine))
     + ((site2->getY())) * ((site2->getY())));
 a = c1 - c2;
 b = (c2 * ((site2->getX())) - c1 * ((site1->getX()))) * 2;
 c = c01 - c02;

 if (a == 0) {
  result = -c / b;

 } else {
  sol = std::sqrt(b * b - a * 4 * c);

  sol1 = (-b + sol) / (a * 2);
  sol2 = (-b - sol) / (a * 2);
//TODO SC same Y?
//site2 on top of site1 (from sweep line)
  if (site1->getY() > site2->getY())
   result = (sol1 > sol2 ? sol1 : sol2); //bigger solution
  else
//site1 is on top
   result = (sol1 > sol2 ? sol2 : sol1); //smaller solution

 }

 break_x = result;

 return result;
}
void BeachLineNode::getParabolaIntersection_mp(
  const VERTEX_COORDINATE ySweepLine, mpq_class & result) const {
 double sol;
 mpq_class mp_ySweepLine(ySweepLine);
 mpq_class mp_site1X(site1->getX());
 mpq_class mp_site1Y(site1->getY());
 mpq_class mp_site2X(site2->getX());
 mpq_class mp_site2Y(site2->getY());

 if ((site1->getY() - ySweepLine) <= 0.00001) {
  result = site1->getX();
  return;
 }
 if ((site2->getY() - ySweepLine) <= 0.00001) {
  result = site2->getX();
  return;
 }

 mpq_class mp_c1((1 / ((mp_site1Y - mp_ySweepLine) * 2)));
 mpq_class mp_c2((1 / ((mp_site2Y - mp_ySweepLine) * 2)));

 mpq_class mp_c01(
   mp_c1
     * (mp_site1X * mp_site1X - mp_ySweepLine * mp_ySweepLine
       + mp_site1Y * mp_site1Y));
 mpq_class mp_c02(
   mp_c2
     * (mp_site2X * mp_site2X - mp_ySweepLine * mp_ySweepLine
       + mp_site2Y * mp_site2Y));

 mpq_class mp_a(mp_c1 - mp_c2);

 mpq_class mp_b((mp_c2 * mp_site2X - mp_c1 * mp_site1X) * 2);
 mpq_class mp_c(mp_c01 - mp_c02);

 if (mp_a == 0) {
  result = ((-mp_c) / mp_b);
 } else {
  mpq_class mp_sol(mp_b * mp_b - mp_a * 4 * mp_c);
  sol = std::sqrt(mp_sol.get_d());

  mpq_class sol1(((-mp_b) + sol) / (mp_a * 2));
  mpq_class sol2(((-mp_b) - sol) / (mp_a * 2));
//TODO SC same Y?
//site2 on top of site1 (from sweep line)
  if (site1->getY() > site2->getY())
   result = ((sol1 > sol2 ? sol1 : sol2)); //bigger solution
  else
//site1 is on top
   result = ((sol1 > sol2 ? sol2 : sol1)); //smaller solution

 }

}

CircleEvent::CircleEvent(Point_p im, BeachLineNode *iarc) {
 sweep = im;
 arc = iarc;
LOG("Construct CircleEvent")
LOG(this)
}
CircleEvent::~CircleEvent() {

}
void CircleEvent::removeArcMultiEvent(Triangle ** t1, Triangle ** t2) {
 if (arc != 0) {
  arc->removeMultiEvent(t1, t2);
  arc = 0;
 }
}

bool EventQueue::isCircleAlreadyInQueue(const Point_p& sweeppoint) const {

 std::multimap<Point_p, CircleEvent *>::const_iterator it =
   circleEventQueue.find(sweeppoint);

 return it != circleEventQueue.end();

}
const std::multimap<Point_p, CircleEvent*>::iterator EventQueue::addCircleEvent(
  CircleEvent& e, bool & alreadyExists) {
 std::pair<Point_p, CircleEvent*> ins(e.getSweepPoint(), &e);
 LOGP
 alreadyExists = false;
 return circleEventQueue.insert(ins);
}

void EventQueue::removeEvent(BeachLineNode* arc) {
 LOGP
 if (arc && arc->isValidEvent()) //if exists remove event
   {
  delete arc->getEvent()->second;
  circleEventQueue.erase(arc->getEvent());
  arc->resetEvent();
 }
LOGP}

EventQueue::~EventQueue() {
 if (verticesPriorSection)
  delete verticesPriorSection;

}
void EventQueue::createTriangle(const Vertex * v1, const Vertex * v2,
  const Vertex * v3, Triangle** newtriangle) {
 try {
  this->tt->addTriangle_p(*v1, *v2, *v3, newtriangle);
 } catch (std::exception & e) {
  MSG_TO_USER(e.what());
  *newtriangle = 0;
 }
}
/*
 The method doFortuneAlgorithm really executes the Fortunes Algorithm
 on the Vertices provided by ~currentSection~. The bool ~finalSection~
 tells whether it is the last section. The last section will free all
 memory occupied by the Algorithm itself.

*/
void EventQueue::doFortuneAlgorithm(VertexContainerSet * currentSection,
  bool finalSection) {
 VertexContainerSet * tmpRescuedVertices = 0;
 std::multimap<Point_p, CircleEvent*>::iterator itc;
 std::multimap<Point_p, CircleEvent*>::iterator it_end_same_circle;
 std::multimap<Point_p, CircleEvent*>::iterator it_end_same_circle_tmp;
 VERTEX_COORDINATE begin_section_y, end_section_y, end_priorsection_y=0;
 const Vertex * v;

 LOG_EXP(eventId = 0)

//Get reverse_iterator as SiteEventQueue
 siteEventQueue = currentSection->getIteratorRbegin();

 if (currentSection && currentSection->getNoVertices() > 0) {
  begin_section_y = currentSection->getVertexByYIndex(
    currentSection->getNoVertices() - 1)->getY();
  end_section_y = currentSection->getVertexByYIndex(0)->getY();
 } else {
  begin_section_y = -std::numeric_limits<VERTEX_COORDINATE>::max();
  end_section_y = -std::numeric_limits<VERTEX_COORDINATE>::max();
 }

 if (!verticesPriorSection) {
  end_priorsection_y = std::numeric_limits<VERTEX_COORDINATE>::max();
  verticesPriorSection = new VertexContainerSet(VERTEX_CONTAINER_BIG_SIZE);
 } else {
  v = verticesPriorSection->getVertexByYIndex(0);
  if (v)
   end_priorsection_y = v->getY();

 }
//sections from top to bottom and no overlapping !
 if (begin_section_y >= end_priorsection_y)
  throw std::runtime_error(E_EVENTQUEUE_DOFORTUNEALGORITHM);

 LOG_EXP(this->print())

//Handle all events
 while (true) {
  LOG_EXP(eventId++)
  itc = circleEventQueue.end();
  if (circleEventQueue.size() > 0)
   --itc;

  if (itc == circleEventQueue.end()
    && siteEventQueue == currentSection->getIteratorRend())
   break; //no more Events

//decide which Queue
//the Event above comes first, but in case of equality SiteEvents first
//do only events above the end of section in the final section do all
  if ((itc != circleEventQueue.end()
    && (((*itc).first.y >= end_section_y) || finalSection))
    && (siteEventQueue == currentSection->getIteratorRend()
      || ((*itc).first.y > (*siteEventQueue).getY()))) {
//so CircleEvent is next
   LOG_EXP(this->print())

//treats special case more than 3 points on a circle
//-> check all circles being at the same place
   it_end_same_circle = circleEventQueue.end();
   --it_end_same_circle;

   if (it_end_same_circle != circleEventQueue.begin()) {
    --it_end_same_circle;

    while ((*itc).second->isEqual((*it_end_same_circle).second)
      && it_end_same_circle != circleEventQueue.begin()) {
     --it_end_same_circle;
    }

    if (!(*itc).second->isEqual((*it_end_same_circle).second)) {
     ++it_end_same_circle;
    }

   }
//here it_end_same_circle points to
//the last equal circle or both point to a SiteEvent

   if (it_end_same_circle == itc) {
//regular case 3 points on a circle
//or SiteEvent (since SiteEvents are always different
//in order there is always only one)

    (*itc).second->handleEvent(*this);
    delete (*itc).second;
    circleEventQueue.erase(itc);
   } else {
//special case more than 3 points on a circle
//-> several circle events at the same place
    static_cast<CircleEvent*>((*itc).second)->handleMultiEvent(*this,
      it_end_same_circle, itc);
   }

  } else if (siteEventQueue != currentSection->getIteratorRend()
    && (((*siteEventQueue).getY() >= end_section_y) || finalSection)) {
   SiteEvent::handleEvent(siteEventQueue, *this);
   siteEventQueue++;
  } else {

   break; //stop at the end of section
  }
 }

 if (verticesPriorSection && !finalSection) {
//rescue the vertices needed from the last section
  tmpRescuedVertices = new VertexContainerSet(VERTEX_CONTAINER_BIG_SIZE);
  bl.rescueVertices(tmpRescuedVertices);
  delete verticesPriorSection;
  verticesPriorSection = tmpRescuedVertices;
  delete currentSection;
 }

 if (finalSection) {
  if (verticesPriorSection)
   delete verticesPriorSection;
  verticesPriorSection = 0;
  delete currentSection;
 }
LOG("Finished Fortune Algorithm")

}

bool CircleEvent::isEqual(CircleEvent * e) const {

 if (absolute(e->sweep.x - sweep.x) < 0.0000001
   && absolute(e->sweep.y - sweep.y) < 0.0000001)
  return true;

 return false;

}
VERTEX_COORDINATE CircleEvent::getY() const {
 return sweep.y;
}

EventQueue::EventQueue(Tin * itt) {
 tt = itt;
 verticesPriorSection = 0;
}
void CircleEvent::print(std::ostream& os) const {

 os << "CircleEvent --- " << this << " sweep: ";
 sweep.print(os);
 os << "\n";
 if (arc != 0) {
  arc->getLeftNeighbor()->print();
  arc->print(os);
  arc->getRightNeighbor()->print();

 }

}
bool CircleEvent::isIdentical(CircleEvent * e) const {

 const Vertex * v1=0, *v2=0, *v3=0;
 const Vertex * ev1=0, *ev2=0, *ev3=0;
 BeachLineNode* ln, *rn;

 ev1 = e->arc->getSite();

 ln = e->arc->getLeftNeighbor();
 rn = e->arc->getRightNeighbor();
 if (ln)
  ev2 = ln->getSite();
 if (rn)
  ev3 = rn->getSite();

 v1 = arc->getSite();

 ln = arc->getLeftNeighbor();
 rn = arc->getRightNeighbor();
 if (ln)
  v2 = ln->getSite();
 if (rn)
  v3 = rn->getSite();

 if (*ev1 == *v1 && *ev2 == *v2 && *ev3 == *v3)
  return true;
 if (*ev1 == *v1 && *ev2 == *v3 && *ev3 == *v2)
  return true;
 if (*ev1 == *v2 && *ev2 == *v1 && *ev3 == *v3)
  return true;
 if (*ev1 == *v2 && *ev2 == *v3 && *ev3 == *v1)
  return true;
 if (*ev1 == *v3 && *ev2 == *v2 && *ev3 == *v1)
  return true;
 if (*ev1 == *v3 && *ev2 == *v1 && *ev3 == *v2)
  return true;

 return false;

}
bool CircleEvent::isIdentical(BeachLineNode * n) const {

 const Vertex * v1, *v2, *v3;
 const Vertex * ev1, *ev2, *ev3;
 BeachLineNode* ln, *rn;

 if (!n)
  return false;

 ev1 = n->getSite();

 ln = n->getLeftNeighbor();
 rn = n->getRightNeighbor();
 if (ln)
  ev2 = ln->getSite();
 else
  return false;

 if (rn)
  ev3 = rn->getSite();
 else
  return false;
 v1 = arc->getSite();

 ln = arc->getLeftNeighbor();
 rn = arc->getRightNeighbor();
 if (ln)
  v2 = ln->getSite();
 else
  return false;
 if (rn)
  v3 = rn->getSite();
 else
  return false;

 if (*ev1 == *v1 && *ev2 == *v2 && *ev3 == *v3)
  return true;
 if (*ev1 == *v1 && *ev2 == *v3 && *ev3 == *v2)
  return true;
 if (*ev1 == *v2 && *ev2 == *v1 && *ev3 == *v3)
  return true;
 if (*ev1 == *v2 && *ev2 == *v3 && *ev3 == *v1)
  return true;
 if (*ev1 == *v3 && *ev2 == *v2 && *ev3 == *v1)
  return true;
 if (*ev1 == *v3 && *ev2 == *v1 && *ev3 == *v2)
  return true;

 return false;

}
Point_p CircleEvent::calculateSweepPoint_mp(const Vertex * p1,
  const Vertex * p2, const Vertex * p3) {
 mpq_class v12, v23, v13, xm, ym, r, x1, y1, x2, y2, x3, y3, sol;
 Point_p sweeppoint;

 x1 = p1->getX();
 y1 = p1->getY();
 x2 = p2->getX();
 y2 = p2->getY();
 x3 = p3->getX();
 y3 = p3->getY();

 if (p1 == p2 || p1 == p3 || p2 == p3)
  throw std::invalid_argument(E_CIRCLEEVENT_CALCULATESWEEPPOINT);

 v12 = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

 if (!(y2 == y3)) {

  v23 = (x2 * x2 + y2 * y2 - x3 * x3 - y3 * y3) / 2;

  xm = (v12 - (v23 * (y1 - y2) / (y2 - y3)));
  sol = ((x1 - x2) + ((x3 - x2) / (y2 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v23 - xm * (x2 - x3)) / (y2 - y3);
 } else if (!(y1 == y3)) {

  v13 = (y1 * y1 + x1 * x1 - y3 * y3 - x3 * x3) / 2;

  xm = (v12 - (v13 * (y1 - y2) / (y1 - y3)));
  sol = ((x1 - x2) + ((x3 - x1) / (y1 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v13 - xm * (x1 - x3)) / (y1 - y3);
 } else // all points on a horizontal line
 {
  sweeppoint.x = ERROR_VALUE;
  return sweeppoint;
 }

 sol = (x3 - xm) * (x3 - xm) + (y3 - ym) * (y3 - ym);
 r = std::sqrt(sol.get_d());

 sweeppoint.x = xm.get_d();
 sol = ym - r;
 sweeppoint.y = sol.get_d();

 return sweeppoint;
}
Point_p CircleEvent::calculateSweepPoint(Point_p p1, Point_p p2,
  Point_p p3) {
 double v12, v23, v13, xm, ym, r, x1, y1, x2, y2, x3, y3, sol;
 Point_p sweeppoint;

 if (p1 < p2) {
  sweeppoint = p1;
  p1 = p2;
  p2 = sweeppoint;
 }

 if (p2 < p3) {
  sweeppoint = p2;
  p2 = p3;
  p3 = sweeppoint;

  if (p1 < p2) {
   sweeppoint = p1;
   p1 = p2;
   p2 = sweeppoint;
  }
 }

 x1 = p1.x;
 y1 = p1.y;
 x2 = p2.x;
 y2 = p2.y;
 x3 = p3.x;
 y3 = p3.y;

 if (p1 == p2 || p1 == p3 || p2 == p3)
  throw std::invalid_argument(E_CIRCLEEVENT_CALCULATESWEEPPOINT);

 v12 = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

 if (!(y2 == y3)) {

  v23 = (x2 * x2 + y2 * y2 - x3 * x3 - y3 * y3) / 2;

  xm = (v12 - (v23 * (y1 - y2) / (y2 - y3)));
  sol = ((x1 - x2) + ((x3 - x2) / (y2 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v23 - xm * (x2 - x3)) / (y2 - y3);
 } else if (!(y1 == y3)) {

  v13 = (y1 * y1 + x1 * x1 - y3 * y3 - x3 * x3) / 2;

  xm = (v12 - (v13 * (y1 - y2) / (y1 - y3)));
  sol = ((x1 - x2) + ((x3 - x1) / (y1 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v13 - xm * (x1 - x3)) / (y1 - y3);
 } else // all points on a horizontal line
 {
  sweeppoint.x = ERROR_VALUE;
  return sweeppoint;
 }

 sol = (x3 - xm) * (x3 - xm) + (y3 - ym) * (y3 - ym);
 r = std::sqrt(sol);

 sweeppoint.x = xm;
 sol = ym - r;
 sweeppoint.y = sol;

 return sweeppoint;
}
inline Point_p CircleEvent::calculateSweepPoint_quad(const Vertex* s1,
  const Vertex* s2, const Vertex* s3, const __float128 & precalc12,
  const __float128 & precalc23) {

 __float128 v12, v23, v13, xm, ym, r, x1, y1, x2, y2, x3, y3, sol;

 Point_p sweeppoint;

 x1 = s1->getX();
 y1 = s1->getY();
 x2 = s2->getX();
 y2 = s2->getY();
 x3 = s3->getX();
 y3 = s3->getY();

 if (precalc12 == BeachLineNode::PRECALC_UNSET)
  v12 = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;
 else {
  v12 = precalc12;
 }

 if (!(y2 == y3)) {

  if (precalc23 == BeachLineNode::PRECALC_UNSET)
   v23 = (x2 * x2 + y2 * y2 - x3 * x3 - y3 * y3) / 2;
  else
   v23 = precalc23;

  xm = (v12 - (v23 * (y1 - y2) / (y2 - y3)));
  sol = ((x1 - x2) + ((x3 - x2) / (y2 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v23 - xm * (x2 - x3)) / (y2 - y3);
 } else if (!(y1 == y3)) {

  v13 = (y1 * y1 + x1 * x1 - y3 * y3 - x3 * x3) / 2;

  xm = (v12 - (v13 * (y1 - y2) / (y1 - y3)));
  sol = ((x1 - x2) + ((x3 - x1) / (y1 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   sweeppoint.x = ERROR_VALUE;
   return sweeppoint;
  }

  ym = (v13 - xm * (x1 - x3)) / (y1 - y3);
 } else // all points on a horizontal line
 {
  sweeppoint.x = ERROR_VALUE;
  return sweeppoint;
 }

 sol = (x3 - xm) * (x3 - xm) + (y3 - ym) * (y3 - ym);
 r = sqrtq(sol);

 sweeppoint.x = xm;
 sol = ym - r;
 sweeppoint.y = sol;

 return sweeppoint;
}

bool CircleEvent::isVertexInside(Point_p & middle, double radius,
  const Vertex & v) {
 Vector2D dmiddle = Point(middle.x, middle.y) - v;

 PreciseDouble lx = dmiddle.getDx() * dmiddle.getDx();
 PreciseDouble ly = dmiddle.getDy() * dmiddle.getDy();

 if ((PreciseDouble) radius * (PreciseDouble) radius > lx + ly + 1)
  return true;

 return false;

}
Point_p CircleEvent::calculateCircle_mp(const Vertex& v1, const Vertex& v2,
  const Vertex& v3, double& r) {
 mpq_class v12, v23, v13, xm, ym, x1, y1, x2, y2, x3, y3, sol;
 Point_p middle, p1, p3, p2;
 p1 = Point_p(v1.getX(), v1.getY());
 p2 = Point_p(v2.getX(), v2.getY());
 p3 = Point_p(v3.getX(), v3.getY());

 x1 = p1.x;
 y1 = p1.y;
 x2 = p2.x;
 y2 = p2.y;
 x3 = p3.x;
 y3 = p3.y;

 if (p1 == p2 || p1 == p3 || p2 == p3)
  throw std::invalid_argument(E_CIRCLEEVENT_CALCULATESWEEPPOINT);

 v12 = (x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2) / 2;

 if (!(y2 == y3)) {

  v23 = (x2 * x2 + y2 * y2 - x3 * x3 - y3 * y3) / 2;

  xm = (v12 - (v23 * (y1 - y2) / (y2 - y3)));
  sol = ((x1 - x2) + ((x3 - x2) / (y2 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   middle.x = ERROR_VALUE;
   return middle;
  }

  ym = (v23 - xm * (x2 - x3)) / (y2 - y3);
 } else if (!(y1 == y3)) {

  v13 = (y1 * y1 + x1 * x1 - y3 * y3 - x3 * x3) / 2;

  xm = (v12 - (v13 * (y1 - y2) / (y1 - y3)));
  sol = ((x1 - x2) + ((x3 - x1) / (y1 - y3)) * (y1 - y2));

  if (sol != 0)
   xm = xm / sol;
  else {
   middle.x = ERROR_VALUE;
   return middle;
  }

  ym = (v13 - xm * (x1 - x3)) / (y1 - y3);
 } else // all points on a horizontal line
 {
  middle.x = ERROR_VALUE;
  return middle;
 }

 sol = (x3 - xm) * (x3 - xm) + (y3 - ym) * (y3 - ym);
 r = std::sqrt(sol.get_d());

 middle.x = xm.get_d();
 middle.y = ym.get_d();

 return middle;
}
void CircleEvent::handleEvent(EventQueue & eq) {
 LOGP
 LOG(this)
 BeachLineNode * rneigh = 0;
 BeachLineNode * lneigh = 0;
 BeachLineNode * crneigh = 0;
 BeachLineNode * clneigh = 0;
 BeachLineNode * parabolaAbove = 0;
 BeachLineNode * cmid = 0;
 BeachLineNode * lbp = 0;
 BeachLineNode * mbp = 0;
 BeachLineNode * rbp = 0;
 Triangle* newtriangle = 0;

 eq.getBeachLine()->setSweepLine(sweep.y);

 parabolaAbove = arc;

 rneigh = parabolaAbove->getRightNeighbor();
 lneigh = parabolaAbove->getLeftNeighbor();

 eq.createTriangle(lneigh->getSite(), parabolaAbove->getSite(),
   rneigh->getSite(), &newtriangle);

//this condition treats the special case where
//a circle is founded by a site on the sweep line
//here the circle must not be removed
//because the arc lives on despite the circle event
 if ((sweep.y != arc->getSite()->getY())) { //regular case here

  arc->resetEvent();
  mbp = parabolaAbove->remove(newtriangle);
  arc = 0;

// remove neighbor events ,
//this event is deleted by the event loop (doFortuneAlgorithm)

  eq.removeEvent(rneigh);
  eq.removeEvent(lneigh);

//now check for new CircleEvents
  LOG("Check Event")
  cmid = lneigh;
  clneigh = lneigh->getLeftNeighbor(&lbp);
  crneigh = rneigh;

  eq.checkEventAndInsert(crneigh, cmid, clneigh, mbp, lbp);

  cmid = rneigh;
  clneigh = lneigh;
  crneigh = rneigh->getRightNeighbor(&rbp);

  eq.checkEventAndInsert(crneigh, cmid, clneigh, rbp, mbp);

 } else { //here special case SiteCircleEvent
//std::cout<<"SiteCircle !  \n";
//now that the triangle is known add it to the rescued edge
  arc->getVoronoiEdge()->addEnd(newtriangle);
//and get rid of the rescued edge
  arc->resetRescuedVoronoiEdge();

//also special treatment of the new edges created by former insertion
  arc->getLeftBreakPoint()->getVoronoiEdge()->addEnd(newtriangle);
  arc->getRightBreakPoint()->getVoronoiEdge()->addEnd(newtriangle);

  arc->resetEvent();
 }

}
void CircleEvent::handleMultiEvent(EventQueue & eq,
  std::multimap<Point_p, CircleEvent*>::iterator from,
  std::multimap<Point_p, CircleEvent*>::iterator to) {
 LOGP
 LOG(this)
 std::multimap<Point_p, CircleEvent*>::iterator it;
 std::multimap<Point_p, CircleEvent*>::iterator toplus;
 VertexContainerSet vc;
 std::deque<Triangle *> neighborMatch;
 Point pt;
 int sidenextvertex, sidevertextriangle;
 BeachLineNode * rmost = 0;
 BeachLineNode * lmost = 0;
 BeachLineNode * currentbp = 0;
 BeachLineNode * bplmostrmost = 0;
 BeachLineNode * nneigh = 0;
 BeachLineNode * sitecircle = 0;
 CircleEvent * cev;
 POINT_COORDINATE bp_left = -std::numeric_limits<double>::max();
 POINT_COORDINATE bp_right = std::numeric_limits<double>::max();
 Triangle* newtriangle, *t1, *t2;
 toplus = to;

 if (toplus != eq.getCircleQueue()->end())
  toplus++;

 eq.getBeachLine()->setSweepLine(sweep.y);

//remove events including the direct neighbors
//of the range and remove arcs in range

 it = from;

 while (true) {
//delete all events in range and determine
//leftmost and rightmost arc and remove arcs
  cev = (*it).second;

  LOG_EXP(cev->print())

  if (it == to) {
   lmost = cev->arc->getLeftNeighbor();
   rmost = cev->arc->getRightNeighbor();
  }

//remember site for triangulation
  vc.insertVertex_p(cev->arc->getSite());

  if ((cev->arc->getSite()->getY() == sweep.y)) {
   cev->arc->resetEvent();
   sitecircle = cev->arc;
  } else {
   cev->removeArcMultiEvent(&t1, &t2);
   neighborMatch.push_back(t1);
   neighborMatch.push_back(t2);
  }
  delete cev;

  if (it == to)
   break;
  ++it;
 }

 eq.getCircleQueue()->erase(from, toplus);

 if (sitecircle) {
  lmost = sitecircle->getLeftNeighbor();
  rmost = sitecircle->getRightNeighbor();
 }

 vc.insertVertex_p(lmost->getSite()); //remember site for triangulation
 vc.insertVertex_p(rmost->getSite()); //remember site for triangulation
//remove neighbor events of the range
 eq.removeEvent(lmost);
 eq.removeEvent(rmost);

/////////////triangulation///////////////////////////////////////////////
//begin with the first 3 vertices
 LOGP
 eq.createTriangle(vc.getVertexByYIndex(0), vc.getVertexByYIndex(1),
   vc.getVertexByYIndex(2), &newtriangle);
//TODO remove Bug -> references can get lost due to unloading.
//.. implement the reference counting in this section
//set the real end to the only outgoing new voronoi
//edge of the circle - now that it is known

 if (sitecircle) {
  VoronoiEdge * rescuedEdge = sitecircle->getVoronoiEdge();
  rescuedEdge->addEnd(VORONOI_OPEN_END);
  sitecircle->resetRescuedVoronoiEdge();

  BeachLineNode * lbpsce = sitecircle->getLeftBreakPoint();
  BeachLineNode * rbpsce = sitecircle->getRightBreakPoint();

  if (lbpsce && lbpsce->getVoronoiEdge())
   lbpsce->getVoronoiEdge()->setOnlyEnd(VORONOI_OPEN_END);
  if (rbpsce && rbpsce->getVoronoiEdge())
   rbpsce->getVoronoiEdge()->setOnlyEnd(VORONOI_OPEN_END);

 } else {
  bplmostrmost = lmost->getRightBreakPoint();
  bplmostrmost->getVoronoiEdge()->setOnlyEnd(newtriangle);
 }

 neighborMatch.push_back(newtriangle);

 for (int i = 3; i < vc.getNoVertices(); i++) {
//then find the common edge
  if (newtriangle) {
   Edge common = Edge(newtriangle->getMinYVertex(),
     newtriangle->getMaxYVertex());
   pt.x = newtriangle->getMaxYVertex()->getX();
   pt.y = newtriangle->getMaxYVertex()->getY();

   Line lcommonedge = Line(common.getVector2D(), pt);
   pt.x = vc.getVertexByYIndex(i)->getX();
   pt.y = vc.getVertexByYIndex(i)->getY();
   sidenextvertex = lcommonedge.getSide(pt);

   pt.x = newtriangle->getMiddleYVertex()->getX();
   pt.y = newtriangle->getMiddleYVertex()->getY();
   sidevertextriangle = lcommonedge.getSide(pt);

   if (sidevertextriangle != sidenextvertex) {

    eq.createTriangle(newtriangle->getMaxYVertex(),
      newtriangle->getMinYVertex(), vc.getVertexByYIndex(i), &newtriangle);
    neighborMatch.push_back(newtriangle);

   } else {
    eq.createTriangle(newtriangle->getMaxYVertex(),
      newtriangle->getMiddleYVertex(), vc.getVertexByYIndex(i),
      &newtriangle);
    neighborMatch.push_back(newtriangle);
   }
  }
 }

//Matching the neighbors, since in case of MultiEvent
//this is not done by the VoronoiEdges
 LOGP
 std::deque<Triangle *>::iterator itneigh;

 for (itneigh = neighborMatch.begin(); itneigh != neighborMatch.end();
   ++itneigh) {
  if ((*itneigh) && (*itneigh) != VORONOI_OPEN_END)
   (*itneigh)->matchNeighbors(neighborMatch.begin(), neighborMatch.end());
 }

 LOGP

/////////////////////////////////////////////////////////////////////////

//build the new events
 if (!sitecircle) //regular case
 {
  nneigh = lmost->getLeftNeighbor(&currentbp);
  eq.checkEventAndInsert(rmost, lmost, nneigh, bplmostrmost, currentbp);
  nneigh = rmost->getRightNeighbor(&currentbp);
  eq.checkEventAndInsert(nneigh, rmost, lmost, currentbp, bplmostrmost);
 } else //SC SiteCircleEvent during MultiEvent
 {
  lmost = sitecircle->getLeftNeighbor();
  rmost = sitecircle->getRightNeighbor();
//eq.checkEventAndInsert(lmost, sitecircle, rmost); cannot happen
  nneigh = rmost->getRightNeighbor(&currentbp);
  eq.checkEventAndInsert(nneigh, rmost, sitecircle, currentbp,
    rmost->getLeftBreakPoint());
  nneigh = lmost->getLeftNeighbor(&currentbp);
  eq.checkEventAndInsert(sitecircle, lmost, nneigh,
    lmost->getRightBreakPoint(), currentbp);

 }
}
bool EventQueue::checkSiteCircleEvent(BeachLineNode* clneigh,
  const Vertex * vmid, BeachLineNode* crneigh, Point_p & sweep) {
 LOGP
 sweep.x = ERROR_VALUE;

 if (clneigh != 0 && vmid != 0 && crneigh != 0) {
  LOG("Check SiteCircleEvent")

  if (clneigh->getSite() != vmid && crneigh->getSite() != vmid
    && clneigh->getSite() != crneigh->getSite()) {

   sweep = CircleEvent::calculateSweepPoint_quad(clneigh->getSite(), vmid,
     crneigh->getSite(), BeachLineNode::PRECALC_UNSET,
     BeachLineNode::PRECALC_UNSET);

   if (sweep.x == ERROR_VALUE) //special case 3 sites on a line
    return false;

   if (std::abs(sweep.y - vmid->getY()) <= 0.0000001) //circle top is vertex
     {
    return true;
   }

  }
 }

 return false;
}
void EventQueue::print(std::ostream& os) {

 os << "\n";
 os << "VoronoiEdgeInstances currently............\n";
 os << "Events in the Queue.......................\n";
 std::multimap<Point_p, CircleEvent*>::const_iterator itc =
   circleEventQueue.begin();

 os << "CircleEventQueue\n\n";
 while (itc != circleEventQueue.end()) {
  os << "\n";
  (*itc).second->print(os);
  itc++;
 }

 os << "Current BeachLineTree.....................\n";

 bl.print(os);

 os << "Event handling............................\n";
}
void EventQueue::checkEventAndInsert(BeachLineNode* crneigh,
  BeachLineNode * cmid, BeachLineNode* clneigh, BeachLineNode* rbp,
  BeachLineNode* lbp) {
 LOGP
 CircleEvent * cev;
 std::multimap<Point_p, CircleEvent*>::iterator it;
 bool alreadyInQueue = false;

 if (clneigh != 0 && cmid != 0 && crneigh != 0) {

  if (clneigh->getSite() != cmid->getSite()
    && crneigh->getSite() != cmid->getSite()
    && clneigh->getSite() != crneigh->getSite()) {

//try to find out the collapse the easy way
   int percol = cmid->isCollapsing_performance();
   if (percol == 0)
    return;

   Point_p sweeppoint = CircleEvent::calculateSweepPoint_quad(
     crneigh->getSite(), cmid->getSite(), clneigh->getSite(),
     rbp->getPrecalc(), lbp->getPrecalc());

   if (sweeppoint.x == ERROR_VALUE) //special case 3 sites on a line
    return;

//Special case boundary arc: can be false alarm,
//since arcs on the boundary are not necessarily destroyed
//here check whether arc is really zero length, only then it will be added

   if ((percol == -1) && !cmid->isCollapsing_mp(sweeppoint.y)) {
    return;
   }

   removeEvent(cmid);

   cev = new CircleEvent(sweeppoint, cmid);
   it = addCircleEvent(*cev, alreadyInQueue);
   cmid->setEvent(it);

  }
 }

}
void EventQueue::checkEventAndInsert(BeachLineNode* mid, Point_p& sweep) {
 LOGP
 CircleEvent * cev;
 std::multimap<Point_p, CircleEvent*>::iterator it;
 bool alreadyInQueue = false;

 if (sweep.x == ERROR_VALUE) //no potential CircleEvent
  return;

//try to find out the collapse the easy way
 int percol = mid->isCollapsing_performance();
 if (percol == 0)
  return;
//Special case boundary arc: can be false alarm,
//since arcs on the boundary are not necessarily destroyed
//here check whether arc is really zero length,
//only then it will be added

 if ((percol == -1) && !mid->isCollapsing_mp(sweep.y))
  return;

 removeEvent(mid);

 cev = new CircleEvent(sweep, mid);
 it = addCircleEvent(*cev, alreadyInQueue);
 mid->setEvent(it);

}
void EventQueue::insertSiteCircleEvent(BeachLineNode* arc) {
 LOGP
 CircleEvent * cev;
 Point_p sweeppoint;
 std::multimap<Point_p, CircleEvent*>::iterator it;
 bool alreadyInQueue = false;

 LOG("Insert SiteCircle Event")

 sweeppoint.x = arc->getSite()->getX();
 sweeppoint.y = arc->getSite()->getY();

 cev = new CircleEvent(sweeppoint, arc);
 it = addCircleEvent(*cev, alreadyInQueue);
 arc->setEvent(it);

}
void SiteEvent::handleEvent(
  std::set<Vertex>::reverse_iterator currentVertex, EventQueue & eq) {
 LOGP
 BeachLineNode * newarc = 0;
 BeachLineNode * rneigh = 0;
 BeachLineNode * lneigh = 0;
 BeachLineNode * mid = 0;
 BeachLineNode * lbp = 0;
 BeachLineNode * rbp = 0;
 Point_p sweeppoint;
 Point_p sweeprneigh;
 Point_p sweeplneigh;

 LOG("Handling SiteEvent")
 LOG_EXP((*currentVertex).print())
 LOG("siteevent")

 eq.getBeachLine()->setSweepLine((*currentVertex).getY());
 BeachLineNode* pParabolaAbove = eq.getBeachLine()->find(
   (*currentVertex).getX());

 if (pParabolaAbove->isRoot()) {
  eq.getBeachLine()->insert(&(*currentVertex), pParabolaAbove);
//for root just insert
  return;
 }

//delete CircleEvents interfered by new circle
 eq.removeEvent(pParabolaAbove);

 rneigh = pParabolaAbove->getRightNeighbor();
 lneigh = pParabolaAbove->getLeftNeighbor();

//special case SiteEvent at the bottom of a circle
//-> SiteEvent and corresponding CircleEvent coincide
 if (eq.checkSiteCircleEvent(pParabolaAbove, &(*currentVertex), rneigh,
   sweeprneigh)) {
// lneigh pParabolaAbove rneigh
//-> lneigh  pParabolaAbove newarc rneigh

//SiteCircleEvents do not split the arc below
  newarc = pParabolaAbove->insert_no_split(&(*currentVertex),
    eq.getBeachLine()->getSweepLineY());
//SiteCircleEvents are inserted without checks
//since these are done by checkSiteCircleEvent
  eq.insertSiteCircleEvent(newarc);

  rneigh = newarc->getRightNeighbor();
  lneigh = newarc->getLeftNeighbor();

  eq.removeEvent(rneigh);
  eq.removeEvent(lneigh);
// makes sure Event wont be reinserted below
  sweeprneigh.x = ERROR_VALUE;

 } else if (eq.checkSiteCircleEvent(pParabolaAbove, &(*currentVertex),
   lneigh, sweeplneigh)) {
// lneigh pParabolaAbove rneigh
//-> lneigh  newarc pParabolaAbove rneigh

  newarc = pParabolaAbove->insert_no_split(&(*currentVertex),
    eq.getBeachLine()->getSweepLineY());

  eq.insertSiteCircleEvent(newarc);

  rneigh = newarc->getRightNeighbor();
  lneigh = newarc->getLeftNeighbor();

  eq.removeEvent(rneigh);
  eq.removeEvent(lneigh);
// makes sure Event wont be reinserted below
  sweeplneigh.x = ERROR_VALUE;

 } else //regular case
 {
// lneigh pParabolaAbove rneigh
//-> lneigh pParabolaAbove' newarc pParabolaAbove'' rneigh

  newarc = eq.getBeachLine()->insert(&(*currentVertex), pParabolaAbove);

  if (newarc) {
   if (sweeprneigh.x != ERROR_VALUE) {
    mid = newarc->getRightNeighbor();
    eq.checkEventAndInsert(mid, sweeprneigh);
   }
   if (sweeplneigh.x != ERROR_VALUE) {
    mid = newarc->getLeftNeighbor();
    eq.checkEventAndInsert(mid, sweeplneigh);
   }
  }

  return;
 }

//now check for new CircleEvents to the left and to the right
 if (newarc) {
  mid = newarc->getLeftNeighbor(&rbp);
  if (mid != 0)
   lneigh = mid->getLeftNeighbor(&lbp);
  rneigh = newarc;

  eq.checkEventAndInsert(rneigh, mid, lneigh, rbp, lbp);

  mid = newarc->getRightNeighbor(&lbp);
  if (mid != 0)
   rneigh = mid->getRightNeighbor(&rbp);
  lneigh = newarc;

  eq.checkEventAndInsert(rneigh, mid, lneigh, rbp, lbp);
 }
}

}

