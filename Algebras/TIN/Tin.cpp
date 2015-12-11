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

#include "Tin.h"
#include "TinPart.h"
#ifndef UNIT_TEST
#include "Point.h"
#include "Stream.h"
#include "RelationAlgebra.h"
#include "Attribute.h"
#include "../Rectangle/RectangleAlgebra.h"
#include <stack>
#endif

#include <TinLogging.h>

using namespace std;

namespace tin {
#ifndef UNIT_TEST
struct tin2tuplestreamState {
 Tin::triangle_iterator * itTriangles;
 TupleType * tupleType;

 tin2tuplestreamState() {
  LOGP
  itTriangles = 0;
  tupleType = 0;
 }
 ~tin2tuplestreamState() {
  LOGP
  if (itTriangles)
   delete itTriangles;
  if (tupleType)
   tupleType->DeleteIfAllowed();
 }
};
struct tin2tinattributestreamState {
 deque<TinPart*>::iterator itParts;
 deque<TinPart*>::iterator itEnd;
 TupleType * tupleType;

 tin2tinattributestreamState() {
  LOGP
  tupleType = 0;
 }
 ~tin2tinattributestreamState() {
  LOGP
  if (tupleType)
   tupleType->DeleteIfAllowed();
 }
};
#endif
#ifndef UNIT_TEST
Tin::Tin(const TinConfiguration & conf) :
  file(false)
#else
Tin::Tin(const TinConfiguration & conf)
#endif
{
 LOGP
 if (conf.memoryState == RANDOMACCESS || conf.memoryState == GRADUALFILE
   || conf.memoryState == GRADUALTEMPORARYFILE)
  throw std::runtime_error("MemoryState RANDOMACCES|GRADUAL(TEMPORARY)F"
    "ILE cannot be constructed. (Tin::Tin)");

#ifndef UNIT_TEST
 SmiRecord contentRec;
 rtree = 0;
#endif
 LOG(tinParts.size())
 config = conf;
 noParts = 0;
 tinTypeCurrent = MANIPULATE;
 constructionQueue = 0;
 defined = false;
}

#ifndef UNIT_TEST
Tin::Tin(VertexContainerSet * vertices, const TinConfiguration& conf) :
  file(false)
#else
Tin::Tin(VertexContainerSet * vertices, const TinConfiguration& conf)
#endif
{
 LOGP
 if (conf.memoryState == RANDOMACCESS || conf.memoryState == GRADUALFILE
   || conf.memoryState == GRADUALTEMPORARYFILE)
  throw std::runtime_error("MemoryState RANDOMACCES|GRADUAL(TEMPORARY)FILE"
    " cannot be constructed. (Tin::Tin)");

 config = conf;
 noParts = 0;
 tinTypeCurrent = MANIPULATE;

 constructionQueue = new EventQueue(this);
// to avoid horizontal stripes in fortune
// algorithm the parts are organized in columns
 initColumnLayout(*vertices);

 constructionQueue->doFortuneAlgorithm(vertices, true);

 finishLayout();
 unloadAllParts();

 delete constructionQueue;
 constructionQueue = 0;
 defined = false;

}

Tin::~Tin() {
 deque<TinPart *>::iterator it = tinParts.begin();

 LOG(tinParts.size())

 while (it != tinParts.end()) {
  LOGP
  delete (*it);
  ++it;
 }

 if (constructionQueue)
  delete constructionQueue;

#ifndef UNIT_TEST

 LOGP

 switch (config.memoryState) {
  case INMEMORY:
//for objects from disc the file
//will always stay open, thus no deletion of the tree
   if (config.memoryState == INMEMORY && !file.IsOpen()) {
    if (rtree)
     rtree->DeleteFile();
   }

   if (file.IsOpen())
    file.Close(true);

   break;
  case GRADUALFILE:
   if (file.IsOpen()) //file should be open !
    file.Close(true);
   break;
  case GRADUALTEMPORARYFILE:
   LOGP
   deleteFile();
   if (rtree)
    rtree->DeleteFile();
   break;
  case RANDOMACCESS:
   if (file.IsOpen()) //file should be open !
    file.Close(true);
   break;
 }

 if (rtree)
  delete rtree;

#endif

}

//////Analysis  /////////////////////////////////////////////////
Triangle::triangleWalker Tin::getWalker(const Point_p & p) {
 deque<TinPart*>::const_iterator it;
 LOGP
 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  if ((*it)->bbox().contains(p)) {
   Point_p * dest = new Point_p(p);
   return (*it)->getWalker(dest, 1);
LOGP }

 it++;
}
LOGP

 throw std::runtime_error(
   "Cannot walk here. Point is not part of the tin.(Tin::getWalker)");
}
VERTEX_Z Tin::atlocation(const Point_p& p) {
 deque<TinPart*>::const_iterator it;
 TinPart * raPart = 0;
 LOGP
 VERTEX_Z result = ERROR_VALUE;
#ifndef UNIT_TEST

 double min[2] = { p.x, p.y };
 double max[2] = { p.x, p.y };

 ::Rectangle<2> pointbox = ::Rectangle<2>(true, min, max);
 R_TreeLeafEntry<2, uint32_t> entry;

 if (rtree) {
  LOGP

  if (rtree->First(pointbox, entry)) {
   LOGP
   if (!(config.memoryState == RANDOMACCESS))
    result = tinParts.at(entry.info)->atlocation(p);
   else {
    raPart = getPartFromDisc(entry.info);
    result = raPart->atlocation(p);
    delete raPart;
   }
   LOGP
   if (result != ERROR_VALUE)
    return result;

   while (rtree->Next(entry)) {
    if (!(config.memoryState == RANDOMACCESS))
     result = tinParts.at(entry.info)->atlocation(p);
    else {
     raPart = getPartFromDisc(entry.info);
     result = raPart->atlocation(p);
     delete raPart;
    }

    LOGP
    if (result != ERROR_VALUE)
     return result;
   }
  }

 } else {
  it = tinParts.begin();
  while (it != tinParts.end()) {
   LOGP
   if ((*it)->bbox().contains(p)) {
    LOGP
    result = (*it)->atlocation(p);
    if (result != ERROR_VALUE)
     return result;
   }

   it++;
  }
LOGP}

#else
it = tinParts.begin();
while (it != tinParts.end()) {
 LOGP
 if ((*it)->bbox().contains(p)) {
  LOGP
  result = (*it)->atlocation(p);
  if (result != ERROR_VALUE)
  return result;
 }

 it++;
}
LOGP
#endif

LOGP
 return ERROR_VALUE;
}
;
const Rectangle& Tin::bbox() const {
 return features.bbox;
}
;
VERTEX_Z Tin::minimum() const {
 return features.m_minValue;
}
;
VERTEX_Z Tin::maximum() const {
 return features.m_maxValue;
}
;
/////////////////////////////////////////////////////////////////

///Manipulation//////////////////////////////////////////////////

void Tin::unaryOp(VERTEX_Z (*op)(VERTEX_Z z)) {
 deque<TinPart*>::const_iterator it;

 features.reset();
 it = tinParts.begin();
 while (it != tinParts.end()) {
  (*it)->unaryOp(op, features);
  ++it;
 }

}

void Tin::addPart(TinPart * p) {
 LOGP
 features.update(p->getFeatures());

#ifndef UNIT_TEST

 LOGP
 if (!rtree)
  rtree = new R_Tree<2, uint32_t>(
    WinUnix::getPageSize() - 2 * 2 * int(sizeof(double)));

 const TinFeatures& feat = p->getFeatures();
 LOGP
 double min[2] = { feat.bbox.getX1(), feat.bbox.getY1() };
 double max[2] = { feat.bbox.getX2(), feat.bbox.getY2() };

 R_TreeLeafEntry<2, uint32_t> entry = R_TreeLeafEntry<2, uint32_t>(
   ::Rectangle<2>(true, min, max), noParts);

 rtree->Insert(entry);

 LOG_EXP(feat.print())
#endif

 tinParts.push_back(p);
 noParts++;
 p->unloadContentData();
}

#ifndef UNIT_TEST
void Tin::unaryOp(void* function, Tin * result) {
 deque<TinPart*>::const_iterator it;
 TinPart* opPart;

 result->tinTypeCurrent = MANIPULATE;
 result->config = config;
 result->config.memoryState = INMEMORY;
 result->noParts = 0;
 result->constructionQueue = 0;

 if (this->estimateMaxSizeInMemory() * 2 > OPERATE_IN_MEMORY_THRESHOLD) {
//do not keep everything in memory
  result->setMemoryStateGradual();
  this->setMemoryStateGradual();

 } else {
  result->setMemoryState(INMEMORY);
  this->setMemoryState(INMEMORY);
 }

 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  opPart = (*it)->clone(result);
  (*it)->unloadContentData();
  opPart->unaryOp(function);
  result->addPart(opPart);
  ++it;
 }

LOGP}

