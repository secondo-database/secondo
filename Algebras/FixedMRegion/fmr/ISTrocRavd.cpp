/* 
----
 * This file is part of libfmr
 * 
 * File:   ISTrocRavd.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 19, 2016, 12:38 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class ISTrocRavd

[TOC]

1 Overview

This class calculates the intersection points between a
Trochoid and a Ravdoid.

*/

#include "ISTrocRavd.h"

using namespace fmr;

/*
2 Constructor from a Trochoid and a Ravdoid

*/
ISTrocRavd::ISTrocRavd(Trochoid& _o1, Ravdoid& _o2) : o1(_o1), o2(_o2),
                                                          RootDetect(_o1, _o2) {
}

/*
3 ~getT1~

Determins the times, where the Trochoid ~o1~ is at the same y-coordinate
as the Ravdoid ~o2~ at time ~t2~. There are two possible values for each
time ~t2~, which are selected by ~first~.

*/
double ISTrocRavd::getT1(double t2, bool first) {
    double ac = (o2.hp * cos(2 * (o2.toff + t2 * o2.rot))
            + o2.cd * sin(o2.toff + t2 * o2.rot)
            + o2.yoff
            - o1.yoff + o1.a) / o1.b;
    double val = (acos(ac) - o1.toff) / o1.rot;
    if (!isnan(val) && !first) {
        val = -val - 2 * o1.toff / o1.rot;
    }

    return val;
}

/*
4 ~f~

The roots of this function are the intersection times on ~o2~.

*/
double ISTrocRavd::f(double t2) {
    double t1 = getT1(t2, first);

    t1 = o1.toff + t1 * o1.rot;
    t2 = o2.toff + t2 * o2.rot;

    double val = (o1.xoff + o1.a * t1 - o1.b * sin(t1))
            - (o2.hp * (2 * t2 - sin(2 * t2)) + o2.cd * cos(t2) + o2.xoff);
        
    return val;
}

/*
5 ~findIntersectionTimes~
 
Determine the roots of ~f~ (4) via ~findRoots~ (defined in class ~RootDetect~)
and calculate all valid pairs of times taking into account that the graphs are
periodic with period ~pd~.
The parameter ~first~ selects one of two sections of ~o1~ in which
an intersection is looked for.

*/
std::vector<std::pair<double, double> >
                                 ISTrocRavd::findIntersectionTimes(bool first) {
    this->first = first;
    std::vector<double> roots = findRoots(-1, 1); // Determine all roots

    double pd = 2.0 * M_PI / o1.rot; // The period of the functions
    std::vector<std::pair<double, double> > ret;
    for (int i = 0; i < roots.size(); i++) {
        double t2 = roots[i], t1 = getT1(t2, first);
        // Find all valid intersection pairs (t1,t2)
        while (t1 >= pd && t2 >= pd) {
            t1 -= pd;
            t2 -= pd;
        }
        while (t1 < 1 && t2 < 1) {
            if (o1.valid(t1) && o2.valid(t2)) {// Both curves defined at t1/t2?
                ret.push_back(std::pair<double, double>(t1, t2));
            }
            t1 += pd; // Advance one period
            t2 += pd; // in both graphs
        }
    }

    return ret;
}

/*
6 ~findIntersectionTimesTouch~

A Ravdoid splits/merges from/to a Trochoid. These are also intersections which
have to be taken into account. In this case, the intersection times are
identical.
All borders of the ravdoids definition intervals (tmin/tmax) have to be
examined.

*/
std::vector<std::pair<double, double> >ISTrocRavd::findIntersectionTimesTouch(){
    double pd = 2.0 * M_PI / o1.rot; // The period of the graphs
    // Calculcate the border times of the definition intervals [tmin1;tmax1]
    // and [tmin2;tmax2]
    double tmin1 = o2.tmin1 / o2.rot;
    double tmax1 = o2.tmax1 / o2.rot;
    double tmin2 = !isnan(o2.tmin2) ? o2.tmin2 / o2.rot : nan("");
    double tmax2 = !isnan(o2.tmax2) ? o2.tmax2 / o2.rot : nan("");
    while (tmin1 > pd)
        tmin1 -= pd;
    while (tmax1 > pd)
        tmax1 -= pd;
    while (tmin2 > pd)
        tmin2 -= pd;
    while (tmax2 > pd)
        tmax2 -= pd;

    std::vector<std::pair<double, double> > ret;
    double times[] = {tmin1, tmax1, tmin2, tmax2};
    for (int i = 0; i < 4; i++) { // Check for all four times
        double tt = times[i];
        if (isnan(tt))
            continue;
        if (o1.project(tt).near(o2.project(tt))) { // Do they touch?
            while (tt < 1) {
                if (tt >= 0) {
                    ret.push_back(std::pair<double, double>(tt, tt));
                }
                tt += pd;
            }
        }
    }
    
    return ret;
}

/*
7 ~findIntersectionTimes~

Calculate all different kinds of intersections and merge them here.

*/
std::vector<std::pair<double, double> > ISTrocRavd::findIntersectionTimes() {
    std::vector<std::pair<double, double> > ret, tmp;

    ret = findIntersectionTimes(true);
    tmp = findIntersectionTimes(false);
    ret.insert(ret.end(), tmp.begin(), tmp.end());
    tmp = findIntersectionTimesTouch();
    ret.insert(ret.end(), tmp.begin(), tmp.end());

    return ret;
}