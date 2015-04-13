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
#include<iostream>

namespace spatial3d_geometric
{
    /* TODO: Remove debugging helpers (Jens Breit) */
  
  void print(SimplePoint3d a)
  {
    std::cerr << "SimplePoint3d: (" << a.getX() << ","
                                    << a.getY() << ","
                                    << a.getZ() << ")";
  }
  
  void print(Vector3d a)
  {
    std::cerr << "Vector3d: (" << a.getX() << ","
                               << a.getY() << ","
                               << a.getZ() << ")";
  }
  
  void print(SimplePoint2d a, string name)
  {
    std::cerr << "Point2d \"" << name << "\": (" << a.getX() << ","
                                                 << a.getY() << ")" << endl;
  }

  void print(Triangle a)
  {
    std::cerr << "Triangle(";
    print(a.getA());
    print(a.getB());
    print(a.getC());
    std::cerr << ")" << endl;
  }
  
  void numeric_fail()
  {
    //assert(false);
    throw NumericFailure();
  }
  
  /* Helper functions */
  
  bool AlmostLte(double a, double b)
  {
    if (AlmostEqual(a, b))
      return true;
    return a < b;
  }
  

  /* Points */

  const SimplePoint3d origin(0,0,0);

  bool almostEqual(const SimplePoint3d& p1, const SimplePoint3d& p2)
  {
    return AlmostEqual(p1.getX(), p2.getX())
        && AlmostEqual(p1.getY(), p2.getY())
        && AlmostEqual(p1.getZ(), p2.getZ());
  }
  
  bool collinear(const SimplePoint3d& pA,
                 const SimplePoint3d& pB,
                 const SimplePoint3d& pC)
  {
    if (almostEqual(pB, pC))
      return true;
    
    // compute distance of pA to the line through pB und pC
    return AlmostEqual(0, distancePointToLine(pA, pB, pC));
  }

  double distance(const SimplePoint3d& p1, const SimplePoint3d& p2)
  {
    return length(Vector3d(p1, p2));
  }

  double distancePointToLine(const SimplePoint3d& distantPoint,
           const SimplePoint3d& linePoint1, const SimplePoint3d& linePoint2)
  {
    Vector3d v(origin, distantPoint);
    Vector3d v1(origin, linePoint1);
    Vector3d v2(origin, linePoint2);
    
    return length(crossProduct(v - v1, v - v2))
         / length(v2 - v1);
  }

  /* Vectors */

  bool almostEqual(const Vector3d& p1, const Vector3d& p2)
  {
    return AlmostEqual(p1.getX(), p2.getX())
        && AlmostEqual(p1.getY(), p2.getY())
        && AlmostEqual(p1.getZ(), p2.getZ());
  }
  
  double length(const Vector3d& vector)
  {
    return sqrt(vector * vector);
  }
  
  bool collinear(const Vector3d& v1, const Vector3d& v2)
  {
    return collinear(origin, origin + v1, origin + v2);
    // TODO: Alternative: Kreuzprodukt = 0
    // oder inneres Produkt = Quadrat der Länge
  }

  bool orthogonal(const Vector3d& v1, const Vector3d& v2)
  {
    return AlmostEqual(v1 * v2, 0);
  }

  Vector3d operator+(const Vector3d& v1, const Vector3d& v2)
  {
    return Vector3d(v1.getX() + v2.getX(),
                    v1.getY() + v2.getY(),
                    v1.getZ() + v2.getZ());
  }

  Vector3d operator-(const Vector3d& v1, const Vector3d& v2)
  {
    return v1 + (-1 * v2);
  }

  Vector3d operator*(double scalar, const Vector3d& vector)
  {
    return Vector3d(scalar * vector.getX(),
                    scalar * vector.getY(),
                    scalar * vector.getZ());
  }
  
  double operator*(const Vector3d& v1, const Vector3d& v2)
  {
    return v1.getX() * v2.getX()
         + v1.getY() * v2.getY()
         + v1.getZ() * v2.getZ();
  }

