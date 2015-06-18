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

#ifndef ALGOFORTUNE_H_
#define ALGOFORTUNE_H_

#include "Vertex.h"
#include "TinHelper.h"
#include "Tin.h"
#include <map>
#include "gmpxx.h"

#ifdef FORTUNE_NO_LOGGING
#define LOGGING_SWITCH_OFF
#endif
#include <TinLogging.h>

namespace tin {

class EventQueue;
class BeachLineNode;
class VoronoiEdge;
/*
 The class ~CircleEvent~ represents a circle event.
 It keeps the lower coordinate of the
 circle in attribute ~sweep~.
 In attribute ~arc~ it keeps a pointer to the leaf in the
 beach line tree representing the disappearing arc.
 There are two ways of handling a
 circle event depending on the kind of circle event.
 Whenever there are exactly three vertices
 on an empty circle the method ~handleCircleEvent~ is called.
 In the special case, that
 there are more vertices on an empty
 circle the method ~handleMultiEvent~ is called.

*/
class CircleEvent {
protected:
 Point_p sweep;
 BeachLineNode * arc;
public:
 CircleEvent(Point_p ip, BeachLineNode *arc);
 ~CircleEvent();
 bool isIdentical(CircleEvent * e) const;
 bool isIdentical(BeachLineNode * n) const;
 VERTEX_COORDINATE getY() const;
 Point_p getSweepPoint() const {
  return sweep;
 }
 void removeArcMultiEvent(Triangle ** t1, Triangle ** t2);
 void handleEvent(EventQueue & eq);
 void handleMultiEvent(EventQueue & eq,
   std::multimap<Point_p, CircleEvent*>::iterator from,
   std::multimap<Point_p, CircleEvent*>::iterator to);
 bool isEqual(CircleEvent * e) const;
 static Point_p calculateSweepPoint_mp(const Vertex * p1,
   const Vertex * p2, const Vertex * p3);
 static Point_p calculateSweepPoint(Point_p p1, Point_p p2, Point_p p3);
 inline static Point_p calculateSweepPoint_quad(const Vertex* sright,
   const Vertex* smid, const Vertex* sleft, const __float128 & precalc12,
   const __float128 & precalc23);
 static Point_p calculateCircle_mp(const Vertex & v1, const Vertex &v2,
   const Vertex & v3, double & r);
 static bool isVertexInside(Point_p & middle, double radius,
   const Vertex & v);
 void print(std::ostream& os = std::cout) const;
};

/*
 The class SiteEvent keeps just the handling routine for site events.
 The coordinates of site events are kept in the VertexContainerSet,
 which itself is used as site event queue.

*/
class SiteEvent {
public:
 static void handleEvent(std::set<Vertex>::reverse_iterator v,
   EventQueue & eq);

};

/*
 For performance reasons the direction of breakpoints is evaluated.
 This enum keeps the possible values.

*/

enum BREAK_DIRECTION {
 BREAK_X_UNDEFINED,
 BREAK_X_LEFT,
 BREAK_X_RIGHT,
 BREAK_X_VERTICAL,
 BREAK_Y_HORIZONTAL,
 BREAK_Y_UP,
 BREAK_Y_DOWN
};

/*

 The class ~BeachLineNode~ represents a node in the beach line tree.
 This can be an inner node as well as a leaf. Inner nodes represent
 a breakpoint on the beach line and leafs are the arcs. So inner nodes carry
 the two sites building the breakpoint between them (parabola intersection).
 The leafs just carry one site that represents a parabola or arc on the beach
 line. Leafs always carry their site in the attribute ~site1~. Leafs have an
 iterator ~event~ pointing to the connected ~CircleEvent~ in the queue.
 The direction right and left is always seen from the beach line. For inner
 nodes ~site1~ is to the left of the breakpoint and ~site2~ is to the right of the breakpoint.
 Since the beach line tree is a red/black tree the nodes carry an attribute ~red~.
 For performance reasons there is the attribute ~breakx~. It has just meaning for
 inner nodes. It remembers the x coordinate of the breakpoint.

*/
class BeachLineNode {
 friend class BeachLine;
 friend class BeachLine_test;
protected:
 const Vertex * site1; // for leafs and inner nodes
 const Vertex * site2; // for inner nodes
 std::multimap<Point_p, CircleEvent*>::iterator event; // for leafs
 BeachLineNode * pleftSon; // for inner nodes
 BeachLineNode * prightSon; // for inner nodes
 BeachLineNode * parent; // for leafs and inner nodes

