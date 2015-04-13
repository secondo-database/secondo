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

#include<memory>

#include "Spatial3D.h"
#include "geometric_algorithm.h"
#include "geometric_algorithm_intersection_line_plane.h"
#include "geometric_algorithm_intersection_triangles.h"

namespace spatial3d_geometric
{
  TriangleIntersectionResult::TriangleIntersectionType
  TriangleIntersectionResult::getIntersectionType()
  {
    return intersectionType;
  }
   
  vector<SimplePoint3d>& TriangleIntersectionResult::getIntersectionPoints()
  {
    return intersectionPoints;
  }
  
  TriangleIntersectionResult::TriangleIntersectionResult()
    : intersectionPoints() { }

  TriangleIntersectionResult::TriangleIntersectionResult(
                  vector<SimplePoint3d> _intersectionPoints,
                  TriangleIntersectionType type)
     : intersectionPoints(_intersectionPoints),
       intersectionType(type) { }
  
  TriangleIntersectionResult intersection(const Plane3d& plane,
                                          const Triangle& triangle)
  {
    vector<SimplePoint3d> resultSegment;
    
    vector<SimplePoint3d> pointsLeft;
    vector<SimplePoint3d> pointsRight;
    vector<SimplePoint3d> pointsOn;
    
    SimplePoint3d points[3] = {
      triangle.getA(), triangle.getB(), triangle.getC() };
    
    for (int c = 0; c < 3; ++c)
    {
      double side = plane.getNormalVector()
                    * Vector3d(plane.getPoint(), points[c]);
      if (AlmostEqual(side, 0))
      {
        pointsOn.push_back(points[c]);
      }
      else if (side < 0)
      {
        pointsLeft.push_back(points[c]);
      }
      else // if (site > 0)
      {
        pointsRight.push_back(points[c]);
      }
    }
    
    if (pointsLeft.size() == 3 || pointsRight.size() == 3)
    {
      // All points are on the same side of the plane: triangle cannot intersect
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::NO_INTERSECTION);
    }
    if (pointsOn.size() == 3)
    {
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::AREA);
    }
    if (pointsOn.size() == 2)
    {
      resultSegment = pointsOn;
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::SEGMENT);
    }
    if (pointsOn.size() == 1 && pointsLeft.size() == 1)
    {
      resultSegment = pointsOn;
      SimplePoint3d intersectionPoint =
           intersection(pointsLeft[0], pointsRight[0], plane)
                    .getIntersectionPoint();
      resultSegment.push_back(intersectionPoint);
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::SEGMENT);
    }
    if (pointsOn.size() == 1)
    {
      resultSegment = pointsOn;
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::POINT);
    }
    else
    {
      // two points on one side, one point on the other side.
      for (vector<SimplePoint3d>::iterator l = pointsLeft.begin();
           l != pointsLeft.end(); ++l)
      {
        for (vector<SimplePoint3d>::iterator r = pointsRight.begin();
             r != pointsRight.end(); ++r)
        {
          resultSegment.push_back(
              intersection(*l, *r, plane).getIntersectionPoint());
        }
      }
      return TriangleIntersectionResult(resultSegment,
                              TriangleIntersectionResult::SEGMENT);
    }
  }

  // helper function for some cases in intersection(Triangle&, Triangle&) below
  TriangleIntersectionResult::TriangleIntersectionType
  intersectionType(const SimplePoint3d& point,
                   const Triangle& triangle,
                   vector<SimplePoint3d>& result)
  {
    switch(pointInsideTriangle(point, triangle))
    {
      case OUTSIDE:
        return TriangleIntersectionResult::NO_INTERSECTION;
      default:
        result.push_back(point);
        return TriangleIntersectionResult::POINT;
    }
  }
  
  TriangleIntersectionResult intersection(const Triangle& t1,
                                          const Triangle& t2)
  {
    Plane3d plane1 = t1.getPlane();
    Plane3d plane2 = t2.getPlane();
    
    vector<SimplePoint3d> resultSegment;
    
    TriangleIntersectionResult res1 = intersection(plane1, t2);
    TriangleIntersectionResult res2 = intersection(plane2, t1);
      
    if (res1.getIntersectionType() == TriangleIntersectionResult::AREA ||
        res2.getIntersectionType() == TriangleIntersectionResult::AREA)
    {
      // Triangles are on the same plane.
        
      Transformation2d t(plane1);
        
      SimplePoint2d triangle1_2d[3] = { t.transform(t1.getA()),
                                        t.transform(t1.getB()),
                                        t.transform(t1.getC()) };
      SimplePoint2d triangle2_2d[3] = { t.transform(t2.getA()),
                                        t.transform(t2.getB()),
                                        t.transform(t2.getC()) };

      InsideResult pointsOf1Inside2[3];
      InsideResult pointsOf2Inside1[3];
        
      for (int c = 0; c < 3; ++c)
      {
        pointsOf1Inside2[c] = pointInsideTriangle(triangle1_2d[c],
                                                  triangle2_2d[0],
                                                  triangle2_2d[1],
                                                  triangle2_2d[2]);
        pointsOf2Inside1[c] = pointInsideTriangle(triangle2_2d[c],
                                                  triangle1_2d[0],
                                                  triangle1_2d[1],
                                                  triangle1_2d[2]);
      }
      int number_of_points_not_outside_1 = 0;
      int number_of_points_not_outside_2 = 0;
      for (int c = 0; c < 3; ++c)
      {
        if (pointsOf1Inside2[c] == INSIDE || pointsOf2Inside1[c] == INSIDE)
        {
          // If any point is on the inside of the other triangle,
          // we can be sure to have an overlap area.
          return TriangleIntersectionResult(resultSegment,
                                    TriangleIntersectionResult::AREA);
        }
        if (pointsOf2Inside1[c] != OUTSIDE)
        {
          ++ number_of_points_not_outside_1;
        }
        if (pointsOf1Inside2[c] != OUTSIDE)
        {
          ++ number_of_points_not_outside_2;
        }
      }
      if (number_of_points_not_outside_1 == 3 ||
          number_of_points_not_outside_2 == 3)
      {
        return TriangleIntersectionResult(resultSegment,
                                    TriangleIntersectionResult::AREA);
      }

      // Now that we know that no triangle is completely inside the other
      // triangle, we can tell whether they intersect by searching for
      // intersections of the edges.
        
      bool intersection_of_edges[3][3];
      SimplePoint2d intersectionPoints[3][3];
      int number_of_intersections = 0;
      
      for (int c1 = 0; c1 < 3; ++c1)
      {
        for (int c2 = 0; c2 < 3; ++c2)
        {
          intersection_of_edges[c1][c2] = doSegmentsIntersect(
            triangle1_2d[c1], triangle1_2d[(c1 + 1) % 3],  // first edge
            triangle2_2d[c2], triangle2_2d[(c2 + 1) % 3]);
          if (intersection_of_edges[c1][c2])
          {
            SimplePoint2d out_point;
            ++ number_of_intersections;
            lineIntersectionPoint(
                triangle1_2d[c1], triangle1_2d[(c1 + 1) % 3],  // first edge
                triangle2_2d[c2], triangle2_2d[(c2 + 1) % 3], out_point);
            intersectionPoints[c1][c2] = out_point;
            if (!almostEqual(out_point, triangle1_2d[c1])
             && !almostEqual(out_point, triangle1_2d[(c1 + 1) % 3])
             && !almostEqual(out_point, triangle2_2d[c2])
             && !almostEqual(out_point, triangle2_2d[(c2 + 1) % 3]))
            {
              // If two segments intersect, but the intersection point is not
              // the end of either segment, then we know for sure that we
              // have an intersection area.
              return TriangleIntersectionResult(resultSegment,
                                         TriangleIntersectionResult::AREA);
            }
          }
        }
      }

      if (number_of_intersections == 0)
      {
        return TriangleIntersectionResult(resultSegment,
                          TriangleIntersectionResult::NO_INTERSECTION);
      }
        
      // There are intersections, so we now know the triangles intersect.
      // We still have to find out whether it is an intersection point or
      // an intersection segment. (Area is ruled out before.)
        
      // To find out, compare the intersection points. If its the same point
      // for all intersections, we have an intersection point. If its two
      // different points, we have an intersection segment between these
      // points. Three points are impossible without an intersection area.
        
      vector<SimplePoint2d> resultSegment2d;

      for (int c1 = 0; c1 < 3; ++c1)
      {
        for (int c2 = 0; c2 < 3; ++c2)
        {
          if (intersection_of_edges[c1][c2])
          {
            // compare to intersection points we already have
            bool pointAlreadyKnown = false;
            for (vector<SimplePoint2d>::iterator d = resultSegment2d.begin();
                 d != resultSegment2d.end(); ++d)
            {
              if (almostEqual(*d, intersectionPoints[c1][c2]))
              {
                pointAlreadyKnown = true;
              }
            }
            if (!pointAlreadyKnown)
            {
              resultSegment2d.push_back(intersectionPoints[c1][c2]);
            }
          }
        }
      }

      for (vector<SimplePoint2d>::iterator it = resultSegment2d.begin();
           it != resultSegment2d.end(); ++it)
      {
        resultSegment.push_back(t.transform(*it));
      }       

      return TriangleIntersectionResult(resultSegment,
                                        resultSegment.size() == 1 ?
                                        TriangleIntersectionResult::POINT :
                                        TriangleIntersectionResult::SEGMENT);
    }
    else if (collinear(plane1.getNormalVector(), plane2.getNormalVector()))
    {
      // Triangle planes are not identical, but parallel.
      return TriangleIntersectionResult(resultSegment,
                            TriangleIntersectionResult::NO_INTERSECTION);
    }
    else
    {
      if (res1.getIntersectionType() ==
                                 TriangleIntersectionResult::NO_INTERSECTION
        || res2.getIntersectionType() ==
                                 TriangleIntersectionResult::NO_INTERSECTION)
      {
        // A triangle does not intersect the plane of the other triangle
        return TriangleIntersectionResult(resultSegment,
                                TriangleIntersectionResult::NO_INTERSECTION);
      }

      if (res1.getIntersectionType() == TriangleIntersectionResult::POINT)
      {
        // Triangle2 shares a single point with the plane of triangle1.
        // Is it inside of triangle1?
        
        TriangleIntersectionResult::TriangleIntersectionType resultType =
          intersectionType(res1.getIntersectionPoints()[0], t1, resultSegment);
        return TriangleIntersectionResult(resultSegment, resultType);
      }
      if (res2.getIntersectionType() == TriangleIntersectionResult::POINT)
      {
        TriangleIntersectionResult::TriangleIntersectionType resultType =
          intersectionType(res2.getIntersectionPoints()[0], t2, resultSegment);
        return TriangleIntersectionResult(resultSegment, resultType);
      }

      // Both intersections are segments that are on a common line.
      // To find out whether they overlap, project them onto an axis of
      // the coordinate system. Choose the axis where the coordinates
      // have the largest difference.

      double min_value[3], max_value[3];
      
      for(int axis = X; axis <= Z; axis++)
      {
        min_value[axis] = res1.getIntersectionPoints()[0].get((Axis)axis);
        min_value[axis] = min(min_value[axis],
                              res1.getIntersectionPoints()[1].get((Axis)axis));
        min_value[axis] = min(min_value[axis],
                              res2.getIntersectionPoints()[0].get((Axis)axis));
        min_value[axis] = min(min_value[axis],
                              res2.getIntersectionPoints()[1].get((Axis)axis));

        max_value[axis] = res1.getIntersectionPoints()[0].get((Axis)axis);
        max_value[axis] = max(max_value[axis],
                              res1.getIntersectionPoints()[1].get((Axis)axis));
        max_value[axis] = max(max_value[axis],
                              res2.getIntersectionPoints()[0].get((Axis)axis));
        max_value[axis] = max(max_value[axis],
                              res2.getIntersectionPoints()[1].get((Axis)axis));
      }

      Axis chosenAxis = X;
      double max_difference = max_value[X] - min_value[X];
      
      for (int axis = Y; axis <= Z; axis++)
      {
        if (max_value[axis] - min_value[axis] > max_difference)
        {
          chosenAxis = (Axis)axis;
          max_difference = max_value[axis] - min_value[axis];
        }
      }
      
      double segment1[2], segment2[2];
      
      segment1[0] = res1.getIntersectionPoints()[0].get(chosenAxis);
      segment1[1] = res1.getIntersectionPoints()[1].get(chosenAxis);
      segment2[0] = res2.getIntersectionPoints()[0].get(chosenAxis);
      segment2[1] = res2.getIntersectionPoints()[1].get(chosenAxis);

      double minSegment1 = min(segment1[0], segment1[1]);
      double maxSegment1 = max(segment1[0], segment1[1]);
      double minSegment2 = min(segment2[0], segment2[1]);
      double maxSegment2 = max(segment2[0], segment2[1]);
      
      if (AlmostEqual(minSegment1, maxSegment2))
      {
        // Overlapping with a single point
        if (segment1[0] == minSegment1)
        {
          resultSegment.push_back(res1.getIntersectionPoints()[0]);
        }
        else
        {
          resultSegment.push_back(res1.getIntersectionPoints()[1]);
        }
        return TriangleIntersectionResult(resultSegment,
                                          TriangleIntersectionResult::POINT);
      }
      if (AlmostEqual(minSegment2, maxSegment1))
      {
        // Overlapping with a single point
        if (segment2[0] == minSegment2)
        {
          resultSegment.push_back(res2.getIntersectionPoints()[0]);
        }
        else
        {
          resultSegment.push_back(res2.getIntersectionPoints()[1]);
        }
        return TriangleIntersectionResult(resultSegment,
                                          TriangleIntersectionResult::POINT);
      }
      if (minSegment1 > maxSegment2 && !AlmostEqual(minSegment1, maxSegment2))
      {
        return TriangleIntersectionResult(resultSegment,
                                  TriangleIntersectionResult::NO_INTERSECTION);
      }
      if (minSegment2 > maxSegment1 && !AlmostEqual(minSegment2, maxSegment1))
      {
        return TriangleIntersectionResult(resultSegment,
                                  TriangleIntersectionResult::NO_INTERSECTION);
      }
      if (minSegment1 >= minSegment2 && maxSegment1 <= maxSegment2)
      {
        // Segment 1 is completely inside of segment 2. Overlap is segment 1.
        resultSegment.push_back(res1.getIntersectionPoints()[0]);
        resultSegment.push_back(res1.getIntersectionPoints()[1]);
        return TriangleIntersectionResult(resultSegment,
                                          TriangleIntersectionResult::SEGMENT);
      }
      if (minSegment2 >= minSegment1 && maxSegment2 <= maxSegment1)
      {
        // Segment 2 is completely inside of segment 1. Overlap is segment 2.
        resultSegment.push_back(res2.getIntersectionPoints()[0]);
        resultSegment.push_back(res2.getIntersectionPoints()[1]);
        return TriangleIntersectionResult(resultSegment,
                                          TriangleIntersectionResult::SEGMENT);
      }
      // Now we know that the segments overlap and its a segment, not a point.
      // For the left point, choose the maximum of the minimum.
      // For the right point, choose the minimum of the maximum.
      double left1d = max(minSegment1, minSegment2);
      double right1d = min(maxSegment1, maxSegment2);

      // Now we have to find the original points.
      SimplePoint3d left3d, right3d;

      SimplePoint3d points[4] = { res1.getIntersectionPoints()[0],
                                  res1.getIntersectionPoints()[1],
                                  res2.getIntersectionPoints()[0],
                                  res2.getIntersectionPoints()[1] };

      for(int c = 0; c < 4; ++c)
      {
        if (points[c].get(chosenAxis) == left1d)
        {
          left3d = points[c];
        }
        if (points[c].get(chosenAxis) == right1d)
        {
          right3d = points[c];
        }
      }
    
      resultSegment.push_back(left3d);
      resultSegment.push_back(right3d);
      return TriangleIntersectionResult(resultSegment,
                                        TriangleIntersectionResult::SEGMENT);
    }
  }
}
