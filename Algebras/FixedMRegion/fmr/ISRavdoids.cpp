/* 
 * This file is part of libfmr
 * 
 * File:   ISRavdoids.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 19, 2016, 12:09 AM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class ISRavdoids

[TOC]

1 Overview

This class calculates the intersection points between two Ravdoids.

*/

#include "ISRavdoids.h"

using namespace fmr;

/*
2 Constructor from two Ravdoids

*/
ISRavdoids::ISRavdoids(Ravdoid& _o1, Ravdoid& _o2) : o1(_o1), o2(_o2),
        RootDetect(_o1, _o2) { }

/*
3 ~getT1~

Determins the times, where the Ravdoid ~o1~ is at the same y-coordinate
as ~o2~ at time ~t2~. There are up to four possible values for each time
~t2~, which are selected by ~first~ and ~second~.

*/
double ISRavdoids::getT1(double t2, bool first, bool second) {
    double qq = o2.hp * cos(2 * (t2 * o2.rot + o2.toff)) +
                o2.cd * sin(t2 * o2.rot + o2.toff) + o2.yoff;
    double as = (-o1.cd + (second ? 1 : -1) * // +
                sqrt(o1.cd * o1.cd + 8 * o1.hp * (o1.hp + o1.yoff - qq)))
                / (-4 * o1.hp);
    double val = (asin(as) - o1.toff) / o1.rot;
    if (!isnan(val) && !first) {
        //            val = (M_PI-asin(as)-o1.toff)/o1.rot;
        val = M_PI / o1.rot - val - 2 * o1.toff / o1.rot;
    }

    return val;
}

/*
4 ~f~

The roots of this function are the intersection times on ~o2~.

*/
double ISRavdoids::f(double t2) {
    double t1 = getT1(t2, first, second);

    t1 = o1.toff + t1 * o1.rot;
    t2 = o2.toff + t2 * o2.rot;

    double r = (o1.hp * (2 * t1 - sin(2 * t1)) + o1.cd * cos(t1) + o1.xoff)
             - (o2.hp * (2 * t2 - sin(2 * t2)) + o2.cd * cos(t2) + o2.xoff);

    return r;
}

/*
5 ~findIntersectionTimes~
 
Determine the roots of ~f~ (4) via ~findRoots~ (defined in class ~RootDetect~)
and calculate all valid pairs of times taking into account that the graphs are
periodic with period ~pd~.
The parameters ~first~ and ~second~ select one of four sections of ~o1~ in which
an intersection is looked for.

*/
std::vector<std::pair<double, double> >
                    ISRavdoids::findIntersectionTimes(bool first, bool second) {
    this->first = first;
    this->second = second;
    std::vector<double> roots = findRoots(-1, 1); // Determine all roots
    std::vector<std::pair<double, double> > ret;

    double pd = 2 * M_PI / o1.rot;  // The period of the Ravdoids
    for (int i = 0; i < roots.size(); i++) {
        double t2 = roots[i], t1 = getT1(t2, first, second);
        if (o1 == o2 && ((std::abs(t1 - t2) < 0.001) || (t1 > t2)))
            continue; // Eliminate errors and duplicates on selfintersections
        // Find all valid intersection pairs (t1,t2)
        while (t1 > pd || t2 > pd) {
            t1 -= pd;
            t2 -= pd;
        }
        while (t1 < 1 && t2 < 1) {
            if (o1.valid(t1) && o2.valid(t2)) { // Are both Ravdoids defined
                                                // at the intersection times?
                ret.push_back(std::pair<double, double>(t1, t2));
            }
            t1 += pd; // Advance one period 
            t2 += pd; // in the graphs o1 and o2
        }
    }

    return ret;
}

/*
6 ~findIntersectionTimes~
 
Find all intersection times for all combinations of ~first~ and ~second~

*/
std::vector<std::pair<double, double> > ISRavdoids::findIntersectionTimes() {
    std::vector<std::pair<double, double> > ret, tmp;

    ret = findIntersectionTimes(false, false);
    tmp = findIntersectionTimes(false, true);
    ret.insert(ret.end(), tmp.begin(), tmp.end());
    tmp = findIntersectionTimes(true, false);
    ret.insert(ret.end(), tmp.begin(), tmp.end());
    tmp = findIntersectionTimes(true, true);
    ret.insert(ret.end(), tmp.begin(), tmp.end());

    return ret;
}