 VoronoiEdge * edge; // for inner nodes

 bool validIterator; // for leafs
 bool red; // inner nodes

 mutable VERTEX_COORDINATE break_x; // for inner nodes
 mutable __float128 precalc_circle; // for inner nodes

 static const VERTEX_COORDINATE BREAK_X_UNSET;

 static BeachLineNode nullnode;
public:
 static const __float128 PRECALC_UNSET;

////////Construction/Destruction/////////////////////////////////
protected:
 BeachLineNode(BeachLineNode * iparent, bool leftins, BeachLineNode * left,
   BeachLineNode * right); //inner node
 BeachLineNode(const Vertex * pV1, BeachLineNode * iparent); //Leaf
 BeachLineNode(); //root

 virtual ~BeachLineNode();
////////Manipulation/////////////////////////////////////////////
protected:
 void setParent(BeachLineNode * p);
 void setLeft(BeachLineNode * nl);
 void setRight(BeachLineNode * nr);
 BeachLineNode* insert(const Vertex * pV);
 void balanceTree(); // red black tree balancing for insertion
 void balanceTreeDeletion(); // red black tree balancing for deletion
 void rotateLeft();
 void rotateRight();
 void setSite1(const Vertex * psite1);
 void setSite2(const Vertex * psite2);
 void resetBreakX() {
  break_x = BREAK_X_UNSET;
 }
public:
 void resetVoronoiEdge();
 void resetRescuedVoronoiEdge();
 void rescueVoronoiEdge(VoronoiEdge *e);
 void setVoronoiEdge(VoronoiEdge *e);
 BeachLineNode * remove(Triangle * t);
 BeachLineNode * removeMultiEvent(Triangle** t1, Triangle** t2);
 BeachLineNode* insert_no_split(const Vertex * pV,
   const VERTEX_COORDINATE sweepy);
 void setEvent(std::multimap<Point_p, CircleEvent*>::iterator & it) {
  if (validIterator)
   throw std::runtime_error(E_BEACHLINENODE_SETEVENT);
  validIterator = true;
  event = it;
 }
 void resetEvent() {
  validIterator = false;
 }
///////Query/////////////////////////////////////////////////////
protected:
 VERTEX_COORDINATE getBreakX() {
  return break_x;
 }

