/* 
----
 * This file is part of libfmr
 * 
 * File:   CRegion.h
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

#ifndef FMR_CREGION_H
#define FMR_CREGION_H

#include <vector>

#include "fmr_Seg.h"
#include "fmr_Region.h"
#include "fmr_RFace.h"
#include "fmr_RList.h"
#include "fmr_BoundingBox.h"

namespace fmr {

/*
3 Definition of class ~CRegion~

*/
class CRegion {
public:
    // Constructors
    CRegion() {}
    CRegion(RList& r);
    virtual ~CRegion() {}

    // Methods
    bool inside (Point p);
    bool intersects (Region& region);
    Region toRegion(int nrsegs);
    
    std::vector<Point> intersections(Seg s);
    BoundingBox boundingBox();
    RList toRList();
    
    // Fields
    std::vector<RFace> faces;
};

}

#endif  /* CREGION_H */
