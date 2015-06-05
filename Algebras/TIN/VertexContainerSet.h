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

#ifndef VERTEXCONTAINER_H_
#define VERTEXCONTAINER_H_
#include <set>
#include "TinHelper.h"
#include "Vertex.h"
#include "AbstractVertexContainer.h"
using std::set;

namespace tin {

class VertexContainerSet: public AbstractVertexContainer {
protected:
set<Vertex> setVertices;
mutable Vertex** tmpYorderedindex;
mutable bool indexYUpToDate;
mutable Vertex** tmpXorderedindex;
mutable bool indexXUpToDate;
mutable TinFeatures feat;
mutable bool initFeat;

public:
VertexContainerSet(TIN_SIZE imaxsize = VERTEX_CONTAINER_STANDARD_MAX_SIZE);
~VertexContainerSet();

VertexContainerSet* clone();
AbstractVertexContainer* clone_empty();
#ifndef UNIT_TEST
VertexContainerSet(SmiRecord* r, SmiSize off);
ListExpr outContainer() const;
void serialize(char* storage, size_t& offset) const;
void rebuild(char* state, size_t & offset);
protected:
bool open(SmiRecord& valueRecord);
public:
bool save(SmiRecord& valueRecord);
#endif
std::set<Vertex>::reverse_iterator getIteratorRbegin() const;
std::set<Vertex>::reverse_iterator getIteratorRend() const;
void removeVertex(std::set<Vertex>::reverse_iterator it);
void resize(TIN_SIZE imaxSize);
void updateXindexAndFeatures() const;
TinFeatures getFeatures() const;
const Vertex* getVertex(const Vertex& v) const;
const Vertex* getVertexByYIndex(const FILE_VERTEX_POINTER i) const;
const Vertex* getVertexByXIndex(const int i) const;
void loadVertices(const Vertex* parray, int numberofvertices);
const Vertex* insertVertex(const Vertex* v, bool & newVertex);
void insertVertex_p(const Vertex* v);
void insertVertices(const VertexContainerSet& vc);
void removeVertex(const Vertex * v);
void clear();
int getYIndex(const Vertex * v) const;
#ifndef UNIT_TEST
void unaryOp(void * function, TinFeatures& feat);
#endif
void unaryOp(VERTEX_Z (*op)(VERTEX_Z z), TinFeatures& feat);
void binaryOp(Tin& tt, VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2),
TinFeatures& feat);
static TIN_SIZE getSizeOnDiscStatic(const TIN_SIZE inoVertices) {
return inoVertices * Vertex::getSizeOnDisc()
+ AbstractVertexContainer::getSizeOnDiscStatic(inoVertices);
}
static TIN_SIZE estimateSizeInMemoryStatic(const TIN_SIZE inoVertices) {
//rough estimate
return inoVertices * (sizeof(Vertex) + 24) + sizeof(VertexContainerSet);
}
TIN_SIZE getContainerSizeOnDisc() const;
int getSizeInMemory() const;

void addVerticesTo(AbstractVertexContainer& vc);

void print(std::ostream & os = std::cout) const;
protected:

friend std::ostream& operator <<(std::ostream& os, VertexContainerSet& vc);

friend class VertexContainer_test;

};

}
#endif
