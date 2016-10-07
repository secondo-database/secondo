/* 
 * This file is part of libfmr
 * 
 * File:   Curve.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 14, 2016, 3:07 PM

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Class Curve

[TOC]

1 Overview

The abstract class Curve represents a single curve.
Implemented curves:

- Seg
- Trochoid
- Ravdoid


*/

#include "Curve.h"

using namespace fmr;

/*

2 ~addIntersection~

Define an intersection point on this curve.
~t1~ is the intersection time on this curve, ~t2~ is the time
on the peer ~curve~. Multiple curves can join at one intersection
time.
 
*/
void Curve::addIntersection(double t1, double t2, Curve *curve) {
    is[t1].push_back(std::pair<double, Curve*>(t2, curve));
}

/*
 
3 ~getAdjacentCurves~

Returns a vector of all curves, which have an intersection point
with this curve.
 
*/
std::vector<Curve *> Curve::getAdjacentCurves() {
    std::vector<Curve *> ret;
    std::map<double, std::vector<std::pair<double, Curve *> > >::iterator it =
                                                                     is.begin();
    while (it != is.end()) {
        std::vector<std::pair<double, Curve *> >::iterator cit =
                                                             it->second.begin();
        while (cit != (it->second.end())) {
            ret.push_back(cit->second);
            cit++;
        }
        it++;
    }

    return ret;
}

/*

4 ~getAdjacentIntersectionTimes~

Return the times of the previous and next intersection on this curve relative
to ~t~. ~t~ may be an intersection time itself, it is not taken into account
in that case.

*/
std::pair<double, double> Curve::getAdjacentIntersectionTimes(double t) {
    bool found = false;
    double tprev = nan(""), tnext = nan("");
    std::map<double, std::vector<std::pair<double, Curve *> > >::iterator it;
    
    for (it = is.begin(); it != is.end(); it++) {
        double tmp = it->first;
        if (tmp >= t) {
            if (tmp > t)
                tnext = tmp;
            found = true;
            break;
        }
        tprev = tmp;
    }
    it++;
    if (found && it != is.end() && isnan(tnext)) {
        tnext = it->first;
    }
    if (!isnan(tprev) && !valid((t + tprev) / 2))
        tprev = nan("");
    if (!isnan(tnext) && !valid((t + tnext) / 2))
        tnext = nan("");

    return std::pair<double, double>(tprev, tnext);
}

#define DELTA 0.0000001

/*
5 ~getAngle~

Return the angle of this curve at time ~t~. This is approximated
by calculating the angle between the point at time ~t~ and a
point a small step further. ~fromearlier~ specifies the direction.

*/
double Curve::getAngle(double t, bool fromearlier) {
    Point p1, p2;
    if (fromearlier) {
        p1 = project(t - DELTA);
        p2 = project(t);
    } else {
        p1 = project(t + DELTA);
        p2 = project(t);
    }
    double a = Seg(p1, p2).getAngle();

    return a;
}

/*
6 ~getAngle~

Return the angle of this curve in the point at time ~t1~ in
direction ~t2~ similar as in 5.

*/
double Curve::getAngle(double t1, double t2) {
    double delta = t2 > t1 ? DELTA : -DELTA;
    Point p1 = project(t1);
    Point p2 = project(t1 + delta);
    double a = Seg(p1, p2).getAngle();

    return a;
}

/*
7 ~getDeltaAngleCW~

Calculate the (clockwise) delta angle between two angles.

*/
static double getDeltaAngleCW(double a1, double a2) {
    double ret = a1 + M_PI - a2;
    if (ret < 0) {
        ret += 2*M_PI;
    } else if (ret > 2*M_PI) {
        ret -= 2*M_PI;
    }
    return ret;

}

/*
8 ~getNext~

Get next curve from the curve segment defined by this curve between times ~t1~
and ~t2~. This is the curve segment on an intersecting curve proceeding with the
lowest angle. The function is used to calculate the border of the traversed 
area.
 
*/
std::pair<double, std::pair<double, Curve*> > Curve::getNext(double t1,
                                                                    double t2) {
    double a1 = getAngle(t2, (bool) (t1 < t2)); // Angle of this curve
    double bestangle = nan("");
    std::pair<double, std::pair<double, Curve*> > bestpeer;
    bestpeer.first = nan(""); // No curve found
    
    std::vector<std::pair<double, Curve *> > peers = is[t2];
    for (int i = 0; i < peers.size(); i++) { // iterate all intersecting curves
        std::pair<double, Curve *> peer = peers[i];
        std::pair<double, double> tts =
                          peer.second->getAdjacentIntersectionTimes(peer.first);
        for (int j = 0; j < 2; j++) { // Calculate the angle in both directions
            double tt = (j == 0) ? tts.first : tts.second;
            if (isnan(tt))
                continue;
            double a2 = peer.second->getAngle(peer.first, tt); // Angle on peer
            double a = getDeltaAngleCW(a1, a2); // Angle between two curve segs
            if (a < 0.00001) // Too small angle, probably a numeric error
                continue;
            if (isnan(bestangle) || a < bestangle) { // Remember the best match
                bestangle = a;
                bestpeer = std::pair<double, std::pair<double, Curve *> >
                                                                     (tt, peer);
            }
        }
    }

    return bestpeer;
}