 BREAK_DIRECTION determineBreakXDirection(const Vertex* v1,
   const Vertex* v2) const;
 BREAK_DIRECTION determineBreakYDirection(const Vertex* v1,
   const Vertex* v2) const;
 BeachLineNode * getLeft() const;
 BeachLineNode * getRight() const;
 BeachLineNode * find(const VERTEX_COORDINATE x,
   const VERTEX_COORDINATE ySweepLine) const;
 bool isVirtualRoot() const;
 void check() const;
public:
 __float128& getPrecalc()
 {
  return precalc_circle;
 }
 BREAK_DIRECTION getBreakXDirection() const {
  if (isLeaf())
  throw std::runtime_error(
    "Leafs have no Breakpoint !(BeachLineNode::getBreakXDirection)");
  if (*site1 == *site2)
  throw std::runtime_error(
    "BeachLine corrupted.(BeachLineNode::getBreakXDirection)");

  return determineBreakXDirection(site1, site2);
 }
 BREAK_DIRECTION getBreakYDirection() {
  if (isLeaf())
  throw std::runtime_error(
    "Leafs have no Breakpoint !(BeachLineNode::getBreakXDirection)");
  if (*site1 == *site2)
  throw std::runtime_error(
    "BeachLine corrupted.(BeachLineNode::getBreakXDirection)");

  return determineBreakYDirection(site1, site2);
 }
 VoronoiEdge* getVoronoiEdge() const;
 bool isLeftChild() const;
 bool isNull() const;
 bool isLeaf() const;
 bool isRoot() const;
 int isCollapsing_performance() const {
  BeachLineNode * lbp = getLeftBreakPoint();
  BeachLineNode * rbp = getRightBreakPoint();

  if (!lbp || !rbp)
  return 0;

  BREAK_DIRECTION lx = lbp->getBreakXDirection();
  BREAK_DIRECTION ly = lbp->getBreakYDirection();
  BREAK_DIRECTION rx = rbp->getBreakXDirection();
  BREAK_DIRECTION ry = rbp->getBreakYDirection();

//first try to find a solution without calculations

  if (((lx == BREAK_X_RIGHT && rx == BREAK_X_LEFT)
      || (lx == BREAK_X_RIGHT && rx == BREAK_X_VERTICAL)
      || (lx == BREAK_X_VERTICAL && rx == BREAK_X_LEFT)
      || (((ly == BREAK_Y_UP||ly == BREAK_Y_HORIZONTAL) && (ry == BREAK_Y_DOWN))
        && (lx == BREAK_X_RIGHT && rx == BREAK_X_RIGHT))
      || ((ly == BREAK_Y_UP && (ry == BREAK_Y_DOWN||ry == BREAK_Y_HORIZONTAL))
        && (lx == BREAK_X_RIGHT && rx == BREAK_X_RIGHT))
      || (((ly == BREAK_Y_DOWN||ly==BREAK_Y_HORIZONTAL)&& (ry == BREAK_Y_UP))
        && (lx == BREAK_X_LEFT && rx == BREAK_X_LEFT))
      || (((ly == BREAK_Y_DOWN)&& (ry == BREAK_Y_UP||ry==BREAK_Y_HORIZONTAL))
        && (lx == BREAK_X_LEFT && rx == BREAK_X_LEFT)))) {
   return 1;
  }

  if (((lx == BREAK_X_LEFT && rx == BREAK_X_RIGHT)
      || (lx == BREAK_X_LEFT && rx == BREAK_X_VERTICAL)
      || (lx == BREAK_X_VERTICAL && rx == BREAK_X_RIGHT)
      || (lx == BREAK_X_VERTICAL && rx == BREAK_X_VERTICAL)
      || (((ly == BREAK_Y_DOWN||ly == BREAK_Y_HORIZONTAL) && ry == BREAK_Y_UP)
        && (lx == BREAK_X_RIGHT && rx == BREAK_X_RIGHT))
      || ((ly == BREAK_Y_DOWN && (ry == BREAK_Y_UP||ry == BREAK_Y_HORIZONTAL))
        && (lx == BREAK_X_RIGHT && rx == BREAK_X_RIGHT))
      || ((ly == BREAK_Y_UP && (ry == BREAK_Y_DOWN||ry == BREAK_Y_HORIZONTAL))
        && (lx == BREAK_X_LEFT && rx == BREAK_X_LEFT))
      || (((ly == BREAK_Y_UP||ly == BREAK_Y_HORIZONTAL) && ry == BREAK_Y_DOWN)
        && (lx == BREAK_X_LEFT && rx == BREAK_X_LEFT))
    )) {
   return 0;
  }
//some cases cannot be found without calculation

  PreciseDouble lbpdydx, rbpdydx;

  lbpdydx = ((PreciseDouble) lbp->site1->getY()
    - (PreciseDouble) lbp->site2->getY())
  / ((PreciseDouble) lbp->site1->getX()
    - (PreciseDouble) lbp->site2->getX());
  rbpdydx = ((PreciseDouble) rbp->site1->getY()
    - (PreciseDouble) rbp->site2->getY())
  / ((PreciseDouble) rbp->site1->getX()
    - (PreciseDouble) rbp->site2->getX());

  lbpdydx.makeAbs();
  rbpdydx.makeAbs();

  if (lx == BREAK_X_RIGHT) {
   SecureOperator::startSecureCalc();
   if (ly == BREAK_Y_DOWN && rbpdydx < lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 1;
    else
    return -1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_DOWN) && rbpdydx > lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 0;
    else
    return -1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_UP) && rbpdydx > lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_UP) && rbpdydx < lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 0;
    else
    return -1;

   }
  }
  if (lx == BREAK_X_LEFT) {
   SecureOperator::startSecureCalc();
   if (ly == BREAK_Y_DOWN && rbpdydx > lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 1;
    else
    return -1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_DOWN) && rbpdydx < lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 0;
    else
    return -1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_UP) && rbpdydx < lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 1;
    else
    return -1;
   }
   SecureOperator::startSecureCalc();
   if ((ly == BREAK_Y_UP) && rbpdydx > lbpdydx) {
    if (SecureOperator::isSecureResult())
    return 0;
    else
    return -1;
   }
  }

  return -1;

 }
 bool isCollapsing_mp(const VERTEX_COORDINATE y) const {
  BeachLineNode * lbp = getLeftBreakPoint();
  BeachLineNode * rbp = getRightBreakPoint();
// normal precision is not enough so try mp...
  mpq_class lbpdydx, rbpdydx, rsite1x, rsite1y, rsite2x, rsite2y, lsite1x,
  lsite1y, lsite2x, lsite2y;

  rsite1y = rbp->site1->getY();
  rsite1x = rbp->site1->getX();
  rsite2y = rbp->site2->getY();
  rsite2x = rbp->site2->getX();

  lsite1y = lbp->site1->getY();
  lsite1x = lbp->site1->getX();
  lsite2y = lbp->site2->getY();
  lsite2x = lbp->site2->getX();

  BREAK_DIRECTION lx = lbp->getBreakXDirection();
  BREAK_DIRECTION ly = lbp->getBreakYDirection();

  lbpdydx = (lsite1x - lsite2x) / (lsite1y - lsite2y);
  rbpdydx = (rsite1x - rsite2x) / (rsite1y - rsite2y);

  if (lx == BREAK_X_RIGHT) {
   if (ly == BREAK_Y_DOWN && abs(rbpdydx) < abs(lbpdydx)) {
    return true;
   }
   if ((ly == BREAK_Y_DOWN) && abs(rbpdydx) > abs(lbpdydx)) {
    return false;
   }
   if ((ly == BREAK_Y_UP) && abs(rbpdydx) > abs(lbpdydx)) {
    return true;
   }
   if ((ly == BREAK_Y_UP) && abs(rbpdydx) < abs(lbpdydx)) {
    return false;

   }
  }
  if (lx == BREAK_X_LEFT) {
   if (ly == BREAK_Y_DOWN && abs(rbpdydx) > abs(lbpdydx)) {
    return true;
   }
   if ((ly == BREAK_Y_DOWN) && abs(rbpdydx) < abs(lbpdydx)) {
    return false;
   }
   if ((ly == BREAK_Y_UP) && abs(rbpdydx) < abs(lbpdydx)) {
    return true;
   }
   if ((ly == BREAK_Y_UP) && abs(rbpdydx) > abs(lbpdydx)) {
    return false;
   }
  }

//return false;
  mpq_class bp_left,bp_right;

  if (lbp)
  lbp->getParabolaIntersection_mp(y,bp_left);
  else
  throw std::runtime_error(E_EVENTQUEUE_CHECKEVENTANDINSERT);

  if (rbp)
  rbp->getParabolaIntersection_mp(y,bp_right);
  else
  throw std::runtime_error(E_EVENTQUEUE_CHECKEVENTANDINSERT);

  if ((abs(bp_left - bp_right) < 0.000001))
  {
   return true;
  }

  return false;

 }
 BeachLineNode * getLeftBreakPoint() const;
 BeachLineNode * getRightBreakPoint() const;
 BeachLineNode * getBrother() const;
 BeachLineNode * getLeftNeighbor() const;
 BeachLineNode * getLeftNeighbor(BeachLineNode** bp) const;
 BeachLineNode * getRightNeighbor() const;
 BeachLineNode * getRightNeighbor(BeachLineNode** bp) const;
 BeachLineNode * getLeftMost() const;
 BeachLineNode * getRightMost() const;
 BeachLineNode * getLeftNeighbor(POINT_COORDINATE & breakpoint,
   const VERTEX_COORDINATE sweepliney) const;
 BeachLineNode * getRightNeighbor(POINT_COORDINATE& breakpoint,
   const VERTEX_COORDINATE sweepliney) const;
 inline const Vertex * getSite() const {
  return site1;
 }

 BeachLineNode * getParent() const;
 BeachLineNode * getGrandParent() const;
 BeachLineNode * getUncle() const;
 const std::multimap<Point_p, CircleEvent*>::iterator getEvent() const {
  return event;
 }
 bool isValidEvent() const {
  return validIterator;
 }
 POINT_COORDINATE getParabolaIntersection_sec(
   const VERTEX_COORDINATE ySweepLine) const;
 VERTEX_COORDINATE getParabolaIntersection(
   const VERTEX_COORDINATE ySweepLine) const;
 void getParabolaIntersection_mp(const VERTEX_COORDINATE ySweepLine,
   mpq_class & result) const;
