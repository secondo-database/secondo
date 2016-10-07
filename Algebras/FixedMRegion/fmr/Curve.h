/* 
----
 * This file is part of libfmr
 * 
 * File:   Curve.h
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 14, 2016, 3:07 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header file for class Curve

[TOC]

1 Overview

Header file with the class definition for the abstract class ~Curve~.

2 Includes and definitions

*/

#ifndef FMR_CURVE_H
#define FMR_CURVE_H

#include <cassert>
#include <string>
#include <sstream>
#include <map>

#include "RCurve.h"
#include "Point.h"
#include "Seg.h"

#define PRECISION 0.0000001

namespace fmr {

class RCurve; // Forward declaration

/*

3 Definition of the class ~Curve~

*/
    
class Curve {
public:
    Curve();
    virtual ~Curve();

    void addIntersection(double t1, double t2, Curve *c);
    double fx(double t) { return project(t).x; }
    double fy(double t) { return project(t).y; }
    
    virtual std::string ToString() { return "Curve"; }
    virtual Point project (double t) = 0;
    virtual bool valid (double t) { return (t >= 0 - PRECISION &&
                                            t <= 1 + PRECISION); }
    virtual RCurve rcurve (double t1, double t2) = 0;
    std::pair<double, double> getAdjacentIntersectionTimes(double t);
    std::vector<Curve *> getAdjacentCurves();
    std::pair<double, std::pair<double, Curve*> > getNext(double t1, double t2);
    
    void setTransformation (Point _t_center, double _t_angle) {
        t_center = _t_center;
        t_angle = _t_angle;
    }
    Point t_center;
    double t_angle;
    
    std::map<double, std::vector<std::pair<double, Curve *> > > is;
    
private:
    double getAngle(double t, bool fromearlier);
    double getAngle(double t1, double t2);

};

}

#endif  /* CURVE_H */