#endif
/*
 The method setMemoryState configures the data being kept in memory.
 The state INMEMORY for example loads the whole TIN in memory and
 keeps it there. See the master thesis for detailed information.

*/
void Tin::setMemoryState(MemoryState st) {
 LOGP
 deque<TinPart *>::iterator it;

 LOG("Current MemoryState:")
 LOG(config.memoryState)
 LOG(" ... set to ... ")
 LOG(st)

 if (config.memoryState == st) {
  LOG(config.memoryState)
  LOGP
  return;
 }
 if (st == RANDOMACCESS)
  throw std::runtime_error(
    "Switch to RANDOMACCESS is not implemented.(Tin::setMemoryState)");
 if (config.memoryState == GRADUALFILE && st == GRADUALTEMPORARYFILE)
  throw std::runtime_error("Not reasonable.(Tin::setMemoryState)");

#ifndef UNIT_TEST
 if (file.IsOpen() && st == GRADUALTEMPORARYFILE)
  throw std::runtime_error("Switching to temporary file is not possible,"
    " when the object comes from db!(Tin::setMemoryState)");

 if (config.memoryState == RANDOMACCESS) {
  switch (st) {
   case GRADUALFILE:
    openParts(false);
    break;
   case GRADUALTEMPORARYFILE:
    throw std::runtime_error(
      "This is not reasonable.(Tin::setMemoryState)");
    break;
   case INMEMORY:
    openParts(true);
    break;
   case RANDOMACCESS:
    break;
  }

 }

#endif
 if (config.memoryState == GRADUALTEMPORARYFILE && st == INMEMORY) {
  LOGP
  loadAllParts();
#ifndef UNIT_TEST
  deleteFile();
#endif
 }

 if (config.memoryState == INMEMORY && st == GRADUALTEMPORARYFILE) {
  LOGP

  config.memoryState = st;

#ifndef UNIT_TEST
  getFile(); // this makes sure that there is a file
#endif

  it = tinParts.begin();
  while (it != tinParts.end()) {
   LOGP
   (*it)->setMemoryState(st);
   ++it;
  }
  unloadAllParts();
  return;
 }
 if (config.memoryState == INMEMORY && st == GRADUALFILE) {

  LOGP

  config.memoryState = st;
#ifndef UNIT_TEST
  getFile(); // this makes sure that there is a file
#endif

  it = tinParts.begin();
  while (it != tinParts.end()) {
   LOGP
   (*it)->setMemoryState(st);
   ++it;
  }
  unloadAllParts();
  return;
 }

 config.memoryState = st;

 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  (*it)->setMemoryState(st);
  ++it;
 }
}

