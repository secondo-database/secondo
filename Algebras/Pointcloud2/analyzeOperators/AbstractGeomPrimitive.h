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



1 Interface for Geometric Primitives

This interface declares the functions needed for the analyzeGeom operator
for a primitive type.

*/
#pragma once
#include <vector>
#include <memory>

#include "ParamsAnalyzeGeom.h"
#include "ObjectIdManager.h"
#include "../utility/Timer.h"
#include "../utility/MMRTree2.h"
#include "../utility/PcPoint.h"
#include "../utility/MMRCloud.h"

namespace pointcloud2{

/*
Represents a point from the sample which is taken from the Pointcloud2
for each primitive type.

*/
typedef PointBase<DIMENSIONS> SamplePoint;

class AbstractGeomPrimitive {
public:

    virtual ~AbstractGeomPrimitive() = default;

    virtual std::string getCaption(const bool plural) const = 0;

/*
1.2  Implemented by the Abstract Template GeomPrimitive

1.2.1 projectClusterSetObjID

Geometric analysis is at its core performed by this method.
We do three things in sequence:
First, we project a sample of points to a dual space in which we can represent
the Geometric Primitive we are looking for as a point.
Second, we now might have a lot of scattered points in dual space, since
objects won't be perfectly regular, so we cluster those and reduce them to a
single center, representing a single Shape.
Third, for each such shape the corresponding part of our Cloud is assigned
ObjectIDs.

Also, for each assigned point, a bit in the bitMask of our MMRCloud is set,
to prevent reassignment of this point.

*/
    virtual void projectClusterSetObjID(const std::vector<SamplePoint>& sample,
            std::shared_ptr<MMRCloud> cloud,
            const ParamsAnalyzeGeom& params,
            std::shared_ptr<ObjectIdManager> objManager,
            Timer& timer) = 0;

};
}
