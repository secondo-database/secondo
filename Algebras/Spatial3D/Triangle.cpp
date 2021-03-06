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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2014 / 2015

<our names here>

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of a Spatial3D algebra

[TOC]

1 Includes and Defines

*/

#include "AuxiliaryTypes.h"
#include "geometric_algorithm.h"

using namespace std;

Triangle::Triangle(const SimplePoint3d& a,
                   const SimplePoint3d& b,
                   const SimplePoint3d& c)
    : pA(a), pB(b), pC(c) { }

Triangle::Triangle(const Triangle& src)
    : pA(src.pA), pB(src.pB), pC(src.pC) { }

Vector3d Triangle::getNormalVector() const
{
  return spatial3d_geometric::normalVector(getA(), getB(), getC());
}

Plane3d Triangle::getPlane() const
{
  return Plane3d(pA, pB, pC);
}

SimplePoint3d Triangle::getA() const
{
  return pA;
}

SimplePoint3d Triangle::getB() const
{
  return pB;
}

SimplePoint3d Triangle::getC() const
{
  return pC;
}

Rectangle<3> Triangle::BoundingBox() const
{
  double xl = min(pA.getX(), min(pB.getX(), pC.getX()));
  double xh = max(pA.getX(), max(pB.getX(), pC.getX()));
  double yl = min(pA.getY(), min(pB.getY(), pC.getY()));
  double yh = max(pA.getY(), max(pB.getY(), pC.getY()));
  double zl = min(pA.getZ(), min(pB.getZ(), pC.getZ()));
  double zh = max(pA.getZ(), max(pB.getZ(), pC.getZ()));
  double minMax[] = {xl,xh,yl,yh,zl,zh};
  return Rectangle<3>(true, minMax);
}

ostream& operator<< (ostream& os, const Triangle& triangle) {
  os.precision(17);
  return os << "<" << triangle.getA()
  << " " << triangle.getB()
  << " " << triangle.getC() << ">";
}
