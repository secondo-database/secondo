/* 
----
 * This file is part of libfmr
 * 
 * File:   UPoint.h
 * Author: flo
 *
 * Created on September 10, 2016, 6:20 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class UPoint

[TOC]

1 Overview

Header file with the class definition for the class ~UPoint~

2 Includes and definitions

*/

#ifndef FMR_UPOINT_H
#define FMR_UPOINT_H

#include "fmr_Interval.h"
#include "fmr_Point.h"
#include "fmr_TransformationUnit.h"
#include "fmr_Curve.h"

namespace fmr {

/*
3 Definition of class ~UPoint~

*/
class UPoint : public Curve {
public:
    //Constructors
    UPoint();
    UPoint(RList& l);
    UPoint(Point s, Point e, Interval iv);
    virtual ~UPoint() {}

    // Methods
    UPoint transform(TransformationUnit& tu);
    Point atinstant (double time);
    Point project (double t);
    UPoint restrict (Interval iv);
    RCurve rcurve (double t1, double t2) { return RCurve(); }
    double fx(double t);
    double fy(double t);
    RList toRList();
    
    // Fields
    Interval iv;
    Point s, v, c;
    double rot;
    bool valid;
};

}

#endif  /* UPOINT_H */

