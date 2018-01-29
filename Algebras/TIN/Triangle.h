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

#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "SecondoDependencies.h"
#include "AbstractVertexContainer.h"
#include "TinHelper.h"
#include "Edge.h"
#include <vector>
#include <set>
#include <deque>

#ifdef TRIANGLE_NO_LOGGING
#define LOGGING_SWITCH_OFF
#endif
#include <TinLogging.h>

namespace tin {

struct heightWalkParameter {
 double distance;
 std::vector<double> values;
};

class TinPart;
class Triangle {
protected:
 TinPart * myPart;

 const Vertex * v1;
 const Vertex * v2;
 const Vertex * v3;

 Triangle * n1;
 Triangle * n2;
 Triangle * n3;

////////Construction/Destruction/////////////////////////////////
public:
 Triangle();
 ~Triangle();
 Triangle(const Vertex * v1, const Vertex * v2, const Vertex * v3,
   TinPart * imyPart = 0);
 Triangle(AbstractVertexContainer* vc, void * buff, uint32_t & offset,
   TinPart * imyPart);
 Triangle* clone();

////////Manipulation/////////////////////////////////////////////
 void addReference();
 void deleteReference();
 void removeNeighbor(Triangle *t);
 void removeDifferentPartNeighbors();
 void addNeighbor(Triangle *t, bool bidirectional = true);
 void matchNeighbors(std::deque<Triangle *>::iterator match,
   const std::deque<Triangle *>::iterator& end);
protected:
 void addOpenEnd();

///////Query/////////////////////////////////////////////////////
public:

 class triangleWalker {
 protected:
  bool (*userExit)(Triangle::triangleWalker * walker, Triangle* next);
  bool stayInSamePart;
  bool noload;
  Triangle * last;
  Triangle * current;
  Point_p * arPoints;
  Segment currentSegment;
  TIN_SIZE noPoints;
  int currentPoint;
  const void * userParameter1;

 public:
  static const int END_OF_SEGMENT = 1;
  static const int END_OF_ACCESS = 2;
  static const int NEW_POINT = 3;
  static const int TRIANGLE_ENTERED = 4;
  static const int END_OF_WALK = 5;
  static bool heightWalk(Triangle::triangleWalker * walker,
    Triangle* next);
  static bool stopAtEdgeWalk(Triangle::triangleWalker * walker,
    Triangle* next);

  triangleWalker(Triangle * istart, Point_p * iarPoints,
    TIN_SIZE inoPoints, bool istayInSamePart = false, bool inoload = false,
    bool (*iuserExit)(Triangle::triangleWalker * walker,
      Triangle* next) = 0, const void * istopParameter1 = 0) {

   if (inoPoints < 1)
    throw std::runtime_error("No points for construction is"
      " not allowed.(triangleWalker::triangleWalker)");

   if (!istart)
    throw std::runtime_error("No start triangle for constru"
      "ction is not allowed.(triangleWalker::triangleWalker)");

   noload = inoload;
   currentPoint = 0;
   arPoints = iarPoints;
   noPoints = inoPoints;
   userParameter1 = istopParameter1;
   userExit = iuserExit;
   stayInSamePart = istayInSamePart;
   last = 0;
   current = istart;

   currentSegment.init(*current->getVertex(1), iarPoints[0]);

  }

  triangleWalker(const triangleWalker & w) {
   currentPoint = w.currentPoint;

   arPoints = new Point_p[w.noPoints];

   for (int i = 0; i < w.noPoints; i++) {
    arPoints[i] = w.arPoints[i];
   }

   noPoints = w.noPoints;
   userParameter1 = w.userParameter1;
   userExit = w.userExit;
   stayInSamePart = w.stayInSamePart;
   last = w.last;
   current = w.current;

   currentSegment.init(*current->getVertex(1), arPoints[currentPoint]);
  }

  triangleWalker& operator=(const triangleWalker & w) {
   currentPoint = w.currentPoint;

   arPoints = new Point_p[w.noPoints];

   for (int i = 0; i < w.noPoints; i++) {
    arPoints[i] = w.arPoints[i];
   }

   noPoints = w.noPoints;
   userParameter1 = w.userParameter1;
   userExit = w.userExit;
   stayInSamePart = w.stayInSamePart;
   last = w.last;
   current = w.current;

   currentSegment.init(*current->getVertex(1), arPoints[currentPoint]);

   return *this;
  }

