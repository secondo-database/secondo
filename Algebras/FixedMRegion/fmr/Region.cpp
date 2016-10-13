/* 
 * This file is part of libfmr
 * 
 * File:   Region.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 3:20 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Region~

[TOC]

1 Overview

The class ~Region~ represents a set of faces. A face is a polygon
with optionally one or more holes (see also Face.cpp)

*/

#include "fmr_Region.h"
#include "fmr_FMRegion.h"
#include <iostream>

using namespace fmr;

/*
2 Constructor

Create a Region from an RList representation.

*/
Region::Region(RList& l) {
    for (int i = 0; i < l.size(); i++) {
        Face f(l[i]);
        faces.push_back(f);
    }
}

/*
3 ~inside~

Tests, if a point is inside a region.

A point is exactly then inside a region, if it is inside a face
of this region.

*/
bool Region::inside(Point p) {
    for (int i = 0; i < faces.size(); i++) {
        if (faces[i].inside(p))
            return true;
    }

    return false;
}

/*
4 ~transform~

Transform a region according to the given TransformationUnit ~tu~ at
the fraction ~frac~ of the interval.

*/
Region Region::transform(TransformationUnit& tu, double frac) {
    Region r;
    for (int i = 0; i < faces.size(); i++) {
        // Transform each face
        r.faces.push_back(faces[i].transform(tu, frac));
    }

    return r;
}

/*
5 ~getAllSegments~

Returns all segments of this region (including all faces and holes)

*/
std::vector<Seg> Region::getAllSegments() {
    std::vector<Seg> ret;

    for (int nrface = 0; nrface < faces.size(); nrface++) {
        Face& face = faces[nrface];
        ret.insert(ret.end(), face.segs.begin(), face.segs.end());
        for (int nrhole = 0; nrhole < face.holes.size(); nrhole++) {
            Face& hole = face.holes[nrhole];
            ret.insert(ret.end(), hole.segs.begin(), hole.segs.end());
        }
    }

    return ret;
}

/*
6 ~checkTransformation~

Check, if the transformation ~tu~ transforms all segments ~segs1~
to the segments in ~segs2~. Used to find a valid interpolation.

*/
static bool checkTransformation(TransformationUnit& tu,
        std::vector<Seg> segs1, std::vector<Seg> segs2) {
    for (int s1 = 0; s1 < segs1.size(); s1++) {
        Seg& seg1 = segs1[s1];
        std::vector<Seg>::iterator s2it = segs2.begin();
        bool found = false;
        while (s2it != segs2.end()) {
            Seg& seg2 = *s2it;
            if (seg1.transform(tu, 1).near(seg2, 0.01)) {
                found = true;
                segs2.erase(s2it);
                break;
            }
            s2it++;
        }
        if (!found)
            return false;
    }

    return true;
}

/*
7 ~interpolate~

Find a valid interpolation of this region with the specified region
~r~ over the interval ~iv~. The desired center point of the rotation
is the centroid of the region.

*/
FMRegion Region::interpolate(Region& r, Interval iv) {
    return interpolate(r, this->centroid(), iv);
}

/*
8 ~interpolate~

Find a valid interpolation of this region with the specified region
~r~ over the interval ~iv~ using the centerpoint ~center~.
The success of the operation is not depending on the choice of the
center point.

*/
FMRegion Region::interpolate(Region& r, Point center, Interval iv) {
    std::vector<Seg> segs1 = getAllSegments();
    std::vector<Seg> segs2 = r.getAllSegments();

    std::pair<bool, TransformationUnit> trafo;

    if (segs1.size() == 0 || segs1.size() != segs2.size()) {
        return FMRegion();
    }

    // Choose the longest segment of segs1 (to minimize numeric instabilities)
    Seg seg1 = segs1[0];
    for (int s1 = 1; s1 < segs1.size(); s1++) {
        Seg tmp = segs1[s1];
        if (tmp.length() > seg1.length())
            seg1 = tmp;
    }
    
    // And for each segment in segs2 ...
    for (int s2 = 0; s2 < segs2.size(); s2++) {
        Seg& seg2 = segs2[s2];

        // ... try to find a valid transformation to convert seg1 to seg2
        trafo = seg1.calculateTransformation(seg2, center, 0.01);
        if (!trafo.first)
            continue;
        // If such a transformation was found, check if it is valid for
        // all other segments, too. If this is the case, we have found
        // a valid interpolation.
        if (checkTransformation(trafo.second, segs1, segs2))
            break;
        else
            trafo.first = false;
    }

    if (trafo.first) {
        // A valid interpolation was found, create an FMRegion from it
        trafo.second.iv = iv;
        return FMRegion(*this, trafo.second);
    } else {
        // No valid interpolation was found, return an empty FMRegion
        return FMRegion();
    }
}

/*
9 ~area~

Calculates the area of this region as the sum of the area of all faces.

*/
double Region::area() {
    double ret = 0;
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        ret += faces[nrface].area();
    }
        
    return ret;
}

/*
10 ~centroid~

Calculates the centroid of this region from the faces and the area.

*/
Point Region::centroid() {
    double A = area();
    double xs = 0, ys = 0;
    
    for (int nrface = 0; nrface < faces.size(); nrface++) {
         std::pair<double, double> p = faces[nrface].centroidParams();
         xs += p.first;
         ys += p.second;
    }
    
    return Point(xs/(6*A), ys/(6*A));
}

/*
11 ~ToString~

Return a string representation of this region.

*/
std::string Region::ToString() {
    std::string ret = "";
    for (int i = 0; i < faces.size(); i++) {
        ret += "( " + faces[i].ToString() + " )\n";
    }

    return ret;
}

/*
12 ~toRList~

Return an RList representation of this region.

*/
RList Region::toRList() {
    RList ret;
    
    for (int nrface = 0; nrface < faces.size(); nrface++) {
        ret.append(faces[nrface].toRList());
    }
    
    return ret;
}
