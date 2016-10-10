/* 
----
 * File:   BoundingBox.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on October 10, 2016, 3:08 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class RCurve

[TOC]

1 Overview

Header file with the class definition for the class ~RCurve~

2 Includes and definitions

*/

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Point.h"

namespace fmr {

class BoundingBox {
public:
    // Constructors
    BoundingBox() {}
    virtual ~BoundingBox() {}
    
    // Methods
    void update (Point p);
    void update (BoundingBox bb);
    void rotate (Point center, double angle);
    bool valid () { return lowerLeft.valid() && upperRight.valid(); }

    // Fields
    Point lowerLeft, upperRight;
};

}
#endif /* BOUNDINGBOX_H */

