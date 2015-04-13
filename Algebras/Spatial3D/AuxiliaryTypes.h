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

//the above header was modified and copied from SuffixTree.h

#include "RectangleAlgebra.h"

#ifndef _AUXILIARY_TYPES_H
#define _AUXILIARY_TYPES_H


class SimplePoint3d;
class Point3d;
class Vector3d;
class Plane3d;
class Triangle;
class TriangleContainer;

enum Axis { X = 0, Y, Z };

class SimplePoint3d
{
public:
  SimplePoint3d();
  SimplePoint3d(double x, double y, double z);
  SimplePoint3d(const SimplePoint3d& src); 

  void set(double x, double y, double z);

  double getX() const;
  double getY() const;
  double getZ() const;
  double get(Axis axis) const;
  
  bool operator==(const SimplePoint3d& other) const;
  
  Rectangle<3> BoundingBox() const;

private:
  
  double x, y, z;
};

class SimplePoint2d
{
public:

  SimplePoint2d();
  SimplePoint2d(double x, double y);
  SimplePoint2d(const SimplePoint2d& src); 

  void set(double x, double y);

  double getX() const;
  double getY() const;
  
  bool operator==(const SimplePoint2d& other) const;

private:
  
  double x, y;
};

class Vector3d : public Attribute
{
public:

  Vector3d();
  Vector3d(bool def);
  Vector3d(double x, double y, double z);
  Vector3d(const SimplePoint3d& from, const SimplePoint3d& to);
  
  ~Vector3d();

  void set(double x, double y, double z);
  void set(const SimplePoint3d& from, const SimplePoint3d& to);

  double getX() const;
  double getY() const;
  double getZ() const;

  double getLength() const;
    
  static const string BasicType();
  static const bool checkType(const ListExpr list);
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  virtual int NumOfFLOBs() const;
  virtual Flob* GetFLOB(const int i);
  size_t Sizeof() const;
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute *arg) const;
  size_t HashValue() const;
  void CopyFrom(const Attribute *arg);
  Vector3d& operator=(const Vector3d& src);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr value, ListExpr typeInfo);
  bool operator==(Vector3d& other) const;


private:
  
  double x, y, z;
};

class Plane3d : public Attribute
{
public:
  Plane3d();
  Plane3d(bool def);
  Plane3d(const SimplePoint3d& point, const Vector3d& normalVector);
  Plane3d(double distanceToOrigin, const Vector3d& normalVector);
  Plane3d(const SimplePoint3d& p1,
          const SimplePoint3d& p2,
          const SimplePoint3d& p3);
  Plane3d(const Triangle& triangle);
  
  void set(const SimplePoint3d& point, const Vector3d& normalVector);
  void set(const double distanceToOrigin, const Vector3d& normalVector);
  void set(const SimplePoint3d& p1,
           const SimplePoint3d& p2,
           const SimplePoint3d& p3);
  void set(const Triangle& triangle);

  SimplePoint3d getPoint() const;
  Vector3d getNormalVector() const;
  double getDistanceToOrigin() const;

  static const string BasicType();
  static const bool checkType(const ListExpr list);
  static ListExpr Property();
  static bool CheckKind(ListExpr type, ListExpr& errorInfo);
  virtual int NumOfFLOBs() const;
  virtual Flob* GetFLOB(const int i);
  size_t Sizeof() const;
  int Compare(const Attribute *arg) const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute *arg) const;
  size_t HashValue() const;
  void CopyFrom(const Attribute *arg);
  Plane3d& operator=(const Plane3d& src);
  ListExpr ToListExpr(ListExpr typeInfo);
  bool ReadFrom(ListExpr value, ListExpr typeInfo);
  bool operator==(Plane3d& other) const;  

  
private:

  double distanceToOrigin;
  Vector3d normalVector;
};

class Triangle
{
public:

  Triangle(const SimplePoint3d& a,
           const SimplePoint3d& b,
           const SimplePoint3d& c);
  Triangle(const Triangle& src);
  
  Vector3d getNormalVector() const;
  Plane3d getPlane() const;

  SimplePoint3d getA() const;
  SimplePoint3d getB() const;
  SimplePoint3d getC() const;
  
  Rectangle<3> BoundingBox() const;

private:
  
  SimplePoint3d pA, pB, pC;
};

class Transformation2d
{
public:

  Transformation2d(const Plane3d& plane);
    
  SimplePoint3d transform(const SimplePoint2d& point) const;
  SimplePoint2d transform(const SimplePoint3d& point) const;
  
private:

  
  Plane3d plane;
  Axis lostAxis;

};

class Projection2d
{
public:

  Projection2d();
  
  Projection2d(const Plane3d& plane,
               const SimplePoint3d& base,
               const SimplePoint3d& directionX);
    
  SimplePoint2d project(const SimplePoint3d& point) const;
  
private:

  Plane3d plane;
  SimplePoint3d origin;
  Vector3d unitX, unitY;

};

ostream& operator<< (ostream& os, const SimplePoint2d& point);
ostream& operator<< (ostream& os, const SimplePoint3d& point);
ostream& operator<< (ostream& os, const Triangle& triangle);
ostream& operator<< (ostream& os, const Vector3d& vector);
#endif