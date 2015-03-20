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

#ifndef VECTOR2D_H_
#define VECTOR2D_H_
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>
#include "Point2D.h"


using namespace std;

namespace mregionops2 {

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

    inline Vector2D(Point2D p1, Point2D p2)
    {
       x = p2.GetX() - p1.GetX();
       y = p2.GetY() - p1.GetY();
    }

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

ostream& operator <<(ostream& o, Vector2D& p);

}
#endif





