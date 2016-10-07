/* 
 * This file is part of libfmr
 * 
 * File:   ISSegCurve.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 14, 2016, 4:17 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class ISSegCurve

[TOC]

1 Overview

This class calculates the intersection points between a curve and a
line segment.

*/


#include "ISSegCurve.h"

using namespace fmr;

/*
2 Constructor from a line segment and a curve

*/
ISSegCurve::ISSegCurve(SegT& _segt, Curve& _curve) : segt(_segt),
                                      curve(_curve), RootDetect(_segt, _curve) {
    Seg& seg = segt.seg;
    
    if (seg.f.x == seg.i.x) { // Line is parallel to y-axis
        m = nan("");
        c = seg.i.x; // x = c
    } else {
        // Initialize values m and c, so that the segment is
        // on the line y = m*x + c
        m = (seg.f.y - seg.i.y) / (seg.f.x - seg.i.x);
        c = seg.i.y - m * seg.i.x;
    }
}

/*
3 ~f~

The roots of this function are the intersection times on the ~curve~

*/
double ISSegCurve::f(double t) {
    if (isnan(m)) // Line is parallel to y-axis
        return curve.fx(t) - c;
    else
        return curve.fy(t) - m * curve.fx(t) - c;
}

/*
4 ~findIntersectionTimes~

Determine the roots of ~f~ (3) via ~findRoots~ (defined in class ~RootDetect~)
and calculate all valid pairs of times.

*/
std::vector<std::pair<double, double> > ISSegCurve::findIntersectionTimes() {
    std::vector<std::pair<double, double> > ret;
    
    std::vector<double> roots = findRoots(0, 1);
        
    for (int i = 0; i < roots.size(); i++) {
        double root = roots[i];
        Point p = curve.project(root);
        // Calculate the time of the intersection
        // with the parametric line segment function
        // This is outside the range [0;1] if the intersection
        // point is outside the line segment
        double segtime = segt.getTime(p);
        
        // Only accept the intersection, if both curves are defined
        // at the corresponding intersection times.
        // This is for example not the case, if segtime is outside
        // [0;1]
        if (curve.valid(root) && segt.valid(segtime)) {
            ret.push_back(std::pair<double, double>(segtime, root));
        }
    }
    
    // Extra test for the endpoints of the curve (at times 0 and 1)
    for (double root = 0.0; root < 2; root += 1.0) {
        if (!curve.valid(root))
            continue; // Curve is not defined here
        if (std::abs(f(root)) < PRECISION) {  // We have found a root of f()
            Point p = curve.project(root);    // Calculate intersection point
            double segtime = segt.getTime(p); // Calculate segment time of is
            if (!segt.valid(segtime)) // Did we find a valid segment time?
                continue;
            ret.push_back(std::pair<double,double>(segtime, root));
        }
    }
    
    return ret;
}
