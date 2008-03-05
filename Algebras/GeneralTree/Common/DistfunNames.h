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

This headerfile contains constants for the "DistFunReg"[4] class, in particular the names and short descriptions of all defined distance functions.

1.1 Includes and defines

*/
#ifndef __DIST_FUN_SYMBOLS_H
#define __DIST_FUN_SYMBOLS_H

#include "Symbols.h"
#include <map>

namespace general_tree
{

/*
This constant is not used as name, but if a distance function with this name is requested, the first found "DistFunInfo"[4] object with a defined "DFUN[_]IS[_]DEFAULT"[4] flag.

*/
Sym DFUN_DEFAULT("default");

/*
This constant is used as result in the "defaultName"[4] method of the "DistfunReg"[4] class, if no default value has been found.

*/
Sym DFUN_UNDEFINED("n/a");

/*
1.1 Distance function names and descriptons

*/
Sym DFUN_EUCLID("euclid");
Sym DFUN_EUCLID_DESCR(
    "euclidean distance");

Sym DFUN_EDIT_DIST("edit");
Sym DFUN_EDIT_DIST_DESCR(
    "edit distance");

Sym DFUN_QUADRATIC("quadratic");
Sym DFUN_QUADRATIC_DESCR(
    "quadratic distance using a similarity matrix");

} // namespace distfun_symbols
#endif // #ifndef __DIST_FUN_SYMBOLS_H
