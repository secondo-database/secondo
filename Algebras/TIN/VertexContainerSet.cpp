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

#include "VertexContainerSet.h"
#include "Tin.h"
#include "stdlib.h"
#include "SecondoDependencies.h"

namespace tin {

VertexContainerSet::VertexContainerSet(TIN_SIZE imaxsize) :
  AbstractVertexContainer(imaxsize) {

 if (maxSize < 0) {
  maxSize = (-maxSize) * Vertex::getSizeOnDisc()
    + getContainerSizeOnDisc();
 }

 if (getMaxVertexCount() < 2)
  throw std::invalid_argument(E_VERTEXCONTAINER_CONSTRUCTOR);

 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
 tmpYorderedindex = 0;
 tmpXorderedindex = 0;
}

VertexContainerSet::~VertexContainerSet() {
 this->clear();
 noVertices = 0;
 if (tmpYorderedindex != 0)
  delete[] tmpYorderedindex;
 if (tmpXorderedindex != 0)
  delete[] tmpXorderedindex;
}
void VertexContainerSet::loadVertices(const Vertex* parray,
  int numberofvertices) {
 if (parray == 0)
  throw std::invalid_argument(E_VERTEXCONTAINER_LOADVERTICES);

 if ((int)(setVertices.size() + numberofvertices) > getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_LOADVERTICES2);

 while (numberofvertices > 0) {
  numberofvertices--;
  setVertices.insert(parray[numberofvertices]);
 }
 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
}

void VertexContainerSet::clear() {
 setVertices.clear(); //since vertices have no pointers memory is clean
 indexYUpToDate = false;
 indexXUpToDate = false;
 if (tmpYorderedindex)
  delete[] tmpYorderedindex;
 tmpYorderedindex = 0;
 if (tmpXorderedindex)
  delete[] tmpXorderedindex;
 tmpXorderedindex = 0;
}

const Vertex* VertexContainerSet::insertVertex(const Vertex* v,
  bool& newVertex) {
 if (v == 0)
  throw std::invalid_argument(E_VERTEXCONTAINER_INSERTVERTEX);

 if ((int)(setVertices.size() + 1) > getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_INSERTVERTEX2);

 std::pair<std::set<Vertex>::iterator, bool> r = setVertices.insert((*v));
 if ((*r.first).getZ() != v->getZ())
  throw std::invalid_argument(E_VERTEXCONTAINER_INSERTVERTEX3);

 newVertex = r.second;
 noVertices = setVertices.size();
 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
 return const_cast<Vertex*>(&(*r.first)); //TODO make this clean
}
void VertexContainerSet::resize(TIN_SIZE imaxSize) {

 if (getSizeOnDisc() < imaxSize)
  maxSize = imaxSize;
 else
  throw std::runtime_error(E_VERTEXCONTAINERSET_RESIZE);

}
void VertexContainerSet::insertVertex_p(const Vertex* v) {
 if ((int)(setVertices.size() + 1) > getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_INSERTVERTEX2);

 setVertices.insert((*v));
 noVertices = setVertices.size();
 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;

}
void VertexContainerSet::removeVertex(const Vertex* v) {
 this->setVertices.erase(*v);
 noVertices = setVertices.size();
 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
}

TIN_SIZE VertexContainerSet::getContainerSizeOnDisc() const {
 return sizeof(maxSize) + sizeof(noVertices);
}

void VertexContainerSet::print(std::ostream& os) const {
 std::set<Vertex>::iterator it;
 os << "VertexContainer: " << " Maximum size on disc:" << getMaxSize();
 os << " Current size on disc:"

 << VertexContainerSet::getSizeOnDiscStatic(noVertices)
   << " Maximum Vertex Count: " << getMaxVertexCount()
   << " Actual Vertex count: " << setVertices.size() << "\n";
 it = setVertices.begin();
 while (it != setVertices.end()) {
  (*it).print(os);
  os << "\n";
  it++;
 }
 os << "end VertexContainer.............";
}

VertexContainerSet* VertexContainerSet::clone() {
 VertexContainerSet* result = new VertexContainerSet(maxSize);
 result->noVertices = noVertices;
 result->setVertices = setVertices;
 result->indexYUpToDate = false;
 result->indexXUpToDate = false;
 result->initFeat = false;
 return result;
}
TinFeatures VertexContainerSet::getFeatures() const {

 if (noVertices < 1)
  return feat;

 if (initFeat == false) {
  std::set<Vertex>::iterator it;

  it = setVertices.begin();
  while (it != setVertices.end()) {
   feat.update((*it));
   ++it;
  }

  initFeat = true;
 }

 return feat;
}
const Vertex* VertexContainerSet::getVertex(const Vertex& v) const {
 std::set<Vertex>::iterator it;
 it = setVertices.find(v);
 if (it != setVertices.end())
  return const_cast<Vertex*>(&(*it));
 else
  return 0;
}
const Vertex* VertexContainerSet::getVertexByYIndex(
  const FILE_VERTEX_POINTER i) const {
 LOGP
 LOG(this)
 LOG(noVertices)
 if (noVertices < 1)
  return NULL;

 if (i > noVertices || i < 0)
  throw std::invalid_argument(E_VERTEXCONTAINER_GETVERTEXBYINDEX);

 if (indexYUpToDate == false) {
  LOGP
  if (i == noVertices - 1)
   return &(*(setVertices.rbegin()));

  if (i == 0)
   return &(*(setVertices.begin()));

  if (tmpYorderedindex != 0)
   delete[] tmpYorderedindex;

  tmpYorderedindex = new Vertex*[noVertices];
  int i2 = 0;
  std::set<Vertex>::iterator it;

  it = setVertices.begin();
  while (it != setVertices.end()) {
   tmpYorderedindex[i2++] = const_cast<Vertex*>(&(*it));
   ++it;
  }

  indexYUpToDate = true;
 }
 LOGP
 return tmpYorderedindex[i];
}
void VertexContainerSet::updateXindexAndFeatures() const {
 if (noVertices < 1)
  return;

 if (indexXUpToDate == false) {
  if (tmpXorderedindex != 0)
   delete[] tmpXorderedindex;

  tmpXorderedindex = new Vertex*[noVertices];
  int i2 = 0;
  std::set<Vertex>::iterator it;

  it = setVertices.begin();
  while (it != setVertices.end()) {
   tmpXorderedindex[i2++] = const_cast<Vertex*>(&(*it));
   feat.update((*it));
   ++it;
  }

  qsort(tmpXorderedindex, noVertices, sizeof(Vertex*), Vertex::compareByX);
  initFeat = true;
  indexXUpToDate = true;
 }

}
const Vertex* VertexContainerSet::getVertexByXIndex(const int i) const {
 if (noVertices < 1)
  return NULL;

 if (i > noVertices || i < 0)
  throw std::invalid_argument(E_VERTEXCONTAINER_GETVERTEXBYINDEX2);

 if (indexXUpToDate == false) {
  updateXindexAndFeatures();
 }

 return tmpXorderedindex[i];
}
void VertexContainerSet::unaryOp(VERTEX_Z (*op)(VERTEX_Z z),
  TinFeatures& feat) {

 std::set<Vertex>::iterator it = setVertices.begin();

 while (it != setVertices.end()) {
  Vertex* v = const_cast<Vertex*>(&(*it));

  v->setZ(op(v->getZ()));

  feat.update((*it));

  it++;
 }
}
#ifndef UNIT_TEST
void VertexContainerSet::unaryOp(void * function, TinFeatures& feat) {
 std::set<Vertex>::iterator it = setVertices.begin();

 Word result;
 CcReal vertex_z;

 ArgVector& arguments = *qp->Argument(function);
 arguments[0].setAddr(&vertex_z);

 while (it != setVertices.end()) {
  Vertex* v = const_cast<Vertex*>(&(*it));
  vertex_z.Set(v->getZ());
  result = qp->Request(function);
  v->setZ(((CcReal*) result.addr)->GetValue());
  feat.update((*it));
  ++it;
 }
}
#endif
void VertexContainerSet::binaryOp(Tin& tt,
  VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2), TinFeatures& feat) {
 std::set<Vertex>::iterator it = setVertices.begin();

 VERTEX_Z z_val;

 while (it != setVertices.end()) {
  Vertex* v = const_cast<Vertex*>(&(*it));
  Point_p p(v->getX(), v->getY());
  z_val = tt.atlocation(p);

  if (z_val == ERROR_VALUE) {
   feat.update(*v);
   it++;
   continue;
  } else
   v->setZ(op(v->getZ(), z_val));

  feat.update(*v);
  it++;
 }
}

