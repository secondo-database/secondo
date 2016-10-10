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

#include "Seg.h"
#include "Face.h"
#include "RFace.h"
#include "RCurve.h"
#include "RList.h"
#include "BoundingBox.h"

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
    RList toRList();
    BoundingBox boundingBox();
    
    // Fields
    std::vector<RCurve> face;
    std::vector<RFace> holes;
private:
    void getCycle(RList& r);

};

}

#endif  /* RFACE_H */
