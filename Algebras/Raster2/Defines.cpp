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

using namespace std;

namespace raster2
{

/*
const type name for type sint

*/

string TYPE_NAME_SINT() { return  "sint"; }

/*
const type name for type sreal

*/

string TYPE_NAME_SREAL() { return  "sreal";}

/*
const type name for type sbool

*/

string TYPE_NAME_SBOOL() { return  "sbool";}

/*
const type name for type sstring

*/

string TYPE_NAME_SSTRING(){ return  "sstring";}

/*
const type name for type msint

*/

string TYPE_NAME_MSINT() { return  "msint";}

/*
const type name for type msreal

*/

string TYPE_NAME_MSREAL(){ return   "msreal";}

/*
const type name for type msbool

*/

string TYPE_NAME_MSBOOL(){ return  "msbool";}

/*
const type name for type msstring

*/

string TYPE_NAME_MSSTRING(){ return  "msstring";}

/*
const type name for type isint

*/

string TYPE_NAME_ISINT(){ return "isint"; }

/*
const type name for type isreal

*/

string TYPE_NAME_ISREAL(){ return "isreal";}

/*
const type name for type isbool

*/

string  TYPE_NAME_ISBOOL(){ return  "isbool";}

/*
const type name for type isstring

*/

string TYPE_NAME_ISSTRING(){ return  "isstring";}

/*
const undefined value for type int

*/

int UNDEFINED_INT(){ return std::numeric_limits<int>::min();}

/*
const undefined value for type real

*/

double UNDEFINED_REAL(){ return std::numeric_limits<double>::quiet_NaN();}

/*
const undefined value for type bool

*/

 char UNDEFINED_BOOL(){ return  -1;}

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

char UNDEFINED_MBOOL(){ return -1;}

/*
const undefined value for type string

*/

std::string UNDEFINED_STRING(){ return  ""; }

/*
const undefined string index value

*/

int UNDEFINED_STRING_INDEX(){ return  -1;}

}
