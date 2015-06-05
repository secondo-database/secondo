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
#include "TinHelper.h"
#include "Vector2D.h"
#include "Vertex.h"
#include "Triangle.h"

#define LOG_ERROR -9

#define LOGGING_ERROR(op){}

const double tin::PreciseComp::MACHINE_EPSILON = std::numeric_limits<
PRECISE_VAL>::epsilon();

namespace tin {

const TinConfiguration TinConfiguration::DEFAULT = TinConfiguration(
TIN_PART_STANDARD_SIZE, MANIPULATE, INMEMORY);
const TinConfiguration TinConfiguration::ATTRIBUTE = TinConfiguration(
TIN_PART_STANDARD_SIZE, MANIPULATE, INMEMORY);

VERTEX_Z Op::constant = 0;

void max(mpq_t& result, mpq_t& v1, mpq_t& v2) {
if (mpq_equal(v1, v2)) {
mpq_set(result, v1);
return;
}

if (mpq_cmp(v1, v2) > 0) {

mpq_set(result, v1);
return;
} else {

mpq_set(result, v2);
return;
}
}

std::string AbstractTypeToString(AbstractType t) {
switch (t) {
case QUERY:
return "QUERY";

case MANIPULATE:
return "MANIPULATE";

default:
return "UNKNOWN";
}

}

bool SecureOperator::secure = true;

float absolute(float n) {
return std::fabs(n);
}

double absolute(double n) {
return std::fabs(n);
}

PreciseDouble absolute(PreciseDouble n) {
return n.abs();
}

int absolute(int n) {
return std::abs(n);
}

PreciseDouble& PreciseDouble::operator =(const int& b) {
this->u = (PRECISE_VAL) (b);
this->l = u;

return *this;
}

PreciseDouble& PreciseDouble::operator =(const double& b) {
this->u = (PRECISE_VAL) (b);
this->l = u;

return *this;
}

std::ostream& operator<<(std::ostream& os, const PreciseDouble &v) {

os << "PreciseDouble---Upper bound:" << v.u << " \tlower bound:" << v.l
<< std::endl;

return os;
}
PreciseDouble::PreciseDouble(const PreciseDouble & pd) {
u = pd.u;
l = pd.l;
}

PreciseDouble::PreciseDouble() {
l = 0;
u = 0;
}

PreciseDouble::~PreciseDouble() {
}
PreciseDouble::PreciseDouble(PRECISE_VAL i) {
u = i;
l = i;
}

PreciseDouble::PreciseDouble(const mpq_t& v) {
u = mpq_get_d(v);
l = u;
}

PRECISE_VAL PreciseDouble::getRoundedVal() {
if (l < 0.000000001 && l > -0.000000001)
return 0;
return std::floor(l * 1000000000) / 1000000000;
}
PreciseDouble PreciseDouble::operator -() const {
PreciseDouble d;
d.l = -u;
d.u = -l;

return d;
}

PreciseDouble PreciseDouble::abs() const {
PreciseDouble p;
p.l = std::abs(l);
p.u = std::abs(u);

if (p.l > p.u) {
PRECISE_VAL tmp = p.l;
p.l = p.u;
p.u = tmp;
}
return p;
}
void PreciseDouble::makeAbs() {

l = std::abs(l);
u = std::abs(u);

if (l > u) {
PRECISE_VAL tmp = l;
l = u;
u = tmp;
}
}

PreciseDouble PreciseDouble::operator +(const PreciseDouble& b) const {
PreciseDouble result;
PreciseArithmetic::error_add(*this, b, result);
return result;
}

;

PreciseDouble PreciseDouble::operator -(const PreciseDouble& b) const {
PreciseDouble result;
PreciseArithmetic::error_add(*this, (-b), result);
return result;
}

;

PreciseDouble PreciseDouble::operator *(const PreciseDouble& b) const {
PreciseDouble result;
PreciseArithmetic::error_mul(*this, b, result);
return result;
}

;

PreciseDouble PreciseDouble::operator /(const PreciseDouble& b) const {
PreciseDouble result;
PreciseArithmetic::error_div(*this, b, result);
return result;
}

;

PreciseDouble& PreciseDouble::operator =(const PreciseDouble& b) {
this->l = b.l;
this->u = b.u;
return *this;
}

;

PreciseDouble& PreciseDouble::operator +=(const PreciseDouble& b) {
PreciseDouble result;
PreciseArithmetic::error_add(*this, b, result);
return *this = result;
}

;

PreciseDouble& PreciseDouble::operator -=(const PreciseDouble& b) {
PreciseDouble result;
PreciseArithmetic::error_add(*this, (-b), result);
return *this = result;
}

;

bool PreciseDouble::operator <(const PreciseDouble& b) const {
return PreciseComp::lt(*this, b);
}

;

bool PreciseDouble::operator >(const PreciseDouble& b) const {
return PreciseComp::gt(*this, b);
}

;

bool PreciseDouble::operator >=(const PreciseDouble& b) const {
return PreciseComp::ge(*this, b);
}

;

bool PreciseDouble::operator <=(const PreciseDouble& b) const {
return PreciseComp::le(*this, b);
}

;

bool PreciseDouble::operator ==(const PreciseDouble& b) const {
return PreciseComp::eq(*this, b);
}

bool PreciseDouble::hasNoError() const {
if (l == u)
return true;
else
return false;
}

//////////////////////////////////////////////////////////////
void PreciseArithmetic::error_mul(const PreciseDouble& a,
const PreciseDouble& b, PreciseDouble& result) {

PRECISE_VAL tmp;

if (std::isnan(b.l) || std::isnan(a.l) || std::isnan(b.u)
|| std::isnan(a.u)) {
throw std::invalid_argument("NaN as input for calculation not allowed");
}

result.u = a.u * b.u;
result.l = a.l * b.l;

if (result.u < result.l) {
tmp = result.l;
result.l = result.u;
result.u = tmp;
}

result.l = result.l - result.l * PreciseComp::MACHINE_EPSILON;
result.u = result.u + result.u * PreciseComp::MACHINE_EPSILON;

LOGGING_ERROR("mul");
}
void PreciseArithmetic::error_add(const PreciseDouble& a,
const PreciseDouble& b, PreciseDouble& result) {

if (std::isnan(b.l) || std::isnan(a.l) || std::isnan(b.u)
|| std::isnan(a.u)) {
throw std::invalid_argument("NaN as input for calculation not allowed");
}

result.u = a.u + b.u;
result.l = a.l + b.l;

result.l = result.l - std::abs(result.l * PreciseComp::MACHINE_EPSILON);
result.u = result.u + std::abs(result.u * PreciseComp::MACHINE_EPSILON);

LOGGING_ERROR("add");
}

;

void PreciseArithmetic::error_add(const int a, const PreciseDouble& b,
PreciseDouble& result) {

if (std::isnan(b.l) || std::isnan(b.u)) {
throw std::invalid_argument("NaN as input for calculation not allowed");
}

result.u = a + b.u;
result.l = a + b.l;

result.l = result.l - std::abs(result.l * PreciseComp::MACHINE_EPSILON);
result.u = result.u + std::abs(result.u * PreciseComp::MACHINE_EPSILON);

LOGGING_ERROR("add");
}

;

void PreciseArithmetic::error_div(const PreciseDouble& a,
const PreciseDouble& b, PreciseDouble& result) {

PRECISE_VAL tmp;

if (std::isnan(b.l) || std::isnan(a.l) || std::isnan(b.u)
|| std::isnan(a.u)) {
throw std::invalid_argument("NaN as input for calculation not allowed");
}

result.u = a.u / b.l;
result.l = a.l / b.u;

if (result.u < result.l) {
tmp = result.l;
result.l = result.u;
result.u = tmp;
}

result.l = result.l - std::abs(result.l * PreciseComp::MACHINE_EPSILON);
result.u = result.u + std::abs(result.u * PreciseComp::MACHINE_EPSILON);

LOGGING_ERROR("div");
}
void PreciseArithmetic::error_sqrt(const PreciseDouble &a,
PreciseDouble &result) {
if (a.l < 0 || a.u < 0)
throw std::invalid_argument(E_PRECISEARITHMETIC_SQRT);

result.u = std::sqrt(a.u);
result.l = std::sqrt(a.l);

result.l = result.l - std::abs(result.l * PreciseComp::MACHINE_EPSILON);
result.u = result.u + std::abs(result.u * PreciseComp::MACHINE_EPSILON);

LOGGING_ERROR("SQRT");
}
void Point::print(std::ostream& out) {

}

Point_mp::Point_mp() {
mpq_init(x);
mpq_init(y);
}

Point_mp::Point_mp(POINT_COORDINATE ix, POINT_COORDINATE iy) {
if (ix.hasNoError() && iy.hasNoError()) {
mpq_init(x);
mpq_init(y);
mpq_set_d(y, iy.l);
mpq_set_d(x, ix.l);
} else
throw std::invalid_argument(E_POINTMP_CONSTRUCTOR);

}
Point_mp::Point_mp(VERTEX_COORDINATE ix, VERTEX_COORDINATE iy) {

mpq_init(x);
mpq_init(y);
mpq_set_d(y, iy);
mpq_set_d(x, ix);

}
Point_mp::Point_mp(const Point& p) {
if (p.x.hasNoError() && p.y.hasNoError()) {
mpq_init(x);
mpq_init(y);
mpq_set_d(y, p.y.l);
mpq_set_d(x, p.x.l);
} else
throw std::invalid_argument(E_POINTMP_CONSTRUCTOR);
}
Point_mp::Point_mp(const Point_p& p) {
mpq_init(x);
mpq_init(y);
mpq_set_d(y, p.y);
mpq_set_d(x, p.x);
}
Point_mp::Point_mp(const Vertex * v) {
mpq_init(x);
mpq_init(y);
mpq_set_d(y, v->getY());
mpq_set_d(x, v->getX());
}
Point_mp::Point_mp(const Point_mp& p) {
mpq_init(x);
mpq_init(y);
mpq_set(y, p.y);
mpq_set(x, p.x);
}

Point_mp::~Point_mp() {
mpq_clear(x);
mpq_clear(y);
}

Point_mp& Point_mp::operator=(const Point_mp& p) {
mpq_set(y, p.y);
mpq_set(x, p.x);

return *this;
}
Point_mp& Point_mp::operator=(const Vertex& v) {
mpq_set_d(x, v.getX());
mpq_set_d(y, v.getY());
return *this;
}

Point_mp Point_mp::operator+(const Vector2D_mp& v) const {
Point_mp n;
mpq_add(n.x, x, v.dx.get_mpq_t());
mpq_add(n.y, y, v.dy.get_mpq_t());

return n;
}
Vector2D_mp Point_mp::minus2D_mp(const Vertex& v) const {
Vector2D_mp n;
mpq_t vx, vy, resdy, resdx;
mpq_init(vx);
mpq_init(vy);
mpq_init(resdx);
mpq_init(resdy);
mpq_set_d(vx, v.getX());
mpq_set_d(vy, v.getY());

mpq_sub(resdx, x, vx);
mpq_sub(resdy, y, vy);

n.dx = (mpq_class) resdx;
n.dy = (mpq_class) resdy;

mpq_clear(vx);
mpq_clear(vy);
mpq_clear(resdx);
mpq_clear(resdy);

return n;

}
void Point_mp::print(std::ostream& out) {

}

Point Point::operator+(const Vector2D& v) const {

const POINT_COORDINATE & resultx = x + v.getDx();
const POINT_COORDINATE & resulty = y + v.getDy();
return Point(resultx, resulty);
}

Vector2D Point::operator-(const Vertex& v) const {
const POINT_COORDINATE & resultx = x - v.getX();
const POINT_COORDINATE & resulty = y - v.getY();
return Vector2D(resultx, resulty);
}
Vector2D_mp Point::minus2D_mp(const Vertex& v) const {
return Vector2D_mp(x.l - v.getX(), y.l - v.getY());
}
Point& Point::operator=(const Vertex& v) {
x = v.getX();
y = v.getY();
return *this;
}
Point& Point::operator=(const Point_mp& mpp) {
x = (PreciseDouble) mpp.x;
y = (PreciseDouble) mpp.y;
return *this;
}

bool Point::operator<(const Point &p) const {

if (this->y < p.y)
return true;
if (this->y == p.y && this->x < p.x) {
return true;
}

return false;
}
Point::Point(const Point_p& p) {
x = p.x;
y = p.y;
}
Point::Point(const Vertex * v) {
x = v->getX();
y = v->getY();
}
Point::Point(const Vertex & v) {
x = v.getX();
y = v.getY();
}

Point_p::Point_p() {
x = 0;
y = 0;
}

Point_p::Point_p(VERTEX_COORDINATE ix, VERTEX_COORDINATE iy) {
x = ix;
y = iy;
}
Point_p::Point_p(const Point& pt) {
x = pt.x.l;
y = pt.y.l;

}
Point_p::Point_p(const Vertex& v) {
x = v.getX();
y = v.getY();

}
Vector2D Point_p::operator -(const Vertex& v) const {
const VECTOR_COMPONENT & resultx = (VECTOR_COMPONENT) x - v.getX();
const VECTOR_COMPONENT & resulty = (VECTOR_COMPONENT) y - v.getY();
return Vector2D(resultx, resulty);
}

Vector2D_mp Point_p::minus2D_mp(const Vertex& v) const {
return Vector2D_mp(x - v.getX(), y - v.getY());
}

Point_p& Point_p::operator=(const Point_mp& mpp) {
x = mpq_get_d(mpp.x);
y = mpq_get_d(mpp.y);
return *this;
}
bool Point_p::operator==(const Point_p & p) {
return (x == p.x && y == p.y);
}

void TinFeatures::update(const TinFeatures & f) {
if (f.m_maxValue == (-std::numeric_limits<VERTEX_Z>::max()))
return;

if (m_maxValue < f.m_maxValue)
m_maxValue = f.m_maxValue;
if (m_minValue > f.m_minValue)
m_minValue = f.m_minValue;

if (bbox.getX1() > f.bbox.getX1())
bbox.setX1(f.bbox.getX1());
if (bbox.getX2() < f.bbox.getX2())
bbox.setX2(f.bbox.getX2());
if (bbox.getY1() > f.bbox.getY1())
bbox.setY1(f.bbox.getY1());
if (bbox.getY2() < f.bbox.getY2())
bbox.setY2(f.bbox.getY2());
}
void TinFeatures::update(const Vertex & v) {

if (m_maxValue < v.getZ())
m_maxValue = v.getZ();
if (m_minValue > v.getZ())
m_minValue = v.getZ();

if (bbox.getX1() > v.getX())
bbox.setX1(v.getX());
if (bbox.getX2() < v.getX())
bbox.setX2(v.getX());
if (bbox.getY1() > v.getY())
bbox.setY1(v.getY());
if (bbox.getY2() < v.getY())
bbox.setY2(v.getY());
}
void TinFeatures::update(const Triangle * at) {
VERTEX_Z m_minValuetmp, m_maxValuetmp;

m_minValuetmp = std::min(at->getVertex(1)->getZ(),
at->getVertex(2)->getZ());
m_minValuetmp = std::min(m_minValuetmp, at->getVertex(3)->getZ());
m_maxValuetmp = std::max(at->getVertex(1)->getZ(),
at->getVertex(2)->getZ());
m_maxValuetmp = std::max(m_maxValuetmp, at->getVertex(3)->getZ());

if (m_minValuetmp < m_minValue)
m_minValue = m_minValuetmp;
if (m_maxValuetmp > m_maxValue)
m_maxValue = m_maxValuetmp;

if (bbox.getX1() > at->bbox().getX1())
bbox.setX1(at->bbox().getX1());
if (bbox.getX2() < at->bbox().getX2())
bbox.setX2(at->bbox().getX2());
if (bbox.getY1() > at->bbox().getY1())
bbox.setY1(at->bbox().getY1());
if (bbox.getY2() < at->bbox().getY2())
bbox.setY2(at->bbox().getY2());

}

}
