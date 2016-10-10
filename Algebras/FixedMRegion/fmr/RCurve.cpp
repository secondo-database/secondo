/* 
 * This file is part of libfmr
 * 
 * File:   RCurve.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 20, 2016, 4:22 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~RCurve~

[TOC]

1 Overview

The class ~RCurve~ describes curved line segments from which a ~RFace~ of
a ~Region2~ is constructed.
The RCurve is defined by:

Curve type: ~type~
Startpoint: ~off~
Angle: ~angle~
Type-specific parameters: ~params~

Currently, three types are defined:

T : A trochoidal line segment
Parameters: <a> <b> <toff> <rotation>
Parametric functions:
fx(t) = a * t * rot - b * (sin(t * rot + toff) - sin(toff))
fy(t) = b * (cos(t * rot + toff) - cos(toff))

R : A ravdoidal line segment
Parameters: <hp> <cd> <toff> <rotation>
Parametric functions:
fx(t) = hp * (2 * t * rot - sin(2 * (t * rot + toff)) + sin(2 * toff)) +
        cd *               (cos     (t * rot + toff)  - cos(    toff))
fy(t) = hp * (cos(2 * (t * rot + toff)) - cos(2 * toff)) +
        cd * (sin     (t * rot + toff)  - sin(    toff))

S : A straight line segment
Parameters: <xd> <yd>
Parametric functions:
fx(t) = xd * t
fy(t) = yd * t

When implementing more types, it should be ensured that fx(0) = fy(0) = 0,
so that the point ~off~ is really the startpoint of the line segment.

A point on the curve at time ~t~ is then calculated as:
Point(fx(t), fy(t)).rotate(~angle~) + ~off~

*/

#include "RCurve.h"
#include "Trochoid.h"
#include "Ravdoid.h"
#include "SegT.h"
#include "ISSegCurve.h"

#define MIN(a,b,c,d) std::min(std::min(std::min(a,b),c),d)
#define MAX(a,b,c,d) std::max(std::max(std::max(a,b),c),d)

using namespace fmr;

/*
2 Construct from RList

Constructs an ~RCurve~ object from an RList representation.

*/
RCurve::RCurve(RList& r) {
    off = Point(r[0].getNr(), r[1].getNr());
    angle = r[2].getNr();
    type = r[3].getSym();
    for (int i = 4; i < r.size(); i++) {
        params.push_back(r[i].getNr());
    }
}

/*
3 ~getCurve~

Reconstructs the underlying curvetype (trochoid, ravdoid, segment)
from this RCurve. These are used for example for intersection checks then.

*/
Curve* RCurve::getCurve() {
    Curve *ret;
    
    if (type == "T") { // Construct a Trochoid
        double a = params[0];
        double b = params[1];
        double toff = params[2];
        double rot = params[3];
        double xoff = off.x - a*toff + b*sin(toff);
        double yoff = off.y + a - b*cos(toff);
        
        ret = new Trochoid(a, b, xoff, yoff, toff, rot);
    } else if (type == "R") { // Construct a Ravdoid
        double hp = params[0];
        double cd = params[1];
        double toff = params[2];
        double rot = params[3];
        double xoff = off.x - hp*(2*toff - sin(2*toff)) - cd * cos(toff);
        double yoff = off.y - hp*cos(2*toff) - cd * sin(toff);
        
        ret = new Ravdoid(hp, cd, xoff, yoff, toff, NAN, NAN, NAN, NAN, rot);
    } else if (type == "S") { // Construct a line segment
        ret = new SegT(Seg(Point(off.x, off.y),
                           Point(off.x+params[0], off.y+params[1])));
    } else
        return NULL;
    
    ret->setTransformation(off, angle); // Copy the stored transformation
    
    return ret;
}

