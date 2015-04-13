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
#include "RelationAlgebra.h"
#include "geometric_algorithm.h"
#include "geometric_algorithm_intersection_line_plane.h"
#include "geometric_algorithm_intersection_triangles.h"
#include "MultiObjectTriangleContainer.h"

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace spatial3d_geometric {
  
int number_of_tests_run;
int number_of_tests_failed;

void assert_(std::string message, bool success)
{
  ++number_of_tests_run;
  if (! success)
  {
    ++number_of_tests_failed;
    std::cerr << "Test failed: " << message << std::endl;
  }
}

void assert_(std::string test, std::string message, bool success)
{
  ++number_of_tests_run;
  if (! success)
  {
    ++number_of_tests_failed;
    std::cerr << "Test failed: " << test << ": " << message << std::endl;
  }
}

bool almostEqualPoints(const vector<SimplePoint3d>& points1,
                       const vector<SimplePoint3d>& points2)
{
  if (points1.size() != points2.size())
    return false;
  vector<bool> points1matched(points1.size(), false);
  vector<bool> points2matched(points2.size(), false);
  for (unsigned int c1 = 0; c1 < points1.size(); ++c1)
  {
    if (points1matched[c1])
      continue;
    for (unsigned int c2 = 0; c2 < points2.size(); ++c2)
    {
      if (points2matched[c2])
        continue;
      if (almostEqual(points1[c1], points2[c2]))
      {
        points1matched[c1] = true;
        points2matched[c2] = true;
      }
    }
  }
  for (unsigned int c = 0; c < points1.size(); ++c)
  {
    if (!points1matched.at(c) || !points2matched.at(c))
      return false;
  }
  return true;
}

Rectangle<3> getBoundingBox(const vector<Triangle>& triangles)
{
  Rectangle<3> result = triangles[0].BoundingBox();
  for (int c = 1; c < triangles.size(); ++c)
  {
    result.Extend(triangles[c].BoundingBox());
  }
  return result;
}


void test_getPolarAngle()
{
  SimplePoint2d points[8] = {
    SimplePoint2d(-1, -1),
    SimplePoint2d(-1, -2),
    SimplePoint2d(0, -0.5), // 2
    SimplePoint2d(4, -3),
    SimplePoint2d(1, 0),
    SimplePoint2d(0.5, 0.3),
    SimplePoint2d(0, 400),
    SimplePoint2d(-1, 1)
  };

  SimplePoint2d testP(0, -1.5);

  
  double phi[8];
  
  bool inc = true;
  
  for (int c = 0; c < 8; ++c)
  {
    phi[c] = getPolarAngle(points[c]);
    if (c > 0)
    {
      inc = inc && (phi[c - 1] < phi[c]);
    }
  }
  
  assert_("test_getPolarAngle", "same value", 
          inc && phi[2] == getPolarAngle(testP));
}

void test_intersects_2d_true()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(0,2);
  SimplePoint2d b2(3.5,2);
  
  SimplePoint2d expectedP(1,2);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_true", "Segments intersect", result1);
  assert_("test_intersects_2d_true", "Lines intersect", result2);
  assert_("test_intersects_2d_true", "Intersection point",
          almostEqual(expectedP, intersection));
}

void test_intersects_2d_false()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(0,2);
  SimplePoint2d b2(-3.5,2);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_false", "Segments do not intersect", !result1);
  assert_("test_intersects_2d_false", "Lines intersect", result2);
}

void test_intersects_2d_true_1_endpoint()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(0,2);
  SimplePoint2d b2(1,2);
  
  SimplePoint2d expectedP(1,2);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_true_1_endpoint", "Segments intersect", result1);
  assert_("test_intersects_2d_true_1_endpoint", "Lines intersect", result2);
  assert_("test_intersects_2d_true_1_endpoint", "Intersection point",
          almostEqual(expectedP, intersection));
}

void test_intersects_2d_true_2_endpoints()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(1,2);
  SimplePoint2d b1(0,2);
  SimplePoint2d b2(1,2);
  
  SimplePoint2d expectedP(1,2);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_true_2_endpoints", "Segments intersect", result1);
  assert_("test_intersects_2d_true_2_endpoints", "Lines intersect", result2);
  assert_("test_intersects_2d_true_2_endpoints", "Intersection point",
          almostEqual(expectedP, intersection));
}

void test_intersects_2d_false_parallel()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(0,2);
  SimplePoint2d b2(2,6);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_false_parallel",
          "Segments do not intersect", !result1);
  assert_("test_intersects_2d_false_parallel",
          "Lines do not intersect", !result2);
}

void test_intersects_2d_same()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(2,4);
  SimplePoint2d b2(0,0);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_same",
          "Segments do not intersect", !result1);
  assert_("test_intersects_2d_same",
          "Lines do not intersect", !result2);
}

void test_intersects_2d_subset()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(0,0);
  SimplePoint2d b2(1,2);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_subset", "Segments do not intersect", !result1);
  assert_("test_intersects_2d_subset", "Lines do not intersect", !result2);
}

void test_intersects_2d_overlap()
{
  SimplePoint2d a1(0,0);
  SimplePoint2d a2(2,4);
  SimplePoint2d b1(1,2);
  SimplePoint2d b2(4,8);
  
  bool result1 = doSegmentsIntersect(a1, a2, b1, b2);
  SimplePoint2d intersection;
  bool result2 = lineIntersectionPoint(a1, a2, b1, b2, intersection);

  assert_("test_intersects_2d_overlap", "Segments do not intersect", !result1);
  assert_("test_intersects_2d_overlap", "Lines do not intersect", !result2);
}

void test_intersection2d_false()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(0,1);
  SimplePoint2d segmentB(0,2);
  
  SegmentTriangle2dIntersectionResult expectedResult = NONE;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_false",
          "No intersection",
          result == expectedResult);
}

void test_intersection2d_point_corner_middle()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(0,2);
  SimplePoint2d segmentB(4,2);
  
  SegmentTriangle2dIntersectionResult expectedResult = POINT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_point_corner_middle",
          "Intersection point",
          result == expectedResult);
}

void test_intersection2d_point_corner_end()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(0,2);
  SimplePoint2d segmentB(2,2);
  
  SegmentTriangle2dIntersectionResult expectedResult = POINT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_point_corner_end",
          "Intersection point",
          result == expectedResult);
}

void test_intersection2d_point_edge_end()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(0,1);
  SimplePoint2d segmentB(1,1);
  
  SegmentTriangle2dIntersectionResult expectedResult = POINT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_point_edge_end",
          "Intersection point",
          result == expectedResult);
}

void test_intersection2d_segment_edge()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(2,2);
  SimplePoint2d segmentB(0,0);
  
  SegmentTriangle2dIntersectionResult expectedResult = SEGMENT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_segment_edge",
          "Intersection segment",
          result == expectedResult);
}

void test_intersection2d_segment_inner()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(2,1);
  SimplePoint2d segmentB(1,0.5);
  
  SegmentTriangle2dIntersectionResult expectedResult = SEGMENT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_segment_inner",
          "Intersection segment",
          result == expectedResult);
}

void test_intersection2d_segment_cut()
{
  SimplePoint2d triangle[3] = {
    SimplePoint2d(0,0),
    SimplePoint2d(4,0),
    SimplePoint2d(2,2)
  };
  SimplePoint2d segmentA(0,1);
  SimplePoint2d segmentB(4,1);
  
  SegmentTriangle2dIntersectionResult expectedResult = SEGMENT;
  SegmentTriangle2dIntersectionResult result;
  
  result = intersection(segmentA, segmentB, triangle);
  
  assert_("test_intersection2d_segment_cut",
          "Intersection segment",
          result == expectedResult);
}


void test_pointInsideTriangle_true()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d pointToTest(3,2);
  
  InsideResult result = pointInsideTriangle(pointToTest, t1, t2, t3);
  assert_("test_pointInsideTriangle_true",
          "Point is inside", result == INSIDE);
}

void test_pointInsideTriangle_false()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d pointToTest(-3,2);
  
  InsideResult result = pointInsideTriangle(pointToTest, t1, t2, t3);
  assert_("test_pointInsideTriangle_false",
          "Point is not inside", result == OUTSIDE);
}

void test_pointInsideTriangle_edge()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d pointToTest(2,2);
  
  InsideResult result = pointInsideTriangle(pointToTest, t1, t2, t3);
  assert_("test_pointInsideTriangle_edge",
          "Point is on the edge", result == EDGE);
}

void test_pointInsideTriangle_vertex()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d pointToTest(3,3);
  
  InsideResult result = pointInsideTriangle(pointToTest, t1, t2, t3);
  assert_("test_pointInsideTriangle_vertex",
          "Point is on the vertex", result == CORNER);
}

