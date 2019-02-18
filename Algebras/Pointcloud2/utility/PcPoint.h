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



1 The PcPoint class

*/
#pragma once
#include <memory>
#include <string>

#include "PointBase.h"

namespace pointcloud2 {

/*
0.2 PcPoint Struct

Represents a single Point in the cloud.

*/
struct PcPoint { // TODO: von PointBase ableiten? Oder führt das zu
                 // Effizienzverlusten?
    double _x;
    double _y;
    double _z;
    TupleId _tupleId;

    PcPoint() :
        _x(0.0), _y(0.0), _z(0.0), _tupleId(0) {
    }

    PcPoint(double x, double y, double z, TupleId tupleId = 0) :
        _x(x), _y(y), _z(z), _tupleId(tupleId) {
    }

    ~PcPoint() = default;

    std::string toString() const {
        return "(" + std::to_string(_x) + ", " + std::to_string(_y) + ", "
                + std::to_string(_z) + ") | " + std::to_string(_tupleId);
    }

    static bool checkType(const ListExpr list) {
        return nl->HasLength(list, 3)
                && CcReal::checkType(nl->First(list))
                && CcReal::checkType(nl->Second(list))
                && CcReal::checkType(nl->Third(list));
    }

    inline double getCoord(size_t d) const {
        assert (d < DIMENSIONS);
        if (d == 0)
            return _x;
        return (d == 1) ? _y : _z;
    }

    Rectangle<DIMENSIONS> getBoundingBox() const {
        return getBoundingBox(BOX_EXPAND);
    }

    Rectangle<DIMENSIONS> getBoundingBox(double expand) const {
        double minMax[] = { _x - expand, _x + expand, _y - expand, _y + expand,
                _z - expand, _z + expand };
        return Rectangle<DIMENSIONS>(true, minMax);
    }

    PointBase<DIMENSIONS> toPointBase() const {
        return PointBase<DIMENSIONS>(_x, _y, _z);
    }
};
} // end of namespace
