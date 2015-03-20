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

[1] Headerfile

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#ifndef POINT3D_H_
#define POINT3D_H_

#include <gmp.h>
#include <gmpxx.h>
#include <iostream>

#include "Point2D.h"
#include "Vector3D.h"


using namespace std;

namespace mregionops2 {

class Point3D {

public:
    

    inline Point3D() :
        x(0), y(0), z(0) {
    }

    inline Point3D(mpq_class _x, mpq_class _y, mpq_class _z) :
        x(_x), y(_y), z(_z) {
    }

    Point3D(const Point2D& p);

//  to the base p1 the ratio of the difference between the vectors is added
    inline Point3D(Point3D _p1, Point3D _p2, mpq_class _ratio) :
           x(_p1.x + ( _p2.x - _p1.x) * _ratio),
           y(_p1.y + ( _p2.y - _p1.y) * _ratio),
           z(_p1.z + ( _p2.z - _p1.z) * _ratio) {
          } 


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
    
    inline bool operator ==(const Point3D& p) const {
            return ((x == p.x) && (y == p.y) && (z == p.z));
    }

    inline Point3D operator +(const Vector3D& v) const
    {
       Point3D p;
       p.x = x + v.GetX();
       p.y = y + v.GetY();
       p.z = z + v.GetZ();
       return p;
    }

    bool LiesBetween(const Point3D& p1, const Point3D& p2) const;

private:

    mpq_class x;
    mpq_class y;
    mpq_class z;
};

ostream& operator <<(ostream& o, Point3D& p);

}
#endif /*POINT3D_H_*/