/////////////////////////////////////////////////////////////////
void Tin::unloadAllParts() {
 deque<TinPart*>::iterator it;
 if (config.memoryState != INMEMORY
   && config.memoryState != RANDOMACCESS) {
  LOGP
  it = tinParts.begin();
  while (it != tinParts.end()) {
   LOGP
//try to unload all unreferenced parts
   (*it)->unloadContentData();
   ++it;
  }

 }

LOGP}
void Tin::loadAllParts() {
 deque<TinPart*>::iterator it;
 if (config.memoryState != INMEMORY
   && config.memoryState != RANDOMACCESS) {
  LOGP
  it = tinParts.begin();
  while (it != tinParts.end()) {
   LOGP
   (*it)->loadContentData();
   ++it;
  }

 }

LOGP}
void Tin::triangulateSection(VertexContainerSet * vc) {
 LOGP
 if (constructionQueue) {
  initColumnLayout(*vc);
  constructionQueue->doFortuneAlgorithm(vc);
 } else {
  constructionQueue = new EventQueue(this);
  initColumnLayout(*vc);
  constructionQueue->doFortuneAlgorithm(vc);
 }

 unloadAllParts();

}
void Tin::finishTriangulation() {
 LOGP
 if (constructionQueue) {
  VertexContainerSet *settmp = new VertexContainerSet(
    VERTEX_CONTAINER_BIG_SIZE);

  constructionQueue->doFortuneAlgorithm(settmp, true);
  finishLayout();

  delete constructionQueue;
  constructionQueue = 0;

  unloadAllParts();
 }
}
void Tin::triangulateBulk(VertexContainerSet * vc) {
 deque<TinPart*>::iterator it;

 if (constructionQueue) {
  initColumnLayout(*vc);
  constructionQueue->doFortuneAlgorithm(vc, true);
 } else {
  constructionQueue = new EventQueue(this);
  initColumnLayout(*vc);
  constructionQueue->doFortuneAlgorithm(vc, true);
 }

 finishLayout();

 unloadAllParts();

 delete constructionQueue;
 constructionQueue = 0;

}
bool Tin::canAddTriangle(const Triangle & t, std::string & msg) {
 std::deque<TinPart *>::iterator it = tinParts.begin();
 std::stack<TinPart *> partsToCheck;

 while (it != tinParts.end()) {
  if ((*it)->bbox().hasIntersection(t.bbox()))
   partsToCheck.push((*it));

  ++it;
 }

 while (!partsToCheck.empty()) {
  if (!partsToCheck.top()->isValidTriangle(t, msg)) {
   return false;
  }

  partsToCheck.pop();
 }

 return true;
}
void Tin::addTriangle(const Vertex & v1, const Vertex & v2,
  const Vertex & v3, Triangle** newtriangle) {
 TinPart * newPart;
 std::map<ColumnKey, TinPart *>::iterator it;
 std::string msg;

 it = columnMap.find(ColumnKey(v1.getX(), v1.getX()));

 if (it == columnMap.end())
  throw std::runtime_error(E_TIN_ADDTRIANGLE);
 LOGP

 Triangle t(&v1, &v2, &v3);

 if (!canAddTriangle(t, msg)) {
  throw std::runtime_error(msg);
 }
 if (!(*it).second->addTriangle_p2(v1, v2, v3, newtriangle)) {
  newPart = getNewPart();
  LOGP
  newPart->addTriangle_p2(v1, v2, v3, newtriangle);
  LOGP
  finishPart((*it));

  (*it).first.setNoPart(noParts - 1);
  (*it).second = newPart;
  (*it).second->addContentReference();
 }

}
void Tin::addTriangle_p(const Vertex & v1, const Vertex & v2,
  const Vertex & v3, Triangle** newtriangle) {

 TinPart * newPart;
 std::map<ColumnKey, TinPart *>::iterator it;

 it = columnMap.find(ColumnKey(v1.getX(), v1.getX()));

 if (it == columnMap.end())
  throw std::runtime_error(E_TIN_ADDTRIANGLE);

 if (!(*it).second->addTriangle_p(v1, v2, v3, newtriangle)) {

  newPart = getNewPart();
  newPart->addTriangle_p(v1, v2, v3, newtriangle);

  finishPart((*it), true);

  (*it).first.setNoPart(noParts - 1);
  (*it).second = newPart;
  (*it).second->addContentReference();
 }

}
void Tin::finishLayout(bool updatePartsFeatures) {
 std::map<ColumnKey, TinPart*>::iterator it;
 it = columnMap.begin();

 while (it != columnMap.end()) {
  finishPart((*it), updatePartsFeatures);
  ++it;
 }
 columnMap.clear();
}
double Tin::calculateNoColumns(VertexContainerSet & vc) {
 TIN_SIZE noTrianglesPerPartEstimate;
 double noVertices, noPages, noColumns;
 int noRows;
 TinFeatures bbox;

 bbox = vc.getFeatures();
//estimate pages

 noTrianglesPerPartEstimate = TinPart::estimateNoTrianglesTotal(
   config.maxSizePart);
 noVertices = vc.getNoVertices();
 noPages = noVertices * TRIANGLES_PER_VERTEX / noTrianglesPerPartEstimate;

//calculate columns
 noColumns = std::sqrt(
   (noPages * (bbox.bbox.getX2() - bbox.bbox.getX1()))
     / (bbox.bbox.getY2() - bbox.bbox.getY1()));
 noRows = noPages / noColumns;

 if (noRows == 0)
  noColumns = noPages;

 return noColumns;

}
double Tin::calculateNoRows(VertexContainerSet & vc) {
 TIN_SIZE noTrianglesPerPartEstimate;
 double noVertices, noPages, noColumns;
 double noRows;
 TinFeatures bbox;

 bbox = vc.getFeatures();
//estimate pages

 noTrianglesPerPartEstimate = TinPart::estimateNoTrianglesTotal(
   config.maxSizePart);
 noVertices = vc.getNoVertices();
 noPages = noVertices * TRIANGLES_PER_VERTEX / noTrianglesPerPartEstimate;

//calculate columns
 noColumns = std::sqrt(
   (noPages * (bbox.bbox.getX2() - bbox.bbox.getX1()))
     / (bbox.bbox.getY2() - bbox.bbox.getY1()));
 noRows = noPages / noColumns;

 return noRows;
}
void Tin::finishPart(std::pair<const ColumnKey, TinPart*>& partpair,
  bool updatePartFeat) {

 if (updatePartFeat)
  partpair.second->updateFeatures();

 features.update(partpair.second->getFeatures());

#ifndef UNIT_TEST

 if (!rtree)
  rtree = new R_Tree<2, uint32_t>(
    WinUnix::getPageSize() - 2 * 2 * int(sizeof(double)));

 const TinFeatures& feat = partpair.second->getFeatures();

 double min[2] = { feat.bbox.getX1(), feat.bbox.getY1() };
 double max[2] = { feat.bbox.getX2(), feat.bbox.getY2() };

 R_TreeLeafEntry<2, uint32_t> entry = R_TreeLeafEntry<2, uint32_t>(
   ::Rectangle<2>(true, min, max), partpair.first.getNoPart());

 rtree->Insert(entry);

#endif
 partpair.second->deleteContentReference();
 partpair.second->unloadContentData();
}
/*
 The method initSimpleLayout initializes a single infinite column
 as layout for the distribution of new triangles among the TinParts.

*/
void Tin::initSimpleLayout() {
 std::pair<ColumnKey, TinPart *> p;

//clear existing Layout
 finishLayout();

//just one column with a new part
 p.first = ColumnKey(-std::numeric_limits<VERTEX_COORDINATE>::max(),
   std::numeric_limits<VERTEX_COORDINATE>::max(), noParts);
 p.second = getNewPart();
 p.second->addContentReference();

 columnMap.insert(p);
}
/*
 The method initColumnLayout initializes several columns as layout.
 The columns try to achieve squared TinParts. This is done by
 the evaluation of the VertexContainerSet vc an the configuration.

*/
void Tin::initColumnLayout(VertexContainerSet & vc) {
 TinFeatures bbox;
 std::stack<std::pair<const ColumnKey, TinPart *> > incompleteParts;
 std::pair<ColumnKey, TinPart *> p;
 std::map<ColumnKey, TinPart *>::reverse_iterator ritMap;
 int columnWidth;
 double noColumns;
 int inoColumns;
 TinPart * newPart;
 uint32_t noPart;

 if (columnMap.size()) {
  ritMap = columnMap.rbegin();
// rescue current incomplete parts for the next row
  while (ritMap != columnMap.rend()) {
   incompleteParts.push((*ritMap));
   ++ritMap;
  }

  columnMap.clear();
 }

 vc.updateXindexAndFeatures();

 noColumns = calculateNoColumns(vc);

 if ((int) noColumns > 1)
  columnWidth = vc.getNoVertices() / noColumns;
 else {
//just one column

  if (!incompleteParts.empty()) {
   p = incompleteParts.top();
   p.first = ColumnKey(-std::numeric_limits<VERTEX_COORDINATE>::max(),
     std::numeric_limits<VERTEX_COORDINATE>::max(), p.first.getNoPart());

   incompleteParts.pop();
  } else {
   p.first = ColumnKey(-std::numeric_limits<VERTEX_COORDINATE>::max(),
     std::numeric_limits<VERTEX_COORDINATE>::max(), noParts);

   p.second = getNewPart();
   p.second->addContentReference();
  }

  columnMap.insert(p);

  while (!incompleteParts.empty()) {
   finishPart(incompleteParts.top(), true);
   incompleteParts.pop();
  }

  return;
 }

 inoColumns = noColumns;

 for (int i = 0; i < inoColumns; i++) {

  if (!incompleteParts.empty()) {
   p = incompleteParts.top();
   noPart = p.first.getNoPart();
   newPart = p.second;
   incompleteParts.pop();
  } else {
   noPart = noParts;
   newPart = getNewPart();
   newPart->addContentReference();
  }
//due to rounding take care of the last row to contain surplus
  if (i == (inoColumns - 1))
   p.first = ColumnKey(vc.getVertexByXIndex(i * columnWidth)->getX(),
     std::numeric_limits<VERTEX_COORDINATE>::max(), noPart);
  else if (i == 0)
   p.first = ColumnKey(-std::numeric_limits<VERTEX_COORDINATE>::max(),
     vc.getVertexByXIndex((i + 1) * columnWidth)->getX(), noPart);
  else
   p.first = ColumnKey(vc.getVertexByXIndex(i * columnWidth)->getX(),
     vc.getVertexByXIndex((i + 1) * columnWidth)->getX(), noPart);

  p.second = newPart;
  columnMap.insert(p);
 }

 while (!incompleteParts.empty()) {
  finishPart(incompleteParts.top(), true);
  incompleteParts.pop();
 }
}
void Tin::saveSTLFile(std::ostream& os) {
 triangle_iterator it = begin();
 deque<TinPart*>::iterator itp;

 TIN_SIZE notriangles = 0;

 itp = tinParts.begin();
 while (itp != tinParts.end()) {
  notriangles += (*itp)->getNoTriangles();
  itp++;
 }

 char* buffer = new char[80 + 4 + notriangles * (4 * 12 + 2)];
 uint32_t offset;

 memset(buffer, 0, 80);
 memcpy(buffer + 80, &notriangles, 4);
 offset = 84;

 while ((*it)) {
  (*it)->putSTLbinaryRepresentation(buffer, offset);

  it++;
 }

 os.write(buffer, 80 + 4 + notriangles * (4 * 12 + 2));

 delete[] buffer;

}
void Tin::print(std::ostream& os) {
 deque<TinPart *>::iterator it = tinParts.begin();

 os << "TinType-------\n";
 os << "Configuration: \n";
 os << " Page size: " << config.maxSizePart << " part type "
   << config.abstractType << "\n";

 features.print(os);
 os << "Parts:--------------------------------------------\n";
 while (it != tinParts.end()) {
  os << *(*it);
  ++it;
 }
}
bool Tin::hasTriangle(Vertex& v1, Vertex& v2, Vertex& v3) {
 deque<TinPart*>::const_iterator it;
 LOGP

 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  if ((*it)->bbox().contains(Point_p(v1.getX(), v1.getY()))
    && (*it)->bbox().contains(Point_p(v2.getX(), v2.getY()))
    && (*it)->bbox().contains(Point_p(v3.getX(), v3.getY()))) {
   LOGP
   if ((*it)->hasTriangle(v1, v2, v3))
    return true;
  }

  it++;
 }
 LOGP
 return false;
}
Tin * Tin::clone() const {
 deque<TinPart*>::const_iterator it;
 TinPart * copyPart;
 LOGP
 TinConfiguration conf = config;
 conf.memoryState = INMEMORY;
 Tin * tt = new Tin(conf);

#ifndef UNIT_TEST
 if (this->estimateMaxSizeInMemory() * 2 < OPERATE_IN_MEMORY_THRESHOLD) {
  LOGP
  tt->setMemoryState(INMEMORY);
  const_cast<Tin*>(this)->setMemoryState(INMEMORY);
 } else {
  LOGP
  tt->setMemoryState(GRADUALTEMPORARYFILE);
  const_cast<Tin*>(this)->setMemoryStateGradual();
 }
#endif

 LOGP

 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  copyPart = (*it)->clone(tt);
  tt->addPart(copyPart);
  (*it)->unloadContentData();
  ++it;
 }
 LOGP
 return tt;
}
void Tin::cloneTin(Tin * result) const {
 deque<TinPart*>::const_iterator it;
 LOGP
#ifndef UNIT_TEST
 if (this->estimateMaxSizeInMemory() * 2 < OPERATE_IN_MEMORY_THRESHOLD) {
  LOGP
  result->setMemoryState(INMEMORY);
  const_cast<Tin*>(this)->setMemoryState(INMEMORY);
 } else {
  LOGP
  result->setMemoryStateGradual();
  const_cast<Tin*>(this)->setMemoryStateGradual();
 }
#endif
 LOGP
 result->config.maxSizePart = config.maxSizePart;

 it = tinParts.begin();
 while (it != tinParts.end()) {
  LOGP
  result->addPart((*it)->clone(result));
  (*it)->unloadContentData();
  ++it;
 }
LOGP}
#ifndef UNIT_TEST

void Tin::setMemoryStateGradual() {
 if (config.memoryState == GRADUALFILE
   || config.memoryState == GRADUALTEMPORARYFILE)
  return;

 if (file.IsOpen()) {
  setMemoryState(GRADUALFILE);
  return;
 } else {
  setMemoryState(GRADUALTEMPORARYFILE);
  return;
 }

}

ListExpr Tin::tin2stlfile_tm(ListExpr args) {

 if (nl->ListLength(args) != 2)
  return NList::typeError("Expected two arguments. (tin2stl)");

 if (!nl->IsAtom(nl->First(args))
   || nl->SymbolValue(nl->First(args)) != Tin::BasicType())
  return NList::typeError(
    "The first argument is wrong. Expected a tin. (tin2stl)");
 if (!nl->IsAtom(nl->Second(args))
   || nl->SymbolValue(nl->Second(args)) != CcString::BasicType())
  return NList::typeError(
    "The second argument is wrong. Expected a string. (tin2stl)");

 return nl->SymbolAtom(CcBool::BasicType());
}
int Tin::tin2stlfile_vm(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Tin * tin = static_cast<Tin *>(args[0].addr);
 CcString * path = static_cast<CcString *>(args[1].addr);

 result = qp->ResultStorage(s);
 CcBool * ret = (CcBool*) result.addr;

 if (tin && tin->isDefined()) {
  std::ofstream stlfile;

  stlfile.open(path->GetValue().c_str(), std::ios::binary | std::ios::trunc);

  if (!stlfile.is_open())
   throw std::runtime_error(
     "Could not open|create stl file.(Tin::tin2stl)");

  if (tin->estimateMaxSizeInMemory() > OPERATE_IN_MEMORY_THRESHOLD)
   tin->setMemoryStateGradual();
  else
   tin->setMemoryState(INMEMORY);

  tin->saveSTLFile(stlfile);

  stlfile.close();

  ret->Set(true, true);
 } else {
  ret->Set(true, false);
 }

 return 1;

}

#endif

