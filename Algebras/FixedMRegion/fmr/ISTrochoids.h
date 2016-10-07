/* 
----
 * This file is part of libfmr
 * 
 * File:   ISTrochoids.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 16, 2016, 4:53 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class ISTrochoids

[TOC]

1 Overview

Header file with the class definition for the class ~ISTrochoids~

2 Includes and definitions

*/

#ifndef FMR_ISTROCHOIDS_H
#define FMR_ISTROCHOIDS_H

#include <math.h>

#include "RootDetect.h"
#include "Trochoid.h"

namespace fmr {

/*

3 Declaration for the class ~ISTrochoids~

*/
class ISTrochoids : public RootDetect {
public:
    ISTrochoids(Trochoid& o1, Trochoid& o2);
    virtual ~ISTrochoids() {}

    double f(double x);
    std::vector<std::pair<double, double> > findIntersectionTimes();
    
    Trochoid& o1, o2;
    
private:
    bool first;
    std::vector<std::pair<double, double> > findIntersectionTimes(bool first);
    double getT1 (double t2, bool first);

};

}

#endif  /* ISTROCHOIDS_H */
