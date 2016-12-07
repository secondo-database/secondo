/* 
----
 * This file is part of libfmr
 * 
 * File:   UPoint.cpp
 * Author: flo
 * 
 * Created on September 10, 2016, 6:20 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~UPoint~

[TOC]

1 Overview

The class ~UPoint~ represents a single time interval of a moving point.
Besides moving linearly from the startpoint ~s~ along a vector ~v~, it
may optionally also rotate around a center ~c~ by an angle ~a~. A set
of those UPoints with disjunct time intervals make up an ~MPoint~.
The main purpose of this construct is to implement the operation
MPoint ~inside~ FMRegion (see also MPoint.cpp)
To implement this operation, the rotation and movement of the FMRegion is
projected on the MPoint (resp. on each of its UPoint units), which is the
reason, why a rotating movement component is implemented here.

- First, transform the start of the UPoint to match the initial vector
  and angle of the FMRegion
- Then, add the movement vector of the FMRegion to the UPoint and store
  the center point and rotation angle of the FMRegion
- During the movement, the center point moves away from the startpoint
  along the vector of the UPoint, and then the rotation is performed.

After this transformation, the FMRegion can be seen as a static region
while the intersection times are still the same as with the initial
configuration, since the relative position of the UPoint to the FMRegion
is exactly the same as before at all times.

Calculation of movement for time ~t~: s.rotate(c-v[*]t, a[*]t) + v[*]t

The parametric functions of this moving point are as follows:

fx(t) = c.x + (s.x-c.x+v.x[*]t)[*]cos(a[*]t) - (s.y-c.y+v.y[*]t)[*]sin(a[*]t)
fy(t) = c.y + (s.x-c.x+v.x[*]t)[*]sin(a[*]t) + (s.y-c.y+v.y[*]t)[*]cos(a[*]t)

*/

#include "fmr_UPoint.h"

using namespace fmr;

/*
2 Constructor

Constructs an invalid moving point.

*/
UPoint::UPoint() {
    valid = false;
}

/*
3 Construct form RList

Constructs a UPoint from its RList representation.

*/
UPoint::UPoint(RList& l) {
    iv = Interval(l[0]);
    s = Point((double) l[1][0], (double) l[1][1]);
    Point   e((double) l[1][2], (double) l[1][3]);
    v = e - s;
    valid = true;
}

/*
4 Construct from start and endpoint

For a given startpoint ~s~,  endpoint ~e~ and interval ~iv~,
construct a UPoint without rotation.

*/
UPoint::UPoint(Point s, Point e, Interval iv) : s(s), iv(iv) {
    v = e - s;
    rot = 0;
    valid = true;
}

/*
5 ~transform~

Transforms a straight moving UPoint to also reflect the movement
of an fmregion transform unit.

*/
UPoint UPoint::transform(TransformationUnit& _tu) {
    // First, calculate the intersection of the time intervals.
    // The resulting UPoint will only cover the overlapping
    // part of the time intervals
    Interval niv = _tu.iv.intersection(iv);
    
    if ((niv.start == niv.end) && (!niv.lc || !niv.rc))
        return UPoint();
    // Calculcate the UPoint and TransformationUnit restricted to
    // the overlapping time interval
    UPoint up = restrict(niv);
    TransformationUnit tu = _tu.restrict(niv);
    
    // Now, calculcate the new startpoint and vector of the resulting
    // UPoint to honour the initial vector and angle of the fmregion
    up.s = (up.s - tu.v0).rotate(tu.c, -tu.a0);
    up.v = (up.v - tu.v ).rotate(-tu.a0);
    // Store the center of rotation and the (inverted) angle, since the
    // UPoint will rotate in the other direction.
    up.c = tu.c;
    up.rot = -tu.a;
    
    // The resulting UPoint will now at all times t have exactly the
    // same relative position to the (static) fmregion at time t=0 as
    // the original UPoint had to the moving fmregion.
    return up;
}

/*
6 ~restrict~

Returns a new UPoint, which is restricted to the time interval ~niv~.
The original object is not modified.

*/
UPoint UPoint::restrict (Interval niv) {
    double st = iv.getFrac(niv.start);
    double et = iv.getFrac(niv.end);
    
    return UPoint(s + v*st, s + v*et, niv);
}

/*
7 ~project~

Calculate the position of the UPoint at time fraction ~frac~.
First, calculate the center point (c-v[*]t), then rotate it and
finally, add the movement along the vector.

*/
Point UPoint::project(double t) {
    return s.rotate(c-v*t, rot*t) + v*t;
}

/*
8 ~atinstant~

Calculate the position of the UPoint at time ~time~

*/
Point UPoint::atinstant(double time) {
    double t = (time - iv.start)/(iv.end - iv.start);
    return project(t);
}

/*
9 ~toRList~

Returns an RList representation of this UPoint.

*/
RList UPoint::toRList() {
    RList ret;
    
    ret.append(iv.toRList());
    RList p;
    p.append(s.x);
    p.append(s.y);
    p.append(s.x+v.x);
    p.append(s.y+v.y);
    ret.append(p);

    return ret;
}
