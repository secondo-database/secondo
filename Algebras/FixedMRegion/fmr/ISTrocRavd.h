/* 
----
 * This file is part of libfmr
 * 
 * File:   ISTrocRavd.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 19, 2016, 12:38 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file of class ISTrocRavd

[TOC]

1 Overview

This file contains the declaration of the class ~ISTrocRavd~
 
2 Includes and definitions

*/

#ifndef FMR_ISTROCRAVD_H
#define FMR_ISTROCRAVD_H

#include <vector>
#include <math.h>

#include "RootDetect.h"
#include "Trochoid.h"
#include "Ravdoid.h"

namespace fmr {

/*

3 Definition of the class ~ISTrocRavd~

*/
class ISTrocRavd : public RootDetect {
public:
    ISTrocRavd(Trochoid& o1, Ravdoid& o2);
    ISTrocRavd(const ISTrocRavd& orig);
    virtual ~ISTrocRavd() {}
    
    double f(double x);
    std::vector<std::pair<double, double> > findIntersectionTimesTouch();
    std::vector<std::pair<double, double> > findIntersectionTimes();
    
    Trochoid& o1;
    Ravdoid& o2;
private:
    bool first;
    std::vector<std::pair<double, double> > findIntersectionTimes(bool first);
    double getT1 (double t2, bool first);
};

}

#endif  /* ISTROCRAVD_H */