/*
----
 * This file is part of libfmr
 * 
 * File:   TraversedAreaFactory.cpp
 * Author: Florian Heinz <fh@sysv.de>
 * 
 * Created on September 16, 2016, 10:48 AM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the operation ~traversedArea~

[TOC]

1 Overview

The operation ~traversedArea~ creates the traversed area from a FMRegion and
a TransformationUnit in several steps.

- First, the curves are created from the TransformationUnit ~tu~ and the
  FMRegion ~fmregion~.
- Then, the intersections between these curves are calculated and stored
  in the curves.
- The partitions of intersecting curves are calculated if the areas are
  non-contiguous.
- After that, the border cycles of these partitions are determined and
  converted to RCurve segments.
- Finally, a CRegion made up from RFaces is returned, which represents
  the traversed area.

*/

#include "fmr_TraversedAreaFactory.h"

using namespace fmr;

// Declarations
CRegion fmr::traversedArea(FMRegion& fmregion);
static std::vector<Curve*> createCurves(Region& region, TransformationUnit&);
static void findIntersections(std::vector<Curve*>&);
static std::vector<std::vector<Curve *> > getPartitions(std::vector<Curve *>);
static std::vector<RCurve> getCycle(std::vector<Curve*> cs);
static std::pair<double,std::pair<double,Curve*> >getFirst(std::vector<Curve*>);

/*
2 ~traversedArea~

Main function to calculate the traversed region of FMRegion ~fmregion~.

*/
CRegion fmr::traversedArea(FMRegion& fmregion) {
    CRegion ret;

    // Iterate over all transformations
    for (int i = 0; i < fmregion.trafos.size(); i++) {
        // Create the necessary curves
        std::vector<Curve*> curves = createCurves(fmregion.region,
                                                            fmregion.trafos[i]);
        // Find intersections between curves
        findIntersections(curves);
        // Divide the intersecting curves into disjunct partitons
        std::vector<std::vector<Curve *> > partitions = getPartitions(curves);
        for (int i = 0; i < partitions.size(); i++) {
            // From each partiton create a cycle of RCurve border segments
            std::vector<RCurve> ta = getCycle(partitions[i]);
            RFace rf;
            rf.face = ta;
            // Store this RFace into this CRegion
            ret.faces.push_back(rf);
        }
        
        // Free all curves
        std::vector<Curve *>::iterator it = curves.begin();
        while (it != curves.end()) {
            delete (*it);
            it++;
        }
    }

    return ret;
}

/*
3 ~createCurves~

Takes a TransformationUnit ~tu~ and creates the curve objects described by
the FMRegion under this transformation.
For each segment, the following curves are generated:

- Segment at start instant
- Segment at end instant
- Trochoid described by the start point during the time interval
- Ravdoid described by the segment during the time interval

*/
static std::vector<Curve*> createCurves(Region& region, TransformationUnit& tu){
    std::vector<Curve*> curves;
    for (int nrface = 0; nrface < region.faces.size(); nrface++) {
        Face& mainface = region.faces[nrface];
        for (int nrcycle = -1; nrcycle < (int)mainface.holes.size(); nrcycle++){
            Face& face = (nrcycle < 0) ? mainface : mainface.holes[nrcycle];
            for (int nrseg = 0; nrseg < face.segs.size(); nrseg++) {
                Seg seg = face.segs[nrseg];
                Trochoid *tr = new Trochoid(Trochoid::create(seg.i, tu));
                Ravdoid *rd = new Ravdoid(Ravdoid::create(seg, tu));
                SegT *si = new SegT(SegT::create(seg, tu, true));
                SegT *sf = new SegT(SegT::create(seg, tu, false));

                if (tr->b < PRECISION) {
                    curves.push_back(
                                 new SegT(Seg(tr->project(0), tr->project(1))));
                    delete tr;
                } else {
                    curves.push_back(tr);
                }

                if (rd->tmin1 == 0 && rd->tmin2 == 0)
                    delete rd;
                else
                    curves.push_back(rd);

                curves.push_back(si);
                curves.push_back(sf);
            }
        }
    }

    return curves;
}

