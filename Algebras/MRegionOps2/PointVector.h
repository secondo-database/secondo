/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Headerfile of the Point and Vector classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#pragma once
#ifndef POINTVECTOR_H_
#define POINTVECTOR_H_




#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>

#include <math.h>
#include "NumericUtil.h"
#include "MovingRegion2Algebra.h"

using namespace std;

namespace mregionops2 {

/*
2.1 VRML Constants

Used for generating a VRML file for debugging.

*/

#define VRML_SCALE_FACTOR 0.3
#define VRML_DOUBLE_PRECISION 2

/*
2.2 Forward declarations

*/

class Point2D;
class Point3D;
class Vector2D;
class Vector3D;
class Segment2D;

/*
3 Class Vector3D

This class provides a spatial vector of dimension 3.
It's components are represented by three double values.

*/
class Vector3D {

public:
    
/*
3.1 Constructors

*/

    inline Vector3D() :
        x(0.0), y(0.0), z(0.0) {

    }
// SuS
    inline Vector3D(mpq_class _x, mpq_class _y, mpq_class _z) :
        x(_x), y(_y), z(_z) {
    }

    Vector3D(const Vector2D& v);

/*
3.2 Getter and setter methods

*/

    inline mpq_class GetX() const {
        return x;
    }

    inline mpq_class GetY() const {
        return y;
    }

    inline mpq_class GetZ() const {
        return z;
    }

    inline mpq_class GetT() const {
        return z;
    }
    
/*
3.3.2 Length2
    
Returns the quadratic length of this vector.

*/

    inline mpq_class Length2() const {
        return x*x + y*y + z*z;
    }
    
/*
3.3.3 IsZero
    
Returns ~true~, if all components are nearly equal to zero.

*/
    
    inline bool IsZero() const {   
     return (x == 0) && (y == 0) && ( z == 0) ;   


    }
    
/*
3.3.4 operator -
    
Returns the negative of this vector.

*/   

    inline Vector3D operator -() const {
        return Vector3D(-x, -y, -z);
    }
    
/*
3.3.5 operator [*]

Returns the scalar multplication of w and c.

*/     
    inline friend Vector3D operator *(const mpq_class c, const Vector3D& w) {

        Vector3D v;
        v.x = c * w.x;
        v.y = c * w.y;
        v.z = c * w.z;
        return v;
    }
    
    inline friend Vector3D operator *(const Vector3D& w, const mpq_class c) {

        Vector3D v;
        v.x = c * w.x;
        v.y = c * w.y;
        v.z = c * w.z;
        return v;
    }
    
/*
3.3.6 operator /
    
Returns the scalar multplication of w and 1/c.

*/         

    inline friend Vector3D operator /(const Vector3D& w, const mpq_class c) {

        Vector3D v;
        v.x = w.x / c;
        v.y = w.y / c;
        v.z = w.z / c;
        return v;
    }
    
/*
3.3.7 operator +
    
Returns the vector sum of this and w.

*/       

    inline Vector3D operator +(const Vector3D& w) const {

        Vector3D v;
        v.x = x + w.x;
        v.y = y + w.y;
        v.z = z + w.z;
        return v;
    }
    
/*
3.3.8 operator -
    
Returns the vector difference of this and w.

*/   

    inline Vector3D operator -(const Vector3D& w) const  {

        Vector3D v;
        v.x = x - w.x;
        v.y = y - w.y;
        v.z = z - w.z;
        return v;
    }

/*
3.3.9 operator [*] 

Returns the dot product of this and w.

*/  
// SuS - Skalarprodukt
    inline mpq_class operator *(const Vector3D& w) const {

        return (x * w.x + y * w.y + z * w.z);
    }

/*
3.3.10 operator power
    
Returns the cross product of this and w.

*/      
    
