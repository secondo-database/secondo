/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Cylinder

Presently only paraxial cylinders are found.

*/
#include "GpCylinder.h"

#include "../utility/MathUtils.h"

using namespace pointcloud2;
using namespace std;

double GpCylinder::SHIFT_ID[3] = { 1000.0, 2000.0, 3000.0 };

std::string GpCylinder::getCaption(const bool plural) const {
    return plural ? "cylinders" : "cylinder";
}

unsigned GpCylinder::getTupleSize() const {
    return TUPLE_SIZE_CYLINDER;
}

unique_ptr<vector<DbScanPoint<DUAL_DIM_CYLINDER>>>
GpCylinder::projectTupleToDual(const std::vector<SamplePoint>& tuple,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) {

    assert(tuple.size() == TUPLE_SIZE_CYLINDER);

    // calculate one dual point for each axis (z, x, y)
    // the following code is worded with a cylinder parallel to the z axis
    // in mind; however, if we "shift" the dimensions, we can similarly
    // calculate from the tuple the dual points for cylinders parallel to the
    // x and y axis
    unique_ptr<vector<DbScanPoint<DUAL_DIM_CYLINDER>>> results(
            new vector<DbScanPoint<DUAL_DIM_CYLINDER>>());
    for (unsigned shift = 0; shift < DIMENSIONS; ++shift) {
        // create a linear system to calculate the x and y coordinate of the
        // axis of this cylinder. For the following, cp. section 2.1 of
        // http://www.ipb.uni-bonn.de/pdfs/Beder2006Direkte.pdf
        std::vector<std::array<double, EQUATION_COUNT_CYLINDER + 1>> eqs;
        for (unsigned i = 0; i < EQUATION_COUNT_CYLINDER; ++i) {
            const SamplePoint& point = tuple[i];
            double x = point._coords[shiftDim(0, shift)];
            double y = point._coords[shiftDim(1, shift)];
            array<double, EQUATION_COUNT_CYLINDER + 1> eq
                { 2.0 * x, 2.0 * y, -1.0, x * x + y * y };
            eqs.push_back(std::move(eq));
        }

        std::vector<double> result =
                solveLinearSystem<EQUATION_COUNT_CYLINDER>(eqs);
        if (result.size() == 0)
            continue; // next shift - linear system could be solved

        // calculate the position of the cylinder's axis and its radius
        double cx = result[0];
        double cy = result[1];
        double u = result[2]; // see below
        if (isPointFarOutsideShifted(cx, cy, shift,
                  bboxOfEntrieCloud)) {
            // the three points are almost on the same line, so a huge
            // cylinder was calculated from them the center of which is
            // far outside the bbox of the cloud
            continue;// next shift
        }
        double radius = std::sqrt(cx * cx + cy * cy - u);

        // we challenge this cylinder with one or several additional point
        // from the same neighborhood in order to avoid noise in dual space,
        // especially as we are looking in all three DIMENSIONS
        bool challengeFail = false;
        for (unsigned i = 0; i < CHALLENGE_COUNT_CYLINDER; ++i) {
            const SamplePoint& point = tuple[EQUATION_COUNT_CYLINDER + i];
            // the test is the same as in getPredicateForShape:
            double dx = point._coords[shiftDim(0, shift)] - cx;
            double dy = point._coords[shiftDim(1, shift)] - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (std::abs(radius - dist) > params._matchTolerance) {
                challengeFail = true;
                break;
            }
        }
        if (challengeFail)
            continue;

        // create the dual point
        DbScanPoint<DUAL_DIM_CYLINDER> dualPoint;
        dualPoint.initialize();
        dualPoint._coords[0] = SHIFT_ID[shift];
        dualPoint._coords[1] = cx;
        dualPoint._coords[2] = cy;
        dualPoint._coords[3] = radius;

        if (REPORT_DETAILED) { // do this before std::move is used
            std::cout << "+ dual point for " << getCaption(false) << ": "
                    << dualPoint.toString() << std::endl;
        }

        results->push_back(dualPoint);
    }
    return results;
}

