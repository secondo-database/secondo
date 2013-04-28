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
  void InternalLineSegment::AddHobbyIntersection(
    int x,
    int y,
    int hobbySpacing)
  {
    InternalIntersectionPoint i0, i1;
    if (LineIntersection::GetIntersections(
      _left,
      _right,
      InternalPoint(x - hobbySpacing, y - hobbySpacing),
      InternalPoint(x + hobbySpacing, y + hobbySpacing),
      true, i0, i1) == 1 &&
      i0.GetX() < x + hobbySpacing && i0.GetY() < y + hobbySpacing) {
    } else if (LineIntersection::GetIntersections(
      _left,
      _right,
      InternalPoint(x + hobbySpacing, y - hobbySpacing),
      InternalPoint(x - hobbySpacing, y + hobbySpacing),
      true, i0, i1) == 1  &&
      i0.GetX() < x + hobbySpacing && i0.GetY() < y + hobbySpacing) {
    } else {
      throw new std::logic_error("there should be exactly one intersection!");
    }

    AddIntersection(i0);
  }

  InternalPointTransformation::InternalPointTransformation(
    const long long offsetX,
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
  }

  // assumes that s0 is above s1
  // (respectively s0 is right of s1 if s0/s1 are vertical)
  InternalResultLineSegment InternalResultLineSegment::MergeSegments(
    const InternalResultLineSegment& s0,
    const InternalResultLineSegment& s1)
  {
    bool fa, fb, sa, sb;

    {
      bool a = s0.GetInternalAttribute().IsFirstAbove();
      bool b = s0.GetInternalAttribute().IsFirstBelow();
      bool c = s1.GetInternalAttribute().IsFirstAbove();
      bool d = s1.GetInternalAttribute().IsFirstBelow();
      InternalAttribute::Merge(a, b, c, d, fa, fb);
    }

    {
      bool a = s0.GetInternalAttribute().IsSecondAbove();
      bool b = s0.GetInternalAttribute().IsSecondBelow();
      bool c = s1.GetInternalAttribute().IsSecondAbove();
      bool d = s1.GetInternalAttribute().IsSecondBelow();
      InternalAttribute::Merge(a, b, c, d, sa, sb);
    }

    bool firstBorder =
      s0.GetInternalAttribute().IsFirstBorder() ||
      s1.GetInternalAttribute().IsFirstBorder();

    bool secondBorder =
      s0.GetInternalAttribute().IsSecondBorder() ||
      s1.GetInternalAttribute().IsSecondBorder();

    InternalAttribute intAttr = InternalAttribute(
      firstBorder, fa, fb,
      secondBorder, sa, sb);

    return InternalResultLineSegment(
      s0.GetAttr(),
      s1.GetOriginalStart(),
      s1.GetStart(),
      s1.GetOriginalEnd(),
      s1.GetEnd(),
      intAttr,
      false);
  }
}
