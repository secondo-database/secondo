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



1 Cone

Presently only paraxial cones are found.

*/

#include "GpCone.h"

#include "../utility/MathUtils.h"

using namespace pointcloud2;
using namespace std;

double GpCone::SHIFT_ID[3] = { 1000.0, 2000.0, 3000.0 };

std::string GpCone::getCaption(const bool plural) const {
    return plural ? "cones" : "cone";
}

unsigned GpCone::getTupleSize() const {
    return TUPLE_SIZE_CONE;
}

unique_ptr<vector<DbScanPoint<DUAL_DIM_CONE>>>
        GpCone::projectTupleToDual(
        const std::vector<SamplePoint>& tuple,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) {

    assert (tuple.size() == TUPLE_SIZE_CONE);

    // calculate one dual point for each axis (z, x, y)
    // the following code is worded with a cone parallel to the z axis
    // in mind; however, if we "shift" the dimensions, we can similarly
    // calculate from the tuple the dual points for cones parallel to the
    // x and y axis
    unique_ptr<vector<DbScanPoint<DUAL_DIM_CONE>>> results(
            new vector<DbScanPoint<DUAL_DIM_CONE>>());
    for (unsigned shift = 0; shift < DIMENSIONS; ++shift) {
        // create a linear system to calculate the apex (cx, cy, cz) of this
        // cone and the value "aperture" in
        //   aperture = radius / z-distance-to-apex
        // (so, aperture is the tangens of the half angle at the apex)

        // a point (px, py, pz) belongs to the cone if
        //   sqrt((cx - px)^2 + (cy - py)^2) = aperture * (cz - pz).
        // If we subtract the equations for two point p, q from each other,
        // we get four unknown x, y, a in a linear system:
        //   2(px - py)x + 2(py - qy)y - 2(pz - qz)*a^2*z + (pz^2 + qz^2)a^2
        //                                   = px^2 + py^2 - qx^2 - qy^2
        std::vector<std::array<double, EQUATION_COUNT_CONE + 1>> eqs;
        // get the first point p
        const SamplePoint& p = tuple[0];
        double px = p._coords[shiftDim(0, shift)];
        double py = p._coords[shiftDim(1, shift)];
        double pz = p._coords[shiftDim(2, shift)];
        for (size_t i = 0; i < EQUATION_COUNT_CONE; ++i) {
            // create equations using the distance between the first point p
            // and the current point q
            const SamplePoint& q = tuple[1 + i]; // tuple[0] is point "p"
            double qx = q._coords[shiftDim(0, shift)];
            double qy = q._coords[shiftDim(1, shift)];
            double qz = q._coords[shiftDim(2, shift)];
            double res = px * px + py * py - qx * qx - qy * qy;
              array<double, EQUATION_COUNT_CONE + 1> eq {
                     2.0 * (px - qx), 2.0 * (py - qy), -2.0 * (pz - qz),
                             pz * pz - qz * qz, res };
            eqs.push_back(std::move(eq));
        }

        std::vector<double> result =
                solveLinearSystem<EQUATION_COUNT_CONE>(eqs);
        if (result.size() == 0)
            continue; // next shift - linear system could be solved

        // now we get cz, the z coordinate of the apex, using the first point
        // and (cx - px)^2 + (cy - py)^2 = aperture * (cz - pz).
        double cx = result[0];
        double cy = result[1];
        double aaz = result[2];
        double aa = result[3];
        double cz = aaz / aa;
        if (aa <= 0.0)
            continue;
        double aperture = std::sqrt(aa);

        // test whether the calculated apex (cx, cy, cz) makes sense
        if (isPointFarOutsideShifted(cx, cy, cz, shift,
                bboxOfEntrieCloud)) {
            // the result of the linear system was a huge cone,
            // the apex of which is far outside the bbox of the cloud
            continue; // next shift
        }

        // we challenge this cone with one or several additional point
        // from the same neighborhood in order to avoid noise in dual space,
        // especially as we are looking in all three DIMENSIONS
        bool challengeFail = false;
        // cos is required only if the cone is challenged
        double cos = (CHALLENGE_COUNT_CONE == 0) ? 0 :
                1 / std::sqrt(1 + aperture * aperture);
        for (unsigned i = 0; i < CHALLENGE_COUNT_CONE; ++i) {
            const SamplePoint& point = tuple[1 + EQUATION_COUNT_CONE + i];
            // the test is the same as in getPredicateForShape:
            double dx = point._coords[shiftDim(0, shift)] - cx;
            double dy = point._coords[shiftDim(1, shift)] - cy;
            double dz = point._coords[shiftDim(2, shift)] - cz;
            double radius = std::sqrt(dx * dx + dy * dy);
            double expectedRadius = std::abs(aperture * dz);
            double diff = cos * (expectedRadius - radius);
            if (std::abs(diff) > params._matchTolerance) {
                challengeFail = true;
                break;
            }
        }
        if (challengeFail)
            continue;

        // create the dual point. In DbScan, the same eps distance is used
        // for all dimensions, so we do not use "aperture" directly
        // (aperture = 0.1 and aperture = 0.2 can lead to very different cones
        // if only they are high enough). Instead, we use
        // "aperture * minimumObjectSize", i.e. the radius at the bottom
        // of a cone of a realistic height
        DbScanPoint<DUAL_DIM_CONE> dualPoint;
        dualPoint.initialize();
        dualPoint._coords[0] = SHIFT_ID[shift];
        dualPoint._coords[1] = cx;
        dualPoint._coords[2] = cy;
        dualPoint._coords[3] = cz;
        dualPoint._coords[4] = aperture * params._minimumObjectExtent;

        if (REPORT_DETAILED) { // do this before std::move is used
            std::cout << "+ dual point for " << getCaption(false) << ": "
            << dualPoint.toString() << std::endl;
        }

        results->push_back(dualPoint);
    }
    return results;
}