GeomPredicate GpCylinder::getPredicateForShape(
        const PointBase<DUAL_DIM_CYLINDER>& shape,
        const ParamsAnalyzeGeom& params) const {

    // rather than calculating std::sqrt() each time, our predicate will
    // calculate the square of the distance of the given point to
    // the cylinder's axis and compare it to a tolerated range

    // interpret the coords of the dual point. Depending on the value of
    // axis (0, 1, 2), the cylinder is parallel to the z, x, y axis.
    // The code is written with the z axis in mind
    unsigned shift = getShiftFromId(shape._coords[0]);
    double centerX = shape._coords[1];
    double centerY = shape._coords[2];
    double radius = shape._coords[3];
    // the (square) radius range in which points will be considered
    // as belonging to the cylinder
    double radiusMin = radius - params._matchTolerance;
    double radiusMax = radius + params._matchTolerance;
    double sqRadiusMin = radiusMin * radiusMin;
    double sqRadiusMax = radiusMax * radiusMax;

    GeomPredicate predicate =
    [shift, centerX, centerY, sqRadiusMin, sqRadiusMax]
            (const DbScanPoint<DIMENSIONS>& point)
    {
       double dx = point._coords[shiftDim(0, shift)] - centerX;
       double dy = point._coords[shiftDim(1, shift)] - centerY;
       double sqDist = dx * dx + dy * dy;
       return (sqDist >= sqRadiusMin) && (sqDist <= sqRadiusMax);
    };
    return predicate;
}

BboxPredicate GpCylinder::getBboxPredicateForShape(
        const PointBase<DUAL_DIM_CYLINDER>& shape,
        const ParamsAnalyzeGeom& params) const {

    // interpret the coords of the dual point
    unsigned shift = getShiftFromId(shape._coords[0]);
    double centerX = shape._coords[1];
    double centerY = shape._coords[2];
    double radius = shape._coords[3] + params._matchTolerance;
    double minX = centerX - radius;
    double maxX = centerX + radius;
    double minY = centerY - radius;
    double maxY = centerY + radius;

    BboxPredicate predicate =
        [shift, minX, maxX, minY, maxY](const Rectangle2<DIMENSIONS>& bbox) {
        // this predicate only checks whether the cylinder's bounding box
        // intersects with the given bounding box; in some cases, it will
        // return true although there is no actual intersection with the
        // cylinder; however, the points contained in bbox will be checked
        // individually anyway.
        int d0 = shiftDim(0, shift);
        int d1 = shiftDim(1, shift);
        if (maxX < bbox.MinD(d0) || bbox.MaxD(d0) < minX)
            return false;
        if (maxY < bbox.MinD(d1) || bbox.MaxD(d1) < minY)
            return false;
        return true;
    };
    return predicate;
}

unsigned GpCylinder::shiftDim(const unsigned dimension,
        const unsigned shift) {
    return (dimension + shift) % DIMENSIONS;
}

unsigned GpCylinder::getShiftFromId(const double shiftId) {
    for (unsigned shift = 0; shift < DIMENSIONS; ++shift) {
        if (shiftId == SHIFT_ID[shift]) // should not need AlmostEqual
            return shift;
    }
    assert (false);
    return 0;
}

bool GpCylinder::isPointFarOutsideShifted(const double x,
        const double y, const unsigned shift,
        const Rectangle<DIMENSIONS>& bbox) {
    // points are considered "far outside" if their distance to the bbox is
    // larger than (MAX_FACTOR * maximum bbox range) in at least one dimension
    const double MAX_FACTOR = 2.0;

    double dist = 0.0;
    unsigned d0 = shiftDim(0, shift);
    dist = MAX(dist, bbox.MinD(d0) - x);
    dist = MAX(dist, x - bbox.MaxD(d0));
    unsigned d1 = shiftDim(1, shift);
    dist = MAX(dist, bbox.MinD(d1) - y);
    dist = MAX(dist, y - bbox.MaxD(d1));
    if (dist == 0.0) // the point is inside the bbox
        return false;

    double diam = 0.0;
    for (unsigned i = 0; i < DIMENSIONS; ++i)
        diam = MAX(diam, bbox.MaxD(i) - bbox.MinD(i));

    return (dist / diam > MAX_FACTOR);
}