///////Presentation//////////////////////////////////////////////
 void print(std::ostream & os = std::cout, int tab = 1) const;
 void print_simple(std::ostream & os = std::cout) const;

};

/*
 The class ~BeachLine~ represents the beach line by the tree beneath the ~root~.
 The beach line depends on the y coordinate of the sweep line. So attribute
 ~ySweepLine~ carries this coordinate.

*/
class BeachLine {
protected:
 BeachLineNode root;
 VERTEX_COORDINATE ySweepLine;
public:
 BeachLine();
 virtual ~BeachLine();
 void checkTree() const;

 VERTEX_COORDINATE getSweepLineY() const {
  return ySweepLine;
 }

 BeachLineNode* insert(const Vertex * psite, BeachLineNode* parabola);
 void setSweepLine(const VERTEX_COORDINATE ySweepLine);
 BeachLineNode * find(const VERTEX_COORDINATE x) const;
 void print(std::ostream & os = std::cout) const;
 void print_small(std::ostream & os = std::cout) const;
 void rescueVertices(VertexContainerSet* vertices);
 friend class BeachLine_test;

};
/*
 The ~EventQueue~ coordinates the hole process of delaunay triangulation.
 It produces a delaunay triangulation from a set of vertices.
 It receives a tin at construction time to which it delivers the triangles created.
 To triangulate ~Vertices~ just pass a VertexContainerSet to the method ~doFortuneAlgorithm~.
 This set has to be on the heap and will be freed by the algorithm. There can be several
 consecutive calls of doFortuneAlgorithm to make the triangulation piecewise. Memory is freed when possible.
 Whenever the triangulation is finished call ~doFortuneAlgorithm~ with the parameter final set to true.
 The triangulation needs O(n log n) time and O(n) space.
 The triangulation is not robust since there might occur real numbers (sqrt 2)
 arbitrary precision is not implemented in cases where real numbers might occur.
 Experience is that this is rarely a problem and even miscalculations don t necessarily corrupt the hole tin.
 There might be overlapping triangles as a result of miscalculations.

*/
class EventQueue {
protected:
 std::set<Vertex>::reverse_iterator siteEventQueue;
 std::multimap<Point_p, CircleEvent*> circleEventQueue;
 BeachLine bl;
 Tin * tt;
 VertexContainerSet * verticesPriorSection;LOG_EXP(int eventId)
public:
 EventQueue(Tin * itt);
 ~EventQueue();

