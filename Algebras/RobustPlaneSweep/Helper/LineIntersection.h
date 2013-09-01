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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File for the class ~LineIntersection~

[TOC]

1 Overview

This header file contains the class ~LineIntersection~.

This class detects intersections between two line segments.

1 Defines and includes

*/

#pragma once

#include <stdlib.h>

#include "InternalGeometries.h"
#include "Rational.h"

namespace RobustPlaneSweep
{
/*

1 Class ~LineIntersection~

*/
class LineIntersection
{
private:
/*

1.1 ~IsPointOnLineHelper~

*/
  static bool IsPointOnLineHelper(long long p,
                                  long long s0,
                                  long long s1,
                                  bool includePseudoIntersections)
  {
    if (includePseudoIntersections) {
      if ((p >= s0 && p <= s1) || (p >= s1 && p <= s0)) {
        return true;
      }
    } else {
      if ((p > s0 && p < s1) || (p > s1 && p < s0)) {
        return true;
      }
    }

    return false;
  }

public:
/*

1.1 ~GetIntersections~

Determines if the segments with the end points a0-a1 and b0-b1 have any 
intersections. Returns 0 if there is none, 
1 if there is one (not necessarily proper) intersection and
2 if there is the segments overlap. 
if ~includePseudoIntersections~ is false, then only intersection points are
returned that split at least one segment into two pieces.
if ~includeCompleteOverlaps~ is false, then overlapping segments where 
both segments have equal end points are returned as zero intersection points.

*/
  static int GetIntersections(const InternalPoint& a0,
                              const InternalPoint& a1,
                              const InternalPoint& b0,
                              const InternalPoint& b1,
                              const bool includePseudoIntersections,
                              const bool includeCompleteOverlaps,
                              InternalIntersectionPoint &i0,
                              InternalIntersectionPoint &i1)
  {
    long long ax = (a1.GetX() - a0.GetX());
    long long ay = (a1.GetY() - a0.GetY());
    long long bx = (b1.GetX() - b0.GetX());
    long long by = (b1.GetY() - b0.GetY());
    long long abx = (a0.GetX() - b0.GetX());
    long long aby = (a0.GetY() - b0.GetY());

    long long denominator = bx * ay - by * ax;
    if (denominator == 0) {
      if ((bx * aby - by * abx) != 0 || (ax * aby - ay * abx) != 0) {
        return 0;
      }

      bool aIsPoint = (ax == 0 && ay == 0);
      bool bIsPoint = (bx == 0 && by == 0);

      if (aIsPoint && bIsPoint) {
        if (abx == 0 && aby == 0) {
          i0 = InternalIntersectionPoint(a0);
          return 1;
        } else {
          return 0;
        }
      } else if (aIsPoint) {
        if (llabs(bx) > llabs(by)) {
          if (!IsPointOnLineHelper(a0.GetX(),
                                   b0.GetX(),
                                   b1.GetX(),
                                   includePseudoIntersections)) {
            return 0;
          }
        } else {
          if (!IsPointOnLineHelper(a0.GetY(),
                                   b0.GetY(),
                                   b1.GetY(),
                                   includePseudoIntersections)) {
            return 0;
          }
        }

        i0 = InternalIntersectionPoint(a0);
        return 1;
      } else if (bIsPoint) {
        if (llabs(ax) > llabs(ay)) {
          if (!IsPointOnLineHelper(b0.GetX(),
                                   a0.GetX(),
                                   a1.GetX(),
                                   includePseudoIntersections)) {
            return 0;
          }
        } else {
          if (!IsPointOnLineHelper(b0.GetY(),
                                   a0.GetY(),
                                   a1.GetY(),
                                   includePseudoIntersections)) {
            return 0;
          }
        }

        i0 = InternalIntersectionPoint(b0);
        return 1;
      } else {
        SimpleRational t0(0, 1), t1(0, 1);
        if (llabs(bx) > llabs(by)) {
          t0 = SimpleRational(abx, bx);
          t1 = SimpleRational(a1.GetX() - b0.GetX(), bx);
        } else {
          t0 = SimpleRational(aby, by);
          t1 = SimpleRational(a1.GetY() - b0.GetY(), by);
        }

        if (t0 > t1) {
          SimpleRational temp = t0;
          t0 = t1;
          t1 = temp;
        }

        if (t1 < 0 || t0 > 1) {
          return 0;
        }

        if (t0 < 0) {
          t0 = SimpleRational(0, 1);
        }

        if (t1 > 1) {
          t1 = SimpleRational(1, 1);
        }

        if (t1 == 0) {
          // touches only at start point
          if (!includePseudoIntersections) {
            return 0;
          }

          i0 = InternalIntersectionPoint(b0);
          return 1;
        } else if (t0 == 1) {
          // touches only at end point
          if (!includePseudoIntersections) {
            return 0;
          }

          i0 = InternalIntersectionPoint(b1);
          return 1;
        } else if (t0 == 0 && t1 == 1) {
          if (!includeCompleteOverlaps) {
            if (a0.GetX() == b0.GetX() && a0.GetY() == b0.GetY()
                && a1.GetX() == b1.GetX()
                && a1.GetY() == b1.GetY()) {
              return 0;
            }
            if (a0.GetX() == b1.GetX() && a0.GetY() == b1.GetY()
                && a1.GetX() == b0.GetX()
                && a1.GetY() == b0.GetY()) {
              return 0;
            }
          }
          i0 = InternalIntersectionPoint(b0);
          i1 = InternalIntersectionPoint(b1);
          return 2;
        } else if (t0 == t1) {
          Rational ratt0 = (Rational)t0;
          i0 = InternalIntersectionPoint((ratt0 * (int)bx + b0.GetX()),
                                         (ratt0 * (int)by + b0.GetY()));
          return 1;
        } else {
          Rational ratt0 = (Rational)t0;
          Rational ratt1 = (Rational)t1;

          i0 = InternalIntersectionPoint((ratt0 * (int)bx + b0.GetX()),
                                         (ratt0 * (int)by + b0.GetY()));

          i1 = InternalIntersectionPoint((ratt1 * (int)bx + b0.GetX()),
                                         (ratt1 * (int)by + b0.GetY()));

          return 2;
        }
      }
    } else {
      SimpleRational si((by * abx - bx * aby), denominator);

      if (si < 0 || si > 1) {
        return 0;
      }

      SimpleRational ti((ax * aby - ay * abx), -denominator);
      if (ti < 0 || ti > 1) {
        return 0;
      }

      if (si == 0) {
        if (!includePseudoIntersections && (ti == 0 || ti == 1)) {
          return 0;
        }

        i0 = InternalIntersectionPoint(a0);
        return 1;
      }

      if (si == 1) {
        if (!includePseudoIntersections && (ti == 0 || ti == 1)) {
          return 0;
        }

        i0 = InternalIntersectionPoint(a1);
        return 1;
      }

      if (ti == 0) {
        if (!includePseudoIntersections && (si == 0 || si == 1)) {
          return 0;
        }

        i0 = InternalIntersectionPoint(b0);
        return 1;
      }

      if (ti == 1) {
        if (!includePseudoIntersections && (si == 0 || si == 1)) {
          return 0;
        }

        i0 = InternalIntersectionPoint(b1);
        return 1;
      }

      Rational ratSi = (Rational)si;
      Rational ix = (ratSi * (int)ax + a0.GetX());
      Rational iy = (ratSi * (int)ay + a0.GetY());

      i0 = InternalIntersectionPoint(ix, iy);
      return 1;
    }
  }
};
}
