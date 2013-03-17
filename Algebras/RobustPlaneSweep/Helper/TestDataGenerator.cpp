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
  std::vector<HalfSegment>* TestDataGenerator::GenerateRandomWalk(
    unsigned int seed,
    double offsetX,
    double offsetY,
    double extentX,
    double extentY,
    unsigned count,
    int roundToDecimals) 
  {
    std::vector<HalfSegment>* segments=new std::vector<HalfSegment>();

    srand(seed);

    Point lastPoint = Point(true,offsetX, offsetY);

    int index=0;
    while ((unsigned int)index  < count) {
      double rx=(rand()/(double)RAND_MAX)-0.5;
      double ry=(rand()/(double)RAND_MAX)-0.5;

      Point point = Point(true,
        Utility::Round(lastPoint.GetX() + extentX * rx, roundToDecimals), 
        Utility::Round(lastPoint.GetY() + extentY * ry, roundToDecimals));
      if (point.GetX() != lastPoint.GetX() || 
          point.GetY() != lastPoint.GetY()) {

        HalfSegment h1(true,lastPoint,point); h1.attr.edgeno=index;
        HalfSegment h2(false,lastPoint,point); h2.attr.edgeno=index;

        segments->push_back (h1);
        segments->push_back (h2);
        lastPoint = point;
        ++index;
      }
    }

    sort(segments->begin(),segments->end(),HalfSegment::Less); 

    return segments;
  }
}
