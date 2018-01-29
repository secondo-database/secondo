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

#ifndef TINPART_H_
#define TINPART_H_

#include "TinHelper.h"
#include "VertexContainerArray.h"
#include "VertexContainerSet.h"
#include "AbstractVertexContainer.h"
#include "Triangle.h"
#include  <vector>
#include <cmath>

#include <TinLogging.h>

namespace tin {
/*
 1 Class TinPart

*/
class TinPart: public noncopyable {
protected:
 TinFeatures features; //features of this part     (Header information)
 TIN_SIZE noVertices; //                                    (Header information)
 TIN_SIZE noTriangles; //number of
#ifndef UNIT_TEST
 SmiRecordId recId; //Record id for content data    (Header information)
 bool isRecInitialized; //whether content record exists   (not persisted)

#endif

 AbstractVertexContainer *pVertexContainer; //Vertices         (Content data)
 Triangle *arTriangles; //Triangles        (Content data)
 TIN_SIZE noTrianglesMax; //number of triangles
//maximum currently (not persisted)

 Tin *tin; //TIN this part belongs to     (not persisted)
 TinConfiguration config; //Configuration of this part   (not persisted)
 bool contentLoaded; //Indicates whether content is loaded (not persisted)
 mutable bool ismodified; //Indicates whether
//content is modified (not persisted)
 int contentRefs; //counts external references to content (not persisted)

 class atLocationRandomIndex {
 private:
  TIN_SIZE noIndexTriangles;
  TIN_SIZE step;
  TinPart* myPart;

  void build() {
   noIndexTriangles = cbrt(myPart->noTriangles);
   if (noIndexTriangles) {
    step = myPart->noTriangles / noIndexTriangles;
   } else if (myPart->noTriangles > 0)
    noIndexTriangles = 1;

  }

 public:
  atLocationRandomIndex(TinPart * p) {
   noIndexTriangles = 0;
   step = 0;
   myPart = p;
  }
  ~atLocationRandomIndex() {

  }

  Triangle* getAtLocationIndex(const Point_p & p) {
   const Vertex *v;
   Vector2D dist;
   Triangle * result = 0;

   double minDist = std::numeric_limits<double>::max();

   build();
   try {

    for (FILE_TRIANGLE_POINTER i = 0; i < noIndexTriangles; i++) {
     v = myPart->getTriangleByIndex(i * step)->getVertex(1);
     dist = p - *v;

     if (dist.getLength().u < minDist) {
      minDist = dist.getLength().u;
      result = myPart->getTriangleByIndex(i * step);
     }

    }
   } catch (std::exception & e) {
//might throw because of empty part...so index out of bounds
   }
   return result;
  }

 } idxAtLocation;

////////Construction/Destruction/////////////////////////////////
protected:
 TinPart() :
   idxAtLocation(this) {
  pVertexContainer = 0;
  arTriangles = 0;
  tin = 0;
  noTriangles = 0;
  noTrianglesMax = 0;
  noVertices = 0;
  contentRefs = 0;
  contentLoaded = true;
  ismodified = true;
#ifndef UNIT_TEST
  isRecInitialized = false;
#endif
 }
 TinPart(bool theEmptyConstructor) :
   features(true), pVertexContainer(0), arTriangles(0), config(true), 
   idxAtLocation(this) {

 }
public:
 virtual ~TinPart() {
  if (pVertexContainer != 0) {
   delete pVertexContainer;
   pVertexContainer = 0;
  }
  if (arTriangles) {
   delete[] arTriangles;
   arTriangles = 0;
  }

  noTriangles = 0;
 }

 static TinPart* getInstanceNew(Tin* tt, const TinConfiguration& conf =
   TinConfiguration::DEFAULT);
#ifndef UNIT_TEST
 static TinPart* getInstanceFromDisc(Tin* tt, SmiRecord& valueRecord,
   bool bulkload,
   const TinConfiguration& conf = TinConfiguration::DEFAULT);
 static TinPart* getInstanceFromDiscRandomAccess(Tin* tt,
   SmiRecord& valueRecord, const uint32_t idx,
   const TinConfiguration& conf = TinConfiguration::DEFAULT);
 static TinPart* getInstanceFromBuffer(Tin* tt, char* buffer,
   uint32_t &offset, bool bulkload, const TinConfiguration& conf);
#endif

 virtual TinPart* clone(Tin*tt);

protected:

 static TinPart* getInstanceNew_fortestonly(Tin* tt,
   const Vertex *iarTriangles, int inoTriangles, int& noTrianglesAdded,
   const TinConfiguration& conf = TinConfiguration::DEFAULT);
////////Manipulation/////////////////////////////////////////////
public:

 void overlay(Tin& tt, VertexContainerSet& vc);

#ifndef UNIT_TEST
 void unaryOp(void* function);
#endif

 TinPart* unaryOp(VERTEX_Z (*op)(VERTEX_Z), TinFeatures & feat);

 TinPart* binaryOp(AbstractTinType* tinResult,
   VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2));
 void updateFeatures();

 bool addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3,
   Triangle** newtriangle);
 bool addTriangle_p(const Vertex& v1, const Vertex& v2, const Vertex& v3,
   Triangle** newtriangle);
 bool addTriangle_p2(const Vertex& v1, const Vertex& v2, const Vertex& v3,
   Triangle** newtriangle);
