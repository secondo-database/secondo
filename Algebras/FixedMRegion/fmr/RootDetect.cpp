/* 
 * This file is part of libfmr
 * 
 * File:   RootDetect.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 14, 2016, 2:44 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~RootDetect~

[TOC]

1 Overview

This class determines the roots of the difference of two parametric
curves ~c1~ and ~c2~ (i.e. the times where they intersect).
For this, the relevant time range is sampled in uniform steps
(see STEP below) and if a transition from positive to negative
(or vice-versa) is found, a binary search for the exact root is
performed.
If STEP is chosen too big, some roots might not be detected.

*/

#include "RootDetect.h"

#define STEP 0.0001

using namespace fmr;
/*
2 Constructor

Construct from two curves ~c1~ and ~c2~

*/
RootDetect::RootDetect(Curve& _c1, Curve& _c2) : c1(&_c1), c2(&_c2) {
}

/*
3 ~findRoots~

Find and return all roots between ~cur~ and ~end~.

*/
std::vector<double> RootDetect::findRoots (double cur, double end) {
    std::vector<double> ret;
    double val = _f(cur);
        
    double prev = val;
    while (cur < end) {
        if (val != 0)
            prev = val;
        val = _f(cur);
        if ((prev < 0 && val > 0) || (prev > 0 && val < 0)) {
             // A transition between + and - has been found
            double root = findRootBinary(cur - STEP, cur, prev < 0);
            if (!isnan(root)) {
                ret.push_back(root);
            }
        }
        // Advance to the next value
        cur += STEP;
    }
    
    return ret;
}

/*
4 ~findRootBinary~

Find the exact root between two borders ~l~ and ~r~. The parameter
~rising~ determines, if the transition is from - to + (true) or from
+ to - (false).
Aborts after ~maxiter~ iterations if the root cannot be determined
(in some cases it might not be a real root, but a jump in the function)

*/
double RootDetect::findRootBinary (double l, double r, bool rising) {
    double val, m;
    int maxiter = 100;
    do {
        m = (l + r) / 2; // determine the middle of the interval
        val = f(m);      // and the corresponding value
        if ((val < 0 && rising) || (val > 0 && !rising))
            l = m; // m is the new left border
        else
            r = m; // m is the new right border
        
        if (maxiter-- <= 0) {
            return nan(""); // No root found here
        }
        
    // Do also a minimum of maxiter iterations to enhance accuracy
    } while (std::abs(val) > PRECISION || maxiter > 0);
    
    return m;
}

/*
5 ~findIntersection~

Find the intersections between the two curves.

*/
std::vector<Point> RootDetect::findIntersection() {
    std::vector<Point> ret;
    
    // findIntersectionTimes must be implemented by some subclass
    std::vector<std::pair<double, double> > its = findIntersectionTimes();
    for (int i = 0; i < its.size(); i++) {
        // t1 is the intersection time of curve ~c1~
        double t1 = its[i].first;
        // t2 is the intersection time of curve ~c2~
        double t2 = its[i].second;
        // Calculate the intersection points at times ~t1~ and ~t2~
        Point p = c1->project(t1);
        Point p2 = c2->project(t2);
        // If ~p~ is not near ~p2~, something went wrong.
        if (p.near(p2)) {
            // Otherwise, add the intersection information to the curves
            c1->addIntersection(t1, t2, c2);
            c2->addIntersection(t2, t1, c1);
        }
        
        ret.push_back(p);
    }
    
    return ret;
}
