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
#include <gmp.h>
#include <gmpxx.h>
#include <set>
#include <utility>
#include <exception>
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
#include "Toolbox.h"

namespace p2d {

class Vertex;

/*
1 Class CmpVertex

 This struct is used for a set which saves the vertices of the graph in
 a lexicographical order.

*/
struct CmpVertex {

 bool operator()(Vertex* v1, Vertex* v2) const;

};

/*
2 Direction

 This enumeration type is used in the coarsening graph to describe the
 direction of the edges.

*/
enum Direction {
 up, down, left, right, noDirection
};

/*
3 Class CoarseningGraph

 With this graph we compute a ~region~-object from a ~region2~-object.
 The graph takes the grid-coordinates of the segments from the ~region2~-object
 and creates vertices and the edges between the vertices. With this graph we
 generate the ~region~-object by using the edges to calculate a circle as large
 as possible.

*/
class CoarseningGraph {
private:

 set<Vertex*, CmpVertex> vertices;

/*
1.1 ~clear~

  removes all vertices and edge.

*/
 void clear();

/*
1.1 ~removeIsleNodes~

  There might be some vertices without an edge. These vertices have to be
  removed.

*/
 void removeIsleNodes();

/*
1.1 ~removeIsleNode~

  The vertex ~v~ has no edge and have to be removed.

*/
 void removeIsleNode(Vertex* v);

public:
/*
1.1 Constructor and destructor

*/
 CoarseningGraph();

 virtual ~CoarseningGraph();

/*
1.1 ~insert~

 adds a new vertex with the coordinate (~x~, ~y~) to the graph.

*/
 Vertex* insert(int x, int y);

/*
1.1 ~computeGraph~

  computes a new graph from the given ~Region2~-object.

*/
 void computeGraph(/*const*/Region2& r);

/*
1.1 ~computeGraphBetween~

 adds all needed vertices and edges from (~lx~, ~ly~) to (~rx~, ~ry~)
 to the graph.

*/
 void computeGraphBetween(int lx, int ly, int rx, int ry);

/*
1.1 ~calculateBoundary~

  creates a region-object from the coarseningGraph. The intermediate
  results are of type region too.

*/
 void calculateBoundary(Region& result);

/*
1.1 ~calculateBoundary2~

  creates a region-object from the coarseningGraph. The intermediate
  results are of type region2.

*/
 void calculateBoundary2(Region2& result);

/*
1.1 ~Print~

*/
 void Print() const;

};

/*
1 Class Vertex

 A Vertex-object saves the grid-coordinates and pointer to the adjacent
 vertices.

*/
class Vertex {
private:
 int x;
 int y;

 set<Vertex*, CmpVertex> edgeSet;

/*
1.1 ~hasEdgePointing...~

  ~next~ points to the requested vertex or will be NULL.

*/
 bool hasEdgePointingUpwards(Vertex** next);
 bool hasEdgePointingDownwards(Vertex** next);
 bool hasEdgePointingToTheLeft(Vertex** next);
 bool hasEdgePointingToTheRight(Vertex** next);

/*
1.1 ~is...~

  returns true, if ~v~ is in the requested direction to ~this~,
  false otherwise.

*/
 bool isBelow(const Vertex& v);
 bool isAbove(const Vertex& v);
 bool isRightOf(const Vertex& v);
 bool isLeftOf(const Vertex& v);

public:

/*
1.1 Constructors and destructor

*/
 Vertex(int a, int b);

 Vertex() {};

 Vertex(const Vertex& v);

 ~Vertex();

/*
1.1 ~=~

*/
 Vertex& operator=(const Vertex& v);

/*
1.1 getter

*/
 int getX() const {
  return x;
 }

 int getY() const {
  return y;
 }

/*
1.1 Comparison operators

*/
 bool operator!=(const Vertex& v) const;

 bool operator<(const Vertex& v) const;

/*
1.1 ~addEdge~

 adds a new edge to ~v~ to the vertex.

*/
 bool addEdge(Vertex* v);

/*
1.1 ~getNoOfEdges~

 returns the number of edges.

*/
 int getNoOfEdges() const {
  return edgeSet.size();
 }

/*
1.1 ~getNoOfEdges~

  returns all edges.

*/
 set<Vertex*, CmpVertex> getEdges() const;

/*
1.1 ~removeEdge~

  removes ~v~ from the set of edges.

*/
 void removeEdge(Vertex* v);

/*
1.1 ~removeEdge~

  removes all edges.

*/
 void removeEdges();

/*
1.1 ~getNextVertex~
  ~next~ points to the next vertex, depending on the direction ~d~ of the last
  edge. ~d~ and ~insideAbove~ are updated.

*/
 void getNextVertex(Vertex** next, Direction& d, bool& insideAbove);

/*
1.1 ~Print~

  ~print~ prints only the coordinates of ~this~. ~Print~ prints the
  coordinates and the coordinates of all edges.

*/
 void print() const;
 void Print() const;

};

/*
1 ~coarseRegion2~

 This method generates a CoarseningGraph from ~r~ and computes the
 region-object, saved in ~result~.

*/
void coarseRegion2(/*const*/Region2& r, Region& result);

/*
1 ~coarseRegion2b~

 This method generates a CoarseningGraph from ~r~ and computes the
 region-object, saved in ~result~. The intermediate results are all of
 type region2.

*/
void coarseRegion2b(/*const*/Region2& r, Region& result);

/*
1 ~createRegion~

 precondition: there are no decimals

*/
void createRegion(Region2& s, Region& r);

} //end namespace p2d
#endif/* COARSENING_H_*/
