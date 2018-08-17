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

#ifndef TINHELPER_H_
#define TINHELPER_H_

#include <cmath>
#include <stdexcept>
#include <iostream>
#include <limits>
#include <string>
#include "stdint.h"
#include <gmp.h>
#include "SecondoDependencies.h"

////////Error messages/(due to lack of time not all of them)///////////
#define E_VERTEXCONTAINER_VERTEXCONTAINER \
"Number of vertices could not be read. (VertexContainer::VertexContainer)"
#define E_VERTEXCONTAINER_VERTEXCONTAINER2 \
"Vertex could not be read. (VertexContainer::VertexContainer)"
#define E_INDEXTRIANGLE_GETVERTEX \
"Vertex pointer could not be read. (IndexTriangle::getVertex)"
#define E_VERTEX_SETCOMPAREMETHOD \
"Unknown compare method.(Vertex::setCompareMethod)"
#define E_VERTEX_CONTAINER_GETVERTEX \
"Access to an unknown id.(Vertex::getVertex)"
#define E_VERTEXCONTAINER_WRITETORECORD \
"Number of vertices could not be written.(Vertex::writeToRecord)"
#define E_VERTEXCONTAINER_WRITETORECORD2 \
"Vertex could not be written.(Vertex::writeToRecord)"
#define E_VERTEXCONTAINER_LOADVERTICES \
"Null pointer as a parameter !(VertexContainer::loadVertices)"
#define E_VERTEXCONTAINER_LOADVERTICES2 \
"Maximum number of vertices in VertexContainer "\
"exceeded!(VertexContainer::loadVertices)"
#define E_VERTEXCONTAINER_INSERTVERTEX \
"Null pointer as a parameter !(VertexContaine"\
"r(Small)::insertVertex)"
#define E_VERTEXCONTAINER_INSERTVERTEX2 \
"Maximum number of vertices in VertexContainer"\
"exceeded!(VertexContainer(Small)::insertVertex)"
#define E_TRIANGLE_UNINITIALIZED \
"Called a function on uninitialized AbstractTriangle."
#define E_TRIANGLE_GETEDGE \
"Unknown edge. (AbstractTriangle::getEdge)"
#define E_INDEXTRIANGLE_INDEXTRIANGLE \
"Null pointer as a parameter ! (IndexTriangle::IndexTriangle)"
#define E_VERTEX_PARSEVERTEX \
"Tried to make a SimpleTriangle with a Vertex not consist" \
"ing of 3 elements!(Vertex::parseVertex)"
#define E_VERTEX_PARSEVERTEX2 \
"First element of a vertex is neither a rea" \
"l nor an integer!(Vertex::parseVertex)"
#define E_VERTEX_PARSEVERTEX3 \
"Second element of a vertex is neither a real no"\
"r an integer!(Vertex::parseVertex)"
#define E_VERTEX_PARSEVERTEX4 \
"Third element of a vertex is neither a re"\
"al nor an integer!(Vertex::parseVertex)"
#define E_ABSTRACTTINPART_ADDTRIANGLE \
"Triangle cannot be added to TinPart.(overlapping)"
#define E_RECTANGLE_SETXY \
"Tried to set invalid coordinate for Rectangle (Rectangle::Rectangle)"
#define E_EDGE_EDGE \
"Edge with length 0 is not allowed. (Edge::Edge)"
#define E_VERTEXCONTAINER_CONSTRUCTOR \
"VertexContainer is too small. Please increase size. " \
"The reason is possibly a very small part size. Try to" \
"increase the maximum size of the parts. (VertexContainerSet(Array))"
#define E_ABSTRACTTINPART_CONSTRUCTOR \
"TinPart constructed with undefined type. (TinPart::initMemoryContent)"
#define E_ABSTRACTTINPART_CONSTRUCTOR0 \
"TinPart constructed with null pointer for array of triangles. (TinPart::init)"
#define E_ABSTRACTTINPART_CONSTRUCTOR1 \
"TinPart constructed with null pointer for dummy triangle. (TinPart::init)"
#define E_ABSTRACTTINPART_CONSTRUCTOR3 \
"Static construction method is wrong.(TinPart::getInstanceFromDisc)"
#define E_ABSTRACTTINPART_INIT \
"Init part from records failed.(TinPart::getInstanceFromDisc)"
#define E_ABSTRACTTINPART_SAVE \
"Tried to persist on record, although mechanism is SERIALIZE.(TinPart::save)"
#define E_ABSTRACTTINPART_SERIALIZE \
"Tried to serialize, although mechanism is RECORD.(TinPart::serialize/rebuild)"
#define E_ABSTRACTTINPART_OPEN \
"Tried to open from record, although mechanism is SERIALIZE.(TinPart::open)"
#define E_TRIANGLE_GETVERTEX \
"Vertex range is 1-3. GetVertex called out of range.(TinPart::getVertex)"
#define E_TINTYPE_GETNEWPART \
"Unknown TinPartType (TinType::getNewPart)"
#define E_TINTYPE_SAVE \
"Tried to persist on record, although mechanism is SERIALIZE.(TinType::save)"
#define E_TINTYPE_SAVE1 \
"General error writing to record.(TinType::save)"
#define E_TINTYPE_OPEN \
"Tried to open from record, although mechanism is SERIALIZE.(TinType::open)"
#define E_TINTYPE_SERIALIZE \
"Tried to serialize, although mechanism is RECORD.(TinType::open)"
#define E_TINTYPE_GETFILE \
"Cannot provide file. Mechanism SERIALIZE (TinType::getFile)"
#define E_VECTOR2DDIR_OP_EQ \
"Null vector not allowed as input for == .(Vector2DDir::operator==)"
#define E_POINTMP_CONSTRUCTOR \
"Point_mp constructed with PreciseDouble having an" \
"error assigned.(Point_mp::Point_mp)"
#define E_VERTEX_CONSTRUCTOR \
"Z value cannot be the ERROR_VALUE std::numeric_limits<V" \
"ERTEX_Z>::min()(Vertex::Vertex)"
#define E_VERTEXCONTAINER_INSERTVERTEX3 \
"Vertex cannot be added. Same Vertex with different" \
"z coordinate already exists.(VertexContainer(Small)::InsertVertex)"
#define E_EDGE_EDGE0 \
"Constructor called with null pointer.(Edge::Edge)"
#define E_EDGE_INTERSECTION \
"Unexpected value 0 during calculation. 0 should not occur here, " \
"since parallel edges should have been treated above.(Edge::intersection_mp" \
")"
#define E_VERTEXCONTAINER_INSERTVERTICES \
"Maximum size of Container exceeded.(VertexContainer::insertVertices)"
#define E_ABSTRACTTINPART_CONSTRUCTOR2 \
"Null pointer for TinType is not allowed.(TinPart::getInstanceFromDisc)"
#define E_TINPART_GETINSTANCEFROMBUFFER \
"Null pointer for TinType is not allowed.(TinPart::getInstanceFromBuffer)"
#define E_INDEXTINPART_CONSTRUCTOR \
"IndexTinPart can only be constructed with IndexTriangles."\
"(IndexTinPart::IndexTinPart)"
#define E_VERTEXCONTAINERSMALL_GETVERTEXBYINDEX \
"Index out of bounds.(VertexContainerSmall::getVertexByIndex)"
#define E_VERTEXCONTAINER_GETVERTEXBYINDEX \
"Index out of bounds.(VertexContainer::getVertexByYIndex)"
#define E_VERTEXCONTAINER_GETVERTEXBYINDEX2 \
"Index out of bounds.(VertexContainer::getVertexByXIndex)"
#define E_TINTYPE_IN1 \
"The number of vertices has to be a multiple of 3" \
". 3 vertices each triangle. (Ti" \
"nType::In)"
#define E_TINTYPE_IN2 \
"There are no triangles in this part."\
"Expected: A list containing the triangles " \
"in the part.(Tin::In)"
#define E_TINATTRIBUTE_IN \
"There are no triangles in this part. Expecte"! \
"d: A list containing the triangles in the part.(TinAttribute::In)"
#define E_VERTEXCONTAINERSMALL_REMOVEVERTEX \
"This vertex container is not for manipulation." \
"Removal not possible.(VertexContainerSmall::removeVertex)"

