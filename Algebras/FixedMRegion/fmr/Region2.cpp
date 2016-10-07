/* 
 * This file is part of libfmr
 * 
 * File:   Region2.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 20, 2016, 10:50 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class Region2

[TOC]

1 Overview

The class ~Region2~ represents a set of ~RFace~s (see RFace.cpp). These are
areas with curved borders and optionally one or more holes.

*/

#include "Region2.h"

using namespace fmr;

/*
2 Constructor from RList

Constructs a ~Region2~ from its RList representation.

*/
Region2::Region2(RList& r) {
    for (int i = 0; i < r.size(); i++) {
        faces.push_back(RFace(r[i]));
    }
}

/*
3 ~inside~

Tests, if a given point ~p~ is inside the region. This is exactly the case, if
it is inside of one of its ~RFace~s. To test this, the number of intersections
between a segment with start point ~p~ and some endpoint which is guaranteed
outside the Region2 is examined. If it is odd, the point is inside. This also
works if the point is inside a hole of a face.

*/
bool Region2::inside(Point p) {
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        std::vector<Point> is = faces[nrface].intersections(
                Seg(p, Point(1000000, 1000000)));
        if (is.size()&1)
            return true;
    }
    
    return false;
}

/*
4 ~intersections~

Calculate all intersections between a segment ~s~ and this region.

*/
std::vector<Point> Region2::intersections (Seg s) {
    std::vector<Point> ret;
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        RFace& rface = faces[nrface];
        for (int nrcycle = -1; nrcycle < (int) rface.holes.size(); nrcycle++) {
            RFace& rf = (nrcycle < 0) ? rface : rface.holes[nrcycle];
            for (int nrrcurve = 0; nrrcurve < rf.face.size(); nrrcurve++) {
                RCurve& rc = rf.face[nrrcurve];
                std::vector<Point> is = rc.intersections(s);
                ret.insert(ret.end(), is.begin(), is.end());
            }
        }
    }
    
    return ret;
}

/*
5 ~toRList~

Returns an ~RList~ representation of this Region2 object

*/
RList Region2::toRList() {
    RList ret;
    
    for (int i = 0; i < faces.size(); i++) {
        ret.append(faces[i].toRList());
    }
    
    return ret;
}

