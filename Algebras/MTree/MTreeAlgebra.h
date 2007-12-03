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

1 MTreeAlgebra

November 2007, Mirko Dibbert

1.1 Overview

TODO insert algebra description

*/
#ifndef __MTREE_ALGEBRA_H
#define __MTREE_ALGEBRA_H

// #define __MT_DEBUG
// #define __MT_PRINT_ENTRY_INFO
// #define __MT_PRINT_NODE_INFO

#define __MT_PRINT_NODE_CACHE_INFO
#define __MT_PRINT_CONFIG_INFO
// #define __MT_PRINT_SPLIT_INFO
#define __MT_PRINT_INSERT_INFO
#define __MT_PRINT_SEARCH_INFO

#include <stack>
#include "StandardTypes.h"
#include "WinUnix.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "MetricRegistry.h"
#include "MetricalAttribute.h"

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

const unsigned MAX_CACHED_NODES = 1024;
/*
The maximum number of nodes, which should be hold open in the node cache

*/

const bool ROOT = true;
const bool SIZE_CHANGED = true;
/*

Some constants to make the source better readable.

*/

}

#endif
