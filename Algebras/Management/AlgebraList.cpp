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

*/

#include "AlgebraInit.h"

/*
1.2 List of available Algebras

*/

/*
Creation of the prototypes of the initilization functions of all requested
algebra modules.

*/
#define ALGEBRA_INCLUDE ALGEBRA_PROTO_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_PROTO_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_PROTO_DYNAMIC
#include "AlgebraList.i"

/*
Creation of the list of all requested algebra modules.
The algebra manager uses this list to initialize the algebras and
to access the type constructor and operator functions provided by
the algebra modules.

*/

#undef ALGEBRA_INCLUDE
#undef ALGEBRA_EXCLUDE
#undef ALGEBRA_DYNAMIC
#define ALGEBRA_INCLUDE ALGEBRA_LIST_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_LIST_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_LIST_DYNAMIC

AlgebraListEntry& GetAlgebraEntry( const int j )
{
ALGEBRA_LIST_START
#include "AlgebraList.i"
ALGEBRA_LIST_END
/*
is the static list of all available algebra modules.

*/
  return (algebraList[j]);
}

