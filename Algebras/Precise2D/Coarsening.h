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

#ifndef COARSENING_H_
#define COARSENING_H_

#include <stdio.h>
#include <set>
#include "Algebra.h"
#include "RectangleAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "AttrType.h"
#include "Precise2DAlgebra.h"
#include "HalfSegment.h"
#include "SpatialAlgebra.h"
#include "Region2Algebra.h"
#include "Attribute.h"

namespace p2d {

class Vertex;

/*
 ~CmpVertex~

 This struct is used for a set which saves the vertices of the graph in
 a lexicographical order.

*/
struct CmpVertex {

 bool operator()(Vertex* v1, Vertex* v2) const;

};

/*
 ~Direction~

 ~Direction~ is used in the coarsening graph to describe the direction
 of the edges.

*/
enum Direction {
 up, down, left, right, noDirection
};

/*
 ~CoarseningGraph~

 With this graph we compute a ~region~-object from a ~region2~-object.
 The graph takes the grid-coordinates of the segments from the ~region2~-object
 and creates vertices and the edges between the vertices. With this graph we
 generate the ~region~-object by using the edges to calculate a circle as large
 as possible.

*/
class CoarseningGraph {
private:

 set<Vertex*, CmpVertex> vertices;

 void clear();

 /*
  ~cutLooseEnds~
  If only the grid coordinates are used sometimes an area degenerates to a
  blind ending  line. This method removes all vertices and edges, which
  belong to a such a loose end.

 */
 void cutLooseEnds();

 /*
  ~cutLooseEnd~

  If an edge from ~v~ to another vertex has been processed, it is removed
  from the graph. If ~v~ has only one edge left, ~v~ becomes a startpoint
  of a loose end, which has to be removed too.

 */
 void cutLooseEnd(Vertex* v);


public:
 CoarseningGraph();

 virtual ~CoarseningGraph();

 Vertex* insert(int x, int y);

 void computeGraph(/*const*/Region2& r);

 void computeGraphBetween(int lx, int ly, int rx, int ry);

 void calculateBoundary(Region& result);

 void Print() const;

};

/*
 ~Vertex~

 Saves the grid-coordinates and pointer to the adjacent vertices.

*/
class Vertex {
private:
 int x;
 int y;
 int visited;

 set<Vertex*, CmpVertex> edgeSet;

 /*
  ~hasEdgePointing...~

  ~next~ points to the requested vertex or will be NULL.

 */
 bool hasEdgePointingUpwards(Vertex** next);
 bool hasEdgePointingDownwards(Vertex** next);
 bool hasEdgePointingToTheLeft(Vertex** next);
 bool hasEdgePointingToTheRight(Vertex** next);

 /*
  ~is...~

  returns true, if ~v~ is in the requested direction to ~this~,
  false otherwise.
 */
 bool isBelow(const Vertex& v);
 bool isAbove(const Vertex& v);
 bool isRightOf(const Vertex& v);
 bool isLeftOf(const Vertex& v);

public:
 Vertex(int a, int b);

 Vertex(){};

 Vertex(const Vertex& v);

 ~Vertex();

 Vertex& operator=(const Vertex& v);

 int getX() const {
  return x;
 }

 int getY() const {
  return y;
 }

 void setNoVisited(int v){
  visited = v;
 }

 int getNoVisited() const{
  return visited;
 }

 bool operator!=(const Vertex& v) const;

 bool operator<(const Vertex& v) const;

 void addEdge(Vertex* v);

 int getNoOfEdges() const {
  return edgeSet.size();
 }

 set<Vertex*, CmpVertex> getEdges() const;

 Vertex* getLastNeighbor();

 void removeEdge(Vertex* v);

 void removeEdges();

 /*
  ~getNextVertex~
  ~next~ points to the next vertex, depending on the direction ~d~ of the last
  edge. ~d~ and ~insideAbove~ are updated.

 */
 void getNextVertex(Vertex** next, Direction& d, bool& insideAbove, int xStart,
   int yStart, int& visited);

/*
 ~hasAConnectionTo~

 looks for a path to the startpoint with the coordiate (~xStart~, ~yStart~).
 ~visited~ has to be incremented after each time the method was called.

*/
 bool hasAConnectionTo(int xStart, int yStart, int& visited);

 void print() const;
 void Print() const;

};

/*
 ~coarseRegion2~

 This method generates a CoarseningGraph from ~r~ and computes the
 region-object, saved in ~result~.

*/
void coarseRegion2(/*const*/Region2& r, Region& result);

} //end namespace p2d
#endif /* COARSENING_H_ */
