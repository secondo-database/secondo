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



1 The DbScanPoint class

*/

#include "DbScanPoint.h"

using namespace pointcloud2;

template<unsigned dim>
void DbScanPoint<dim>::initialize(bool isLastInSeq /* = true */) {
    // no need to set _coords at this point
    _clusterId = SCAN_UNCLASSIFIED;
    _isLastInSeq = isLastInSeq;
}

template<unsigned dim>
void DbScanPoint<dim>::clearCoords() {
    for (size_t d = 0; d < dim; ++d) {
        _coords[d] = 0.0;
    }
}

template<unsigned dim>
void DbScanPoint<dim>::setCoords(const double coords[dim]) {
    for (size_t d = 0; d < dim; ++d) {
        _coords[d] = coords[d];
    }
}

template<unsigned dim>
std::string DbScanPoint<dim>::toString() const {
    std::stringstream sb;
    sb << "(";
    for (unsigned d = 0; d < dim; ++d) {
        if (d > 0)
            sb << ", ";
        sb << _coords[d];
    }
    sb << ") | ";
    if (_clusterId == SCAN_NOISE)
        sb << "Noise ";
    else if (_clusterId == SCAN_UNCLASSIFIED)
        sb << "Cluster UNCLASSIFIED";
    else
        sb << "Cluster " << _clusterId;
    return sb.str();
}

template<unsigned dim>
Rectangle<dim> DbScanPoint<dim>::getBoundingBox(double expand) const {
    double minMax[2 * dim];
    for (unsigned d = 0; d < dim; ++d) {
        minMax[2 * d] = _coords[d] - expand;
        minMax[2 * d + 1] = _coords[d] + expand;
    }
    return Rectangle<dim>(true, minMax);
}

template<unsigned dim>
Rectangle2<dim> DbScanPoint<dim>::getBoundingBox2(double expand) const {
    double minMax[2 * dim];
    for (unsigned d = 0; d < dim; ++d) {
        minMax[2 * d] = _coords[d] - expand;
        minMax[2 * d + 1] = _coords[d] + expand;
    }
    return Rectangle2<dim>(minMax);
}

template<unsigned dim>
double DbScanPoint<dim>::calculateDistanceSquare(
        const DbScanPoint<dim>& other) const {
    // TODO: Distanzberechnung vom Referenzsystem abhängig machen?
    double result = 0.0;
    for (unsigned d = 0; d < dim; ++d) {
        double diff = _coords[d] - other._coords[d];
        result += diff * diff;
    }
    return result;
}

template<unsigned dim>
void DbScanPoint<dim>::copyCoordsFrom(const DbScanPoint<dim>& source) {
    for (unsigned d = 0; d < dim; ++d)
        _coords[d] = source._coords[d];
}

template<unsigned dim>
std::string DbScanPoint<dim>::getStringForBBox(const Rectangle<dim>& bbox) {
    std::stringstream sb;
    for (unsigned d = 0; d < dim; ++d) {
        if (d > 0)
            sb << ", ";
        sb << bbox.MinD(d) << " - " << bbox.MaxD(d);
    }
    sb << ")" << endl;
    return sb.str();
}

// ensure defined references for the dimensions needed
namespace pointcloud2 {
template struct DbScanPoint<3>;
template struct DbScanPoint<4>;
template struct DbScanPoint<5>;
template struct DbScanPoint<6>;
}


