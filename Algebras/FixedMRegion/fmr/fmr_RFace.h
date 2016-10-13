/* 
----
 * This file is part of libfmr
 * 
 * File:   RFace.h
 * Author: flo
 *
 * Created on September 20, 2016, 10:48 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RFace

[TOC]

1 Overview

Header file with the class definition for the class ~RFace~

2 Includes and definitions

*/

#ifndef FMR_RFACE_H
#define FMR_RFACE_H

#include <vector>

#include "fmr_Seg.h"
#include "fmr_Face.h"
#include "fmr_RFace.h"
#include "fmr_RCurve.h"
#include "fmr_RList.h"
#include "fmr_BoundingBox.h"

namespace fmr {

/*
3 Definition of class RFace

*/
class RFace {
public:
    // Constructors
    RFace() {}
    RFace(RList& r);
    virtual ~RFace()  {}

    // Methods
    std::vector<Point> intersections(Seg s);
    bool inside (Point p);
    bool intersects (Face& f);
    BoundingBox boundingBox();
    Face toFace(int nrsamples);
    RList toRList();
    
    // Fields
    std::vector<RCurve> face;
    std::vector<RFace> holes;
private:
    void getCycle(RList& r);

};

}

#endif  /* RFACE_H */
