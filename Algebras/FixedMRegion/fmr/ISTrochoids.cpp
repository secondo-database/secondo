/* 
 * This file is part of libfmr
 * 
 * File:   ISTrochoids.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 16, 2016, 4:53 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class ISTrochoids

[TOC]

1 Overview

This class calculates the intersection points between two Trochoids.

*/

#include "ISTrochoids.h"

using namespace fmr;

/*
2 Constructor from two Trochoids

*/
ISTrochoids::ISTrochoids(Trochoid& _o1, Trochoid& _o2) : o1(_o1), o2(_o2),
                                                         RootDetect(_o1, _o2) {}

/*
3 ~getT1~

Determins the times, where the Trochoid ~o1~ is at the same y-coordinate
as the Trochoid ~o2~ at time ~t2~. There are two possible values for each
time ~t2~, which are selected by ~first~.

*/
double ISTrochoids::getT1(double t2, bool first) {
    if (o1 == o2) {
        return ((first ? 0 : 2) * M_PI - t2 * o1.rot - 2 * o1.toff) / o1.rot;
    } else {
        double ac = (o2.yoff - o1.yoff - o2.a + o1.a +
                    o2.b * cos(o2.toff + t2 * o2.rot)) / o1.b;
        double val = (acos(ac) - o1.toff) / o1.rot;
        if (!first)
            //                return (-Math.acos(ac)-o1.toff)/o1.rot;
            val = (-val * o1.rot - 2 * o1.toff) / o1.rot;
        while (val > 2 * M_PI / o1.rot)
            val -= 2 * M_PI / o1.rot;
        while (val < 0)
            val += 2 * M_PI / o1.rot;
        return val;
    }
}

/*
4 ~f~

The roots of this function are the intersection times on ~o2~.

*/
double ISTrochoids::f(double t2) {
    double ret;
    double t1 = getT1(t2, first);
    if (o1 == o2) {
        ret = o1.a * (t1 - t2) * o1.rot +
              o1.b * (sin(o1.toff + t2 * o1.rot) - sin(o1.toff + t1 * o1.rot));
    } else {
        ret = (o1.xoff + o1.a * (o1.toff + t1 * o1.rot)
                - o1.b * sin(o1.toff + t1 * o1.rot))
                - (o2.xoff + o2.a * (o2.toff + t2 * o2.rot)
                - o2.b * sin(o2.toff + t2 * o2.rot));
    }

    return ret;
}

/*
5 ~findIntersectionTimes~
 
Determine the roots of ~f~ (4) via ~findRoots~ (defined in class ~RootDetect~)
and calculate all valid pairs of times taking into account that the graphs are
periodic with period ~pd~.
The parameter ~first~ selects one of two sections of ~o1~ in which
an intersection is looked for.

*/
std::vector<std::pair<double, double> > ISTrochoids::findIntersectionTimes(bool
                                                                        first) {
    this->first = first;
    std::vector<double> roots = findRoots(-1, 1); // Determine all roots
    std::vector<std::pair<double, double> > ret;
    
    double pd = 2 * M_PI / o1.rot; // The period of the Trochoids
    for (int i = 0; i < roots.size(); i++) {
        double t2 = roots[i], t1 = getT1(t2, first);
        if (o1 == o2 && ((std::abs(t1-t2) < PRECISION) || (t1 > t2)))
            continue; // Eliminate duplicates and errors on selfintersection
        // Find all valid intersection pairs (t1,t2)
        while (t1 > pd || t2 > pd) {
            t1 -= pd;
            t2 -= pd;
        }
        while (t1 < 1 && t2 < 1) {
            if (t1 > 0 && t2 > 0) { // 0 < t1/t2 < 1 ?
                ret.push_back(std::pair<double, double>(t1, t2));
            }
            t1 += pd; // Advance one period
            t2 += pd; // in both graphs
        }
    }
    
    return ret;
}


/*
6 ~findIntersectionTimes~

Calculate both kinds of intersections and merge them here.

*/
std::vector<std::pair<double, double> > ISTrochoids::findIntersectionTimes() {
    std::vector<std::pair<double, double> > ret, tmp;
    
    ret = findIntersectionTimes(true);
    tmp = findIntersectionTimes(false);
    ret.insert(ret.end(), tmp.begin(), tmp.end());
    
    return ret;
}