bool Tin::checkDelaunay() {
 deque<TinPart*>::const_iterator it;
 LOGP

 it = tinParts.begin();
 while (it != tinParts.end()) {
  if (!(*it)->checkDelaunay())
   return false;

  it++;
 }
 LOGP
 return true;
}
bool Tin::checkNeighborRelations() {
 deque<TinPart*>::const_iterator it;
 LOGP
 it = tinParts.begin();
 while (it != tinParts.end()) {
  if (!(*it)->checkNeighborRelations())
   return false;

  it++;
 }
 LOGP
 return true;
}
TinPart* Tin::getNewPart() {
 TinPart * newPart;
 noParts++;
 newPart = TinPart::getInstanceNew(this, config);
 tinParts.push_back(newPart);

 LOGP
 LOG(newPart)
 return newPart;
}

#ifndef UNIT_TEST
TinPart* Tin::getPartFromDisc(SmiRecord & valueRecord, bool bulkload) {
 TinPart * newPart;
 LOGP
 LOG(valueRecord.GetPos())

 newPart = TinPart::getInstanceFromDisc(this, valueRecord, bulkload,
   config);
 tinParts.push_back(newPart);

 return newPart;
}
TinPart* Tin::getPartFromDisc(uint32_t idx) {
 TinPart * newPart;
 SmiRecord contentRec;

 if (!file.IsOpen())
  throw std::runtime_error("No file. For random access "
    "there has to be a file from db.(Tin::getPartFromDisc)");

 if (!file.SelectRecord(contentId, contentRec, SmiFile::ReadOnly))
  throw std::runtime_error("Could not open record.(Tin::getPartFromDisc)");

 newPart = TinPart::getInstanceFromDiscRandomAccess(this, contentRec, idx,
   config);

 LOG(*newPart)
 LOGP
 return newPart;
}
#endif
TinPart* Tin::getNextPartIntersecting(Rectangle& bbox) {
 static unsigned int i = 0;

//TODO make more efficient
 while (i < tinParts.size()) {
  if (tinParts[i]->bbox().hasIntersection(bbox))
   return tinParts[i++];

  i++;
 }

 if (i == tinParts.size())
  i = 0;
 return 0;
}
TIN_SIZE Tin::getSizeOnDisc() const {
 TIN_SIZE size = 0;
 LOGP
 deque<TinPart*>::const_iterator it;

#ifndef UNIT_TEST

 size += sizeof(SmiFileId);
 size += sizeof(contentId);
 size += sizeof(SmiFileId); //tree

#endif
 size += sizeof(config.maxSizePart);
 size += sizeof(config.abstractType);
 size += sizeof(noParts);
 size += features.getSizeOnDisc();

 it = tinParts.begin();

 while (it != tinParts.end()) {
  size += (*it)->getSizeOnDisc();
  ++it;
 }
 LOGP

 return size;
}
TIN_SIZE Tin::getPartHeadOrigin() const {
 TIN_SIZE size = 0;
 LOGP

 size += sizeof(config.maxSizePart);
 size += sizeof(config.abstractType);
 size += sizeof(noParts);
 size += features.getSizeOnDisc();

#ifndef UNIT_TEST

 size += sizeof(SmiFileId); //for rtree file

#endif
 LOGP

 return size;
}
#ifndef UNIT_TEST
void TinAttribute::Serialize2Flob() {
 LOGP

 TIN_SIZE sz = getSizeOnDisc();
 size_t offset = 0;

 char * storage = new char[sz];

//uint8_t def = (IsDefined() ? 1 : 0);
//WriteVar<uint8_t>(def, storage, offset);
//LOG(def)

 if (IsDefined()) {

//noVertices = pVertexContainer->getNoVertices();

//WriteVar<TIN_SIZE>(noTriangles, storage, offset);
//WriteVar<TIN_SIZE>(noVertices, storage, offset);

//features.serialize(storage, offset);

  pVertexContainer->serialize(storage, offset);

  LOG("Number of triangles:")
  LOG(noTriangles)
  LOGP

  for (int i = 0; i < noTriangles; i++) {
   LOG(i)
   arTriangles[i].putSecondoRepresentation(pVertexContainer, storage,
     (uint32_t &) offset);
  }

  binData.resize(sz);

  if (!binData.write(storage, (SmiSize) sz, 0)) {
   delete[] storage;
   throw std::runtime_error(
     "Data could not be written.(TinAttribute::Serialize2Flob)");
  }

LOGP}

 delete[] storage;
LOGP}

void TinAttribute::RebuildFromFlob() {
 LOGP
 TIN_SIZE sz = (TIN_SIZE) binData.getSize();

 char * state = new char[sz];

 size_t offset = 0;
 uint8_t def;
 LOGP

 LOG(sz)

 if (!binData.read(state, sz, 0)) {
  delete[] state;
  SetDefined(false);
  throw std::runtime_error(
    "Data could not be read.(TinAttribute::RebuildFromFlob)");
 }

//ReadVar<uint8_t>(def, state, offset);
//LOG(def)
//SetDefined((def == 1 ? true : false));

 if (IsDefined()) {

//ReadVar<TIN_SIZE>(noTriangles, state, offset);
//ReadVar<TIN_SIZE>(noVertices, state, offset);
//
//features.rebuild(state, offset);

  LOG(recId)
  LOG(noTriangles)
  LOG(noVertices)

  freeContentMemory();
  initContentMemory(noVertices, noTriangles);

  pVertexContainer->rebuild(state, offset);

  LOGP
  if (noTriangles) {
   LOG(noTriangles)
   LOGP
   for (int i = 0; i < noTriangles; i++) {
    LOG(i)
    new (&arTriangles[i]) Triangle(pVertexContainer, state,
      (uint32_t &) offset, this);
   }

LOGP }
}

 delete[] state;

LOGP}

Triangle* TinAttribute::getNeighbor(const Edge& commonEdge,
  Triangle* caller) {
 LOGP
 return this->findEdgeInPart(commonEdge, caller);
}

bool Tin::open(SmiRecord& valueRecord, bool bypass, SmiFileId ifileid,
  SmiRecordId irec) {
 LOGP
 SmiRecord contentRec;
 SmiFileId fileid;

 if (config.memoryState != INMEMORY)
  throw std::runtime_error(
    "Opening only in state INMEMORY ! Transistion not possible.(Tin::open)");

//Record structure root record and file:
//-fileid|contentId
//then in file pointed to by fileid and record contentId:
//config.maxSizePart|config.abstractType|noParts|treefile|features|
//-all header data of parts (see open method of TinPart)-

 config.memoryState = RANDOMACCESS;

 if (bypass) {
  fileid = ifileid;
  contentId = irec;
 } else {
  valueRecord.Read(fileid);
  valueRecord.Read(contentId);
 }

 if (!file.Open(fileid))
  throw std::runtime_error(E_TIN_OPEN1);
 if (!file.SelectRecord(contentId, contentRec, SmiFile::ReadOnly))
  throw std::runtime_error(E_TIN_OPEN);

 contentRec.SetPos(0);

 contentRec.Read(config.maxSizePart);
 contentRec.Read(config.abstractType);

 LOG("AbstractType read:")
 LOG(config.abstractType)

 tinTypeCurrent = config.abstractType;
 contentRec.Read(noParts);
 LOG("Parts read:")
 LOG(noParts)

 contentRec.Read(fileid);
 LOG("rtree fileid:")
 LOG(fileid)

 if (!rtree)
  rtree = new R_Tree<2, uint32_t>(fileid);
 else {
  delete rtree;
  rtree = new R_Tree<2, uint32_t>(fileid);
 }

 features.open(contentRec);

 LOGP
 return true;

}
bool Tin::openParts(bool bulkload) {
 LOGP
 SmiRecord contentRec;
 SmiSize sz = 0;
 SmiSize offset = 0;

 if (!file.IsOpen())
  throw std::runtime_error(
    "Cannot open parts without file.(Tin::openParts)");

 if (!file.SelectRecord(contentId, contentRec, SmiFile::ReadOnly))
  throw std::runtime_error(
    "ContentRec could not be opened.(Tin::openParts)");

 LOG(contentId)

 sz = noParts * TinPart::getSizeOnDisc_head();
 char * buffer = new char[sz];

 contentRec.Read(buffer, sz, getPartHeadOrigin());

 tinParts.resize(noParts);

 for (int i = 0; i < noParts; i++) {
  tinParts[i] = TinPart::getInstanceFromBuffer(this, buffer,
    (uint32_t &) offset, bulkload, config);
 }

 delete[] buffer;
 LOGP
 return true;
}

