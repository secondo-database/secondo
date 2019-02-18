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
#pragma once
#include <memory>

#include "GeomPrimitive.h"

constexpr unsigned EQUATION_COUNT_SPHERE = 4;
constexpr unsigned CHALLENGE_COUNT_SPHERE = 1;
constexpr unsigned TUPLE_SIZE_SPHERE =
        EQUATION_COUNT_SPHERE + CHALLENGE_COUNT_SPHERE;
constexpr unsigned DUAL_DIM_SPHERE = 4;


namespace pointcloud2{

class GpSphere: public GeomPrimitive<DUAL_DIM_SPHERE> {
public:
    virtual ~GpSphere() = default;

    virtual std::string getCaption(const bool plural) const final;

    virtual unsigned getTupleSize() const final;

    virtual std::unique_ptr<std::vector<DbScanPoint<DUAL_DIM_SPHERE>>>
            projectTupleToDual(
            const std::vector<SamplePoint>& tuple,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntrieCloud) final;

    virtual GeomPredicate getPredicateForShape(
            const PointBase<DUAL_DIM_SPHERE>& shape,
            const ParamsAnalyzeGeom& params) const final;

    virtual BboxPredicate getBboxPredicateForShape(
            const PointBase<DUAL_DIM_SPHERE>& shape,
            const ParamsAnalyzeGeom& params) const final;
};
}
