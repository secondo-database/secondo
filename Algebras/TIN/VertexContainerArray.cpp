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

#include "VertexContainerArray.h"
#include "SecondoDependencies.h"
namespace tin {

VertexContainerArray::VertexContainerArray(TIN_SIZE imaxsize) :
  AbstractVertexContainer(imaxsize) {

 if (maxSize < 0) {
  maxSize = (-maxSize) * Vertex::getSizeOnDisc()
    + getContainerSizeOnDisc();
 }

 if (getMaxVertexCount() < 2)
  throw std::invalid_argument(E_VERTEXCONTAINER_CONSTRUCTOR);

 arVertices = new Vertex[getMaxVertexCount()];
}

VertexContainerArray::~VertexContainerArray() {
 this->clear();
}
void VertexContainerArray::resize(TIN_SIZE imaxSize) {
 throw std::runtime_error("NOT IMPLEMENTED YET");
// if(getSizeOnDisc()<imaxSize)
//  maxSize = imaxSize;
// else
//  throw std::runtime_error(E_VERTEXCONTAINERSET_RESIZE);

}
void VertexContainerArray::unaryOp(VERTEX_Z (*op)(VERTEX_Z z),
  TinFeatures& feat) {

 for (int i = 0; i < noVertices; i++) {
  arVertices[i].setZ(op(arVertices[i].getZ()));
  feat.update(arVertices[i]);
 }
}
#ifndef UNIT_TEST
void VertexContainerArray::unaryOp(void * function, TinFeatures& feat) {
 VERTEX_Z z;
 ArgVector& arguments = *qp->Argument(function);
 Word result;

 double manipulated_z;
 result.addr = &manipulated_z;

 arguments[0].setAddr(&z);
 for (int i = 0; i < noVertices; i++) {
  z = arVertices[i].getZ();
  qp->Request(function, result);
  arVertices[i].setZ(manipulated_z);
  feat.update(arVertices[i]);
 }
}
#endif

void VertexContainerArray::loadVertices(const Vertex* parray,
  int numberofvertices) {
}

Vertex* VertexContainerArray::insertVertex(const Vertex* v,
  bool& newVertex) {
 if (v == 0)
  throw std::invalid_argument(E_VERTEXCONTAINER_INSERTVERTEX);

 if (noVertices >= getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_INSERTVERTEX2);

 newVertex = false;
 for (int i = 0; i < noVertices; i++) {
  if (arVertices[i] == *v) {
   if (arVertices[i].getZ() != v->getZ())
    throw std::invalid_argument(E_VERTEXCONTAINER_INSERTVERTEX3);

   return &arVertices[i];
  }
 }
 new (arVertices + noVertices) Vertex(*v);
 noVertices++;
 newVertex = true;
 return &arVertices[noVertices - 1];

}

void VertexContainerArray::removeVertex(const Vertex* v) {
 throw std::runtime_error(E_VERTEXCONTAINERSMALL_REMOVEVERTEX);
// for (int i = noVertices - 1; i >= 0; i--) {
//  if (arVertices[i] == *v) {
//   for (int j = i + 1; j < noVertices; j++) {
//    arVertices[j - 1] = arVertices[j];
//   }
//   noVertices--;
//   return;
//  }
//
// }

}

void VertexContainerArray::clear() {
 delete[] arVertices;
 noVertices = 0;
}

TIN_SIZE VertexContainerArray::getContainerSizeOnDisc() const {
 return sizeof(maxSize) + sizeof(noVertices);
}

void VertexContainerArray::print(std::ostream& os) const {
 os << "VertexContainerSmall: " << " maximum size on disc:"
   << getMaxSize();
 os << " current size on disc:"

 << VertexContainerArray::getSizeOnDiscStatic(noVertices)
   << " maximum vertex count: " << getMaxVertexCount()
   << " actual vertex count: " << noVertices << "\n";
 for (int i = 0; i < noVertices; i++) {
  arVertices[i].print(os);
 }
 os << "end VertexContainer.............";
}
#ifndef UNIT_TEST
ListExpr VertexContainerArray::outContainer() const {
 ListExpr ret=nl->TheEmptyList(), tmp, last;
 int i = 0;
 if (noVertices > 0) {
  ret = nl->OneElemList(arVertices[0].outVertex());
 }

 last = ret;

 for (i = 1; i < noVertices; i++) {
  tmp = arVertices[i].outVertex();
  last = nl->Append(last, tmp);
 }

 return ret;
}
bool VertexContainerArray::open(SmiRecord& valueRecord) {
 valueRecord.Read(this->maxSize);
 valueRecord.Read(noVertices);

 for (int i = 0; i < noVertices; i++) {
  new (arVertices + i) Vertex();
  arVertices[i].open(valueRecord);
 }
 return true;
}
void VertexContainerArray::rebuild(char* state, size_t & offset) {
 ReadVar<TIN_SIZE>(maxSize, state, offset);
 ReadVar<TIN_SIZE>(noVertices, state, offset);
 for (int i = 0; i < noVertices; i++) {
  arVertices[i].rebuild(state, offset);
 }
}
void VertexContainerArray::serialize(char* storage, size_t& offset) const {
 WriteVar<TIN_SIZE>(maxSize, storage, offset);
 WriteVar<TIN_SIZE>(noVertices, storage, offset);
 for (int i = 0; i < noVertices; i++) {
  arVertices[i].serialize(storage, offset);
 }
}
bool VertexContainerArray::save(SmiRecord& valueRecord) {
 valueRecord.Write(maxSize);
 valueRecord.Write(noVertices);
 for (int i = 0; i < noVertices; i++) {
  arVertices[i].save(valueRecord);
 }
 return true;
}
#endif
VertexContainerArray* VertexContainerArray::clone() {
 VertexContainerArray* result = new VertexContainerArray(maxSize);
 result->noVertices = noVertices;
 for (int i = 0; i < noVertices; i++) {
  new (result->arVertices + i) Vertex(arVertices[i]);
 }
 return result;
}
void VertexContainerArray::binaryOp(Tin& tt,
  VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2), TinFeatures& feat) {
 throw std::runtime_error(
   "VertexContainerSmall::binaryOp not implemented");
}
Vertex* VertexContainerArray::getVertex(const Vertex& v) const {
 for (int i = 0; i < noVertices; i++) {
  if (arVertices[i] == v) {
   if (arVertices[i].getZ() == v.getZ())
    return &arVertices[i];
  }
 }

 return 0;
}

