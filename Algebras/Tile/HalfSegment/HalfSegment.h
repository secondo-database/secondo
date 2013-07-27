/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#ifndef TILEALGEBRA_HALFSEGMENT_H
#define TILEALGEBRA_HALFSEGMENT_H

/*
SECONDO includes

*/

#include "../../Spatial/HalfSegment.h"
#include "SpatialAlgebra.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method CheckPoints checks the consistency of given left point and right point.

author: Dirk Zacher
parameters: rLeftPoint - reference to left point
            rRightPoint - reference to right point
return value: true, if left point and right point were exchanged,
              otherwise false
exceptions: -

*/

bool CheckPoints(Point& rLeftPoint,
                 Point& rRightPoint);

/*
Method CheckHalfSegment checks the consistency of the given HalfSegment.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: -
exceptions: -

*/

void CheckHalfSegment(HalfSegment& rHalfSegment);

/*
Method IsHorizontalHalfSegment checks if given HalfSegment is a horizontal line.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: true, if rHalfSegment is a horizontal line, otherwise false
exceptions: -

*/

bool IsHorizontalHalfSegment(const HalfSegment& rHalfSegment);

/*
Method IsVerticalHalfSegment checks if given HalfSegment is a vertical line.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
return value: true, if rHalfSegment is a vertical line, otherwise false
exceptions: -

*/

bool IsVerticalHalfSegment(const HalfSegment& rHalfSegment);

/*
Method HalfSegmentIntersectsRectangle checks if given HalfSegment
intersects given Rectangle<2>.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
return value: true, if rHalfSegment intersects rRectangle, otherwise false
exceptions: -

*/

bool HalfSegmentIntersectsRectangle(const HalfSegment& rHalfSegment,
                                    const Rectangle<2>& rRectangle);

/*
Method GetPointsInRectangle returns a left point and a right point
of given HalfSegment inside the given Rectangle<2>.

author: Dirk Zacher
parameters: rHalfSegment - reference to a HalfSegment
            rRectangle - reference to a Rectangle<2>
            rLeftPoint - reference to the left point inside rRectangle
            rRightPoint - reference to the right point inside rRectangle
return value: true, if left point and right point were successfully calculated,
              otherwise false
exceptions: -

*/

bool GetPointsInRectangle(const HalfSegment& rHalfSegment,
                          const Rectangle<2>& rRectangle,
                          Point& rLeftPoint,
                          Point& rRightPoint);

}

#endif // TILEALGEBRA_HALFSEGMENT_H
