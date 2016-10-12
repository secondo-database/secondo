/* 
----
 * This file is part of libfmr
 * 
 * File:   RCurve.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 20, 2016, 4:22 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RCurve

[TOC]

1 Overview

Header file with the class definition for the class ~RCurve~

2 Includes and definitions

*/

#ifndef FMR_RCURVE_H
#define FMR_RCURVE_H

#include <vector>


#include "Point.h"
#include "Seg.h"
#include "RList.h"
#include "Curve.h"
#include "BoundingBox.h"

namespace fmr {

class Curve; // Forward declaration

/*
3 Definition of class RCurve

*/
class RCurve {
public:
    // Constructors
    RCurve() {}
    RCurve(RList& r);
    virtual ~RCurve() {}
    
    // Methods
    Curve* getCurve();
    std::vector<Point> intersections (Seg s);
    BoundingBox boundingBox();
    std::vector<Seg> toSegs(int nrsegs);
    RList toRList();

    // Fields
    std::string type;
    Point off;
    double angle;
    std::vector<double> params;
    
private:
    BoundingBox bb;
};

}

#endif  /* RCURVE_H */
