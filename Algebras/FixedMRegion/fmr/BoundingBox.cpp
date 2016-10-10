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

#include "BoundingBox.h"

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
        if (upperRight.y > p.y)
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
    
    lowerLeft = Point();
    upperRight = Point();
    update(p1);
    update(p2);
    update(p3);
    update(p4);
}