#define E_BEACHLINENODE_CONSTRUCTOR \
"BeachLineNode (inner node)constructed w" \
"ith null pointer.(BeachLineNode::BeachLi" \
"neNode)"
#define E_BEACHLINENODE_CONSTRUCTOR1 \
"BeachLineNode (leaf) constructed with null point" \
"er.(BeachLineNode::BeachLineNod" \
"e)"
#define E_BEACHLINENODE_INSERT \
"Tried to insert null pointer. (BeachLineNode::insert)"
#define E_BEACHLINENODE_SETEVENT \
"Tried to overwrite event. (BeachLineNode::setEvent)"
#define E_BEACHLINENODE_INSERT1 \
"Tried to insert duplicate. (BeachLineNode::insert)"
#define E_BEACHLINENODE_INSERT2 \
"Tried to insert in inner node. Insert is only pos" \
"sible on leafs. (BeachLineNode" \
"::insert)"
#define E_BEACHLINENODE_REMOVE \
"Tried to remove a leaf directly with coordin" \
"ate. Please call parent. (BeachLine" \
"Node::remove)"
#define E_BEACHLINENODE_REMOVE1 \
"Tried to remove inner node. Inner nodes canno" \
"t be removed directly. (BeachLineN" \
"ode::remove)"
#define E_BEACHLINENODE_ROTATELEFT \
"Tried to rotate an incorrect node. Only inner node" \
"s with a left inner node can " \
"be rotated left. (BeachLineNode::rotateLeft)"
#define E_BEACHLINENODE_ROTATERIGHT \
"Tried to rotate an incorrect node. Only i" \
"nner nodes with a right inner node can" \
" be rotated right. (BeachLineNode::rotateRight)"
#define E_BEACHLINENODE_BALANCETREE \
"Tried to balance leaf. (BeachLineNode::balanceTree)"
#define E_BEACHLINENODE_BALANCETREE1 \
"Balancing failed. (BeachLineNode::balanceTree)"
#define E_BEACHLINE_INSERT1 \
"Tried to insert null pointer.(BeachLine::insert)"
#define E_BEACHLINE_INSERT \
"Tried to insert vertex below the sweep line.(BeachLine::insert)"
#define E_BEACHLINE_REMOVE \
"Tried to remove from empty beach line.(BeachLine::remove)"
#define E_BEACHLINENODE_GETNEIGHBOR \
"Neighbor only available for leafs.(BeachLineNode::getLeft(Right)Neighbor)"
#define E_PRECISEARITHMETIC_SQRT \
"Square root of negative number not allowed. (PreciseArithmetic::error_sqrt)"
#define E_CIRCLEEVENT_CALCULATESWEEPPOINT \
"Calculating circle with identical points. "\
"Three different points needed.(Circle" \
"Event::calculateSweepPoint)"
#define E_PARSEINTINCONFIG1 \
"Number of configuration parameters is not cor"\
"rect. Expected: First element of a" \
" tin is a list with an integer giving the maxim"\
"um size of the tin parts. (Tin::parseInTinConfig)"
#define E_PARSEINTINCONFIG2 \
"The configuration parameter is not an atom.(Tin::parseInTinConfig)"
#define E_PARSEINTINCONFIG3 \
"The configuration parameter has not the correct type. " \
"Expected: Integer(Tin::pa" \
"rseInTinConfig)"
#define E_ATLOCATION_TM \
"Operator atlocation expects two real coordinates"\
"x and y and a TinType. (mytin " \
"atlocation [1.0,2.0])"
#define E_VECTOR2D_ISBETWEEN0 \
"Vector2D isBetween called with null vector(Vector2D::isBetween)"
#define E_VECTOR2D_ISBETWEEN1 \
"Vector2D isBetween called with possibly (in"\
"secure due to rounding) co linear ve" \
"ctors(Vector2D::isBetween)"
#define E_LINE_LINE \
"Line constructed with null vector.(Line::Line)"
#define E_EVENTQUEUE_CHECKEVENTANDINSERT \
"Should never happen.(EventQueue::CheckEventAndInsert)"
#define E_CIRCLEEVENT_HANDLEEVENT \
"CircleEvent with zero length arc handled. The E"\
"vent should not have been added " \
"to queue in the first place.(CircleEvent::handleEvent)"
#define E_TIN_ADDTRIANGLE \
"Column in tin layout not found. Construction f"\
"ailed.(Tin::addTriangle)"
#define E_EVENTQUEUE_DOFORTUNEALGORITHM \
"Section is above prior section. Sections have"\
"to follow the direction top to bo" \
"ttom. Overlapping sections are not allo"\
"wed.(EventQueue::doFortuneAlgorithm)"
#define E_CREATETIN_TM \
"The operator createTin expects just a tuple "\
"stream as input and the size of the" \
" TinParts in bytes. Input is not just a tuple st"\
"ream and an Integer for the size of the TinParts."
#define E_CREATETIN_TM2 \
"The operator createTin expects a tuple stream "\
"consisting of tuples with exactly" \
" two attributes. (tuple(point real))"
#define E_CREATETIN_VM \
"The operator createTin requires a stream of po"\
"ints sorted by y coordinate in de" \
"scending order.(Tin::createtin_vm)"
#define E_TIN_SAVE \
"Saving failed due to null pointer. (Tin::Save)"
#define E_TIN_OUT \
"Out failed due to null pointer. (Tin::Out)"
#define E_TIN_OPEN \
"Content record of TinType could not be opened.(Tin::open)"
#define E_TIN_OPEN1 \
"File of TinType could not be opened.(Tin::open)"
#define E_TIN_OPEN2 \
"File of TinType could not be opened since it is damaged.(Tin::Open)"
#define E_TINATTRIBUTE_OPEN \
"File of TinAttribute could not be opened s"\
"ince it is damaged.(TinAttribute::Ope" \
"n)"
#define E_VERTEXCONTAINERSET_RESIZE \
"Resize under current size on disc is not possibl"\
"e.(VertexContainerSet::resize)"
#define E_TRIANGLE_ADDNEIGHBOR \
"Neighbor could not be added.Possible reasons: All"\
"neighbors already set/The tri" \
"angle is no neighbor.(Triangle::addNeighbor)"
#define E_TRIANGLE_ADDOPENEND \
"All three neighbors of the triangle are already set"\
". No more neighbors can be a" \
"dded.(Triangle::addOpenEnd)"
#define E_VORONOIEDGE_ADDEND \
"Both ends are already set. There must be an "\
"error.(VoronoiEdge::addEnd)"
#define E_VORONOIEDGE_ADDEND0 \
"Tried to add a null pointer.(VoronoiEdge::addEnd)"
#define E_TRIANGLE_PUTDISCREPRESENTATION \
"Putting triangle to disc failed. The limit of the tri"\
"angle pointers in the file" \
" is breached.(Triangle::putDiscRepresentation)"
#define E_TINPART_GETTRIANGLEBYINDEX \
"Index out of bounds.(TinPart::getTriangleByIndex)"