    inline Vector3D operator ^(const Vector3D& w) const {

        Vector3D v;
        v.x = y * w.z - z * w.y;
        v.y = z * w.x - x * w.z;
        v.z = x * w.y - y * w.x;
        return v;
    }
    
/*
3.3.11 Normalize
    
Normalize this vector to length one.

*/      

// SuS    inline void Normalize() {
// SuS
// nicht exakt
// SuS        const mpq_class len = sqrt(x*x + y*y + z*z);

// SuS        if (len != 0.0) {

// SuS            x /= len;
// SuS            y /= len;
// SuS            z /= len;
// SuS        }
// SuS    }
    
/*
3.3.12 operator ==
    
Returns ~true~, if all components of this are nearly equal to all
components of p.

*/        
    
    inline bool operator ==(const Vector3D& p) const {

// muss dann gleich sein
// x = p.x und y = p.y und z = p.z

    return (x == p.x) && (y == p.y) && (z == p.z);
    }

private:

// neuer Datentyp mpq_class aus MovingRegion3Algebra.h

// SuS
    mpq_class x;
    mpq_class y;
    mpq_class z;
};

/*
4 Class Vector2D

This class provides a spatial vector of dimension 2.
It's components are represented by two double values.

*/
class Vector2D {

public:
    
/*
4.1 Constructors

*/

    inline Vector2D() :
        x(0.0), y(0.0) {

    }


    inline Vector2D(mpq_class _x, mpq_class _y) :
        x(_x), y(_y) {

    }

    Vector2D(const Vector3D& v);

/*
4.2 Getter and setter methods

*/


    inline mpq_class GetX() const {
        return x;
    }

    inline mpq_class GetW() const {
        return x;
    }

    inline mpq_class GetY() const {
        return y;
    }

    inline mpq_class GetT() const {
        return y;
    }
    

/*
4.3.2 Length2
    
Returns the quadratic length of this vector.

*/

    inline mpq_class Length2() const {

        return x*x + y*y;
    }
    
/*
4.3.3 IsZero
    
Returns ~true~, if all components are nearly equal to zero.

*/    
    
    inline bool IsZero() const {
     return (x == 0) && (y == 0);
    }
    
/*
4.3.4 operator -
    
Returns the negative of this vector.

*/      

    inline Vector2D operator -() const {
        return Vector2D(-x, -y);
    }
    
/*
4.3.5 operator [*]
    
Returns the scalar multplication of w and c.

*/        
 
    inline friend Vector2D operator *(const mpq_class c, const Vector2D& w) {
        Vector2D v;
        v.x = c * w.x;
        v.y = c * w.y;
        return v;
    }

    inline friend Vector2D operator *(const Vector2D& w, const mpq_class c) {
        Vector2D v;
        v.x = c * w.x;
        v.y = c * w.y;
        return v;
    }
    
/*
4.3.6 operator /
    
Returns the scalar multplication of w and 1/c.

*/         
 
    inline friend Vector2D operator /(const Vector2D& w, const mpq_class c) {

        Vector2D v;
        v.x = w.x / c;
        v.y = w.y / c;
        return v;
    }
    
/*
3.3.7 operator +
    
Returns the vector sum of this and w.

*/         

    inline Vector2D operator +(const Vector2D& w) const {
        Vector2D v;
        v.x = x + w.x;
        v.y = y + w.y;
        return v;
    }
    
/*
3.3.8 operator -
    
Returns the vector difference of this and w.

*/         

    inline Vector2D operator -(const Vector2D& w) const {
        Vector2D v;
        v.x = x - w.x;
        v.y = y - w.y;
        return v;
    }
    
/*
3.3.9 operator [*]
    
Returns the dot product of this and w.

*/  
  
    inline mpq_class operator *(const Vector2D& w) const {
        return (x * w.x + y * w.y);
    }
    
/*
3.3.10 operator $|$
    
Returns the perp product of this and w: a scalar.

*/  
 
    inline mpq_class operator |(const Vector2D& w) const {
        return (x * w.y - y * w.x);
    }
    
/*
3.3.11 operator power
    
Returns the cross product of this and w: a Vector3D

*/  
    
    inline Vector3D operator ^(const Vector2D& w) const {
        return Vector3D(0.0, 0.0, x * w.y - y * w.x);
    }
    
   
/*
3.3.13 operator ==
    
Returns ~true~, if all components of this are nearly equal to all
components of p.

*/       
    
