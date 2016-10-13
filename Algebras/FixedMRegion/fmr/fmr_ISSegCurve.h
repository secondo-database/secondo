/* 
----
 * This file is part of libfmr
 * 
 * File:   ISSegCurve.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 14, 2016, 4:17 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file of class ISSegCurve

[TOC]

1 Overview

This file contains the declaration of the class ~ISSegCurve~
 
2 Includes and definitions

*/

#ifndef FMR_ISSEGCURVE_H
#define FMR_ISSEGCURVE_H

#include "fmr_RootDetect.h"
#include "fmr_Seg.h"
#include "fmr_SegT.h"
#include "fmr_Curve.h"
#include "fmr_UPoint.h"

namespace fmr {

/*

3 Definition of the class ~ISSegCurve~

*/
class ISSegCurve : public RootDetect {
public:
    ISSegCurve(SegT& seg, Curve& curve);
    virtual ~ISSegCurve() {}
    double f(double x);
    std::vector<std::pair<double, double> > findIntersectionTimes();
    
    Curve& curve;
    SegT segt;
    double c, m;

};

}

#endif  /* ISSEGCURVE_H */

