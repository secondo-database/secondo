/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation 

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Point2D.h"
#include "Point3D.h"

namespace mregionops2 {

/*

3 Class Point2D

*/

Point2D::Point2D(const Point3D& p) :
         x(p.GetX()), y(p.GetY()) {

}

bool Point2D::LiesBetween(Point2D p1, Point2D p2)

{
// *this = referenziertes object to compare
  if ((*this == p1) || (*this == p2)) return true;

// not beetween
  if (p1 == p2) return false;
    
  if (p1.x != p2.x)
  {
    // x-Koordinate from object
    // what's happend if p2 < p1
    mpq_class ratio = (x - p1.x) / (p2.x - p1.x);

    // shortens common divisor, canonische Form: ratio.canonicalize();  
    // < 0 if the point is to the left of p1 
	// > 0 if the point is to the right of p1
	// when x is between p1 and p2 should be 
	// ratio of the y - values correspond to the x- values
	
    if ((ratio >= 0) && (ratio <= 1) &&
        ((y - p1.y) == (ratio * (p2.y - p1.y)))) return true;
  }

  // p1, p2 lie on a line parallel to the x- axis
  // we know, y1 <> y2 and examine the relationship new
  
  else
  {
    mpq_class ratio = (y - p1.y) / (p2.y - p1.y);
    if ((ratio >= 0) && (ratio <= 1) &&
        ((x - p1.x) == (ratio * (p2.x - p1.x)))) return true;
  }
  
return false;
}

} // end of namespace mregionops2
