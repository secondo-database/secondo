/*
----
 * This file is part of libfmr
 * 
 * File:   MPoint.cpp
 * Author: Florian Heinz <fh@sysv.de>
 *
 * Created on September 10, 2016, 6:14 PM
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the class ~MPoint~

[TOC]

1 Overview

The class ~MPoint~ represents a moving point made up from one or more
units (see class ~UPoint~) each consisting of a time interval and
a start- and endpoint. Its main task is to implement the operation
~inside~, which calculates the times when the moving point is inside
an ~FMRegion~.
Signature: mpoint x fmregion -> mbool
Example: mpoint1 inside fmregion1

*/


#include "MPoint.h"

using namespace fmr;


/*
2 Constructor

Construct an ~MPoint~ object from a RList representation.

*/
MPoint::MPoint(RList& l) {
    for (int i = 0; i < l.size(); i++) {
        units.push_back(UPoint(l[i]));
    }
}

/*
3 ~inside~

Calculates the times when this ~MPoint~ is inside the
FMRegion ~fmr~ and returns a corresponding ~MBool~ object.

*/
MBool MPoint::inside(FMRegion &fmr) {
    int tui = 0, upi = 0;
    std::vector<UPoint> nunits;
    
    // First, calculate new units (~UPoint~) to match the times of the
    // fmregion transformations and to project the relative movement of
    // the point to the moving region solely on the point, so the region
    // can be seen as static from now on
    while (tui < fmr.trafos.size() && upi < units.size()) {
        TransformationUnit& tu = fmr.trafos[tui];
        UPoint& up = units[upi];
        UPoint tup = up.transform(tu);
        if (tup.valid)
            nunits.push_back(tup);
        if (tu.iv.end <= up.iv.end) {
            tui++;
        }
        if (up.iv.end <= tu.iv.end) {
            upi++;
        }
    }
    
    Region& region = fmr.region;
    MBool mb;
    for (int nrunit = 0; nrunit < nunits.size(); nrunit++) {
        // Iterate over the units
        UPoint& up = nunits[nrunit];
        std::vector<double> is;
        // And for each unit over all segments of the (now static) region
        for (int nrface = 0; nrface < region.faces.size(); nrface++) {
            Face& mainface = region.faces[nrface];
            for (int nrcycle = -1; nrcycle < (int) mainface.holes.size();
                                                                    nrcycle++) {
                Face& face = (nrcycle < 0) ? mainface : mainface.holes[nrcycle];
                for (int nrseg = 0; nrseg < face.segs.size(); nrseg++) {
                    // Find all intersections between the segment
                    // and the path of the UPoint
                    SegT segt(face.segs[nrseg]);
                    ISSegCurve isc(segt, up);
                    std::vector<double> roots = isc.findRoots(0, 1);
                    is.insert(is.end(), roots.begin(), roots.end());
                }
            }
        }
        // Always include the end of the time interval, too
        is.push_back(1);
        // Sort the intersection times ascending
        std::sort(is.begin(), is.end());
        double prev = 0;
        for (int nris = 0; nris < is.size(); nris++) {
            // Create an mbool unit (ubool) for each intersection
            double p = is[nris];
            double middle = (prev + p)/2;
            bool lc = (prev != 0) || up.iv.lc;
            bool rc = (p    == 1) && up.iv.rc;
            Interval niv(up.iv.project(prev), up.iv.project(p), lc, rc);
            // Project the point in the middle of the time interval and store,
            // if it is inside or outside the (static) region
            mb.addUnit(niv, region.inside(up.project(middle)));
            prev = p;
        }
    }
    
    return mb;
}