void test_pointInsideSegment_true()
{
  SimplePoint2d s1(1,1);
  SimplePoint2d s2(3,5);
  
  SimplePoint2d pointToTest(2,3);
  
  InsideResult result = pointInsideSegment(pointToTest, s1, s2);
  assert_("test_pointInsideSegment_true",
          "Point is inside the segment", result == INSIDE);
}

void test_pointInsideSegment_false()
{
  SimplePoint2d s1(1,1);
  SimplePoint2d s2(3,5);
  
  SimplePoint2d pointToTest(2,4);
  
  InsideResult result = pointInsideSegment(pointToTest, s1, s2);
  assert_("test_pointInsideSegment_false",
          "Point is not inside the segment", result == OUTSIDE);
}

void test_pointInsideSegment_endpoint()
{
  SimplePoint2d s1(1,1);
  SimplePoint2d s2(3,5);
  
  SimplePoint2d pointToTest(3,5);
  
  InsideResult result = pointInsideSegment(pointToTest, s1, s2);
  assert_("test_pointInsideSegment_endpoint",
          "Point is inside the segment", result == CORNER);
}

void test_firstPointInsideTriangle_inside()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);

  SimplePoint2d s1(2.5,2);
  SimplePoint2d s2(3,2);
  
  SimplePoint2d expectedP(2.5,2);
  
  SimplePoint2d result = firstPointInsideTriangle(s1, s2, t1, t2, t3);
  assert_("test_firstPointInsideTriangle_inside",
          "Point in triangle", almostEqual(expectedP, result));
}

void test_firstPointInsideTriangle_edge()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);

  SimplePoint2d s1(1,1);
  SimplePoint2d s2(3,2);

  SimplePoint2d expectedP(1,1);
  
  SimplePoint2d result = firstPointInsideTriangle(s1, s2, t1, t2, t3);
  assert_("test_firstPointInsideTriangle_edge",
          "Point in triangle", almostEqual(expectedP, result));
}

void test_firstPointInsideTriangle_vertex()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d s1(2,2);
  SimplePoint2d s2(3,2);

  SimplePoint2d expectedP(2,2);
  
  SimplePoint2d result = firstPointInsideTriangle(s1, s2, t1, t2, t3);
  assert_("test_firstPointInsideTriangle_vertex",
          "Point in triangle", almostEqual(expectedP, result));
}

void test_firstPointInsideTriangle_outside_edge()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);
  
  SimplePoint2d s1(1,2);
  SimplePoint2d s2(3,2);

  SimplePoint2d expectedP(2,2);
  
  SimplePoint2d result = firstPointInsideTriangle(s1, s2, t1, t2, t3);
  assert_("test_firstPointInsideTriangle_outside_edge",
          "Point in triangle", almostEqual(expectedP, result));  
}

/*
Function test\_firstPointInsideTriangle\_outside\_vertex

*/

void test_firstPointInsideTriangle_outside_vertex()
{
  SimplePoint2d t1(1,1);
  SimplePoint2d t2(3,3);
  SimplePoint2d t3(5,1);

  SimplePoint2d s1(-1,0);
  SimplePoint2d s2(3,2);
  
  SimplePoint2d expectedP(1,1);
  
  SimplePoint2d result = firstPointInsideTriangle(s1, s2, t1, t2, t3);
  assert_("test_firstPointInsideTriangle_outside_vertex",
          "Point in triangle", almostEqual(expectedP, result));  
}

void test_point_almostEqual_true()
{
  SimplePoint3d p1(2, 3.5, -9);
  SimplePoint3d p2(2, 3.5, -9);

  assert_("Equal points are almost equal", almostEqual(p1, p2));
}

void test_point_almostEqual_false()
{
  SimplePoint3d p1(2, 3.5, -9);
  SimplePoint3d p2(2, 3.4, -9);

  assert_("Different points are not almost equal", ! almostEqual(p1, p2));
}

void test_vector_almostEqual_true()
{
  Vector3d p1(2, 3.5, -9);
  Vector3d p2(2, 3.5, -9);

  assert_("Equal vectors are almost equal", almostEqual(p1, p2));
}

void test_plane_almostEqual_true()
{
  Plane3d p1(SimplePoint3d(0,0,0), SimplePoint3d(1,1,1), SimplePoint3d(1,0,0));
  Plane3d p2(SimplePoint3d(0,0,0), SimplePoint3d(1,1,1), SimplePoint3d(0,1,1));
  
  assert_("test_plane_almostEqual_true",
          "Planes are equal", almostEqual(p1, p2));
}

void test_vector_almostEqual_false()
{
  Vector3d p1(2, 3.5, -9);
  Vector3d p2(2, 3.4, -9);

  assert_("Different vectors are not almost equal", ! almostEqual(p1, p2));
}

void test_collinear_true()
{
  SimplePoint3d p1(0, 1, 0);
  SimplePoint3d p2(0 -2, 1 -2, 0 -2);
  SimplePoint3d p3(0 -3, 1 -3, 0 -3);
  
  assert_("Points are collinear", collinear(p1, p2, p3));
}

void test_collinear_true_same_points()
{
  SimplePoint3d p1(1, 2, 3);
  SimplePoint3d p2(1, 2, 3);
  SimplePoint3d p3(1, 2, 3);

  assert_("Same points are collinear", collinear(p1, p2, p3));
}

void test_collinear_false()
{
  SimplePoint3d p1(0, 0, 0);
  SimplePoint3d p2(0, 0, 1);
  SimplePoint3d p3(0, 1, 1);
  
  assert_("Points not are collinear", ! collinear(p1, p2, p3));
}

void test_distance_point_zero()
{
  SimplePoint3d p1(1,2,3);
  SimplePoint3d p2(1,2,3);

  double d = distance(p1, p2);

  assert_("Distance point to point is zero", AlmostEqual(0, d));  
}

void test_distance_point_nonzero()
{
  SimplePoint3d p1(1,2,3);
  SimplePoint3d p2(1,3,2);

  double d = distance(p1, p2);
  assert_("Distance point to point is nonzero", AlmostEqual(sqrt(2), d));  
}

void test_distance_line_zero()
{
  SimplePoint3d linePoint1(1,2,3);
  Vector3d vector(7, -2, 3.6);  
  SimplePoint3d linePoint2(linePoint1.getX() + vector.getX(),
                           linePoint1.getY() + vector.getY(),
                           linePoint1.getZ() + vector.getZ());
  SimplePoint3d linePoint3(linePoint1.getX() + 2.5 * vector.getX(),
                           linePoint1.getY() + 2.5 * vector.getY(),
                           linePoint1.getZ() + 2.5 * vector.getZ());
  
  double d = distancePointToLine(linePoint1, linePoint2, linePoint3);
  assert_("Distance point to line is zero", AlmostEqual(0, d)); 
}

void test_distance_line_nonzero()
{
  SimplePoint3d linePoint1(0,0,0);
  SimplePoint3d linePoint2(0,1,1);

  {
    SimplePoint3d distantPoint(0,0,1);
    double d = distancePointToLine(distantPoint, linePoint1, linePoint2);
    assert_("Distance point to line nonzero 1", AlmostEqual(sqrt(2) / 2.0, d));
  }
  {
    SimplePoint3d distantPoint(1,1,1);
    double d = distancePointToLine(distantPoint, linePoint1, linePoint2);
    assert_("Distance point to line nonzero 2", AlmostEqual(1, d));
  }    
}

void test_length_zero()
{
  Vector3d v(0, 0, 0);
  assert_("Vector length zero", AlmostEqual(0, length(v)));
}

void test_length_nonzero()
{
  Vector3d v(1, 1, 1);
  assert_("Vector length nonzero", AlmostEqual(sqrt(3), length(v)));
}

void test_orthogonal_true()
{
  
}

void test_orthogonal_false()
{
  
}

void test_scalar_multiplication()
{
  
}

void test_multiplication()
{
  
}

void test_crossProduct()
{
  Vector3d v1(1, 2, 3);
  Vector3d v2(-7, 8, 9);
  Vector3d expected(-6, -30, 22);
  Vector3d actual = crossProduct(v1, v2);
  assert_("Cross product incorrect", almostEqual(expected, actual));
}

void test_normal_vector_1()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(-2, 7, 2);
  
  Vector3d actual = normalVector(a, b, c);
  Vector3d expected = (1 / sqrt(2842)) * Vector3d(-36, -39, 5);
  assert_("Normal vector 1 incorrect", almostEqual(expected, actual));
}

void test_normal_vector_2()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);
  
  Vector3d actual = normalVector(k, l, m);
  Vector3d expected = (1 / sqrt(16401)) * Vector3d(-46, -118, 19);
  assert_("Normal vector 2 incorrect", almostEqual(expected, actual));
}

void test_distance_plane_zero()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);

  SimplePoint3d p(4 + 2.5 * (-6 - 5), 1 + 2.5 * (2 + 1), 9 + 2.5 * (-9 + 1));
  
  Plane3d plane(k, l, m);
  double dist = distance(p, plane);
  assert_("Distance to plane zero", AlmostEqual(0, dist));
}

