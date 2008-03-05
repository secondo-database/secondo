/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Headerfile "DistFunSymbols.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains constants for the "DistDataReg"[4] class, in particular the names and short descriptions of all defined distdata types.

1.1 Includes and defines

*/
#ifndef __DIST_DATA_SYMBOLS_H
#define __DIST_DATA_SYMBOLS_H

#include "Symbols.h"
#include <map>

namespace general_tree
{

/*
This constant is not used as name, but if a distdata object with this name is requested, the first found "DistDataInfo"[4] object with a defined "DDATA[_]IS[_]DEFAULT"[4] flag.

*/
Sym DDATA_DEFAULT("default");

/*
This constant is used as result in the "defaultName"[4] method of the "DistDataReg"[4] class, if no default value has been found.

*/
Sym DDATA_UNDEFINED("n/a");

/*
1.1 Distdata type names and descriptons

*/

/* native data representation, e.g. the memory representation of
   attributes (for CcInt, CcReal and CcString attributes only the
   value is stored, since the defined flag is not needed) */
Sym DDATA_NATIVE("native");
Sym DDATA_NATIVE_DESCR(
        "native representation of the respective datatype");

// types for the "picture" type constructor
Sym DDATA_HSV128("hsv128");
Sym DDATA_HSV128_DESCR(
        "hsv histogram with 128 bins");

Sym DDATA_HSV256("hsv256");
Sym DDATA_HSV256_DESCR(
        "hsv histogram with 256 bins");

Sym DDATA_LAB256("lab256");
Sym DDATA_LAB256_DESCR(
        "lab histogram with 256 bins");

Sym DDATA_HSV128_NCOMPR("hsv128_ncompr");
Sym DDATA_HSV128_NCOMPR_DESCR(
        "uncompressed hsv histogram with 128 bins");

Sym DDATA_HSV256_NCOMPR("hsv256_ncompr");
Sym DDATA_HSV256_NCOMPR_DESCR(
        "uncompressed hsv histogram with 256 bins");

Sym DDATA_LAB256_NCOMPR("lab256_ncompr");
Sym DDATA_LAB256_NCOMPR_DESCR(
        "uncompressed lab histogram with 256 bins");

/*
1.1 Distdata types id's

Each distdata type needs a unique id, that is stored within distdata attributes instead of the distdata type name (the id will be assigned to the respective distdata type in the "DistDataInfo"[4] constructor).

*/
const int DDATA_NATIVE_ID = 0;
const int DDATA_HSV128_ID = 1;
const int DDATA_HSV256_ID = 2;
const int DDATA_LAB256_ID = 3;
const int DDATA_HSV128_NCOMPR_ID = 4;
const int DDATA_HSV256_NCOMPR_ID = 5;
const int DDATA_LAB256_NCOMPR_ID = 6;

} // namespace distfun_symbols
#endif // #ifndef __DIST_FUN_SYMBOLS_H
