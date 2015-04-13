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
#include "geometric_algorithm_intersection_line_plane.h"

using namespace spatial3d_geometric;

Transformation2d::Transformation2d(const Plane3d& plane)
{
  this->plane = plane;
  Vector3d n = plane.getNormalVector();
  double absX = abs(n.getX());
  double absY = abs(n.getY());
  double absZ = abs(n.getZ());
  if (absX >= absY && absX >= absZ)
  {
    lostAxis = X;
  }
  else if (absY >= absX && absY >= absZ)
  {
    lostAxis = Y;
  }
  else
  {
    lostAxis = Z;
  }
}
    
SimplePoint3d Transformation2d::transform(const SimplePoint2d& point) const
{
  SimplePoint3d base;
  Vector3d dir;
  
  switch(lostAxis)
  {
    case X:
      base = SimplePoint3d(0, point.getX(), point.getY());
      dir = Vector3d(1, 0, 0);
      break;
    case Y:
      base = SimplePoint3d(point.getX(), 0, point.getY());
      dir = Vector3d(0, 1, 0);
      break;
    case Z:
      base = SimplePoint3d(point.getX(), point.getY(), 0);
      dir = Vector3d(0, 0, 1);
      break;
  }
  return intersection(base, base + dir, plane).getIntersectionPoint();
}

SimplePoint2d Transformation2d::transform(const SimplePoint3d& point) const
{
  switch(lostAxis)
  {
    case X:
      return SimplePoint2d(point.getY(), point.getZ());
    case Y:
      return SimplePoint2d(point.getX(), point.getZ());
    case Z:
      return SimplePoint2d(point.getX(), point.getY());
    default:
      assert(false);
  }
}