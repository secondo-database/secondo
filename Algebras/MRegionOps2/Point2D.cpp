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

[2] Implementation with exakt dataype

April - November 2014, S. Schroer for master thesis.

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
// *this = referenziertes Objekt das zu überprüfen ist
  if ((*this == p1) || (*this == p2)) return true;

// kann nicht dazwischen liegen
  if (p1 == p2) return false;
    
  if (p1.x != p2.x)
  {
    // x-Koordinate vom Objekt
    // was passiert wenn p2 < p1
    mpq_class ratio = (x - p1.x) / (p2.x - p1.x);

    // kürzt gemeinsame Teiler, canonische Form: ratio.canonicalize();  
    // < 0 liegt der Punkt links von p1, > 0 liegt der Punkt rechts von p1
    // wenn x zwischen p1 und p2 liegen soll, muss das 
    // Verhältnis der y-Werte dem der x-Werte entsprechen
    if ((ratio >= 0) && (ratio <= 1) &&
        ((y - p1.y) == (ratio * (p2.y - p1.y)))) return true;
  }

  // p1, p2 liegen auf einer parallelen zur x-Achse
  // wir wissen, y1 <> y2 und prüfen das Verhältnis neu
  else
  {
    mpq_class ratio = (y - p1.y) / (p2.y - p1.y);
    if ((ratio >= 0) && (ratio <= 1) &&
        ((x - p1.x) == (ratio * (p2.x - p1.x)))) return true;
  }
  
return false;
}

} // end of namespace mregionops2
