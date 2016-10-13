/* 
----
 * This file is part of libfmr
 * 
 * File:   RootDetect.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 14, 2016, 2:44 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RootDetect

[TOC]

1 Overview

Header file with the class definition for the class ~RootDetect~

2 Includes and definitions

*/

#ifndef FMR_ROOTDETECT_H
#define FMR_ROOTDETECT_H

#include "fmr_Curve.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#ifndef PRECISION
#define PRECISION 0.0000001
#endif

namespace fmr {

class Curve; // Forward declaration

/*
3 Definition of class RootDetect

*/
class RootDetect {
public:
    // Constructors
    RootDetect(Curve& c1, Curve& c2);
    virtual ~RootDetect() {}
    
    // Methods
    std::vector<double> findRoots (double start, double end);
    double findRootBinary (double l, double r, bool rising);
    std::vector<Point> findIntersection();

    // Abstract methods
    virtual double f(double x) = 0; // function to find the roots for
    virtual std::vector<std::pair<double, double> >findIntersectionTimes() = 0; 

protected:
    Curve *c1, *c2;

private:
    double _f(double x) {
        double val = f(x);
        // Eliminate flaps around 0 due to numeric instability
        if (std::abs(val) < PRECISION)
            return 0;
        
        return val;
    }

};

}

#endif  /* ROOTDETECT_H */
