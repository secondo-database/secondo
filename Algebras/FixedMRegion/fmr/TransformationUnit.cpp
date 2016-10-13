/* 
 * This file is part of libfmr
 * 
 * File:   TransformationUnit.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 4:54 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~TransformationUnit~

[TOC]

1 Overview

A TransformationUnit describes the movement of an object over a
time interval. Movement is a combination of translation and
rotation. An initial translation and rotation can be specified.

Parameters:

c : center point of rotation (relative to object)

v0: initial translation
a0: initial rotation

v : translation during time interval
a : rotation during time interval
iv: time interval

A Point ~p~ at time ~curtime~ is then transformed to:

(p.rotate(c, a0) + v0).rotate(c+v0, a*t) + v*t

where 0 < t < 1 is a fraction of the time interval iv:
t = (curtime - iv.start)/(iv.end - iv.start)

*/

#include <sstream>
#include <iostream>
#include "fmr_TransformationUnit.h"

using namespace fmr;

/*
2 Constructor

Constructs a static transformation unit (no movement).

*/
TransformationUnit::TransformationUnit() : a0(0), a(0) {
}

/*
3 Constructor

Constructs a transformation unit without initial displacement/rotation.

*/
TransformationUnit::TransformationUnit(Point c, Point v, double a) 
                          : c(c), v0(Point(0,0)), v(v), a0(0), a(a) {
}

/*
4 Constructor

Constructs a transformation unit from all parameters.

*/
TransformationUnit::TransformationUnit(Point c, Point v0, Point v, double a0,
            double a, Interval iv) : c(c), v0(v0), v(v), a0(a0), a(a), iv(iv) {}

/*
5 Construct from RList

Construct a TransformationUnit from its RList representation.

*/
TransformationUnit::TransformationUnit(RList& l) {
    c = Point(l[0]);
    v0 = Point(l[1]);
    v = Point(l[2]);
    a0 = (double) l[3];
    a = (double) l[4];
    iv = Interval(l[5]);
}

/*
6 ~restrict~

Create a new TransformationUnit from this one restricted to the
time interval ~niv~.

*/
TransformationUnit TransformationUnit::restrict (Interval niv) {
    double st = ((niv.start - iv.start)/(iv.end - iv.start));
    double et = ((niv.end - iv.start)/(iv.end - iv.start));
    
    TransformationUnit ret;
    
    ret.a0 = a0 + a*st;
    ret.v0 = v0 + v*st;
    ret.a  =  a * (et-st);
    ret.v  =  v * (et-st);
    ret.c  =  c + v*st;
    ret.iv = niv;
    
    return ret;
}

/*
7 ~ToString~

Return a string representation of this TransformationUnit.

*/
std::string TransformationUnit::ToString() {
    std::stringstream ss;
    ss << "( " << c.ToString() << " " << v0.ToString() << " " << v.ToString() <<
           " " << a0 << " " << a << " " << iv.ToString() << " )";
    
    return ss.str();
}

/*
8 ~toRList~

Return an RList representation of this TransformationUnit.

*/
RList TransformationUnit::toRList() {
    RList ret;
    
    ret.append(c.toRList());
    ret.append(v0.toRList());
    ret.append(v.toRList());
    ret.append(a0);
    ret.append(a);
    ret.append(iv.toRList());
    
    return ret;
}
