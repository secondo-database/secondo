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



#ifdef GEOMETRIC_TEST

#include<iostream>
#include<stdlib.h>
#include<math.h>

#include "AuxiliaryTypes.h"
#include "geometric_algorithm.h"

using namespace spatial3d_geometric;

int number_of_tests_run;
int number_of_tests_failed;

void assert(std::string message, bool success)
{
  ++number_of_tests_run;
  if (! success)
  {
    ++number_of_tests_failed;
    std::cerr << "Test failed: " << message << std::endl;
  }
}

void test_point_almostEqual_true()
{
  SimplePoint3d p1(2, 3.5, -9);
  SimplePoint3d p2(2, 3.5, -9);

  assert("Equal points are almost equal", almostEqual(p1, p2));
}

void test_point_almostEqual_false()
{
  SimplePoint3d p1(2, 3.5, -9);
  SimplePoint3d p2(2, 3.4, -9);

  assert("Different points are not almost equal", ! almostEqual(p1, p2));
}

void test_vector_almostEqual_true()
{
  Vector3d p1(2, 3.5, -9);
  Vector3d p2(2, 3.5, -9);

  assert("Equal vectors are almost equal", almostEqual(p1, p2));
}

void test_vector_almostEqual_false()
{
  Vector3d p1(2, 3.5, -9);
  Vector3d p2(2, 3.4, -9);

  assert("Different vectors are not almost equal", ! almostEqual(p1, p2));
}

void test_collinear_true()
{
  SimplePoint3d p1(0, 1, 0);
  SimplePoint3d p2(0 -2, 1 -2, 0 -2);
  SimplePoint3d p3(0 -3, 1 -3, 0 -3);
  
  assert("Points are collinear", collinear(p1, p2, p3));
}

void test_collinear_true_same_points()
{
  SimplePoint3d p1(1, 2, 3);
  SimplePoint3d p2(1, 2, 3);
  SimplePoint3d p3(1, 2, 3);

  assert("Same points are collinear", collinear(p1, p2, p3));
}

void test_collinear_false()
{
  SimplePoint3d p1(0, 0, 0);
  SimplePoint3d p2(0, 0, 1);
  SimplePoint3d p3(0, 1, 1);
  
  assert("Points not are collinear", ! collinear(p1, p2, p3));
}

void test_distance_point_zero()
{
  SimplePoint3d p1(1,2,3);
  SimplePoint3d p2(1,2,3);

  double d = distance(p1, p2);
  assert("Distance point to point is zero", AlmostEqual(0, d));  
}

void test_distance_point_nonzero()
{
  SimplePoint3d p1(1,2,3);
  SimplePoint3d p2(1,3,2);

  double d = distance(p1, p2);
  assert("Distance point to point is nonzero", AlmostEqual(sqrt(2), d));  
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
  assert("Distance point to line is zero", AlmostEqual(0, d)); 
}

void test_distance_line_nonzero()
{
  SimplePoint3d linePoint1(0,0,0);
  SimplePoint3d linePoint2(0,1,1);

  {
    SimplePoint3d distantPoint(0,0,1);
    double d = distancePointToLine(distantPoint, linePoint1, linePoint2);
    assert("Distance point to line nonzero 1", AlmostEqual(sqrt(2) / 2.0, d));
  }
  {
    SimplePoint3d distantPoint(1,1,1);
    double d = distancePointToLine(distantPoint, linePoint1, linePoint2);
    assert("Distance point to line nonzero 2", AlmostEqual(1, d));
  }    
}

void test_length_zero()
{
  Vector3d v(0, 0, 0);
  assert("Vector length zero", AlmostEqual(0, length(v)));
}

void test_length_nonzero()
{
  Vector3d v(1, 1, 1);
  assert("Vector length nonzero", AlmostEqual(sqrt(3), length(v)));
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
  assert("Cross product incorrect", almostEqual(expected, actual));
}

void test_normal_vector_1()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(-2, 7, 2);
  
  Vector3d actual = normalVector(a, b, c);
  Vector3d expected = (1 / sqrt(2842)) * Vector3d(-36, -39, 5);
  assert("Normal vector 1 incorrect", almostEqual(expected, actual));
}

void test_normal_vector_2()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);
  
  Vector3d actual = normalVector(k, l, m);
  Vector3d expected = (1 / sqrt(16401)) * Vector3d(-46, -118, 19);
  assert("Normal vector 2 incorrect", almostEqual(expected, actual));
}

void test_distance_plane_zero()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);

  SimplePoint3d p(4 + 2.5 * (-6 - 5), 1 + 2.5 * (2 + 1), 9 + 2.5 * (-9 + 1));
  
  Plane3d plane(k, l, m);
  double dist = distance(p, plane);
  assert("Distance to plane zero", AlmostEqual(0, dist));
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
  assert("Point is in plane", isPointInPlane(p, plane));
}

void test_isPointInPlane_false()
{
  SimplePoint3d k(4, 1, 9);
  SimplePoint3d l(-6, 2, -9);
  SimplePoint3d m(5, -1, -1);

  SimplePoint3d p(4 + 1.5 * (-6 - 5), 1 + 2.5 * (2 + 1), 9 + 3.5 * (-9 + 1));
  
  Plane3d plane(k, l, m);
  assert("Point is not in plane", ! isPointInPlane(p, plane));
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

void test_isValidTriangle_true()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(-2, 7, 2);

  assert("Valid triangle", isValidTriangle(a, b, c));
}

void test_isValidTriangle_false_same_point_twice()
{
  SimplePoint3d a(3, 2, -1);
  SimplePoint3d b(6, 0, 5);
  SimplePoint3d c(6, 0, 5);

  assert("Not a valid triangle if two points are identical",
         ! isValidTriangle(a, b, c));
}

void test_isValidTriangle_false_points_collinear()
{
  SimplePoint3d a(0, 1, 0);
  SimplePoint3d b(0 -2, 1 -2, 0 -2);
  SimplePoint3d c(0 -3, 1 -3, 0 -3);
  
  assert("Not a valid triangle if points are collinear",
         ! isValidTriangle(a, b, c));
}

void run_tests()
{
  test_point_almostEqual_true();
  test_point_almostEqual_false();
  test_vector_almostEqual_true();
  test_vector_almostEqual_false();
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
  test_isValidTriangle_true();
  test_isValidTriangle_false_same_point_twice();
  test_isValidTriangle_false_points_collinear();
}

int main()
{
  number_of_tests_run = 0;
  number_of_tests_failed = 0;
  
  run_tests();
  
  std::cerr << number_of_tests_run << " tests run, "
       << number_of_tests_failed << " tests failed." << std::endl;
  
  exit(number_of_tests_failed);
}

bool AlmostEqual(double const& d1, double const& d2)
{
  const double eps = 0.00000001;
  return d1 - d2 < eps && d1 - d2 > -eps;
}

#endif