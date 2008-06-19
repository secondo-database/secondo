/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/

#include "PointVector.h"
#include "Segment.h"

namespace mregionops {

Point2D::Point2D(const Point3D& p) :
	x(p.GetX()), y(p.GetY()) {

}

double Point2D::WhichSide(const Segment2D& s) const {

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

Point3D::Point3D(const Point2D& p) :
	x(p.GetX()), y(p.GetY()), z(0.0) {

}

Vector2D::Vector2D(const Vector3D& v) :
	x(v.GetX()), y(v.GetY()) {

}

Vector3D::Vector3D(const Vector2D& v) :
	x(v.GetX()), y(v.GetY()), z(0.0) {

}

} // end of namespace mregionops