 bool isCircleAlreadyInQueue(const Point_p& sweeppoint) const;
 const std::multimap<Point_p, CircleEvent*>::iterator addCircleEvent(
   CircleEvent & e, bool & alreadyExists);
 /*
  The method doFortuneAlgorithm triangulates a section of vertices.
  The vertices are given by the container current section.
  The parameter finalSection
  indicates the last section. The final section call frees all
  memory occupied by the algorithm.
  The produced triangles will be delivered to the Tin given at
  construction time.
  The container currentSection has to be on the heap and will
  be freed by the algorithm !
  The sections have to be ordered by the y coordinate from top to
  bottom (top first) !

 */
 void doFortuneAlgorithm(VertexContainerSet * currentSection,
   bool finalSection = false);
 void createTriangle(const Vertex * v1, const Vertex * v2,
   const Vertex * v3, Triangle** newtriangle);
 BeachLine * getBeachLine() {
  return &bl;
 }
 Tin * getTinType() {
  return tt;
 }
 std::multimap<Point_p, CircleEvent*> * getCircleQueue() {
  return &circleEventQueue;
 }
 std::set<Vertex>::reverse_iterator getSiteQueue() {
  return siteEventQueue;
 }
 void print(std::ostream& os = std::cout);
 void print_small(std::ostream& os = std::cout) {
  os << "Events in the Queue.......................\n";
  std::multimap<Point_p, CircleEvent*>::reverse_iterator itc =
    circleEventQueue.rbegin();

  os << "Current Event: \n";
  (*itc).second->print(os);
  (*siteEventQueue).print(os);

  os << "..........................................\n";

  bl.print_small(os);

  os << "..........................................\n";
 }
 void checkEventAndInsert(BeachLineNode* mid, Point_p& sweep);
 bool checkSiteCircleEvent(BeachLineNode* crneigh, const Vertex * vmid,
   BeachLineNode* clneigh, Point_p& sweep);
 void checkEventAndInsert(BeachLineNode* crneigh, BeachLineNode * cmid,
   BeachLineNode* clneigh, BeachLineNode* rbp, BeachLineNode* lbp);
 void insertSiteCircleEvent(BeachLineNode* arc);
 void removeEvent(BeachLineNode* arc);
private:
 EventQueue(const EventQueue & eq) {
  throw std::runtime_error("NOT IMPLEMENTED");
 }
 EventQueue& operator=(const EventQueue & ieq) {
  throw std::runtime_error("NOT IMPLEMENTED");
 }
 ;
};
/*
 The class ~VoronoiEdge~ represents an edge in the Voronoi diagram.
 As an Edge in a Voronoi diagram is between two vertices of the Voronoi diagram,
 it is between two triangles (or the outside) of the Delaunay triangulation.
 It can thus be used to generate the neighborship of the generated triangles
 without much additional effort. Each edge in the Voronoi diagram is traced out by
 one or two breakpoints of the ~BeachLine~. And each breakpoint on the ~BeachLine~
 is on an edge of the Voronoi diagram. VoronoiEdge 1 - 1|2 breakpoint . The breakpoint
 is represented by an inner node of the ~BeachLine~ tree. These inner nodes carry
 the relationship as an attribute ~edge~ (pointer).
 Whenever a breakpoint disappears due to the creation of a triangle the breakpoint adds
 an end to the ~VoronoiEdge~ connected. Whenever an edge has two ends it installs the
 neighborship between the two triangles at both vertices of the edge and deletes itself.
 So the coordinates of the edge do not play a role. The remaining ~VoronoiEdges~ at the
 end of the triangulation represent the edges at the boundary of the triangulation and are
 finished by an open end.

*/
class VoronoiEdge: public noncopyable {
private:
 Triangle *t1;
 Triangle *t2;

