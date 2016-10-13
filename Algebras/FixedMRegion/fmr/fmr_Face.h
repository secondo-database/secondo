/* 
----
 * This file is part of libfmr
 * 
 * File:   Face.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 9, 2016, 3:21 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Curve

[TOC]

1 Overview

Header file with the class definition for the class ~Face~.

2 Includes and definitions

*/

#ifndef FMR_FACE_H
#define FMR_FACE_H

#include <vector>
#include <string>
#include "fmr_TransformationUnit.h"
#include "fmr_RList.h"
#include "fmr_Seg.h"

namespace fmr {

/*

3 Definition of the class ~Face~

*/
class Face {
public:
    Face();
    Face(RList& l);
    virtual ~Face() {}
    void addSeg (Seg s);
    void addPoint (Point p);
    void close();
    std::string ToString();
    RList toRList();
    Face transform(TransformationUnit& tu, double frac);
    double area();
    std::pair<double, double> centroidParams();
    bool inside (Point p);
    
    std::vector<Seg> segs;
    std::vector<Face> holes;
    
protected:
    void readRList(RList& l);
    
private:
    bool lastPointValid;
    Point lastPoint;
};

}

#endif  /* FACE_H */
