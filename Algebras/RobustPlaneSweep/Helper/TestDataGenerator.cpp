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

#include <stdlib.h>
#include <algorithm>

#include "TestDataGenerator.h"
#include "Utility.h"

namespace RobustPlaneSweep
{
std::vector<HalfSegment>*
TestDataGenerator::GenerateRandomWalk(unsigned int seed,
                                      double offsetX,
                                      double offsetY,
                                      double extentX,
                                      double extentY,
                                      unsigned count,
                                      int roundToDecimals)
{
  std::vector<HalfSegment>* segments = new std::vector<HalfSegment>();

  srand(seed);

  Point lastPoint = Point(true, offsetX, offsetY);

  int index = 0;
  while ((unsigned int)index < count) {
    double rx = (rand() / (double)RAND_MAX) - 0.5;
    double ry = (rand() / (double)RAND_MAX) - 0.5;

    Point point = Point(true,
                        Utility::Round(lastPoint.GetX() + extentX * rx,
                                       roundToDecimals),
                        Utility::Round(lastPoint.GetY() + extentY * ry,
                                       roundToDecimals));

    if (point.GetX() != lastPoint.GetX() || point.GetY() != lastPoint.GetY()) {
      HalfSegment h1(true, lastPoint, point);
      h1.attr.edgeno = index;
      HalfSegment h2(false, lastPoint, point);
      h2.attr.edgeno = index;

      segments->push_back(h1);
      segments->push_back(h2);
      lastPoint = point;
      ++index;
    }
  }

  // to be perfectly compatible with DbArray::Sort
  qsort(segments->data(),
        segments->size(),
        sizeof(HalfSegment),
        HalfSegmentCompare);
  // sort(segments->begin(), segments->end(), HalfSegment::Less);

  return segments;
}

std::vector<std::vector<HalfSegment>*>*
TestDataGenerator::GenerateTriangles(unsigned int seed,
                                     double offsetX,
                                     double offsetY,
                                     double extentX,
                                     double extentY,
                                     unsigned count,
                                     int roundToDecimals)
{
  std::vector<std::vector<HalfSegment>*>* result =
      new std::vector<std::vector<HalfSegment>*>();

  srand(seed);

  int index = 0;

  for (unsigned int geometry = 0; geometry < count; geometry++) {
    Point points[3];

    for (int i = 0;;) {
      double rx = (rand() / (double)RAND_MAX);
      double ry = (rand() / (double)RAND_MAX);

      Point p = Point(true,
                      Utility::Round(offsetX + extentX * rx, roundToDecimals),
                      Utility::Round(offsetY + extentY * ry, roundToDecimals));

      if (i == 0) {
        points[0] = p;
        i = 1;
      } else {
        if (points[0].GetX() == p.GetX() && points[0].GetY() == p.GetY()) {
          continue;
        }

        if (i == 1) {
          points[1] = p;
          i = 2;
        } else {
          if (points[1].GetX() == p.GetX() && points[1].GetY() == p.GetY()) {
            continue;
          }

          double area2 = ((points[1].GetX() - points[0].GetX())
                          * (p.GetY() - points[0].GetY()))
                         - ((p.GetX() - points[0].GetX())
                            * (points[1].GetY() - points[0].GetY()));

          if (fabs(area2) < 0.0001) {
            continue;
          }

          points[2] = p;
          break;
        }
      }
    }

    double centerX =
        (points[0].GetX() + points[1].GetX() + points[2].GetX()) / 3;

    double centerY =
        (points[0].GetY() + points[1].GetY() + points[2].GetY()) / 3;

    std::vector<HalfSegment>* triangle = new std::vector<HalfSegment>();

    for (int j = 0; j < 3; j++) {
      Point s = points[j];
      Point e = points[(j + 1) % 3];

      if (s.GetX() > e.GetX()
          || (e.GetX() == s.GetX() && s.GetY() > e.GetY())) {
        Point temp = s;
        s = e;
        e = temp;
      }

      bool isLeft =
          ((e.GetX() - s.GetX()) * (centerY - s.GetY()) -
           (e.GetY() - s.GetY()) * (centerX - s.GetX()))
          > 0;

      HalfSegment h1(true, s, e);
      h1.attr.edgeno = index;
      h1.attr.insideAbove = isLeft;

      HalfSegment h2(false, s, e);
      h2.attr.edgeno = index;
      h2.attr.insideAbove = isLeft;

      triangle->push_back(h1);
      triangle->push_back(h2);

      index++;
    }

    result->push_back(triangle);
  }

  return result;
}

}
