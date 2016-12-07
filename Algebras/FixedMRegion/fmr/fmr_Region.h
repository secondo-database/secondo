/* 
----
 * This file is part of libfmr
 * 
 * File:   Region.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 3:20 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Region

[TOC]

1 Overview

Header file with the class definition for the class ~Region~

2 Includes and definitions

*/

#ifndef FMR_REGION_H
#define FMR_REGION_H

#include <vector>
#include <string>
#include "fmr_BoundingBox.h"
#include "fmr_RList.h"
#include "fmr_Face.h"
#include "fmr_TransformationUnit.h"

namespace fmr {

class FMRegion; // Forward declaration

/*
3 Definition of class ~Region~

*/
class Region {
public:
    // Constructors
    Region() {}
    Region(RList& l);
    virtual ~Region() {}
    
    // Methods
    bool inside (Point p);
    double area();
    Point centroid();
    Region transform(TransformationUnit& tu, double frac);
    FMRegion interpolate(Region& r, Interval iv);
    FMRegion interpolate(Region& r, Point center, Interval iv);
    BoundingBox boundingBox();
    std::string ToString();
    RList toRList();
    
    // Fields
    std::vector<Face> faces;

private:
    std::vector<Seg> getAllSegments();

};

}

#endif  /* REGION_H */

