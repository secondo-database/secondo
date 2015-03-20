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

[1] Implementation of the MRegionOps2Algebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "PointVector.h"
#include "Segment.h"

namespace mregionops2 {

/*
3 Class Point2D

*/

Point2D::Point2D(const Point3D& p) :
	x(p.GetX()), y(p.GetY()) {

}

mpq_class Point2D::WhichSide(const Segment2D& s) const {

    return WhichSide(s.GetStart(), s.GetEnd());
}

bool Point2D::IsLeft(const Segment2D& s) const {

    return IsLeft(s.GetStart(), s.GetEnd());
}

bool Point2D::IsRight(const Segment2D& s) const {

    return IsRight(s.GetStart(), s.GetEnd());
}

bool Point2D::IsColinear(const Segment2D& s) const {

    return IsColinear(s.GetStart(), s.GetEnd());
}

ostream& operator <<(ostream& o, const Point2D& p) {

    o << "(" << p.GetX() << ", " << p.GetY() << ")";

    return o;
}




/*
4 Class Point3D

*/

Point3D::Point3D(const Point2D& p) :
	x(p.GetX()), y(p.GetY()), z(0.0) {

}

ostream& operator <<(ostream& o, const Point3D& p) {

    o << "(" << p.GetX() << ", " << p.GetY() <<  ", " << p.GetZ() << ")";

    return o;
}

/*
5 Class Vector2D

*/

Vector2D::Vector2D(const Vector3D& v) :
	x(v.GetX()), y(v.GetY()) {

}

ostream& operator <<(ostream& o, const Vector2D& p) {

    o << "(" << p.GetX() << ", " << p.GetY() << ")";

    return o;
}

/*
6 Class Vector3D

*/

Vector3D::Vector3D(const Vector2D& v) :
	x(v.GetX()), y(v.GetY()), z(0.0) {

}

ostream& operator <<(ostream& o, const Vector3D& p) {

    o << "(" << p.GetX() << ", " << p.GetY() <<  ", " << p.GetZ() << ")";

    return o;
}

} // end of namespace mregionops2