VertexContainerArray* VertexContainerArray::clone_empty() {
 VertexContainerArray* result = new VertexContainerArray(maxSize);
 return result;
}

void VertexContainerArray::addVerticesTo(AbstractVertexContainer& vc) {
 for (int i = 0; i < noVertices; i++) {
  vc.insertVertex_p(&arVertices[i]);
 }
}

void VertexContainerArray::insertVertices(const VertexContainerSet& vc) {
 throw std::runtime_error(
   "not implemented(VertexContainerSmall::insertVertices)");
}

void VertexContainerArray::insertVertex_p(const Vertex* v) {
 if (noVertices >= getMaxVertexCount())
  throw std::runtime_error(E_VERTEXCONTAINER_INSERTVERTEX2);

//  for (int i = 0; i < noVertices; i++) {
//   if (arVertices[i] == *v) {
//    return;
//   }
//  }
 new (arVertices + noVertices) Vertex(*v);
 noVertices++;

}
int VertexContainerArray::getSizeInMemory() const {
 return noVertices * (sizeof(Vertex) + sizeof(Vertex*)) + sizeof(*this);
}
int VertexContainerArray::getYIndex(const Vertex * v) const {
 for (int i = 0; i < noVertices; i++) {
  if (*v == arVertices[i])
   return i;
 }

 return -1;
}
Vertex* VertexContainerArray::getVertexByYIndex(
  const FILE_VERTEX_POINTER i) const {
 if (i >= noVertices || i < 0)
  throw std::runtime_error(E_VERTEXCONTAINERSMALL_GETVERTEXBYINDEX);
 return &arVertices[i];
}
std::ostream& operator <<(std::ostream& os, VertexContainerArray& vc) {
 vc.print(os);
 return os;
}
/* namespace tin*/

}