  Vector3d crossProduct(const Vector3d& v1, const Vector3d& v2)
  {
    return Vector3d(v1.getY() * v2.getZ() - v1.getZ() * v2.getY(),
                    v1.getZ() * v2.getX() - v1.getX() * v2.getZ(),
                    v1.getX() * v2.getY() - v1.getY() * v2.getX());
  }
  
  SimplePoint3d operator+(const SimplePoint3d& p, const Vector3d& v)
  {
    return SimplePoint3d(p.getX() + v.getX(),
                         p.getY() + v.getY(),
                         p.getZ() + v.getZ());
  }

  /* Planes */

  bool almostEqual(const Plane3d& p1, const Plane3d& p2)
  {
    return AlmostEqual(p1.getDistanceToOrigin(), p2.getDistanceToOrigin())
        && almostEqual(p1.getNormalVector(), p2.getNormalVector());
  }

  Vector3d normalVector(const SimplePoint3d& pA,
                        const SimplePoint3d& pB,
                        const SimplePoint3d& pC)
  {
    double ax = pA.getX(), ay = pA.getY(), az = pA.getZ();
    double bx = pB.getX(), by = pB.getY(), bz = pB.getZ();
    double cx = pC.getX(), cy = pC.getY(), cz = pC.getZ();
    
    double nx = (by-ay)*(cz-az)-(bz-az)*(cy-ay);
    double ny = (bz-az)*(cx-ax)-(bx-ax)*(cz-az);
    double nz = (bx-ax)*(cy-ay)-(by-ay)*(cx-ax);

    Vector3d direction = Vector3d(nx, ny, nz);
    
    return (1 / length(direction)) * direction;
  }

  double distance(const SimplePoint3d& point, const Plane3d& plane)
  {
    Vector3d pv = Vector3d(origin, point);
    Vector3d nv = plane.getNormalVector();
    double d = plane.getDistanceToOrigin();
    
    return pv * nv - d;
  }

  bool isPointInPlane(const SimplePoint3d& point, const Plane3d& plane)
  {
    return AlmostEqual(distance(point, plane), 0);
  }

  double planeDistanceToOrigin(const SimplePoint3d& pointInPlane,
                               const Vector3d& normalVector)
  {
    return normalVector * Vector3d(origin, pointInPlane);
  }
  
  void planeHessianNormalForm(const SimplePoint3d& pA,
                              const SimplePoint3d& pB,
                              const SimplePoint3d& pC,
                              double& out_distanceToOrigin,
                              Vector3d& out_normalVector)
  {
    out_normalVector = normalVector(pA, pB, pC);
    out_distanceToOrigin = planeDistanceToOrigin(pA, out_normalVector);
    
    // Make sure the same plane always has the same representation.
    if (out_distanceToOrigin < 0)
    {
      out_distanceToOrigin *= -1;
      out_normalVector = -1.0 * out_normalVector;
    }
    else if (out_distanceToOrigin == 0)
    {
      if (out_normalVector.getX() != 0)
      {
        if (out_normalVector.getX() < 0)
        {
          out_normalVector = -1.0 * out_normalVector;
        }
        return;
      }
      if (out_normalVector.getY() != 0)
      {
        if (out_normalVector.getY() < 0)
        {
          out_normalVector = -1.0 * out_normalVector;
        }
        return;
      }
      if (out_normalVector.getZ() < 0)
      {
        out_normalVector = -1.0 * out_normalVector;
      }
    }
  }

  SimplePoint3d projectPointOntoPlane(const SimplePoint3d &point,
                                      const Plane3d& plane)
  {
    Vector3d v(plane.getPoint(), point);
    Vector3d n = plane.getNormalVector();
    return point + (-(n * v)) * n;
  }


  /* Triangles */
  
  // assumes directed triangles
  bool almostEqual(const Triangle& triangle1, const Triangle& triangle2)
  {
    SimplePoint3d points1[3] = { triangle1.getA(),
                                 triangle1.getB(),
                                 triangle1.getC() };
    SimplePoint3d points2[3] = { triangle2.getA(),
                                 triangle2.getB(),
                                 triangle2.getC() };
                                 
    for (int c = 0; c < 3; ++c)
    {
      bool differenceFound = false;
      for (int d = 0; d < 3; ++d)
      {
        if (!almostEqual(points1[d], points2[(d + c) % 3]))
        {
          differenceFound = true;
          break;
        }
      }
      if (!differenceFound)
        return true;
    }
    return false;
  }
  
