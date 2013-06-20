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

#include "Coarsening.h"


namespace p2d {

CoarseningGraph::CoarseningGraph() {
}

CoarseningGraph::~CoarseningGraph() {
}

void CoarseningGraph::computeGraph(/*const*/Region2& r) {
 Reg2GridHalfSegment gs;
 for (int i = 0; i < r.Size(); i++) {
  r.getgridCoordinates()->Get(i, gs);
  if (gs.IsLeftDomPoint()
    && ((gs.GetLeftPointX() != gs.GetRightPointX())
      || (gs.GetLeftPointY() != gs.GetRightPointY()))) {

   int absX = abs(gs.GetLeftPointX() - gs.GetRightPointX());
   int absY = abs(gs.GetLeftPointY() - gs.GetRightPointY());
   if ((absX + absY) == 1) {
    //the segment starts and ends in adjacent grid points.
    Vertex* v1 = insert(gs.GetLeftPointX(), gs.GetLeftPointY());
    Vertex* v2 = insert(gs.GetRightPointX(), gs.GetRightPointY());
    v1->addEdge(v2);
    v2->addEdge(v1);
   } else {
    //the start- and end-point of the segment are not adjacent, the segment
    //runs through a number of grid-points. For each interesting points
    //a vertex has to be created.
    computeGraphBetween(gs.GetLeftPointX(), gs.GetLeftPointY(),
      gs.GetRightPointX(), gs.GetRightPointY());
   }
  }
 }
}

void CoarseningGraph::computeGraphBetween(int lx, int ly, int rx, int ry) {
 Vertex left(lx, ly);
 Vertex right(rx, ry);
 if (left != right) {
  if (left < right) {
   //nothing to do
  } else {
   Vertex temp(left);
   left = right;
   right = temp;
  }
 } else {
  //start- and end-points are equal, there is no need for a nother vertex.
  return;
 }


 int x = left.getX();
 int y = left.getY();
 if ((x!=right.getX()) && ((y != right.getY()))){
 Vertex* v1 = insert(left.getX(), left.getY());
 //double slope = (left.getY() - right.getY())
 //  / (((double) left.getX()) - right.getX());
 mpq_class num = left.getY() - right.getY();
 mpq_class den = left.getX() - right.getX();
 mpq_class slope = num / den;

 while ((x != right.getX()) && (y != right.getY())) {
  //the segment runs through some grids in both directions

  // the segment intersects the vertical line in (x+1) in ~yValue~.

  mpq_class yValue = ((slope * (x + 1)) + right.getY()
    - (slope * right.getX()));
  int newY; // gets the y-value of the next vertex
  if (cmp(y, yValue)<0){
   if (cmp(slope, 0)>0){
    //ascending segment
    //yValue is above y and the right point has not been reached. For the next
    //vertex, yValue is rounded up to the next integer.
    newY = (int) (ceil_mpq(yValue)).get_d();
   } else {
    //horizontal or descending segment
    newY = y;
   }

   while (y < newY) {
    //(newY-y) new vertices above y and the corresponding edges
    //have to be created.
    Vertex*v2 = insert(x, y + 1);
    v1->addEdge(v2);
    v2->addEdge(v1);
    v1 = v2;
    y++;
   }
   //create a new Vertex right of the vertex (x, newY) incl. the corresponding
   //edges
   Vertex* v2 = insert(x + 1, y);
   v1->addEdge(v2);
   v2->addEdge(v1);
   v1 = v2;
  } else {
   //for more info see if-branch
   if (cmp (slope, 0)<=0) {
    newY = (int)(floor_mpq(yValue)).get_d();
   } else {
    newY = y;
   }
   while (y > newY) {
    Vertex* v2 = insert(x, y - 1);
    v1->addEdge(v2);
    v2->addEdge(v1);
    v1 = v2;
    y--;
   }
   Vertex* v2 = insert(x + 1, y);
   v1->addEdge(v2);
   v2->addEdge(v1);
   v1 = v2;
  }
  x++;
 }
 }
 if (x != right.getX()) {
  //the segment runs through some grid which have all the same y-value,
  //the coarse segment will be horizontal line
  Vertex* first = insert(x, y);
  x++;
  while (x <= right.getX()) {
   Vertex* second = insert(x, y);

   first->addEdge(second);
   second->addEdge(first);
   first = second;
   x++;
  }
 }

 if (y != right.getY()) {
  //the segment runs through some grid which have all the same x-value,
  //the coarse segment will be vertical line
  Vertex* first = insert(x, y);
  y++;
  while (y <= right.getY()) {
   Vertex* second = insert(x, y);
   first->addEdge(second);
   second->addEdge(first);
   first = second;
   y++;
  }
 }
}

void CoarseningGraph::calculateBoundary(Region& result) {

 result.Clear();
 if (vertices.size() < 4) {
  //a region needs at least 4 nodes a coarse region has only 90 degree angles
  result.SetDefined(true);
  return;
 }

 //cut loose ends
 cutLooseEnds();

 //compute boundary
 result.SetDefined(true);
 Region tmpRegion(0);

 int visited = 1;

 while (vertices.size() >= 4) {
  //there are enough vertices to create another cycle

  try {
   tmpRegion.Clear();
   tmpRegion.StartBulkLoad();

   //create next cycle, which becomes an outercycle or a hole
   bool lastInsideAbove = false;
   bool insideAbove = false;
   Vertex* firstVertex = *(vertices.begin());
   Vertex* last = firstVertex;
   Direction lastDirection = noDirection;
   Direction dir = noDirection;
   Vertex* next;

   int firstX = last->getX(); //the x-value of the first vertex
   int firstY = last->getY(); //the y-value of the first vertex

   HalfSegment* hs;
   int edgeNo = 0;
   Point* lp = new Point(true, last->getX(), last->getY());
   Point* rp = NULL;

   do {
    last->getNextVertex(&next, dir, insideAbove, firstX, firstY, visited);

    if (next == NULL) {
     //throw exception();
    }
    if (lastDirection == noDirection) {
     //last contains the first vertex of the cycle
     lastDirection = dir;
    }
    if (lastDirection != dir) {
     //~next~ goes in another direction than ~lastDirection~
     //a new endpoint ~rp~ for the next halfsegment has to be created and the
     //next segment extends in the direction ~dir~
     rp = new Point(true, last->getX(), last->getY());
     lastDirection = dir;
    }

    if (rp != NULL) {
     // a new halfsegment has to be created.
     if (*lp < *rp) {
      hs = new HalfSegment(true, *lp, *rp);
      hs->attr.edgeno = edgeNo;
      hs->attr.insideAbove = lastInsideAbove;
      tmpRegion += *hs;
      hs->SetLeftDomPoint(false);
      tmpRegion += *hs;
      edgeNo++;
     } else {
      hs = new HalfSegment(true, *rp, *lp);
      hs->attr.edgeno = edgeNo;
      hs->attr.insideAbove = lastInsideAbove;
      tmpRegion += *hs;
      hs->SetLeftDomPoint(false);
      tmpRegion += *hs;
      edgeNo++;
     }
     lastInsideAbove = insideAbove;
     lp = rp;
     rp = NULL;
    }

    next->removeEdge(last);
    last->removeEdge(next);

    if ((*last != *firstVertex) && (last->getNoOfEdges() < 2)) {
     cutLooseEnd(last);
    }
    last = next;
   } while (*last != *firstVertex);

   rp = new Point(true, firstX, firstY);
   hs = new HalfSegment(true, *rp, *lp);
   hs->attr.edgeno = edgeNo;
   hs->attr.insideAbove = true;
   tmpRegion += *hs;
   hs->SetLeftDomPoint(false);
   tmpRegion += *hs;
   edgeNo++;

   if (last->getNoOfEdges() < 2) {
    cutLooseEnd(last);
   }

   //build region
   tmpRegion.EndBulkLoad();
   if (tmpRegion.IsDefined()) {

    if (result.Size() == 0) {
     //~tmpRegion~ was the first cycle
     result = tmpRegion;
    } else {
     //there are already some cycles in ~result~
     if (tmpRegion.Inside(result)) {
      //~tmpRegion~ is within ~result~ and becomes a hole in ~result~
      Region res(0);
      result.Minus(tmpRegion, res);
      result = res;
     } else {
      //~tmpRegion~ and ~result~ are merged
      Region res(0);
      result.Union(tmpRegion, res);
      result = res;
     }
    }
   }
  } catch (exception& e) {
   cutLooseEnds();
  }
 }

 //remove the remaining vertices and edges
 clear();
}

Vertex* CoarseningGraph::insert(int x, int y) {
 Vertex* v = new Vertex(x, y);
 pair<set<Vertex*, CmpVertex>::iterator, bool> pair1 = vertices.insert(v);
 return *(pair1.first);
}

void CoarseningGraph::Print() const {
 for (set<Vertex*, CmpVertex>::iterator i = vertices.begin();
   i != vertices.end(); i++) {
  (*i)->Print();
 }
}

void CoarseningGraph::clear() {
 set<Vertex*, CmpVertex>::iterator it = vertices.begin();
 while (it != vertices.end()) {
  Vertex* v = *it;
  v->removeEdges();
  vertices.erase(it);
  delete v;
  it++;
 }
}

void CoarseningGraph::cutLooseEnds() {
 set<Vertex*, CmpVertex>::iterator current = vertices.begin();
 while (current != vertices.end()) {
  Vertex* curVertex = *current;
  if (curVertex->getNoOfEdges() == 1) {
   // loose end found
   Vertex* endVertex = curVertex;
   Vertex* v = endVertex->getLastNeighbor();

   v = (*(vertices.find(v)));
   v->removeEdge(endVertex);
   vertices.erase(current);
   delete endVertex;
   while (v->getNoOfEdges() == 1) {
    endVertex = v;
    v = endVertex->getLastNeighbor();
    v = (*(vertices.find(v)));
    v->removeEdge(endVertex);
    vertices.erase(vertices.find(endVertex));
    delete endVertex;
   }
   if (v->getNoOfEdges() == 0) {
    vertices.erase(vertices.find(v));
    delete v;
   }
   current = vertices.begin();
  } else {
   current++;
  }
 }
}

void CoarseningGraph::cutLooseEnd(Vertex* v) {
 if (v->getNoOfEdges() == 1) {
  // loose end found
  Vertex* looseEnd = v;
  Vertex* next = looseEnd->getLastNeighbor();
  next = (*(vertices.find(next)));
  next->removeEdge(looseEnd);
  vertices.erase(vertices.find(looseEnd));
  delete looseEnd;

  while (next->getNoOfEdges() == 1) {
   looseEnd = next;
   next = looseEnd->getLastNeighbor();
   next = (*(vertices.find(next)));
   next->removeEdge(looseEnd);
   vertices.erase(vertices.find(looseEnd));
   delete looseEnd;
  }

  if (next->getNoOfEdges() == 0) {
   vertices.erase(vertices.find(next));
   delete next;
  }
 } else {
  if (v->getNoOfEdges() == 0) {
   vertices.erase(vertices.find(v));
   delete v;
  }
 }
}

Vertex::Vertex(int a, int b) :
  x(a), y(b), visited(0) {
}

Vertex::Vertex(const Vertex& v) :
  x(v.x), y(v.y), visited(v.visited), edgeSet(v.edgeSet) {
}

Vertex::~Vertex() {
}
;

Vertex& Vertex::operator=(const Vertex& v) {
 x = v.getX();
 y = v.getY();
 visited = v.getNoVisited();
 edgeSet = v.getEdges();
 return *this;
}

bool Vertex::operator!=(const Vertex& v) const {
 if (x != v.getX()) {
  return true;
 }
 if (y != v.getY()) {
  return true;
 }
 return false;
}

bool Vertex::operator<(const Vertex& v) const {
 if (x < v.getX()) {
  return true;
 }
 if (x == v.getX() && y < v.getY()) {
  return true;
 }
 return false;
}

void Vertex::addEdge(Vertex* v) {
 edgeSet.insert(v);
}

set<Vertex*, CmpVertex> Vertex::getEdges() const {
 set<Vertex*, CmpVertex> edges;
 for (set<Vertex*, CmpVertex>::iterator i = edgeSet.begin(); i != edgeSet.end();
   i++) {
  edges.insert(*i);
 }
 return edges;
}

Vertex* Vertex::getLastNeighbor() {
 if (edgeSet.size() != 1) {
  assert(false);
 }
 Vertex* v = *(edgeSet.begin());
 return v;
}

void Vertex::removeEdge(Vertex* v) {
 set<Vertex*, CmpVertex>::iterator it1 = edgeSet.begin();
 while ((it1 != edgeSet.end()) && (*v != *(*(it1)))) {
  it1++;
 }
 if (it1 == edgeSet.end()) {
  cerr << "error: vertex";
  Print();
  cerr << endl << " has no edge to ";
  v->print();
  assert(false);
 } else {
  edgeSet.erase(it1);
 }
}

void Vertex::removeEdges() {
 for (set<Vertex*, CmpVertex>::iterator it = edgeSet.begin();
   it != edgeSet.end(); it++) {
  Vertex* v = *it;
  v->removeEdge(this);
  edgeSet.erase(it);
 }
}

void Vertex::getNextVertex(Vertex** next, Direction& d, bool& insideAbove,
  int xStart, int yStart, int& visited) {

 bool wrongEdgeFound = false;

 switch (d) {
 case noDirection: {
  if (hasEdgePointingToTheLeft(next)) {
   d = left;
   insideAbove = !insideAbove;
  } else {
   if (hasEdgePointingUpwards(next)) {
    d = up;
   } else {
    if (hasEdgePointingToTheRight(next)) {
     d = right;
     insideAbove = !insideAbove;
    }
   }
  }
  break;
 }
 case up: {
  if (hasEdgePointingToTheLeft(next)) {
   d = left;
   insideAbove = !insideAbove;
   if (getNoOfEdges() != 2) {
    return;
   } else {
    //there might be a connection either to a loose end or to
    //another circle which has no path to the startpoint.
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    wrongEdgeFound = true;
    visited++;
   }
  }
  if (hasEdgePointingUpwards(next)) {
   if (wrongEdgeFound || (getNoOfEdges() != 2)) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    visited++;
   }
  }
  if (hasEdgePointingToTheRight(next)) {
   d = right;
  }
  break;
 }
 case down: {
  if (hasEdgePointingToTheRight(next)) {
   d = right;
   insideAbove = !insideAbove;
   if (getNoOfEdges() != 2) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    wrongEdgeFound = true;
    visited++;
   }
  }
  if (hasEdgePointingDownwards(next)) {
   if (wrongEdgeFound || (getNoOfEdges() != 2)) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    visited++;
   }
  }
  if (hasEdgePointingToTheLeft(next)) {
   d = left;
  }
  break;
 }
 case left: {
  if (hasEdgePointingDownwards(next)) {
   d = down;
   if (getNoOfEdges() != 2) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    wrongEdgeFound = true;
    visited++;
   }
  }
  if (hasEdgePointingToTheLeft(next)) {
   if (wrongEdgeFound || (getNoOfEdges() != 2)) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    visited++;
   }
  }
  if (hasEdgePointingUpwards(next)) {
   d = up;
   insideAbove = !insideAbove;
  }

  break;
 }
 case right: {
  if (hasEdgePointingUpwards(next)) {
   d = up;
   if (getNoOfEdges() != 2) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    wrongEdgeFound = true;
    visited++;
   }
  }
  if (hasEdgePointingToTheRight(next)) {
   if (wrongEdgeFound || (getNoOfEdges() != 2)) {
    return;
   } else {
    setNoVisited(visited);
    if ((*next)->hasAConnectionTo(xStart, yStart, visited)) {
     visited++;
     return;
    }
    visited++;
   }
  }
  if (hasEdgePointingDownwards(next)) {
   d = down;
   insideAbove = !insideAbove;
  }
  break;
 }
 default: {
  assert(false);
 }
 }
}

