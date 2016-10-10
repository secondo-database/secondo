/* 
----
 * This file is part of libfmr
 * 
 * File:   Region2.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 20, 2016, 10:50 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RCurve

[TOC]

1 Overview

Header file with the class definition for the class ~RFace~

2 Includes and definitions

*/

#ifndef FMR_REGION2_H
#define FMR_REGION2_H

#include <vector>

#include "Seg.h"
#include "Region.h"
#include "RFace.h"
#include "RList.h"
#include "BoundingBox.h"

namespace fmr {

/*
3 Definition of class ~Region2~

*/
class Region2 {
public:
    // Constructors
    Region2() {}
    Region2(RList& r);
    virtual ~Region2() {}

    // Methods
    bool inside (Point p);
    std::vector<Point> intersections(Seg s);
    bool intersects (Region& region);
    BoundingBox boundingBox();
    RList toRList();
    
    // Fields
    std::vector<RFace> faces;
};

}

#endif  /* REGION2_H */