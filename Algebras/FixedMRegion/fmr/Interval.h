/* 
----
 * This file is part of libfmr
 * 
 * File:   Interval.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 5:00 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Interval

[TOC]

1 Overview

Header file with the class definition for the class ~Interval~

2 Includes and definitions

*/

#ifndef FMR_INTERVAL_H
#define FMR_INTERVAL_H

#include <string>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "RList.h"

namespace fmr {

/*
3 Definition of class ~Interval~

*/
class Interval { // a time interval
public:
    double start; // start point in ms since epoch
    double end;   // end point in ms since epoch
    bool lc;      // left-closed interval?
    bool rc;      // right-closed interval?
    
    Interval(double start, double end, bool lc, bool rc) : 
             start(start), end(end), lc(lc), rc(rc) {};
    Interval(std::string start, std::string end, bool lc, bool rc);
    Interval(RList& l);
    Interval() : start(0), end(0), lc(true), rc(true) {};
    Interval intersect (Interval& iv);
    double getFrac(double currentTime);
    double project (double t);
    static std::string timestr(double currentTime);
    std::string startstr(); // start in YYYY-mm-dd-HH:MM:ss.SSS
    std::string endstr();   // end   in YYYY-mm-dd-HH:MM:ss.SSS
    
    std::string ToString();
    RList toRList();
};

}

#endif  /* INTERVAL_H */