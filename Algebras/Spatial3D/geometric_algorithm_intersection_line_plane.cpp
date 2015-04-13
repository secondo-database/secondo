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


#include "Spatial3D.h"
#include "geometric_algorithm.h"
#include "geometric_algorithm_intersection_line_plane.h"

namespace spatial3d_geometric
{
  bool IntersectionPointResult::segmentIntersects()
  {
    switch (resultType)
    {
      case NONE:
        return false;
      case EDGE:
      case INNER:
      case ON_PLANE:
        return intersectionParameter >= 0 && intersectionParameter <= 1;
      default:
        assert(false);
    }
  }
  
  bool IntersectionPointResult::rayIntersects()
  {
    switch (resultType)
    {
      case NONE:
        return false;
      case EDGE:
      case INNER:
      case ON_PLANE:
        return intersectionParameter >= 0;
      default:
        assert(false);
    }
  }
  
  bool IntersectionPointResult::lineIntersects()
  {
    switch (resultType)
    {
      case NONE:
        return false;
      case EDGE:
      case INNER:
      case ON_PLANE:
        return true;
      default:
        assert(false);
    }
  }
  
  bool IntersectionPointResult::hasIntersectionPoint()
  {
    switch (resultType)
    {
      case NONE:
      case ON_PLANE:
        return false;
      case EDGE:
      case INNER:
        return true;
      default:
        assert(false);
    }
  }
 
  double IntersectionPointResult::getIntersectionParameter()
  {
    assert(resultType == EDGE || resultType == INNER);
    return intersectionParameter;
  }
  
  SimplePoint3d IntersectionPointResult::getIntersectionPoint()
  {
    assert(resultType == EDGE || resultType == INNER);
    return intersectionPoint;
  }
  
  bool IntersectionPointResult::isIntersectionOnTriangleEdge()
  {
    return resultType == EDGE;
  }
  
  IntersectionPointResult::IntersectionPointResult(
                  IntersectionPointResult::TriangleLineIntersection _resultType,
                  double _intersectionParameter,
                  const SimplePoint3d& _intersectionPoint)
      : resultType(_resultType),
        intersectionParameter(_intersectionParameter),
        intersectionPoint(_intersectionPoint) { }
  
  
  IntersectionPointResult intersection(const SimplePoint3d& p0,
                                       const SimplePoint3d& p1,
                                       const Plane3d& plane)
  {
    return intersection(p0, Vector3d(p0, p1), plane);
  }
  
  IntersectionPointResult intersection(const SimplePoint3d& p0,
                                       const Vector3d& segmentVector,
                                       const Plane3d& plane)
  {
    Vector3d n = plane.getNormalVector();

    double denominator = n * segmentVector;

    if (AlmostEqual(denominator, 0))
    {
      // Parallel
      
      if (isPointInPlane(p0, plane))
      {
        return IntersectionPointResult(IntersectionPointResult::ON_PLANE,
                                       0, SimplePoint3d(0,0,0));
      }
      else
      {      
        return IntersectionPointResult(IntersectionPointResult::NONE,
                                       0, SimplePoint3d(0,0,0));
      }
    }
    
    double r = n * Vector3d(p0, plane.getPoint()) / denominator;
    SimplePoint3d intersectionPoint(p0 + r * segmentVector);
    return IntersectionPointResult(IntersectionPointResult::INNER,
                                   r, intersectionPoint);
  }
  
  IntersectionPointResult intersection(const SimplePoint3d& p0,
                                       const SimplePoint3d& p1,
                                       const Triangle& triangle)
  {
    return intersection(p0, Vector3d(p0, p1), triangle);
  }
  
  IntersectionPointResult intersection(const SimplePoint3d& p0,
                                       const Vector3d& segmentVector,
                                       const Triangle& triangle)
  {
    Vector3d n = triangle.getNormalVector();

    double denominator = n * segmentVector;
    
    if (AlmostEqual(denominator, 0))
    {
      // Parallel
      
      if (isPointInPlane(p0, Plane3d(triangle.getA(),
                                     triangle.getB(),
                                     triangle.getC())))
      {
        Transformation2d t(triangle.getPlane());
        
        SimplePoint2d segment1_2d = t.transform(p0);
        SimplePoint2d segment2_2d = t.transform(p0 + segmentVector);
        SimplePoint2d triangleA_2d = t.transform(triangle.getA());
        SimplePoint2d triangleB_2d = t.transform(triangle.getB());
        SimplePoint2d triangleC_2d = t.transform(triangle.getC());
        
        bool intersectionOnPlane =
           doSegmentsIntersect(segment1_2d, segment2_2d,
                               triangleA_2d, triangleB_2d) ||
           doSegmentsIntersect(segment1_2d, segment2_2d,
                               triangleB_2d, triangleC_2d) ||
           doSegmentsIntersect(segment1_2d, segment2_2d,
                               triangleC_2d, triangleA_2d);

        if (intersectionOnPlane)
        {
          // TODO: IntersectionParameter
          return IntersectionPointResult(IntersectionPointResult::ON_PLANE,
                                         0, SimplePoint3d(0,0,0));
        }
        else
        {
          return IntersectionPointResult(IntersectionPointResult::NONE,
                                         0, SimplePoint3d(0,0,0));
        }
      }
      else
      {      
        return IntersectionPointResult(IntersectionPointResult::NONE,
                                       0, SimplePoint3d(0,0,0));
      }
    }
    
    double r = n * Vector3d(p0, triangle.getA()) / denominator;
    SimplePoint3d intersectionPoint(p0 + r * segmentVector);
    
    IntersectionPointResult::TriangleLineIntersection resultType;
    switch(pointInsideTriangle(intersectionPoint, triangle))
    {
      case CORNER:
      case EDGE:
        resultType = IntersectionPointResult::EDGE;
        break;
      case INSIDE:
        resultType = IntersectionPointResult::INNER;
        break;
      case OUTSIDE:
        resultType = IntersectionPointResult::NONE;
        break;
    }

    return IntersectionPointResult(resultType, r, intersectionPoint);
  }
}