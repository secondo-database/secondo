/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

#include <stdexcept>
#include "InternalGeometries.h"
#include "LineIntersection.h"
#include "Utility.h"

namespace RobustPlaneSweep
{
void InternalLineSegment::AddHobbyIntersection(int x,
                                               int y,
                                               int hobbySpacing)
{
  InternalIntersectionPoint i0, i1;
  int c = LineIntersection::GetIntersections(_left,
                                             _right,
                                             InternalPoint(x - hobbySpacing,
                                                           y - hobbySpacing),
                                             InternalPoint(x + hobbySpacing,
                                                           y + hobbySpacing),
                                             true,
                                             i0,
                                             i1);

  if (c == 1 && i0.GetX() < x + hobbySpacing && i0.GetY() < y + hobbySpacing) {
    AddIntersection(i0);
    return;
  }

  c = LineIntersection::GetIntersections(_left,
                                         _right,
                                         InternalPoint(x + hobbySpacing,
                                                       y - hobbySpacing),
                                         InternalPoint(x - hobbySpacing,
                                                       y + hobbySpacing),
                                         true,
                                         i0,
                                         i1);

  if (c == 1 && i0.GetX() < x + hobbySpacing && i0.GetY() < y + hobbySpacing) {
    AddIntersection(i0);
    return;
  }

  throw new std::logic_error("there should be exactly one intersection!");

}

InternalPointTransformation::
InternalPointTransformation(const long long offsetX,
                            const long long offsetY,
                            const int scaleFactor,
                            const int roundResultToDecimals,
                            const int roundingDecimalSteps)
{
  _offsetX = offsetX;
  _offsetY = offsetY;
  _scaleFactor = scaleFactor;
  _roundResultToDecimals = roundResultToDecimals;

  _roundingStep = scaleFactor;

  for (int i = 0; i < roundResultToDecimals; ++i) {
    if (_roundingStep % 10 != 0) {
      throw new std::invalid_argument("roundResultToDecimals");
    }

    _roundingStep /= 10;
  }
  _roundingStep *= roundingDecimalSteps;

  _almostEqualSortMargin = 5 * (int)ceil(_scaleFactor * 0.00000001);
}

// assumes that s0 is above s1
// (respectively s0 is right of s1 if s0/s1 are vertical)
InternalResultLineSegment
InternalResultLineSegment::MergeSegments(const InternalResultLineSegment& s0,
                                         const InternalResultLineSegment& s1)
{
  BoundaryType newFirst =
      InternalAttribute::Merge(s0.GetInternalAttribute().GetFirst(),
                               s1.GetInternalAttribute().GetFirst());

  BoundaryType newSecond =
      InternalAttribute::Merge(s0.GetInternalAttribute().GetSecond(),
                               s1.GetInternalAttribute().GetSecond());

  InternalAttribute intAttr = InternalAttribute(newFirst, newSecond);

  return InternalResultLineSegment(s1.GetOriginalStart(),
                                   s1.GetStart(),
                                   s1.GetOriginalEnd(),
                                   s1.GetEnd(),
                                   intAttr,
                                   false);
}
}
