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

#ifndef VECTOR3D_H_
#define VECTOR3D_H_

#include "Vector2D.h"

namespace tin {

class Vector3D: public tin::Vector2D {
protected:
VECTOR_COMPONENT dz;
public:
Vector3D() {

}
;
Vector3D(const VECTOR_COMPONENT& idx, const VECTOR_COMPONENT& idy,
const VECTOR_COMPONENT& idz) {
this->dx = idx;
this->dy = idy;
this->dz = idz;
}
;
~Vector3D();

VECTOR_COMPONENT getDz() const {
return dz;
}
;

void setDz(VECTOR_COMPONENT& dz) {
this->dz = dz;
}
;
VECTOR_DIR dzdx();
VECTOR_DIR dzdy();
Vector3D operator*(const VECTOR_COMPONENT &f) const;
};

} /* namespace tin */
#endif /* VECTOR3D_H_ */
