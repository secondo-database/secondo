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



1 Sphere

*/
#include "GpSphere.h"

#include "../utility/MathUtils.h"

using namespace pointcloud2;
using namespace std;

std::string GpSphere::getCaption(bool plural) const {
    return plural ? "spheres" : "sphere";
}

unsigned GpSphere::getTupleSize() const {
    return TUPLE_SIZE_SPHERE;
}

unique_ptr<vector<DbScanPoint<DUAL_DIM_SPHERE>>>
GpSphere::projectTupleToDual(const vector<SamplePoint>& tuple,
        const ParamsAnalyzeGeom& params,
        const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) {
    assert (tuple.size() == TUPLE_SIZE_SPHERE);

    std::vector<std::array<double, EQUATION_COUNT_SPHERE + 1>> eqs;
    for (unsigned i = 0; i < EQUATION_COUNT_SPHERE; ++i) {
        const SamplePoint& point = tuple[i];
        double x = point._coords[0];
        double y = point._coords[1];
        double z = point._coords[2];
        double res = -(x * x + y * y + z * z);
        array<double, EQUATION_COUNT_SPHERE + 1> eq { 1.0, x, y, z, res };
        eqs.push_back(std::move(eq));
    }

    std::vector<double> result =
            solveLinearSystem<EQUATION_COUNT_SPHERE>(eqs);

    if (result.size() == 0)
        return nullptr; // linear system cannot be solved if all four points
                      // are on the same plane

    // calculate center and radius
    double cx = -result[1] / 2.0;
    double cy = -result[2] / 2.0;
    double cz = -result[3] / 2.0;
    if (isPointFarOutside(cx, cy, cz, bboxOfEntrieCloud)) {
        // the four points are almost on the same plane, so a huge sphere was
        // calculated from them the center of which is far outside the bbox
        // of the sample points
        return nullptr;
    }
    double radius = std::sqrt(cx * cx + cy * cy + cz * cz - result[0]);

    // we challenge this sphere with one or several additional point from
    // the same neighborhood in order to avoid noise in dual space
    for (unsigned i = 0; i < CHALLENGE_COUNT_SPHERE ; ++i) {
        const SamplePoint& point = tuple[EQUATION_COUNT_SPHERE + i];
        // the test is the same as in getPredicateForShape:
        double dx = point._coords[0] - cx;
        double dy = point._coords[1] - cy;
        double dz = point._coords[2] - cz;
        double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (std::abs(radius - dist) > params._matchTolerance)
            return nullptr; // challenge failed
    }

    // create the dual point
    DbScanPoint<DUAL_DIM_SPHERE> dualPoint;
    dualPoint.initialize();
    dualPoint._coords[0] = cx;
    dualPoint._coords[1] = cy;
    dualPoint._coords[2] = cz;
    dualPoint._coords[3] = radius;

    if (REPORT_DETAILED) { // do this before std::move is used
        std::cout << "+ dual point for " << getCaption(false) << ": "
        << dualPoint.toString() << std::endl;
    }

    unique_ptr<vector<DbScanPoint<DUAL_DIM_SPHERE>>> results(
            new vector<DbScanPoint<DUAL_DIM_SPHERE>>());
    results->push_back(dualPoint);
    return results;
}

GeomPredicate GpSphere::getPredicateForShape (
        const PointBase<DUAL_DIM_SPHERE>& shape,
        const ParamsAnalyzeGeom& params) const {
    // rather than calculating std::sqrt() each time, our predicate will
    // calculate the square of the distance of the given point to
    // the sphere's center and compare it to a tolerated range

    // interpret the coords of the dual point
    double centerX = shape._coords[0];
    double centerY = shape._coords[1];
    double centerZ = shape._coords[2];
    double radius = shape._coords[3];
    // the (square) radius range in which points will be considered
    // as belonging to the sphere
    double radiusMin = radius - params._matchTolerance;
    double radiusMax = radius + params._matchTolerance;
    double sqRadiusMin = radiusMin * radiusMin;
    double sqRadiusMax = radiusMax * radiusMax;

    GeomPredicate predicate =
    [centerX, centerY, centerZ, sqRadiusMin, sqRadiusMax]
            (const DbScanPoint<DIMENSIONS>& point)
    {
       double dx = point._coords[0] - centerX;
       double dy = point._coords[1] - centerY;
       double dz = point._coords[2] - centerZ;
       double sqDist = dx * dx + dy * dy + dz * dz;
       return (sqDist >= sqRadiusMin) && (sqDist <= sqRadiusMax);
    };
    return predicate;
}

BboxPredicate GpSphere::getBboxPredicateForShape(
        const PointBase<DUAL_DIM_SPHERE>& shape,
        const ParamsAnalyzeGeom& params) const {
    // interpret the coords of the dual point
    double radius = shape._coords[3] + params._matchTolerance;
    double minMax[6];
    for (unsigned d = 0; d < DIMENSIONS; ++d) {
        minMax[2 * d] = shape._coords[d] - radius;
        minMax[2 * d + 1] = shape._coords[d] + radius;
    }
    Rectangle2<DIMENSIONS> sphereBbox = Rectangle2<DIMENSIONS>(minMax);

    BboxPredicate predicate =
            [sphereBbox](const Rectangle2<DIMENSIONS>& bbox) {
        // this predicate only checks whether the sphere's bounding box
        // intersects with the given bounding box; in some cases, it will
        // return true although there is no actual intersection with the
        // sphere; however, the points contained in bbox will be checked
        // individually anyway.
        return sphereBbox.Intersects(bbox);
    };
    return predicate;
}
