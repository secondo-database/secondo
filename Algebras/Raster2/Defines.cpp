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

#include "Defines.h"

namespace raster2
{

/*
const type name for type sint

*/

const char* TYPE_NAME_SINT = "sint";

/*
const type name for type sreal

*/

const char* TYPE_NAME_SREAL = "sreal";

/*
const type name for type sbool

*/

const char* TYPE_NAME_SBOOL = "sbool";

/*
const type name for type sstring

*/

const char* TYPE_NAME_SSTRING = "sstring";

/*
const type name for type msint

*/

const char* TYPE_NAME_MSINT = "msint";

/*
const type name for type msreal

*/

const char* TYPE_NAME_MSREAL = "msreal";

/*
const type name for type msbool

*/

const char* TYPE_NAME_MSBOOL = "msbool";

/*
const type name for type msstring

*/

const char* TYPE_NAME_MSSTRING = "msstring";

/*
const type name for type isint

*/

const char* TYPE_NAME_ISINT = "isint";

/*
const type name for type isreal

*/

const char* TYPE_NAME_ISREAL = "isreal";

/*
const type name for type isbool

*/

const char* TYPE_NAME_ISBOOL = "isbool";

/*
const type name for type isstring

*/

const char* TYPE_NAME_ISSTRING = "isstring";

/*
const undefined value for type int

*/

const int UNDEFINED_INT = std::numeric_limits<int>::min();

/*
const undefined value for type real

*/

const double UNDEFINED_REAL = std::numeric_limits<double>::quiet_NaN();

/*
const undefined value for type bool

*/

const char UNDEFINED_BOOL = -1;

/*
const undefined value for type MInt

*/

// TODO: if this is needed in the future, it has to be defined
// in a different way. C++ provides template specialization of
// std::numeric_limits only for built-in types
//const MInt UNDEFINED_MINT = std::numeric_limits<MInt>::min();

/*
const undefined value for type MReal

*/

// TODO: if this is needed in the future, it has to be defined
// in a different way. C++ provides template specialization of
// std::numeric_limits only for built-in types
//const MReal UNDEFINED_MREAL = std::numeric_limits<MReal>::quiet_NaN();

/*
const undefined value for type MBool

*/

const char UNDEFINED_MBOOL = -1;

/*
const undefined value for type string

*/

const string UNDEFINED_STRING = "";

/*
const undefined string index value

*/

const int UNDEFINED_STRING_INDEX = -1;

}
