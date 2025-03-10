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

Projection2d::Projection2d() { }

Projection2d::Projection2d(const Plane3d& plane,
                           const SimplePoint3d& base,
                           const SimplePoint3d& directionX)
{
  this->plane = plane;
  this->origin = projectPointOntoPlane(base, plane);
  Vector3d unitXNotNormalized(this->origin,
                              projectPointOntoPlane(directionX, plane));
  this->unitX = (1 / length(unitXNotNormalized)) * unitXNotNormalized;
  this->unitY = crossProduct(plane.getNormalVector(), this->unitX);
}
    
SimplePoint2d Projection2d::project(const SimplePoint3d& point) const
{
  double x = unitX * Vector3d(this->origin, point);
  double y = unitY * Vector3d(this->origin, point);
  
  return SimplePoint2d(x, y);
}