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

[1] Implementation of the MRegionOps2Algebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[2] Implementation with exakt dataype, 

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Point3D.h"

namespace mregionops2 {


bool Point3D::LiesBetween(const Point3D& p1, const Point3D& p2) const
{
Point3D th = *this;
Point3D po1 = p1;
Point3D po2 = p2;

   cout << "LiesBetween --- this point: " << th << "   p1: " << 
    po1 << "   p2: " << po2 << endl;
  if ((*this == p1) || (*this == p2)) return true;
cout << " may be false" << endl;

  if (p1 == p2) return false;

  mpq_class ratio;
      
  if (p1.x != p2.x)
  {
    ratio = (x - p1.x) / (p2.x - p1.x);
  }
  else if (p1.y != p2.y)
  {
    ratio = (y - p1.y) / (p2.y - p1.y);
  }
  else
  {
    ratio = (z - p1.z) / (p2.z - p1.z);
  }
  
  if ((ratio < 0) || (ratio > 1)) return false;
  
  if ((x - p1.x) != (ratio * (p2.x - p1.x))) return false;
  if ((y - p1.y) != (ratio * (p2.y - p1.y))) return false;
  if ((z - p1.z) != (ratio * (p2.z - p1.z))) return false;
cout << " is true" << endl;
 
  return true;
}

}