 VoronoiEdge() {
  t1 = 0;
  t2 = 0;
//instanceCounter++;
 }
 ~VoronoiEdge() {
//instanceCounter--;
LOG ("Delete VoronoiEdge")
 LOG(this)
}
public:

//static int instanceCounter; // just to be safe
void destroy() {
 addEnd(VORONOI_OPEN_END);
}

void destroyHard() {
 if (t1 && t1 != VORONOI_OPEN_END)
 t1->deleteReference();
 if (t2 && t2 != VORONOI_OPEN_END)
 t2->deleteReference();
 delete this;
}

static VoronoiEdge * getInstance(BeachLineNode * bp1, BeachLineNode * bp2 =
  0) {
 if (!bp1 && !bp2)
 throw std::runtime_error(
   "Tried to create a VoronoiEdge "
   "without any breakpoint.(VoronoiEdge::getInstance)");

 VoronoiEdge * newedge = new VoronoiEdge();

 if (bp1) {
  bp1->setVoronoiEdge(newedge);
 }
 if (bp2) {
  bp2->setVoronoiEdge(newedge);
 }

 return newedge;
}

Triangle * getOnlyEnd() {
 if (t1 && t2)
 throw std::runtime_error(
   "There are two ends.(VoronoiEdge::getOnlyEnd)");

 if (t1)
 return t1;
 else
 return t2;
}

void setOnlyEnd(Triangle *t) {
 if (t1 && t2)
 throw std::runtime_error(
   "There are two ends.(VoronoiEdge::setOnlyEnd)");

 if (t1)
 t1 = t;
 else
 t2 = t;

 if (t && t != VORONOI_OPEN_END)
 t->addReference();
}

void addEnd(Triangle * t) {

 try {

  if (!t)
  throw std::runtime_error(E_VORONOIEDGE_ADDEND0);
  if (t1 && t2)
  throw std::runtime_error(E_VORONOIEDGE_ADDEND);

  if (t != VORONOI_OPEN_END)
  t->addReference();

  if (!t1)
  t1 = t;
  else
  t2 = t;

  if (t1 && t2) {
   if (!(t1 == VORONOI_OPEN_END)) {
    t1->addNeighbor(t2, false);
    t1->deleteReference();
   }

   if (!(t2 == VORONOI_OPEN_END)) {
    t2->addNeighbor(t1, false);
    t2->deleteReference();
   }

   delete this;
  }
 } catch (std::exception & e) {

  LOG(e.what())
  delete this;
 }
}

};

}
#endif
