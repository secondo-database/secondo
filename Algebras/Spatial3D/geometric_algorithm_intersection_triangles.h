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

#ifndef _SPATIAL3DGEOMETRIC_ALGORITHM_INTERSECTION_TRIANGLES_H
#define _SPATIAL3DGEOMETRIC_ALGORITHM_INTERSECTION_TRIANGLES_H

#include<memory>

namespace spatial3d_geometric
{
  class TriangleIntersectionResult {
    
  public:
    
    enum TriangleIntersectionType
      { NO_INTERSECTION, POINT, SEGMENT, AREA };
    
    TriangleIntersectionType getIntersectionType();
    
    // empty if NO_INTERSECTION;
    // single point if POINT;
    // two points if SEGMENT;
    // empty if AREA    // TODO: werden Punkte doch gebraucht?
    vector<SimplePoint3d>& getIntersectionPoints();
    
    TriangleIntersectionResult();
    
  private:
    
    vector<SimplePoint3d> intersectionPoints;
    TriangleIntersectionType intersectionType;
    
    // Ownership is transfered to this result class
    TriangleIntersectionResult(vector<SimplePoint3d> _intersectionPoints,
                               TriangleIntersectionType type);

    friend TriangleIntersectionResult intersection(const Plane3d& plane,
                                                   const Triangle& triangle);
  
    friend TriangleIntersectionResult intersection(const Triangle& t1,
                                                   const Triangle& t2);
  };

  TriangleIntersectionResult intersection(const Plane3d& plane,
                                          const Triangle& triangle);

  TriangleIntersectionResult intersection(const Triangle& t1,
                                          const Triangle& t2);
}

#endif