////////////////////////Standard parameter///////////////////////
#define VERTEX_CONTAINER_STANDARD_MAX_SIZE 2000
// about half a page ->not important any more
#define TIN_PART_STANDARD_SIZE 4000
// about a 4 kb page including the VertexContainer -> changeable
#define VERTEX_CONTAINER_BIG_SIZE   500000000
// 500 MB for overlay and construction -> changeable
#define TRIANGLES_PER_VERTEX    1.6
//important for TinPart since memory is
//divided according to this factor
//-> changeable (only values between 1 and 2 are reasonable)
#define OPERATE_IN_MEMORY_THRESHOLD   10000000
// 10 MB a rough threshold in bytes fo
//r operators to start putting stuff to disc temporarily
//small pointers on disc used to make small par
//ts with page size efficient
//-> flip side is that the size of the pa
//rts is restricted
//-> this can be adjusted here
#define MAX_CONFIG_SIZE      1500000
//due to 16 bit pointer value FILE_TRIAN
//GLE/VERTEX_POINTER no bigger size addressable

#define MIN_CONFIG_SIZE      800
//due to overhead -> dont use smaller parts
typedef uint16_t FILE_TRIANGLE_POINTER;
typedef uint16_t FILE_VERTEX_POINTER;
#define FILE_TRIANGLE_POINTER_BOUNDARY  65000
// special value for the boundary
#define FILE_TRIANGLE_POINTER_UNKNOWN  65001
// special value for errors during fortunes algorithm
//-> there is little harm, just more calculations
/////////////////////////////////////////////////////////////////

