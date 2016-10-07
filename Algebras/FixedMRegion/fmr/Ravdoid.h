/* 
----
 * This file is part of libfmr
 * 
 * File:   Ravdoid.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 15, 2016, 2:12 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Ravdoid

[TOC]

1 Overview

Header file with the class definition for the class ~Ravdoid~

2 Includes and definitions

*/

#ifndef FMR_RAVDOID_H
#define FMR_RAVDOID_H

#include "Curve.h"
#include "Seg.h"
#include "TransformationUnit.h"

namespace fmr {

/*
3 Definition of the class Ravdoid

*/
class Ravdoid : public Curve {
public:
    // Constructors/Destructors
    Ravdoid(double hp, double cd, double xoff, double yoff, double toff,
        double tmin1, double tmax1, double tmin2, double tmax2, double rot);
    virtual ~Ravdoid() {}
    
    // Methods
    Point project (double t);
    bool operator==(Ravdoid& t);
    RCurve rcurve (double t1, double t2);
    bool valid (double t);
    std::string ToString();
    static Ravdoid create (Seg s, TransformationUnit &tu);

    // Fields
    double hp, cd, xoff, yoff, toff, tmin1, tmax1, tmin2, tmax2, rot;
};

}

#endif  /* RAVDOID_H */

