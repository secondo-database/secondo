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

#include "Vertex.h"
#include "SecondoDependencies.h"
#include "VertexContainerSet.h"
#include <iomanip>
namespace tin {

const Vertex Vertex::nullvertex = Vertex(0, 0, 0);

Vector3D Vertex::minus3D(const Vertex& v) const {
 return Vector3D(x - v.x, y - v.y, z - v.z);
}

Vector2D_mp Vertex::minus2D_mp(const Vertex& v) const {
 return Vector2D_mp(v, *this);
}
Vector2D Vertex::minus2D(const Vertex& v) const {
 return Vector2D(x - v.x, y - v.y);
}
Vector2D Vertex::operator-(const Point &p) const {

 VECTOR_COMPONENT dx = (-p.x) + x;
 VECTOR_COMPONENT dy = (-p.y) + y;

 return Vector2D(dx, dy);
}
Point Vertex::operator +(const Vector2D& v) const {

 VECTOR_COMPONENT rx = v.getDx() + x;
 VECTOR_COMPONENT ry = v.getDy() + y;

 return Point(rx, ry);
}
bool Vertex::operator ==(const Vertex& v) const {
 return (v.x == x && v.y == y);
}
bool Vertex::operator ==(const Point_p& p) const {
 return (p.x == x && p.y == y);
}
bool Vertex::equal3D(const Vertex& v) const {
 return (v.x == x && v.y == y && v.z == z);
}
void Vertex::print(std::ostream & out) const {
 out << "\t" << std::setprecision(15) << x << "\t" << std::setprecision(15)
   << y << "\t" << std::setprecision(15) << z;
}
void Vertex::putSTLbinaryRepresentation(void * buff,
  uint32_t & offset) const {

 *((float*) (buff + offset)) = (float) x;
 offset += sizeof(float);
 *((float*) (buff + offset)) = (float) y;
 offset += sizeof(float);
 *((float*) (buff + offset)) = (float) z;
 offset += sizeof(float);

}
#ifndef UNIT_TEST
bool Vertex::open(SmiRecord& valueRecord) {
 valueRecord.Read(this->x);
 valueRecord.Read(this->y);
 valueRecord.Read(this->z);
 return true;
}
void Vertex::rebuild(char* state, size_t & offset) {
 ReadVar<VERTEX_COORDINATE>(x, state, offset);
 ReadVar<VERTEX_COORDINATE>(y, state, offset);
 ReadVar<VERTEX_Z>(z, state, offset);
}
void Vertex::serialize(char* storage, size_t& offset) const {
 WriteVar<VERTEX_COORDINATE>(this->x, storage, offset);
 WriteVar<VERTEX_COORDINATE>(this->y, storage, offset);
 WriteVar<VERTEX_Z>(this->z, storage, offset);
}
bool Vertex::save(SmiRecord& valueRecord) {
 valueRecord.Write(this->x);
 valueRecord.Write(this->y);
 valueRecord.Write(this->z);

 return true;
}
#endif

bool Vertex::operator <(const Vertex& v) const {

 if (this->y < v.y)
  return true;
 if (this->y == v.y && this->x < v.x) {
  return true;
 }
 return false;

}
bool Vertex::smaller3D(const Vertex* v) const {

 if (this->y < v->y)
  return true;

 if (this->y == v->y && this->x < v->x) {
  return true;
 }

 if (this->y == v->y && this->x == v->x && this->z < v->z) {
  return true;
 }
 return false;

}
int Vertex::compareByX(const void * v1, const void * v2) {

 Vertex * pv1 = (*(Vertex**) v1);
 Vertex * pv2 = (*(Vertex**) v2);

 if (pv1->x < pv2->x)
  return -1;
 if (pv1->x == pv2->x) {
  return 0;
 }
 return 1;

}

#ifndef UNIT_TEST
ListExpr Vertex::outVertex() const {
 return nl->ThreeElemList(nl->RealAtom(x), nl->RealAtom(y),
   nl->RealAtom(z));
}
Vertex* Vertex::parseVertex(ListExpr l, Vertex * place) {

 VERTEX_COORDINATE x, y;
 VERTEX_Z z;

 if (nl->ListLength(l) != 3) {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX);
 }
 ListExpr atom = nl->First(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  x = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX2);
 }
 atom = nl->Second(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  y = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX3);
 }
 atom = nl->Third(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  z = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX4);
 }
 if (place == 0)
  return new Vertex(x, y, z);
 else
  return new (place) Vertex(x, y, z);
}
void Vertex::parseVertex(ListExpr l, VertexContainerSet& vc) {

 VERTEX_COORDINATE x, y;
 VERTEX_Z z;
 Vertex tmpvertex;
 bool newv;

 if (nl->ListLength(l) != 3) {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX);
 }
 ListExpr atom = nl->First(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  x = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX2);
 }
 atom = nl->Second(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  y = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX3);
 }
 atom = nl->Third(l);
 if (nl->IsAtom(atom)
   && (nl->AtomType(atom) == IntType || nl->AtomType(atom) == RealType)) {
  z = (
    nl->AtomType(atom) == IntType ?
      nl->IntValue(atom) : nl->RealValue(atom));
 } else {
  throw std::invalid_argument(E_VERTEX_PARSEVERTEX4);
 }

 tmpvertex.setX(x);
 tmpvertex.setY(y);
 tmpvertex.setZ(z);

 vc.insertVertex(&tmpvertex, newv);
}

#endif

TIN_SIZE Vertex::getSizeOnDisc() {
 return sizeof(VERTEX_COORDINATE) + sizeof(VERTEX_COORDINATE)
   + sizeof(VERTEX_Z);
}

}