AbstractVertexContainer* VertexContainerSet::clone_empty() {
 VertexContainerSet* result = new VertexContainerSet(maxSize);
 return result;
}

void VertexContainerSet::insertVertices(const VertexContainerSet& vc) {
 if ((int)(setVertices.size() + vc.setVertices.size() + 1)
   > getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_INSERTVERTICES);

 setVertices.insert(vc.setVertices.begin(), vc.setVertices.end());
 noVertices = setVertices.size();

 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
}

void VertexContainerSet::addVerticesTo(AbstractVertexContainer& vc) {
 vc.insertVertices(*this);
}
std::set<Vertex>::reverse_iterator
VertexContainerSet::getIteratorRbegin() const {
 return setVertices.rbegin();
}
void VertexContainerSet::removeVertex(
  std::set<Vertex>::reverse_iterator it) {

 setVertices.erase(++(it.base()));

}
std::set<Vertex>::reverse_iterator VertexContainerSet::getIteratorRend() const {
 return setVertices.rend();
}
#ifndef UNIT_TEST
void VertexContainerSet::serialize(char* storage, size_t& offset) const {
 LOGP
 std::set<Vertex>::iterator it;
 Vertex *v;
 WriteVar<TIN_SIZE>(maxSize, storage, offset);
 WriteVar<TIN_SIZE>(noVertices, storage, offset);

 LOG(noVertices)

 it = setVertices.begin();
 while (it != setVertices.end()) {
  v = const_cast<Vertex*>(&(*it));
  v->serialize(storage, offset);
  ++it;
 }
LOGP}
void VertexContainerSet::rebuild(char* state, size_t & offset) {
 LOGP
 LOG(this)
 std::set<Vertex>::iterator it;
 Vertex v;
 ReadVar<TIN_SIZE>(maxSize, state, offset);
 ReadVar<TIN_SIZE>(noVertices, state, offset);

 LOG(noVertices)

 for (int i = 0; i < noVertices; i++) {
  v.rebuild(state, offset);
  setVertices.insert(v);
 }

 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
LOGP}
ListExpr VertexContainerSet::outContainer() const {
 LOGP
 std::set<Vertex>::iterator it;
 it = setVertices.begin();
 ListExpr ret=nl->TheEmptyList(), tmp, last;
 if (it != setVertices.end()) {
  ret = nl->OneElemList((*it).outVertex());
  it++;
 }
 last = ret;
 while (it != setVertices.end()) {
  tmp = (*it).outVertex();
  last = nl->Append(last, tmp);
  it++;
 }

 return ret;
}
bool VertexContainerSet::open(SmiRecord& valueRecord) {
 LOGP
 Vertex v;
 valueRecord.Read(maxSize);
 valueRecord.Read(noVertices);
 std::set<Vertex>::iterator preceedingIt;
 LOG(noVertices)

 preceedingIt = setVertices.begin();

 for (int i = 0; i < noVertices; i++) {
  v.open(valueRecord);
  preceedingIt = setVertices.insert(preceedingIt, v);
 }

 indexYUpToDate = false;
 indexXUpToDate = false;
 initFeat = false;
 LOGP
 return true;
}
bool VertexContainerSet::save(SmiRecord& valueRecord) {
 LOGP
 std::set<Vertex>::iterator it;
 Vertex *v;
 valueRecord.Write(maxSize);
 valueRecord.Write(noVertices);

 LOG(noVertices)

 it = setVertices.begin();
 while (it != setVertices.end()) {
  v = const_cast<Vertex*>(&(*it));
  v->save(valueRecord);
  ++it;
 }
 LOGP
 return true;
}
#endif

