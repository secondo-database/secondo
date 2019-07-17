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


1 Typedefs and constants for the CDACSpatialJoin(Count) operators

To keep includes as limited as possible, this header contains common defines,
typedefs and constants used in the context of the CDACSpatialJoin(Count)
operators.

For activating the Performance API (PAPI), see Timer.h.

*/

#pragma once

// #define CDAC_SPATIAL_JOIN_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_DETAILED_REPORT_TO_CONSOLE
// #define CDAC_SPATIAL_JOIN_METRICS

#include <memory>
#include <vector>

namespace cdacspatialjoin {

/*
an enumeration for the rectangle sets to be joined

*/
enum SET {
   A = 0,
   B = 1
};

/*
the number of rectangle sets to be joined (always 2, but to be used for
semantic clarity)

*/
static constexpr unsigned SET_COUNT = 2;

/*
the integer type used for indices in vectors of SortEdges or JoinEdges

*/
typedef uint32_t EdgeIndex_t;

/*
the integer type used for indices of the TBlocks or RectangleBlocks of an
input stream

*/
typedef uint32_t BlockIndex_t;

/*
the integer type used for indices of rows inside a TBlock

*/
typedef uint32_t RowIndex_t;

/*
the integer type used to store the full 'address' of a rectangle: the
set (i. e. input stream A or B), the index of the TBlock (or RectangleBlock),
and the row inside the block. Since each rectangle requires at least
32 bytes of memory and only 512 MiB are provided per operator, this type
must be able to address a maximum of 16 million rectangles, which easily
fits into a uint32\_t (the number of bits used for blocks and rows is flexible
to adapt to very large or very many blocks)

*/
typedef uint32_t SetRowBlock_t;

/* the bit mask for the "set" information (SET::A / SET::B) which is
* always stored in the highest bit of a SetRowBlock_t value */
static constexpr SetRowBlock_t SET_MASK = 0x80000000;
}