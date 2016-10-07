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

#include "FMRegion.h"
#include "ISSegCurve.h"
#include "UPoint.h"
#include "MBool.h"

namespace fmr {

/*

3 Declaration for the class ~MPoint~

*/
class MPoint {
public:
    MPoint() {}
    virtual ~MPoint() {}
    MPoint(RList& l);
    MBool inside (FMRegion& fmr);
    
    std::vector<UPoint> units;
private:

};

}

#endif  /* MPOINT_H */