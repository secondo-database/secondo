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



0 Header for the Geometric Primitives
Most top-level documentation can be found here.

We are using a threefold structure:

  * the Interface GeomPrimitiveInterface,
    which is used for the iteration over all registered primitives
    in the analyze procedure (Pointcloud2::analyzeGeom);

  * the abstract template GeomPrimitive which implements
    all functionality common across all primitives;

  * the implementations of the primitives and their specific
    functions.

Extensions of the latter are fairly easy as
each primitive must implement the following functions:

  * get-caption

  * get-tuple-size

  * project-tuple-to-dual

  * get-predicate-for-shape

  * get-bbox-predicate-for-shape

*/
#pragma once
#include <memory>
#include <vector>
#include <functional>

#include "AbstractGeomPrimitive.h"


namespace pointcloud2 {


/*
Returns true if the given point is considered part of the specific shape
from which this predicate was created.

*/
typedef std::function<bool(const DbScanPoint<DIMENSIONS>&)> GeomPredicate;

/*
Returns true if the given bounding box possibly intersects with the
specific shape from which this predicate was created. In the case of an
intersection, the points in the bounding box will be individually tested
with the GeomPredicate.

*/
typedef std::function<bool(const Rectangle2<DIMENSIONS>&)> BboxPredicate;


/*
2 Abstract Template Class GeomPrimitive

*/
template<unsigned dual>
class GeomPrimitive : public AbstractGeomPrimitive {
protected:
    /* whether the dual points should be reported to the console */
    static const bool REPORT_TO_CONSOLE = true;
    static const bool REPORT_DETAILED = REPORT_TO_CONSOLE && false;

public:
    virtual ~GeomPrimitive() = default;

/*
2.1 Functions implemented by this class.

2.1.1 project\_cluster\_setObjID

For documentation see the project\_cluster\_setObjID method
in GeomPrimitiveInterface.

*/
    void projectClusterSetObjID(const std::vector<SamplePoint>& sample,
            std::shared_ptr<MMRCloud> cloud,
            const ParamsAnalyzeGeom& params,
            std::shared_ptr<ObjectIdManager> objManager,
            Timer& timer) override;

private:

/*
2.1.2 project sample to dual

This function takes a sample of regular points from the Pointcloud2 and
returns a vector of points in dual space.

*/
    std::shared_ptr<std::vector<DbScanPoint<dual>>> projectSampleToDual(
            const std::vector<SamplePoint>& sample,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntireCloud);

/*
2.1.3 project neighborhood to dual

Adds dual points from the given neighborhood to the vector dualPoints.
neighbors contains indices of points in the sample; centerPoint is the
point around which this neighborhood was retrieved.

*/
    std::vector<DbScanPoint<dual>> projectNeighborhoodToDual(
            const std::vector<PointIndex>& neighbors,
            const PointIndex centerPoint,
            const BitArray& neighborIndexSelector,
            const std::vector<SamplePoint>& sample,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntireCloud);

/*
2.1.4 cluster and reduce to centers

This function is mainly a templating-wrapper of the DbScan interface.
It uses recursive DBSCAN on the given dualPoints to obtain for each cluster
a single point in dual space. The recursion ensures that the bounding box
of the clusters gets small enough and then returns the weighted center of
every cluster that contains enough points.

A vector of those centers is returned, each representing a geometric shape
in normal space.

*/
    std::vector<PointBase<dual>> clusterAndReduceToCenters(
            std::shared_ptr<std::vector<DbScanPoint<dual>>> dualPoints,
            const ParamsAnalyzeGeom& params);

/*
2.1.5 match shapes with points

This function iterates over the given shapes and uses the given mmRTree to
identify all candidate points in the Pointcloud2 that may belong to the
respective shape. It then calls DBSCAN to identify clusters among the
candidate points. For all points in large enough clusters, the \_clusterId
is set to a new object id.

*/
    void matchShapesWithPoints(
            const std::vector<PointBase<dual>>& shapes,
            std::shared_ptr<MMRCloud> cloud,
            const ParamsAnalyzeGeom& params,
            std::shared_ptr<ObjectIdManager> iih) const;

protected:
/*
2.1.5 is point far outside

Returns true if the given point is far outside the given bbox. This helps
identify unlikely shape candidates (e.g. huge spheres the center of which
lies far outside the bbox; this can occur when a sphere is
calculated from points that are almost on a plane).

*/
    static bool isPointFarOutside(const double x, const double y,
            const double z, const Rectangle<DIMENSIONS>& bbox);

public:
/*
2.2 Functions that must be implemented by Geometric Primitives.

2.2.1 Name of this primitive

Returns the name of this primitive used for console outputs.

*/
    virtual std::string getCaption(const bool plural) const override = 0;

/*
2.2.2 get tuple size

Returns the number of points needed in a tuple for project\_tuple\_to\_dual
(e.g., three points are needed to define a plane in the 3-dimensional space).
Note that the tuple size is often, but not necessarily equal to the dimension
of the dual space.

*/
    virtual unsigned getTupleSize() const = 0;

/*
2.2.3 project tuple into dual space

This function must be implemented by each primitive. The given vector
"tuple" holds the (indexes of the) required number of points from the
given "sample". It is then assumed that these points define a primitive
of the current type, and the coordinates in dual space which represent
that specific primitive are calculated. This dual point is then entered
to the vector \_dualPoints.

The function returns true if a dual point could be added to the given vector,
or false if no dual point could be calculated from the given tuple.

*/
    virtual std::unique_ptr<std::vector<DbScanPoint<dual>>> projectTupleToDual(
            const std::vector<SamplePoint>& tuple,
            const ParamsAnalyzeGeom& params,
            const Rectangle<DIMENSIONS>& bboxOfEntireCloud) = 0;

/*
2.2.4 Calculation of a Predicate for a given Shape

This function must be implemented by each Primitive.
It returns a function that tests whether a Point is contained in the shape
which is represented by the argument dual point. The returned function will
be used to test whether specific points from the Pointcloud2 can be assigned
to the shape.

*/
    virtual GeomPredicate getPredicateForShape(
                const PointBase<dual>& shape,
                const ParamsAnalyzeGeom& params) const = 0;

/*
2.2.5 Calculation of bbox Predicate for a given Shape

This function must be implemented by each Primitive.
It returns a function that tests whether a bounding box may intersect with
the shape which is represented by the argument dual point. If the function
evaluates to true, all points in the bounding box will be tested individually
using the predicate created by get\_predicate\_for\_shape.

*/
    virtual BboxPredicate getBboxPredicateForShape(
                    const PointBase<dual>& shape,
                    const ParamsAnalyzeGeom& params) const = 0;
};

} //end of namespace
