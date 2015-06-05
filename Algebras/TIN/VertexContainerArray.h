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

#ifndef VERTEXCONTAINERSMALL_H_
#define VERTEXCONTAINERSMALL_H_

#include "TinHelper.h"
#include "Vertex.h"
#include "AbstractVertexContainer.h"
namespace tin {
class VertexContainerSet;

class VertexContainerArray: public AbstractVertexContainer {
protected:
Vertex *arVertices;
public:
////////Construction/////////////////////////////////////////////
VertexContainerArray(
TIN_SIZE imaxsize = VERTEX_CONTAINER_STANDARD_MAX_SIZE);
virtual ~VertexContainerArray();

VertexContainerArray* clone();
VertexContainerArray* clone_empty();

////////Manipulation/////////////////////////////////////////////
void loadVertices(const Vertex* parray, const int numberofvertices);
Vertex* insertVertex(const Vertex* v, bool & newVertex);
void insertVertex_p(const Vertex* v);
void removeVertex(const Vertex * v);
void clear();
void resize(TIN_SIZE imaxSize);
void addVerticesTo(AbstractVertexContainer& vc);
void insertVertices(const VertexContainerSet& vc);

void unaryOp(VERTEX_Z (*op)(VERTEX_Z z), TinFeatures& feat);
#ifndef UNIT_TEST
void unaryOp(void* function, TinFeatures& feat);
#endif
void binaryOp(Tin& tt, VERTEX_Z (*op)(VERTEX_Z z1, VERTEX_Z z2),
TinFeatures& feat);

///////Query/////////////////////////////////////////////////////

int getSizeInMemory() const;
Vertex* getVertex(const Vertex& v) const;
Vertex* getVertexByYIndex(const FILE_VERTEX_POINTER i) const;
int getYIndex(const Vertex * v) const;

///////Presentation//////////////////////////////////////////////
#ifndef UNIT_TEST
ListExpr outContainer() const;
#endif
void print(std::ostream& os) const;
friend std::ostream& operator <<(std::ostream& os,
VertexContainerArray& vc);

//////Persistence////////////////////////////////////////////////
TIN_SIZE getContainerSizeOnDisc() const;
static TIN_SIZE getSizeOnDiscStatic(TIN_SIZE inoVertices) {
return inoVertices * Vertex::getSizeOnDisc()
+ AbstractVertexContainer::getSizeOnDiscStatic(inoVertices);
}
#ifndef UNIT_TEST
void rebuild(char* state, size_t & offset);
void serialize(char* storage, size_t& offset) const;
protected:
bool open(SmiRecord& valueRecord);
public:
bool save(SmiRecord& valueRecord);
#endif

/////////////////////////////////////////////////////////////////

friend class VertexContainerSmall_test;
};

} /* namespace tin */
#endif /* VERTEXCONTAINERSMALL_H_ */
