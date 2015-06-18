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

#ifndef EDGE_H_
#define EDGE_H_
#include "Vertex.h"

namespace tin {

class Edge {
protected:
 const Vertex *v1;
 const Vertex *v2;
 static Vertex intersection_point;
 static Vertex intersection_point_2;

public:
 Edge(const Vertex *iv1, const Vertex * iv2);
 Edge() {
  v1 = 0;
  v2 = 0;
 }
 virtual ~Edge();
 bool intersection_sec(const Edge & e, Edge &result) const;
 bool intersects_sec(const Edge * e) const;
 bool intersection_mp(const Edge & e, Edge &result) const;
 bool intersects_mp(const Edge * e) const;
 int getSide_sec(const Point_p& p);

 bool contains(const Point & p) const;
 bool hasVertex(const Vertex *v) const;
 Point_p getMiddle() const;
 bool operator==(const Edge & e) const;
 bool equal3D(const Edge & e) const;
 bool isPoint() const;
 bool operator<(const Edge & e) const;
 static const Edge nulledge;
 bool isNull() const {
  return (v1 == 0 && v2 == 0);
 }
 Vector2D getVector2D() const;
 Vector2D_mp getVector2D_mp() const;
 VERTEX_Z getZat(const Point_p & p) const;

 const Vertex* getV1() const {
  return v1;
 }

 const Vertex* getV2() const {
  return v2;
 }

};

class NeighborEdge: public Edge {
protected:
 Triangle * n1;
 Triangle * n2;
public:
 NeighborEdge(const Vertex *iv1, const Vertex * iv2, Triangle * t1,
   Triangle * t2);

 NeighborEdge() {
  v1 = 0;
  v2 = 0;
  n1 = 0;
  n2 = 0;
 }
 Triangle* getN1();
 Triangle* getN2();

 void setN1(Triangle * t);
 void setN2(Triangle * t);

 NeighborEdge getNextEdge();

 NeighborEdge getPriorEdge();
 NeighborEdge getNextEdge_noload();

 NeighborEdge getPriorEdge_noload();

 bool operator==(const NeighborEdge& ne);
 bool operator==(const Edge& ne);

};

class Segment: public Edge {
protected:

public:
 Segment() {
  v1 = 0;
  v2 = 0;
 }
 Segment(const Point_p & p1, const Point_p & p2) {
  v1 = new Vertex(p1);
  v2 = new Vertex(p2);
 }
 Segment(const Vertex & iv1, const Point_p & p2) {
  v1 = new Vertex(iv1);
  v2 = new Vertex(p2);
 }
 Segment(const Vertex & iv1, const Vertex & iv2) {
  v1 = new Vertex(iv1);
  v2 = new Vertex(iv2);
 }
 ~Segment() {
  if (v1)
   delete v1;
  if (v2)
   delete v2;
 }

 Segment& operator=(const Segment& seg) {
  if (v1)
   delete v1;
  if (v2)
   delete v2;

  v1 = new Vertex(*seg.v1);
  v2 = new Vertex(*seg.v2);
  return *this;
 }
 Segment& operator=(const Edge& seg) {
  if (v1)
   delete v1;
  if (v2)
   delete v2;

  v1 = new Vertex(*seg.getV2());
  v2 = new Vertex(*seg.getV1());

  return *this;
 }
 void init(const Vertex & iv1, const Point_p & p2) {
  v1 = new Vertex(iv1);
  v2 = new Vertex(p2);
 }

};

} /* namespace tin*/
#endif /* EDGE_H_*/
