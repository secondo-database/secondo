/* 
 * File:   BoundingBox.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on October 10, 2016, 3:08 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~BoundingBox~

[TOC]

1 Overview

This class represents the bounding box of an object. Several functions to
update and modify the bounding box as well as some tests (point inside, segment
intersects) are implemented.

*/

#include "fmr_BoundingBox.h"

using namespace fmr;

/*
2 ~update~ with point

Update the bounding box to contain the given point ~p~

*/
void BoundingBox::update (Point p) {
    if (!lowerLeft.valid()) {
        lowerLeft = p;
    } else {
        if (lowerLeft.x > p.x)
            lowerLeft.x = p.x;
        if (lowerLeft.y > p.y)
            lowerLeft.y = p.y;
    }
    if (!upperRight.valid()) {
        upperRight = p;
    } else {
        if (upperRight.x < p.x)
            upperRight.x = p.x;
        if (upperRight.y < p.y)
            upperRight.y = p.y;
    }
}

/*
3 ~update~ with boundingbox

Update the bounding box to contain the given boundingbox ~bb~

*/
void BoundingBox::update (BoundingBox bb) {
    update(bb.lowerLeft);
    update(bb.upperRight);
}

/*
4 ~rotate~ 

Rotate the bounding box around ~center~ by ~angle~ and update this bounding box
to contain the rotated bounding box.

*/
void BoundingBox::rotate (Point center, double angle) {
    Point p1 = lowerLeft.rotate(center, angle);
    Point p2 = Point(lowerLeft.x, upperRight.y).rotate(center, angle);
    Point p3 = Point(upperRight.x, lowerLeft.y).rotate(center, angle);
    Point p4 = upperRight.rotate(center, angle);
    
    invalidate();
    
    update(p1);
    update(p2);
    update(p3);
    update(p4);
}

/*
5 ~inside~

Tests, if a given Point ~p~ is inside the bounding box.
 
*/
bool BoundingBox::inside (Point p) {
    return valid() &&
             lowerLeft.x <= p.x &&  lowerLeft.y <= p.y &&
            upperRight.x >= p.x && upperRight.y >= p.y;
}

/*
6 ~intersects~

Tests, if a line segment ~seg~ intersects with this bounding box.

*/
bool BoundingBox::intersects (Seg& seg) {
    bool intersects = false;
    
    if (seg.f.x == seg.i.x) { // Line is parallel to y-axis
        double x = seg.i.x;
        double y1 = std::min(seg.i.y, seg.f.y);
        double y2 = std::max(seg.i.y, seg.f.y);
        intersects =  ((lowerLeft.x <= x && x <= upperRight.x) &&
                       ((lowerLeft.y <= y1 && y1 <= upperRight.y) ||
                        (lowerLeft.y <= y2 && y2 <= upperRight.y) ||
                        (y1 < lowerLeft.y && y2 > upperRight.y)));
    } else {
        // Initialize values m and c, so that the segment is
        // on the line y = m*x + c
        double m = (seg.f.y - seg.i.y) / (seg.f.x - seg.i.x);
        double c = seg.i.y - m * seg.i.x;
        
        double y1, y2;
        if (m > 0) { // Ensure that y1 < y2
            y1 = m*lowerLeft.x + c;
            y2 = m*upperRight.x + c;
        } else {
            y1 = m*upperRight.x + c;
            y2 = m*lowerLeft.x + c;
        }
        intersects = ((lowerLeft.y <= y1 && y1 <= upperRight.y)  ||
                      (lowerLeft.y <= y2 && y2 <= upperRight.y)  ||
                      (y1  < lowerLeft.y && upperRight.y  < y2));
    }
    
    if (!intersects) {
        // Test, if the segment is completely inside the bounding box
        if (lowerLeft.x <= seg.i.x && seg.i.x <= upperRight.x &&
            lowerLeft.y <= seg.i.y && seg.i.y <= upperRight.y)
            intersects = true;
    }
    
    return intersects;
}

/*
7 ~toString~ 

Returns a string representation of this bounding box.
 
*/
std::string BoundingBox::ToString() {
    std::stringstream ss;
    
    ss << "BB (" << lowerLeft.ToString() << " " << upperRight.ToString() << ")";
    
    return ss.str();
}