  ~triangleWalker() {
   if (arPoints)
    delete[] arPoints;

   noPoints = 0;
   arPoints = 0;
  }
  Point_p getCurrentDestination() {
   return arPoints[currentPoint];
  }
  bool isTriangle(Triangle * t) {
   return (t != (Triangle *) END_OF_ACCESS
     && t != (Triangle *) END_OF_SEGMENT && t != (Triangle *) END_OF_WALK);
  }
  void getPathInCurrentTriangle(Segment & s1, Segment & s2) {
   Edge e1, e2, e3;
   std::set<Vertex> vset;
   std::set<Vertex>::iterator it;

   if (currentSegment.intersection_sec(current->getEdge(1), e1)) {
    vset.insert(*(e1.getV1()));
    vset.insert(*(e1.getV2()));
   }
   if (currentSegment.intersection_sec(current->getEdge(2), e2)) {
    vset.insert(*(e2.getV1()));
    vset.insert(*(e2.getV2()));
   }
   if (currentSegment.intersection_sec(current->getEdge(3), e3)) {
    vset.insert(*(e3.getV1()));
    vset.insert(*(e3.getV2()));
   }

   it = vset.begin();

//special case turning in triangle
   if (currentPoint < noPoints - 1
     && current->isInside_sec(arPoints[currentPoint])) {
    if ((Point_p) (*it) == arPoints[currentPoint]) {
     s1 = Segment((*(++it)), arPoints[currentPoint]);
     s2 = Segment((*(++it)), arPoints[currentPoint]);
     return;
    }

    if ((Point_p) (*++it) == arPoints[currentPoint]) {
     s1 = Segment((*(--it)), arPoints[currentPoint]);
     it++;
     s2 = Segment((*(++it)), arPoints[currentPoint]);
     return;
    }

    s1 = Segment((*(it)), arPoints[currentPoint]);
    s2 = Segment((*(--it)), arPoints[currentPoint]);

    return;
   } else // regular case
   {
    if (vset.size() == 2)
     s1 = Segment((*it), (*(++it)));
    if (vset.size() == 1)
     s1 = Segment((*it), (*it));
   }
  }
  void addPoint(const Point_p & p) {
   Point_p * tmparPoints = new Point_p[noPoints + 1];

   for (int i = 0; i < noPoints; i++) {
    tmparPoints[i] = arPoints[i];
   }
   tmparPoints[noPoints] = p;
   noPoints++;

   delete[] arPoints;

   arPoints = tmparPoints;

   if (currentPoint == noPoints - 2 && currentPoint != 0) {

    currentPoint++; //move to next segment
    currentSegment = Segment(arPoints[currentPoint - 1],
      arPoints[currentPoint]);
    last = 0;
   }

   if (userExit)
    userExit(this, (Triangle*) NEW_POINT);
  }
  void setUserParameter1(void * up) {
   userParameter1 = up;
  }
  void setUserExit(
    bool (*iuserExit)(Triangle::triangleWalker * walker, Triangle* next)) {
   userExit = iuserExit;
  }
  void walk_sec();
  const void* getUserParameter1() {
   return userParameter1;
  }
  void walk_segment_sec();
  int walk_step_sec();
  int walk_step_sec_return(Edge &edge);
  Triangle * getCurrentTriangle() {
   return current;
  }
 };

 bool hasVertex(const Point_p& p) const {
  return (*v1 == p || *v2 == p || *v3 == p);
 }
 NeighborEdge getEdge(const Edge & e);
 const Vertex * getMaxYVertex() const;
 const Vertex * getMaxYCWVertex() const;
 const Vertex * getMaxYCCWVertex() const;
 const Vertex * getMiddleYVertex() const;
 const Vertex * getMinYVertex() const;
 NeighborEdge getNextEdge(const NeighborEdge& currentEdge);
 NeighborEdge getNextEdge_noload(const NeighborEdge& currentEdge);
 Triangle* getNeighbor(const int index);
 Triangle * walkToTriangle_sec(const Point_p & destination);
 bool isInSamePart(const Triangle *t) const;
 bool hasNeighbor(Triangle * t);
 bool isNeighbor(Triangle &t);
 bool checkNeighbors();
 TinPart* getPart() {
  return myPart;
 }
 static const Vertex* isTriangle_sec(const Vertex *iv1, const Vertex *iv2,
   const Vertex *iv3);
 bool isCompatibleWith_sec(const Triangle & iat) const;
 bool isInside_sec(const Point_p& p) const;
 bool operator==(const Triangle & t) const;
 bool isEqual(Vertex& v1, Vertex& v2, Vertex& v3);
 VERTEX_Z getValue(const Point_p& p) const;

 Rectangle bbox() const;
 bool hasEdge(const Edge& e);
 Edge getEdge(const int n) const;
 const Vertex* getVertex(const int n) const;
 int getIndexInArray() const;

 NeighborEdge getEdgeWithVertex(const Vertex * iv);

