/* 
 * This file is part of libfmr
 * 
 * File:   Trochoid.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 15, 2016, 12:36 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~Trochoid~

[TOC]

1 Overview

The class Trochoid represents a curve, which is described by a point on
(or inside/outside) a moving wheel. The Trochoid (from greek trochos: wheel)
is part of the border of the traversed area of a fmregion.
See also: http://en.wikipedia.org/wiki/Trochoid

*/

#include "fmr_Trochoid.h"

using namespace fmr;

/*
2 Constructor

Construct a Trochoid from its parameters

*/
Trochoid::Trochoid(double a, double b, double xoff, double yoff, double toff,
       double rot) : xoff(xoff), yoff(yoff), toff(toff), a(a), b(b), rot(rot) {}

/*
3 ~project~

Calculate the point on the curve at instant ~t~

*/
Point Trochoid::project (double t) {
    t = toff + t * rot;
    
    double x = xoff + (a*t - b * sin(t));
    double y = yoff - (a - b * cos(t));
    
    return Point(x, y);
}

/*
4 ~create~

Calculate the Trochoid parameters to describe the curve of
point ~p~ under the FMRegion transformation unit ~tu~.

*/
Trochoid Trochoid::create (Point p, TransformationUnit& tu) {
    p = p.rotate(tu.c, tu.a0) + tu.v0;
    Point c = tu.c + tu.v0;
    Point v = tu.v;
    double rot = tu.a;
    
    // normalize the rotation
    if (rot < 0) {
        p = p.rotate(c, rot) + v;
        c = c + v;
        v = v * -1;
        rot = -rot;
    }
    
    // normalize and determine phase, orthogonalize vector
    p = p.rotate(c, -v.angle());
    Point pc = p - c;
    double vx = v.length();
    double ph = atan2(pc.y, pc.x);
    
    // Calculate parameters
    double a = vx/rot;
    double b = pc.length();
    double xoff = -ph * a + c.x + vx/rot * M_PI/2;
    double yoff = a + c.y;
    double toff = ph - M_PI/2;
    
    Trochoid t(a, b, xoff, yoff, toff, rot);
    t.setTransformation(c, v.angle());
    
    return t;
}

/*
5 ~rcurve~

Create an ~RCurve~ to describe a part of the Trochoid (between
the instants ~t1~ and ~t2~), which is then used
as part of the border of a ~CRegion~ (see also CRegion.cpp)

*/
RCurve Trochoid::rcurve (double t1, double t2) {
    RCurve ret;
    
    ret.type = "T";
    ret.angle = t_angle;
    ret.off = project(t1).rotate(t_center, t_angle);    
    ret.params.push_back(a);
    ret.params.push_back(b);
    ret.params.push_back(toff+t1*rot);
    ret.params.push_back(rot*(t2-t1));
    
    return ret;
}

/*
6 ~operator==~

Two trochoids are equal, if all parameters are equal.

*/
bool Trochoid::operator==(Trochoid& t) {
    return a==t.a&&b==t.b&&xoff==t.xoff&&yoff==t.yoff&&toff==t.toff&&rot==t.rot;
}

/*
7 ~ToString~

Return a string representation of this Trochoid.

*/
std::string Trochoid::ToString() {
    std::stringstream ss;
    ss << "Trochoid; a:" << a << " b:" << b << " xoff:" << xoff << " yoff:"
            << yoff << " toff:" << toff << " rot:" << rot;
    
    return ss.str();
}