bool Tin::save(SmiRecord& valueRecord, bool bypass) {
 LOGP
 SmiRecord contentRec;
 if (config.memoryState == INMEMORY
   || config.memoryState == GRADUALTEMPORARYFILE)
  throw std::runtime_error("Tried to save a tin in state stay"
    " in memory or state temporary file.(Tin::save)");

//Record structure root record and file:
//-fileid|contentId
//then in file pointed to by fileid and record contentId:
//|config.maxSizePart|config.abstractType|noParts|features|
//-all header data of parts (see open method of TinPart)-

 SmiFileId fileid = file.GetFileId();

 if (!bypass) {
  valueRecord.Write(fileid);
  valueRecord.Write(contentId);
 }

//save header information of Tin
 if (!file.SelectRecord(contentId, contentRec, SmiFile::Update))
  throw std::runtime_error("Error selecting record.(Tin::save)");

//////////////////////////////////////////////
 SmiSize sz = 0;
 SmiSize offset = 0;

 sz = tinParts.size() * TinPart::getSizeOnDisc_head()
   + Tin::getPartHeadOrigin();

 char * buffer = new char[sz];

 WriteVar<TIN_SIZE>(config.maxSizePart, buffer, offset);
 WriteVar<AbstractType>(config.abstractType, buffer, offset);

 LOG("AbstractType written:")
 LOG(config.abstractType)
 LOG("Parts written:")
 LOG(noParts)

 WriteVar<TIN_SIZE>(noParts, buffer, offset);

 if (!rtree)
  rtree = new R_Tree<2, uint32_t>(
    WinUnix::getPageSize() - 2 * 2 * int(sizeof(double)));

 fileid = rtree->FileId();
 LOG("rtree fileid:")
 LOG(fileid)

 WriteVar<SmiFileId>(fileid, buffer, offset);

 features.serialize(buffer, offset);

 LOGP

 deque<TinPart*>::iterator it;
 it = tinParts.begin();

 while (it != tinParts.end()) {
  (*it)->serialize(buffer, offset);
  ++it;
 }

 if (sz != contentRec.Write(buffer, sz, 0))
  throw std::runtime_error(E_TINTYPE_SAVE1);

 delete[] buffer;
//////////////////////////////////////////////

 LOGP
 return true;
}

SmiRecordFile* Tin::getFile() {
 SmiRecord contentRec;

 LOGP
 if (!file.IsOpen()) {
  if (config.memoryState == INMEMORY)
   throw std::runtime_error(
     "This tin is configured stay in memory. No file.(Tin::getFile)");

  file.Create();
  file.AppendRecord(contentId, contentRec);
  contentRec.Finish();
 }

 return &file;
}
#endif
/*
 The method findNeighbor tries to find the neighbor of the triangle caller
 at the edge commonEdge. This is necessary at the boundary of a TinPart,
 since boundary neighbors are not persisted due to small pointers.

*/
Triangle* Tin::findNeighbor(const Edge& commonEdge, Triangle* caller) {
 LOGP

 deque<TinPart *>::iterator it = tinParts.begin();
 Triangle * result;

 while (it != tinParts.end()) {

  result = (*it)->findEdgeInPart(commonEdge, caller);
  if (result)
   return result;

  it++;
 }
 LOGP

 return 0;
}

#ifndef UNIT_TEST

std::string Tin::BasicType() {
 return "tin";
}
std::string TinAttribute::BasicType() {
 return "tinattribute";
}

ListExpr Tin::Property() {
 ListExpr listreplist = nl->TextAtom();
 ListExpr examplelist = nl->TextAtom();
 ListExpr remarks = nl->TextAtom();
 nl->AppendText(listreplist,
   "( (maximumpartsize)  ( ((triangle1_x1 triangle1_y1 triangle1_z1)"
     "(triangle1_x2 triangle1_y2 triangle1_z2)(triangle1_x3"
     " triangle1_y3 triangle1_z3))* )* )");
 nl->AppendText(examplelist,
   "( (800)  ( ((-1.0 0.0 1.0)(0.0 1.0 1.0)(1.0 0.0 3.0)) )  "
     " ( ((9.0 0.0 1.0)(10.0 1.0 1.0)(11.0 0.0 3.0)) ) )");
 nl->AppendText(remarks,
   "The TIN is partitioned in TinParts on disc. Operators "
     "are acting partwise. The maximum part size configures the"
     " size of these parts in bytes. Due to overhead, parts should"
     " have at least page size. Tins can be created from a raster"
     " or from a tuple stream. To visualize a tin use operator "
     "tin2stlfile and a tool like meshlab. ");
 return (nl->TwoElemList(
   nl->FiveElemList(nl->StringAtom("Signature"),
     nl->StringAtom("Example Type List"), nl->StringAtom("List Rep"),
     nl->StringAtom("Example List"), nl->StringAtom("Remarks")),
   nl->FiveElemList(nl->StringAtom("-> SIMPLE"),
     nl->StringAtom(Tin::BasicType()), listreplist, examplelist, remarks)));
}
ListExpr TinAttribute::Property() {
 ListExpr listreplist = nl->TextAtom();
 ListExpr examplelist = nl->TextAtom();
 ListExpr remarks = nl->TextAtom();
 nl->AppendText(listreplist,
   "( (maximumpartsize)   ((triangle1_x1 triangle1_y1 triangle1_z1)"
     "(triangle1_x2 triangle1_y2 triangle1_z2)(triangle1_x3 triangle1_y3 "
     "triangle1_z3))* )");
 nl->AppendText(examplelist,
   "( (800) ((-1.0 0.0 1.0)(0.0 1.0 1.0)(1.0 0.0 3.0))    "
     " ((9.0 0.0 1.0)(10.0 1.0 1.0)(11.0 0.0 3.0))  )");
 nl->AppendText(remarks,
   "The tinattribute is actually a tin, but with the "
     "ability to be used as an attribute. A tinattribute"
     "consistes just of one TinPart. Tins can be converted"
     "to tinattributes with operator tin2tinattribute.");

 return (nl->TwoElemList(
   nl->FiveElemList(nl->StringAtom("Signature"),
     nl->StringAtom("Example Type List"), nl->StringAtom("List Rep"),
     nl->StringAtom("Example List"), nl->StringAtom("Remarks")),
   nl->FiveElemList(nl->StringAtom("-> DATA"),
     nl->StringAtom(TinAttribute::BasicType()), listreplist, examplelist,
     remarks)));
}

Word Tin::Clone(const ListExpr typeInfo, const Word& w) {
 LOGP
 Word result;
 Tin * p = static_cast<Tin*>(w.addr);
 Tin * clone = 0;

 try {
  if (p) {
   clone = p->clone();
   if (clone) {

    clone->setDefined();
   } else
    clone = new Tin(TinConfiguration::DEFAULT);
  } else {
   clone = new Tin(TinConfiguration::DEFAULT);
  }
 } catch (std::exception& e) {
  if (!clone)
   clone = new Tin(TinConfiguration::DEFAULT);

  result.addr = clone;

  return result;
 }

 result.addr = clone;

 return result;
}
Word TinAttribute::Clone(const ListExpr typeInfo, const Word& w) {
 LOGP
 Word result;
 TinAttribute * p = static_cast<TinAttribute*>(w.addr);

 if (p)
  result = SetWord(p->Clone());
 else
  result = SetWord(new TinAttribute(false));

 return result;
}

int Tin::sizeOfObj() {

 return sizeof(Tin);

}
int TinAttribute::sizeOfObj() {
 LOGP
 return sizeof(TinAttribute);

}
bool Tin::Open(SmiRecord& valueRecord, size_t& offset,
  const ListExpr typeInfo, Word& value) {
 LOGP
 int fileState;
 Tin * tt = 0;

 try {

  tt = new Tin(TinConfiguration::DEFAULT);
  value.addr = tt;

  valueRecord.SetPos(offset);
  valueRecord.Read(fileState);

  if (fileState == FILE_DAMAGED)
   throw std::runtime_error(E_TIN_OPEN2);

//start with RANDOMACCESS and let operator decide
  tt->open(valueRecord);

  offset = valueRecord.GetPos();

 } catch (std::exception &e) {
  OUT_EXCEPT(e);
  LOGP
  return false;
 }

 tt->setDefined();
 LOGP
 return true;
}

bool Tin::Save(SmiRecord& valueRecord, size_t& offset,
  const ListExpr typeInfo, Word& value) {
 LOGP
 Tin * tt = static_cast<Tin*>(value.addr);
 LOG(tt)
 int fileState;

 try {
  valueRecord.SetPos(offset);

  if (!tt || !tt->isDefined()) {
   fileState = FILE_DAMAGED;
   valueRecord.Write(fileState);

   if (tt) {
    tt->deleteFile();

    if (tt->rtree)
     tt->rtree->DeleteFile();
   }

   throw std::runtime_error(E_TIN_SAVE);
  } else {
   fileState = FILE_OK;
   valueRecord.Write(fileState);
  }

  tt->setMemoryState(GRADUALFILE);
  tt->save(valueRecord);
  offset = valueRecord.GetPos();

 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  LOGP
  return false;
 }

 LOGP
 return true;
}

bool TinAttribute::Save(SmiRecord& valueRecord, size_t& offset,
  const ListExpr typeInfo, Word& value) {
 LOGP
 TinAttribute *bf = static_cast<TinAttribute*>(value.addr);

 bf->Serialize2Flob();

// This Save function is implemented in the Attribute class
// and uses the same method of the Tuple manager to save objects
 Attribute::Save(valueRecord, offset, typeInfo, bf);
 return true;
}

bool TinAttribute::Open(SmiRecord& valueRecord, size_t& offset,
  const ListExpr typeInfo, Word& value) {
 LOGP
// This Open function is implemented in the Attribute class
// and uses the same method of the Tuple manager to open objects
 TinAttribute * at = static_cast<TinAttribute*>(Attribute::Open(
   valueRecord, offset, typeInfo));

 value = SetWord(at);
 return true;
LOGP}