  bool isValidTriangle(const SimplePoint3d& pA,
                       const SimplePoint3d& pB,
                       const SimplePoint3d& pC)
  {
    if (almostEqual(pA, pB) || almostEqual(pB, pC) || almostEqual(pC, pA))
    {
      return false;
    }
    if (collinear(pA, pB, pC))
    {
      return false;
    }
    return true;
  }
  
  bool isValidTriangle(const Triangle& triangle)
  {
    return isValidTriangle(triangle.getA(), triangle.getB(), triangle.getC());
  }
  
  // precondition: point is in the plane of the triangle.
  InsideResult pointInsideTriangle(const SimplePoint3d& pointToTest,
                                   const Triangle& triangle)
  {
    Vector3d w = Vector3d(triangle.getA(), pointToTest);
    Vector3d u = Vector3d(triangle.getA(), triangle.getB());
    Vector3d v = Vector3d(triangle.getA(), triangle.getC());
    
    double denominator = (u * v) * (u * v) - (u * u) * (v * v);
    double s = ((u * v) * (w * v) - (v * v) * (w * u)) / denominator;
    double t = ((u * v) * (w * u) - (u * u) * (w * v)) / denominator;

    if (AlmostEqual(s, 0) && AlmostEqual(t, 0))
    {
      return CORNER;
    }
    else if ((AlmostEqual(s, 0) || AlmostEqual(t, 0)) && AlmostEqual(s + t, 1))
    {
      return CORNER;
    }
    else if (AlmostEqual(s, 0) && t > 0 && t < 1)
    {
      return EDGE;
    }
    else if (AlmostEqual(t, 0) && s > 0 && s < 1)
    {
      return EDGE;
    }
    else if (AlmostEqual(s + t, 1) && s > 0 && t > 0)
    {
      return EDGE;
    }
    else if (s > 0 && t > 0 && s + t < 1)
    {
      return INSIDE;
    }
    else
    {
      return OUTSIDE;
    }
  }

  bool isCompletelyInside(const Triangle& t1, const Triangle& t2)
  {
    // if any corner is not inside, then the triangle is not completely inside
    if (pointInsideTriangle(t1.getA(), t2) == OUTSIDE)
    {
      return false;
    }
    if (pointInsideTriangle(t1.getB(), t2) == OUTSIDE)
    {
      return false;
    }
    if (pointInsideTriangle(t1.getC(), t2) == OUTSIDE)
    {
      return false;
    }
    return true;
  }
  
  /* 2D */
  
  bool almostEqual(const SimplePoint2d& p1, const SimplePoint2d& p2)
  {
    return AlmostEqual(p1.getX(), p2.getX())
        && AlmostEqual(p1.getY(), p2.getY());
  }
  
  double distance(const SimplePoint2d& p1, const SimplePoint2d& p2)
  {
    return sqrt(distance_square(p1, p2));
  }
  
  double distance_square(const SimplePoint2d& p1, const SimplePoint2d& p2)
  {
    double lx = p1.getX() - p2.getX();
    double ly = p1.getY() - p2.getY();
    return lx * lx + ly * ly;
  }
  
  double getPolarAngle(const SimplePoint2d& point)
  {
    return atan2(point.getY(), point.getX());
  }

  // Positiv: Rechtsknick; Negativ: Linksknick
  double clockwise(const SimplePoint2d& p1,
                   const SimplePoint2d& p2,
                   const SimplePoint2d& p3)
  {
    return (p3.getX() - p1.getX()) * (p2.getY() - p1.getY()) -
           (p2.getX() - p1.getX()) * (p3.getY() - p1.getY());
  }