/*
4 ~findIntersections~

Find all intersections in a set of curves. Stores the intersection
points together with the peer curve in the curve.

*/
static void findIntersections(std::vector<Curve*>& curves) {
    std::vector<Curve *>::iterator c1 = curves.begin(), c2;
    
    while (c1 != curves.end()) {
        c2 = std::vector<Curve *>::iterator(c1);
        while (c2 != curves.end()) {
            Trochoid *t1 = dynamic_cast<Trochoid*> (*c1), *t2 =
                                                  dynamic_cast<Trochoid*> (*c2);
            Ravdoid *r1 = dynamic_cast<Ravdoid*> (*c1), *r2 =
                                                  dynamic_cast<Ravdoid *> (*c2);
            SegT *s1 = dynamic_cast<SegT*> (*c1), *s2 =
                                                     dynamic_cast<SegT *> (*c2);

            std::vector<Point> it;

            if (false) { // Dummy entry
            } else if (t1 && t2) {
                // Intersection between two trochoids. The trochoid with
                // the higher b-value should be the first parameter, since
                // the calculation is numerically more stable then.
                if (t1->b > t2->b)
                    it = ISTrochoids(*t1, *t2).findIntersection();
                else
                    it = ISTrochoids(*t2, *t1).findIntersection();
            } else if (t1 && r2) {
                // Intersection between trochoid and ravdoid
                it = ISTrocRavd(*t1, *r2).findIntersection();
            } else if (r1 && t2) {
                // Intersection between trochoid and ravdoid
                it = ISTrocRavd(*t2, *r1).findIntersection();
            } else if (r1 && r2) {
                // Intersection between two ravdoids
                it = ISRavdoids(*r1, *r2).findIntersection();
            } else if ((t1 || r1) && s2) {
                // Intersection between a segment and a curve
                it = ISSegCurve(*s2, **c1).findIntersection();
            } else if (s1 && (t2 || r2)) {
                // Intersection between a segment and a curve
                it = ISSegCurve(*s1, **c2).findIntersection();
            } else if (s1 && s2 && s1 != s2) {
                // Intersection between two segments
                // A self-intersection cannot happen here
                it = ISSegCurve(*s1, *s2).findIntersection();
            }
            c2++;
        }
        c1++;
    }
}

/*
5 ~getPartitions~

Divide the intersecting curves into disjunct partitions.

*/
static std::vector<std::vector<Curve *> > getPartitions(std::vector<Curve *>cs){
    std::vector<std::vector<Curve *> > ret;

    while (cs.size() > 0) {
        Curve *c = *(cs.begin());
        cs.erase(cs.begin());
        if (c->getAdjacentCurves().size() == 0) {
            continue;
        }
        std::vector<Curve*> partition;
        partition.push_back(c);
        int added;
        // Iterate and add all adjacent curves (and the curves adjacent to
        // those) until no more curves were added
        do {
            added = 0;
            std::vector<Curve *> newp;
            for (int i = 0; i < partition.size(); i++) {
                Curve *t = partition[i];
                for (int j = 0; j < t->getAdjacentCurves().size(); j++) {
                    Curve *adj = t->getAdjacentCurves()[j];
                    if (std::find(partition.begin(), partition.end(), adj) ==
                                                              partition.end() &&
                                  std::find(newp.begin(), newp.end(), adj) ==
                                                                   newp.end()) {
                        newp.push_back(adj);
                        std::vector<Curve*>::iterator it = 
                                           std::find(cs.begin(), cs.end(), adj);
                        if (it != cs.end())
                            cs.erase(it);
                        added++;
                    }
                }
            }
            partition.insert(partition.end(), newp.begin(), newp.end());
        } while (added > 0);
        ret.push_back(partition);
    }

    return ret;
}