/*
4 ~intersections~

Calculate all intersections of a line segment with this ~RCurve~ segment.

*/
std::vector<Point> RCurve::intersections (Seg s) {
    Curve *c = getCurve(); // First, construct the corresponding Curve
    if (!c) { // Can fail if the type is invalid/unsupported by getCurve()
        std::vector<Point> none;
        return none;
    }
    
    SegT segt(s.rotate(off, -angle)); // Correct the orientation of the segment
    ISSegCurve is(segt, *c);
    
    std::vector<Point> ret = is.findIntersection(); // Get intersections
    // Undo the transformation above
    for (int nrpoint = 0; nrpoint < ret.size(); nrpoint++) {
        ret[nrpoint] = ret[nrpoint].rotate(off, angle);
    }
    
    return ret;
}

/*
5 ~boundingBox~

Calculate the bounding box of this line segment. This is not necessarily a
minimal bounding box.

*/
Seg RCurve::boundingBox() {
    Seg bb;
    
    if (type == "T") {
        // Curve type is Trochoid, expand parameters
        double a = params[0];
        double b = params[1];
        double toff = params[2];
        double rot = params[3];
        double xoff = off.x - a*toff + b*sin(toff);
        double yoff = off.y + a - b*cos(toff);
        
        // Calculate maximum possible x and y values
        double x1 = xoff + a*toff - b;
        double x2 = xoff + a*(toff+rot) + b;
        double y1 = yoff - a - b;
        double y2 = yoff - a + b;
        // and create the resulting bounding box
        bb.i.x = std::min(x1, x2);
        bb.i.y = std::min(y1, y2);
        bb.f.x = std::max(x1, x2);
        bb.f.y = std::max(y1, y2);
    } else if (type == "R") {
        // Curve type is Ravdoid, expand parameters
        double hp = params[0];
        double cd = params[1];
        double toff = params[2];
        double rot = params[3];
        double xoff = off.x - hp*(2*toff - sin(2*toff)) - cd * cos(toff);
        double yoff = off.y - hp*cos(2*toff) - cd * sin(toff);
        
        // Calculate maximum possible x and y values
        double x1 = xoff + hp * (2* toff      - 1) - cd;
        double x2 = xoff + hp * (2*(toff+rot) + 1) + cd;
        double y1 = yoff - hp - cd;
        double y2 = yoff + hp + cd;
        // and create the resulting bounding box
        bb.i.x = std::min(x1, x2);
        bb.i.y = std::min(y1, y2);
        bb.f.x = std::max(x1, x2);
        bb.f.y = std::max(y1, y2);
    } else if (type == "S") {
        // Curve type is Straight segment, calculate start and end point
        Point start(off.x, off.y);
        Point end(off.x+params[0], off.y+params[1]);
        // and create the resulting bounding box
        bb.i.x = std::min(start.x, end.x);
        bb.i.y = std::min(start.y, end.y);
        bb.f.x = std::max(start.x, end.x);
        bb.f.y = std::max(start.y, end.y);
    }
    
    // The segment is possibly rotated, rotate the corners of the bounding box
    Point p1 = Point(bb.i.x, bb.i.y).rotate(off, angle);
    Point p2 = Point(bb.f.x, bb.i.y).rotate(off, angle);
    Point p3 = Point(bb.i.x, bb.f.y).rotate(off, angle);
    Point p4 = Point(bb.f.x, bb.f.y).rotate(off, angle);
    // and calculate the bounding box of that rotated box.
    bb.i.x = MIN(p1.x, p2.x, p3.x, p4.x);
    bb.f.x = MAX(p1.x, p2.x, p3.x, p4.x);
    bb.i.y = MIN(p1.y, p2.y, p3.y, p4.y);
    bb.f.y = MAX(p1.y, p2.y, p3.y, p4.y);
    
    return bb;
}

/*
6 ~toRList~

Return the RList representation for this RCurve.

*/
RList RCurve::toRList() {
    RList ret;
    
    ret.append(off.x);
    ret.append(off.y);
    ret.append(angle);
    ret.appendSym(type);
    for (int i = 0; i < params.size(); i++) {
        ret.append(params[i]);
    }
    
    return ret;
}