Word Tin::Create(const ListExpr typeInfo) {
 LOGP
 return (SetWord(new Tin(TinConfiguration::DEFAULT)));
}
Word TinAttribute::Create(const ListExpr typeInfo) {
 LOGP
 TinAttribute * attr = new TinAttribute(false);

 Word ret = (SetWord(attr));
 LOGP

 LOG(ret.addr)
 return ret;
}

void Tin::deleteFile() {
 LOGP
 if (file.IsOpen()) //a new Tin has no file yet
 {
  LOGP
  file.Close();
  file.Drop();
 }
LOGP}
void Tin::Delete(const ListExpr typeInfo, Word& w) {
 LOGP
 deque<TinPart*>::iterator it;
 Tin * ptt = (Tin*) ((((w.addr))));

 if (ptt) {
  if (ptt->rtree)
   ptt->rtree->DeleteFile();

  ptt->deleteFile();
  delete ptt;
  w.addr = 0;
 }

LOGP}

void TinAttribute::Delete(const ListExpr typeInfo, Word& w) {
 LOGP
 TinAttribute * ptt = (TinAttribute*) ((((w.addr))));

 ptt->binData.destroy();

 delete ptt;
 w.addr = 0;

LOGP}
ListExpr Tin::Out(ListExpr typeInfo, Word value) {
 LOGP
 Tin* t = static_cast<Tin*>(value.addr);
 LOG(t)
 ListExpr ret, last;
 deque<TinPart*>::iterator it;
 LOGP
 try {
  if (!t || !t->isDefined())
   throw std::runtime_error(E_TIN_OUT);

  LOGP
  LOG_EXP(t->features.print())
  LOG(t->tinParts.size())
  LOG(&(t->tinParts))

  if (t->estimateMaxSizeInMemory() > OPERATE_IN_MEMORY_THRESHOLD)
   t->setMemoryStateGradual();
  else
   t->setMemoryState(INMEMORY);

  it = t->tinParts.begin();
  if (it != t->tinParts.end()) {
   last = (*it)->outPart();
   (*it)->unloadContentData();
   ret = nl->OneElemList(
     nl->OneElemList(nl->IntAtom(t->config.maxSizePart)));
   last = nl->Append(ret, last);
   ++it;
  } else
   ret = nl->Empty();
  LOGP

  while (it != t->tinParts.end()) {
   LOG(nl->ToString(last))
   last = nl->Append(last, (*it)->outPart());
   (*it)->unloadContentData();
   ++it;
  }

 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  LOGP
  return nl->Empty();
 }
 return ret;
}
ListExpr TinAttribute::Out(ListExpr typeInfo, Word value) {
 LOGP
 TinAttribute* tinattr = static_cast<TinAttribute*>(value.addr);

 tinattr->syncFlob(false);

 ListExpr ret, last;
 LOGP
 try {
  if (!tinattr || !tinattr->IsDefined())
   throw std::runtime_error(E_TIN_OUT);
  LOGP
  LOG_EXP(tinattr->features.print())
  last = tinattr->outPart();
  ret = nl->TwoElemList(nl->IntAtom(tinattr->config.maxSizePart), last);
LOGP} catch (std::exception & e) {
 OUT_EXCEPT(e);
 LOGP
 return nl->Empty();
}
 return ret;
}
/*
 Parses the configuration of a TIN from a nested list.
 For now just the maximum size of a TinPart is parsed.

*/
void Tin::parseInTinConfig(const ListExpr confExp,
  TinConfiguration& conf) {
 string intype;
 int partSize;

 if (nl->IsEmpty(confExp)) //easy default config
   {
  conf.maxSizePart = TIN_PART_STANDARD_SIZE;
  return;
 } else {
  if (nl->ListLength(confExp) != 1)
   throw std::invalid_argument(E_PARSEINTINCONFIG1);
  if (!nl->IsAtom(nl->First(confExp)))
   throw std::invalid_argument(E_PARSEINTINCONFIG2);
  if (!(nl->AtomType(nl->First(confExp)) == IntType))
   throw std::invalid_argument(E_PARSEINTINCONFIG3);

  partSize = nl->IntValue(nl->First(confExp));

  conf.maxSizePart = partSize;
 }
}
Word Tin::In(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr& errorInfo, bool& correct) {
 LOGP
 Word obj = SetWord(Address(0));
 Vertex arVertices[3];
 ListExpr parts, currentPart, currentTriangle;
 int noPartsl = 0;
 int noTrianglesl = 0;
 TinConfiguration conf;
 Tin * intin = 0;
 Triangle * t = 0;

 correct = true;
 try {

  conf = TinConfiguration::DEFAULT;
  intin = new Tin(conf);
  obj.addr = intin;

  Tin::parseInTinConfig(nl->First(instance), conf);

  intin->config = conf;

  parts = nl->Rest(instance);

  noPartsl = nl->ListLength(parts);

  if (intin->estimateMaxSizeInMemory(
    noPartsl) > OPERATE_IN_MEMORY_THRESHOLD)
   intin->setMemoryStateGradual();
  else
   intin->setMemoryState(INMEMORY);

  LOG("noParts")
  LOG(noPartsl)

  while (noPartsl) {
   LOGP
   currentPart = nl->First(parts);
   parts = nl->Rest(parts);

   LOG(nl->ToString(currentPart))

//this makes a separate part for the triangles to come
   intin->initSimpleLayout();

   if (nl->IsEmpty(currentPart))
    throw std::invalid_argument(E_TINTYPE_IN2);

   noTrianglesl = nl->ListLength(currentPart);

   LOG("noTriangles")
   LOG(noTrianglesl)

   while (noTrianglesl) {
    currentTriangle = nl->First(currentPart);
    currentPart = nl->Rest(currentPart);
    LOGP
    LOG(nl->ToString(currentTriangle))
    if (nl->ListLength(currentTriangle) != 3)
     throw std::runtime_error(
       "There is a triangle not containing 3 vertices. (Tin::In)");

    for (int i = 0; i < 3; i++) {
     Vertex::parseVertex(nl->First(currentTriangle), &arVertices[i]);
     currentTriangle = nl->Rest(currentTriangle);
    }

    intin->addTriangle(arVertices[0], arVertices[1], arVertices[2], &t);

    LOGP
    noTrianglesl--;
   }

   noPartsl--;
  }

  if (intin)
   intin->finishLayout(false);
 } catch (std::exception& e) {

  OUT_EXCEPT(e);
  correct = false;
  LOGP
  return obj;
 }

 intin->setDefined();
 LOG(intin)
 LOG(intin->config.memoryState)

 LOGP
 return obj;
}
Word TinAttribute::In(const ListExpr typeInfo, const ListExpr instance,
  const int errorPos, ListExpr& errorInfo, bool& correct) {
 LOGP
 Word obj = SetWord(Address(0));
 Vertex arVertices[3];
 ListExpr part, currentTriangle;
 int noTrianglesl = 0;
 TinConfiguration conf;
 TinAttribute * intin = 0;
 Triangle * t = 0;

 correct = true;
 try {
  intin = new TinAttribute(false);
  obj.addr = intin;

  Tin::parseInTinConfig(nl->First(instance), conf);

  intin->config.maxSizePart = conf.maxSizePart;
  intin->initContentMemory();

  part = nl->Rest(instance);

  LOG(nl->ToString(part))

  noTrianglesl = nl->ListLength(part);

  LOG("noTriangles")
  LOG(noTrianglesl)

  while (noTrianglesl) {
   currentTriangle = nl->First(part);
   part = nl->Rest(part);
   LOGP
   LOG(nl->ToString(currentTriangle))
   if (nl->ListLength(currentTriangle) != 3)
    throw std::runtime_error(
      "There is a triangle not containing 3 vertices. (Tin::In)");

   for (int i = 0; i < 3; i++) {
    Vertex::parseVertex(nl->First(currentTriangle), &arVertices[i]);
    currentTriangle = nl->Rest(currentTriangle);
   }

   intin->addTriangle(arVertices[0], arVertices[1], arVertices[2], &t);

   LOGP
   noTrianglesl--;
  }

 } catch (std::exception& e) {

  OUT_EXCEPT(e);
  correct = false;
  LOGP
  return obj;
 }

 intin->SetDefined(true);
 intin->syncFlob(true);
 LOG_EXP(intin->features.print());
 LOGP
 return obj;
}
void Tin::Close(const ListExpr typeInfo, Word& w) {
 delete (Tin*) ((((w.addr))));
 w.addr = 0;
LOGP}
void TinAttribute::Close(const ListExpr typeInfo, Word& w) {
 delete (TinAttribute*) ((((w.addr))));
 w.addr = 0;
LOGP}
ListExpr Tin::atlocation_tm(ListExpr args) {
 LOGP
 return mappings::SimpleMaps<2, 4>(map_atlocation, args);
}
int Tin::atlocation_sf(ListExpr args) {
 return mappings::SimpleSelect<2, 4>(map_atlocation, args);
}
int Tin::atlocation_vm(Word* args, Word& result, int message, Word& local,
  Supplier s) {
 CcReal* x = static_cast<CcReal*>(args[1].addr);
 CcReal* y = static_cast<CcReal*>(args[2].addr);
 Tin* tt = static_cast<Tin*>(args[0].addr);
 LOGP
 result = qp->ResultStorage(s);

 CcReal* r = static_cast<CcReal*>(result.addr);

 try {
  if (!tt || !tt->isDefined())
   throw std::runtime_error("Null pointer or undefined as input."
     " Operation not possible. (Tin::atlocation_vm)");
  double result_d = tt->atlocation(Point_p(x->GetValue(), y->GetValue()));

  if (result_d == ERROR_VALUE)
   r->SetDefined(false);
  else
   r->Set(true, result_d);

 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  r->SetDefined(false);
  return -1;
 }

 LOGP
 return 0;
}
ListExpr Tin::tinmin_tm(ListExpr args) {
 return mappings::SimpleMaps<2, 2>(map_min, args);
}
int Tin::tinmin_sf(ListExpr args) {
 return mappings::SimpleSelect<2, 2>(map_min, args);
}
int Tin::tinmin_vm(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Tin* tt = static_cast<Tin*>(args[0].addr);
 LOGP
 result = qp->ResultStorage(s);

 CcReal* r = static_cast<CcReal*>(result.addr);

 if (tt && tt->isDefined())
  r->Set(true, tt->features.m_minValue);
 else
  r->SetDefined(false);

 LOGP
 return 0;
}
ListExpr Tin::tinmax_tm(ListExpr args) {
 return mappings::SimpleMaps<2, 2>(map_min, args);
}
int Tin::tinmax_sf(ListExpr args) {
 return mappings::SimpleSelect<2, 2>(map_min, args);
}
int Tin::tinmax_vm(Word* args, Word& result, int message, Word& local,
  Supplier s) {

 Tin* tt = static_cast<Tin*>(args[0].addr);
 LOGP
 result = qp->ResultStorage(s);

 CcReal* r = static_cast<CcReal*>(result.addr);

 if (tt && tt->isDefined())
  r->Set(true, tt->features.m_maxValue);
 else
  r->SetDefined(false);

 LOGP
 return 0;
}
int TinAttribute::tinmax_vm(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 TinAttribute* atin = static_cast<TinAttribute*>(args[0].addr);

 LOGP
 LOG(atin->features.m_minValue);LOG(atin->features.m_maxValue);

 result = qp->ResultStorage(s);
 CcReal* r = static_cast<CcReal*>(result.addr);

 if (!atin || !atin->IsDefined()) {
  r->SetDefined(false);
  return -1;
 }

//no sync necessary since just header data used
 r->Set(true, atin->features.m_maxValue);

 LOGP
 return 0;
}
int TinAttribute::tinmin_vm(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 TinAttribute* atin = static_cast<TinAttribute*>(args[0].addr);

 LOGP

 result = qp->ResultStorage(s);
 CcReal* r = static_cast<CcReal*>(result.addr);

 if (!atin || !atin->IsDefined()) {
  r->SetDefined(false);
  return -1;
 }

//no sync necessary since just header data used
 r->Set(true, atin->features.m_minValue);

 LOGP
 return 0;
}
int TinAttribute::atlocation_vm(Word* args, Word& result, int message,
  Word& local, Supplier s) {
 TinAttribute * tt = static_cast<TinAttribute *>(args[0].addr);
 LOGP
 CcReal* x = static_cast<CcReal*>(args[1].addr);
 CcReal* y = static_cast<CcReal*>(args[2].addr);
 result = qp->ResultStorage(s);

 CcReal* r = static_cast<CcReal*>(result.addr);

 try {
  if (!tt || !tt->IsDefined())
   throw std::runtime_error(
     "Null. Operation not possible. (TinAttribute::atlocation_vm)");

  tt->syncFlob(false);

  double result_d = tt->atlocation(Point_p(x->GetValue(), y->GetValue()));

  if (result_d == ERROR_VALUE)
   r->SetDefined(false);
  else
   r->Set(true, result_d);

 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  r->SetDefined(false);
  return -1;
 }

 LOGP
 return 0;
}
ListExpr Tin::unaryOp_tm(ListExpr args) {

//tin x (real -> real) -> tin

//two arguments
 if (nl->ListLength(args) != 2)
  return NList::typeError("Expected two arguments.(Tin::unaryOp_tm)");

//check both types
 if (!nl->IsAtom(nl->First(args))
   || !(nl->SymbolValue(nl->First(args)) == Tin::BasicType())
   || !listutils::isMap<1>(nl->Second(args)))
  return NList::typeError("Expected type tin as first argument and a "
    "mapping function as second argument.(Tin::unaryOp_tm)");

//check mapping functions parameter types
 if (!(nl->SymbolValue(nl->Second(nl->Second(args))) == CcReal::BasicType()))
  return NList::typeError(
    "Expected function having one argument of type real.(Tin::unaryOp_tm)");
 if (!(nl->SymbolValue(nl->Third(nl->Second(args))) == CcReal::BasicType()))
  return NList::typeError(
    "Expected function returning type real.(Tin::unaryOp_tm)");
 LOGP
 return NList(Tin::BasicType()).listExpr();
}
int Tin::unaryOp_vm(Word* args, Word& result, int message, Word& local,
  Supplier s) {
 void* function = args[1].addr;
 Tin* tt = static_cast<Tin*>(args[0].addr);

 LOGP
 result = qp->ResultStorage(s);
 Tin* nt = (Tin*) result.addr;

 try {
  if (!tt || !tt->isDefined())
   throw std::runtime_error("Null pointer or undefined as input. "
     "Operation not possible. (Tin::unaryOp_vm)");
  LOGP
  tt->unaryOp(function, nt);
LOGP} catch (std::exception & e) {
 OUT_EXCEPT(e);
 return -1;
}

 nt->setDefined();
 LOGP
 return 0;
}
ListExpr Tin::raster2tin_tm(ListExpr args) {

//sreal/sint  -> tin

 if (nl->ListLength(args) != 2)
  return NList::typeError("Expected two arguments.(Tin::raster2tin_tm)");

 string raster = nl->ToString(nl->First(args));
 if (!((raster == raster2::sreal::BasicType())
   || (raster == raster2::sint::BasicType())))
  return NList::typeError(
    "Expected first argument of type sreal.(Tin::raster2tin_tm)");

 if (!nl->IsAtom(nl->Second(args)) || !nl->IntAtom(nl->Second(args)))
  return NList::typeError("Expected second argument of type sint"
    " (size of TinParts in bytes).(Tin::raster2tin_tm)");

 return NList(Tin::BasicType()).listExpr();
}
int Tin::raster2tin_sf(ListExpr args) {
 string raster = nl->ToString(nl->First(args));
 if ((raster == raster2::sreal::BasicType()))
  return 1;
 else
  return 0;
}
ListExpr Tin::createTin_tm(ListExpr args) {

 ListExpr first = nl->First(args);
 ListExpr attrList;

 if (nl->ListLength(args) != 2 || !listutils::isTupleStream(first)
   || !nl->IsAtom(nl->Second(args)) || !nl->IntAtom(nl->Second(args))) {
  return NList::typeError(E_CREATETIN_TM);
 }

 attrList = nl->Second(nl->Second(first));

 if (nl->ListLength(attrList) != 2
   || !nl->IsAtom(nl->Second(nl->First(attrList)))
   || nl->SymbolValue(nl->Second(nl->First(attrList)))
     != ::Point::BasicType()
   || !nl->IsAtom(nl->Second(nl->Second(attrList)))
   || (nl->SymbolValue(nl->Second(nl->Second(attrList)))
     != ::CcReal::BasicType()
     && nl->SymbolValue(nl->Second(nl->Second(attrList)))
       != ::CcInt::BasicType())) {
  return NList::typeError(E_CREATETIN_TM2);
 }

 return nl->SymbolAtom(Tin::BasicType());
}
int Tin::createTin_sf(ListExpr args) {
 if (nl->SymbolValue(
   nl->Second(nl->Second(nl->Second(nl->Second(nl->First(args))))))
   == CcReal::BasicType())
  return 0;
 else
  return 1;
}

