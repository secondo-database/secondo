/* 
 * This file is part of libfmr
 * 
 * File:   Ravdoid.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 15, 2016, 2:12 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Ravdoid~

[TOC]

1 Overview

The class Ravdoid represents a curve, which is described by a point moving on
a moving and rotating line segment. The Ravdoid (from greek ravdos: pole) is
part of the border of the traversed area of a fmregion.

*/

#include "fmr_Ravdoid.h"

using namespace fmr;

static double distance (Point lp1, Point lp2, Point p);

/*
2 Constructor

A Ravdoid constructed from its basic parameters.

*/
Ravdoid::Ravdoid(double hp, double cd, double xoff, double yoff, double toff,
        double tmin1, double tmax1, double tmin2, double tmax2, double rot) : 
        hp(hp), cd(cd), xoff(xoff), yoff(yoff), toff(toff),
        tmin1(tmin1), tmin2(tmin2), tmax1(tmax1), tmax2(tmax2), rot(rot) {
}

/*
3 ~project~

Calculate the point on the curve at time ~t~
The formula for x and y are the parametric equations of the
Ravdoid.

*/
Point Ravdoid::project (double t) {
    t = toff + t * rot;
    
    double x = hp * (2*t - sin(2*t)) + cd * cos(t) + xoff;
    double y = hp *        cos(2*t)  + cd * sin(t) + yoff;
    
    return Point(x, y);
}

/*
4 ~valid~

Depending on the position relative to the rotation center, the
length of the generating line segment and the parameters of the
movement and rotation, the Ravdoid may be only valid in parts
of the time interval. These parts are defined by tmin1/2 and
tmax1/2.

*/
bool Ravdoid::valid (double t) {
    double t2 = t * rot;

    while (t2 > tmax1) {
        t2 -= 2 * M_PI;
    }

    bool valid1 = !((t2 + PRECISION < tmin1) || (t2 - PRECISION > tmax1));
    bool valid2 = false;
    
    if (!std::isnan(tmax2)) {
        t2 = t * rot;
        while (t2 > tmax2)
            t2 -= 2 * M_PI;
        valid2 = !((t2 + PRECISION < tmin2) || (t2 - PRECISION > tmax2));
    }
    
    return (t >= 0) && (t <= 1) &&
           (valid1 || valid2 || (std::isnan(tmax1) && std::isnan(tmax2)));
}

/*
5 ~create~

Create a Ravdoid from a line segment ~seg~. The movement of this segment
is described by the TransformationUnit ~tu~.

*/
Ravdoid Ravdoid::create (Seg seg, TransformationUnit &tu) {
    // First, transform the segment by the initial transformations v0 and a0
    seg = seg.rotate(tu.c, tu.a0) + tu.v0;
    Point c = tu.c + tu.v0;
    Point v = tu.v;
    double rot = tu.a;
    
    // If the rotation is negative, invert the movement to make it positive
    if (rot < 0) {
        seg = seg.rotate(c, rot) + v;
        c = c + v;
        v = v * -1;
        rot = -rot;
    }
    
    // Calculate all relevant parameters for the curve
    double vx = v.length();
    seg = seg.rotate(c, -v.angle());
    double angle = seg.angle() + M_PI/2;
    double toff = angle;
    double tmin2 = nan(""), tmax2 = nan("");
    
    double hp = vx / rot / 2;
    double cd = distance(seg.f, seg.i, c);
    double xoff = c.x - angle * hp * 2;
    double yoff = c.y + hp;
    
    // Calculate the validity periods tmin1/2 and tmax1/2
    double c1 = ((seg.i.x - c.x) * sin(-angle) + (seg.i.y - c.y) * cos(-angle))
                * rot / vx;
    double c2 = ((seg.f.x - c.x) * sin(-angle) + (seg.f.y - c.y) * cos(-angle))
                * rot / vx;

    double tmin = acos(c2) - angle;
    double tmax = acos(c1) - angle;
    double retp = -toff;
    double boundary = std::isnan(tmax) ? tmin : tmax;
    if (!std::isnan(boundary)) {
        while (retp > boundary)
            retp -= M_PI;
        while (retp < boundary)
            retp += M_PI;
    }
    
    if (std::isnan(tmax) && !std::isnan(tmin)) {
        tmax = -tmin + 2 * retp;
    } else if (std::isnan(tmin) && !std::isnan(tmax)) {
        tmin = -tmax + 2 * retp;
    } else if (std::isnan(tmin) && std::isnan(tmax)) {
        tmin = 0;
        tmax = ((c1 > 0 && c2 > 0) || (c1 < 0 && c2 < 0)) ? 0 : 2.0 * M_PI;
    } else {
        tmax2 = -tmin + 2 * retp;
        tmin2 = -tmax + 2 * retp;
    }
    while (tmin > tmax) {
        tmax += 2.0 * M_PI;
    }
    while (tmax < 0) {
        tmin += 2.0 * M_PI;
        tmax += 2.0 * M_PI;
    }
    while (tmax > 2.0 * M_PI) {
        tmin -= 2.0 * M_PI;
        tmax -= 2.0 * M_PI;
    }
    
    Ravdoid r(hp, cd, xoff, yoff, toff, tmin, tmax, tmin2, tmax2, rot);
    r.setTransformation(c, v.angle());
    
    return r;
}

/*
6 ~rcurve~

Create a ~rcurve~ from the part of this Ravdoid from time ~t1~ to ~t2~.
See also RCurve.cpp

*/
RCurve Ravdoid::rcurve (double t1, double t2) {
    RCurve ret;
    
    ret.type = "R";
    ret.angle = t_angle;
    ret.off = project(t1).rotate(t_center, t_angle);    
    ret.params.push_back(hp);
    ret.params.push_back(cd);
    ret.params.push_back(toff+t1*rot);
    ret.params.push_back(rot*(t2-t1));
    
    return ret;
}

// Helper function to calculate the signed distance between a line
// segment (~lp1~ ~lp2~) and a point ~p~
static double distance (Point lp1, Point lp2, Point p) {
    return ((lp2.y - lp1.y) * p.x - (lp2.x - lp1.x) * p.y +
                                                  lp2.x * lp1.y - lp2.y * lp1.x)
            / sqrt((lp2.y - lp1.y) * (lp2.y - lp1.y) +
                                             (lp2.x - lp1.x) * (lp2.x - lp1.x));
}

/*
7 ~operator==~

Two Ravdoids are equal if all parameters are equal.

*/
bool Ravdoid::operator==(Ravdoid& r) {
    return hp==r.hp&&cd==r.cd&&xoff==r.xoff&&
            yoff==r.yoff&&toff==r.toff&&rot==r.rot;
}

/*
8 ~ToString~

Returns a string representation of this Ravdoid.

*/
std::string Ravdoid::ToString() {
    std::stringstream ss;
    ss << "Ravdoid; hp:" << hp << " cd:" << cd << " xoff:" << xoff << " yoff:"
            << yoff << " toff:" << toff << " rot:" << rot;
    
    return ss.str();
}
