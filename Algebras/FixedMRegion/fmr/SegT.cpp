/* 
 * This file is part of libfmr
 * 
 * File:   SegT.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 15, 2016, 3:38 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of class SegT

[TOC]

1 Overview

The class ~SegT~ represents a curve of the form of a straight
line segment. It is one of the three curve types which make
up the border of the traversed area of an FMRegion.

*/

#include "SegT.h"
#include "Curve.h"

using namespace fmr;

/*
3 Constructor

Construct a SegT object from a Seg(ment)

*/ 
SegT::SegT(Seg seg) : seg(seg) {
}

/*
4 ~project~

Calculate the point on the Segment at time fraction ~t~
([0;1]).

*/
Point SegT::project (double t) {
    return seg.i + (seg.f-seg.i)*t;
}

/*
5 ~create~

Create a SegT object from a Segment ~seg~ of a region and a
TransformationUnit ~tu~. The initial displacement and rotation
is taken into account here. The parameter ~start~ chooses, if
the result represents the segment at the beginning or at the
end of the time interval represented by ~tu~

*/
SegT SegT::create (Seg seg, TransformationUnit& tu, bool start) {
    // Initial transformation
    seg = seg.rotate(tu.c, tu.a0) + tu.v0;
    // Transformed center point
    Point c = tu.c + tu.v0;
    
    if (!start) {
        // End segment requested, rotate by tu.a and translate by tu.v
        seg = seg.rotate(c, tu.a) + tu.v;
    }

    // Transform the segment to match the (normalized) orientation
    // of all other segments (the y parameter of tu.v is 0),
    // otherwise the intersections cannot be calculated.
    double vangle = tu.v.angle();
    seg = seg.rotate(c, -vangle);
    SegT segt(seg); // Create the new object here

    // Remember the orientation before normalization here to revert it
    // later
    segt.setTransformation(c, vangle);
    
    return segt;
}

/*
6 ~getTime~

Calculates the time at which the curve is at Point ~p~.

*/
double SegT::getTime(Point p) {
    double t;
    double xd = seg.f.x - seg.i.x;
    double yd = seg.f.y - seg.i.y;
    
    
    if (xd == 0 && yd == 0)
        return 0; // Segment is degenerated to a point
    
    // Choose the coordinate with the bigger span to be
    // numerically more stable
    if (std::abs(xd) > std::abs(yd)) {
        // Use x coordinate
        double x = p.x - seg.i.x;
        t = x / xd;
    } else {
        // Use y coordinate
        double y = p.y - seg.i.y;
        t = y / yd;
    }
    
    // If the time is very near to 0 or 1, assume 0 or 1 because
    // the point is probably located on an endpoint of this segment
    if (std::abs(t) < PRECISION) {
        t = 0;
    } else if (std::abs(1.0 - t) < PRECISION) {
        t = 1;
    }
    
    // We now know the time t, at which the curve is at the
    // same x (resp. y) coordinate as p. Now test, if the
    // point is really _on_ the line segment.
    if (project(t).near(p)) // p is really on the line segment
        return t;
    else                    // p is not on the line segment
        return nan("");
}

/*
7 ~rcurve~

Create an RCurve from this curve between instants ~t1~ and ~t2~

*/
RCurve SegT::rcurve (double t1, double t2) {
    RCurve ret;
    
    ret.type = "S";
    ret.angle = t_angle;
    ret.off = project(t1).rotate(t_center, t_angle);    
    ret.params.push_back((seg.f.x-seg.i.x)*(t2-t1));
    ret.params.push_back((seg.f.y-seg.i.y)*(t2-t1));
    
    return ret;
}

/*
8 ~ToString~

Create a string representation of this SegT object.

*/
std::string SegT::ToString() {
    std::stringstream ss;
    ss << "SegT; seg:" << seg.ToString();
    
    return ss.str();
}