ListExpr Tin::tin2tuplestream_tm(ListExpr args) {

 if (nl->ListLength(args) != 1)
  return NList::typeError(
    "The operator tin2tuplestream expects exactly one argument.");

 if (!nl->IsAtom(nl->First(args))
   || nl->SymbolValue((nl->First(args))) != Tin::BasicType())
  return NList::typeError(
    "The operator tin2tuplestream expects a tin as argument.");

 NList tuplelist;

 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("V1"),
     nl->SymbolAtom(::Point::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("H1"),
     nl->SymbolAtom(CcReal::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("V2"),
     nl->SymbolAtom(::Point::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("H2"),
     nl->SymbolAtom(CcReal::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("V3"),
     nl->SymbolAtom(::Point::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("H3"),
     nl->SymbolAtom(CcReal::BasicType())));
 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("Part"),
     nl->SymbolAtom(CcInt::BasicType())));

 return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
   nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
     tuplelist.listExpr()));
}
/*
 pdcheck

*/
int Tin::tin2tuplestream_vm(Word* args, Word& result, int message,
  Word& local, Supplier s) {

 Tin* tin;
 tin2tuplestreamState *state = 0;
 ListExpr tupleType;

 try {
  switch (message) {
   case OPEN:

    tin = (Tin*) args[0].addr;

    if (!tin || !tin->isDefined()) {
     throw std::runtime_error(
       "Input is null pointer or undefined.(Tin::tin2tuplestream)");
    }
    if (tin->estimateMaxSizeInMemory() > OPERATE_IN_MEMORY_THRESHOLD)
     tin->setMemoryStateGradual();
    else
     tin->setMemoryState(INMEMORY);
    tupleType = GetTupleResultType(s);
    state = new tin2tuplestreamState;
    state->itTriangles = new Tin::triangle_iterator(tin);
    state->tupleType = new TupleType(nl->Second(tupleType));
    local = SetWord(state);
    LOGP
    return 0;

   case REQUEST:
    LOGP
    if (local.addr)
     state = (tin2tuplestreamState*) local.addr;
    else
     return CANCEL;

    if ((*(*state->itTriangles))) {
     Tuple * t = new Tuple(state->tupleType);

     const Vertex * v1 = (*(*state->itTriangles))->getVertex(1);
     const Vertex * v2 = (*(*state->itTriangles))->getVertex(2);
     const Vertex * v3 = (*(*state->itTriangles))->getVertex(3);

     ::Point * p = new ::Point(true, v1->getX(), v1->getY());
     CcReal * h = new CcReal(true, v1->getZ());
     t->PutAttribute(0, p);
     t->PutAttribute(1, h);

     p = new ::Point(true, v2->getX(), v2->getY());
     h = new CcReal(true, v2->getZ());
     t->PutAttribute(2, p);
     t->PutAttribute(3, h);

     p = new ::Point(true, v3->getX(), v3->getY());
     h = new CcReal(true, v3->getZ());
     t->PutAttribute(4, p);
     t->PutAttribute(5, h);

     CcInt *part = new CcInt(state->itTriangles->getCurrentPart());
     t->PutAttribute(6, part);

     (*state->itTriangles)++;

     result = SetWord(t);
     return YIELD;
    } else
     return CANCEL;

   case CLOSE:
    if (local.addr) {
     state = (tin2tuplestreamState*) local.addr;
     LOGP
     delete state;
     local.addr = 0;
    }
    return 0;

  }
 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  result.addr = 0;
  return CANCEL;
 }

 return 0;
}
ListExpr Tin::tin2tinattribute_tm(ListExpr args) {

 if (nl->ListLength(args) != 1)
  return NList::typeError(
    "The operator tin2tinattribute expects exactly one argument.");

 if (!nl->IsAtom(nl->First(args))
   || nl->SymbolValue((nl->First(args))) != Tin::BasicType())
  return NList::typeError(
    "The operator tin2tinattribute expects a tin as argument.");

 NList tuplelist;

 tuplelist.append(
   nl->TwoElemList(nl->SymbolAtom("TinPart"),
     nl->SymbolAtom(TinAttribute::BasicType())));

 return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
   nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
     tuplelist.listExpr()));

}
TinAttribute::TinAttribute(TinPart & part) :
  binData(0) {
 bool n;
 const Vertex * v1, *v2, *v3;
 LOGP
 part.loadContentData();

 config = part.tin->getConfiguration();
 config.memoryState = INMEMORY;
 noTriangles = 0;
 features = part.features;
 contentLoaded = true;
 ismodified = true;
 pVertexContainer = part.pVertexContainer->clone_empty();

 arTriangles = new Triangle[part.noTriangles];

 for (int i = 0; i < part.noTriangles; i++) {
  v1 = pVertexContainer->insertVertex(part.arTriangles[i].getVertex(1), n);
  v2 = pVertexContainer->insertVertex(part.arTriangles[i].getVertex(2), n);
  v3 = pVertexContainer->insertVertex(part.arTriangles[i].getVertex(3), n);

  constructTriangle(v1, v2, v3);
  noTriangles++;
LOGP}
 noVertices = pVertexContainer->getNoVertices();
 contentRefs = 0;
 noTrianglesMax = part.noTrianglesMax;
 SetDefined(true);
#ifndef UNIT_TEST
 isRecInitialized = false;
#endif
 syncFlob(true);

LOGP}
int Tin::tin2tinattribute_vm(Word* args, Word& result, int message,
  Word& local, Supplier s) {
 Tin* tin;
 tin2tinattributestreamState * state;
 ListExpr tupleType;

 try {
  switch (message) {
   case OPEN:

    tin = (Tin*) args[0].addr;

    if (!tin || !tin->isDefined()) {
     throw std::runtime_error(
       "Input is null pointer or undefined.(Tin::tin2tinattribute)");
    }

    if (tin->estimateMaxSizeInMemory() > OPERATE_IN_MEMORY_THRESHOLD)
     tin->setMemoryStateGradual();
    else
     tin->setMemoryState(INMEMORY);

    tupleType = GetTupleResultType(s);
    state = new tin2tinattributestreamState;
    state->tupleType = new TupleType(nl->Second(tupleType));
    state->itParts = tin->tinParts.begin();
    state->itEnd = tin->tinParts.end();
    local = SetWord(state);
    LOGP
    return 0;

   case REQUEST:
    LOGP
    if (local.addr)
     state = (tin2tinattributestreamState*) local.addr;
    else
     return CANCEL;

    if (state->itParts != state->itEnd) {
     Tuple * t = new Tuple(state->tupleType);
     TinAttribute * attr = new TinAttribute(*(*(state->itParts)));
     t->PutAttribute(0, attr);
     ++(state->itParts);
     result = SetWord(t);
     return YIELD;
    } else
     return CANCEL;

   case CLOSE:
    if (local.addr) {
     state = (tin2tinattributestreamState*) local.addr;
     LOGP
     delete state;
     local.addr = 0;
    }
    return 0;

  }
 } catch (std::exception & e) {
  OUT_EXCEPT(e);
  result.addr = 0;
  return CANCEL;
 }

 return 0;
}
/*
 pdcheck

*/
ListExpr TinAttribute::tinattribute2tin_tm(ListExpr args) {
 ListExpr attrList;
 ListExpr str = nl->First(args);

 if (nl->ListLength(args) != 1 || !listutils::isTupleStream(str)) {
  return NList::typeError("The operator tinattribute2tin "
    "expects exactly one argument of type"
    " tuple stream. (TinAttribute::tinattribute2tin_tm)");
 }

 attrList = nl->Second(nl->Second(str));

 if (nl->ListLength(attrList) != 1
   || !nl->IsAtom(nl->Second(nl->First(attrList)))
   || nl->SymbolValue(nl->Second(nl->First(attrList)))
     != TinAttribute::BasicType()) {

  return NList::typeError("The tuples of the input"
    " tuple stream have to consist of just one "
    "attribute of type tinattribute."
    "(TinAttribute::tinattribute2tin_tm)");
 }

 return nl->SymbolAtom(Tin::BasicType());

}
int TinAttribute::tinattribute2tin_sf(ListExpr args) {
 return 0;
}
TinPart* TinAttribute::clone(Tin*tt) {
 if (!IsDefined())
  return 0;
 LOGP
 LOG_EXP(features.print())
 syncFlob(false);
 return TinPart::clone(tt);
}
int TinAttribute::tinattribute2tin_vm(Word* args, Word& result,
  int message, Word& local, Supplier s) {
 LOGP

 Stream<Tuple> stream(args[0]);
 result = qp->ResultStorage(s);
 Tin * nt = (Tin*) result.addr;
 Tuple * currentTuple;
 TinAttribute * currentTinattr;
 TinPart * copyPart;
 bool first = true;
 TinConfiguration targetConf;

 try {
  if (args[0].addr == 0)
   throw std::runtime_error("Null pointer as input."
     " Operation not possible. (TinAttribute::tinattribute2tin_vm)");
  LOGP

  stream.open();

  while ((currentTuple = stream.request()) != 0) {
   currentTinattr = (TinAttribute *) currentTuple->GetAttribute(0);

   if (first) {
    targetConf = currentTinattr->config;
    nt->resetConfiguration(targetConf);
    first = false;

   } else if (!(targetConf == currentTinattr->config)) {
    throw std::runtime_error("The stream of tinattributes contains"
      " an attribute with a different configuration."
      " (e.g. an attribute with a different part size)"
      " The conversion is not possible currently."
      " (TinAttribute::tinattribute2tin_vm)");
   }

   copyPart = currentTinattr->clone(nt);
   if (copyPart)
    nt->addPart(copyPart);

   currentTuple->DeleteIfAllowed();
  }

LOGP} catch (std::exception & e) {
 OUT_EXCEPT(e);
 nt->setDefined(false);
 stream.close();
 return -1;
}

 nt->setDefined(true);
 stream.close();

 LOGP
 return 0;
}
#endif

ColumnKey::ColumnKey(VERTEX_COORDINATE ixfrom, VERTEX_COORDINATE ixto,
  uint32_t inoPart) {
 xFrom = ixfrom;
 xTo = ixto;
 noPart = inoPart;
}
ColumnKey::ColumnKey(VERTEX_COORDINATE ix, uint32_t inoPart) {
 xFrom = ix;
 xTo = ix;
 noPart = inoPart;
}
ColumnKey::~ColumnKey() {

}

bool ColumnKey::operator<(const ColumnKey & ckey) const {

 if (ckey.xFrom == ckey.xTo) {
  if (ckey.xFrom < xFrom)
   return false;
  if (ckey.xFrom > xTo)
   return true;

  return false;
 }
 if (xFrom == xTo) {
  if (xFrom < ckey.xFrom)
   return true;
  if (xFrom > ckey.xTo)
   return false;

  return false;
 }

 if (xFrom < ckey.xFrom)
  return true;
 else
  return false;

}
/* namespace tin*/
}