#define ERROR_VALUE -std::numeric_limits<VERTEX_Z>::max()
#define FILE_DAMAGED 1
#define FILE_OK 0
#define VORONOI_OPEN_END (Triangle*)1

////////////////////////Logging /////////////////////////////////
/*
 Define just the macro LOGGING and full logging will be activated.
 Define just the macro LOGGINGP and many code positions will be logged,
 but no comprehensive information will be shown. Standard output is std::cout.
 This can be changed in TinLogging.h .

*/

//log what
//#define LOGGING
//#define LOGGINGP
/*
 Define the places where no logging shall be done. This is especially useful
 to switch off the abundant logging of Fortune's Algorithm.

*/
//log where (switch of some places)
#define FORTUNE_NO_LOGGING
//#define TRIANGLE_NO_LOGGING

//general logging definition
#include "TinLogging.h"

//////////////////////////////////////////////////////////////////

#define MPQ_OUT(q) std::cout<<mpq_get_d(q)<<"\n";
#define MSG_TO_USER(msg) std::cout<<msg<<"\n";
#define OUT_EXCEPT(except) cmsg.otherError(except.what());
namespace tin {

enum MECHANISM {
 RECORD, SERIALIZE
};
enum MemoryState {
 GRADUALFILE, GRADUALTEMPORARYFILE, INMEMORY, RANDOMACCESS
};
enum AbstractType {
 QUERY, MANIPULATE
};
enum TinPartType {
 INDEX
};
std::string AbstractTypeToString(AbstractType t);
void max(mpq_t& result, mpq_t &v1, mpq_t &v2);

typedef double PRECISE_VAL;
#define PRECISE_VAL_LAST_BIT    0x1;
typedef int ERROR_VAL;

class PreciseDouble {
public:

 PRECISE_VAL u;
 PRECISE_VAL l;
 PreciseDouble();
 ~PreciseDouble();
 PreciseDouble(PRECISE_VAL i);
 PreciseDouble(const PreciseDouble & pd);
 explicit PreciseDouble(const mpq_t& v);

