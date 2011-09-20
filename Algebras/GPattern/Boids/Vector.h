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

This file was originally written by Christopher John Kline, under the copy
right statement below. Mahomud Sakr, September 2011, has made the necessary
changes to make it available as a SECONDO operator.

  Copyright (C) 1996, Christopher John Kline
  Electronic mail: ckline@acm.org

  This software may be freely copied, modified, and redistributed
  for academic purposes and by not-for-profit organizations, provided that
  this copyright notice is preserved on all copies, and that the source
  code is included or notice is given informing the end-user that the source
  code is publicly available under the terms described here.

  Persons or organizations wishing to use this code or any modified version
  of this code in a commercial and/or for-profit manner must contact the
  author via electronic mail (preferred) or other method to arrange the terms
  of usage. These terms may be as simple as giving the author visible credit
  in the final product.

  There is no warranty or other guarantee of fitness for this software,
  it is provided solely "as is". Bug reports or fixes may be sent
  to the author, who may or may not act on them as he desires.

  If you use this software the author politely requests that you inform him
  via electronic mail.


Vector.h
INTERFACE: Class for 3-vectors with double precision
(c) 1996 Christopher Kline <ckline@acm.org>

September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/

#ifndef __VECTOR_H
#define __VECTOR_H

#include <math.h>
#include <stdarg.h>
#include <iostream>
#include <stdlib.h>
using namespace std;
// ---------------- VECTORS CLASS ------------------
 
class MathVector {

public:
  double x, y, z;
  // direction with magnitude
  
  MathVector(void);
  // Default constructor
  MathVector( double a, double b, double c);
  // Constructor
  
  void Set(double a, double b, double c);
  // Set each component of the vector
  void SetDirection(const MathVector &d);
  // Set the direction of the vector without modifying the length
  void CopyDirection(const MathVector &d);
  // Set the direction of this vector to be the same as the direction of
  // the argument
  void SetMagnitude(const double m);
  // Set the magnitude of the vector without modifying the direction
  void CopyMagnitude(const MathVector &v);
  // Set the magnitude of this vector to be the same as the
  // magnitude of the argument
  void Normalize(void);
  // Normalize the vector to have a length of 1
  double Length(void);
  // Return the magnitude (length) of this vector
  
  MathVector &operator=(const MathVector &b);
  // ex: a = b
  double &operator[](const int index);
  // ex: a[1] (same as a.y)
  friend int operator!=(const MathVector &a, const MathVector &b);
  // ex: a != b
  friend int operator==(const MathVector &a, const MathVector &b);
  // ex: a == b
  friend MathVector operator+(const MathVector &a, const MathVector &b);
  // ex: a + b
  friend MathVector operator-(const MathVector &a, const MathVector &b);
  // ex: a - b
  friend MathVector operator-(const MathVector &a);
  // ex: -a
  friend MathVector &operator+=(MathVector &a, const MathVector &b);
  // ex: a += b
  friend MathVector &operator-=(MathVector &a, const MathVector &b);
  // ex: a -= b
  friend MathVector operator%(const MathVector &a, const MathVector &b);
  // ex: a % b (cross product)
  friend double operator*(const MathVector &a, const MathVector &b);
  // ex: a * b (dot product)
  friend MathVector operator*(const double &a, const MathVector &b);
  // ex: a * b (scalar multiplication)
  friend MathVector operator*(const MathVector &a, const double &b);
  // ex: a * b (scalar multiplication)
  friend MathVector &operator*=(MathVector &a, const double &b);
  // ex: a *= b (scalar multiplication + assignment)
  friend MathVector operator/(const MathVector &a, const double &b);
  // ex: a / b (scalar divide)
  friend MathVector &operator/=(MathVector &a, const double &b);
  // ex: a /= b (scalar divide + assignment)

  friend double Magnitude(const MathVector &a);
  // Returns the length of the argument
  friend double AngleBetween(const MathVector &a, const MathVector &b);
  // Returns the angle (in radians!) between the two arguments
  
private:

};



// ------------ CALCULATIONS USING VECTORS --------------

MathVector Direction(const MathVector &a);
MathVector Direction(const double &x, const double &y, const double &z);
MathVector Average(int numvectors, MathVector a, ...);

// --------------- I/O OPERATORS ------------------------

std::ostream &operator<<(std::ostream &strm, const MathVector &v);


//-----------------------------------------------------
// INLINE FUNCTIONS
//-----------------------------------------------------

#define VLENSQRD(a, b, c) ((a)*(a) + (b)*(b) + (c)*(c))
#define VLEN(a, b, c) sqrt(VLENSQRD(a, b, c))

inline void 
MathVector::Set(double a, double b, double c) {

  x = a;
  y = b;
  z = c;

}

