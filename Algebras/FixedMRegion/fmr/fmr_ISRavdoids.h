/* 
----
 * This file is part of libfmr
 * 
 * File:   ISRavdoids.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 19, 2016, 12:09 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class ISRavdoids

[TOC]

1 Overview

Header file with the class definition for the class ~ISRavdoids~

2 Includes and definitions

*/

#ifndef FMR_ISRAVDOIDS_H
#define FMR_ISRAVDOIDS_H

#include <math.h>

#include "fmr_RootDetect.h"
#include "fmr_Ravdoid.h"

namespace fmr {

/*

3 Definition of the class ~ISRavdoids~

*/
class ISRavdoids : public RootDetect {
public:
    ISRavdoids(Ravdoid& o1, Ravdoid& o2);
    virtual ~ISRavdoids() {}
    
    double f(double x);
    std::vector<std::pair<double, double> > findIntersectionTimes();
    
    Ravdoid& o1, o2;
    
private:
    bool first, second;
    std::vector<std::pair<double, double> > findIntersectionTimes(bool first,
                                                                   bool second);
    double getT1 (double t2, bool first, bool second);
};

}

#endif  /* ISRAVDOIDS_H */