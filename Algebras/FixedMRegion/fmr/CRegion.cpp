/* 
 * This file is part of libfmr
 * 
 * File:   CRegion.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 20, 2016, 10:50 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class CRegion

[TOC]

1 Overview

The class ~CRegion~ represents a set of ~RFace~s (see RFace.cpp). These are
areas with curved borders and optionally one or more holes.

*/

#include "fmr_CRegion.h"

using namespace fmr;

/*
2 Constructor from RList

Constructs a ~CRegion~ from its RList representation.

*/
CRegion::CRegion(RList& r) {
    for (int i = 0; i < r.size(); i++) {
        faces.push_back(RFace(r[i]));
    }
}

/*
3 ~inside~

Tests, if a given point ~p~ is inside the region. This is exactly the case, if
it is inside of one of its ~RFace~s.

*/
bool CRegion::inside(Point p) {
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        if (faces[nrface].inside(p))
            return true;
    }
    
    return false;
}

/*
4 ~intersections~

Calculate all intersections between a segment ~s~ and this region.

*/
std::vector<Point> CRegion::intersections (Seg s) {
    std::vector<Point> ret;
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        RFace& rface = faces[nrface];
        std::vector<Point> is = rface.intersections(s);
        ret.insert(ret.end(), is.begin(), is.end());
    }
    
    return ret;
}

/*
5 ~intersects~ with Region

Tests, if this CRegion intersects with a given Region.
This is exactly the case if a segment of the cycle of a face of the Region
intersects with the CRegion or a single corner point of the Region lies inside
the CRegion.

*/
bool CRegion::intersects(Region& region) {
    // Iterate over all pairs of face/rface of the regions
    for (unsigned int nrrf = 0; nrrf < faces.size(); nrrf++) {
        RFace& r = faces[nrrf];
        for (unsigned int nrf = 0; nrf < region.faces.size(); nrf++) {
            Face& f = region.faces[nrf];
            if (r.intersects(f))
                return true;
        }
    }
    
    return false;
}

/*
6 ~boundingBox~

Returns the bounding box for this CRegion. This is not necessarily a minimal
bounding box.

*/
BoundingBox CRegion::boundingBox() {
    BoundingBox bb;
    
    for (unsigned int nrface = 0; nrface < faces.size(); nrface++) {
        RFace& f = faces[nrface];
        bb.update(f.boundingBox());
    }
    
    return bb;
}

/*
7 ~toRegion~

Converts this CRegion to a Region by approximating the border of each face in
~nrsegs~ straight segments.

*/
Region CRegion::toRegion(int nrsegs) {
    Region ret;
    
    for (int i = 0; i < faces.size(); i++) {
        ret.faces.push_back(faces[i].toFace(nrsegs));
    }
    
    return ret;
}

/*
8 ~toRList~

Returns an ~RList~ representation of this CRegion object

*/
RList CRegion::toRList() {
    RList ret;
    
    for (int i = 0; i < faces.size(); i++) {
        ret.append(faces[i].toRList());
    }
    
    return ret;
}