 class edge_iterator {
 protected:
  NeighborEdge firstEdge;
  NeighborEdge currentEdge;
  bool direction;
  static const bool FORWARD;
  static const bool BACKWARD;

 public:

  edge_iterator(NeighborEdge ne) :
    firstEdge(ne), currentEdge(ne) {
   direction = FORWARD;
  }

  NeighborEdge& operator*() {
   LOGP
   return currentEdge;
  }

  void operator++(int) {
   LOGP
   if (direction == FORWARD) {
    currentEdge = currentEdge.getNextEdge();

    if (currentEdge.isNull()) //reached the end of tin
    {
     direction = BACKWARD;
     currentEdge = firstEdge;
     (*this)++;
     return;
    }
   } else {
    currentEdge = currentEdge.getPriorEdge();

    if (currentEdge.isNull()) //reached the end of tin
    {
     direction = FORWARD;
     currentEdge = firstEdge;
    }

   }
LOGP }

 void operator--(int) {
  LOGP
  if (direction == FORWARD) {
   currentEdge = currentEdge.getPriorEdge();

   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = BACKWARD;
    currentEdge = firstEdge;
    (*this)--;
    return;
   }
  } else {
   currentEdge = currentEdge.getNextEdge();

   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = FORWARD;
    currentEdge = firstEdge;
   }
  }
  LOGP
 }

 bool isFirst() {
  return currentEdge == firstEdge;
 }

 bool isEnd() {
  return currentEdge.isNull();
 }

 edge_iterator first() {
  LOGP
  return edge_iterator(firstEdge);
 }

};

class samepart_edge_iterator: public edge_iterator {
protected:
 TinPart * part;
public:
 samepart_edge_iterator(NeighborEdge ne,TinPart * p) :
 edge_iterator(ne) {
  part = p;
 }
 void operator++(int) {
  LOGP
  if (direction == FORWARD) {

   if (currentEdge.getN2()&&currentEdge.getN2()
     !=VORONOI_OPEN_END && currentEdge.getN2()->getPart() == part)
   currentEdge = currentEdge.getNextEdge_noload();
   else //don t load neighbor
   {
    direction = BACKWARD;
    currentEdge = firstEdge;
    (*this)++;
    return;
   }

   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = BACKWARD;
    currentEdge = firstEdge;
    (*this)++;
   }
  } else {

   if (currentEdge.getN1()&&currentEdge.getN1()
     !=VORONOI_OPEN_END && currentEdge.getN1()->getPart() == part)
   currentEdge = currentEdge.getPriorEdge_noload();
   else //don t load neighbor
   {
    direction = FORWARD;
    currentEdge = firstEdge;
   }

   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = FORWARD;
    currentEdge = firstEdge;
   }

  }
  LOGP
 }

 void operator--(int) {
  LOGP
  if (direction == FORWARD) {
   if (currentEdge.getN1()&&currentEdge.getN1()
     !=VORONOI_OPEN_END && currentEdge.getN1()->getPart() == part)
   currentEdge = currentEdge.getPriorEdge_noload();
   else {
    direction = BACKWARD;
    currentEdge = firstEdge;
    (*this)--;
    return;
   }

   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = BACKWARD;
    currentEdge = firstEdge;
    (*this)--;
   }
  } else {
   if (currentEdge.getN2()&&currentEdge.getN2()
     !=VORONOI_OPEN_END && currentEdge.getN2()->getPart() == part)
   currentEdge = currentEdge.getNextEdge_noload();
   else {
    direction = FORWARD;
    currentEdge = firstEdge;
   }
   if (currentEdge.isNull()) //reached the end of tin
   {
    direction = FORWARD;
    currentEdge = firstEdge;
   }
  }
  LOGP
 }

};
protected:
static const Vertex* isTriangle_mp
(const Vertex *iv1,const Vertex *iv2,const Vertex *iv3);
bool isInside_mp(const Point_p& p) const;
bool hasVertices2D(const Vertex& iv1, const Vertex& iv2);
public:
///////Presentation//////////////////////////////////////////////
void print(std::ostream & out = std::cout) const;
void printNeighbor(std::ostream & out = std::cout) const;
#ifndef UNIT_TEST
ListExpr outTriangle();
#endif
///////Persistence///////////////////////////////////////////////
static TIN_SIZE getSizeOnDisc();
bool putSecondoRepresentation(const AbstractVertexContainer* vc, void * buff,
  uint32_t & offset) const;
bool putSTLbinaryRepresentation(void * buff,
  uint32_t & offset) const;
/////////////////////////////////////////////////////////////////

};

} /* namespace tin*/
#endif /* INDEXTRIANGLE_H_*/