void test_distance_plane_nonzero()
{
  
}

void test_isPointInPlane_true()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);

  SimplePoint3d p(4 + 2.5 * (-6 - 5), 1 + 2.5 * (2 + 1), 9 + 2.5 * (-9 + 1));

  Plane3d plane(k, l, m);
  assert_("Point is in plane", isPointInPlane(p, plane));
}

void test_isPointInPlane_false()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);

  SimplePoint3d p(4 + 1.5 * (-6 - 5), 1 + 2.5 * (2 + 1), 9 + 3.5 * (-9 + 1));
  
  Plane3d plane(k, l, m);
  assert_("Point is not in plane", ! isPointInPlane(p, plane));
}

void test_planeDistanceToOrigin_zero()
{
  
}

void test_planeDistanceToOrigin_nonzero()
{
  
}

void test_planeHessianNormalForm()
{
  
}

void test_projectPointOntoPlane()
{
  Plane3d plane(SimplePoint3d(0,0,1),
                SimplePoint3d(0,1,0),
                SimplePoint3d(1,0,1));
  SimplePoint3d point(1,1,1);
  
  SimplePoint3d expectedP(1,0.5,0.5);
  
  SimplePoint3d result = projectPointOntoPlane(point, plane);
  
  assert_("test_projectPointOntoPlane",
          "Projected point",
          almostEqual(expectedP, result));
}

void test_isValidTriangle_true()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(-2, 7, 2);

  assert_("Valid triangle", isValidTriangle(a, b, c));
}

void test_isValidTriangle_false_same_point_twice()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(6, 0, 5);

  assert_("Not a valid triangle if two points are identical",
         ! isValidTriangle(a, b, c));
}

void test_isValidTriangle_false_points_collinear()
{
  SimplePoint3d a(0, 1, 0);
  SimplePoint3d b(0 -2, 1 -2, 0 -2);
  SimplePoint3d c(0 -3, 1 -3, 0 -3);
  
  assert_("Not a valid triangle if points are collinear",
         ! isValidTriangle(a, b, c));
}

void test_intersection_segmentRayLine_plane_true()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(1,1,1);
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, plane);
  
  assert_("test_intersection_segmentRayLine_plane_true",
          "Segment intersects plane", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_plane_true",
          "Ray intersects plane", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_plane_true",
          "Line intersects plane", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_plane_true",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_plane_true",
          "Intersection parameter",
          AlmostEqual(0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_plane_true",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
}

void test_intersection_segmentRayLine_plane_true_onPlane()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(1,1,1);
  Plane3d plane(SimplePoint3d(-1, -1, -1),
                SimplePoint3d(-0.5, -0.5, -0.5),
                SimplePoint3d(1, 2, 3));
  
  IntersectionPointResult result = intersection(p0, p1, plane);
  
  assert_("test_intersection_segmentRayLine_plane_true_onPlane",
          "Segment on plane intersects plane", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_plane_true_onPlane",
          "Ray on plane intersects plane", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_plane_true_onPlane",
          "Line on plane intersects plane", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_plane_true_onPlane",
          "Intersection on plane: no point", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_plane_false_parallel()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(1,1,1);
  Plane3d plane(SimplePoint3d(-2, -1, -1),
                SimplePoint3d(-1.5, -0.5, -0.5),
                SimplePoint3d(1, 2, 3));
  
  IntersectionPointResult result = intersection(p0, p1, plane);
  
  assert_("test_intersection_segmentRayLine_plane_false_parallel",
          "Segment parallel to plane", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_plane_false_parallel",
          "Ray parallel to plane", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_plane_false_parallel",
          "Line parallel to plane", !result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_plane_false_parallel",
          "Intersection parallel to plane: no point", 
           !result.hasIntersectionPoint());
}

/*
Function test\_intersection\_segmentRayLine\_plane\_OnlyLine

*/

void test_intersection_segmentRayLine_plane_OnlyLine()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(-1,-1,-1);
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, plane);
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Segment intersects plane", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Ray intersects plane", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Line intersects plane", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Intersection parameter",
          AlmostEqual(-0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_plane_OnlyLine",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
}

void test_intersection_segmentRayLine_plane_OnlyLineRay()
{
  SimplePoint3d p0(-1,-1,-1);
  SimplePoint3d p1(0,0,0);
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, plane);
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Segment intersects plane", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Ray intersects plane", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Line intersects plane", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Intersection parameter",
          AlmostEqual(1.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_plane_OnlyLineRay",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
}

void test_intersection_segmentRayLine_triangle_true()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(1,1,1);
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 2),
                    SimplePoint3d(0.5, 2, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Intersection parameter",
          AlmostEqual(0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
  assert_("test_intersection_segmentRayLine_triangle_true",
          "Intersection point not on edge",
          !result.isIntersectionOnTriangleEdge());
}

void test_intersection_segmentRayLine_triangle_true_edge()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(1,1,1);
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 1),
                    SimplePoint3d(0.5, 1, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Intersection parameter",
          AlmostEqual(0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
  assert_("test_intersection_segmentRayLine_triangle_true_edge",
          "Intersection point on edge",
          result.isIntersectionOnTriangleEdge());
}

void test_intersection_segmentRayLine_triangle_true_corner()
{
  SimplePoint3d p0(0,1,0);
  SimplePoint3d p1(1,1,0);
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 1),
                    SimplePoint3d(0.5, 1, 0));
  SimplePoint3d expectedIntersection(0.5, 1, 0);
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Intersection parameter",
          AlmostEqual(0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
  assert_("test_intersection_segmentRayLine_triangle_true_corner",
          "Intersection point on edge",
          result.isIntersectionOnTriangleEdge());
}

