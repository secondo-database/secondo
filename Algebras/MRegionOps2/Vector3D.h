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

April - November 2014, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#pragma once
#ifndef VECTOR3D_H_
#define VECTOR3D_H_

#include <gmp.h>
#include <gmpxx.h>
#include <iostream>

using namespace std;

namespace mregionops2 {

class Point3D;

class Vector3D {

public:
    

    inline Vector3D() :
        x(0), y(0), z(0) {

    }

    inline Vector3D(mpq_class _x, mpq_class _y, mpq_class _z) :
        x(_x), y(_y), z(_z) {

    }

    Vector3D(Point3D p1, Point3D p2);

    inline mpq_class GetX() const {
        return x;
    }


    inline mpq_class GetY() const {
        return y;
    }

    inline mpq_class GetZ() const {
        return z;
    }


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

    inline mpq_class operator *(const Vector3D& w) const {
        return (x * w.x + y * w.y + z * w.z);
    }
    
    Vector3D CrossProduct(Vector3D vec);

private:

    mpq_class x;
    mpq_class y;
    mpq_class z;
};

ostream& operator <<(ostream& o, Vector3D& p);

}
#endif
