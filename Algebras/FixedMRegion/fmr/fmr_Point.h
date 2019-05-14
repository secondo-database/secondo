/* 
----
 * This file is part of libfmr
 * 
 * File:   Point.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 3:21 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Point

[TOC]

1 Overview

Header file with the class definition for the class ~Point~

2 Includes and definitions

*/

#ifndef FMR_POINT_H
#define FMR_POINT_H

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>

#include "fmr_RList.h"

#ifndef PRECISION
#define PRECISION 0.0000001
#endif

namespace fmr {

class TransformationUnit; // Forward declaration

/*

3 Definition of class ~Point~

*/
class Point {
public:
    // Constructors
    Point();
    Point(RList& l);
    Point(double x, double y);
    virtual ~Point() {}
    
    // Operators
    Point operator+(Point p);
    Point operator-(Point p);
    Point operator*(double nr);
    bool operator==(Point p) { return p.x==x&&p.y==y; }
    
    // Methods
    bool near(Point p) { return std::abs(p.x-x) < PRECISION &&
                                std::abs(p.y-y) < PRECISION; }
    Point transform(TransformationUnit& tu, double frac);
    Point rotate(double angle);
    Point rotate(Point center, double angle);
    double angle();
    double length();
    bool valid() { return !std::isnan(x) && !std::isnan(y); }
    double distance(Point& p2) { return (*this-p2).length(); }
    
    std::string ToString();
    RList toRList();

    // Fields
    double x, y;
};

}

#endif  /* FMR_POINT_H */