void test_intersection_segmentRayLine_triangle_true_onPlane_segment()
{
  SimplePoint3d p0(-1,-1,-1);
  SimplePoint3d p1(1,1,1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_segment",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_segment",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_segment",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_segment",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_true_onPlane_edge()
{
  SimplePoint3d p0(1,0,0);
  SimplePoint3d p1(0,1,1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_edge",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_edge",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_edge",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_edge",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge()
{
  SimplePoint3d p0(-1,-1,-1);
  SimplePoint3d p1(0.5, 0.5, 0.5);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_true_onPlane_corner()
{
  SimplePoint3d p0(1,-1,-1);
  SimplePoint3d p1(1, 1, 1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_corner",
          "Segment intersects triangle", result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_corner",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_corner",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_true_onPlane_corner",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_false_onPlane()
{
  SimplePoint3d p0(-1,0,0);
  SimplePoint3d p1(-1,1,1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_false_onPlane",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_false_onPlane",
          "Ray intersects triangle", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_false_onPlane",
          "Line intersects triangle", !result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_false_onPlane",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_OnlyLine_onPlane()
{
  SimplePoint3d p0(-1,-1,-1);
  SimplePoint3d p1(-2,-2,-2);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine_onPlane",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine_onPlane",
          "Ray intersects triangle", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine_onPlane",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine_onPlane",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane()
{
  SimplePoint3d p0(-2,-2,-2);
  SimplePoint3d p1(-1,-1,-1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_false_parallel()
{
  SimplePoint3d p0(-1,-0.5,-1);
  SimplePoint3d p1(1,1.5,1);
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 0, 0),
                    SimplePoint3d(0, 1, 1));
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_false_parallel",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_false_parallel",
          "Ray intersects triangle", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_false_parallel",
          "Line intersects triangle", !result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_false_parallel",
          "Intersection point is false", !result.hasIntersectionPoint());
}

void test_intersection_segmentRayLine_triangle_OnlyLine()
{
  SimplePoint3d p0(0,0,0);
  SimplePoint3d p1(-1,-1,-1);
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 2),
                    SimplePoint3d(0.5, 2, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Ray intersects triangle", !result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Intersection parameter",
          AlmostEqual(-0.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_triangle_OnlyLine",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
}

void test_intersection_segmentRayLine_triangle_OnlyLineRay()
{
  SimplePoint3d p0(-1,-1,-1);
  SimplePoint3d p1(0,0,0);
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 2),
                    SimplePoint3d(0.5, 2, 0));
  SimplePoint3d expectedIntersection(0.5, 0.5, 0.5);
  
  IntersectionPointResult result = intersection(p0, p1, triangle);
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Segment intersects triangle", !result.segmentIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Ray intersects triangle", result.rayIntersects());
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Line intersects triangle", result.lineIntersects());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Intersection point is true", result.hasIntersectionPoint());
  
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Intersection parameter",
          AlmostEqual(1.5, result.getIntersectionParameter()));
  assert_("test_intersection_segmentRayLine_triangle_OnlyLineRay",
          "Intersection point",
          almostEqual(expectedIntersection, result.getIntersectionPoint()));
}

void test_intersection_plane_triangle_false()
{
  Plane3d plane(SimplePoint3d(2, 0, 0),
                SimplePoint3d(2, 0, 1),
                SimplePoint3d(2, 1, 0));
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 1, 1),
                    SimplePoint3d(1, 0, 0));
  
  TriangleIntersectionResult result = intersection(plane, triangle);

  assert_("test_intersection_plane_triangle_false",
          "Plane and triangle do not intersect",
       result.getIntersectionType() 
           == TriangleIntersectionResult::NO_INTERSECTION);
  assert_("test_intersection_plane_triangle_false",
          "No intersection segment",
       result.getIntersectionPoints().size() == 0);
}

void test_intersection_plane_triangle_false_parallel()
{
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  Triangle triangle(SimplePoint3d(1, 0, 0),
                    SimplePoint3d(1, 0, 1),
                    SimplePoint3d(1, 1, 0));
  
  TriangleIntersectionResult result = intersection(plane, triangle);

  assert_("test_intersection_plane_triangle_false_parallel",
          "Plane and triangle do not intersect",
          result.getIntersectionType() ==
                 TriangleIntersectionResult::NO_INTERSECTION);
  assert_("test_intersection_plane_triangle_false_parallel",
          "No intersection segment",
       result.getIntersectionPoints().size() == 0);
}

void test_intersection_plane_triangle_true()
{
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 1, 1),
                    SimplePoint3d(1, 0, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0.5, 0, 0));
  expectedP.push_back(SimplePoint3d(0.5, 0.5, 0.5));
  
  TriangleIntersectionResult result = intersection(plane, triangle);
  
  assert_("test_intersection_plane_triangle_true",
          "Plane and triangle intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_plane_triangle_true",
          "Plane and triangle have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_plane_triangle_true",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_plane_triangle_true_parallel()
{
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  Triangle triangle(SimplePoint3d(0.5, 0, 0),
                    SimplePoint3d(0.5, 0, 1),
                    SimplePoint3d(0.5, 1, 0));

  TriangleIntersectionResult result = intersection(plane, triangle);

  assert_("test_intersection_plane_triangle_true_parallel",
          "Plane and triangle intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_intersection_plane_triangle_true_single_vertex()
{
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(0.5, 0.5, 0.5),
                    SimplePoint3d(0, 1, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);
  
  TriangleIntersectionResult result = intersection(plane, triangle);
  
  assert_("test_intersection_plane_triangle_true_single_vertex",
          "Plane and triangle intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_plane_triangle_true_single_vertex",
          "Plane and triangle have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_plane_triangle_true_single_vertex",
          "Intersection segment",
          almostEqual(expectedP,
                      result.getIntersectionPoints().at(0)));
}

/*
Function test\_intersection\_plane\_triangle\_true\_edge

*/

void test_intersection_plane_triangle_true_edge()
{
  Plane3d plane(SimplePoint3d(0.5, 0, 0),
                SimplePoint3d(0.5, 0, 1),
                SimplePoint3d(0.5, 1, 0));
  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(0.5, 0.5, 0.5),
                    SimplePoint3d(0.5, 0.5, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0.5, 0.5, 0.5));
  expectedP.push_back(SimplePoint3d(0.5, 0.5, 0));
  
  TriangleIntersectionResult result = intersection(plane, triangle);
  
  assert_("test_intersection_plane_triangle_true_edge",
          "Plane and triangle intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_plane_triangle_true_edge",
          "Plane and triangle have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_plane_triangle_true_edge",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_triangle_triangle_false()
{
  Triangle t1(SimplePoint3d(2, 0, 0),
              SimplePoint3d(2, 0, 1),
              SimplePoint3d(2, 1, 0));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 0, 0));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_false",
          "Plane and triangle do not intersect",
       result.getIntersectionType() 
            == TriangleIntersectionResult::NO_INTERSECTION);
  assert_("test_intersection_triangle_triangle_false",
          "No intersection segment",
       result.getIntersectionPoints().size() == 0);
}

void test_intersection_triangle_triangle_false_parallel()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 1),
              SimplePoint3d(0.5, 1, 0));
  Triangle t2(SimplePoint3d(1, 0, 0),
              SimplePoint3d(1, 0, 1),
              SimplePoint3d(1, 1, 0));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_false_parallel",
          "Plane and triangle do not intersect",
       result.getIntersectionType() 
                == TriangleIntersectionResult::NO_INTERSECTION);
  assert_("test_intersection_triangle_triangle_false_parallel",
          "No intersection segment",
       result.getIntersectionPoints().size() == 0);
}

void test_intersection_triangle_triangle_false_samePlane()
{
  Triangle t1(SimplePoint3d(1, 0, 0),
              SimplePoint3d(1, 0, 0.4),
              SimplePoint3d(1, 0.4, 0));
  Triangle t2(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 0.6, 0),
              SimplePoint3d(1, 0, 0.6));
  
  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_false_samePlane",
          "Plane and triangle do not intersect",
       result.getIntersectionType() 
             == TriangleIntersectionResult::NO_INTERSECTION);
  assert_("test_intersection_triangle_triangle_false_samePlane",
          "No intersection segment",
       result.getIntersectionPoints().size() == 0);
  assert_("test_intersection_triangle_triangle_false_samePlane",
          "planes are equal", almostEqual(t1.getPlane(), t2.getPlane()));
}

void test_intersection_triangle_triangle_point_vertexOnSurface()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 2),
              SimplePoint3d(0.5, 2, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_vertexOnSurface",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_vertexOnSurface",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_vertexOnSurface",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints().at(0)));
}

void test_intersection_triangle_triangle_point_vertexOnEdge()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 1),
              SimplePoint3d(0.5, 1, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_vertexOnEdge",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_vertexOnEdge",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_vertexOnEdge",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints()[0]));
}

void test_intersection_triangle_triangle_point_vertexOnVertex()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 0, 1),
              SimplePoint3d(0.5, 0.5, 0.5));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(1, 0, 0),
              SimplePoint3d(1, 1, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_vertexOnVertex",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_vertexOnVertex",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_vertexOnVertex",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints()[0]));
}

void test_intersection_triangle_triangle_point_vertexOnEdge_samePlane()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 1),
              SimplePoint3d(0.5, 1, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 1, 0.5),
              SimplePoint3d(0.5, 0.5, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_vertexOnEdge_samePlane",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_vertexOnEdge_samePlane",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_vertexOnEdge_samePlane",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints()[0]));
  assert_("test_intersection_triangle_triangle_point_vertexOnEdge_samePlane",
          "planes are equal", almostEqual(t1.getPlane(), t2.getPlane()));
}

void test_intersection_triangle_triangle_point_vertexOnVertex_samePlane()
{
  Triangle t1(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 0, 0.5),
              SimplePoint3d(0.5, 0.5, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 1, 0.5),
              SimplePoint3d(0.5, 0.5, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_vertexOnVertex_samePlane",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_vertexOnVertex_samePlane",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_vertexOnVertex_samePlane",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints()[0]));
  assert_("test_intersection_triangle_triangle_point_vertexOnVertex_samePlane",
          "planes are equal", almostEqual(t1.getPlane(), t2.getPlane()));
}

void test_intersection_triangle_triangle_point_edgeIntersection()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 0, 0),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(1, 1, 1),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(1, 0, 1));

  SimplePoint3d expectedP(0.5, 0.5, 0.5);

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_point_edgeIntersection",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::POINT);
  assert_("test_intersection_triangle_triangle_point_edgeIntersection",
          "Triangles have intersection point",
          result.getIntersectionPoints().size() == 1);
  assert_("test_intersection_triangle_triangle_point_edgeIntersection",
          "Intersection point",
          almostEqual(expectedP,
                      result.getIntersectionPoints()[0]));
}