GeomPredicate GpCone::getPredicateForShape(
        const PointBase<DUAL_DIM_CONE>& shape,
        const ParamsAnalyzeGeom& params) const {

    // rather than calculating std::sqrt() each time, our predicate will
    // calculate the square of the distance of the given point to
    // the cone's axis and compare it to a tolerated range

    // interpret the coords of the dual point. Depending on the value of
    // axis (0, 1, 2), the cone is parallel to the z, x, y axis.
    // The code is written with the z axis in mind
    unsigned shift = getShiftFromId(shape._coords[0]);
    double centerX = shape._coords[1];
    double centerY = shape._coords[2];
    double centerZ = shape._coords[3];
    double aperture = shape._coords[4] / params._minimumObjectExtent;
    // calculate the cos of the (half) aperture angle at the apex
    double cos = 1 / std::sqrt(1 + aperture * aperture);
    double tolerance = params._matchTolerance;

    GeomPredicate predicate =
    [shift, centerX, centerY, centerZ, aperture, cos, tolerance]
            (const DbScanPoint<DIMENSIONS>& point)
    {
       double dx = point._coords[shiftDim(0, shift)] - centerX;
       double dy = point._coords[shiftDim(1, shift)] - centerY;
       double dz = point._coords[shiftDim(2, shift)] - centerZ;
       double radius = std::sqrt(dx * dx + dy * dy);
       double expectedRadius = std::abs(aperture * dz);
       // with cos we get the distance of point to the surface of the cone
       // i.e. the distance to the line running through the apex
       // (cx, cy, cz) and the point of the cone's surface that is
       // between (cx, cy, pz(!)) and point.
       double diff = cos * (expectedRadius - radius);
       return (std::abs(diff) < tolerance);
    };
    return predicate;
}

BboxPredicate GpCone::getBboxPredicateForShape(
        const PointBase<DUAL_DIM_CONE>& shape,
        const ParamsAnalyzeGeom& params) const {

    // interpret the coords of the dual point
    unsigned shift = getShiftFromId(shape._coords[0]);
    double centerX = shape._coords[1];
    double centerY = shape._coords[2];
    double centerZ = shape._coords[3];
    double aperture = shape._coords[4] / params._minimumObjectExtent;
    double tolerance = params._matchTolerance;

    BboxPredicate predicate =
            [shift, centerX, centerY, centerZ, aperture, tolerance]
             (const Rectangle2<DIMENSIONS>& bbox)
    {
        // determine whether all corners of the bbox are on the same side
        // (i.e. either all "outside" or all "inside") of the cone. If so,
        // there is no intersection and false is returned; otherwise, the
        // bbox intersects with the cone.
        size_t combinations = (1 << DIMENSIONS);
        bool sign = false;
        int d0 = shiftDim(0, shift);
        int d1 = shiftDim(1, shift);
        int d2 = shiftDim(2, shift);
        for (size_t cornerBits = 0; cornerBits < combinations; ++cornerBits) {
            double x = (cornerBits & 1) ?
                    bbox.MinD(d0) - tolerance : bbox.MaxD(d0) + tolerance;
            double y = (cornerBits & 2) ?
                    bbox.MinD(d1) - tolerance : bbox.MaxD(d1) + tolerance;
            double z = (cornerBits & 4) ?
                    bbox.MinD(d2) - tolerance : bbox.MaxD(d2) + tolerance;

            double dx = x - centerX;
            double dy = y - centerY;
            double dz = z - centerZ;
            double radius = std::sqrt(dx * dx + dy * dy);
            double expectedRadius = std::abs(aperture * dz);
            double diff = expectedRadius - radius;
            if (cornerBits == 0)
                sign = std::signbit(diff);
            else if (sign != std::signbit(diff))
                return true;
        }
        if (sign) {
            // all bbox corners are "outside" the cone; the cone can still
            // extend into it, if either the x or the y range of the bbox
            // (or both) contains the cone's axis:
            if (bbox.MinD(d0) <= centerX && centerX <= bbox.MaxD(d0))
                return true;
            if (bbox.MinD(d1) <= centerY && centerY <= bbox.MaxD(d1))
                return true;
        }
        return false;
    };
    return predicate;
}

unsigned GpCone::shiftDim(const unsigned dimension,
        const unsigned shift) {
    return (dimension + shift) % DIMENSIONS;
}

unsigned GpCone::getShiftFromId(const double shiftId) {
    for (unsigned shift = 0; shift < DIMENSIONS; ++shift) {
        if (shiftId == SHIFT_ID[shift]) // should not need AlmostEqual
            return shift;
    }
    assert (false);
    return 0;
}

bool GpCone::isPointFarOutsideShifted(
        const double x, const double y, const double z,
        const unsigned shift, const Rectangle<DIMENSIONS>& bbox) {
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
    unsigned d2 = shiftDim(2, shift);
    dist = MAX(dist, bbox.MinD(d2) - z);
    dist = MAX(dist, z - bbox.MaxD(d2));
    if (dist == 0.0) // the point is inside the bbox
        return false;

    double diam = 0.0;
    for (unsigned i = 0; i < DIMENSIONS; ++i)
        diam = MAX(diam, bbox.MaxD(i) - bbox.MinD(i));

    return (dist / diam > MAX_FACTOR);
}
