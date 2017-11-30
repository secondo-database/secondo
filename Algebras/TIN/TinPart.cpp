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
#include "Tin.h"
#include "TinPart.h"
#include  <vector>
#include "AlgoFortune.h"
#include <sstream>
#include "SecondoDependencies.h"
#include <typeinfo>

#include <TinLogging.h>

namespace tin {

#ifndef UNIT_TEST
void TinPart::unaryOp(void* function) {
 loadContentData();

 features.reset();
 pVertexContainer->unaryOp(function, features);
 setModified();
}
#endif

void TinPart::addVerticesTo(VertexContainerSet& vc) {
 loadContentData();
 pVertexContainer->addVerticesTo(vc);
}
#ifndef UNIT_TEST
ListExpr TinPart::outPart() {
 LOGP
 ListExpr last, first;

 loadContentData();

 if (noTriangles > 0) {
  first = nl->OneElemList(arTriangles[0].outTriangle());
 } else {
  LOGP
  return nl->Empty();
 }

 last = first;

 for (int i = 1; i < noTriangles; i++) {
  last = nl->Append(last, arTriangles[i].outTriangle());
 }

 LOGP
 LOG(nl->ToString(first))
 return first;
}

void TinPart::rebuild(char* state, size_t& offset) {
 LOGP

 ReadVar<SmiRecordId>(recId, state, offset);
 ReadVar<TIN_SIZE>(noTriangles, state, offset);
 ReadVar<TIN_SIZE>(noVertices, state, offset);
 features.rebuild(state, offset);

 isRecInitialized = true;

LOG(recId)
LOG(noTriangles)
LOG(noVertices)

}
void TinPart::serialize(char* storage, size_t& offset) {
 LOGP
 SmiRecord rec;

 if (!isRecInitialized) {
  tin->getFile()->AppendRecord(recId, rec);
  isRecInitialized = true;
  rec.Finish();
 }

 if (isContentDataLoaded())
  noVertices = pVertexContainer->getNoVertices();

 WriteVar<SmiRecordId>(recId, storage, offset);
 WriteVar<TIN_SIZE>(noTriangles, storage, offset);
 WriteVar<TIN_SIZE>(noVertices, storage, offset);

 features.serialize(storage, offset);

 if (isContentDataLoaded())
  save_content();

 LOGP

 setModified(false);
}
bool TinPart::save(SmiRecord& valueRecord) {
 if (config.memoryState == INMEMORY || config.memoryState == RANDOMACCESS)
  throw std::runtime_error(
    "Tried to save a tin part in state INMEMORY"
    " ¦¦ RANDOMACCESS.(TinPart::save)");

 LOGP

 SmiRecord rec;
 SmiSize noBytesTransferred = 0;
 SmiSize currentPos;

 if (isModified()) {
  if (!isRecInitialized) {
   tin->getFile()->AppendRecord(recId, rec);
   isRecInitialized = true;
   rec.Finish();
  }
  noBytesTransferred = valueRecord.Write(recId);

  if (noBytesTransferred != sizeof(recId))
   return false;

  noBytesTransferred = valueRecord.Write(noTriangles);
  if (noBytesTransferred != sizeof(noTriangles))
   return false;

  if (isContentDataLoaded())
   noVertices = pVertexContainer->getNoVertices();

  noBytesTransferred = valueRecord.Write(noVertices);
  if (noBytesTransferred != sizeof(noVertices))
   return false;

  if (!features.save(valueRecord))
   return false;

  if (isContentDataLoaded())
   save_content();

  setModified(false);

LOGP} else {
 currentPos = valueRecord.GetPos();
 currentPos += TinPart::getSizeOnDisc_head();
 valueRecord.SetPos(currentPos);
}

 return true;
}
bool TinPart::open(SmiRecord& valueRecord) {
 SmiSize noBytesTransferred = 0;
 noBytesTransferred = valueRecord.Read(recId);
 if (noBytesTransferred != sizeof(recId))
  return false;
 isRecInitialized = true;

 noBytesTransferred = valueRecord.Read(noTriangles);
 if (noBytesTransferred != sizeof(noTriangles))
  return false;

 noBytesTransferred = valueRecord.Read(noVertices);
 if (noBytesTransferred != sizeof(noVertices))
  return false;

 if (!features.open(valueRecord))
  return false;

 LOG(recId)
 LOG(noTriangles)
 LOG(noVertices)
 return true;
}
bool TinPart::open_head(SmiRecord& valueRecord, uint32_t idx) {
 SmiSize noBytesTransferred = 0;
 LOGP
 LOG(idx * getSizeOnDisc_head()+tin->getPartHeadOrigin())

 valueRecord.SetPos(idx * getSizeOnDisc_head() + tin->getPartHeadOrigin());

 noBytesTransferred = valueRecord.Read(recId);
 if (noBytesTransferred != sizeof(recId))
  return false;
 isRecInitialized = true;

 LOG(recId)

 noBytesTransferred = valueRecord.Read(noTriangles);
 if (noBytesTransferred != sizeof(noTriangles))
  return false;

 LOG(noTriangles)

 noBytesTransferred = valueRecord.Read(noVertices);
 if (noBytesTransferred != sizeof(noVertices))
  return false;

 LOG(noVertices)

 if (!features.open(valueRecord))
  return false;

 LOGP
 return true;
}

void TinPart::save_content() {
 LOGP
 SmiRecord rec;
 SmiSize noBytesTransferred = 0;
 SmiSize sz = 0;
 SmiSize offset = 0;

 if (config.memoryState == INMEMORY)
  throw std::runtime_error(
    "Tried to save a tin part in state stay in memory.(TinPart::save_content)");

 if (!isRecInitialized) {
  tin->getFile()->AppendRecord(recId, rec);
  isRecInitialized = true;
 } else
  tin->getFile()->SelectRecord(recId, rec, SmiFile::Update);

 sz = getSizeOnDisc_content();
 char * buffer = new char[sz];

 pVertexContainer->serialize(buffer, offset);

 LOG("Number of triangles:")
 LOG(noTriangles)
 LOGP

 for (int i = 0; i < noTriangles; i++) {
  LOG(i)
  arTriangles[i].putSecondoRepresentation(pVertexContainer, buffer,
    (uint32_t &) offset);
 }
 LOGP

 noBytesTransferred = rec.Write(buffer, sz, 0);
 if (noBytesTransferred != sz)
  throw std::runtime_error(
    "Error writing triangles to file.(TinPart::save_content)");

 delete[] buffer;

LOGP}
void TinPart::open_content() {
 SmiSize sz = 0;
 SmiSize offset = 0;
 SmiRecord rec;
 tin->getFile()->SelectRecord(recId, rec, SmiFile::ReadOnly);

 sz = getSizeOnDisc_content();
 char * buffer = new char[sz];

 rec.Read(buffer, sz, 0);
 pVertexContainer->rebuild(buffer, offset);

 LOGP
 if (noTriangles) {
  LOG(noTriangles)
  LOGP
  for (int i = 0; i < noTriangles; i++) {
   LOG(i)
   new (&arTriangles[i]) Triangle(pVertexContainer, buffer,
     (uint32_t &) offset, this);
  }

LOGP}

 delete[] buffer;

 contentLoaded = true;

LOGP}

#endif
std::set<Edge>* TinPart::getEdgeSet() {

 std::set<Edge>* setEdges = new std::set<Edge>();
 loadContentData();

 for (int i = 0; i < noTriangles; i++) {
  setEdges->insert(this->arTriangles[i].getEdge(1));
  setEdges->insert(this->arTriangles[i].getEdge(2));
  setEdges->insert(this->arTriangles[i].getEdge(3));
 }
 return setEdges;

}
bool TinPart::canAdd() {
 if (!pVertexContainer)
  return false;

 if (pVertexContainer->countVerticesAddable() < 3)
//try to move memory to vertex container
   {
  TIN_SIZE memtomove = (3 - pVertexContainer->countVerticesAddable())
    * Vertex::getSizeOnDisc();
  TIN_SIZE trianglestoreduce = memtomove / Triangle::getSizeOnDisc();

  trianglestoreduce++;

  if ((noTrianglesMax - noTriangles - 1) >= trianglestoreduce) {
   pVertexContainer->resize(pVertexContainer->getMaxSize() + memtomove);
   noTrianglesMax -= trianglestoreduce;
  } else
   return false;

 }
 if (noTrianglesMax == noTriangles) {
  return false;

 }

 return true;

}

void TinPart::updateFeatures() {

 if (!isContentDataLoaded())
  loadContentData();

 if (typeid(*pVertexContainer) != typeid(VertexContainerSet))
  throw std::runtime_error(
    "This only works for VertexContainerSet !(TinPart::updateFeatures)");

 VertexContainerSet * set =
   static_cast<VertexContainerSet*>(pVertexContainer);

 features.update(set->getFeatures());

}
bool TinPart::isValidTriangle(const Triangle & at,
  std::string & error_msg) {

 loadContentData();

 std::stringstream msg;

 for (int i = 0; i < noTriangles; i++) {
  if (!this->arTriangles[i].isCompatibleWith_sec(at)) {
   msg << "Existing triangle: \n ";
   arTriangles[i].print(msg);
   msg << " is not compatible with : \n";
   at.print(msg);
   error_msg = msg.str();
   return false;
  }

 }

 return true;
}
bool TinPart::addTriangle(const Vertex& v1, const Vertex& v2,
  const Vertex& v3, Triangle** newtriangle) {

 bool nv1, nv2, nv3;
 std::string error_msg;
 const Vertex * pVr, *pV2r, *pV3r;
 Triangle *triangle2add;

 if (canAdd()) {

  pVr = pVertexContainer->insertVertex(&v1, nv1);
  pV2r = pVertexContainer->insertVertex(&v2, nv2);
  pV3r = pVertexContainer->insertVertex(&v3, nv3);

  triangle2add = constructTriangle(pVr, pV2r, pV3r);

  if (isValidTriangle(*triangle2add, error_msg)) {

   features.update(triangle2add);
   *newtriangle = triangle2add;
   noVertices = pVertexContainer->getNoVertices();
   setModified();
  } else {

   if (config.abstractType == MANIPULATE) {
    if (nv3)
     pVertexContainer->removeVertex(pV3r);
    if (nv2)
     pVertexContainer->removeVertex(pV2r);
    if (nv1)
     pVertexContainer->removeVertex(pVr);
   }

//std::cout<<*this;
   throw std::invalid_argument(E_ABSTRACTTINPART_ADDTRIANGLE + error_msg);
  }
  noTriangles++;
  return true;
 } else
  return false;
}
bool TinPart::addTriangle_p(const Vertex& v1, const Vertex& v2,
  const Vertex& v3, Triangle** newtriangle) {

 bool nv1, nv2, nv3;
 const Vertex * pVr, *pV2r, *pV3r;
 Triangle *triangle2add;

 if (canAdd()) {

  pVr = pVertexContainer->insertVertex(&v1, nv1);
  pV2r = pVertexContainer->insertVertex(&v2, nv2);
  pV3r = pVertexContainer->insertVertex(&v3, nv3);

  triangle2add = constructTriangle(pVr, pV2r, pV3r);
  *newtriangle = triangle2add;
  noVertices = pVertexContainer->getNoVertices();
  noTriangles++;
  setModified();
  return true;
 } else
  return false;
}
bool TinPart::addTriangle_p2(const Vertex& v1, const Vertex& v2,
  const Vertex& v3, Triangle** newtriangle) {
 LOGP
 bool nv1, nv2, nv3;
 const Vertex * pVr, *pV2r, *pV3r;
 Triangle *triangle2add;

 if (canAdd()) {

  pVr = pVertexContainer->insertVertex(&v1, nv1);
  pV2r = pVertexContainer->insertVertex(&v2, nv2);
  pV3r = pVertexContainer->insertVertex(&v3, nv3);
  LOGP
  triangle2add = constructTriangle(pVr, pV2r, pV3r);
  LOGP
  *newtriangle = triangle2add;
  LOGP
  features.update(triangle2add);
  LOGP
  noVertices = pVertexContainer->getNoVertices();
  LOGP
  noTriangles++;
  setModified();
  return true;
 } else
  return false;
}
void TinPart::deleteContentReference() {
 if (contentRefs > 0)
  contentRefs--;

//std::cout<<"delete ref for "<<this<<" value "<<contentRefs<<endl;
}
void TinPart::addContentReference() {
 contentRefs++;

//std::cout<<"add ref for "<<this<<" value "<<contentRefs<<endl;

}
VERTEX_Z TinPart::atlocation_brute_force(const Point_p& p) {
 LOGP
 loadContentData();

 int n = noTriangles;
 n--;

 while (n >= 0) {
  LOGP
  if (arTriangles[n].isInside_sec(p)) {
   LOG(*this)
   LOG("Dreieck nr:")
   LOG(n)
   LOG_EXP(arTriangles[n].print())
   return arTriangles[n].getValue(p);
  }
  n--;
 }
 LOGP
 return ERROR_VALUE;
}

VERTEX_Z TinPart::atlocation_bywalker(const Point_p& p) {
 LOGP
 loadContentData();

 Triangle * idxTriangle;

 if (noTriangles > 0) {

  idxTriangle = idxAtLocation.getAtLocationIndex(p);

  Triangle * t = idxTriangle->walkToTriangle_sec(p);

  LOGP

  if (!t)
   return atlocation_brute_force(p);
  LOG_EXP(t->print())
  LOGP
  return t->getValue(p);
 } else
  return ERROR_VALUE;

}

Triangle::triangleWalker TinPart::getWalker(Point_p * path, int noPoints) {
 loadContentData();

 return Triangle::triangleWalker(idxAtLocation.getAtLocationIndex(path[0]),
   path, noPoints);
}
Triangle* TinPart::findEdgeInPart(const Edge & e, Triangle* caller) {
 LOGP
 Point_p p1(*(e.getV1()));
 Point_p p2(*(e.getV2()));

 if (!features.bbox.contains(p1) || !features.bbox.contains(p2))
  return 0;

 loadContentData();

 Triangle * idxTriangle, *resultTriangle;
 Point_p * pdest = new Point_p(p1);

 if (noTriangles > 0) {

  idxTriangle = idxAtLocation.getAtLocationIndex(p1);

  Triangle::triangleWalker walker(idxTriangle, pdest, 1, true, true,
    Triangle::triangleWalker::stopAtEdgeWalk, &e);

  walker.walk_sec();

  resultTriangle = walker.getCurrentTriangle();

//if result could be determined via index without further loading
  if (resultTriangle && !resultTriangle->getEdge(e).isNull()
    && resultTriangle != caller) {
   return resultTriangle;
  }

// fall back brute force in case the index could not determine a neighbor
  int n = noTriangles;

  n--;

  while (n >= 0) {
   LOGP
   if (arTriangles[n].hasEdge(e)) {
    if (&arTriangles[n] != caller)
     return &arTriangles[n];
   }
   n--;
  }

 }

 return 0; // Edge is definitely not in this part

LOGP}

Triangle* TinPart::getNeighbor(const Edge& commonEdge, Triangle* caller) {
 LOGP
 return tin->findNeighbor(commonEdge, caller);
}
TIN_SIZE TinPart::getSizeInMemory() {
 if (isContentDataLoaded())
return  sizeof(*this) + noTriangles * sizeof(Triangle)

    + noTrianglesMax * sizeof(Triangle*)
    + pVertexContainer->getSizeInMemory();
 else
  return sizeof(*this);
}
TIN_SIZE TinPart::estimateMaxSizeInMemory(const TinConfiguration & conf) {

return  sizeof(TinPart)

   + TinPart::estimateNoTrianglesTotal(conf.maxSizePart) * sizeof(Triangle)
   + TinPart::estimateNoTrianglesTotal(conf.maxSizePart)
     * sizeof(Triangle*)
   + TinPart::estimateVertexContainerSizeOnDisc(conf.maxSizePart);

}
TIN_SIZE TinPart::estimateVertexContainerSizeOnDisc(
  TIN_SIZE imaxSizePart) {

//This method divides the ~imaxSizePart~ on disc in some memory for the Ver
//texContainer
//and some memory for the triangles, since the ratio between vertices and t
//riangles (~TRIANGLES_PER_VERTEX~) is
//not sure in advance it is just an estimate based on the assumption of ~TR
//IANGLES_PER_VERTEX~.
//For more than just a few vertices ~TRIANGLES_PER_VERTEX~ has to be betwee
//n 1 and 2.
//The exact formula is: nt = nv*2 - k - 5  where nt is the number of triang
//les and nv is the number of vertices
//           and k is the number of vertices on the convex hull of the set
//of vertices of the TinPart

 return double(imaxSizePart - (sizeof(int) + TinFeatures::getSizeOnDisc()))
   * (double(Vertex::getSizeOnDisc())
     / double(
       (Vertex::getSizeOnDisc()
         + TRIANGLES_PER_VERTEX * Triangle::getSizeOnDisc())));
}
void TinPart::initContentMemory() {

 TIN_SIZE containerSize = TinPart::estimateVertexContainerSizeOnDisc(
   config.maxSizePart);

 switch (config.abstractType) {
  case QUERY:
   pVertexContainer = new VertexContainerArray(containerSize);
   break;
  case MANIPULATE:
   pVertexContainer = new VertexContainerSet(containerSize);
   break;
  default:
   LOGP
   LOG(config.abstractType)
   throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR);
 }

