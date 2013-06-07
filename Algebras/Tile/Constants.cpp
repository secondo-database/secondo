 
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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
#include "grid/tgrid.h"

namespace TileAlgebra
{

/*
const type name for type tgrid

*/

const char* TYPE_NAME_TGRID = "tgrid";

/*
const type name for type mtgrid

*/

const char* TYPE_NAME_MTGRID = "mtgrid";

/*
const type name for type uniquestringarray

*/

const char* TYPE_NAME_UNIQUESTRINGARRAY = "uniquestringarray";

/*
const type name for type tint

*/

const char* TYPE_NAME_TINT = "tint";

/*
const type name for type treal

*/

const char* TYPE_NAME_TREAL = "treal";

/*
const type name for type tbool

*/

const char* TYPE_NAME_TBOOL = "tbool";

/*
const type name for type tstring

*/

const char* TYPE_NAME_TSTRING = "tstring";

/*
const type name for type mtint

*/

const char* TYPE_NAME_MTINT = "mtint";

/*
const type name for type mtreal

*/

const char* TYPE_NAME_MTREAL = "mtreal";

/*
const type name for type mtbool

*/

const char* TYPE_NAME_MTBOOL = "mtbool";

/*
const type name for type mtstring

*/

const char* TYPE_NAME_MTSTRING = "mtstring";

/*
const type name for type itint

*/

const char* TYPE_NAME_ITINT = "itint";

/*
const type name for type itreal

*/

const char* TYPE_NAME_ITREAL = "itreal";

/*
const type name for type itbool

*/

const char* TYPE_NAME_ITBOOL = "itbool";

/*
const type name for type itstring

*/

const char* TYPE_NAME_ITSTRING = "itstring";

/*
const undefined value for type int

*/

const int UNDEFINED_INT = std::numeric_limits<int>::min();

/*
const undefined value for type real

*/

const int UNDEFINED_REAL = std::numeric_limits<double>::quiet_NaN();

/*
const undefined value for type bool

*/

const int UNDEFINED_BOOL = -1;

/*
const undefined value for type string

*/

const std::string UNDEFINED_STRING = "";

/*
const undefined string index value

*/

const int UNDEFINED_STRING_INDEX = -1;

/*
const tintArray size

*/

const int TINTARRAY_SIZE = static_cast<unsigned int>
                           (std::pow
                             (static_cast<unsigned int>
                             (std::pow((WinUnix::getPageSize() -
                                        sizeof(tgrid)) /
                                        sizeof(int),
                                        0.5)),
                             2)
                           );

/*
const tintArray dimension size

*/

const int TINTARRAY_DIMENSION_SIZE = static_cast<unsigned int>
                                     (std::pow(TINTARRAY_SIZE, 0.5));

/*
const tintFlob elements

*/

const int TINTFLOB_ELEMENTS = static_cast<unsigned int>
                              (std::pow
                                (static_cast<unsigned int>
                                (std::pow((WinUnix::getPageSize() -
                                           sizeof(tgrid)) /
                                           sizeof(int),
                                           0.5)),
                                2)
                              );

/*
const tintFlob size

*/

const int TINTFLOB_SIZE = TINTFLOB_ELEMENTS * sizeof(int);

/*
const tintFlob dimension size

*/

const int TINTFLOB_DIMENSION_SIZE = static_cast<unsigned int>
                                    (std::pow(TINTFLOB_ELEMENTS, 0.5));

}
