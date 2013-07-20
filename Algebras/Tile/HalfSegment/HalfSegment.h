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

#include "../../Spatial/HalfSegment.h"
#include "SpatialAlgebra.h"

namespace TileAlgebra
{

/*
declaration of CheckPoints function

*/

bool CheckPoints(Point& rLeftPoint,
                 Point& rRightPoint);

/*
declaration of CheckHalfSegment function

*/

void CheckHalfSegment(HalfSegment& rHalfSegment);

/*
declaration of IsHorizontalHalfSegment function

*/

bool IsHorizontalHalfSegment(const HalfSegment& rHalfSegment);

/*
declaration of IsVerticalHalfSegment function

*/

bool IsVerticalHalfSegment(const HalfSegment& rHalfSegment);

/*
declaration of HalfSegmentIntersectsRectangle function

*/

bool HalfSegmentIntersectsRectangle(const HalfSegment& rHalfSegment,
                                    const Rectangle<2>& rRectangle);

/*
declaration of GetPointsInRectangle function

*/

bool GetPointsInRectangle(const HalfSegment& rHalfSegment,
                          const Rectangle<2>& rRectangle,
                          Point& rLeftPoint,
                          Point& rRightPoint);

}

#endif // TILEALGEBRA_HALFSEGMENT_H