 arTriangles = new Triangle[TinPart::estimateNoTrianglesTotal(
   config.maxSizePart)];
 noTrianglesMax = TinPart::estimateNoTrianglesTotal(config.maxSizePart);
}
void TinPart::initContentMemory(TIN_SIZE inoVertices,
  TIN_SIZE inoTriangles) {
 LOGP
 switch (config.abstractType) {
  case QUERY:
   pVertexContainer = new VertexContainerArray(-inoVertices);
   break;
  case MANIPULATE:
   pVertexContainer = new VertexContainerSet(-inoVertices);
   break;
  default:
   LOG(config.abstractType)
   throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR);
 }

 arTriangles = new Triangle[inoTriangles];
 noTrianglesMax = inoTriangles;
}
void TinPart::freeContentMemory() {
 LOGP
 if (pVertexContainer) {
  noVertices = pVertexContainer->getNoVertices();
  delete pVertexContainer;
 }

 if (arTriangles) {
  delete[] arTriangles;
 }
 pVertexContainer = 0;
 arTriangles = 0;

 noTrianglesMax = 0;
LOGP}
void TinPart::unloadContentData() {
 LOGP

 if (isContentDataLoaded() && contentRefs == 0
   && (config.memoryState != INMEMORY)) {
  LOGP
  if (isModified()) {
#ifndef UNIT_TEST
   save_content();
#endif
  }

  freeContentMemory();
  contentLoaded = false;
 }
LOGP}
void TinPart::loadContentData() {
 if (!isContentDataLoaded()) {
  initContentMemory(noVertices, noTriangles);
#ifndef UNIT_TEST
  open_content();
#endif

 }

}
bool TinPart::hasTriangle(Vertex &v1, Vertex &v2, Vertex &v3) {

 loadContentData();

 for (int i = 0; i < noTriangles; i++) {
  if (arTriangles[i].isEqual(v1, v2, v3))
   return true;
 }

 return false;
}
bool TinPart::checkNeighborRelations() {
 TinPart::iterator it(this);
 loadContentData();

 while ((*it)) {
  if (!(*it)->checkNeighbors())
   return false;
  it++;
 }

 return true;
}
bool TinPart::checkDelaunay() {
 loadContentData();
 const Vertex* vcomp;
 Point_p middle;
 double radius;

 for (int i = 0; i < noTriangles; i++) {
  middle = CircleEvent::calculateCircle_mp(*arTriangles[i].getVertex(1),
    *arTriangles[i].getVertex(2), *arTriangles[i].getVertex(3), radius);

  for (int vi = 0; vi < pVertexContainer->getNoVertices(); vi++) {
   vcomp = pVertexContainer->getVertexByYIndex(vi);
   if ((!(*vcomp == *arTriangles[i].getVertex(1)
     || *vcomp == *arTriangles[i].getVertex(2)
     || *vcomp == *arTriangles[i].getVertex(3)))
     && CircleEvent::isVertexInside(middle, radius, *vcomp)) {
    arTriangles[i].print();
    vcomp->print();
    std::cout << "\n";
    return false;
   }
  }

 }

 return true;

}
TinPart* TinPart::unaryOp(VERTEX_Z (*op)(VERTEX_Z), TinFeatures & feat) {
 loadContentData();
 features.reset();
 pVertexContainer->unaryOp(op, features);
 feat.update(features);
 setModified();
 return this;
}
std::ostream& operator <<(std::ostream& os, TinPart& a) {

 a.loadContentData();

os  << "AbstractTinPart---- maximum size: " << a.config.maxSizePart

   << " no. triangles: " << a.noTriangles << " current size on disc: "
   << a.getSizeOnDisc() << " triangles total: " << a.noTrianglesMax
   << "\n";

 a.features.print(os);

 os << "\n" << *(a.pVertexContainer) << "\n";

 for (int i = 0; i < a.noTriangles; i++) {
  a.arTriangles[i].print(os);
 }
 os << "end AbstractTinPart\n";
 return os;

}
TinPart* TinPart::getInstanceNew_fortestonly(Tin* tt,
  const Vertex *iarTriangles, int inoTriangles, int& noTrianglesAdded,
  const TinConfiguration& conf) {

 if (tt == 0)
  throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR2);
 if (iarTriangles == 0 && inoTriangles > 0)
  throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR0);

 Triangle * newtriangle;
 TinPart* part = new TinPart();

