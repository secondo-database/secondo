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



1 PointBase

Represents an dim-dimensional point with no additional information.

*/

#pragma once
#include <string>
#include <iostream>
#include <sstream>

#include "Rectangle2.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

namespace pointcloud2 {

static const double BOX_EXPAND = FACTOR;
constexpr unsigned int DIMENSIONS = 3;
constexpr unsigned MMRTREE_NODE_SIZE_MAX = 8;

template<unsigned dim>
struct PointBase {
    double _coords[dim];

    PointBase() {
        clear();
    }

    /* this constructor is for convenience in case dim == 3 */
    PointBase(const double coords[]) {
        setCoords(coords);
    }

    /* this constructor is for convenience in case dim == 3 */
    PointBase(const double x, const double y, const double z) {
        // implementation is in header file to avoid "undefined reference"
        // linker errors caused by the lack of particular <dim> instantiations
        assert (dim == 3);
        _coords[0] = x;
        _coords[1] = y;
        _coords[2] = z;
    }

    virtual ~PointBase() = default;

    void clear() {
        for (size_t d = 0; d < dim; ++d)
            _coords[d] = 0.0;
    }

    void setCoords(const double coords[dim]) {
        for (size_t d = 0; d < dim; ++d)
            _coords[d] = coords[d];
    }

    inline double getX() { return _coords[0]; }
    inline double getY() { return _coords[1]; }
    inline double getZ() { return _coords[2]; }

    void copyCoordsFrom(const PointBase<dim>& source) {
        for (unsigned d = 0; d < dim; ++d)
            _coords[d] = source._coords[d];
    }

    double calculateDistanceSquare(const PointBase<dim>& other) const {
        // TODO: Distanzberechnung vom Referenzsystem abhängig machen?
        double result;
        for (unsigned d = 0; d < dim; ++d) {
            double diff = _coords[d] - other._coords[d];
            result += diff * diff;
        }
        return result;
    }

    bool operator < (const PointBase& other) const {
        for (int d = dim - 1; d >= 0; --d) {
            if (_coords[d] < other._coords[d])
                return true;
            else if (_coords[d] > other._coords[d])
                return false;
        }
        return false; // other point is equal, i.e. not less
    }

    std::string toString() const {
        std::stringstream st;
        st << "(";
        for (size_t d = 0; d < dim; ++d) {
            if (d > 0)
                st << ", ";
            st << _coords[d];
        }
        st << ")";
        return st.str();
    }

    Rectangle2<dim> getBoundingBox2() const {
        return getBoundingBox2(BOX_EXPAND);
    }

    Rectangle2<dim> getBoundingBox2(double expand) const {
        double minMax[2 * dim];
        for (unsigned d = 0; d < dim; ++d) {
            minMax[2 * d] = _coords[d] - expand;
            minMax[2 * d + 1] = _coords[d] + expand;
        }
        return Rectangle2<dim>(minMax);
    }

    bool isInsideBbox(Rectangle2<dim>& bbox) {
        for (unsigned i = 0; i < dim; ++i) {
            if (_coords[i] < bbox.MinD(i) || _coords[i] > bbox.MaxD(i))
                return false;
        }
        return true;
    }

    /* returns true if this point is closer to the center of the given bbox
     * than to the border of that bbox */
    bool isCloseToCenter(const Rectangle2<dim>& bbox) const {
        const double EDGE_WIDTH = 0.25;

        for (unsigned d = 0; d < dim; ++d) {
            double bboxWidth = bbox.MaxD(d) - bbox.MinD(d);
            double requiredDist = bboxWidth * EDGE_WIDTH;
            double coord = _coords[d];
            if (coord - bbox.MinD(d) < requiredDist)
                return false;
            if (bbox.MaxD(d) - coord < requiredDist)
                return false;
        }
        return true;
    }
};


} /* namespace pointcloud2 */

