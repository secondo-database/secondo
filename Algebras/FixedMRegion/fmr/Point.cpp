/* 
 * This file is part of libfmr
 * 
 * File:   Point.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 3:21 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Point~

[TOC]

1 Overview

The class ~Point~ represents a point in 2D. Several operations are
implemented, i.e. addition, scalar-multiplication, rotation and
(polar) angle

*/

#include "Point.h"
#include "TransformationUnit.h"

using namespace fmr;

/*
2 Constructor

Constructs an invalid point

*/
Point::Point() : x(nan("")), y(nan("")) {
}

/*
3 Constructor with x/y

Constructs a point from its cartesian coordinates

*/
Point::Point(double x, double y) : x(x), y(y) {
}

/*
4 Constructor from RList

Constructs a point from its RList representation

*/
Point::Point(RList& l) {
    x = (double) l[0];
    y = (double) l[1];
}

/*
5 ~operator-~

Subtract a point from this point.
Returns the result, this point is not modified.

*/
Point Point::operator- (Point p) {
    return Point(x-p.x, y-p.y);
}

/*
6 ~operator+~

Add a point from this point.
Returns the result, this point is not modified.

*/
Point Point::operator+ (Point p) {
    return Point(x+p.x, y+p.y);
}

/*
7 ~operator multiply~

Multiply each coordinate with the given factor.
Returns the result, this point is not modified.

*/
Point Point::operator* (double factor) {
    return Point(x*factor, y*factor);
}

/*
8 ~transform~

Transform this point according to the given TransformationUnit
~tu~ (see TransformationUnit.cpp) at fraction ~frac~ of the
time interval.
Returns the result, this point is not modified.

*/
Point Point::transform(TransformationUnit& tu, double frac) {
    return this->rotate(tu.c, tu.a0 + tu.a * frac) + tu.v0 + tu.v * frac;
}

/*
9 ~rotate~

Rotate this point around ~center~ by ~angle~.
Returns the result, this point is not modified.

*/
Point Point::rotate(Point center, double angle) {
    Point p = (*this-center);
    
    double s = sin(angle);
    double c = cos(angle);
    double xn = p.x * c - p.y * s;
    double yn = p.x * s + p.y * c;
    
    return Point(xn, yn)+center;
}

/*
10 ~rotate~

Rotate this point around the origin (0/0) by ~angle~.
Returns the result, this point is not modified.

*/
Point Point::rotate(double angle) {
    double s = sin(angle);
    double c = cos(angle);
    return Point(x*c-y*s, x*s+y*c);
}

/*
11 ~angle~

Calculates the polar angle of this point

*/
double Point::angle() {
    return atan2(y, x);
}

/*
12 ~length~

Calculates the distance of this point to
the origin (0/0)

*/
double Point::length() {
    return sqrt(x*x+y*y);
}

/*
13 ~ToString~

Returns a string representation of this point.

*/
std::string Point::ToString() {
    std::stringstream ss;
    ss << "( " << x << " " << y << " )";
    return ss.str();
}

/*
14 ~toRList~

Returns the RList representation of this point.

*/
RList Point::toRList() {
    RList ret;
    
    ret.append(x);
    ret.append(y);
    
    return ret;
}
