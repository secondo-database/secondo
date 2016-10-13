/* 
----
 * This file is part of libfmr
 * 
 * File:   TransformationUnit.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 4:54 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class TransformationUnit

[TOC]

1 Overview

Header file with the class definition for the class ~TransformationUnit~

2 Includes and definitions

*/

#ifndef FMR_TRANSFORMATIONUNIT_H
#define FMR_TRANSFORMATIONUNIT_H

#include "fmr_Point.h"
#include "fmr_Interval.h"
#include "fmr_RList.h"

namespace fmr {

class TransformationUnit {
public:
    // Constructors
    TransformationUnit();
    TransformationUnit(Point c, Point v, double a);
    TransformationUnit(Point c, Point v0, Point v,
                                              double a0, double a, Interval iv);
    TransformationUnit(RList& l);
    virtual ~TransformationUnit() {}

    // Methods
    TransformationUnit restrict(Interval _iv);
    std::string ToString();
    RList toRList();
    
    // Fields
    Point c, v0, v;
    double a0, a;
    Interval iv;
};

}

#endif  /* TRANSFORMATIONUNIT_H */