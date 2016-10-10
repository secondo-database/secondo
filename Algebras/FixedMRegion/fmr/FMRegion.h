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

#include "RList.h"
#include "Region.h"
#include "Region2.h"
#include "TransformationUnit.h"
#include "BoundingBox.h"

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
    Region2 traversedArea();
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