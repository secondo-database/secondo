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

#ifndef _SPATIAL3DGEOMETRIC_ALGORITHM_H
#define _SPATIAL3DGEOMETRIC_ALGORITHM_H

#include<math.h>
#include<vector>
#include "../../include/AlmostEqual.h"
#include "Spatial3D.h"

namespace spatial3d_geometric
{
    /* TODO: Remove debugging helpers (Jens Breit) */
  
  void print(SimplePoint3d a);
  
  void print(Vector3d a);
  
  void print(SimplePoint2d a, string name);
  
  void print(Triangle a);

  class NumericFailure {};

  void numeric_fail();
  
  /* Types */
  
  enum InsideResult { INSIDE=0, EDGE=1, CORNER=2, OUTSIDE=3 };
  
  
  /* Points */
  
  bool almostEqual(const SimplePoint3d& p1, const SimplePoint3d& p2);
  
  bool collinear(const SimplePoint3d& pA,
                 const SimplePoint3d& pB,
                 const SimplePoint3d& pC);
  
  double distance(const SimplePoint3d& p1, const SimplePoint3d& p2);

  double distancePointToLine(const SimplePoint3d& p,
           const SimplePoint3d& linePoint1, const SimplePoint3d& linePoint2);
  
  /* Vectors */
  
  bool almostEqual(const Vector3d& v1, const Vector3d& v2);
  
  double length(const Vector3d& vector);
  
  bool collinear(const Vector3d& v1, const Vector3d& v2);
  
  bool orthogonal(const Vector3d& v1, const Vector3d& v2);

  Vector3d operator+(const Vector3d& v1, const Vector3d& v2);
  Vector3d operator-(const Vector3d& v1, const Vector3d& v2);
  Vector3d operator*(double scalar, const Vector3d& vector);
  double operator*(const Vector3d& v1, const Vector3d& v2);
  Vector3d crossProduct(const Vector3d& v1, const Vector3d& v2);
  SimplePoint3d operator+(const SimplePoint3d& p, const Vector3d& v);

  
  /* Planes */

  bool almostEqual(const Plane3d& p1, const Plane3d& p2);
  
  Vector3d normalVector(const SimplePoint3d& pA,
                        const SimplePoint3d& pB,
                        const SimplePoint3d& pC);
  
  double distance(const SimplePoint3d& point, const Plane3d& plane);
  
  bool isPointInPlane(const SimplePoint3d& point, const Plane3d& plane);
  
  double planeDistanceToOrigin(const SimplePoint3d& pointInPlane,
                               const Vector3d& normalVector);
  
  void planeHessianNormalForm(const SimplePoint3d& pA,
                              const SimplePoint3d& pB,
                              const SimplePoint3d& pC,
                              double& out_distanceToOrigin,
                              Vector3d& out_normalVector);

  SimplePoint3d projectPointOntoPlane(const SimplePoint3d &point,
                                      const Plane3d& plane);

  /* Triangles */

  // assumes directed triangles
  bool almostEqual(const Triangle& triangle1, const Triangle& triangle2);
  
  bool isValidTriangle(const SimplePoint3d& pA,
                       const SimplePoint3d& pB,
                       const SimplePoint3d& pC);
  bool isValidTriangle(const Triangle& triangle);

  // precondition: point is in the plane of the triangle.
  InsideResult pointInsideTriangle(const SimplePoint3d& pointToTest,
                                   const Triangle& triangle);
  
  bool isCompletelyInside(const Triangle& t1, const Triangle& t2);

  /* 2D */
  
  bool almostEqual(const SimplePoint2d& p1, const SimplePoint2d& p2);
  
  double distance(const SimplePoint2d& p1, const SimplePoint2d& p2);
  
  double distance_square(const SimplePoint2d& p1, const SimplePoint2d& p2);
  
  double getPolarAngle(const SimplePoint2d& point);
  
  double clockwise(const SimplePoint2d& p1,
                   const SimplePoint2d& p2,
                   const SimplePoint2d& p3);

  // never true for parallel segments
  bool doSegmentsIntersect(const SimplePoint2d& a1, const SimplePoint2d& a2,
                           const SimplePoint2d& b1, const SimplePoint2d& b2);

  // returns false if lines are parallel
  bool lineIntersectionPoint(const SimplePoint2d& a1, const SimplePoint2d& a2,
                             const SimplePoint2d& b1, const SimplePoint2d& b2,
                             SimplePoint2d& out_intersection);

  InsideResult pointInsideTriangle(const SimplePoint2d& pointToTest,
                                   const SimplePoint2d& pointA,
                                   const SimplePoint2d& pointB,
                                   const SimplePoint2d& pointC);

  // Never returns edge
  InsideResult pointInsideSegment(const SimplePoint2d& pointToTest,
                                  const SimplePoint2d& segmentPoint1,
                                  const SimplePoint2d& segmentPoint2);

  // precondition: the segment does share a point with the triangle
  SimplePoint2d firstPointInsideTriangle(const SimplePoint2d& from,
                                         const SimplePoint2d& to,
                                         const SimplePoint2d& t1,
                                         const SimplePoint2d& t2,
                                         const SimplePoint2d& t3);

  enum SegmentTriangle2dIntersectionResult { NONE = 0, POINT = 1, SEGMENT = 2 };

  SegmentTriangle2dIntersectionResult
  intersection(const SimplePoint2d& segmentA, const SimplePoint2d& segmentB,
               const SimplePoint2d* triangle);
  
  /* Set operations */ 
 
  bool prepareSetOperationSurface(const TriangleContainer& in_1,
                                  const TriangleContainer& in_2,
                                  vector<Triangle>& out_only_1,
                                  vector<Triangle>& out_only_2,
                                  vector<Triangle>& out_both);

  bool prepareSetOperationVolume(const Volume3d& in_1,
                                 const Volume3d& in_2,
                                 vector<Triangle>& out_only_1_outside_2,
                                 vector<Triangle>& out_only_2_outside_1,
                                 vector<Triangle>& out_only_1_inside_2,
                                 vector<Triangle>& out_only_2_inside_1,
                                 vector<Triangle>& out_both_same_direction,
                                 vector<Triangle>& out_both_opposite_direction);
}

#endif