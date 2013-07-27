 
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

/*
system includes

*/

#include <cmath>
#include <limits>
#include "WinUnix.h"

/*
TileAlgebra includes

*/

#include "Constants.h"
#include "grid/tgrid.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constant contains the typename tgrid.

*/

const char* TYPE_NAME_TGRID = "tgrid";

/*
Constant contains the typename mtgrid.

*/

const char* TYPE_NAME_MTGRID = "mtgrid";

/*
Constant contains the typename uniquestringarray.

*/

const char* TYPE_NAME_UNIQUESTRINGARRAY = "uniquestringarray";

/*
Constant contains the typename tint.

*/

const char* TYPE_NAME_TINT = "tint";

/*
Constant contains the typename treal.

*/

const char* TYPE_NAME_TREAL = "treal";

/*
Constant contains the typename tbool.

*/

const char* TYPE_NAME_TBOOL = "tbool";

/*
Constant contains the typename tstring.

*/

const char* TYPE_NAME_TSTRING = "tstring";

/*
Constant contains the typename mtint.

*/

const char* TYPE_NAME_MTINT = "mtint";

/*
Constant contains the typename mtreal.

*/

const char* TYPE_NAME_MTREAL = "mtreal";

/*
Constant contains the typename mtbool.

*/

const char* TYPE_NAME_MTBOOL = "mtbool";

/*
Constant contains the typename mtstring.

*/

const char* TYPE_NAME_MTSTRING = "mtstring";

/*
Constant contains the typename itint.

*/

const char* TYPE_NAME_ITINT = "itint";

/*
Constant contains the typename itreal.

*/

const char* TYPE_NAME_ITREAL = "itreal";

/*
Constant contains the typename itbool.

*/

const char* TYPE_NAME_ITBOOL = "itbool";

/*
Constant contains the typename itstring.

*/

const char* TYPE_NAME_ITSTRING = "itstring";

/*
Constant contains the undefined value for datatype int.

*/

const int UNDEFINED_INT = std::numeric_limits<int>::min();

/*
Constant contains the undefined value for datatype real.

*/

const double UNDEFINED_REAL = std::numeric_limits<double>::quiet_NaN();

/*
Constant contains the undefined value for datatype bool.

*/

const char UNDEFINED_BOOL = -1;

/*
Constant contains the undefined value for datatype string.

*/

const std::string UNDEFINED_STRING = "";

/*
Constant contains the undefined value for a string index.

*/

const int UNDEFINED_STRING_INDEX = -1;

/*
Constant contains the dimension size of time dimension.

*/

const int TIME_DIMENSION_SIZE = 10;

/*
Constant contains the size of a tintArray.

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
Constant contains the dimension size of a tintArray.

*/

const int TINTARRAY_DIMENSION_SIZE = static_cast<unsigned int>
                                     (std::pow(TINTARRAY_SIZE, 0.5));

/*
Constant contains the number of elements of a tintFlob.

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
Constant contains the size of a tintFlob.

*/

const int TINTFLOB_SIZE = TINTFLOB_ELEMENTS * sizeof(int);

/*
Constant contains the dimension size of a tintFlob.

*/

const int TINTFLOB_DIMENSION_SIZE = static_cast<unsigned int>
                                    (std::pow(TINTFLOB_ELEMENTS, 0.5));

}
