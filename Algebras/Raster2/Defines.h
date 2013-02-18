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

#ifndef RASTER2_DEFINES_H
#define RASTER2_DEFINES_H

#include <limits>
#include <string>
#include "TemporalAlgebra.h"

using namespace std;

namespace raster2
{
  
/*
definition of RASTER2\_TRACE Macro

*/

#define RASTER2_TRACE cout << __PRETTY_FUNCTION__ \
                           << " of object " << this \
                           << " in " << __FILE__ \
                           << " line " << __LINE__ \
                           << endl;
                           
/*
definition of RASTER2\_STATIC\_TRACE Macro

*/

#define RASTER2_STATIC_TRACE cout << __PRETTY_FUNCTION__ \
                                  << " in " << __FILE__ \
                                  << " line " << __LINE__ \
                                  << endl;

/*
const type name for type sint

*/

extern const char* TYPE_NAME_SINT;

/*
const type name for type sreal

*/

extern const char* TYPE_NAME_SREAL;

/*
const type name for type sbool

*/

extern const char* TYPE_NAME_SBOOL;

/*
const type name for type sstring

*/

extern const char* TYPE_NAME_SSTRING;

/*
const type name for type msint

*/

extern const char* TYPE_NAME_MSINT;

/*
const type name for type msreal

*/

extern const char* TYPE_NAME_MSREAL;

/*
const type name for type msbool

*/

extern const char* TYPE_NAME_MSBOOL;

/*
const type name for type msstring

*/

extern const char* TYPE_NAME_MSSTRING;

/*
const type name for type isint

*/

extern const char* TYPE_NAME_ISINT;

/*
const type name for type isreal

*/

extern const char* TYPE_NAME_ISREAL;

/*
const type name for type isbool

*/

extern const char* TYPE_NAME_ISBOOL;

/*
const type name for type isstring

*/

extern const char* TYPE_NAME_ISSTRING;

/*
const undefined value for type int

*/

extern const int UNDEFINED_INT;

/*
const undefined value for type real

*/

extern const double UNDEFINED_REAL;

/*
const undefined value for type bool

*/

extern const char UNDEFINED_BOOL;

/*
const undefined value for type string

*/

extern const string UNDEFINED_STRING;

/*
const undefined string index value

*/

extern const int UNDEFINED_STRING_INDEX;

/*
const undefined value for type MInt

*/

extern const MInt UNDEFINED_MINT;

/*
const undefined value for type MReal

*/

extern const MReal UNDEFINED_MREAL;

/*
const undefined value for type MBool

*/

extern const char UNDEFINED_MBOOL;

}

#endif // RASTER2\_DEFINES\_H