    inline bool operator ==(const Vector2D& p) const {
     return (x == p.x) && ( y == p.y);
    }

private:

    mpq_class x;
    mpq_class y;
};

/*
5 Class Point2D

This class provides a point in the euclidian plane.
It's components are represented by two double values.

*/
class Point2D {

public:
    
/*
5.1 Constructors

*/   

    inline Point2D() :
        x(0.0), y(0.0) {

    }

    inline Point2D(mpq_class _x, mpq_class _y) :
        x(_x), y(_y) {

    }

    inline Point2D(Point2D p1, Point2D p2, mpq_class ratio)
    {
    x = p1.x + (p2.x - p1.x) * ratio;
    y = p1.y + (p2.y - p1.y) * ratio;

    }


    Point2D(const Point3D& p);

/*
5.2 Getter and setter methods.

*/      

        inline mpq_class GetX() const {
          return x;
        }

         inline mpq_class GetW() const {
          return x;
        }

         inline mpq_class GetY() const {
          return y;
        }

         inline mpq_class GetT() const {
          return y;
        }
    
/*
5.3 Operators and Predicates
    
5.3.1 Operators for comparison.

*/     
    
    inline bool operator ==(const Point2D& p) const {
      return (x == p.x) && (y == p.y);
    }
    
    inline bool operator !=(const Point2D& p) const {        
        return !(*this == p);
    }
    
    inline bool operator <(const Point2D& p) const {
    if (x < p.x)
            return true;

    if (x > p.x)
            return false;
            
            return (y < p.y);
    }
    
/*
5.3.2 operator -
    
Returns the Vector2D pointing from p to this.

*/      

    inline Vector2D operator -(const Point2D& p) const {
        return Vector2D(x - p.x, y - p.y);
    }
    
/*
5.3.3 operator +
    
Returns the translation of this along v.

*/      

    inline Point2D operator +(const Vector2D& v) const {
        Point2D p;
        p.x = x + v.GetX();
        p.y = y + v.GetY();
        return p;
    }
    
/*
5.3.4 operator -
    
Returns the translation of this along -v.

*/      

    inline Point2D operator -(const Vector2D& v) const {
        Point2D p;
        p.x = x - v.GetX();
        p.y = y - v.GetY();
        return p;
    }
    
/*
5.3.5 operator +
    
Returns the affine sum of this and p.

*/        

    inline Point2D operator +(const Point2D& p)const {
        Point2D sum;
        sum.x = x + p.x;
        sum.y = y + p.y;
        return sum;
    }
    
/*
5.3.6 operator [*]
    
Returns this point, scaled by the factor f.

*/     
    inline Point2D operator *(const mpq_class& f) const {

        Point2D res;
        res.x = x * f;
        res.y = y * f;
        return res;
    }
    
/*
5.3.7 Distance
    
Returns the distance between this and p.


5.3.8 Distance2
    
Returns the quadratic distance between this and p.

*/        

    inline mpq_class Distance2(const Point2D& p) const {
        return (p - *this).Length2();
    }
    
/*
5.3.9 WhichSide
    
Let l be the line defined by the points start and end.
Then WhichSide returns:

  * A value greater than zero, if this is left of l.
  * A value lower than zero, if this is right of l.
  * Zero, if this is on l.

*/    
    inline mpq_class WhichSide(const Point2D& start, const Point2D& end) const {
        return (start.x - x) * (end.y - y) - (end.x - x) * (start.y - y);
    }
    
    mpq_class WhichSide(const Segment2D& s) const;
    
/*
5.3.10 IsLeft/IsRight/IsColinear
    
This predicates evaluates the WhichSide method using 
an epsilon to avoid rounding errors.

*/        

    inline bool IsLeft(const Point2D& start, const Point2D& end) const {
    return ( WhichSide(start, end) > 0);
    }
    
    inline bool IsRight(const Point2D& start, const Point2D& end) const {
    return ( WhichSide(start, end) < 0);
    }
    
    inline bool IsColinear(const Point2D& start, const Point2D& end) const {
    return ( WhichSide(start, end) == 0);
    }
    
