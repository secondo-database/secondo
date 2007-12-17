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

1 MTreeAlgebra

November/December 2007, Mirko Dibbert

1.1 Overview

TODO insert algebra description

1.2 Global includes and Defines

*/
#ifndef MTREE_ALGEBRA_H
#define MTREE_ALGEBRA_H

#define __MT_DEBUG

// #define __MT_PRINT_SPLIT_INFO
#define __MT_PRINT_NODE_CACHE_INFO
#define __MT_PRINT_CONFIG_INFO
#define __MT_PRINT_INSERT_INFO
#define __MT_PRINT_SEARCH_INFO

using namespace std;

namespace MT
{

const unsigned NODE_PAGESIZE = ( WinUnix::getPageSize() - 60 );
/*
Size of a m-tree node. If an error like

----
DbEnv: Record size of x too large for page size of y
----
occurs, the integer value needs to be increased!

*/

const unsigned NODE_CACHE_SIZE = 8*1024*1024;
/*
Size of the node cache in bytes.

Warning: If the cache size is to huge (greater than about 1GB on my sytem
(SuSE 10.2, 32bit, system pagesize = 4k)), the following error could occur:

----
DbEnv: Lock table is out of available object entries
----

*/

const bool ROOT = true;
const bool SIZE_CHANGED = true;
/*

Some constants to make the source code better readable.

*/

}

#endif