void test_intersection_triangle_triangle_segment_sharedEdge()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(0, 1, 0));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0, 0, 0));
  expectedP.push_back(SimplePoint3d(1, 1, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_segment_sharedEdge",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_triangle_triangle_segment_sharedEdge",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_triangle_triangle_segment_sharedEdge",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_triangle_triangle_segment_sharedEdge_samePlane()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 0, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0, 0, 0));
  expectedP.push_back(SimplePoint3d(1, 1, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_segment_sharedEdge_samePlane",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_triangle_triangle_segment_sharedEdge_samePlane",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_triangle_triangle_segment_sharedEdge_samePlane",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
  assert_("test_intersection_triangle_triangle_segment_sharedEdge_samePlane",
          "planes are equal", almostEqual(t1.getPlane(), t2.getPlane()));
}

void test_intersection_triangle_triangle_segment_partiallySharedEdge()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0.7, 0.7, 0.7),
              SimplePoint3d(0, 1, 0));
  Triangle t2(SimplePoint3d(0.3, 0.3, 0.3),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0.3, 0.3, 0.3));
  expectedP.push_back(SimplePoint3d(0.7, 0.7, 0.7));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_segment_partiallySharedEdge",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_triangle_triangle_segment_partiallySharedEdge",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_triangle_triangle_segment_partiallySharedEdge",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_triangle_triangle_segment_partiallySharedEdge_samePlane()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 0, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0, 0, 0));
  expectedP.push_back(SimplePoint3d(1, 1, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_tri_tri_segment_partiallySharedEdge_samePlane",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_tri_tri_segment_partiallySharedEdge_samePlane",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_tri_tri_segment_partiallySharedEdge_samePlane",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
  assert_("test_intersection_tri_tri_segment_partiallySharedEdge_samePlane",
          "planes are equal", almostEqual(t1.getPlane(), t2.getPlane()));
}

void test_intersection_triangle_triangle_segment_edgeOnSurface()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 2),
              SimplePoint3d(0.5, 2, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0, 0, 0));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0.5, 0.5, 0.5));
  expectedP.push_back(SimplePoint3d(0.5, 0, 0));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_segment_edgeOnSurface",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_triangle_triangle_segment_edgeOnSurface",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_triangle_triangle_segment_edgeOnSurface",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_triangle_triangle_segment_nonEdge()
{
  Triangle t1(SimplePoint3d(1, 0, 0),
              SimplePoint3d(-1, 0, 2),
              SimplePoint3d(-1, 0, -2));
  Triangle t2(SimplePoint3d(0, 1, 1),
              SimplePoint3d(0, -1, 3),
              SimplePoint3d(0, -1, -1));

  vector<SimplePoint3d> expectedP;
  expectedP.push_back(SimplePoint3d(0, 0, 0));
  expectedP.push_back(SimplePoint3d(0, 0, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_segment_nonEdge",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::SEGMENT);
  assert_("test_intersection_triangle_triangle_segment_nonEdge",
          "Triangles have intersection segment",
          result.getIntersectionPoints().size() == 2);
  assert_("test_intersection_triangle_triangle_segment_nonEdge",
          "Intersection segment",
          almostEqualPoints(expectedP, result.getIntersectionPoints()));
}

void test_intersection_triangle_triangle_area_same()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(3, 3, 3),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(1, 1, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_area_same",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_intersection_triangle_triangle_area_inside()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(1.5, 1.5, 2),
              SimplePoint3d(1.5, 1.5, 2.5),
              SimplePoint3d(2, 2, 2.5));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_area_inside",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_intersection_triangle_triangle_area_inside_edges()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(1, 1, 2),
              SimplePoint3d(2, 2, 2),
              SimplePoint3d(2, 2, 3));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_area_inside_edges",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_intersection_triangle_triangle_area_half()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(2, 2, 2),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(1, 1, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_area",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_intersection_triangle_triangle_area_no_point_inside()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(0.5, 0.5, 2),
              SimplePoint3d(2, 2, 3.5),
              SimplePoint3d(3, 3, 1));

  TriangleIntersectionResult result = intersection(t1, t2);

  assert_("test_intersection_triangle_triangle_area_no_point_inside",
          "Triangles intersect",
          result.getIntersectionType() == TriangleIntersectionResult::AREA);
}

void test_cutTriangle2d()
{
  MultiObjectTriangleContainer container;

  Triangle ta1(SimplePoint3d(-3, 0, 0),
               SimplePoint3d(1, 0, 0),
               SimplePoint3d(-1, 2, 0));
  Triangle ta2(SimplePoint3d(-1, 0, 0),
               SimplePoint3d(3, 0, 0),
               SimplePoint3d(1, 2, 0));

  Triangle tb1(SimplePoint3d(0, 0, 0),
               SimplePoint3d(2, 0, 0),
               SimplePoint3d(0, 2, 0));
  Triangle tb2(SimplePoint3d(1, 0, 0),
               SimplePoint3d(1, 1, 0),
               SimplePoint3d(0, 1, 0));
  
  
  
  container.addTriangle(tb2, 3, true);
  container.addTriangle(tb1, 4, true);
  
  container.test();
}

void test_container_insert_export()
{
  MultiObjectTriangleContainer container;

  Triangle triangle(SimplePoint3d(0, 0, 0),
                    SimplePoint3d(1, 1, 1),
                    SimplePoint3d(1, 0, 0));
  
  container.addTriangle(triangle, 3, false);
  
  vector<Triangle> exported;
  
  container.exportObject(3, exported);
  
  assert_("test_container_insert_export",
          "One triangle exported",
          1 == exported.size());
  assert_("test_container_insert_export",
          "Triangles are identical",
          almostEqual(triangle, exported[0]));
}

void test_container_check_ok()
{
  Triangle t1(SimplePoint3d(2, 0, 0),
              SimplePoint3d(2, 0, 1),
              SimplePoint3d(2, 1, 0));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 0, 0));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_ok",
          "No conflict",
          r1 && r2 && container.noTriangles() == 2);
}

void test_container_check_ok_pointOnEdge()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 1),
              SimplePoint3d(0.5, 1, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 1));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_ok_pointOnEdge",
          "No conflict",
          r1 && r2 && container.noTriangles() == 2);
}

void test_container_check_ok_edge()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(0, 1, 0));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_ok_edge",
          "No conflict",
          r1 && r2 && container.noTriangles() == 2);
}

void test_container_check_ok_two_pie_slices()
{
  // First piece
  // Bottom
  Triangle t1(SimplePoint3d(1, 0, 0),
              SimplePoint3d(1, 1, 0),
              SimplePoint3d(2, 0.5, 0));
  // Top
  Triangle t2(SimplePoint3d(1, 0, 1),
              SimplePoint3d(2, 0.5, 1),
              SimplePoint3d(1, 1, 1));
  // Front
  Triangle t3(SimplePoint3d(1, 0, 0),
              SimplePoint3d(2, 0.5, 0),
              SimplePoint3d(1, 0, 1));
  Triangle t4(SimplePoint3d(1, 0, 1),
              SimplePoint3d(2, 0.5, 0),
              SimplePoint3d(2, 0.5, 1));
  // Back
  Triangle t5(SimplePoint3d(1, 1, 0),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(2, 0.5, 0));
  Triangle t6(SimplePoint3d(1, 1, 1),
              SimplePoint3d(2, 0.5, 1),
              SimplePoint3d(2, 0.5, 0));
  // Leftside
  Triangle t7(SimplePoint3d(1, 0, 0),
              SimplePoint3d(1, 0, 1),
              SimplePoint3d(1, 1, 0));
  Triangle t8(SimplePoint3d(1, 0, 1),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));
  // Second piece
  // Bottom
  Triangle t9(SimplePoint3d(2, 0.5, 0),
              SimplePoint3d(3, 1, 0),
              SimplePoint3d(3, 0, 0));
  // Top
  Triangle t10(SimplePoint3d(2, 0.5, 1),
               SimplePoint3d(3, 0, 1),
               SimplePoint3d(3, 1, 1));
  // Front
  Triangle t11(SimplePoint3d(2, 0.5, 0),
               SimplePoint3d(3, 0, 0),
               SimplePoint3d(3, 0, 1));
  Triangle t12(SimplePoint3d(2, 0.5, 0),
               SimplePoint3d(3, 0, 1),
               SimplePoint3d(2, 0.5, 1));
  // Back
  Triangle t13(SimplePoint3d(2, 0.5, 0),
               SimplePoint3d(3, 1, 1),
               SimplePoint3d(3, 1, 0));
  Triangle t14(SimplePoint3d(2, 0.5, 0),
               SimplePoint3d(2, 0.5, 1),
               SimplePoint3d(3, 1, 1));
// Rightside
  Triangle t15(SimplePoint3d(3, 0, 0),
               SimplePoint3d(3, 1, 0),
               SimplePoint3d(3, 0, 1));
  Triangle t16(SimplePoint3d(3, 0, 1),
               SimplePoint3d(3, 1, 0),
               SimplePoint3d(3, 1, 1));

  MultiObjectTriangleContainer container;
  bool result = true;
  result = result && container.addTriangle(t1, 3, false);
  result = result && container.addTriangle(t2, 3, false);
  result = result && container.addTriangle(t3, 3, false);
  result = result && container.addTriangle(t4, 3, false);
  result = result && container.addTriangle(t5, 3, false);
  result = result && container.addTriangle(t6, 3, false);
  result = result && container.addTriangle(t7, 3, false);
  result = result && container.addTriangle(t8, 3, false);
  result = result && container.addTriangle(t9, 3, false);
  result = result && container.addTriangle(t10, 3, false);
  result = result && container.addTriangle(t11, 3, false);
  result = result && container.addTriangle(t12, 3, false);
  result = result && container.addTriangle(t13, 3, false);
  result = result && container.addTriangle(t14, 3, false);
  result = result && container.addTriangle(t15, 3, false);
  result = result && container.addTriangle(t16, 3, false);
  
  assert_("test_container_check_ok_two_pie_slices",
          "No conflict",
          result && container.noTriangles() == 16);
}