  // never true for parallel segments
  bool doSegmentsIntersect(const SimplePoint2d& a1, const SimplePoint2d& a2,
                           const SimplePoint2d& b1, const SimplePoint2d& b2)
  {
    SimplePoint2d ip;
    bool r = lineIntersectionPoint(a1, a2, b1, b2, ip);
    if (r)
    {
      // Lines are not parallel, so we have an intersection point.
      // Just check whether the intersection is inside the segments.
      if (!AlmostLte(min(a1.getX(), a2.getX()), ip.getX()))
        return false;
      if (!AlmostLte(min(b1.getX(), b2.getX()), ip.getX()))
        return false;
      if (!AlmostLte(min(a1.getY(), a2.getY()), ip.getY()))
        return false;
      if (!AlmostLte(min(b1.getY(), b2.getY()), ip.getY()))
        return false;
      if (!AlmostLte(ip.getX(), max(a1.getX(), a2.getX())))
        return false;
      if (!AlmostLte(ip.getX(), max(b1.getX(), b2.getX())))
        return false;
      if (!AlmostLte(ip.getY(), max(a1.getY(), a2.getY())))
        return false;
      if (!AlmostLte(ip.getY(), max(b1.getY(), b2.getY())))
        return false;
      return true;
    }
    else
    {
      return false;
    }
  }

  bool lineIntersectionPoint(const SimplePoint2d& a1, const SimplePoint2d& a2,
                             const SimplePoint2d& b1, const SimplePoint2d& b2,
                             SimplePoint2d& out_intersection)
  {
    double denominator = (a1.getX() - a2.getX()) * (b1.getY() - b2.getY())
                       - (a1.getY() - a2.getY()) * (b1.getX() - b2.getX());

    if (AlmostEqual(denominator, 0))
      return false;
                       
    double X = (a1.getX() * a2.getY() - a1.getY() * a2.getX())
                      * (b1.getX() - b2.getX()) -
               (a1.getX() - a2.getX())
                      * (b1.getX() * b2.getY() - b1.getY() * b2.getX());
    double Y = (a1.getX() * a2.getY() - a1.getY() * a2.getX())
                      * (b1.getY() - b2.getY()) -
               (a1.getY() - a2.getY())
                      * (b1.getX() * b2.getY() - b1.getY() * b2.getX());
    out_intersection.set(X / denominator, Y / denominator);
    return true;
  }
  
  InsideResult pointInsideTriangle(const SimplePoint2d& pointToTest,
                                   const SimplePoint2d& pointA,
                                   const SimplePoint2d& pointB,
                                   const SimplePoint2d& pointC)
  {
    double cw0 = clockwise(pointA, pointB, pointToTest);
    double cw1 = clockwise(pointB, pointC, pointToTest);
    double cw2 = clockwise(pointC, pointA, pointToTest);
  
    if ( !(AlmostLte(cw0, 0) && AlmostLte(cw1, 0) && AlmostLte(cw2, 0))
      && !(AlmostLte(0, cw0) && AlmostLte(0, cw1) && AlmostLte(0, cw2)))
    {
      return OUTSIDE;
    }
    
    int numberOfSegmentsThePointIsIn = 0;
    
    if (AlmostEqual(cw0, 0)) {
      ++ numberOfSegmentsThePointIsIn;
    }
    if (AlmostEqual(cw1, 0)) {
      ++ numberOfSegmentsThePointIsIn;
    }
    if (AlmostEqual(cw2, 0)) {
      ++ numberOfSegmentsThePointIsIn;
    }
    
    switch (numberOfSegmentsThePointIsIn) {
      case 0:
        return INSIDE;
      case 1:
        return EDGE;
      default:
        return CORNER;
    }
  }
  
  InsideResult pointInsideSegment(const SimplePoint2d& pointToTest,
                                  const SimplePoint2d& segmentPoint1,
                                  const SimplePoint2d& segmentPoint2)
  {
    const SimplePoint2d& c = pointToTest;
    const SimplePoint2d& a = segmentPoint1;
    const SimplePoint2d& b = segmentPoint2;
    
    // Idea from here: http://stackoverflow.com/a/328122
    
    double crossproduct = (c.getY() - a.getY()) * (b.getX() - a.getX())
                          - (c.getX() - a.getX()) * (b.getY() - a.getY());
    if (!AlmostEqual(crossproduct, 0))
      return OUTSIDE;

    double dotproduct = (c.getX() - a.getX()) * (b.getX() - a.getX())
                        + (c.getY() - a.getY()) * (b.getY() - a.getY());
    if (dotproduct < 0)
      return OUTSIDE;

    if (dotproduct > distance_square(a, b))
      return OUTSIDE;

    if (almostEqual(pointToTest, segmentPoint1) ||
        almostEqual(pointToTest, segmentPoint2))
    {
      return CORNER;
    }
    else
    {
      return INSIDE;
    }
  }

