/* 
----
 * This file is part of libfmr
 * 
 * File:   MPoint.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 10, 2016, 6:14 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class MPoint

[TOC]

1 Overview

Header file with the class definition for the class ~MPoint~

2 Includes and definitions

*/

#ifndef FMR_MPOINT_H
#define FMR_MPOINT_H

#include <vector>
#include <iostream>
#include <algorithm>

#include "fmr_ISSegCurve.h"
#include "fmr_UPoint.h"
#include "fmr_MBool.h"
#include "fmr_Interval.h"

namespace fmr {

class FMRegion;
    
/*
3 Declaration for the class ~MPoint~

*/
class MPoint {
public:
    MPoint() {}
    virtual ~MPoint() {}
    MPoint(RList& l);
    MPoint intersection (FMRegion& fmr);
    MBool inside (FMRegion& fmr);
    RList toRList();
    
    std::vector<UPoint> units;
private:

};

}

#endif  /* MPOINT_H */