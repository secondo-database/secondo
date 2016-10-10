/* 
 * This file is part of libfmr
 * 
 * File:   FMRegion.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 6, 2016, 11:50 AM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class FMRegion

[TOC]

1 Overview

This class defines a moving and rotating region of fixed shape.
One or more ~transformation units~ define the movement over a
time interval.

*/

#include "FMRegion.h"
#include "TraversedAreaFactory.h"

using namespace fmr;

/*
2 ~FMRegion from RList~

This function creates an ~FMRegion~ from a RList.

2.1 List definition

(<region>(<transformation><transformation>[*]))

<region>: See SpatialAlgebra

<transformation>: (<center><v0><v><a0><a><interval>)

<center>: The center point of the rotation relative to the region
<v0>: Initial displacement of the region
<v>: Linear displacement during the time interval
<a0>: Initial rotation of the region
<a>: Rotation of the region during the time interval
<interval>: The time interval consisting of a start and end instant

2.2 Example

(
   ( \_! region !\_
      ( \_! face 1 !\_
         ( \_! main cycle !\_
           (190 -30) 
           (110 -279) 
           (342 -156) 
           (162 -216) ) ) )
        ( \_! transformation units !\_
            ( \_! transformation unit 1 !\_
                (185 -164)    \_! center !\_
                (0 0)         \_! v0     !\_
                (471 482)     \_! v      !\_
                0             \_! a0     !\_
                8             \_! a      !\_
                \_! interval !\_
                ("1970-01-01-01:00:00" "1970-01-01-01:08:20" TRUE FALSE ) ) 
           ( \_! transformation unit 2 !\_
                (112 -274)    \_! center !\_
                (655 519)     \_! v0     !\_
                (638 -517)    \_! v      !\_
                8             \_! a0     !\_
                5             \_! a      !\_
                \_! interval !\_
                ("1970-01-01-01:08:20" "1970-01-01-01:16:40" TRUE TRUE ) ) ) ) 

*/
FMRegion::FMRegion(RList& l) {
    region = Region(l[0]);
    RList& tus = l[1];
    for (int i = 0; i < tus.size(); i++) {
        trafos.push_back(TransformationUnit(tus[i]));
    }
}

/*
3 ~FMRegion~ from region and transformationunit

Creates an ~FMRegion~ from ~region~ and a first transformation unit ~tu~

*/

FMRegion::FMRegion(Region region, TransformationUnit tu) : region(region) {
    trafos.push_back(tu);
}

/*
4 ~findTransformationUnit~

Find the transformation unit, which is defined at instant ~time~.

*/
TransformationUnit* FMRegion::findTransformationUnit (double time) {
    for (int i = 0; i < trafos.size(); i++) {
        TransformationUnit *tu = &trafos[i];
        if ((tu->iv.start < time && tu->iv.end > time) ||
            (tu->iv.start == time && tu->iv.lc) || 
            (tu->iv.end == time && tu->iv.rc))
            return tu;
    }
    
    return NULL;
}

/*
5 ~traversedArea~

Calculate the traversed area of this ~fmregion~
The result is of type ~Region2~, since the line segments are curved
in general.
The actual calculations are performed in TraversedAreaFactory.cpp

*/
Region2 FMRegion::traversedArea() {
    return fmr::traversedArea(*this);
}

/*
6 ~atinstant~

Calculates the projection of an ~fmregion~ to a ~region~ for the given
instant ~time~.

*/
Region FMRegion::atinstant (double time) {
    TransformationUnit* tu = findTransformationUnit(time);
    if (tu == NULL)
        return Region();
    
    return region.transform(*tu, tu->iv.getFrac(time));
}

/*
7 ~boundingBox~

Calculates the bounding box of this FMRegion. This is not necessarily a minimal
bounding box, but it is guaranteed that no part of the fmregion is outside at
any instant.

*/
Seg FMRegion::boundingBox () {
    Seg bb;
    for (unsigned int nrtrafo = 0; nrtrafo < trafos.size(); nrtrafo++) {
        // Iterate over all transformation units
        TransformationUnit& tu = trafos[nrtrafo];
        Point p;
        double dist = NAN;
        // Find the point with the greatest distance from the center point of
        // the rotation
        for (unsigned int nrface = 0; nrface < region.faces.size(); nrface++) {
            Face& f = region.faces[nrface];
            for (unsigned int nrseg = 0; nrseg < f.segs.size(); nrseg++) {
                Point p2 = f.segs[nrseg].i;
                if (isnan(dist) || tu.c.distance(p2) > dist) {
                    dist = tu.c.distance(p2);
                    p = p2;
                }
            }
        }
        // Take the bounding box of the translation vector and add the distance
        // of the point determined above from the center point to each
        // direction.
        Point start = tu.c + tu.v0;
        Point end = start + tu.v;
        double x1 = std::min(start.x, end.x) - dist;
        double y1 = std::min(start.y, end.y) - dist;
        double x2 = std::max(start.x, end.x) + dist;
        double y2 = std::max(start.y, end.y) + dist;
        
        // Enlarge the previous bounding box accordingly
        if (!bb.valid() || bb.i.x > x1)
            bb.i.x = x1;
        if (!bb.valid() || bb.i.y > y1)
            bb.i.y = y1;
        if (!bb.valid() || bb.f.x < x2)
            bb.f.x = x2;
        if (!bb.valid() || bb.f.y < y2)
            bb.f.y = y2;
    }
    
    return bb;
}

/*
8 ~ToString~

Returns a string representation of this object

*/
std::string FMRegion::ToString() {
    std::stringstream ss;
    
    ss << "( " << region.ToString() << "\n";
    for (int i = 0; i < trafos.size(); i++) {
        ss << trafos[i].ToString() << "\n";
    }
    ss << ")";
    return ss.str();
}

/*
9 ~toRList~

Returns an ~RList~ representation of this object

*/
RList FMRegion::toRList() {
    RList ret;
    
    ret.append(region.toRList());
    RList& tr = ret.nest();
    for (int i = 0; i < trafos.size(); i++) {
        tr.append(trafos[i].toRList());
    }
    
    return ret;
}
