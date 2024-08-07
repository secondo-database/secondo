/* 
 * This file is part of libfmr
 * 
 * File:   RFace.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 20, 2016, 10:48 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Point~

[TOC]

1 Overview

The class ~RFace~ represents a face of a CRegion. It is constructed from
~RCurve~-Segments (see RCurve.cpp) and can contain one or more holes,
which are also RFaces.

*/

#include "fmr_RFace.h"
#include "fmr_RCurve.h"

using namespace fmr;

/*
2 Construct from RList

Constructs an RFace from its RList representation.

*/
RFace::RFace(RList &r) {
    getCycle(r[0]); // Get the main cycle
    for (int i = 1; i < r.size(); i++) {
        RFace hole; // Get (optional) hole cycles
        hole.getCycle(r[i]);
        holes.push_back(hole);
    }
}

/*
3 ~getCycle~

Read a single cycle from an ~RList~.

*/
void RFace::getCycle(RList& r) {
    for (int i = 0; i < r.size(); i++) {
        face.push_back(RCurve(r[i]));
    }
}

/*
4 ~intersections~

Calculate all intersections of line segment ~s~ with this ~RFace~.
This includes intersections with holes.

*/
std::vector<Point> RFace::intersections(Seg s) {
    std::vector<Point> ret;

    for (int nrcycle = -1; nrcycle < (int) holes.size(); nrcycle++) {
        RFace& rf = (nrcycle < 0) ? * this : holes[nrcycle];
        for (int nrrcurve = 0; nrrcurve < rf.face.size(); nrrcurve++) {
            RCurve& rc = rf.face[nrrcurve];
            std::vector<Point> is = rc.intersections(s);
            ret.insert(ret.end(), is.begin(), is.end());
        }
    }

    return ret;
}

/*
5 ~inside~

Test, if a given Point ~p~ is inside this RFace.
Uses the plumbline algorithm: Count the number of intersections between the
Point ~p~ and a point guaranteed to be outside the face. If the number is odd,
the point is inside. This also works if the point is inside a hole of this face.
 
*/
bool RFace::inside(Point p) {
    // A point outside the bounding box is guaranteed to be outside the RFace
    Point outside = boundingBox().lowerLeft - Point(100, 100);
    std::vector<Point> is = intersections(Seg(p, outside));
    return is.size()&1;
}

/*
6 ~intersects~

Test, if a given Face ~f~ intersects or overlaps with this RFace.

*/
bool RFace::intersects(Face& f) {
    // Test each segment of the Face f for intersection with the main cycle or
    // a hole cycle. Both means, the Faces intersect.
    for (unsigned int nrseg = 0; nrseg < f.segs.size(); nrseg++) {
        std::vector<Point> is = intersections(f.segs[nrseg]);
        if (is.size() > 0)
            return true; // We have an intersection
    }

    // Is the Face f completely inside this RFace?
    Point p = f.segs[0].i;
    if (inside(p))
        return true; // Yes, since a point of the face is inside the RFace and
    // there are no intersections

    // Is the RFace completely inside the Face f?
    Point p2 = face[0].off;
    if (f.inside(p2))
        return true; // Yes, since a point of the RFace is inside the Face and
    // there are no intersections.

    return false;
}

/*
7 boundingBox

Calculate the bounding box for this RFace. This is not necessarily a minimal
bounding box.

*/
BoundingBox RFace::boundingBox() {
    BoundingBox bb;
    
    for (unsigned int nrseg = 0; nrseg < face.size(); nrseg++) {
        RCurve& rc = face[nrseg];
        bb.update(rc.boundingBox());
    }
    
    return bb;
}

/*
8 ~toFace~

Converts this RFace to a standard Region Face by approximating the border with
~nrsegs~ straight segments. Also converts its holes.

*/
Face RFace::toFace(int nrsegs) {
    Face ret;
    
    int facesegs = face.size();
    for (int i = 0; i < facesegs; i++) {
        std::vector<Seg> segs = face[i].toSegs(nrsegs/facesegs+1);
        ret.segs.insert(ret.segs.end(), segs.begin(), segs.end());
    }
    
    // Now convert the holes
    for (int nrhole = 0; nrhole < holes.size(); nrhole++) {
        ret.holes.push_back(holes[nrhole].toFace(nrsegs));
    }

    return ret;
}

/*
9 ~toRList~

Returns an RList representation of this RFace.

*/
RList RFace::toRList() {
    RList ret;
    RList& mf = ret.nest();
    for (int i = 0; i < face.size(); i++) {
        mf.append(face[i].toRList());
    }
    for (int j = 0; j < holes.size(); j++) {
        RFace& hole = holes[j];
        RList& hl = ret.nest();
        for (int i = 0; i < hole.face.size(); i++) {
            hl.append(hole.face[i].toRList());
        }
    }

    return ret;
}
