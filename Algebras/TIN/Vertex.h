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

#ifndef VERT_H_
#define VERT_H_

#include <iostream>
#include "Vector2D.h"
#include "Vector3D.h"
#include "TinHelper.h"
#include "SecondoDependencies.h"

namespace tin {
class Vertex;

class VertexContainerSet;

class Vertex {
private:
 VERTEX_COORDINATE x;
 VERTEX_COORDINATE y;
 VERTEX_Z z;
public:
 static const Vertex nullvertex;
public:
////////Construction/Destruction/////////////////////////////////
 Vertex() {
  x = 0;
  y = 0;
  z = 0;
 }
 ;
 Vertex(VERTEX_COORDINATE ix, VERTEX_COORDINATE iy, VERTEX_Z iz) {
  if (iz == ERROR_VALUE)
   throw(E_VERTEX_CONSTRUCTOR);
  z = iz;
  x = ix;
  y = iy;

 }
 explicit Vertex(const Point_p& p) {
  x = p.x;
  y = p.y;
  z = 0;
 }
 Vertex(PreciseDouble ix, PreciseDouble iy, VERTEX_Z iz) {
  if (iz == ERROR_VALUE)
   throw(E_VERTEX_CONSTRUCTOR);
  z = iz;
  x = ix.l;
  y = iy.l;
 }
 Vertex(mpq_t& ix, mpq_t& iy, VERTEX_Z iz) {
  if (iz == ERROR_VALUE)
   throw(E_VERTEX_CONSTRUCTOR);

  x = mpq_get_d(ix);
  y = mpq_get_d(iy);
  z = iz;

 }
 ~Vertex() {

 }

////////Manipulation/////////////////////////////////////////////
 void setX(VERTEX_COORDINATE x) {
  this->x = x;
 }
 void setY(VERTEX_COORDINATE y) {
  this->y = y;
 }
 void setZ(VERTEX_Z iz) {
  if (iz == ERROR_VALUE)
   throw(E_VERTEX_CONSTRUCTOR);

  this->z = iz;
 }

///////Query/////////////////////////////////////////////////////
 static int compareIndex(const void * pv1, const void * ppv2) {
  if (*(Vertex*) pv1 < **(Vertex**) ppv2)
   return -1;
  if (*(Vertex*) pv1 == **(Vertex**) ppv2)
   return 0;

  return 1;
 }
 Vector2D minus2D(const Vertex& v) const;
 Vector2D_mp minus2D_mp(const Vertex& v) const;
 Vector3D minus3D(const Vertex& v) const;
 Vector2D operator -(const Point& p) const;
 Point operator +(const Vector2D& v) const;
 static int compareByX(const void * v1, const void * v2);
 bool operator <(const Vertex& v) const;
 bool smaller3D(const Vertex * v) const;
 bool operator ==(const Vertex& v) const;
 bool operator ==(const Point_p& p) const;
 bool equal3D(const Vertex & v) const;

 inline VERTEX_COORDINATE getX() const {
  return x;
 }

 inline VERTEX_COORDINATE getY() const {
  return y;
 }

 inline VERTEX_Z getZ() const {
  return z;
 }

///////Presentation//////////////////////////////////////////////
 void print(std::ostream& out = std::cout) const;
#ifndef UNIT_TEST
 static Vertex* parseVertex(ListExpr l, Vertex * place = 0);
 static void parseVertex(ListExpr l, VertexContainerSet& vc);
 ListExpr outVertex() const;

///////Persistence///////////////////////////////////////////////
 void serialize(char* storage, size_t& offset) const;
 void rebuild(char* state, size_t & offset);
 bool open(SmiRecord& valueRecord);
 bool save(SmiRecord& valueRecord);
#endif
 void putSTLbinaryRepresentation(void * buff, uint32_t & offset) const;
 static TIN_SIZE getSizeOnDisc();

/////////////////////////////////////////////////////////////////

};

} /* namespace tin*/
#endif /* VERT_H_*/