bool Vertex::hasEdgePointingUpwards(Vertex** next) {
 bool found = false;
 *next = NULL;
 set<Vertex*, CmpVertex>::iterator it1 = edgeSet.begin();
 while ((it1 != edgeSet.end()) && !found) {
  if (isBelow(*(*it1))) {
   *next = *it1;
   found = true;
  } else {
   it1++;
  }
 }
 return found;
}

bool Vertex::hasEdgePointingDownwards(Vertex** next) {
 bool found = false;
 *next = NULL;
 set<Vertex*, CmpVertex>::iterator it1 = edgeSet.begin();
 while ((it1 != edgeSet.end()) && !found) {
  if (isAbove(*(*it1))) {
   *next = *it1;
   found = true;
  } else {
   it1++;
  }
 }
 return found;
}

bool Vertex::hasEdgePointingToTheLeft(Vertex** next) {
 bool found = false;
 *next = NULL;
 set<Vertex*, CmpVertex>::iterator it1 = edgeSet.begin();
 while ((it1 != edgeSet.end()) && !found) {
  if (isRightOf(*(*it1))) {
   *next = *it1;
   found = true;
  } else {
   it1++;
  }
 }
 return found;
}

bool Vertex::hasEdgePointingToTheRight(Vertex** next) {
 bool found = false;
 *next = NULL;
 set<Vertex*, CmpVertex>::iterator it1 = edgeSet.begin();
 while ((it1 != edgeSet.end()) && !found) {
  if (isLeftOf(*(*it1))) {
   *next = *it1;
   found = true;
  } else {
   it1++;
  }
 }
 return found;
}

