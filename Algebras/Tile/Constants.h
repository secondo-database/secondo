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

#include <string>

namespace TileAlgebra
{

/*
const type name for type tgrid

*/

extern const char* TYPE_NAME_TGRID;

/*
const type name for type mtgrid

*/

extern const char* TYPE_NAME_MTGRID;

/*
const type name for type uniquestringarray

*/

extern const char* TYPE_NAME_UNIQUESTRINGARRAY;

/*
const type name for type tint

*/

extern const char* TYPE_NAME_TINT;

/*
const type name for type treal

*/

extern const char* TYPE_NAME_TREAL;

/*
const type name for type tbool

*/

extern const char* TYPE_NAME_TBOOL;

/*
const type name for type tstring

*/

extern const char* TYPE_NAME_TSTRING;

/*
const type name for type mtint

*/

extern const char* TYPE_NAME_MTINT;

/*
const type name for type mtreal

*/

extern const char* TYPE_NAME_MTREAL;

/*
const type name for type mtbool

*/

extern const char* TYPE_NAME_MTBOOL;

/*
const type name for type mtstring

*/

extern const char* TYPE_NAME_MTSTRING;

/*
const type name for type itint

*/

extern const char* TYPE_NAME_ITINT;

/*
const type name for type itreal

*/

extern const char* TYPE_NAME_ITREAL;

/*
const type name for type itbool

*/

extern const char* TYPE_NAME_ITBOOL;

/*
const type name for type itstring

*/

extern const char* TYPE_NAME_ITSTRING;

/*
const undefined value for type int

*/

extern const int UNDEFINED_INT;

/*
const undefined value for type real

*/

extern const int UNDEFINED_REAL;

/*
const undefined value for type bool

*/

extern const int UNDEFINED_BOOL;

/*
const undefined value for type string

*/

extern const std::string UNDEFINED_STRING;

/*
const undefined string index value

*/

extern const int UNDEFINED_STRING_INDEX;

/*
const time dimension size

*/

extern const int TIME_DIMENSION_SIZE;

/*
const tintArray size

*/

extern const int TINTARRAY_SIZE;

/*
const tintArray dimension size

*/

extern const int TINTARRAY_DIMENSION_SIZE;

/*
const tintFlob elements

*/

extern const int TINTFLOB_ELEMENTS;

/*
const tintFlob size

*/

extern const int TINTFLOB_SIZE;

/*
const tintFlob dimension size

*/

extern const int TINTFLOB_DIMENSION_SIZE;

}

#endif // TILEALGEBRA_CONSTANTS_H
