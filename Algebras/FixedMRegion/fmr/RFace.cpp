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

The class ~RFace~ represents a face of a Region2. It is constructed from
~RCurve~-Segments (see RCurve.cpp) and can contain one or more holes,
which are also RFaces.

*/

#include "RFace.h"
#include "RCurve.h"

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
        RFace& rf = (nrcycle < 0) ? *this : holes[nrcycle];
        for (int nrrcurve = 0; nrrcurve < rf.face.size(); nrrcurve++) {
            RCurve& rc = rf.face[nrrcurve];
            std::vector<Point> is = rc.intersections(s);
            ret.insert(ret.end(), is.begin(), is.end());
        }
    }
   
    return ret;
}

/*
5 ~toRList~

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