  // precondition: the segment does share a point with the triangle
  SimplePoint2d firstPointInsideTriangle(const SimplePoint2d& from,
                                         const SimplePoint2d& to,
                                         const SimplePoint2d& t1,
                                         const SimplePoint2d& t2,
                                         const SimplePoint2d& t3)
  {
    if (pointInsideTriangle(from, t1, t2, t3) != OUTSIDE)
      return from;
    SimplePoint2d const * const triangle[3] = { &t1, &t2, &t3 };
    bool intersectsEdge[3];
    SimplePoint2d edgeIntersectionPoints[3];
    for(int c = 0; c < 3; ++c)
    {
      intersectsEdge[c] = doSegmentsIntersect(from, to,
                                              *triangle[c],
                                              *triangle[(c + 1) % 3]);
      if (intersectsEdge[c])
      {
        lineIntersectionPoint(from, to, *triangle[c], *triangle[(c + 1) % 3],
                              edgeIntersectionPoints[c]);
      }
    }
    
    if (!(intersectsEdge[0] || intersectsEdge[1] || intersectsEdge[2]))
    {
      numeric_fail();
    }
    
    // Find the intersection with minimal distance. Initialise a variable
    // with a value that is greater than that distance could be.

    double min_distance_square = 2 * distance_square(from, to) + 1;
    
    SimplePoint2d closest_point;
    
    for(int c = 0; c < 3; ++c)
    {
      if (intersectsEdge[c])
      {
        SimplePoint2d p = edgeIntersectionPoints[c];
        double this_distance_square = distance_square(from, p);
        if (this_distance_square < min_distance_square)
        {
          min_distance_square = this_distance_square;
          closest_point = p;
        }
      }
    }
    return closest_point;
  }
  
  SegmentTriangle2dIntersectionResult
  intersection(const SimplePoint2d& segmentA, const SimplePoint2d& segmentB,
               const SimplePoint2d* triangle)
  {
    // segmentAin is the point of the segment A<->B that is closest to A.
    SimplePoint2d segmentAin, segmentBin;
    bool intersectionFound = false;
    bool segmentAinFound = false, segmentBinFound = false;
    switch(pointInsideTriangle(segmentA, triangle[0], triangle[1], triangle[2]))
    {
      case INSIDE:
        return SEGMENT;
      case EDGE:
      case CORNER:
        segmentAin = segmentA;
        segmentAinFound = true;
        intersectionFound = true;
        break;
      default:
        break;
    }
    switch(pointInsideTriangle(segmentB, triangle[0], triangle[1], triangle[2]))
    {
      case INSIDE:
        return SEGMENT;
      case EDGE:
      case CORNER:
        segmentBin = segmentB;
        segmentBinFound = true;
        intersectionFound = true;
      default:
        break;
    }

    if (!intersectionFound)
    {
      // both points are outside. First, we have to find out whether any
      // edge of the triangle intersects with the segment.
      for (int c = 0; c < 3; ++c)
      {
        if (doSegmentsIntersect(segmentA, segmentB,
                                triangle[c], triangle[(c + 1) % 3]))
        {
          intersectionFound = true;
          break;
        }
      }
    }
    if (intersectionFound)
    {
      if (!segmentAinFound)
      {
        segmentAin = firstPointInsideTriangle(segmentA, segmentB,
                                       triangle[0], triangle[1], triangle[2]);
      }
      if (!segmentBinFound)
      {
        segmentBin = firstPointInsideTriangle(segmentB, segmentA,
                                       triangle[0], triangle[1], triangle[2]);
      }
      if (!almostEqual(segmentAin, segmentBin))
      {
        return SEGMENT;
      }
    }
    else
    {
      return NONE;
    }
    return intersectionFound ? POINT : NONE;
  }
}
