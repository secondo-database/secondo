/* 
 * This file is part of libfmr
 * 
 * File:   Face.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 3:21 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Face~

[TOC]

1 Overview

The class ~Face~ represents a polygon, which may contain one or more
holes. Several faces make up a ~Region~

2 Includes and definitions

*/

#include "fmr_Face.h"
#include <iostream>

using namespace fmr;

/*
3 Default constructor

*/
Face::Face() : lastPointValid(false) {
}

/*
4 Construct from RList

Construct the main cycle and the holes from an RList representation.
Format: (<main cycle><holes>[*])
Example:
( 
( ( 0 0 ) ( 100 0 ) ( 100 100 ) ) \_! main cycle !\_
( ( 5 5 ) (  95 5 ) (  95  95 ) ) \_! hole 1     !\_
)

*/
Face::Face(RList& l) : lastPointValid(false) {
    readRList(l[0]);
    for (int i = 1; i < l.size(); i++) {
        Face hole;
        hole.readRList(l[i]);
        holes.push_back(hole);
    }
}

/*
5 ~readRList~

Construct a single cycle from an RList representation.
This can either be the main cycle or a hole cycle.

*/
void Face::readRList(RList& l) {
    for (int i = 0; i < l.size(); i++) {
        double x = (double) l[i][0];
        double y = (double) l[i][1];
        addPoint(Point(x, y));
    }
    close();
}

/*
6 ~addSeg~

Adds the next line segment to this cycle.
 
*/
void Face::addSeg(Seg s) {
    segs.push_back(s);
}

/*
7 ~addPoint~

Creates and adds a new line segment from point ~p~ and
the previous point (if there is one) to this cycle.

*/
void Face::addPoint(Point p) {
    if (!lastPointValid) // There was no previous point
        lastPointValid = true;
    else {
        Seg s(lastPoint, p);
        addSeg(s);
    }
    lastPoint = p; // Remember the last point
}

/*
8 ~close~

Close the cycle by creating a line segment between the
endpoint of the last segment and the startpoint of
the first segment.

*/
void Face::close() {
    if (segs.size() >= 2) {
        Point last = segs[segs.size() - 1].f;
        Point first = segs[0].i;
        addSeg(Seg(last, first));
    }
}

/*
9 ~transform~

Transform this face according to the given transformation unit ~tu~ at
fraction ~frac~ in the time interval (i.e. 0.5 is the middle of the
time interval). This is done by transforming each line segment of this
cycle.

*/
Face Face::transform(TransformationUnit& tu, double frac) {
    Face f;

    // Transform the main cycle
    for (int i = 0; i < segs.size(); i++) {
        f.addSeg(segs[i].transform(tu, frac));
    }

    // Transform and add each hole
    for (int i = 0; i < holes.size(); i++) {
        f.holes.push_back(holes[i].transform(tu, frac));
    }

    return f;
}

/*
10 Point ~inside~

Tests, if a point is inside this face. This is exactly the case, when
it is inside the main cycle and outside of all hole cycles.
Uses the winding number algorithm.

*/
bool Face::inside(Point p) {
    int wn = 0;
    for (int i = 0; i < segs.size(); i++) {
        Seg& s = segs[i];
        if (s.i.y <= p.y) {
            if (s.f.y > p.y) {
                if (s.sign(p) > 0)
                    wn++;
            }
        } else {
            if (s.f.y <= p.y) {
                if (s.sign(p) < 0)
                    wn--;
            }
        }
    }
    bool inside = (wn != 0);

    if (!inside) // Point is outside the main cycle
        return false;

    for (int i = 0; i < holes.size(); i++) {
        Face& h = holes[i];
        if (h.inside(p))    // Point is inside a hole cycle,
            return false;   // hence outside the face
    }

    return true; // Point is truly inside the face
}

/*
11 ~area~

Calculates the area of this face. Holes are not taken into
account.

*/
double Face::area() {
    double ret = 0;
    
    for (int nrseg = 0; nrseg < segs.size(); nrseg++) {
        Seg s = segs[nrseg];
        ret += (s.i.x*s.f.y - s.f.x*s.i.y);
    }
    
    return ret/2.0;
}

/*
12 ~centroidParams~

Determines two parameters used to calculate the centroid of a region.

*/
std::pair<double, double> Face::centroidParams() {
    double xs = 0, ys = 0;
    
    for (int nrseg = 0; nrseg < segs.size(); nrseg++) {
        Seg seg = segs[nrseg];
        xs += (seg.i.x + seg.f.x) * (seg.i.x*seg.f.y - seg.f.x*seg.i.y);
        ys += (seg.i.y + seg.f.y) * (seg.i.x*seg.f.y - seg.f.x*seg.i.y);
    }
    
    return std::pair<double, double>(xs, ys);
}

/*
13 ~boundingBox~

Returns the bounding box of this face.

*/
BoundingBox Face::boundingBox() {
    BoundingBox bb;
    
    for (unsigned int nrseg = 0; nrseg < segs.size(); nrseg++) {
        Seg& seg = segs[nrseg];
        bb.update(seg.i);
        bb.update(seg.f);
    }
    
    return bb;
}

/*
14 ~ToString~

Returns a string representation of this face.

*/
std::string Face::ToString() {
    std::string ret = "( ";

    for (int i = 0; i < segs.size(); i++) {
        Seg s = segs[i];
        ret += s.i.ToString() + "\n";
    }
    ret += " )\n";

    return ret;
}

/*
15 ~toRList~

Returns an ~RList~ representation of this face.

*/
RList Face::toRList() {
    RList ret;
    
    RList &mf = ret.nest();
    for (int nrseg = 0; nrseg < segs.size(); nrseg++) {
        Seg s = segs[nrseg];
        mf.append(s.i.toRList());
    }
    for (int nrhole = 0; nrhole < holes.size(); nrhole++) {
        Face& hole = holes[nrhole];
        RList &hl = ret.nest();
        for (int nrseg = 0; nrseg < segs.size(); nrseg++) {
            Seg s = hole.segs[nrseg];
            hl.append(s.i.toRList());
        }
    }
    
    return ret;
}