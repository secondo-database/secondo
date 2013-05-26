 
/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

*/

#include <cmath>
#include <limits>
#include "WinUnix.h"
#include "Constants.h"
#include "Grid2.h"

namespace TileAlgebra
{

/*
const undefined value for type int

*/

const int UNDEFINED_INT = std::numeric_limits<int>::min();

/*
const Grid2 size

*/

const unsigned int GRID2_SIZE = sizeof(Grid2);

/*
const tintArray size

*/

const int TINTARRAY_SIZE = static_cast<unsigned int>
                           (std::pow
                           (
                            static_cast<unsigned int>
                           (std::sqrt((WinUnix::getPageSize() -
                            GRID2_SIZE) / sizeof(int))), 2
                           )
                           );

/*
const tintArray dimension size

*/

const int TINTARRAY_DIMENSION_SIZE = static_cast<unsigned int>
                                     (std::sqrt(TINTARRAY_SIZE));

/*
const tintFlob elements

*/

const int TINTFLOB_ELEMENTS = static_cast<unsigned int>
                              (std::pow
                              (
                               static_cast<unsigned int>
                              (std::sqrt((WinUnix::getPageSize() -
                               GRID2_SIZE) / sizeof(int))), 2
                              )
                              );

/*
const tintFlob size

*/

const int TINTFLOB_SIZE = TINTFLOB_ELEMENTS * sizeof(int);

/*
const tintFlob dimension size

*/

const int TINTFLOB_DIMENSION_SIZE = static_cast<unsigned int>
                                    (std::sqrt(TINTFLOB_ELEMENTS));

/*
const type name for type tint

*/

const char* TYPE_NAME_TINT = "tint";

}