/*
6 ~getCycle~

Creates a cycle of ~RCurve~ segments (see RCurve.cpp) from a set of
curves. The intersections have to be calculated first (see
~findIntersections()~. The resulting set of RCurve segments construct
the borderline of the set of intersecting curves ~cs~.

*/
static std::vector<RCurve> getCycle(std::vector<Curve*> cs) {
    // Find a suitable start segment
    std::pair<double, std::pair<double, Curve*> > p = getFirst(cs);

    std::vector<RCurve> ta;
    Point start;
    int i = 1000; // 1000 iterations are maximum
    do {
        // The current curve
        Curve *cc = p.second.second;
        // The start instant of the curve segment
        double t1 = p.second.first;
        // The end instant of the curve segment
        double t2 = p.first;
        Point p1 = cc->project(t1);
        Point p2 = cc->project(t2);
        if (!start.valid())
            start = p2;
        else if (p2.near(start))
            break; // The cycle is closed now!
        // Create an RCurve segment of the current curve from
        // instant t1 to t2 and add it to the cycle
        ta.push_back(cc->rcurve(t1, t2));
        // Determine the next curve segment of the border cycle
        p = cc->getNext(t1, t2);
    } while (i-- > 0);

    return ta;
}

/*
7 ~getFirst~

Try to find a suitable curve segment to start the construction of the border
cycle. This curve segment has to be part of the border cycle and the orientation
has to be correct (counterclockwise).
This approach determines the Trochoid with the biggest ~b~-parameter. If we have
a full cycle (or the correct part of it),a point on that trochoid has the lowest
or highest y-coordinate and is thus part of the border. If the rotation angle is
too small, assume that the part from an endpoint of that trochoid to the next
intersection is part of the cycle.

*/
static std::pair<double, std::pair<double, Curve*> > getFirst(
                                                       std::vector<Curve*> cs) {
    Trochoid *c = NULL;
    // Find trochoid c with the biggest b-value
    for (int i = 0; i < cs.size(); i++) {
        Trochoid* o = dynamic_cast<Trochoid*> (cs[i]);
        if (o && (!c || c->b < o->b))
            c = o;
    }

    int index = 0;
    double tt = 0;
    double pd = 2 * M_PI / c->rot;
    // t1 is the time of the lowest y-coordinate of trochoid c
    double t1 = (-c->toff - M_PI) / c->rot, t2, t3, t4;
    while (t1 < 0) {
        t1 += pd;
    }
    if (t1 > 1) {
        t1 -= pd;
    }
    t2 = t1 + pd / 2;
    t3 = t2 + pd / 2;
    t4 = t3 + pd / 2;

    if (t1 > 0 && t1 < 1) {
        tt = t1;
        index = 1;
    } else if (t2 > 0 && t2 < 1) {
        tt = t2;
        index = c->b > c->a ? 1 : 0;
    } else if (t1 < 0 && 0 < t2) {
        tt = 0;
        if ((t1 + t2) / 2 < 0) {
            index = c->b > c->a ? 1 : 0;
        } else {
            index = 1;
        }
    } else if (t2 < 0 && 0 < t3) {
        tt = 0;
        if ((t2 + t3) / 2 < 0) {
            index = 1;
        } else {
            index = c->b > c->a ? 1 : 0;
        }
    } else if (t3 < 0 && 0 < t4) {
        tt = 0;
        if ((t3 + t4) / 2 < 0) {
            index = c->b > c->a ? 1 : 0;
        } else {
            index = 1;
        }
    }
    // Find the next intersection point of the segment
    std::pair<double, double> tx = c->getAdjacentIntersectionTimes(tt);
    double tt2 = (index == 0) ? tx.first : tx.second;
    Point start = c->project(tt2);

    // Return the succeeding rcurve segment, which is also part of the
    // cycle if c@[t1;t2] is part of the cycle
    return c->getNext(tt, tt2);
}
