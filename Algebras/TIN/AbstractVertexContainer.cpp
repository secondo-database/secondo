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
#include "AbstractVertexContainer.h"
#include "Vertex.h"

namespace tin {
TIN_SIZE AbstractVertexContainer::getSizeOnDisc() const {
return getContainerSizeOnDisc() + noVertices * Vertex::getSizeOnDisc();
}

TIN_SIZE AbstractVertexContainer::getMaxVertexCount() const {

return ((((maxSize - getContainerSizeOnDisc()) / Vertex::getSizeOnDisc())));
}

TIN_SIZE AbstractVertexContainer::countVerticesAddable() const {
return getMaxVertexCount() - noVertices;
}

bool AbstractVertexContainer::isEmpty() const {
return (noVertices == 0);
}

bool AbstractVertexContainer::isFull() const {
return (noVertices >= getMaxVertexCount());
}

void AbstractVertexContainer::print(std::ostream& os) const {

}

std::ostream& operator <<(std::ostream& os, AbstractVertexContainer& vc) {
vc.print(os);
return os;
}

}
