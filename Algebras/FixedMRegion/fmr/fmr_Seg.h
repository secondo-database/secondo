/* 
----
 * This file is part of libfmr
 * 
 * File:   Seg.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 3:21 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Seg

[TOC]

1 Overview

Header file with the class definition for the class ~Seg~

2 Includes and definitions

*/

#ifndef FMR_SEG_H
#define FMR_SEG_H

#include "fmr_Point.h"
#include "fmr_TransformationUnit.h"
#include <string>
#include <iostream>

namespace fmr {

/*
3 Definition of class Seg

*/
class Seg {
public:
    // Constructors
    Seg() {}
    Seg(Point i, Point f);
    virtual ~Seg() {}

    // Methods
    Seg transform(TransformationUnit& tu, double frac);
    double sign (Point p);
    Seg rotate (Point center, double angle);
    Seg operator+ (Point v);
    double angle () { return (i - f).angle(); }
    double getAngle();
    bool near (Seg& s2, double tolerance);
    double length () { return (i - f).length(); };
    std::pair<bool, TransformationUnit> calculateTransformation(Seg& seg,
                                                Point center, double tolerance);
    bool intersects (Seg& seg);
    bool valid() { return i.valid() && f.valid(); }
    std::string ToString();
    
    // Fields
    Point i, f;
};

}

#endif  /* SEG_H */