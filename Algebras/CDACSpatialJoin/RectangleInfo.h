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

\setcounter{tocdepth}{2}
\tableofcontents


1 RectangleInfo struct

Represents a 2-dimensional rectangle (i.e. a 2-dimensional bounding box or the
2-dimensional projection of a 3-dimensional bounding box) extracted from
spatial data. The y coordinates are stored as double values, while for the
x coordinates, only the relative position in the sort order given by
SortEdge::compare is needed (thereby saving 2 * 4 bytes per rectangle).

This struct is used by the JoinState constructor to temporarily store rectangle
information (yMin, yMax, address), then add information from the sort result
(left/rightEdgeIndex), and finally construct JoinEdge instances.

*/
#pragma once

#include "Base.h"

namespace cdacspatialjoin {

struct RectangleInfo {
   /* the rectangle's minimum y coordinate */
   double yMin;

   /* the rectangle's maximum y coordinate */
   double yMax;

   /* the index of the rectangle's left edge in the sort order given by
    * SortEdge (where left and right rectangle edges are sorted first by
    * their x coordinate, then by their isLeft value) */
   EdgeIndex_t leftEdgeIndex;

   /* the index of the rectangle's right edge in the sort order given by
    * SortEdge (where left and right rectangle edges are sorted first by
    * their x coordinate, then by their isLeft value) */
   EdgeIndex_t rightEdgeIndex;

   /* the "address" which this rectangle originates from; this value stores
    * a) the input set A or B; b) the index of a TBlock or RectangleBlock;
    * c) the row index inside the block. */
   SetRowBlock_t address;

   /* creates a new RectangleInfo instance with the given y coordinates and
    * address; the left/rightEdgeIndex values are yet to be determined (and
    * therefore set to 0) */
   RectangleInfo(const double yMin_, const double yMax_,
                 const SetRowBlock_t address_):
        yMin(yMin_), yMax(yMax_), leftEdgeIndex(0), rightEdgeIndex(0),
        address(address_) {
   }

   ~RectangleInfo() = default;
};

} // end of namespace cdacspatialjoin