    bool IsLeft(const Segment2D& s) const;
    bool IsRight(const Segment2D& s) const;
    bool IsColinear(const Segment2D& s) const;

private:

    mpq_class x;
    mpq_class y;
};

/*
6 Class Point3D

This class provides a point in the euclidian space.
It's components are represented by three double values.

*/
class Point3D {

public:
    
/*
6.1 Constructors

*/   

    inline Point3D() :
        x(0.0), y(0.0), z(0.0) {

    }

    inline Point3D(mpq_class _x, mpq_class _y, mpq_class _z) :
        x(_x), y(_y), z(_z) {

    }

    Point3D(const Point2D& p);

/*
6.2 Getter and setter methods.

*/ 

    inline mpq_class GetX() const {
        return x;
    }

    inline mpq_class GetY() const {
        return y;
    }

    inline mpq_class GetZ() const {
        return z;
    }

    inline mpq_class GetT() const {
        return z;
    }
    
/*
6.3 Operators and Predicates
    
6.3.1 Operators for comparison.

*/     

    inline bool operator ==(const Point3D& p) const {
     return (x == p.x) && (y == p.y) && (z == p.z);
    }
    
    inline bool operator !=(const Point3D& p) const {
        return !(*this == p);
    }
    
/*
6.3.2 operator -
    
Returns the Vector3D pointing from p to this.

*/      

    inline Vector3D operator -(const Point3D& p) const {
        return Vector3D(x - p.x, y - p.y, z - p.z);
    }
    
/*
6.3.3 operator +
    
Returns the translation of this along v.

*/       

    inline Point3D operator +(const Vector3D& v) const
    {
        Point3D p;
        p.x = x + v.GetX();
        p.y = y + v.GetY();
        p.z = z + v.GetZ();
        return p;
    }
    
/*
6.3.4 operator -
    
Returns the translation of this along -v.

*/     

    inline Point3D operator -(const Vector3D& v) const
    {
        Point3D p;
        p.x = x - v.GetX();
        p.y = y - v.GetY();
        p.z = z - v.GetZ();
        return p;
    }
    
/*
6.3.5 operator +
    
Returns the affine sum of this and p.

*/        

    inline Point3D operator +(const Point3D& p) const
    {
        Point3D sum;
        sum.x = x + p.x;
        sum.y = y + p.y;
        sum.z = z + p.z;
        return sum;
    }
    
/*
6.3.6 operator [*]
    
Returns this point, scaled by the factor f.

*/       
// SuS
    inline Point3D operator *(const mpq_class& f) const {

        Point3D res;
        res.x = x * f;
        res.y = y * f;
        res.z = z * f;
        return res;
    }
    
/*

6.3.8 Distance2
    
Returns the quadratic distance between this and p.

*/       
 
    inline mpq_class Distance2(const Point3D& p) const {
        return (p - *this).Length2();
    }
    
    
/*
6.3.10 DistanceToPlane2
    
Returns the quadratic distance between this and a plane 
defined by the point p0 and the vector normal.

*/     

  mpq_class DistanceToPlane2(const Point3D& p0, const Vector3D& normal) const { 
  const mpq_class sb = (- (normal * (*this - p0))) / normal.Length2();
  const Point3D base = *this + (sb * normal);

  return Distance2(base);
 }
    
/*
6.3.11 GetVRMLDesc
    
Returns a description of this point in VRML format.

*/     
    
    inline string GetVRMLDesc() const {

        std::ostringstream oss;

        oss << std::setprecision(VRML_DOUBLE_PRECISION) << std::fixed << x
                << " " << y << " " << z << ", ";

        return oss.str();
    }

private:

    mpq_class x;
    mpq_class y;
    mpq_class z;
};

/*
7 Overloaded output operators
    
*/  

ostream& operator <<(ostream& o, const Point2D& p);
ostream& operator <<(ostream& o, const Point3D& p);
ostream& operator <<(ostream& o, const Vector2D& p);
ostream& operator <<(ostream& o, const Vector3D& p);

} // end of namespace mregionops2

#endif // POINTVECTOR_H_
