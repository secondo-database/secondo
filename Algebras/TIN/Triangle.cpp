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

 Class ~Triangle~ and nested class triangleWalker

*/

#include "Triangle.h"
#include <iostream>
#include <sstream>
#include <string>
#include "Edge.h"
#include "TinPart.h"

#ifdef TRIANGLE_NO_LOGGING
#define LOGGING_SWITCH_OFF
#endif
#include <TinLogging.h>

namespace tin {
const bool Triangle::edge_iterator::FORWARD = true;
const bool Triangle::edge_iterator::BACKWARD = false;
Triangle::Triangle(const Vertex* iv1, const Vertex* iv2, const Vertex* iv3,
TinPart * imyPart) {
LOGP
const Vertex * clockwise_second_vertex;

if (!iv1 || !iv2 || !iv3)
throw std::invalid_argument(
"Constructing triangle with null pointer.(Triangle::Triangle)");

if ((clockwise_second_vertex = Triangle::isTriangle_sec(iv1, iv2, iv3))) {

v1 = iv1;
v2 = clockwise_second_vertex;
v3 = (clockwise_second_vertex == iv2 ? iv3 : iv2);

n1 = 0;
n2 = 0;
n3 = 0;
myPart = imyPart;

LOG_EXP(print())
LOG("trianglecreate")
} else {
std::ostringstream str;
str << "Constructing triangle with invalid vertices."
<< "Three vertices on a line.(Triangle::Triangle)";
iv1->print(str);
iv2->print(str);
iv3->print(str);

throw std::invalid_argument(str.str().c_str());

}

LOGP
}

Triangle::Triangle(AbstractVertexContainer* vc, void * buff,
uint32_t & offset, TinPart * imyPart) {
LOGP
FILE_VERTEX_POINTER lv1, lv2, lv3;

FILE_TRIANGLE_POINTER lt1, lt2, lt3;

lv1 = *((FILE_VERTEX_POINTER*) (buff + offset));
offset += sizeof(FILE_VERTEX_POINTER);
lv2 = *((FILE_VERTEX_POINTER*) (buff + offset));
offset += sizeof(FILE_VERTEX_POINTER);
lv3 = *((FILE_VERTEX_POINTER*) (buff + offset));
offset += sizeof(FILE_VERTEX_POINTER);

v1 = vc->getVertexByYIndex(lv1);
v2 = vc->getVertexByYIndex(lv2);
v3 = vc->getVertexByYIndex(lv3);

LOG_EXP(v1->print())
LOG_EXP(v2->print())
LOG_EXP(v3->print())

myPart = imyPart;

lt1 = *((FILE_TRIANGLE_POINTER*) (buff + offset));
offset += sizeof(FILE_TRIANGLE_POINTER);
lt2 = *((FILE_TRIANGLE_POINTER*) (buff + offset));
offset += sizeof(FILE_TRIANGLE_POINTER);
lt3 = *((FILE_TRIANGLE_POINTER*) (buff + offset));
offset += sizeof(FILE_TRIANGLE_POINTER);

switch (lt1) {
case FILE_TRIANGLE_POINTER_BOUNDARY:
LOGP
n1 = VORONOI_OPEN_END;
break;
case FILE_TRIANGLE_POINTER_UNKNOWN:
LOGP
n1 = 0;
break;
default:
LOGP
n1 = myPart->getTriangleByIndex(lt1);
break;
}
switch (lt2) {
case FILE_TRIANGLE_POINTER_BOUNDARY:
n2 = VORONOI_OPEN_END;
break;
case FILE_TRIANGLE_POINTER_UNKNOWN:
n2 = 0;
break;
default:
n2 = myPart->getTriangleByIndex(lt2);
break;
}
switch (lt3) {
case FILE_TRIANGLE_POINTER_BOUNDARY:
n3 = VORONOI_OPEN_END;
break;
case FILE_TRIANGLE_POINTER_UNKNOWN:
n3 = 0;
break;
default:
n3 = myPart->getTriangleByIndex(lt3);
break;
}
}
Triangle::Triangle() {
n1 = 0;
n2 = 0;
n3 = 0;
v1 = 0;
v2 = 0;
v3 = 0;
myPart = 0;
}

Triangle::~Triangle() {
LOGP
LOG(myPart)
LOG(n1)
LOG(n2)
LOG(n3)
if (n1 && n1 != VORONOI_OPEN_END)
n1->removeNeighbor(this);
if (n2 && n2 != VORONOI_OPEN_END)
n2->removeNeighbor(this);
if (n3 && n3 != VORONOI_OPEN_END)
n3->removeNeighbor(this);
LOGP}
NeighborEdge Triangle::getEdgeWithVertex(const Vertex * iv) {
LOGP
if (*iv == *v1)
return NeighborEdge(v1, v2, this, n1);
if (*iv == *v2)
return NeighborEdge(v2, v3, this, n2);
if (*iv == *v3)
return NeighborEdge(v3, v1, this, n3);
LOGP
return NeighborEdge();
}
const Vertex * Triangle::getMaxYVertex() const {
if (*v1 < *v2) {
if (*v2 < *v3) {
return v3;
} else {
return v2;
}
} else {
if (*v3 < *v1) {
return v1;
} else {
return v3;
}
}
}
const Vertex * Triangle::getMaxYCWVertex() const {
if (*v1 < *v2) {
if (*v2 < *v3) {
return v1;
} else {
return v3;
}
} else {
if (*v3 < *v1) {
return v2;
} else {
return v1;
}
}
}
const Vertex * Triangle::getMaxYCCWVertex() const {
if (*v1 < *v2) {
if (*v2 < *v3) {
return v2;
} else {
return v1;
}
} else {
if (*v3 < *v1) {
return v3;
} else {
return v2;
}
}
}
const Vertex * Triangle::getMiddleYVertex() const {
if (*v1 < *v2) {
if (*v2 < *v3) {
return v2;
} else if (*v3 < *v1) {
return v1;
} else {
return v3;
}
} else {
if (*v1 < *v3) {
return v1;
} else if (*v3 < *v2) {
return v2;
} else
return v3;
}
}
const Vertex * Triangle::getMinYVertex() const {
if (*v1 < *v2) {
if (*v1 < *v3) {
return v1;
} else {
return v3;
}
} else {
if (*v3 < *v2) {
return v3;
} else {
return v2;
}
}
}
bool Triangle::hasEdge(const Edge& e) {
return (getEdge(1).equal3D(e) || getEdge(2).equal3D(e)
|| getEdge(3).equal3D(e));
}
NeighborEdge Triangle::getNextEdge(const NeighborEdge& currentEdge) {
LOGP
if (*(currentEdge.getV1()) == *v1) {
if (*(currentEdge.getV2()) == *v2)
return NeighborEdge(v1, v3, this, getNeighbor(3));
else
return NeighborEdge(v1, v2, this, getNeighbor(1));

}

if (*(currentEdge.getV1()) == *v2) {
if (*(currentEdge.getV2()) == *v1)
return NeighborEdge(v2, v3, this, getNeighbor(2));
else
return NeighborEdge(v2, v1, this, getNeighbor(1));

}

if (*(currentEdge.getV1()) == *v3) {
if (*(currentEdge.getV2()) == *v1)
return NeighborEdge(v3, v2, this, getNeighbor(2));
else
return NeighborEdge(v3, v1, this, getNeighbor(3));

}

LOGP
return NeighborEdge(0, 0, 0, 0);
}
NeighborEdge Triangle::getNextEdge_noload(
const NeighborEdge& currentEdge) {
LOGP
if (*(currentEdge.getV1()) == *v1) {
if (*(currentEdge.getV2()) == *v2)
return NeighborEdge(v1, v3, this, n3);
else
return NeighborEdge(v1, v2, this, n1);

}

if (*(currentEdge.getV1()) == *v2) {
if (*(currentEdge.getV2()) == *v1)
return NeighborEdge(v2, v3, this, n2);
else
return NeighborEdge(v2, v1, this, n1);

}

if (*(currentEdge.getV1()) == *v3) {
if (*(currentEdge.getV2()) == *v1)
return NeighborEdge(v3, v2, this, n2);
else
return NeighborEdge(v3, v1, this, n3);

}

LOGP
return NeighborEdge(0, 0, 0, 0);
}
Triangle* Triangle::getNeighbor(const int index) {
LOGP
Triangle * nt = 0;
switch (index) {
case 1:
if (n1)
return n1;
else {
if (myPart)
nt = myPart->getNeighbor(Edge(v1, v2), this);
if (nt)
addNeighbor(nt);
return n1;
}
break;
case 2:
if (n2)
return n2;
else {
if (myPart)
nt = myPart->getNeighbor(Edge(v2, v3), this);
if (nt)
addNeighbor(nt);
return n2;
}
break;
case 3:
if (n3)
return n3;
else {
if (myPart)
nt = myPart->getNeighbor(Edge(v3, v1), this);
if (nt)
addNeighbor(nt);
return n3;
}
break;
default:
LOGP
throw std::runtime_error("Index out of bounds.(Triangle::getNeighbor)");
}

}

bool Triangle::triangleWalker::heightWalk(
Triangle::triangleWalker * walker, Triangle* next) {

if (next == (Triangle*) END_OF_SEGMENT
|| next == (Triangle*) END_OF_WALK) {
heightWalkParameter * p =
(heightWalkParameter *) walker->getUserParameter1();

Triangle * c = walker->getCurrentTriangle();

p->values.push_back(c->getValue(walker->getCurrentDestination()));
}

return false;
}

bool Triangle::triangleWalker::stopAtEdgeWalk(
Triangle::triangleWalker * walker, Triangle* next) {
Edge * e = (Edge*) walker->getUserParameter1();

if (e) {
if (!walker->isTriangle(next))
return true;

if (walker->getCurrentTriangle()->hasEdge(*e) && next->hasEdge(*e))
return true;
}

return false;

}

Triangle * Triangle::walkToTriangle_sec(const Point_p & destination) {
LOGP
Point_p * arPoints = new Point_p(destination);

Triangle::triangleWalker walker(this, arPoints, 1, false, false);

walker.walk_sec();

LOG(*myPart)
LOG("Part of result:")
LOG_EXP((walker.getCurrentTriangle())->print())

LOGP

if (walker.getCurrentTriangle()->isInside_sec(destination))
return walker.getCurrentTriangle();
else
return 0;

}
int Triangle::triangleWalker::walk_step_sec() {
SecureOperator::startSecureCalc();
LOGP
Edge e1 = current->getEdge(1);

LOG("Walk a step in Triangle: ")
LOG_EXP(current->print())
//turn to neighbor n1
if (e1.intersects_sec(&currentSegment)
&& e1.getSide_sec((Point_p) *currentSegment.getV2()) < 0) {

if (!current->n1 && !noload) {
current->getNeighbor(1);
}

if (!current->n1 || current->n1 == VORONOI_OPEN_END
|| (stayInSamePart && !current->n1->isInSamePart(current))) {
if (userExit && userExit(this, (Triangle*) END_OF_ACCESS))
return END_OF_WALK;
LOG("End of walk")
LOG(current->n1)
return END_OF_WALK;

} else {
if (current->n1 != last) {
if (userExit && userExit(this, current->n1))
return END_OF_WALK;

last = current;
current = current->n1;
LOGP
return TRIANGLE_ENTERED;
}
}
}

//turn to neighbor n2
e1 = current->getEdge(2);
if (e1.intersects_sec(&currentSegment)
&& e1.getSide_sec((Point_p) *currentSegment.getV2()) < 0) {
if (!current->n2 && !noload) {
current->getNeighbor(2);
}

if (!current->n2 || current->n2 == VORONOI_OPEN_END
|| (stayInSamePart && !current->n2->isInSamePart(current))) {
if (userExit && userExit(this, (Triangle*) END_OF_ACCESS))
return END_OF_WALK;
LOG("End of walk")
LOG(current->n2)
return END_OF_WALK;

} else {
if (current->n2 != last) {
if (userExit && userExit(this, current->n2))
return END_OF_WALK;

last = current;
current = current->n2;
LOGP
return TRIANGLE_ENTERED;
}

}
}

//turn to neighbor n3
e1 = current->getEdge(3);
if (e1.intersects_sec(&currentSegment)
&& e1.getSide_sec((Point_p) *currentSegment.getV2()) < 0) {
if (!current->n3 && !noload) {
current->getNeighbor(3);
}

if (!current->n3 || current->n3 == VORONOI_OPEN_END
|| (stayInSamePart && !current->n3->isInSamePart(current))) {
if (userExit && userExit(this, (Triangle*) END_OF_ACCESS))
return END_OF_WALK;

LOG(current->n3)

LOG("End of walk")
return END_OF_WALK;

} else {
if (current->n3 != last) {
if (userExit && userExit(this, current->n3))
return END_OF_WALK;

last = current;
current = current->n3;
LOGP
return TRIANGLE_ENTERED;
}

}
}

//currentPoint is in current Triangle -> reached end of current segment
if (currentPoint < noPoints - 1) {

if (userExit)
userExit(this, (Triangle*) END_OF_SEGMENT);

currentPoint++; //move to next segment
currentSegment = Segment(arPoints[currentPoint - 1],
arPoints[currentPoint]);
last = 0;
LOG("End of segment")
return END_OF_SEGMENT;
}

if (userExit)
userExit(this, (Triangle*) END_OF_WALK);

LOGP
LOG("No more segments -> end of walk")
// here no more segments are available so the walk ends
return END_OF_WALK;

}
NeighborEdge Triangle::getEdge(const Edge & e) {
if (getEdge(1).equal3D(e))
return NeighborEdge(v1, v2, this, n1);
if (getEdge(2).equal3D(e))
return NeighborEdge(v2, v3, this, n2);
if (getEdge(3).equal3D(e))
return NeighborEdge(v3, v1, this, n3);

return NeighborEdge();
}
/*
Method matchNeighbors installs the neighborship
between this and all neighboring triangles in ~match~.

*/
void Triangle::matchNeighbors(std::deque<Triangle *>::iterator match,
const std::deque<Triangle *>::iterator& end) {

Edge e1 = getEdge(1);
Edge e2 = getEdge(2);
Edge e3 = getEdge(3);

while (match != end) {
if ((*match) != this && (*match) && (*match) != VORONOI_OPEN_END) {

if ((*match)->hasEdge(e1)) {
n1 = (*match);
(*match)->addNeighbor(this, false);
}
if ((*match)->hasEdge(e2)) {
n2 = (*match);
(*match)->addNeighbor(this, false);
}
if ((*match)->hasEdge(e3)) {
n3 = (*match);
(*match)->addNeighbor(this, false);
}
}
++match;
}

}

void Triangle::triangleWalker::walk_sec() {
LOG("Walking from ...")
LOG_EXP(this->currentSegment.getV1()->print())
LOG(" ...to...  ")
LOG_EXP(this->currentSegment.getV2()->print())
while (this->walk_step_sec() != END_OF_WALK)
;
}
void Triangle::triangleWalker::walk_segment_sec() {
while (this->walk_step_sec() != END_OF_WALK
&& this->walk_step_sec() != END_OF_SEGMENT)
;
}
void Triangle::addReference() {
if (myPart)
myPart->addContentReference();
}
void Triangle::deleteReference() {
if (myPart)
myPart->deleteContentReference();
}
void Triangle::removeDifferentPartNeighbors() {
if (n1 && n1 != VORONOI_OPEN_END && !n1->isInSamePart(this))
removeNeighbor(n1);
if (n2 && n2 != VORONOI_OPEN_END && !n2->isInSamePart(this))
removeNeighbor(n2);
if (n3 && n3 != VORONOI_OPEN_END && !n3->isInSamePart(this))
removeNeighbor(n3);
}
void Triangle::removeNeighbor(Triangle * t) {
if (t == n1) {
n1 = 0;
}
if (t == n2) {
n2 = 0;
}
if (t == n3) {
n3 = 0;
}
}
bool Triangle::isInSamePart(const Triangle * t) const {
if (t && t != VORONOI_OPEN_END)
return t->myPart == myPart;
else
return false;
}
bool Triangle::isNeighbor(Triangle& iat) {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

Vertex *verticesMe[3] = { 0, 0, 0 };
Vertex *verticesIat[3] = { 0, 0, 0 };

SecureOperator::startSecureCalc();

Vertex *cv[2] = { 0, 0 };

int commonVertexCnt = 0;
int intersectCnt = 0;

if (iat.bbox().hasIntersection(this->bbox())) {
for (int i = 0; i < 3; i++) {
verticesMe[i] = const_cast<Vertex *>(getVertex(i + 1));
verticesIat[i] = const_cast<Vertex *>(iat.getVertex(i + 1));
}
//check for common vertices
for (int i2 = 0; i2 < 3; i2++) {

for (int i3 = 0; i3 < 3; i3++) {
if (verticesIat[i3] != 0 && *verticesMe[i2] == *verticesIat[i3]) {
if (commonVertexCnt < 2 && verticesMe[i2]->equal3D(*verticesIat[i3])) {
cv[commonVertexCnt++] = verticesMe[i2];
verticesMe[i2] = 0;
verticesIat[i3] = 0;
break;
} else
return false;
}
}
}

if (commonVertexCnt == 2) //common Edge
return true;

}

return false;
}
bool Triangle::checkNeighbors() {

return (n1 && n1 != VORONOI_OPEN_END ? n1->isNeighbor(*this) : true)

&& (n2 && n2 != VORONOI_OPEN_END ? n2->isNeighbor(*this) : true)
&& (n3 && n3 != VORONOI_OPEN_END ? n3->isNeighbor(*this) : true);
}
void Triangle::addOpenEnd() {
if (!n1) {
n1 = VORONOI_OPEN_END;
return;
}
if (!n2) {
n2 = VORONOI_OPEN_END;
return;
}
if (!n3) {
n3 = VORONOI_OPEN_END;
return;
}
return; //TODO this is not a clean solution there must be an error
throw std::runtime_error(E_TRIANGLE_ADDNEIGHBOR);
}
void Triangle::addNeighbor(Triangle *t, bool bidirectional) {
if (!t)
throw std::runtime_error(
"Null pointer cannot be added as neighbor.(Triangle::addNeighbor)");

if (t == this)
throw std::runtime_error(
"Triangle cannot add itself as neighbor.(Triangle::addNeighbor)");

if (t == VORONOI_OPEN_END) {
addOpenEnd();
return;
}

if (t->hasVertices2D(*v1, *v2)) {
if (n1)
if (n1 == VORONOI_OPEN_END)
addOpenEnd();
else if (n1 != t)
throw std::runtime_error(
"The added neighbor already exists! (Triangle::addNeighbor)");

if (bidirectional && !t->hasNeighbor(this))
t->addNeighbor(this, false);
n1 = t;
return;
}
if (t->hasVertices2D(*v2, *v3)) {
if (n2)
if (n2 == VORONOI_OPEN_END)
addOpenEnd();
else if (n2 != t)
throw std::runtime_error(
"The added neighbor already exists! (Triangle::addNeighbor)");
if (bidirectional && !t->hasNeighbor(this))
t->addNeighbor(this, false);
n2 = t;
return;
}
if (t->hasVertices2D(*v3, *v1)) {
if (n3)
if (n3 == VORONOI_OPEN_END)
addOpenEnd();
else if (n3 != t)
throw std::runtime_error(
"The added neighbor already exists! (Triangle::addNeighbor)");
if (bidirectional && !t->hasNeighbor(this))
t->addNeighbor(this, false);
n3 = t;
return;
}

throw std::runtime_error(E_TRIANGLE_ADDNEIGHBOR);
}
bool Triangle::hasNeighbor(Triangle * t) {
return n1 == t || n2 == t || n3 == t;
}
bool Triangle::hasVertices2D(const Vertex& iv1, const Vertex& iv2) {
if (*v1 == iv1) {
if (*v2 == iv2) {
return true;
} else if (*v3 == iv2) {
return true;
} else
return false;
}

if (*v2 == iv1) {
if (*v1 == iv2) {
return true;
} else if (*v3 == iv2) {
return true;
} else
return false;
}
if (*v3 == iv1) {
if (*v1 == iv2) {
return true;
} else if (*v2 == iv2) {
return true;
} else
return false;
}

return false;
}
Triangle* Triangle::clone() {
Triangle* nt = new Triangle(v1, v2, v3, myPart);
nt->n1 = n1;
nt->n2 = n2;
nt->n3 = n3;

return nt;
}
#ifndef UNIT_TEST
ListExpr Triangle::outTriangle() {
LOGP
if (v1 == 0 || v2 == 0 || v3 == 0)
return nl->SymbolAtom(Symbol::UNDEFINED());

return nl->ThreeElemList(v1->outVertex(), v2->outVertex(), v3->outVertex());

}
#endif

TIN_SIZE Triangle::getSizeOnDisc() {
return sizeof(FILE_VERTEX_POINTER) * 3 + 3 * sizeof(FILE_TRIANGLE_POINTER);
}
int Triangle::getIndexInArray() const {
if (myPart)
return (this - myPart->getTriangleArrayBaseAdress());
else
return -1;
}
bool Triangle::putSecondoRepresentation(const AbstractVertexContainer * vc,
void * buff, uint32_t & offset) const {
FILE_VERTEX_POINTER lv1, lv2, lv3;

//first write the vertex indices to buffer
lv1 = vc->getYIndex(v1);
lv2 = vc->getYIndex(v2);
lv3 = vc->getYIndex(v3);

*((FILE_VERTEX_POINTER*) (buff + offset)) = lv1;
offset += sizeof(FILE_VERTEX_POINTER);
*((FILE_VERTEX_POINTER*) (buff + offset)) = lv2;
offset += sizeof(FILE_VERTEX_POINTER);
*((FILE_VERTEX_POINTER*) (buff + offset)) = lv3;
offset += sizeof(FILE_VERTEX_POINTER);

//then the indices of the neighbors
if (!n1)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;
else if (n1 == VORONOI_OPEN_END)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
FILE_TRIANGLE_POINTER_BOUNDARY;
else {

if (n1->isInSamePart(this))
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
(FILE_TRIANGLE_POINTER) n1->getIndexInArray();
else
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;

}
offset += sizeof(FILE_TRIANGLE_POINTER);

if (!n2)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;
else if (n2 == VORONOI_OPEN_END)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
FILE_TRIANGLE_POINTER_BOUNDARY;
else {
if (n2->isInSamePart(this))
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
(FILE_TRIANGLE_POINTER) n2->getIndexInArray();
else
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;
}

offset += sizeof(FILE_TRIANGLE_POINTER);

if (!n3)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;
else if (n3 == VORONOI_OPEN_END)
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
FILE_TRIANGLE_POINTER_BOUNDARY;
else {
if (n3->isInSamePart(this))
*((FILE_TRIANGLE_POINTER*) (buff + offset)) =
(FILE_TRIANGLE_POINTER) n3->getIndexInArray();
else
*((FILE_TRIANGLE_POINTER*) (buff + offset)) = FILE_TRIANGLE_POINTER_UNKNOWN;
}
offset += sizeof(FILE_TRIANGLE_POINTER);

return true;

}
bool Triangle::putSTLbinaryRepresentation(void * buff,
uint32_t & offset) const {

//normal vector
Vertex normalv(0, 0, 0);
normalv.putSTLbinaryRepresentation(buff, offset);

//vertices
v1->putSTLbinaryRepresentation(buff, offset);
v2->putSTLbinaryRepresentation(buff, offset);
v3->putSTLbinaryRepresentation(buff, offset);

//attribute dummy
*((uint16_t*) (buff + offset)) = (uint16_t) 0;
offset += 2;

return true;

}

const Vertex* Triangle::isTriangle_sec(const Vertex* iv1,
const Vertex* iv2, const Vertex* iv3) {

if (iv1 == 0 || iv2 == 0 || iv3 == 0)
throw std::invalid_argument("isTriangle_sec called with null pointer");

//v1 and v2  etc. in the same place
if (*iv1 == *iv2 || *iv1 == *iv3 || *iv2 == *iv3) {
return false; //always secure
}

SecureOperator::startSecureCalc();

Line l(*iv1, *iv2);
int side = l.getSide(Point(iv3));

if (!SecureOperator::isSecureResult())
return isTriangle_mp(iv1, iv2, iv3);

if (side == 0)
return 0;

if (side > 0)
return iv2;
else
return iv3;

}

const Vertex* Triangle::isTriangle_mp(const Vertex *iv1, const Vertex *iv2,
const Vertex *iv3) {
Line_mp lmp(*iv1, *iv2);

int side = lmp.getSide_mp(Point_mp(iv3));

if (side == 0)
return 0;

if (side > 0)
return iv2;
else
return iv3;

}

bool Triangle::isInside_sec(const Point_p& p) const {

if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

if (hasVertex(p))
return true;

Vector2D vector1, vector2, vectorTest;

//TODO possible overflow
vector1 = v3->minus2D(*v1);
vector2 = v2->minus2D(*v1);
vectorTest = p - (*v1);

SecureOperator::startSecureCalc();

if (vectorTest.isNull()) {
if (!SecureOperator::isSecureResult())
return isInside_mp(p);

return true; //on a vertex

}

if (vectorTest.isBetween(&vector1, &vector2)) {

vector1 = v1->minus2D(*v2);
vector2 = v3->minus2D(*v2);
vectorTest = p - *v2;

if (vectorTest.isNull()) {
if (!SecureOperator::isSecureResult())
return isInside_mp(p);
return true; //on a vertex
}

if (vectorTest.isBetween(&vector1, &vector2)) {
if (!SecureOperator::isSecureResult())
return isInside_mp(p);

return true; // inside or on an edge
} else {

if (!SecureOperator::isSecureResult())
return isInside_mp(p);
return false;

}

} else {
if (!SecureOperator::isSecureResult())
return isInside_mp(p);

return false;
}

}

bool Triangle::isInside_mp(const Point_p& p) const {
Vector2D_mp vector1, vector2, vectorTest;

//TODO possible overflow
vector1 = v3->minus2D_mp(*v1);

vector2 = v2->minus2D_mp(*v1);
vectorTest = p.minus2D_mp(*v1);

if (vectorTest.isNull())
return true; //on a vertex

if (vectorTest.isBetween(&vector1, &vector2)) {

vector1 = v1->minus2D_mp(*v2);
vector2 = v3->minus2D_mp(*v2);
vectorTest = p.minus2D_mp(*v2);

if (vectorTest.isNull())
return true; //on a vertex

if (vectorTest.isBetween(&vector1, &vector2)) {
return true; // inside or on an edge
} else
return false;

} else
return false;

}
bool Triangle::isEqual(Vertex& iv1, Vertex& iv2, Vertex& iv3) {
if ((v1->equal3D(iv1) || v1->equal3D(iv2) || v1->equal3D(iv3))
&& (v2->equal3D(iv1) || v2->equal3D(iv2) || v2->equal3D(iv3))
&& (v3->equal3D(iv1) || v3->equal3D(iv2) || v3->equal3D(iv3)))
return true;

return false;
}
void Triangle::print(std::ostream & out) const {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

out << "\n" << "Triangle in Part: ";
if (myPart)
out << myPart << " index in array: " << getIndexInArray();
v1->print(out);
v2->print(out);
v3->print(out);

if (n1 && n1 != VORONOI_OPEN_END)
n1->printNeighbor(out);
if (n1 == VORONOI_OPEN_END)
out << "\n" << "OPEN_END", getEdge(1).getV1()->print(out),
getEdge(1).getV2()->print(
out);
if (n2 && n2 != VORONOI_OPEN_END)
n2->printNeighbor(out);
if (n2 == VORONOI_OPEN_END)
out << "\n" << "OPEN_END", getEdge(2).getV1()->print(out),
getEdge(2).getV2()->print(
out);
;
if (n3 && n3 != VORONOI_OPEN_END)
n3->printNeighbor(out);
if (n3 == VORONOI_OPEN_END)
out << "\n" << "OPEN_END", getEdge(3).getV1()->print(out),
getEdge(3).getV2()->print(
out);
;
}
void Triangle::printNeighbor(std::ostream & out) const {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

out << "\n" << "Neighbor: ";
v1->print(out);
v2->print(out);
v3->print(out);

}

VERTEX_Z Triangle::getValue(const Point_p& p) const {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);
LOGP

Vector3D vc1 = v2->minus3D(*v1);
Vector3D vc2 = v3->minus3D(*v1);

VERTEX_COORDINATE xp, yp, x_0, y_0, x_1, y_1, x_2, y_2, f1, f2;

x_0 = v1->getX();
y_0 = v1->getY();

LOG(x_0)
LOG(y_0)

xp = p.x - x_0;
yp = p.y - y_0;

LOG(xp)
LOG(yp)

x_1 = vc1.getDx().l;
x_2 = vc2.getDx().l;
y_1 = vc1.getDy().l;
y_2 = vc2.getDy().l;

LOG(x_1)
LOG(y_1)
LOG(x_2)
LOG(y_2)

if (x_1 == 0) {
f2 = xp / x_2;
f1 = (yp - y_2 * f2) / y_1;
} else if (y_1 == 0) {
f2 = yp / y_2;
f1 = (xp - x_2 * f2) / x_1;
} else {
VERTEX_COORDINATE d = x_1 / y_1;
f2 = (xp - (yp * d)) / (x_2 - y_2 * d);
f1 = (xp - (x_2 * f2)) / x_1;
}

LOG(f1)
LOG(f2)

VERTEX_COORDINATE z = f1 * vc1.getDz().l + f2 * vc2.getDz().l + v1->getZ();

LOG("Value calculated:")
LOG(z)

return z;
}

