
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

*/

#ifndef BERLIN2WGS_H
#define BERLIN2WGS_H

#include "Point.h"
#include "HalfSegment.h"
#include "SpatialAlgebra.h"

/*
This file is applied to convert BBBike coordinates into WGS84 coordinates.

The formula is taken from the BBBike sources.

http://bbbike.sourceforge.net/index.de.html

*/
class Berlin2WGS {
 public:
  Berlin2WGS();
  void convert(const Point* source, Point* result);
  void convert(const Points* source, Points* result);
  void convert(const Line* source, Line* result);
  void convert(const Region* source, Region* result);
  
 protected:
  pair<double, double> b2wgs(const double& x, const double& y);
  HalfSegment b2wgs(const HalfSegment& source);
  
  double x0, x1, x2, y0, y1, y2;
};

#endif