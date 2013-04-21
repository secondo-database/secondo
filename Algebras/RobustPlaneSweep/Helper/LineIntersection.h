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

#pragma once

#include <stdlib.h>

#include "InternalGeometries.h"
#include "Rational.h"

namespace RobustPlaneSweep
{
  // source:  http://geomalgorithms.com/a05-_intersect-1.html
  class LineIntersection
  {
  private:
    static bool IsPointOnLineHelper(
      long long p,
      long long s0,
      long long s1,
      bool includePseudoIntersections)
    {
      if (includePseudoIntersections) {
        if ((p >= s0 && p <= s1) ||
          (p >= s1 && p <= s0)) {
            return true;
        }
      } else {
        if ((p > s0 && p < s1) ||
          (p > s1 && p < s0)) {
            return true;
        }
      }

      return false;
    }

  public:
    static int GetIntersections(
      const InternalPoint& a0,
      const InternalPoint& a1,
      const InternalPoint& b0,
      const InternalPoint& b1,
      const bool includePseudoIntersections,
      InternalIntersectionPoint &i0,
      InternalIntersectionPoint &i1)
    {
      long long ux = (a1.GetX() - a0.GetX());
      long long uy = (a1.GetY() - a0.GetY());
      long long vx = (b1.GetX() - b0.GetX());
      long long vy = (b1.GetY() - b0.GetY());
      long long wx = (a0.GetX() - b0.GetX());
      long long wy = (a0.GetY() - b0.GetY());

      long long denominator = vx * uy - vy * ux;
      if (denominator == 0) {
        if ((vx * wy - vy * wx) != 0 ||
          (ux * wy - uy * wx) != 0) {
            return 0;
        }

        bool aIsPoint = (ux == 0 && uy == 0);
        bool bIsPoint = (vx == 0 && vy == 0);

        if (aIsPoint && bIsPoint) {
          if (wx == 0 && wy == 0) {
            i0 = InternalIntersectionPoint(a0);
            return 1;
          } else {
            return 0;
          }
        } else if (aIsPoint) {
          if (llabs(vx) > llabs(vy)) {
            if (!IsPointOnLineHelper(
              a0.GetX(), b0.GetX(), b1.GetX(), includePseudoIntersections)) {
                return 0;
            }
          } else {
            if (!IsPointOnLineHelper(
              a0.GetY(), b0.GetY(), b1.GetY(), includePseudoIntersections)) {
                return 0;
            }
          }

          i0 = InternalIntersectionPoint(a0);
          return 1;
        } else if (bIsPoint) {
          if (llabs(ux) > llabs(uy)) {
            if (!IsPointOnLineHelper(
              b0.GetX(), a0.GetX(), a1.GetX(), includePseudoIntersections)) {
                return 0;
            }
          } else {
            if (!IsPointOnLineHelper(
              b0.GetY(), a0.GetY(), a1.GetY(), includePseudoIntersections)) {
                return 0;
            }
          }

          i0 = InternalIntersectionPoint(b0);
          return 1;
        } else {
          SimpleRational t0(0, 1), t1(0, 1);
          if (llabs(vx) > llabs(vy)) {
            t0 =  SimpleRational(wx, vx);
            t1 =  SimpleRational(a1.GetX()- b0.GetX(), vx);
          } else {
            t0 =  SimpleRational(wy, vy);
            t1 =  SimpleRational(a1.GetY() - b0.GetY(), vy);
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
            if (!includePseudoIntersections) {
              if (
                a0.GetX() == b0.GetX() && a0.GetY() == b0.GetY() &&
                a1.GetX() == b1.GetX() && a1.GetY() == b1.GetY()) {
                  return 0;
              }
              if (
                a0.GetX() == b1.GetX() && a0.GetY() == b1.GetY() &&
                a1.GetX() == b0.GetX() && a1.GetY() == b0.GetY()) {
                  return 0;
              }
            }
            i0 = InternalIntersectionPoint(b0);
            i1 = InternalIntersectionPoint(b1);
            return 2;
          } else if (t0 == t1) {
            Rational ratt0 = (Rational)t0;
            i0 = InternalIntersectionPoint(
              (ratt0 * (int)vx + b0.GetX()),
              (ratt0 * (int)vy + b0.GetY()));
            return 1;
          } else {
            Rational ratt0 = (Rational)t0;
            Rational ratt1 = (Rational)t1;

            i0 = InternalIntersectionPoint(
              (ratt0 * (int)vx + b0.GetX()),
              (ratt0 * (int)vy + b0.GetY()));

            i1 = InternalIntersectionPoint(
              (ratt1 * (int)vx + b0.GetX()),
              (ratt1 * (int)vy + b0.GetY()));

            return 2;
          }
        }
      } else {
        SimpleRational si((vy * wx - vx * wy), denominator);

        if (si < 0 || si > 1) {
          return 0;
        }

        SimpleRational ti((ux * wy - uy * wx), -denominator);
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
        Rational ix = (ratSi * (int)ux + a0.GetX());
        Rational iy = (ratSi * (int)uy + a0.GetY());

        i0 = InternalIntersectionPoint(ix, iy);
        return 1;
      }
    }
  };
}