/*
Function test\_container\_check\_fail\_same

*/

void test_container_check_fail_same()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(3, 3, 3),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(1, 1, 1));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_fail_same",
          "Conflict expected",
          r1 && !r2);
  assert_("test_container_check_fail_same",
          "Only one triangle in container",
          container.noTriangles() == 1);
}

void test_container_check_fail_partiallySharedEdge()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0.7, 0.7, 0.7),
              SimplePoint3d(0, 1, 0));
  Triangle t2(SimplePoint3d(0.3, 0.3, 0.3),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_fail_partiallySharedEdge",
          "Conflict expected",
          r1 && !r2);
  assert_("test_container_check_fail_partiallySharedEdge",
          "Only one triangle in container",
          container.noTriangles() == 1);
}

void test_container_check_fail_edgeOnSurface()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 2),
              SimplePoint3d(0.5, 2, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0, 0, 0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_fail_edgeOnSurface",
          "Conflict expected",
          r1 && !r2);
  assert_("test_container_check_fail_edgeOnSurface",
          "Only one triangle in container",
          container.noTriangles() == 1);
}

void test_container_check_fail_overlap()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 0, 1));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, false);
  bool r2 = container.addTriangle(t2, 3, false);
  
  assert_("test_container_check_fail_overlap",
          "Conflict expected",
          r1 && !r2);
  assert_("test_container_check_fail_overlap",
          "Only one triangle in container",
          container.noTriangles() == 1);
}

void test_container_correct_same()
{
  Triangle t1(SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(3, 3, 3));
  Triangle t2(SimplePoint3d(3, 3, 3),
              SimplePoint3d(1, 1, 3),
              SimplePoint3d(1, 1, 1));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_correct_same",
          "Conflict resolved",
          r1 && r2);
  assert_("test_container_correct_same",
          "Only one triangle in container",
          container.noTriangles() == 1);
}

void test_container_correct_partiallySharedEdge()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0.7, 0.7, 0.7),
              SimplePoint3d(0, 1, 0));
  Triangle t2(SimplePoint3d(0.3, 0.3, 0.3),
              SimplePoint3d(1, 1, 1),
              SimplePoint3d(1, 1, 0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_correct_partiallySharedEdge",
          "Conflict resolved",
          r1 && r2);
  assert_("test_container_correct_partiallySharedEdge",
          "Four triangles in container",
          container.noTriangles() == 4);
  
}

void test_container_correct_edgeOnSurface()
{
  Triangle t1(SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0.5, 0, 2),
              SimplePoint3d(0.5, 2, 0));
  Triangle t2(SimplePoint3d(0.5, 0.5, 0.5),
              SimplePoint3d(0.5, 0, 0),
              SimplePoint3d(0, 0, 0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_correct_edgeOnSurface",
          "Conflict resolved",
          r1 && r2);
  assert_("test_container_check_fail_edgeOnSurface",
          "Four triangles in container",
          container.noTriangles() == 4);
}

void test_container_correct_overlap()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 0, 1));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_correct_overlap",
          "Conflict resolved",
          r1 && r2);
  assert_("test_container_correct_overlap",
          "Three triangles in container",
          container.noTriangles() == 3);
}

void test_container_prepare_set_surface_simple()
{
  Triangle t1(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 1, 1));
  Triangle t2(SimplePoint3d(0, 0, 0),
              SimplePoint3d(0, 1, 0),
              SimplePoint3d(0, 0, 1));

  vector<Triangle> only1, only2, both;

  Triangle expected_only1(SimplePoint3d(0,1,1),
                          SimplePoint3d(0,0.5,0.5),
                          SimplePoint3d(0,1,0));
  
  Triangle expected_only2(SimplePoint3d(0,0,0),
                          SimplePoint3d(0,0.5,0.5),
                          SimplePoint3d(0,0,1));

  Triangle expected_both(SimplePoint3d(0,1,0),
                         SimplePoint3d(0,0.5,0.5),
                         SimplePoint3d(0,0,0));

  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 1, true);
  bool r2 = container.addTriangle(t2, 2, true);
  
  container.prepareSetOperationSurface(1, 2, 3, 4, 5);
  container.exportObject(3, both);
  container.exportObject(4, only1);
  container.exportObject(5, only2);
  
  assert_("test_container_prepare_set_surface_simple",
          "Single triangle only in object 1",
          only1.size() == 1 && almostEqual(expected_only1, only1[0]));
  assert_("test_container_prepare_set_surface_simple",
          "Single triangle only in object 2",
          only2.size() == 1 && almostEqual(expected_only2, only2[0]));
  assert_("test_container_prepare_set_surface_simple",
          "Single triangle in both objects",
          both.size() == 1 && almostEqual(expected_both, both[0]));
}

void test_container_prepare_set_volume_tetris()
{
  // Operlap of
  //
  // XXXX  and  YYYY  like this:   ####
  // XX           YY               XXYY
  
  MultiObjectTriangleContainer container;
  
  // Bottom
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 0 ),
                                 SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(0, 0, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(1, 1, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 0 ),
                                 SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(0, 1, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(1, 2, 0 )),
                        1, true);
  // Top
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 1 ),
                                 SimplePoint3d(0, 0, 1 ),
                                 SimplePoint3d(0, 1, 1 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(1, 1, 1 ),
                                 SimplePoint3d(1, 2, 1 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 1 ),
                                 SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        1, true);
  // Left
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 0 ),
                                 SimplePoint3d(-1, 0, 1 ),
                                 SimplePoint3d(-1, 2, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(-1, 0, 1 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        1, true);
  // rightmiddle
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(0, 0, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(0, 0, 1 )),
                        1, true);
  // right
  container.addTriangle(Triangle(SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(1, 1, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 1, 0 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(1, 1, 1 )),
                        1, true);
  // back
  container.addTriangle(Triangle(SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(-1, 2, 1 ),
                                 SimplePoint3d(1, 2, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(-1, 2, 1 ),
                                 SimplePoint3d(1, 2, 1 )),
                        1, true);
  // frontleft
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 0 ),
                                 SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(-1, 0, 1 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 0, 1 ),
                                 SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(0, 0, 1 )),
                        1, true);
  // frontright
  container.addTriangle(Triangle(SimplePoint3d(1, 1, 0 ),
                                 SimplePoint3d(1, 1, 1 ),
                                 SimplePoint3d(0, 1, 0 )),
                        1, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(1, 1, 1 ),
                                 SimplePoint3d(0, 1, 1 )),
                        1, true);

  // Rechte Tetris-Figur
  // Bottom
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 0 ),
                                 SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(0, 1, 0 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(-1, 1, 0 ),
                                 SimplePoint3d(-1, 2, 0 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 0 ),
                                 SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(1, 2, 0 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(1, 2, 0 )),
                        2, true);
  // Top
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 1 ),
                                 SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(0, 0, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(-1, 2, 1 ),
                                 SimplePoint3d(-1, 1, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 1 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(0, 1, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        2, true);
  // right
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 0 ),
                                 SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(1, 0, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(1, 2, 1 ),
                                 SimplePoint3d(1, 0, 1 )),
                        2, true);
  // leftmiddle
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(0, 1, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 0, 0 ),
                                 SimplePoint3d(0, 0, 1 ),
                                 SimplePoint3d(0, 1, 1 )),
                        2, true);
  // right
  container.addTriangle(Triangle(SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(-1, 1, 0 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 1, 0 ),
                                 SimplePoint3d(-1, 1, 1 ),
                                 SimplePoint3d(-1, 2, 1 )),
                        2, true);
  // back
  container.addTriangle(Triangle(SimplePoint3d(1, 2, 0 ),
                                 SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(1, 2, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(-1, 2, 0 ),
                                 SimplePoint3d(-1, 2, 1 ),
                                 SimplePoint3d(1, 2, 1 )),
                        2, true);
  // frontright
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 0 ),
                                 SimplePoint3d(1, 0, 1 ),
                                 SimplePoint3d(0, 0, 0 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(1, 0, 1 ),
                                 SimplePoint3d(0, 0, 1 ),
                                 SimplePoint3d(0, 0, 0 )),
                        2, true);
  // frontleft
  container.addTriangle(Triangle(SimplePoint3d(-1, 1, 0 ),
                                 SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(-1, 1, 1 )),
                        2, true);
  container.addTriangle(Triangle(SimplePoint3d(0, 1, 0 ),
                                 SimplePoint3d(0, 1, 1 ),
                                 SimplePoint3d(-1, 1, 1 )),
                        2, true);

  assert_("test_container_prepare_set_volume_tetris",
          "Volume 1 correct",
          container.checkVolume(1, false));
  assert_("test_container_prepare_set_volume_tetris",
          "Volume 2 correct",
          container.checkVolume(2, false));

  container.prepareSetOperationVolume(1, 2, 3, 4, 5, 6, 7, 8);
  
  vector<Triangle> common_same, common_opposite, only1_inside2, only2_inside1,
                   only1_outside2, only2_outside1;
  
  container.exportObject(3, common_same);
  container.exportObject(4, common_opposite);
  container.exportObject(5, only1_outside2);
  container.exportObject(6, only2_outside1);
  container.exportObject(7, only1_inside2);
  container.exportObject(8, only2_inside1);
  
