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



1 DbScanPoint

*/

#pragma once
#include <memory>
#include "PointBase.h"
#include "Rectangle2.h"

namespace pointcloud2 {

constexpr int SCAN_UNCLASSIFIED = -1;
constexpr int SCAN_NOISE = 0;

typedef unsigned PointIndex;

template<unsigned dim>
struct DbScanPoint { // TODO: von PointBase ableiten? Dann würden viele der
                     // Funktionen unten entfallen!
/*
1.1 Fields

*/
    /* the coordinates in each of the (dim) dimensions */
    double _coords[dim];

    /* the cluster to which this point belongs (i.e. the result of the DBSCAN
     * algorithm). Special values are SCAN_UNCLASSIFIED (only during DBSCAN)
     * and SCAN_NOISE. Positive values are cluster ids. */
    int _clusterId;

    /* _isLastInSeq.get() is true for the last point in a sequence of points.
     * If no sequences are available, this must be true for every point,
     * and every single point must be referenced by the MMRTree. */
    bool _isLastInSeq;

/*
1.2 Destructor

*/
    ~DbScanPoint() = default;

/*
1.3 Manipulators

*/
    void initialize(bool isLastInSeq = true);

    void clearCoords();

    void setCoords(const double coords[dim]);

    void copyCoordsFrom(const DbScanPoint<dim>& source);

/*
1.4 Getters

*/
    Rectangle<dim> getBoundingBox(double expand) const;

    Rectangle2<dim> getBoundingBox2(double expand) const;

    static std::string getStringForBBox(const Rectangle<dim>& bbox);

    double calculateDistanceSquare(const DbScanPoint<dim>& other) const;

    PointBase<dim> toPointBase() const { return PointBase<dim>(_coords); }

    std::string toString() const;
};

} // namespace
