/* 
 * This file is part of libfmr
 * 
 * File:   Seg.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 9, 2016, 3:21 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Seg~

[TOC]

1 Overview

The class Seg represents a line segment constructed from a start and
end point.

*/

#include "fmr_Seg.h"

using namespace fmr;


/*
2 Construct from start and endpoint

*/
Seg::Seg(Point i2, Point f2) : i(i2), f(f2) {
}

/*
3 ~transform~

Transform this segment according to the TransformationUnit
~tu~ at time fraction frac (in [0;1])

*/
Seg Seg::transform(TransformationUnit& tu, double frac) {
    Point i2 = i.transform(tu, frac);
    Point f2 = f.transform(tu, frac);

    return Seg(i2, f2);
}

/*
4 ~rotate~

Rotate this segment around point ~center~ by ~angle~

*/
Seg Seg::rotate(Point center, double angle) {
    return Seg(i.rotate(center, angle), f.rotate(center, angle));
}

/*
5 ~operator+~

Translate this segment by vector ~p~

*/
Seg Seg::operator+(Point v) {
    return Seg(i + v, f + v);
}

/*
6 ~sign~

Calculate the sign of this segment with point ~p~.

The result is:
<0 if point ~p~ is left of the segment
=0 if point ~p~ is on the segment
>0 if point ~p~ is right of the segment

*/
double Seg::sign(Point p) {
    return ((p.x - f.x)*(i.y - f.y)-(p.y - f.y)*(i.x - f.x));
}

/*
7 ~getAngle~

Calculates the angle of this line segment.

*/
double Seg::getAngle() {
    double ret = atan2(f.y - i.y, f.x - i.x) + 2.5 * M_PI;
    while (ret > 2 * M_PI) {
        ret -= 2 * M_PI;
    }

    return 2 * M_PI - ret;
}

/*
8 ~near~

Test if the segment ~s2~ is near this segment within a specified
~tolerance~.

*/
bool Seg::near(Seg& s2, double tolerance) {
    double limit = (f-i).length()*tolerance;
    return ((i-s2.i).length() < limit && (f-s2.f).length() < limit);
}

/*
9 ~calculateTransformation~

Try to find a TransformationUnit, which would transform this segment into
the given segment ~seg~ with rotation point ~center~ and a specified
~tolerance~. The first item of the pair (the bool) specifies, if the second
part is a valid TransformationUnit.

*/
std::pair<bool, TransformationUnit> Seg::calculateTransformation(Seg& seg,
                                               Point center, double tolerance) {
    double l1 = (f-i).length(), l2 = (seg.f - seg.i).length();
    if (std::abs(l2 - l1) > l1*tolerance) {
        // Length differs too much, no valid transformation can be found here
        return std::pair<bool, TransformationUnit>(false, TransformationUnit());
    }

    // The length of the segments is within the tolerance, a valid
    // transformation can always be found now
    
    // Calculate the necessary rotation angle between ]-Pi;Pi]
    double angle = getAngle() - seg.getAngle();
    if (angle <= -M_PI)
        angle += 2*M_PI;
    else if (angle > M_PI)
        angle -= 2*M_PI;
    
    // Calculate the correct vector for the given center point
    Point vector = seg.i - i.rotate(center, angle);
    TransformationUnit tu(center, vector, angle);
    
    return std::pair<bool, TransformationUnit>(true, tu);
}

/*
10 ~ToString~

Returns a string representation for this segment

*/
std::string Seg::ToString() {
    std::string ret = "";

    ret = "( " + i.ToString() + " " + f.ToString() + " )";

    return ret;
}