bool Vertex::isBelow(const Vertex& v) {
 if ((x == v.getX()) && (y < v.getY())) {
  return true;
 }
 return false;
}

bool Vertex::isAbove(const Vertex& v) {
 if ((x == v.getX()) && (y > v.getY())) {
  return true;
 }
 return false;
}

bool Vertex::isRightOf(const Vertex& v) {
 if ((y == v.getY()) && (x > v.getX())) {
  return true;
 }
 return false;
}

bool Vertex::isLeftOf(const Vertex& v) {
 if ((y == v.getY()) && (x < v.getX())) {
  return true;
 }
 return false;
}

bool Vertex::hasAConnectionTo(int xStart, int yStart, int& visited) {
 if (getNoVisited() < visited) {
  //~this~ was not visited
  //
  if ((getX() == xStart && (getY() == yStart))) {
   //connection found to the startpoint
   return true;
  }
  setNoVisited(visited);
  set<Vertex*, CmpVertex>::iterator it = edgeSet.begin();
  while (it != edgeSet.end()) {
   Vertex* neighbor = *it;
   //look for a connection to the startpoint via the neighbors of ~this~
   if (neighbor->hasAConnectionTo(xStart, yStart, visited)) {
    return true;
   }
   it++;
  }
 }
 return false;
}

void Vertex::print() const {
 cout << " ( " << x << ", " << y << " ) ";
}

void Vertex::Print() const {
 cout << "( " << x << ", " << y << " ) with " << edgeSet.size() << " edges to:"
   << endl;
 for (set<Vertex*, CmpVertex>::iterator i = edgeSet.begin(); i != edgeSet.end();
   i++) {
  (*i)->print();
 }
 cout << endl << "-----------------------------" << endl;
}

bool CmpVertex::operator()(Vertex* v1, Vertex* v2) const {
 return ((*v1) < (*v2));
}

void coarseRegion2(/*const*/Region2& r, Region& result) {
 //faces
 CoarseningGraph g1;
 Region2 faces2(0);
 r.getFaces(faces2);
 g1.computeGraph(faces2);

 ::Region faces(0);
 g1.calculateBoundary(faces);

 //holes
 CoarseningGraph g2;
 Region2 holes2(0);
 r.getHoles(holes2);
 g2.computeGraph(holes2);

 ::Region holes(0);
 g2.calculateBoundary(holes);

 result.Clear();
 faces.Minus(holes, result);
}

}  //end of namespace p2d