const Vertex* Triangle::getVertex(const int n) const {
switch (n) {
case 1:
return v1;
break;
case 2:
return v2;
break;
case 3:
return v3;
break;
default:
throw std::invalid_argument(E_TRIANGLE_GETVERTEX);
}
}

bool Triangle::isCompatibleWith_sec(const Triangle& iat) const {

if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

Vertex *verticesMe[3] = { 0, 0, 0 };
Vertex *verticesIat[3] = { 0, 0, 0 };

SecureOperator::startSecureCalc();

Vertex *cv[2] = { 0, 0 };

int commonVertexCnt = 0;

if (iat.bbox().hasIntersection(this->bbox())) {
for (int i = 0; i < 3; i++) {
verticesMe[i] = const_cast<Vertex *>(getVertex(i + 1));
verticesIat[i] = const_cast<Vertex *>(iat.getVertex(i + 1));
}
//check for common vertices
for (int i2 = 0; i2 < 3; i2++) {

for (int i3 = 0; i3 < 3; i3++) {
if (verticesIat[i3] != 0 && *verticesMe[i2] == *verticesIat[i3]) {
if (commonVertexCnt < 2 && verticesMe[i2]->equal3D(*verticesIat[i3])) {
cv[commonVertexCnt++] = verticesMe[i2];
verticesMe[i2] = 0;
verticesIat[i3] = 0;
break;
} else
return false;
}
}
}

if (commonVertexCnt == 2) //common Edge
{

Point p1(cv[0]);
Point checkpiat;
Point checkpme;

for (int i4 = 0; i4 < 3; i4++) {
if (verticesIat[i4] != 0)
checkpiat = Point(verticesIat[i4]);
if (verticesMe[i4] != 0)
checkpme = Point(verticesMe[i4]);
}

Edge commonEdge(cv[0], cv[1]);
Line commonEdgeLine(commonEdge.getVector2D(), p1);

int sideiat = commonEdgeLine.getSide(checkpiat);
if (!SecureOperator::isSecureResult()) {
Vector2D_mp vmp = commonEdge.getVector2D_mp();

Line_mp common_mp(vmp, (Point_mp) p1);
sideiat = common_mp.getSide_mp((Point_mp) checkpiat);
}

int sideme = commonEdgeLine.getSide(checkpme);
if (!SecureOperator::isSecureResult()) {
Vector2D_mp vmp = commonEdge.getVector2D_mp();
Line_mp common_mp(vmp, (Point_mp) p1);
sideme = common_mp.getSide_mp((Point_mp) checkpme);
}

if (sideme != sideiat) {
return true;
} else {
return false;
}

} else if (commonVertexCnt == 1) //common Vertex
{
int oppositeVertexCnt = 0;
int insideVertexCnt = 0;
Vertex * oVertices[2];
Vertex * verticesForInsideCheck[2];

for (int i4 = 0; i4 < 3; i4++) {
if (verticesIat[i4] != 0)
verticesForInsideCheck[insideVertexCnt++] = verticesIat[i4];
if (verticesMe[i4] != 0)
oVertices[oppositeVertexCnt++] = verticesMe[i4];
}

Edge oEdge(oVertices[0], oVertices[1]);

for (int i5 = 1; i5 < 4; i5++) {
Edge eiat = iat.getEdge(i5);

if (oEdge.intersects_sec(&eiat)) {
return false;
}
}

//still one could be contained in the other
Point_p checkinsideme(oEdge.getV1());
Point_p checkinsideiat0(verticesForInsideCheck[0]);
Point_p checkinsideiat1(verticesForInsideCheck[1]);

if (iat.isInside_sec(checkinsideme) || this->isInside_sec(checkinsideiat0)
|| this->isInside_sec(checkinsideiat1))
return false;

return true;
}

//nothing in common
Edge e1 = iat.getEdge(1);
Edge e2 = iat.getEdge(2);
Edge e3 = iat.getEdge(3);

for (int i6 = 1; i6 < 4; i6++) {

//TODO check again

if (getEdge(i6).intersects_sec(&e1))
return false;
if (getEdge(i6).intersects_sec(&e2))
return false;
if (getEdge(i6).intersects_sec(&e3))
return false;
}
//still possible that one is contained in the other

Point_p p(iat.v1->getX(), iat.v1->getY());
Point_p p2(v1->getX(), v1->getY());
if (isInside_sec(p) || iat.isInside_sec(p2))
return false;

return true;

} else
return true;
}

Rectangle Triangle::bbox() const {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);

VERTEX_COORDINATE x2 = std::max(v1->getX(), v2->getX());
x2 = std::max(x2, v3->getX());
VERTEX_COORDINATE y2 = std::max(v1->getY(), v2->getY());
y2 = std::max(y2, v3->getY());
VERTEX_COORDINATE x1 = std::min(v1->getX(), v2->getX());
x1 = std::min(x1, v3->getX());
VERTEX_COORDINATE y1 = std::min(v1->getY(), v2->getY());
y1 = std::min(y1, v3->getY());

return Rectangle(x1, y1, x2, y2);

}

Edge Triangle::getEdge(const int n) const {
if (v1 == 0)
throw std::runtime_error(E_TRIANGLE_UNINITIALIZED);
switch (n) {
case 1:
return Edge(v1, v2);

case 2:
return Edge(v2, v3);

case 3:
return Edge(v3, v1);

default:
throw std::invalid_argument(E_TRIANGLE_GETEDGE);
}

}

}