protected:

 Triangle* constructTriangle(const Vertex* v1, const Vertex* v2,
   const Vertex* v3);

 void initContentMemory();
 void initContentMemory(TIN_SIZE inoVertices, TIN_SIZE inoTriangles);
 void freeContentMemory();

 void setModified(bool val = true) const {
  ismodified = val;
 }

///////Query/////////////////////////////////////////////////////
public:
 TIN_SIZE getNoTriangles() const {
  return noTriangles;
 }
 Triangle::triangleWalker getWalker(Point_p * path, int noPoints);
 static TIN_SIZE estimateNoTrianglesTotal(TIN_SIZE imaxSizePart) {
  return ((imaxSizePart
    - TinPart::estimateVertexContainerSizeOnDisc(imaxSizePart)
    - (TinFeatures::getSizeOnDisc() + sizeof(int)))
    / Triangle::getSizeOnDisc());
 }

 TIN_SIZE getSizeInMemory();
 static TIN_SIZE estimateMaxSizeInMemory(const TinConfiguration & conf);
 bool isModified() const {
  return ismodified;
 }
 class iterator {
 private:
  int currentTriangle;
  TinPart* tinpart;
 public:
  iterator() {
   currentTriangle = -1;
   tinpart = 0;
  }
  iterator(TinPart* itinpart) {
   if (itinpart)
    itinpart->loadContentData();

   if (itinpart && itinpart->noTriangles > 0)
    currentTriangle = 0;
   else
    currentTriangle = -1;

   tinpart = itinpart;
  }
  Triangle* operator*() {
   if (currentTriangle != -1)
    return &tinpart->arTriangles[currentTriangle];
   else
    return 0;
  }
  void operator++(int) {
   if (currentTriangle != -1 && currentTriangle + 1 < tinpart->noTriangles)
    currentTriangle++;
   else
    currentTriangle = -1;
  }

 };
 bool isValidTriangle(const Triangle & at, std::string & error_msg);

 void addVerticesTo(VertexContainerSet& vc);

 std::set<Edge>* getEdgeSet();
 const TinFeatures& getFeatures() const {
  return features;
 }
 virtual bool isContentDataLoaded() const {
  return contentLoaded;
 }

 Triangle * getTriangleArrayBaseAdress() const {
  return arTriangles;
 }

 Triangle * getTriangleByIndex(FILE_TRIANGLE_POINTER index) {
  if (index >= noTrianglesMax || index < 0)
   throw std::runtime_error(E_TINPART_GETTRIANGLEBYINDEX);

  return &arTriangles[index];
 }
 bool canAdd();

 VERTEX_Z atlocation_brute_force(const Point_p& p);
 VERTEX_Z atlocation_bywalker(const Point_p& p);

 Triangle* findEdgeInPart(const Edge & e, Triangle* caller);

 VERTEX_Z atlocation(const Point_p& p);
 void atlocation_noload(const Point_p& p, Triangle ** ret);
 const Rectangle& bbox() const;
 VERTEX_Z minimum() const;
 VERTEX_Z maximum() const;
 TinPart::iterator begin() {
  return TinPart::iterator(this);
 }

 bool hasTriangle(Vertex &v1, Vertex &v2, Vertex &v3);
 bool checkDelaunay();
 bool checkNeighborRelations();

 virtual Triangle* getNeighbor(const Edge& commonEdge, Triangle* caller);

protected:

///////Presentation//////////////////////////////////////////////
public:
#ifndef UNIT_TEST
 ListExpr outPart();
#endif

 friend std::ostream& operator<<(std::ostream & os, TinPart &a);

///////Persistence///////////////////////////////////////////////
public:

 virtual void loadContentData();
 virtual void unloadContentData();

 virtual void setMemoryState(MemoryState st) {
  LOGP
  config.memoryState = st;
 }

 virtual void deleteContentReference();
 virtual void addContentReference();

 static TIN_SIZE estimateVertexContainerSizeOnDisc(TIN_SIZE imaxSizePart);

 virtual TIN_SIZE getSizeOnDisc() const {
  LOGP
  TIN_SIZE size = getSizeOnDisc_head() + getSizeOnDisc_content();
  LOGP
  return size;
 }
 static TIN_SIZE getSizeOnDisc_head() {
  LOGP
  TIN_SIZE size = sizeof(TinPart::noTriangles)
    + sizeof(TinPart::noVertices) + TinFeatures::getSizeOnDisc();

#ifndef UNIT_TEST
  size += sizeof(TinPart::recId);

#endif
  LOGP
  return size;
 }
 virtual TIN_SIZE getSizeOnDisc_content() const {
  LOGP
  TIN_SIZE size = (
    config.abstractType == MANIPULATE ?
      VertexContainerSet::getSizeOnDiscStatic(noVertices) :
      VertexContainerArray::getSizeOnDiscStatic(noVertices))
    + noTriangles * Triangle::getSizeOnDisc();
  LOGP
  return size;
 }
#ifndef UNIT_TEST

 virtual bool save(SmiRecord& valueRecord);
 virtual void serialize(char* storage, size_t& offset);
 virtual void rebuild(char* state, size_t& offset);
protected:

 virtual void save_content();
 virtual void open_content();

 virtual bool open(SmiRecord& valueRecord);
 virtual bool open_head(SmiRecord& valueRecord, uint32_t idx);

#endif

/////////////////////////////////////////////////////////////////
 friend class TinPart_test;

 friend class TinAttribute;

};

} /* namespace tin*/
#endif /* TINPART_H_*/
