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
#pragma once
#include "GeomPrimitive.h"

constexpr unsigned EQUATION_COUNT_PLANE = 3;
constexpr unsigned CHALLENGE_COUNT_PLANE = 1;
constexpr unsigned TUPLE_SIZE_PLANE =
        EQUATION_COUNT_PLANE + CHALLENGE_COUNT_PLANE;
constexpr unsigned DUAL_DIM_PLANE = 6;

namespace pointcloud2{

class GpPlane : public GeomPrimitive<DUAL_DIM_PLANE> {
public:
    virtual ~GpPlane() = default;

    virtual std::string getCaption(const bool plural) const final;

    virtual unsigned getTupleSize() const final;

    virtual std::unique_ptr<std::vector<DbScanPoint<DUAL_DIM_PLANE>>>
            projectTupleToDual(
            const std::vector<SamplePoint>& tuple,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) final;

    virtual GeomPredicate getPredicateForShape(
            const PointBase<DUAL_DIM_PLANE>& shape,
            const ParamsAnalyzeGeom& params) const final;

    virtual BboxPredicate getBboxPredicateForShape(
            const PointBase<DUAL_DIM_PLANE>& shape,
            const ParamsAnalyzeGeom& params) const final;

private:
    /* returns the position of a "raster point" close to the given pos,
     * with cellExtent being the raster width */
    static double rasterPos(const double pos, const double cellExtent);
};
}
