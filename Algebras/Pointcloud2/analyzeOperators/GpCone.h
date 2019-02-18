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

*/
#pragma once
#include <memory>

#include "GeomPrimitive.h"

constexpr unsigned EQUATION_COUNT_CONE = 4;
constexpr unsigned CHALLENGE_COUNT_CONE = 1;
constexpr unsigned TUPLE_SIZE_CONE =
        EQUATION_COUNT_CONE + 1 + CHALLENGE_COUNT_CONE;
constexpr unsigned DUAL_DIM_CONE = 5;

namespace pointcloud2{
class GpCone: public GeomPrimitive<DUAL_DIM_CONE> {
    // the three SHIFT_ID values are used to represent cones parallel to
    // the z, x, y axis in dual space. The values should be "far" from each
    // other so cones of different orientations are not treated as
    // "neighbors" in DbScan
    static double SHIFT_ID[DIMENSIONS];

public:
    virtual ~GpCone() = default;

    virtual std::string getCaption(const bool plural) const final;

    virtual unsigned getTupleSize() const final;

    virtual std::unique_ptr<std::vector<DbScanPoint<DUAL_DIM_CONE>>>
            projectTupleToDual(
            const std::vector<SamplePoint>& tuple,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) final;

    virtual GeomPredicate getPredicateForShape(
            const PointBase<DUAL_DIM_CONE>& shape,
            const ParamsAnalyzeGeom& params) const final;

    virtual BboxPredicate getBboxPredicateForShape(
            const PointBase<DUAL_DIM_CONE>& shape,
            const ParamsAnalyzeGeom& params) const final;
private:
    inline static unsigned shiftDim(const unsigned dimension,
            const unsigned shift);

    static unsigned getShiftFromId(const double shiftId);

    static bool isPointFarOutsideShifted(
            const double x, const double y, const double z,
            const unsigned shift, const Rectangle<DIMENSIONS>& bbox);
};
}
