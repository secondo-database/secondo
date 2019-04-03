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



1 Plane

*/
#include "GpPlane.h"

using namespace pointcloud2;
using namespace std;

std::string GpPlane::getCaption(bool plural) const {
    return plural ? "planes" : "plane";
}

unsigned GpPlane::getTupleSize() const {
    return TUPLE_SIZE_PLANE;
}

std::unique_ptr<std::vector<DbScanPoint<DUAL_DIM_PLANE>>>
GpPlane::projectTupleToDual(const std::vector<SamplePoint>& tuple,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& boxOfEntireCloud) {
    assert(tuple.size() == TUPLE_SIZE_PLANE);

    // support vector (= dt. Stuetzvektor)
    const SamplePoint& p = tuple[0];
    double px = p._coords[0];
    double py = p._coords[1];
    double pz = p._coords[2];

    // vectors (q - p) and (r - p)
    const SamplePoint& q = tuple[1];
    double qpx = q._coords[0] - px;
    double qpy = q._coords[1] - py;
    double qpz = q._coords[2] - pz;
    const SamplePoint& r = tuple[2];
    double rpx = r._coords[0] - px;
    double rpy = r._coords[1] - py;
    double rpz = r._coords[2] - pz;

    // normal vector n = (q - p) x (r - p)
    double nx = qpy * rpz - qpz * rpy;
    double ny = qpz * rpx - qpx * rpz;
    double nz = qpx * rpy - qpy * rpx;
    double nLengthSquare = nx * nx + ny * ny + nz * nz;
    if (nLengthSquare <= 0.0)
        return nullptr; // p, q, r are on the same line
    double nLength = std::sqrt(nLengthSquare);
    nx /= nLength;
    ny /= nLength;
    nz /= nLength;

    /* to create a dual point for this plane, an obvious idea would be to
     * use the coefficients of the equation a*x + b*y + c*z = d defining
     * this plane, where (a,b,c) is the direction and - if (a,b,c) is
     * normalized - d the length of the normal vector pointing from the
     * origin to the plane. However, this would put remote planes at a severe
     * disadvantage: at a high distance from the origin, a small diffusion
     * in the original points will lead to a very different normal vector
     * making it impossible for DBSCAN to find an appropriate cluster. (Note
     * that intersection point of the normal vector and the plane can be at a
     * very different place from where the pointcloud points are that belong
     * to this plane!)
     *
     * Therefore, rather than the origin, we use different "points of
     * reference" (i.e. "alternative origins") to express the normal vector.
     * These points of reference will always be close to the actual points
     * in the cloud, making the normal vector short and thus enabling DBSCAN
     * to cluster dual points when there is some diffusion. Obviously, we
     * will need to enter the respective "point of reference" as part of
     * the dual point (indices 0-2) along with the normal vector (indices 3-5).
     * We use a "raster" to get a point of reference in the vicinity of the
     * first point p in the tuple.
     *
     * The fact that, even with no diffusion at all, the same geometric plane
     * may now be represented by various dual points (due to different points
     * of reference) is "corrected" later: The planes that are identified as
     * a result of the DBSCAN in dual space will be matched with the actual
     * points using the getPredicateForShape and getBboxPredicateForShape
     * predicates, and those predicates are "universal", i.e. independent
     * of the various points of reference.
     */

    // get a "point of reference" c in the vicinity of these points
    // (esp. of point p)
    double cellExtent = 8.0 * params._minimumObjectExtent;
    double cx = rasterPos(px, cellExtent);
    double cy = rasterPos(py, cellExtent);
    double cz = rasterPos(pz, cellExtent);

    // equation for this plane:
    // 1) a*x + b*y + c*z = d  if (x,y,z) is relative to the origin, or
    // 2) a*x + b*y + c*z = dd if (x, y, z) is relative to c
    // double a = nx;
    // double b = ny;
    // double c = nz;
    double dd = nx * (px - cx) + ny * (py - cy) + nz * (pz - cz);
    double d = nx * px + ny * py + nz * pz;

    // we challenge this sphere with one or several additional point from
    // the same neighborhood in order to avoid noise in dual space
    for (unsigned i = 0; i < CHALLENGE_COUNT_PLANE ; ++i) {
        const SamplePoint& point = tuple[EQUATION_COUNT_PLANE + i];
        // the test is the same as in getPredicateForShape:
        double dist = nx * point._coords[0]
                    + ny * point._coords[1]
                    + nz * point._coords[2] - d;
        if (std::abs(dist) > params._matchTolerance)
            return nullptr; // challenge failed
    }

    /* we do not use a, b, c, d directly in the dual point, since |a|, |b|,
     * |c| are scaled to be <= 1.0, while dd uses an actual scaling of the
     * Pointcloud2, and both scalings could be completely different, making
     * cluster distances in the dimensions a, b, c incommensurable with
     * cluster distances in d.
     *
     * Instead, we simply use the vector from the point of reference (c) to
     * the plane. Thus, all dimensions are commensurable. */

    // create the dual point
    DbScanPoint<DUAL_DIM_PLANE> dualPoint;
    dualPoint.initialize();
    dualPoint._coords[0] = cx;
    dualPoint._coords[1] = cy;
    dualPoint._coords[2] = cz;
    dualPoint._coords[3] = nx * dd;
    dualPoint._coords[4] = ny * dd;
    dualPoint._coords[5] = nz * dd;

    if (REPORT_DETAILED) {
        std::cout << "+ dual point for " << getCaption(false) << ": "
                << dualPoint.toString() << std::endl;
    }

    unique_ptr<vector<DbScanPoint<DUAL_DIM_PLANE>>> results(
            new vector<DbScanPoint<DUAL_DIM_PLANE>>());
    results->push_back(dualPoint);
    return results;
}

