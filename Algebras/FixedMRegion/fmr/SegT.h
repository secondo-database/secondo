/* 
----
 * This file is part of libfmr
 * 
 * File:   SegT.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 15, 2016, 3:38 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class SegT

[TOC]

1 Overview

Header file with the class definition for the class ~SegT~

2 Includes and definitions

*/

#ifndef FMR_SEGT_H
#define FMR_SEGT_H


#include "Curve.h"
#include "Seg.h"
#include "TransformationUnit.h"

namespace fmr {

/*
3 Definition of class SegT

*/
class SegT : public Curve {
public:
    // Constructor
    SegT(Seg seg);
    virtual ~SegT() {}
    
    // Methods
    Point project (double t);
    double getTime (Point p);
    static SegT create (Seg s, TransformationUnit& tu, bool start);
    RCurve rcurve (double t1, double t2);
    std::string ToString();
    
    // Fields
    Seg seg;
};

}

#endif /* SEGT_H */