//  cerr << "common_same:     " << common_same.size()              << " "
//                              << getBoundingBox(common_same)     << endl;
//  cerr << "common_opposite: " << common_opposite.size()          << " "
//                              << getBoundingBox(common_opposite) << endl;
//  cerr << "only1_outside2:  " << only1_outside2.size()           << " "
//                              << getBoundingBox(only1_outside2)  << endl;
//  cerr << "only2_outside1:  " << only2_outside1.size()           << " "
//                              << getBoundingBox(only2_outside1)  << endl;
//  cerr << "only1_inside2:   " << only1_inside2.size()            << " "
//                              << getBoundingBox(only1_inside2)   << endl;
//  cerr << "only2_inside1:   " << only2_inside1.size()            << " "
//                              << getBoundingBox(only2_inside1)   << endl;

  assert_("test_container_prepare_set_volume_tetris",
          "common_same not empty",
          common_same.size() > 0);
  assert_("test_container_prepare_set_volume_tetris",
          "common_opposite not empty",
          common_opposite.size() > 0);
  assert_("test_container_prepare_set_volume_tetris",
          "only1_outside2 not empty",
          only1_outside2.size() > 0);
  assert_("test_container_prepare_set_volume_tetris",
          "only2_outside1 not empty",
          only2_outside1.size() > 0);
  assert_("test_container_prepare_set_volume_tetris",
          "only1_inside2 not empty",
          only1_inside2.size() > 0);
  assert_("test_container_prepare_set_volume_tetris",
          "only2_inside1 not empty",
          only2_inside1.size() > 0);
}

void test_container_crash_1_ok()
{
  Triangle t1(SimplePoint3d(0.596958,-0.0737988,0.502047),
              SimplePoint3d(0.59689,-0.0737821,0.502019),
              SimplePoint3d(0.605682,-0.0759491,0.505594));
  Triangle t2(SimplePoint3d(0.596999,-0.0737792,0.502193),
              SimplePoint3d(0.596958,-0.0737988,0.502047),
              SimplePoint3d(0.605682,-0.0759491,0.505594));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 3, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_correct_overlap",
          "Conflict resolved",
          r1 && r2);
}

void test_container_crash_2_ok()
{
  Triangle t1(SimplePoint3d(0,0,0),
              SimplePoint3d(0,2,0),
              SimplePoint3d(0,0,2));
  Triangle t2(SimplePoint3d(0,1,1),
              SimplePoint3d(0,1,0),
              SimplePoint3d(0,0,1));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  assert_("test_container_crash_2",
          "Conflict resolved",
          r1 && r2);
}