 PRECISE_VAL getRoundedVal();
 PreciseDouble operator-() const;
 PreciseDouble abs() const;
 void makeAbs();
 PreciseDouble operator+(const PreciseDouble & b) const;
//PreciseDouble operator+(const int & b)const;
 PreciseDouble operator-(const PreciseDouble & b) const;
//PreciseDouble operator-(const int & b)const;
 PreciseDouble operator*(const PreciseDouble & b) const;
 PreciseDouble operator/(const PreciseDouble & b) const;
 PreciseDouble& operator=(const PreciseDouble & b);
 PreciseDouble& operator=(const int & b);
 PreciseDouble& operator=(const double & b);
 PreciseDouble& operator+=(const PreciseDouble & b);
 PreciseDouble& operator-=(const PreciseDouble & b);
 bool operator<(const PreciseDouble & b) const;
 bool operator>(const PreciseDouble & b) const;
 bool operator>=(const PreciseDouble & b) const;
 bool operator<=(const PreciseDouble & b) const;
 bool operator==(const PreciseDouble & b) const;
 bool hasNoError() const;
 friend std::ostream& operator<<(std::ostream& os, const PreciseDouble &v);

};

typedef int16_t FILE_VERTEXCONTAINER_NUMBER_VERTICES;
typedef int32_t FILE_TINTYPE_NUMBER_TRIANGLECONTAINER;
typedef int16_t VERTEX_ID;

typedef double VERTEX_Z;
typedef double FILE_VERTEX_Z;

typedef double VERTEX_COORDINATE;
typedef double FILE_VERTEX_COORDINATE;

typedef PreciseDouble POINT_COORDINATE;

typedef PreciseDouble VECTOR_COMPONENT;
typedef PreciseDouble VECTOR_DIR;

typedef int64_t TIN_SIZE;

const PRECISE_VAL VERTICAL_UP =
  std::numeric_limits<PRECISE_VAL>::infinity();
const PRECISE_VAL VERTICAL_DOWN =
  -std::numeric_limits<PRECISE_VAL>::infinity();

float absolute(float n);

double absolute(double n);

PreciseDouble absolute(PreciseDouble n);

int absolute(int n);

class SecureOperator {
private:
 static bool secure;
public:

 SecureOperator() {

 }
 ;
 ~SecureOperator() {

 }
 ;
 static void startSecureCalc() {
  secure = true;
 }
 static void setInsecure() {
  secure = false;
 }
 static void setSecure(bool isec) {
  secure = isec;
 }
 static bool isSecureResult() {
  return secure;
 }

 static const char* toString() {
  return (secure ? "Result is secure." : "Result is NOT secure.");
 }
};

class PreciseArithmetic {
public:
 static void error_mul(const PreciseDouble &a, const PreciseDouble &b,
   PreciseDouble &result);
 static void error_add(const PreciseDouble &a, const PreciseDouble &b,
   PreciseDouble &result);
 static void error_add(const int a, const PreciseDouble &b,
   PreciseDouble &result);
 static void error_div(const PreciseDouble &a, const PreciseDouble &b,
   PreciseDouble &result);
 static void error_sqrt(const PreciseDouble &a, PreciseDouble &result);

};

class PreciseComp: public SecureOperator {
//TODO Precondition infinity +- exist check
//TODO write test
public:
 static const double MACHINE_EPSILON;

public:

 static bool eq(const PreciseDouble& v1, const PreciseDouble& v2) {
//assumed input is not NaN

  if (v1.hasNoError() && v2.hasNoError())
   return v1.l == v2.l;

  if (v1.u == std::numeric_limits<PRECISE_VAL>::infinity()
    && v2.u == std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v1.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    && v2.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v1.u == std::numeric_limits<PRECISE_VAL>::infinity()
    || v1.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    || v2.u == std::numeric_limits<PRECISE_VAL>::infinity()
    || v2.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return false;

  if (v1.u >= v2.l && v1.l <= v2.u) {
   SecureOperator::setInsecure();
   return true;
  }

  return false;
 }
 ;
 static bool ge(const PreciseDouble& v1, const PreciseDouble& v2) {
//assumed input is not NaN

  if (v1.hasNoError() && v2.hasNoError())
   return v1.l >= v2.l;

  if (v2.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v1.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v1.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    || v2.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return false;

  if (v1.l >= v2.u) {
   return true;
  }

  if (v1.u >= v2.l) {
   SecureOperator::setInsecure();
   return true;
  }

  return false;

 }
 ;
 static bool le(const PreciseDouble& v1, const PreciseDouble& v2) {
//assumed input is not NaN
  if (v1.hasNoError() && v2.hasNoError())
   return v1.l <= v2.l;

  if (v1.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v2.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return true;
  if (v2.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    || v1.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return false;

  if (v1.u <= v2.l) {
   return true;
  }

  if (v1.l <= v2.u) {
   SecureOperator::setInsecure();
   return true;
  }

  return false;

 }
 ;
 static bool lt(const PreciseDouble& v1, const PreciseDouble& v2) {
//assumed input is not NaN

  if (v1.hasNoError() && v2.hasNoError())
   return v1.l < v2.l;

  if (v2.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return false;
  if (v1.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return false;
  if (v1.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    || v2.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return true;

  if (v1.u < v2.l) {
   return true;
  }

  if (v1.l < v2.u) {
   SecureOperator::setInsecure();
   return true;
  }

  return false;

 }
 ;
 static bool gt(const PreciseDouble& v2, const PreciseDouble& v1) {
//assumed input is not NaN

  if (v1.hasNoError() && v2.hasNoError())
   return v1.l < v2.l;

  if (v2.u == -std::numeric_limits<PRECISE_VAL>::infinity())
   return false;
  if (v1.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return false;
  if (v1.u == -std::numeric_limits<PRECISE_VAL>::infinity()
    || v2.l == std::numeric_limits<PRECISE_VAL>::infinity())
   return true;

  if (v1.u < v2.l) {
   return true;
  }
  if (v1.l < v2.u) {
   SecureOperator::setInsecure();
   return true;
  }

  return false;
 }
 ;

};

class Vector2D;
class Vertex;
class Vector2D_mp;
class Point_mp;
class Point;
class Tin;
class VertexContainerSet;
class Triangle;

class Point_p {
public:
 VERTEX_COORDINATE x;
 VERTEX_COORDINATE y;

 Point_p();
 Point_p(VERTEX_COORDINATE ix, VERTEX_COORDINATE iy);
 explicit Point_p(const Point& pt);
 explicit Point_p(const Vertex& c);
 Vector2D operator -(const Vertex& v) const;
 bool operator<(const Point_p& p) const {
  if (y < p.y)
   return true;
  if (y > p.y)
   return false;

  if (x < p.x)
   return true;

  return false;
 }
 Vector2D_mp minus2D_mp(const Vertex& v) const;
 Point_p& operator=(const Point_mp& mpp);
 bool operator==(const Point_p & p);
 void print(std::ostream& os = std::cout) const {
  os << "\n x: " << x << " y: " << y;
 }
 ;
};

class Rectangle {
private:
 VERTEX_COORDINATE m_x1;
 VERTEX_COORDINATE m_y1;
 VERTEX_COORDINATE m_x2;
 VERTEX_COORDINATE m_y2;
public:
 Rectangle() {
  m_x1 = 0;
  m_y1 = 0;
  m_x2 = 0;
  m_y2 = 0;
 }
 ;

 Rectangle(bool theEmptyOne) {

 }
 VERTEX_COORDINATE getX1() const {
  return m_x1;
 }

 VERTEX_COORDINATE getX2() const {
  return m_x2;
 }

 VERTEX_COORDINATE getY1() const {
  return m_y1;
 }

 VERTEX_COORDINATE getY2() const {
  return m_y2;
 }

 Rectangle(VERTEX_COORDINATE x1, VERTEX_COORDINATE y1,
   VERTEX_COORDINATE x2, VERTEX_COORDINATE y2) {
  if (!(x1 < x2 && y1 < y2))
   throw std::invalid_argument(E_RECTANGLE_SETXY);
  m_x1 = x1;
  m_x2 = x2;
  m_y1 = y1;
  m_y2 = y2;
 }

 bool hasIntersection(const Rectangle &r) const {
  return ((r.m_x1 <= m_x2 && r.m_x2 >= m_x1)
    && (r.m_y1 <= m_y2 && r.m_y2 >= m_y1));
 }
 bool operator==(const Rectangle &r) const {
  if (m_x1 != r.m_x1 || m_y1 != r.m_y1 || m_x2 != r.m_x2 || m_y2 != r.m_y2)
   return false;

  return true;
 }
 bool contains(const Point_p& p) const {
  if ((p.x >= m_x1) && (p.x <= m_x2) && (p.y >= m_y1) && (p.y <= m_y2))
   return true;
  else
   return false;
 }
 static TIN_SIZE getSizeOnDisc() {
  return 4*sizeof(VERTEX_COORDINATE);
         // m_x1 , m_y1, m_x2, m_y2
 }

#ifndef UNIT_TEST
 void serialize(char* storage, size_t& offset) const {
  WriteVar<VERTEX_COORDINATE>(m_x1, storage, offset);
  WriteVar<VERTEX_COORDINATE>(m_y1, storage, offset);
  WriteVar<VERTEX_COORDINATE>(m_x2, storage, offset);
  WriteVar<VERTEX_COORDINATE>(m_y2, storage, offset);

 }
 void rebuild(char* state, size_t & offset) {
  ReadVar<VERTEX_COORDINATE>(m_x1, state, offset);
  ReadVar<VERTEX_COORDINATE>(m_y1, state, offset);
  ReadVar<VERTEX_COORDINATE>(m_x2, state, offset);
  ReadVar<VERTEX_COORDINATE>(m_y2, state, offset);
 }
 bool open(SmiRecord& valueRecord) {
  SmiSize noBytesTransferred = 0;
  noBytesTransferred = valueRecord.Read(m_x1);
  if (noBytesTransferred != sizeof(m_x1))
   return false;
  noBytesTransferred = valueRecord.Read(m_y1);
  if (noBytesTransferred != sizeof(m_y1))
   return false;
  noBytesTransferred = valueRecord.Read(m_x2);
  if (noBytesTransferred != sizeof(m_x2))
   return false;
  noBytesTransferred = valueRecord.Read(m_y2);
  if (noBytesTransferred != sizeof(m_y2))
   return false;
  return true;

 }
 ;

 bool save(SmiRecord& valueRecord) {
  SmiSize noBytesTransferred = 0;
  noBytesTransferred = valueRecord.Write(m_x1);
  if (noBytesTransferred != sizeof(m_x1))
   return false;
  noBytesTransferred = valueRecord.Write(m_y1);
  if (noBytesTransferred != sizeof(m_y1))
   return false;
  noBytesTransferred = valueRecord.Write(m_x2);
  if (noBytesTransferred != sizeof(m_x2))
   return false;
  noBytesTransferred = valueRecord.Write(m_y2);
  if (noBytesTransferred != sizeof(m_y2))
   return false;

  return true;
 }
 ;

 ListExpr outRectangle() {
  return nl->TwoElemList(
    nl->StringAtom("Rectangle (left bottom right top):"),
    nl->FourElemList(nl->RealAtom(m_x1), nl->RealAtom(m_y1),
      nl->RealAtom(m_x2), nl->RealAtom(m_y2)));
 }
#endif
 void setX1(VERTEX_COORDINATE x1) {
  m_x1 = x1;
 }

 void setX2(VERTEX_COORDINATE x2) {
  m_x2 = x2;
 }

 void setY1(VERTEX_COORDINATE y1) {
  m_y1 = y1;
 }

 void setY2(VERTEX_COORDINATE y2) {
  m_y2 = y2;
 }

 void print(std::ostream & os = std::cout) const {
  os << " x1: " << m_x1 << " y1: " << m_y1 << " x2: " << m_x2 << " y2: "
    << m_y2 << "\n";
 }
};

class AbstractTriangle;
class AbstractTinType;

class TinFeatures {
public:
 VERTEX_Z m_maxValue;
 VERTEX_Z m_minValue;
 Rectangle bbox;
 TinFeatures(bool theEmptyOne) :
   bbox(theEmptyOne) {

 }
 TinFeatures() {
  bbox.setX1(std::numeric_limits<VERTEX_Z>::max());
  bbox.setX2(-std::numeric_limits<VERTEX_Z>::max());
  bbox.setY1(std::numeric_limits<VERTEX_Z>::max());
  bbox.setY2(-std::numeric_limits<VERTEX_Z>::max());

  m_maxValue = (-std::numeric_limits<VERTEX_Z>::max());
  m_minValue = std::numeric_limits<VERTEX_Z>::max();
 }
 void update(const Triangle * at);
 void update(const TinFeatures & f);
 void update(const Vertex& v);
 bool operator==(const TinFeatures & feat) const {
  if (m_maxValue != feat.m_maxValue || m_minValue != feat.m_minValue)
   return false;

  if (!(bbox == feat.bbox))
   return false;

  return true;
 }
#ifndef UNIT_TEST
 void rebuild(char* state, size_t &offset) {
  ReadVar<VERTEX_Z>(m_minValue, state, offset);
  ReadVar<VERTEX_Z>(m_maxValue, state, offset);

  bbox.rebuild(state, offset);
 }
 void serialize(char* storage, size_t& offset) const {
  WriteVar<VERTEX_Z>(m_minValue, storage, offset);
  WriteVar<VERTEX_Z>(m_maxValue, storage, offset);

  bbox.serialize(storage, offset);
 }
 bool open(SmiRecord& valueRecord) {
  SmiSize noBytesTransferred = 0;
  noBytesTransferred = valueRecord.Read(m_minValue);
  if (noBytesTransferred != sizeof(m_minValue))
   return false;
  noBytesTransferred = valueRecord.Read(m_maxValue);
  if (noBytesTransferred != sizeof(m_maxValue))
   return false;

  if (!bbox.open(valueRecord))
   return false;

// this->print();
  return true;

 }
 ;
 bool save(SmiRecord& valueRecord) {
  SmiSize noBytesTransferred = 0;

  noBytesTransferred = valueRecord.Write(m_minValue);
  if (noBytesTransferred != sizeof(m_minValue))
   return false;
  noBytesTransferred = valueRecord.Write(m_maxValue);
  if (noBytesTransferred != sizeof(m_maxValue))
   return false;

  if (!bbox.save(valueRecord))
   return false;

  return true;

 }
 ;

 ListExpr outFeatures() {

  return nl->TwoElemList(
    nl->StringAtom("TinFeatures (minValue maxValue bbox):"),
    nl->ThreeElemList(nl->RealAtom(m_minValue), nl->RealAtom(m_maxValue),
      bbox.outRectangle()));
 }
#endif
 void reset() {
  bbox.setX1(std::numeric_limits<VERTEX_Z>::max());
  bbox.setX2(-std::numeric_limits<VERTEX_Z>::max());
  bbox.setY1(std::numeric_limits<VERTEX_Z>::max());
  bbox.setY2(-std::numeric_limits<VERTEX_Z>::max());

  m_maxValue = (-std::numeric_limits<VERTEX_Z>::max());
  m_minValue = std::numeric_limits<VERTEX_Z>::max();
 }
 void print(std::ostream& os = std::cout) const {
  os << "TinFeatures-----\n" << "maximum value: " << m_maxValue
    << " minimum value: " << m_minValue << " bbox: ";
  bbox.print(os);
 }

 static TIN_SIZE getSizeOnDisc() {
  return Rectangle::getSizeOnDisc() + 2*sizeof(VERTEX_Z);
        //  m_minValue , m_maxValue
 }

};

class Point: public SecureOperator {
public:
 POINT_COORDINATE x;
 POINT_COORDINATE y;

public:
 Point() {
  x = 0;
  y = 0;
 }

 Point(POINT_COORDINATE ix, POINT_COORDINATE iy) {
  x = ix;
  y = iy;
 }
 explicit Point(const Point_p& mp);
 Point(const Vertex * v);

 explicit Point(const Vertex& v);

 ~Point() {
 }

 Point operator +(const Vector2D& v) const;
 Vector2D operator -(const Vertex& v) const;
 Vector2D_mp minus2D_mp(const Vertex& v) const;
 Point& operator=(const Point_mp& mpp);
 Point& operator=(const Vertex& v);
 bool operator<(const Point &p) const;

 void print(std::ostream& out = std::cout);

};

class Point_mp {
public:
 mpq_t x;
 mpq_t y;

public:
 Point_mp();
 Point_mp(POINT_COORDINATE ix, POINT_COORDINATE iy);
 Point_mp(VERTEX_COORDINATE ix, VERTEX_COORDINATE iy);
 Point_mp(const Vertex * v);
 Point_mp(const Point_mp& p);
 explicit Point_mp(const Point& p);
 explicit Point_mp(const Point_p& p);
 ~Point_mp();

 Point_mp& operator=(const Point_mp& p);
 Point_mp& operator=(const Vertex& v);
 Point_mp operator+(const Vector2D_mp& v) const;
 Vector2D_mp minus2D_mp(const Vertex& v) const;
 void print(std::ostream& out = std::cout);

};

class noncopyable {
protected:
 noncopyable() {
 }
 ~noncopyable() {
 }
private:
 noncopyable(const noncopyable&);
 const noncopyable& operator=(const noncopyable&);
};

class Op {
public:
 static VERTEX_Z constant; //TODO make thread_local
public:
 static VERTEX_Z add_const(VERTEX_Z z) {
  return z + constant;
 }

 static VERTEX_Z mul_const(VERTEX_Z z) {
  return z * constant;
 }
public:
 static VERTEX_Z add(VERTEX_Z z1, VERTEX_Z z2) {
  return z1 + z2;
 }
};

class TinConfiguration {
public:
 TIN_SIZE maxSizePart;
 AbstractType abstractType;
 MemoryState memoryState;

 static const TinConfiguration DEFAULT;
 static const TinConfiguration ATTRIBUTE;
 TinConfiguration(bool theEmptyOne) {

 }
 TinConfiguration(TIN_SIZE imaxSizePart = TIN_PART_STANDARD_SIZE,
   AbstractType iabstractType = MANIPULATE,
   MemoryState memstate = INMEMORY) {
  maxSizePart = imaxSizePart;
  abstractType = iabstractType;
  memoryState = memstate;
 }

 bool operator==(const TinConfiguration & rhs) const {
  if (maxSizePart == rhs.maxSizePart && abstractType == rhs.abstractType
    && memoryState == rhs.memoryState)
   return true;
  else
   return false;

 }
};

}
#endif /* TINHELPER_H_*/