inline void 
MathVector::Normalize(void) {

  double mag = VLEN(x, y, z);

  if (mag == 0)
    return;

  x /= mag;
  y /= mag;
  z/= mag;
  
}

inline MathVector::MathVector(void) {

  Set(0, 0, 0);
}         

inline MathVector::MathVector( double a, double b, double c) {

  Set(a, b, c);  
} 

inline MathVector &
MathVector::operator=(const MathVector &b) { // example: a = b

  x = b.x; 
  y = b.y; 
  z = b.z; 

  return(*this);
}

inline void 
MathVector::SetMagnitude(const double m) {

  this->Normalize();
  *this *= m;

}

inline void 
MathVector::CopyMagnitude(const MathVector &v) {

 SetMagnitude(Magnitude(v));
}

inline void 
MathVector::SetDirection(const MathVector &d) {

  double m = Magnitude(*this);
  MathVector v = d;
  
  v.Normalize();
  *this = v * m;
  
}

inline void 
MathVector::CopyDirection(const MathVector &d) {

  SetDirection(Direction(d));
}

inline double
MathVector::Length(void) {

  return Magnitude(*this);
}

inline double &
MathVector::operator[](const int index) {  // example: a[1] (same as a.y)

  if (index == 0) 
    return(x);
  else if (index == 1) 
    return(y);
  else if (index == 2) 
    return(z);
  else {
    std::cerr << "WARNING: You're using subscripting to access a "
        "nonexistant vector component (one other than x, y, or z). "
        "THIS IS BAD!!\n";
    exit(888);
    return(z); // this will never get called, but prevents compiler warnings...
  }
  
}

inline int 
operator!=(const MathVector &a, const MathVector &b) { // example: a 1= b

  return(a.x != b.x || a.y != b.y || a.z != b.z);
}

inline int 
operator==(const MathVector &a, const MathVector &b) { // example: a == b

  return(a.x == b.x && a.y == b.y && a.z == b.z);
}

inline MathVector
operator+(const MathVector &a, const MathVector &b) { // example: a + b

  MathVector c(a.x+b.x, a.y+b.y, a.z+b.z);
  return(c);
}

inline MathVector
operator-(const MathVector &a, const MathVector &b) { // example: a - b

  MathVector c(a.x-b.x, a.y-b.y, a.z-b.z);
  return(c);
}

inline MathVector
operator-(const MathVector &a) { // example: -a

  MathVector c(-a.x, -a.y, -a.z);

  return(c);
}

inline MathVector &
operator+=(MathVector &a, const MathVector &b) { // example: a += b

  a = a + b;
  return(a);
}

inline MathVector &
operator-=(MathVector &a, const MathVector &b) { // example: a -= b

  a = a - b;
  return(a);
}

inline MathVector
operator%(const MathVector &a, const MathVector &b)
{ // example: a % b (cross product)

  MathVector c(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
  return(c);
}

inline double 
operator*(const MathVector &a, const MathVector &b)
{ // example: a * b (dot product)

  return(a.x*b.x + a.y*b.y + a.z*b.z);
}

inline MathVector
operator*(const double &a, const MathVector &b)
{ // example: a * b (scalar multiplication)

  MathVector c = b;
  c.x *= a;
  c.y *= a;
  c.z *= a;
  
  return(c);
}

inline MathVector
operator*(const MathVector &a, const double &b)
{ // example: a * b (scalar multiplication)

  return(b * a);
}

inline MathVector &
operator*=(MathVector &a, const double &b)
{ // example: a *= b (scalar multiplication + assignment)

  a = a * b;
  return(a);
}

inline MathVector
operator/(const MathVector &a, const double &b)
{ // example: a / b (scalar divide)

  if (b == 0) 
    std::cerr << "WARNING: You're dividing a vector by a "
        "zero-length scalar! NOT GOOD!\n";

  MathVector c = a*(1/b);

  return(c);
}

inline MathVector &
operator/=(MathVector &a, const double &b)
{ // example: a / b (scalar divide + assignment)

  a = a/b;
  return(a);
}

inline std::ostream &
operator<<(std::ostream &strm, const MathVector &v) {

  return strm << "[" << v.x << ", " << v.y << ", " << v.z << "]";
}

inline MathVector
Direction(const MathVector &a) {

  MathVector u = a;

  u.Normalize();
  return(u);
}

inline MathVector
Direction(const double &x, const double &y, const double &z) {

  return Direction(MathVector(x, y, z));
}


inline double 
Magnitude(const MathVector &a) {

  return VLEN(a.x, a.y, a.z);
}

inline double 
AngleBetween(const MathVector &a, const MathVector &b) {

  // returns angle between a and b in RADIANS
  return acos((a*b)/(Magnitude(a)*Magnitude(b)));
} 


#undef VLEN
 
#endif // #ifndef __VECTOR_H
