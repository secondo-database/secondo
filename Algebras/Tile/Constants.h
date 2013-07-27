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

#ifndef TILEALGEBRA_CONSTANTS_H
#define TILEALGEBRA_CONSTANTS_H

/*
system includes

*/

#include <string>

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constant contains the typename tgrid.

*/

extern const char* TYPE_NAME_TGRID;

/*
Constant contains the typename mtgrid.

*/

extern const char* TYPE_NAME_MTGRID;

/*
Constant contains the typename uniquestringarray.

*/

extern const char* TYPE_NAME_UNIQUESTRINGARRAY;

/*
Constant contains the typename tint.

*/

extern const char* TYPE_NAME_TINT;

/*
Constant contains the typename treal.

*/

extern const char* TYPE_NAME_TREAL;

/*
Constant contains the typename tbool.

*/

extern const char* TYPE_NAME_TBOOL;

/*
Constant contains the typename tstring.

*/

extern const char* TYPE_NAME_TSTRING;

/*
Constant contains the typename mtint.

*/

extern const char* TYPE_NAME_MTINT;

/*
Constant contains the typename mtreal.

*/

extern const char* TYPE_NAME_MTREAL;

/*
Constant contains the typename mtbool.

*/

extern const char* TYPE_NAME_MTBOOL;

/*
Constant contains the typename mtstring.

*/

extern const char* TYPE_NAME_MTSTRING;

/*
Constant contains the typename itint.

*/

extern const char* TYPE_NAME_ITINT;

/*
Constant contains the typename itreal.

*/

extern const char* TYPE_NAME_ITREAL;

/*
Constant contains the typename itbool.

*/

extern const char* TYPE_NAME_ITBOOL;

/*
Constant contains the typename itstring.

*/

extern const char* TYPE_NAME_ITSTRING;

/*
Constant contains the undefined value for datatype int.

*/

extern const int UNDEFINED_INT;

/*
Constant contains the undefined value for datatype real.

*/

extern const double UNDEFINED_REAL;

/*
Constant contains the undefined value for datatype bool.

*/

extern const char UNDEFINED_BOOL;

/*
Constant contains the undefined value for datatype string.

*/

extern const std::string UNDEFINED_STRING;

/*
Constant contains the undefined value for a string index.

*/

extern const int UNDEFINED_STRING_INDEX;

/*
Constant contains the dimension size of time dimension.

*/

extern const int TIME_DIMENSION_SIZE;

/*
Constant contains the size of a tintArray.

*/

extern const int TINTARRAY_SIZE;

/*
Constant contains the dimension size of a tintArray.

*/

extern const int TINTARRAY_DIMENSION_SIZE;

/*
Constant contains the number of elements of a tintFlob.

*/

extern const int TINTFLOB_ELEMENTS;

/*
Constant  contains the size of a tintFlob.

*/

extern const int TINTFLOB_SIZE;

/*
Constant contains the dimension size of a tintFlob.

*/

extern const int TINTFLOB_DIMENSION_SIZE;

}

#endif // TILEALGEBRA_CONSTANTS_H
