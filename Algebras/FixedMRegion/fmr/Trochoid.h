/*
----
 * This file is part of libfmr
 * 
 * File:   Trochoid.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 15, 2016, 12:36 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Trochoid

[TOC]

1 Overview

Header file with the class definition for the class ~Trochoid~

2 Includes and definitions

*/

#ifndef FMR_TROCHOID_H
#define FMR_TROCHOID_H

#include <math.h>

#include "Curve.h"
#include "Point.h"
#include "TransformationUnit.h"

namespace fmr {

class Trochoid : public Curve {
public:
    // Constructors
    Trochoid(double a, double b, double xoff,
                                          double yoff, double toff, double rot);
    virtual ~Trochoid() {}
    
    // Methods
    static Trochoid create (Point p, TransformationUnit& tu);
    RCurve rcurve (double t1, double t2);
    Point project (double t);
    bool operator==(Trochoid& t);
    std::string ToString();
    
    // Fields
    double a, b, xoff, yoff, toff, rot;
};

}

#endif  /* TROCHOID_H */