int VertexContainerSet::getYIndex(const Vertex * v) const {
 if (noVertices < 1)
  return -1;

 if (indexYUpToDate == false) {
  if (tmpYorderedindex != 0)
   delete[] tmpYorderedindex;

  tmpYorderedindex = new Vertex*[noVertices];
  int i = 0;
  std::set<Vertex>::iterator it;

  it = setVertices.begin();
  while (it != setVertices.end()) {
   tmpYorderedindex[i++] = const_cast<Vertex*>(&(*it));
   ++it;
  }

  indexYUpToDate = true;
 }

 Vertex **pv = (Vertex **) bsearch(v, tmpYorderedindex, noVertices,
   sizeof(Vertex*), Vertex::compareIndex);

 if (pv == NULL)
  return -1;

 return (pv - tmpYorderedindex);

}

int VertexContainerSet::getSizeInMemory() const {
 int sizexindex = 0, sizeyindex = 0;
 if (tmpXorderedindex != 0)
  sizexindex = sizeof(Vertex*) * noVertices;
 if (tmpYorderedindex != 0)
  sizeyindex = sizeof(Vertex*) * noVertices;

 return sizeof(*this) + noVertices * (24 + sizeof(Vertex)) + sizexindex
   + sizeyindex; //Just a guess !!!
}

std::ostream& operator <<(std::ostream& os, VertexContainerSet& vc) {
 vc.print(os);
 return os;
}

}