void test_container_crash_3_ok()
{
  Triangle t1(SimplePoint3d(0,1,1),
              SimplePoint3d(0,1,0),
              SimplePoint3d(0,0,1));
  Triangle t2(SimplePoint3d(0,0,0),
              SimplePoint3d(0,2,0),
              SimplePoint3d(0,0,2));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_crash_3",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_4()
{
  // Aus: loubie_aria_dragon.stl (1)
  
  Triangle t1(
  SimplePoint3d(-0.0019389517822830905,-17.435987038932389,-55.522003173828125),
  SimplePoint3d(-0.0020000000949949026,-17.436000823974609,-55.522003173828125),
  SimplePoint3d(0.02200000174343586,-17.432001113891602,-55.522003173828125)
  );
  
  Triangle t2(
  SimplePoint3d(-0.0019389517822830905,-17.435987038932389,-55.522003173828125),
  SimplePoint3d(-0.0020000000949949026,-17.43800163269043,-55.522003173828125),
  SimplePoint3d(-0.0020000000949949026,-17.436000823974609,-55.522003173828125)
  );
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_4",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_5()
{
  // Aus: dungeonskeeper_devil.stl (4)
  Triangle t1(
    SimplePoint3d(137.99419872755371,-284.2075813005664,221.5294817110792),
    SimplePoint3d(137.87608774223804,-283.32140466532195,221.56437072086595),
    SimplePoint3d(137.99506789716889,-284.20657072935865,221.52923931758463));
  Triangle t2(
    SimplePoint3d(137.87608774223804,-283.32140466532195,221.56437072086595),
    SimplePoint3d(137.79269409179688,-282.69570922851562,221.58900451660156),
    SimplePoint3d(137.99506789716889,-284.20657072935865,221.52923931758463));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_crash_5",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_6()
{
  // Aus: DEVIL_96k.stl (13)
  Triangle t1(
    SimplePoint3d(83.251032091991007,14.571250736003563,76.797335229474953),
    SimplePoint3d(83.251029085850931,14.571244975802594,76.797342157553643),
    SimplePoint3d(83.023323059082031,14.77241325378418,76.9676513671875));
  Triangle t2(
    SimplePoint3d(83.387825012207031,14.593289375305176,76.615570068359375),
    SimplePoint3d(83.251032091991007,14.571250736003563,76.797335229474953),
    SimplePoint3d(83.023323059082031,14.77241325378418,76.9676513671875));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_6",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_7()
{
  // Aus: Dragon_Head_In_STL_for_Insense_v0.6.stl (14)
  Triangle t1(
    SimplePoint3d(14.593143463134766,-16.18879508972168,97.269149780273438),
    SimplePoint3d(14.856311798095703,-16.188919067382812,97.087242126464844),
    SimplePoint3d(14.856310949323289,-16.188919941294042,97.087256863767536));
  Triangle t2(
    SimplePoint3d(14.856310949323289,-16.188919941294042,97.087256863767536),
    SimplePoint3d(14.856311798095703,-16.188919067382812,97.087242126464844),
    SimplePoint3d(14.84428596496582,-16.211263656616211,97.457290649414062));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_7",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_7b()
{
  // Aus: Dragon_Head_In_STL_for_Insense_v0.6.stl (14)
  
  // Dieser Testfall setzt früher an, für aber zu test_container_numeric_7.
  
  Triangle t1(
    SimplePoint3d(14.593143463134766,-16.18879508972168,97.269149780273438),
    SimplePoint3d(14.856311798095703,-16.188919067382812,97.087242126464844),
    SimplePoint3d(14.84428596496582,-16.211263656616211,97.457290649414062));
  Triangle t2(
    SimplePoint3d(14.543923377990723,-16.265754699707031,96.957420349121094),
    SimplePoint3d(14.817930221557617,-16.228437423706055,96.718681335449219),
    SimplePoint3d(14.856311798095703,-16.188919067382812,97.087265014648438));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_7b",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_8()
{
  // Aus: devil_Highres.stl (15)

  // Auch bei diesem Test könnte früher angesetzt werden, da es sich nicht um
  // Dreiecke der ursprünglichen Eingabe handelt. Es lässt sich dann jedoch
  // nicht auf zwei Dreiecke reduzieren.

  Triangle t1(
    SimplePoint3d(-3.8870480060577393,35.440589904785156,1.1701380014419556),
    SimplePoint3d(-0.6039568570707563,36.939367076826215,0.73508382846507603),
    SimplePoint3d(1.3227089643478394,37.818958282470703,0.47980779409408569));
  Triangle t2(
    SimplePoint3d(-1.9184390306472778,36.418651580810547,0.94085437059402466),
    SimplePoint3d(-0.6039568570707563,36.939367076826215,0.73508382846507603),
    SimplePoint3d(0.59919282032245613,37.419324473107125,0.5509744950053318));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_8",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_9()
{
  // Aus: polysoup.stl (16)
  Triangle t1(
    SimplePoint3d(-3.5274658203125,-1.1040725708007812,8.7499980926513672),
    SimplePoint3d(-3.7707037925720215,-0.82064360380172729,8.4703750610351562),
    SimplePoint3d(-3.8896229267120361,-0.85701596736907959,8.0707912445068359));
  Triangle t2(
    SimplePoint3d(-3.8896229267120361,-0.85701596736907959,8.0707912445068359),
    SimplePoint3d(-3.8974847793579102,-0.83505535125732422,8.0809869766235352),
    SimplePoint3d(-5.3856468200683594,-0.22082901000976562,4.6874980926513672));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_9",
          "Conflict resolved",
          r1 && r2);
}

void test_container_numeric_10()
{
  // Aus: devilHighres_fixed.stl (17)

  // Auch bei diesem Test könnte früher angesetzt werden, da es sich nicht um
  // Dreiecke der ursprünglichen Eingabe handelt. Es lässt sich dann jedoch
  // nicht auf zwei Dreiecke reduzieren.
  
  Triangle t1(
    SimplePoint3d(-0.58741863306131581,36.94622482895187,0.73188733902666436),
    SimplePoint3d(-0.58757839413312007,36.946177129441274,0.73192935689591421),
    SimplePoint3d(0.39200001955032349,37.320003509521484,0.54200005531311035));
  Triangle t2(
    SimplePoint3d(-0.58799999952316284,36.946002960205078,0.73200005292892456),
    SimplePoint3d(-0.58757839413312007,36.946177129441274,0.73192935689591421),
    SimplePoint3d(-0.58741863306131581,36.94622482895187,0.73188733902666436));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_numeric_10",
          "Conflict resolved",
          r1 && r2);
}

void test_container_crash_XXX()
{
  // Aus: XXX (XXX)
  Triangle t1(
    SimplePoint3d(000,000,000),
    SimplePoint3d(000,000,000),
    SimplePoint3d(000,000,000));
  Triangle t2(
    SimplePoint3d(000,000,000),
    SimplePoint3d(000,000,000),
    SimplePoint3d(000,000,000));
  
  MultiObjectTriangleContainer container;
  bool r1 = container.addTriangle(t1, 2, true);
  bool r2 = container.addTriangle(t2, 3, true);
  
  container.test();
  
  assert_("test_container_crash_XXX",
          "Conflict resolved",
          r1 && r2);
}


void run_tests()
{

  // Keine richtigen Tests:
  // test_cutTriangle2d();

  // 2D
  
  test_getPolarAngle();

  test_intersects_2d_true();
  test_intersects_2d_false();
  test_intersects_2d_true_1_endpoint();
  test_intersects_2d_true_2_endpoints();
  test_intersects_2d_false_parallel();
  test_intersects_2d_same();
  test_intersects_2d_subset();
  test_intersects_2d_overlap();

  test_pointInsideTriangle_true();
  test_pointInsideTriangle_false();
  test_pointInsideTriangle_edge();
  test_pointInsideTriangle_vertex();

  test_pointInsideSegment_true();
  test_pointInsideSegment_false();
  test_pointInsideSegment_endpoint();

  test_firstPointInsideTriangle_inside();
  test_firstPointInsideTriangle_edge();
  test_firstPointInsideTriangle_vertex();
  test_firstPointInsideTriangle_outside_edge();
  test_firstPointInsideTriangle_outside_vertex();

  test_intersection2d_false();
  test_intersection2d_point_corner_middle();
  test_intersection2d_point_corner_end();
  test_intersection2d_point_edge_end();
  test_intersection2d_segment_edge();
  test_intersection2d_segment_inner();
  test_intersection2d_segment_cut();

  // 3D
  
  test_point_almostEqual_true();
  test_point_almostEqual_false();
  test_vector_almostEqual_true();
  test_vector_almostEqual_false();
  test_plane_almostEqual_true();
  test_collinear_true();
  test_collinear_true_same_points();
  test_collinear_false();
  test_distance_point_zero();
  test_distance_point_nonzero();
  test_distance_line_zero();
  test_distance_line_nonzero();
  test_length_zero();
  test_length_nonzero();
  test_orthogonal_true();
  test_orthogonal_false();
  test_scalar_multiplication();
  test_multiplication();
  test_crossProduct();
  test_normal_vector_1();
  test_normal_vector_2();
  test_distance_plane_zero();
  test_distance_plane_nonzero();
  test_isPointInPlane_true();
  test_isPointInPlane_false();
  test_planeDistanceToOrigin_zero();
  test_planeDistanceToOrigin_nonzero();
  test_planeHessianNormalForm();
  test_projectPointOntoPlane();
  test_isValidTriangle_true();
  test_isValidTriangle_false_same_point_twice();
  test_isValidTriangle_false_points_collinear();
  
  test_intersection_segmentRayLine_plane_true();
  test_intersection_segmentRayLine_plane_true_onPlane();
  test_intersection_segmentRayLine_plane_false_parallel();
  test_intersection_segmentRayLine_plane_OnlyLine();
  test_intersection_segmentRayLine_plane_OnlyLineRay();
  test_intersection_segmentRayLine_triangle_true(); 
  test_intersection_segmentRayLine_triangle_true_edge();
  test_intersection_segmentRayLine_triangle_true_corner();
  test_intersection_segmentRayLine_triangle_true_onPlane_segment();
  test_intersection_segmentRayLine_triangle_true_onPlane_edge();
  test_intersection_segmentRayLine_triangle_true_onPlane_partialEdge();
  test_intersection_segmentRayLine_triangle_true_onPlane_corner();
  test_intersection_segmentRayLine_triangle_false_onPlane();
  // Die folgenden Tests testen Funktionalität, die aktuell nicht benötigt
  // wird. Bei Bedarf fehlt einfach noch die Implementierung. (Jens Breit)
  //test_intersection_segmentRayLine_triangle_OnlyLine_onPlane();
  //test_intersection_segmentRayLine_triangle_OnlyLineRay_onPlane();
  test_intersection_segmentRayLine_triangle_false_parallel();
  test_intersection_segmentRayLine_triangle_OnlyLine();
  test_intersection_segmentRayLine_triangle_OnlyLineRay();
  test_intersection_plane_triangle_false();
  test_intersection_plane_triangle_false_parallel();
  test_intersection_plane_triangle_true();
  test_intersection_plane_triangle_true_parallel();
  test_intersection_plane_triangle_true_single_vertex();
  test_intersection_plane_triangle_true_edge();
  test_intersection_triangle_triangle_false();
  test_intersection_triangle_triangle_false_parallel();
  test_intersection_triangle_triangle_false_samePlane();
  test_intersection_triangle_triangle_point_vertexOnSurface();
  test_intersection_triangle_triangle_point_vertexOnEdge();
  test_intersection_triangle_triangle_point_vertexOnVertex();
  test_intersection_triangle_triangle_point_vertexOnEdge_samePlane();
  test_intersection_triangle_triangle_point_vertexOnVertex_samePlane();
  test_intersection_triangle_triangle_point_edgeIntersection();
  test_intersection_triangle_triangle_segment_sharedEdge();
  test_intersection_triangle_triangle_segment_sharedEdge_samePlane();
  test_intersection_triangle_triangle_segment_partiallySharedEdge();
  test_intersection_triangle_triangle_segment_partiallySharedEdge_samePlane();
  test_intersection_triangle_triangle_segment_edgeOnSurface();
  test_intersection_triangle_triangle_segment_nonEdge();
  test_intersection_triangle_triangle_area_same();
  test_intersection_triangle_triangle_area_inside();
  test_intersection_triangle_triangle_area_inside_edges();
  test_intersection_triangle_triangle_area_half();
  test_intersection_triangle_triangle_area_no_point_inside();
  
  test_container_insert_export();
  test_container_check_ok();
  test_container_check_ok_pointOnEdge();
  test_container_check_ok_edge();
  test_container_check_ok_two_pie_slices();
  test_container_check_fail_same();
  test_container_check_fail_partiallySharedEdge();
  test_container_check_fail_edgeOnSurface();
  test_container_check_fail_overlap();
  test_container_correct_same();
  test_container_correct_partiallySharedEdge();
  test_container_correct_edgeOnSurface();
  test_container_correct_overlap();

  test_container_prepare_set_surface_simple();
  test_container_prepare_set_volume_tetris();

  test_container_crash_1_ok();
  test_container_crash_2_ok();
  test_container_crash_3_ok();
  test_container_numeric_4();
  test_container_numeric_5();
  test_container_numeric_6();
  test_container_numeric_7();
  test_container_numeric_7b();
  test_container_numeric_8();
  test_container_numeric_9();
  test_container_numeric_10();
  
  // */
}

  ListExpr
  Spatial3dTestMap( ListExpr args )
  {
    return ( nl->SymbolAtom( CcInt::BasicType() ));
  }

  int
  Spatial3dTest( Word* args, Word& result, int message,
                 Word& local, Supplier s )
  {
    result = qp->ResultStorage( s );
    CcInt* res = static_cast<CcInt*>(result.addr);

    number_of_tests_run = 0;
    number_of_tests_failed = 0;
  
    run_tests();
  
    std::cerr << number_of_tests_run << " tests run, "
       << number_of_tests_failed << " tests failed." << std::endl;

    res->Set(true, number_of_tests_failed);
    return 0;
  }

  OperatorSpec testSpec(
    "--> int",
    "test()",
    "Computes the number of triangles of a 3D object (surface3d or volume3d).",
    "query size([const surface3d value (((0 0 0) (0 1 0) (1 1 0)) "
      "((1 1 0) (0 1 0) (1 1 2)))])"
  );

  
  Operator* getTestPtr(){
    return new Operator(
    "test",
    testSpec.getStr(),
    Spatial3dTest,
    Operator::SimpleSelect,
    Spatial3dTestMap
    );
  }
}