double GpPlane::rasterPos(const double pos, const double cellExtent) {
    return cellExtent * std::floor(pos / cellExtent);
}

GeomPredicate GpPlane::getPredicateForShape(
        const PointBase<DUAL_DIM_PLANE>& shape,
        const ParamsAnalyzeGeom& params) const {

    // interpret the coords of the dual point: get the "point of reference" c
    // and the normal vector from c to the plane with length dd (see the
    // above explanation in projectTupleToDual)
    double cx = shape._coords[0];
    double cy = shape._coords[1];
    double cz = shape._coords[2];
    double ddnx = shape._coords[3];
    double ddny = shape._coords[4];
    double ddnz = shape._coords[5];
    double dd = std::sqrt(ddnx * ddnx + ddny * ddny + ddnz * ddnz);

    // equation for this plane:
    // 1) a*x + b*y + c*z = d  if (x,y,z) is relative to the origin, or
    // 2) a*x + b*y + c*z = dd if (x, y, z) is relative to c
    double a = ddnx / dd;
    double b = ddny / dd;
    double c = ddnz / dd;
    double d = dd + a * cx + b * cy + c * cz;
    double tolerance = params._matchTolerance;

    // the predicate uses a, b, c, d (so it is "universal" and indepentent
    // from the points of reference that were used for DBSCAN in dual space)
    GeomPredicate predicate =
            [a, b, c, d, tolerance](const DbScanPoint<DIMENSIONS>& point)
    {
        // get the distance of this point from the plane
        double dist = a * point._coords[0] + b * point._coords[1]
                    + c * point._coords[2] - d;
        return std::abs(dist) <= tolerance;
    };
    return predicate;
}

BboxPredicate GpPlane::getBboxPredicateForShape(
        const PointBase<DUAL_DIM_PLANE>& shape,
        const ParamsAnalyzeGeom& params) const {

    // interpret the coords of the dual point: get the "point of reference" c
    // and the normal vector from c to the plane with length dd (see the
    // above explanation in projectTupleToDual)
    double cx = shape._coords[0];
    double cy = shape._coords[1];
    double cz = shape._coords[2];
    double ddnx = shape._coords[3];
    double ddny = shape._coords[4];
    double ddnz = shape._coords[5];
    double dd = std::sqrt(ddnx * ddnx + ddny * ddny + ddnz * ddnz);

    // equation for this plane:
    // 1) a*x + b*y + c*z = d  if (x,y,z) is relative to the origin, or
    // 2) a*x + b*y + c*z = dd if (x, y, z) is relative to c
    double a = ddnx / dd;
    double b = ddny / dd;
    double c = ddnz / dd;
    double d = dd + a * cx + b * cy + c * cz;
    double tolerance = params._matchTolerance;

    // the predicate uses a, b, c, d (so it is "universal" and indepentent
    // from the points of reference that were used for DBSCAN in dual space)
    BboxPredicate predicate =
            [a, b, c, d, tolerance](const Rectangle2<DIMENSIONS>& bbox)
    {
        // determine whether all corners of the bbox are on the same side
        // of the plane, in which case there is no intersection, and false is
        // returned; otherwise, the bbox intersects with the plane.
        size_t combinations = (1 << DIMENSIONS);
        bool sign = false;
        for (size_t cornerBits = 0; cornerBits < combinations; ++cornerBits) {
            double x = (cornerBits & 1) ?
                    bbox.MinD(0) - tolerance : bbox.MaxD(0) + tolerance;
            double y = (cornerBits & 2) ?
                    bbox.MinD(1) - tolerance : bbox.MaxD(1) + tolerance;
            double z = (cornerBits & 4) ?
                    bbox.MinD(2) - tolerance : bbox.MaxD(2) + tolerance;
            double dist = a * x + b * y + c * z - d;
            if (cornerBits == 0)
                sign = std::signbit(dist);
            else if (sign != std::signbit(dist))
                return true;
        }
        return false;
    };
    return predicate;
}
