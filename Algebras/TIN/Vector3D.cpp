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

#include "Vector3D.h"

namespace tin {

Vector3D::~Vector3D() {
// TODO Auto-generated destructor stub
}

VECTOR_DIR Vector3D::dzdx() {
//TODO no error calculated
 if (dx == 0)
  return 0;
 return dz / dx;
}

VECTOR_DIR Vector3D::dzdy() {
 return 0;
}
/* namespace tin*/

Vector3D Vector3D::operator*(const VECTOR_COMPONENT& f) const {
 VECTOR_COMPONENT resultx = dx * f;
 VECTOR_COMPONENT resulty = dy * f;
 VECTOR_COMPONENT resultz = dz * f;

 return Vector3D(resultx, resulty, resultz);
}
}
