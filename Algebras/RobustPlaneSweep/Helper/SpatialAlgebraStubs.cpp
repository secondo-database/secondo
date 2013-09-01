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

[1] Stub File for the SpatialAlgebra

[TOC]

1 Overview

This file is only useful for the test project. It contains various  classes 
required to run the test project without Secondo. The classes and methods 
are extracted from the SpatialAlgebra. 

1 Includes

*/

#include <math.h>

#include "SpatialAlgebraStubs.h"

/*

1 ~AlmostEqual~

*/
bool AlmostEqual(double d1, double d2)
{
  double diff = fabs(d1 - d2);
  return (diff < 0.00000001);
}

/*

1 ~CompareDouble~

*/
int CompareDouble(const double a, const double b)
{
  if (AlmostEqual(a, b)) {
    return 0;
  }

  if (a < b) {
    return -1;
  }

  return 1;
}

/*

1 ~HalfSegmentCompare~

*/
int HalfSegmentCompare(const void *a, const void *b)
{
  const HalfSegment *hsa = (const HalfSegment *)a;
  const HalfSegment *hsb = (const HalfSegment *)b;

  return hsa->Compare(*hsb);
}