#ifndef UNIT_TEST
 part->isRecInitialized = false;
#endif
 part->tin = tt;
 noTrianglesAdded = 0;
 part->noTriangles = 0;
 part->config = conf;

 part->initContentMemory();

 while (noTrianglesAdded < inoTriangles) {
  if (!part->addTriangle(iarTriangles[noTrianglesAdded * 3],
    iarTriangles[noTrianglesAdded * 3 + 1],
    iarTriangles[noTrianglesAdded * 3 + 2], &newtriangle))
   break;

  noTrianglesAdded++;
 }

 part->contentLoaded = true;
 part->ismodified = true;

 LOG(part->config.abstractType)
 LOGP

 return part;
}
TinPart* TinPart::getInstanceNew(Tin* tt, const TinConfiguration& conf) {
 if (tt == 0)
  throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR2);

 if (conf.maxSizePart < MIN_CONFIG_SIZE
   || conf.maxSizePart > MAX_CONFIG_SIZE)
  throw std::invalid_argument(
    "TinPart configured with size out of range.(TinPart::getInstanceNew)");

 TinPart* part = new TinPart();

#ifndef UNIT_TEST
 part->isRecInitialized = false;
#endif
 part->tin = tt;
 part->noTriangles = 0;
 part->config = conf;
 part->initContentMemory();

 part->contentLoaded = true;
 part->ismodified = true;

 LOG(part->config.abstractType)
 LOGP

 return part;
}
#ifndef UNIT_TEST
TinPart* TinPart::getInstanceFromDisc(Tin* tt, SmiRecord& valueRecord,
  bool bulkload, const TinConfiguration& conf) {
 LOGP
 LOG(conf.abstractType)

 if (tt == 0)
  throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR2);

 if (conf.maxSizePart < MIN_CONFIG_SIZE
   || conf.maxSizePart > MAX_CONFIG_SIZE)
  throw std::invalid_argument(
    "TinPart configured with size out of range.(TinPart::getInstanceNew)");

 TinPart* part = new TinPart();

 part->tin = tt;
 part->noTriangles = 0;
 part->config = conf;
 part->pVertexContainer = 0;
 part->arTriangles = 0;
 part->contentLoaded = false;
 part->ismodified = false;

 if (part->open(valueRecord) != true)
  throw std::runtime_error(E_ABSTRACTTINPART_INIT);
 if (bulkload)
  part->loadContentData();

 return part;
}
TinPart* TinPart::getInstanceFromBuffer(Tin* tt, char* buffer,
  size_t &offset, bool bulkload, const TinConfiguration& conf) {
 LOGP
 LOG(conf.abstractType)

 if (conf.maxSizePart < MIN_CONFIG_SIZE
   || conf.maxSizePart > MAX_CONFIG_SIZE)
  throw std::invalid_argument(
    "TinPart configured with size out of range.(TinPart::getInstanceNew)");
 TinPart* part = new TinPart();

 part->tin = tt;
 part->noTriangles = 0;
 part->config = conf;
 part->pVertexContainer = 0;
 part->arTriangles = 0;
 part->contentLoaded = false;
 part->ismodified = false;

 part->rebuild(buffer, offset);

 if (bulkload)
  part->loadContentData();

 return part;
}

