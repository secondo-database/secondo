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

SimplePoint2d::SimplePoint2d() { }

SimplePoint2d::SimplePoint2d(double x, double y)
{
  set(x, y);
}

SimplePoint2d::SimplePoint2d(const SimplePoint2d& src)
{
  set(src.getX(), src.getY());
}

void SimplePoint2d::set(double x, double y)
{
  this->x = x;
  this->y = y;
}

double SimplePoint2d::getX() const
{
  return x;
}

double SimplePoint2d::getY() const
{
  return y;
}

bool SimplePoint2d::operator==(const SimplePoint2d& other) const
{
  return x == other.x && y == other.y;
}

ostream& operator<< (ostream& os, const SimplePoint2d& point) {
  return os << "(" << point.getX()
            << "/" << point.getY() << ")";
}
