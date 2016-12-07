/* 
----
 * This file is part of libfmr
 * 
 * File:   FMRegion.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 6, 2016, 11:50 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class FMRegion

[TOC]

1 Overview

Header file with the class definition for the class ~FMRegion~.

2 Includes and definitions

*/

#ifndef FMR_FMREGION_H
#define FMR_FMREGION_H

#include <vector>
#include <string>
#include <sstream>

#include "fmr_MPoint.h"
#include "fmr_RList.h"
#include "fmr_MBool.h"
#include "fmr_Region.h"
#include "fmr_CRegion.h"
#include "fmr_TransformationUnit.h"
#include "fmr_BoundingBox.h"

namespace fmr {

class Region; // Forward declaration

/*

3 ~Class FMRegion~

The class definition of a Fixed Moving Region

*/
class FMRegion {
public:
    // Constructors
    FMRegion() {}
    FMRegion(Region r, TransformationUnit tu);
    FMRegion(RList& l);
    virtual ~FMRegion() {}
    
    // Methods
    Region atinstant (double time);
    CRegion traversedArea();
    MBool inside (MPoint& mp) { return mp.inside(*this); }
    MPoint intersection (MPoint& mp) { return mp.intersection(*this); }
    FMRegion interpolate (Region& r1, Region& r2, Interval& iv) {
        return r1.interpolate(r2, iv);
    }
    
    BoundingBox boundingBox();
    std::string ToString();
    RList toRList();
    
    // Fields
    Region region; // The shape of the moving region
    std::vector<TransformationUnit> trafos; // Transformation units

private:
    TransformationUnit* findTransformationUnit(double time);
};

}

#endif  /* FMREGION_H */