TinPart* TinPart::getInstanceFromDiscRandomAccess(Tin* tt,
  SmiRecord& valueRecord, uint32_t idx, const TinConfiguration& conf) {
 LOGP
 LOG(conf.abstractType)

 if (tt == 0)
  throw std::invalid_argument(E_ABSTRACTTINPART_CONSTRUCTOR2);

 if (conf.memoryState != RANDOMACCESS)
  throw std::invalid_argument(
    "Wrong configuration.(TinPart::getInstanceFromDiscRandomAccess)");

 if (conf.maxSizePart < MIN_CONFIG_SIZE
   || conf.maxSizePart > MAX_CONFIG_SIZE)
  throw std::invalid_argument(
    "TinPart configured with size out of range.(TinPart::getInstanceNew)");

 TinPart* part = new TinPart();

 part->tin = tt;
 part->noTriangles = 0;
 part->config = conf;
 part->pVertexContainer = 0;
 part->arTriangles = 0;
 part->contentLoaded = false;
 part->ismodified = false;

 if (part->open_head(valueRecord, idx) != true)
  throw std::runtime_error(E_ABSTRACTTINPART_INIT);

 part->loadContentData();

 return part;
}
#endif

TinPart* TinPart::clone(Tin*tt) {
 TinPart* result = new TinPart();
 bool n;
 const Vertex * v1, *v2, *v3;
 LOGP
 loadContentData();

 result->tin = tt;
 result->config = tt->getConfiguration();

 if (config.maxSizePart > result->config.maxSizePart)
  throw std::runtime_error("Tried to clone a part to a tin with smal"
    "ler parts. This is not possible.(TinPart::clone)");

 if (config.abstractType != result->config.abstractType)
  throw std::runtime_error(
    "Tried to clone a part to a tin with different contain"
      "er type. This is not possible.(TinPart::clone)");

 result->noTriangles = 0;
 result->features = features;
 result->contentLoaded = true;
 result->ismodified = true;
 result->pVertexContainer = pVertexContainer->clone_empty();

 result->arTriangles = new Triangle[noTriangles];

 for (int i = 0; i < noTriangles; i++) {
  v1 = result->pVertexContainer->insertVertex(arTriangles[i].getVertex(1),
    n);
  v2 = result->pVertexContainer->insertVertex(arTriangles[i].getVertex(2),
    n);
  v3 = result->pVertexContainer->insertVertex(arTriangles[i].getVertex(3),
    n);

  result->constructTriangle(v1, v2, v3);
  result->noTriangles++;
LOGP}
 result->noVertices = result->pVertexContainer->getNoVertices();
 result->contentRefs = 0;
 result->noTrianglesMax = noTrianglesMax;

#ifndef UNIT_TEST
 result->isRecInitialized = false;
#endif
 LOGP
 return result;

}
VERTEX_Z TinPart::atlocation(const Point_p& p) {
 LOGP
 return atlocation_bywalker(p);
}

const Rectangle& TinPart::bbox() const {
 return features.bbox;
}

VERTEX_Z TinPart::minimum() const {
 return features.m_minValue;
}

VERTEX_Z TinPart::maximum() const {
 return features.m_maxValue;
}

Triangle* TinPart::constructTriangle(const Vertex* v1, const Vertex* v2,
  const Vertex* v3) {
 LOGP
 LOG(this)
 return new (&arTriangles[noTriangles]) Triangle(const_cast<Vertex*>(v1),
   const_cast<Vertex*>(v2), const_cast<Vertex*>(v3), this);
}

TinPart * TinPart::binaryOp(AbstractTinType* tinResult,
  VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2)) {
 return 0;
}

/* namespace tin*/

}
