/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

/*

1 class deklarations

2 Defines and Includes

*/

#ifndef POINT3DEXT_H_
#define POINT3DEXT_H_
#include "MovingRegion2Algebra.h"
#include "PointVector.h"
#include "Segment.h"
#include "Refinement3.h"
#include "IntersectionSegment.h"
#include "IntSegContainer.h"

namespace mregionops2 {

/*

1 Forward declarations

*/
class PointExtSet;
class Point3DExt;

/*

1 Enumeration SourceFlag

Indicates the source unit of a ~Point3DExt~.

*/


enum SourceFlag {

    PFACE_A,
    PFACE_B
};


/*

1 Class Point3DExt

This datastructure is used in the class ~PointExtSet~.
It simply extends ~Point3D~ with the attribute ~sourceFlag~.

*/

class Point3DExt : public Point3D {

public:

/*
1.1 Constructors

*/

    inline Point3DExt() :
        Point3D() {
    }

inline Point3DExt(mpq_class a, mpq_class b, mpq_class c, 
                  SourceFlag _sourceFlag) :

        Point3D(a, b, c),
        sourceFlag(_sourceFlag) {
    }

/*
1.1 Operators and Predicates

1.1.1 operator $<$

*/

    bool operator <(const Point3DExt& p) const;

/*

1.1 Attributes

1.1.1 sourceFlag

An enum, indicating the ~PFace~, this ~Point3D~ belongs to.
Possible values are:
  * $PFACE\_A$
  * $PFACE\_B$

*/

    SourceFlag sourceFlag;
};

/*
1 Class PointExtSet

This set is used in the class ~PFace~ to compute the intersection segment of
two ~PFaces~.

*/

class PointExtSet {

public:

/*

1.1 Constructors

*/

    inline PointExtSet() {

    }

/*

1.1 Operators and Predicates

1.1.1 Insert

Inserts p, if p isn't already inserted.

*/

    inline void Insert(const Point3DExt& p) {

        s.insert(p);
    }

/*

1.1.1 Size

Returns the number of points in the set.

*/

    inline unsigned int Size() const {

        return s.size();
    }

/*
1.1.1 GetIntersectionSegment

Returns ~true~, if there is an intersection segment and writes it to result.

*/

    bool GetIntersectionSegment(Segment3D& result) const;

/*

1.1 Methods for debugging

*/

void Print() const;

private:

/*

1.1 Attributes

1.1.1 s

A ~std::set~, using the overloaded operator $<$ for comparison.

*/

    set<Point3DExt> s;
};



}

#endif /*POINT3DEXT_H_*/

