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

Point2D::Point2D(const Point3D& p) :
	x(p.GetX()), y(p.GetY()) {

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