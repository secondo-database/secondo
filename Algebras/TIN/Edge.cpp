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

#include "Edge.h"
#include "TinHelper.h"
#include "Vector2D.h"
#include "Vertex.h"
#include "Triangle.h"

#define CLEAN_UP_INTERSECTS mpq_clear(tmp);\
mpq_clear(tmp2);\
mpq_clear(tmp3);\
mpq_clear(f1);\
mpq_clear(vec1dx);\
mpq_clear(vec1dy);\
mpq_clear(p1x);\
mpq_clear(p1y);\
mpq_clear(f2);\
mpq_clear(vec2dx);\
mpq_clear(vec2dy);\
mpq_clear(p2x);\
mpq_clear(p2y);

namespace tin {
Vertex Edge::intersection_point = Vertex(0, 0, 0);
Vertex Edge::intersection_point_2 = Vertex(0, 0, 0);
const Edge Edge::nulledge = Edge();
Edge::Edge(const Vertex* iv1, const Vertex* iv2) {
if (iv1 == 0 || iv2 == 0)
throw std::invalid_argument(E_EDGE_EDGE0);
v1 = iv1;
v2 = iv2;
}

Edge::~Edge() {

}
NeighborEdge::NeighborEdge(const Vertex *iv1, const Vertex * iv2,
Triangle * t1, Triangle * t2) :
Edge(iv1, iv2) {
n1 = t1;
n2 = t2;
}
void NeighborEdge::setN1(Triangle * t) {
n1 = t;
}
void NeighborEdge::setN2(Triangle * t) {
n2 = t;
}

Triangle* NeighborEdge::getN1() {
return n1;
}

Triangle* NeighborEdge::getN2() {
return n2;
}

NeighborEdge NeighborEdge::getNextEdge() {
if (n2 != VORONOI_OPEN_END && n2) {
return n2->getNextEdge(*this);
} else
return NeighborEdge();
}
NeighborEdge NeighborEdge::getPriorEdge() {
if (n1 != VORONOI_OPEN_END && n1) {
return n1->getNextEdge(*this);
} else
return NeighborEdge();
}
NeighborEdge NeighborEdge::getNextEdge_noload() {
if (n2 != VORONOI_OPEN_END && n2) {

return n2->getNextEdge_noload(*this);
} else
return NeighborEdge();
}
NeighborEdge NeighborEdge::getPriorEdge_noload() {
if (n1 != VORONOI_OPEN_END && n1) {
return n1->getNextEdge_noload(*this);
} else
return NeighborEdge();
}

bool NeighborEdge::operator==(const NeighborEdge& ne) {
return Edge::operator ==(ne);
}
bool NeighborEdge::operator==(const Edge& ne) {
return ((*v1 == *(ne.getV1()) && *v2 == *(ne.getV2()))
|| (*v1 == *(ne.getV2()) && *v2 == *(ne.getV2())));
}

bool Edge::contains(const Point & p) const {
if (isPoint())
return p.x == v1->getX() && p.y == v1->getY();

Vector2D vec1 = v1->minus2D(*v2);

bool inxrange = ((p.x <= v1->getX() && p.x >= v2->getX())
|| (p.x >= v1->getX() && p.x <= v2->getX()));

if (!vec1.isVertical()) {
PreciseDouble y = v2->getY();
PreciseDouble ycalc = y + (p.x - (PreciseDouble) v2->getX()) * vec1.dydx();

return inxrange && (p.y == ycalc);

} else {
return inxrange
&& ((p.y <= v1->getY() && p.y >= v2->getY())
|| (p.y >= v1->getY() && p.y <= v2->getY()));
}

}
bool Edge::hasVertex(const Vertex* v) const {
return ((*v) == (*v1) || (*v) == (*v2));
}
Vector2D Edge::getVector2D() const {
PreciseDouble dx = (PreciseDouble) v2->getX() - (PreciseDouble) v1->getX();
PreciseDouble dy = (PreciseDouble) v2->getY() - (PreciseDouble) v1->getY();
return Vector2D(dx, dy);
}
Vector2D_mp Edge::getVector2D_mp() const {
mpq_t dx, dy, tmp1, tmp2;
mpq_init(dx);
mpq_init(tmp1);
mpq_init(tmp2);
mpq_init(dy);

mpq_set_d(tmp1, v1->getX());
mpq_set_d(tmp2, v2->getX());
mpq_sub(dx, tmp2, tmp1);
mpq_set_d(tmp1, v1->getY());
mpq_set_d(tmp2, v2->getY());
mpq_sub(dy, tmp2, tmp1);

mpq_clear(tmp1);
mpq_clear(tmp2);

Vector2D_mp ret(dx, dy);

mpq_clear(dy);
mpq_clear(dx);
return ret;

}
bool Edge::operator==(const Edge & e) const {
if ((*e.v1 == *v1 && *e.v2 == *v2) || (*e.v1 == *v2 && *e.v2 == *v1)) {
return true;
} else
return false;
}

bool Edge::intersection_sec(const Edge& e, Edge& result) const {
Point p;
Point_p p1(e.v1->getX(), e.v1->getY());
Point_p p2(e.v2->getX(), e.v2->getY());
PreciseDouble f1, f2;

result.v1 = &Vertex::nullvertex;
result.v2 = &Vertex::nullvertex;

SecureOperator::startSecureCalc();

Vector2D vec1 = v1->minus2D(*v2);
Vector2D vec2 = e.v1->minus2D(*e.v2);
//TODO bbox check

if (vec1.hasEqualDir(vec2)) //edges with the same direction
{
if (!SecureOperator::isSecureResult())
return intersection_mp(e, result); //direction not secure ->recalculate mp
//from here on check whether vertices
//are contained in the other edge
if (e.contains(v1))
{
result.v1 = v1; //and keep result vertices

}
if (e.contains(v2)) {
result.v2 = v2;
}
if (contains(e.v1)) {
intersection_point.setX(e.v1->getX());
intersection_point.setY(e.v1->getY());
intersection_point.setZ(this->getZat(p1));

if (result.v1 == &Vertex::nullvertex) {
result.v1 = &intersection_point;
} else {
result.v2 = &intersection_point;
}
}
if (contains(e.v2)) {
intersection_point_2.setX(e.v2->getX());
intersection_point_2.setY(e.v2->getY());
intersection_point_2.setZ(this->getZat(p2));

if (result.v1 == &Vertex::nullvertex) {
result.v1 = &intersection_point_2;
} else {
result.v2 = &intersection_point_2;
}
}
if (!SecureOperator::isSecureResult())
return intersection_mp(e, result);

if (!(result.v1 == &Vertex::nullvertex)) {
if (result.v2 == &Vertex::nullvertex) {
result.v2 = result.v1;
}

return true;
}

if (!(result.v2 == &Vertex::nullvertex)) {
if (result.v1 == &Vertex::nullvertex) {
result.v1 = result.v2;
}

return true;
}

return false;

}

if (!SecureOperator::isSecureResult())
return intersection_mp(e, result); //direction not secure ->recalculate mp

//now edges cannot have the same direction -> calculate the cutting point

f1 = ((vec2.getDx() * e.v2->getY()) + (vec2.getDy() * v2->getX())
- (vec2.getDy() * e.v2->getX()) - (vec2.getDx() * v2->getY()))
/ ((vec1.getDy() * vec2.getDx()) - (vec2.getDy() * vec1.getDx()));
//avoid zero division
//-> dy and dx cannot both be zero due to precondition
if (vec2.getDx() == 0)
f2 = ((vec1.getDy() * f1) + v2->getY() - e.v2->getY()) / vec2.getDy();
else
f2 = ((vec1.getDx() * f1) + v2->getX() - e.v2->getX()) / vec2.getDx();

if (f1 <= 1 && f1 >= 0) //cutting point within length of this edge
{
if (!SecureOperator::isSecureResult())
return intersection_mp(e, result); //not secure->recalculate mp

if (f2 <= 1 && f2 >= 0) //and cutting point within length of *e*
{
if (!SecureOperator::isSecureResult())
return intersection_mp(e, result); //not secure->recalculate mp

p = *v2 + vec1 * f1;
intersection_point.setX(p.x.getRoundedVal());
intersection_point.setY(p.y.getRoundedVal());
intersection_point.setZ(v2->getZ() + f1.l * (v1->getZ() - v2->getZ()));
//build zero length edge at intersection point for result
result.v1 = &intersection_point;
result.v2 = result.v1;
return true;
}
} else {
return false;
}

return false;

}

bool Edge::intersection_mp(const Edge& e, Edge& result) const {

mpq_t f1, f2, vec1dx, vec1dy, vec2dx, vec2dy, p1x, p1y, p2x, p2y, tmp,
tmp2, tmp3;

Point_p p1(e.v1->getX(), e.v1->getY());
Point_p p2(e.v2->getX(), e.v2->getY());

Vector2D_mp vec1 = v1->minus2D_mp(*v2);
Vector2D_mp vec2 = e.v1->minus2D_mp(*e.v2);
//TODO bbox check
SecureOperator::startSecureCalc();

result.v1 = &Vertex::nullvertex;
result.v2 = &Vertex::nullvertex;

//TODO mp calculation here !

if (vec1.hasEqualDir(vec2)) {

if (e.contains(v1)) {
result.v1 = v1;

}
if (e.contains(v2)) {
result.v2 = v2;
}
if (contains(e.v1)) {
intersection_point.setX(e.v1->getX());
intersection_point.setY(e.v1->getY());
intersection_point.setZ(this->getZat(p1));

if (result.v1 == &Vertex::nullvertex) {
result.v1 = &intersection_point;
} else {
result.v2 = &intersection_point;
}
}
if (contains(e.v2)) {
intersection_point_2.setX(e.v2->getX());
intersection_point_2.setY(e.v2->getY());
intersection_point_2.setZ(this->getZat(p2));

if (result.v1 == &Vertex::nullvertex) {
result.v1 = &intersection_point_2;
} else {
result.v2 = &intersection_point_2;
}
}

if (!(result.v1 == &Vertex::nullvertex)) {
if (result.v2 == &Vertex::nullvertex) {
result.v2 = result.v1;
}

return true;
}

if (!(result.v2 == &Vertex::nullvertex)) {
if (result.v1 == &Vertex::nullvertex) {
result.v1 = result.v2;
}

return true;
}

return false;

}
mpq_init(f1);
mpq_init(vec1dx);
mpq_init(vec1dy);
mpq_init(p1x);
mpq_init(p1y);
mpq_init(f2);
mpq_init(vec2dx);
mpq_init(vec2dy);
mpq_init(p2x);
mpq_init(p2y);
mpq_init(tmp);
mpq_init(tmp2);
mpq_init(tmp3);

mpq_set(vec1dx, vec1.dx.get_mpq_t());
mpq_set(vec1dy, vec1.dy.get_mpq_t());
mpq_set_d(p1x, v2->getX());
mpq_set_d(p1y, v2->getY());

mpq_set(vec2dx, vec2.dx.get_mpq_t());
mpq_set(vec2dy, vec2.dy.get_mpq_t());
mpq_set_d(p2x, e.v2->getX());
mpq_set_d(p2y, e.v2->getY());

mpq_mul(tmp, vec2dx, p2y);
mpq_mul(tmp2, vec2dy, p1x);
mpq_add(tmp, tmp, tmp2);

mpq_mul(tmp2, vec2dy, p2x);
mpq_sub(tmp, tmp, tmp2);
mpq_mul(tmp2, vec2dx, p1y);
mpq_sub(tmp, tmp, tmp2);

mpq_mul(tmp2, vec1dy, vec2dx);
mpq_mul(tmp3, vec2dy, vec1dx);
mpq_sub(tmp2, tmp2, tmp3);

if (mpq_cmp(tmp2, f1) == 0) {

CLEAN_UP_INTERSECTS
throw std::runtime_error("intersects_mp: tmp2 should not be null ");
}

mpq_div(f1, tmp, tmp2);

if (mpq_cmp(vec2dy, f2) == 0) {
mpq_mul(tmp, vec1dx, f1);
mpq_add(tmp, tmp, p1x);
mpq_sub(tmp, tmp, p2x);
mpq_div(f2, tmp, vec2dx);
} else {
mpq_mul(tmp, vec1dy, f1);
mpq_add(tmp, tmp, p1y);
mpq_sub(tmp, tmp, p2y);
mpq_div(f2, tmp, vec2dy);
}

mpq_set_d(tmp, 0.0);
mpq_set_d(tmp2, 1.0);

if (mpq_cmp(f1, tmp) >= 0 && mpq_cmp(f1, tmp2) <= 0
&& mpq_cmp(f2, tmp) >= 0 && mpq_cmp(f2, tmp2) <= 0) {

mpq_mul(tmp, f1, vec1dx);
mpq_add(tmp, tmp, p1x);
mpq_mul(tmp2, f1, vec1dy);
mpq_add(tmp2, tmp2, p1y);

intersection_point.setX(mpq_get_d(tmp));
intersection_point.setY(mpq_get_d(tmp2));
intersection_point.setZ(
v2->getZ() + mpq_get_d(f1) * (v1->getZ() - v2->getZ()));
result.v1 = &intersection_point;
result.v2 = result.v1;
CLEAN_UP_INTERSECTS
return true;

} else {
CLEAN_UP_INTERSECTS
return false;
}

CLEAN_UP_INTERSECTS
return false;

}

bool Edge::equal3D(const Edge & e) const {
if ((e.v1->equal3D(*v1) && e.v2->equal3D(*v2))
|| (e.v2->equal3D(*v1) && e.v1->equal3D(*v2))) {
return true;
}

return false;
}
Point_p Edge::getMiddle() const {
Vector2D vec = getVector2D();
Point result = *v1 + vec * (PreciseDouble) 0.5;
return (Point_p) result;
}
int Edge::getSide_sec(const Point_p& p) {
SecureOperator::startSecureCalc();

if (*v1 == p || *v2 == p)
return 0;

Line l(*v1, *v2);

int side = l.getSide(Point(p));
if (!SecureOperator::isSecureResult()) {
Line_mp lmp(*v1, *v2);
side = lmp.getSide_mp(Point_mp(p));
}

return side;
}
bool Edge::intersects_sec(const Edge* e) const {
SecureOperator::startSecureCalc();

PreciseDouble f2, f1;

Vector2D vec1 = v1->minus2D(*v2);
Vector2D vec2 = e->v1->minus2D(*e->v2);

if (e->isPoint()) {
if (this->contains(e->v1)) {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);

return true;
} else
return false;
}
if (isPoint()) {
if (e->contains(v1)) {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);

return true;
} else
return false;
}

