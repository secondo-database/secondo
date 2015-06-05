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

#include "Vector2D.h"
#include "math.h"
#include "Vertex.h"
#include "gmpxx.h"

namespace tin {

VECTOR_COMPONENT Vector2D::dydx() {
if (isVertical() && dy > 0)
return VERTICAL_UP;
if (isVertical() && dy < 0)
return VERTICAL_DOWN;

if (isHorizontal())
return (VECTOR_COMPONENT) 0;

return dy / dx;
}

void Vector2D::print() {

//TODO std::cout<<"Vector2D \t dx:"<<dx<<" \t dy:"<< dy;
}

bool Vector2D::hasEqualDir(Vector2D& v) {
if (isVertical()) {
if (v.isVertical())
return true;
else
return false;
}

if (v.isVertical()) {
if (isVertical())
return true;
else
return false;
}

if (isHorizontal()) {
if (v.isHorizontal())
return true;
else
return false;
}

if (v.isHorizontal()) {
if (isHorizontal())
return true;
else
return false;
}

return (absolute(dydx()) == absolute(v.dydx())
&& ((std::signbit(v.dx.l) == std::signbit(dx.l)
&& std::signbit(v.dy.l) == std::signbit(dy.l))
|| (std::signbit(v.dx.l) != std::signbit(dx.l)
&& std::signbit(v.dy.l) != std::signbit(dy.l))));
}

bool Vector2D::isVertical() {
return (dx == 0);
}

bool Vector2D::isHorizontal() {
return (dy == 0);
}

bool Vector2D::isNull() {
if (dx == 0 && dy == 0)
return true;
else
return false;
}

Vector2D& Vector2D::operator=(const Vector2D& v) {
dx = v.dx;
dy = v.dy;

return *this;

}
Vector2D Vector2D::operator*(const VECTOR_COMPONENT & f) const {

VECTOR_COMPONENT resultx = dx * f;
VECTOR_COMPONENT resulty = dy * f;

return Vector2D(resultx, resulty);
}

bool Vector2D::isBetween(Vector2D* v1, Vector2D* v2) {

if (this->isNull() || v1->isNull() || v2->isNull())
throw std::invalid_argument(E_VECTOR2D_ISBETWEEN0);

Point nullpoint(0, 0);
Point p2(v2->getDx(), v2->getDy());
Point pme(this->getDx(), this->getDy());

Line l1(*v1, nullpoint);
Line l2(*v2, nullpoint);
Line lme(*this, nullpoint);

int correctside1 = l1.getSide(p2);

if ((l1.getSide(pme) == correctside1 || l1.getSide(pme) == 0)
&& (l2.getSide(pme) == -correctside1 || l2.getSide(pme) == 0)) {
return true;
} else {
return false;
}

}
Vector2D_mp::Vector2D_mp(const Vector2D_mp & v) {

dx = v.dx;
dy = v.dy;

}
Vector2D_mp::Vector2D_mp(const VERTEX_COORDINATE val1,
const VERTEX_COORDINATE val2) :
dx(val1), dy(val2) {

}
Vector2D_mp::Vector2D_mp(const Vertex& iv1, const Vertex& iv2) {

mpq_class v1x(iv1.getX()), v2x(iv2.getX()), v1y(iv1.getY()), v2y(
iv2.getY());
dx = v2x - v1x;
dy = v2y - v1y;

}

Vector2D_mp::Vector2D_mp(mpq_t idx, mpq_t idy) :
dx(idx), dy(idy) {
}

Vector2D_mp::~Vector2D_mp() {
}

void Vector2D_mp::print() {

std::cout << "Vector2D \t dx:" << dx.get_d() << " \t dy:" << dy.get_d();
}

bool Vector2D_mp::hasEqualDir(Vector2D_mp& v) {
if (isVertical()) {
if (v.isVertical())
return true;
else
return false;
}

if (v.isVertical()) {
if (isVertical())
return true;
else
return false;
}

if (isHorizontal()) {
if (v.isHorizontal())
return true;
else
return false;
}

if (v.isHorizontal()) {
if (isHorizontal())
return true;
else
return false;
}

mpq_class i, i2;
dydx(i);
v.dydx(i2);
i = abs(i);
i2 = abs(i2);
//TODO check 0
bool ret = ((cmp(i, i2) == 0)
&& (sgn(dx) == sgn(v.dx) && sgn(dy) == sgn(v.dy)))
|| (sgn(dx) != sgn(v.dx) && sgn(dy) != sgn(v.dy));

return ret;
}

bool Vector2D_mp::isVertical() {
return sgn(dx) == 0 && sgn(dy) != 0;
}

bool Vector2D_mp::isHorizontal() {
return sgn(dx) != 0 && sgn(dy) == 0;
}

bool Vector2D_mp::isNull() {
return sgn(dx) == 0 && sgn(dy) == 0;
}

Vector2D_mp Vector2D_mp::operator*(const VECTOR_COMPONENT &f) const {

Vector2D_mp vn;

mpq_class fq(f.l);

vn.dx = dx * fq;
vn.dy = dy * fq;

return vn;

}

Vector2D_mp& Vector2D_mp::operator=(const Vector2D_mp& v) {

dx = v.dx;
dy = v.dy;
return *this;

}
void Vector2D_mp::dydx(mpq_class & result) {
result = dy / dx;
}
bool Vector2D_mp::isBetween(Vector2D_mp * v1, Vector2D_mp * v2) {

if (this->isNull() || v1->isNull() || v2->isNull())
throw std::invalid_argument(E_VECTOR2D_ISBETWEEN0);

Point_mp nullpoint(0, 0);
Point_mp p2;

mpq_set(p2.x, v2->dx.get_mpq_t());
mpq_set(p2.y, v2->dy.get_mpq_t());

Point_mp pme;

mpq_set(pme.x, this->dx.get_mpq_t());
mpq_set(pme.y, this->dy.get_mpq_t());

Line_mp l1(*v1, nullpoint);
Line_mp l2(*v2, nullpoint);
Line_mp lme(*this, nullpoint);

int correctside1 = l1.getSide_mp(p2);

if ((l1.getSide_mp(pme) == correctside1 || l1.getSide_mp(pme) == 0)
&& (l2.getSide_mp(pme) == -correctside1 || l2.getSide_mp(pme) == 0)) {
return true;
} else {
return false;
}

}

Line::Line(const Vector2D & v, const Point & ip) {
vec = v;

if (vec.isNull())
throw std::invalid_argument(E_LINE_LINE);

p = ip;
}
Line::Line(const Vertex& v1, const Vertex& v2) {
vec = v2.minus2D(v1);
p = v1;
}
int Line::getSide(const Point lp) {

if (vec.getDx() == 0) {

if (lp.x == p.x) {

return 0;
}
if (lp.x > p.x) {
if (vec.getDy() < 0)
return -1;
if (vec.getDy() > 0)
return 1;
}

if (lp.x < p.x) {
if (vec.getDy() < 0)
return 1;
if (vec.getDy() > 0)
return -1;

}

}

PreciseDouble side =
(lp.y - p.y - (lp.x * vec.dydx()) + (p.x * vec.dydx()))
/ (-(vec.dydx() * vec.getDy() + vec.getDx()));

if (side == 0)
return 0;
if (side < 0)
return -1;
else
return 1;
}

Line_mp::Line_mp(const Vertex& v1, const Vertex& v2) {
vec = v2.minus2D_mp(v1);
p = v1;
}
Line_mp::Line_mp(const Vector2D_mp & v, const Point_mp & ip) {
vec = v;
p = ip;
}

int Line_mp::getSide_mp(const Point_mp & lp) {

mpq_class dx;
mpq_class dy;
vec.getDx(dx);
vec.getDy(dy);


if (sgn(dx) == 0) {

if (mpq_cmp(lp.x, p.x) == 0) {

return 0;
}
if (mpq_cmp(lp.x, p.x) > 0) {
if (sgn(dy) < 0)
return -1;
if (sgn(dy) > 0)
return 1;
}

if (mpq_cmp(lp.x, p.x) < 0) {
if (sgn(dy) < 0)
return 1;
if (sgn(dy) > 0)
return -1;

}

}

mpq_class side = ((mpq_class) lp.y - (mpq_class) p.y
- ((mpq_class) lp.x * (dy / dx)) + ((mpq_class) p.x * (dy / dx)))
/ (-((dy / dx) * dy + dx));


if (sgn(side) == 0)
return 0;
if (sgn(side) < 0)
return -1;
else
return 1;

}

} /* namespace tin */
