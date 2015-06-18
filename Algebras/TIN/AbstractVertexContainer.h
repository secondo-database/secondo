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

#ifndef ABSTRACTVERTEXCONTAINER_H_
#define ABSTRACTVERTEXCONTAINER_H_
#include "TinHelper.h"

namespace tin {

class AbstractVertexContainer: public noncopyable {
protected:
 TIN_SIZE noVertices;
 TIN_SIZE maxSize;
public:
////////Construction/Destruction/////////////////////////////////
 AbstractVertexContainer(TIN_SIZE imaxsize =
   VERTEX_CONTAINER_STANDARD_MAX_SIZE) {
  noVertices = 0;
  maxSize = imaxsize;
 }
 virtual ~AbstractVertexContainer() {
  noVertices = 0;
 }
 virtual AbstractVertexContainer* clone()=0;
 virtual AbstractVertexContainer* clone_empty()=0;
////////Manipulation/////////////////////////////////////////////
 virtual const Vertex* insertVertex(const Vertex* v, bool & newVertex)=0;
 virtual void insertVertex_p(const Vertex* v)=0;
 virtual void insertVertices(const VertexContainerSet& vc)=0;
 virtual void loadVertices(const Vertex* parray,
   const int numberofvertices)=0;
 virtual void removeVertex(const Vertex * v)=0;
 virtual void clear()=0;
 virtual void addVerticesTo(AbstractVertexContainer& vc)=0;
 virtual void resize(TIN_SIZE maxSize)=0;
 virtual void unaryOp(VERTEX_Z (*op)(VERTEX_Z v), TinFeatures& feat)=0;
#ifndef UNIT_TEST
 virtual void unaryOp(void * function, TinFeatures& feat)=0;
#endif
 virtual void binaryOp(Tin& tt, VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2),
   TinFeatures& feat)=0;

///////Query/////////////////////////////////////////////////////
 virtual const Vertex* getVertex(const Vertex& v) const=0;
 virtual int getYIndex(const Vertex * v) const=0;
 virtual const Vertex* getVertexByYIndex(
   const FILE_VERTEX_POINTER i) const=0;
 virtual int getSizeInMemory() const=0;

 TIN_SIZE getMaxSize() const {
  return maxSize;
 }

 TIN_SIZE getMaxSizeInMemory() const;
 int getNoVertices() const {
  return noVertices;
 }

 virtual TIN_SIZE getMaxVertexCount() const;
 virtual TIN_SIZE countVerticesAddable() const;
 virtual bool isEmpty() const;
 virtual bool isFull() const;
///////Presentation//////////////////////////////////////////////
 virtual void print(std::ostream& os) const;
 friend std::ostream& operator <<(std::ostream& os,
   AbstractVertexContainer& vc);
#ifndef UNIT_TEST
 virtual ListExpr outContainer() const=0;
#endif

///////Persistence///////////////////////////////////////////////
 static TIN_SIZE getSizeOnDiscStatic(TIN_SIZE noVertices) {
  return sizeof(AbstractVertexContainer::maxSize)
    + sizeof(AbstractVertexContainer::noVertices);
 }
 virtual TIN_SIZE getContainerSizeOnDisc() const=0;
 virtual TIN_SIZE getSizeOnDisc() const;

#ifndef UNIT_TEST
 virtual void serialize(char* storage, size_t& offset) const = 0;
 virtual void rebuild(char* state, size_t & offset)=0;
 virtual bool open(SmiRecord& valueRecord)=0;
 virtual bool save(SmiRecord& valueRecord)=0;
#endif
/////////////////////////////////////////////////////////////////
};

}
#endif