if (vec1.hasEqualDir(vec2)) {

if (e->contains(v1) || e->contains(v2) || contains(e->v1)) {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);
return true;
} else {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);

return false;
}
}

if (!SecureOperator::isSecureResult())
return intersects_mp(e);

f1 = ((vec2.getDx() * e->v2->getY()) + (vec2.getDy() * v2->getX())
- (vec2.getDy() * e->v2->getX()) - (vec2.getDx() * v2->getY()))
/ ((vec1.getDy() * vec2.getDx()) - (vec2.getDy() * vec1.getDx()));

if (vec2.getDx() == 0)
f2 = ((vec1.getDy() * f1) + v2->getY() - e->v2->getY()) / vec2.getDy();
else
f2 = ((vec1.getDx() * f1) + v2->getX() - e->v2->getX()) / vec2.getDx();

if (f1 <= 1 && f1 >= 0) {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);

if (f2 <= 1 && f2 >= 0) {
if (!SecureOperator::isSecureResult())
return intersects_mp(e);
return true;
}
} else {
return false;
}

return false;

}

bool Edge::intersects_mp(const Edge* e) const {

mpq_t f1, f2, vec1dx, vec1dy, vec2dx, vec2dy, p1x, p1y, p2x, p2y, tmp,
tmp2, tmp3;

Vector2D_mp vec1 = v1->minus2D_mp(*v2);
Vector2D_mp vec2 = e->v1->minus2D_mp(*e->v2);
//TODO bbox check
//TODO make mp calc for contains
if (e->isPoint()) {
if (this->contains(e->v1)) {
return true;
} else
return false;
}
if (isPoint()) {
if (e->contains(v1)) {
return true;
} else
return false;
}

if (vec1.hasEqualDir(vec2)) {
if (e->contains(v1) || e->contains(v2) || contains(e->v1)) {

return true;
} else {

return false;
}
}
mpq_init(f1);
mpq_init(vec1dx);
mpq_init(vec1dy);
mpq_init(p1x);
mpq_init(p1y);
mpq_init(f2);
mpq_init(vec2dx);
mpq_init(vec2dy);
mpq_init(p2x);
mpq_init(p2y);
mpq_init(tmp);
mpq_init(tmp2);
mpq_init(tmp3);

mpq_set(vec1dx, vec1.dx.get_mpq_t());
mpq_set(vec1dy, vec1.dy.get_mpq_t());
mpq_set_d(p1x, v2->getX());
mpq_set_d(p1y, v2->getY());

mpq_set(vec2dx, vec2.dx.get_mpq_t());
mpq_set(vec2dy, vec2.dy.get_mpq_t());
mpq_set_d(p2x, e->v2->getX());
mpq_set_d(p2y, e->v2->getY());

mpq_mul(tmp, vec2dx, p2y);
mpq_mul(tmp2, vec2dy, p1x);
mpq_add(tmp, tmp, tmp2);

mpq_mul(tmp2, vec2dy, p2x);
mpq_sub(tmp, tmp, tmp2);
mpq_mul(tmp2, vec2dx, p1y);
mpq_sub(tmp, tmp, tmp2);

mpq_mul(tmp2, vec1dy, vec2dx);
mpq_mul(tmp3, vec2dy, vec1dx);
mpq_sub(tmp2, tmp2, tmp3);

if (mpq_cmp(tmp2, f1) == 0) {

CLEAN_UP_INTERSECTS
throw std::runtime_error("intersects_mp: tmp2 should not be null ");
}

mpq_div(f1, tmp, tmp2);

if (mpq_cmp(vec2dy, f2) == 0) {
mpq_mul(tmp, vec1dx, f1);
mpq_add(tmp, tmp, p1x);
mpq_sub(tmp, tmp, p2x);
mpq_div(f2, tmp, vec2dx);
} else {
mpq_mul(tmp, vec1dy, f1);
mpq_add(tmp, tmp, p1y);
mpq_sub(tmp, tmp, p2y);
mpq_div(f2, tmp, vec2dy);
}

mpq_set_d(tmp, 0.0);
mpq_set_d(tmp2, 1.0);

if (mpq_cmp(f1, tmp) >= 0 && mpq_cmp(f1, tmp2) <= 0
&& mpq_cmp(f2, tmp) >= 0 && mpq_cmp(f2, tmp2) <= 0) {
CLEAN_UP_INTERSECTS
return true;

} else {
CLEAN_UP_INTERSECTS
return false;
}

CLEAN_UP_INTERSECTS
return false;

}
/* namespace tin */

bool Edge::isPoint() const {
return (*v1 == *v2);
}

bool Edge::operator <(const Edge& e) const {
const Vertex* min, *max, *min2, *max2;

if ((*v1) < (*v2)) {
min = v1;
max = v2;
} else {
min = v2;
max = v1;
}
if ((*e.v1) < (*e.v2)) {
min2 = e.v1;
max2 = e.v2;
} else {
min2 = e.v2;
max2 = e.v1;
}

if ((*min) < (*min2)) {
return true;
}
if (*min2 < *min) {
return false;
}
if (*max < *max2) {
return true;
}
if (*max2 < *max) {
return false;
}

return false;

}

VERTEX_Z Edge::getZat(const Point_p& p) const {
Vector3D vec1 = v1->minus3D(*v2);

if (!(vec1.getDx() == 0)) {
return ((p.x - v2->getX()) / vec1.getDx().l) * vec1.getDz().l + v2->getZ();
}

if (!(vec1.getDy() == 0)) {
return ((p.y - v2->getY()) / vec1.getDy().l) * vec1.getDz().l + v2->getZ();
}

return v2->getZ();

}
}
