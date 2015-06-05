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

#ifndef VECTOR2D_H_
#define VECTOR2D_H_

#define OPPOSITE_QUADRANT 3
#define SAME_QUADRANT 0

#include "TinHelper.h"
#include "gmp.h"
#include "gmpxx.h"

namespace tin {

enum QUADRANT {
ONE = 0, TWO = 2, THREE = 3, FOUR = 1, NULLVECTOR = 4, VERTICAL = 8
};

class Vector2D {
protected:
VECTOR_COMPONENT dx;
VECTOR_COMPONENT dy;
public:

Vector2D() {
}
;
Vector2D(const Vector2D & v) {
dx = v.dx;
dy = v.dy;
}
Vector2D(const VECTOR_COMPONENT &idx, const VECTOR_COMPONENT &idy) {
dx = idx;
dy = idy;

}
~Vector2D() {

}
void print();
VECTOR_COMPONENT dydx();
Vector2D operator*(const VECTOR_COMPONENT &f) const;
Vector2D& operator=(const Vector2D& v);
bool hasEqualDir(Vector2D& v);
bool isVertical();
bool isHorizontal();
bool isNull();
bool isBetween(Vector2D * v1, Vector2D * v2);
VECTOR_COMPONENT getLength() {
VECTOR_COMPONENT result;
PreciseArithmetic::error_sqrt(dx * dx + dy * dy, result);

return result;
}
VECTOR_COMPONENT getDx() const {
return dx;
}

VECTOR_COMPONENT getDy() const {
return dy;
}

};

class Vector2D_mp {
protected:
mpq_class dx;
mpq_class dy;

public:
Vector2D_mp() {
}
;
Vector2D_mp(const Vertex& iv1, const Vertex& iv2);
Vector2D_mp(const VERTEX_COORDINATE val1, const VERTEX_COORDINATE val2);
Vector2D_mp(mpq_t idx, mpq_t idy);
Vector2D_mp(const Vector2D_mp & v);
virtual ~Vector2D_mp();

void print();

Vector2D_mp operator*(const VECTOR_COMPONENT &f) const;
Vector2D_mp& operator=(const Vector2D_mp& v);
bool hasEqualDir(Vector2D_mp& v);
bool isVertical();
bool isHorizontal();
bool isNull();
bool isBetween(Vector2D_mp * v1, Vector2D_mp * v2);
void dydx(mpq_class & result);
void getDx(mpq_class& ret) {
ret = dx;
}

void getDy(mpq_class& ret) {
ret = dy;
}
friend class Point_mp;
friend class Edge;

};

class Line {
protected:
Vector2D vec;
Point p;
public:
Line(const Vertex& v1, const Vertex& v2);
Line(const Vector2D & v, const Point & ip);

int getSide(const Point x);
};

class Line_mp: public SecureOperator {
protected:
Vector2D_mp vec;
Point_mp p;
public:
Line_mp(const Vertex& v1, const Vertex& v2);
Line_mp(const Vector2D_mp & v, const Point_mp & ip);
int getSide_mp(const Point_mp & x);

};
} /* namespace tin */
#endif /* VECTOR2D_H_ */
