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

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#ifndef POINT3DEXTSET_H_
#define POINT3DEXTSET_H_

#include "Point3DExt.h"
#include "Segment3D.h"

#include <set>

namespace mregionops2 {

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

#endif /*POINT3DEXTSET_H_*/

