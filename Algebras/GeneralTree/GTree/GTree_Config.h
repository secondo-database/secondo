/*
\newpage

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

1.1 Headerfile "GTree[_]Config.h"[4]

January-May 2008, Mirko Dibbert

This file contains some constants and defines, which could be used to configurate the framework.

*/
#ifndef __GTREE_CONFIG_H__
#define __GTREE_CONFIG_H__

#include "WinUnix.h"
#include "GTree_Msg.h"

namespace gtree
{

/////////////////////////////////////////////////////////////////////
// activate debug mode and open-object-counter for the framework
//
// Warning: Never define __GTREE_DEBUG somewhere else, otherwhise the
// framework is not guaranted to work correct, since it's object
// files could e.g. be compiled with enabled debug mode, which
// would lead to errors if they are used from headers where the debug
// mode is disabled duo to the EntryBase class, which uses a virtual
// destructor in debug mode and a static one otherwhise.
/////////////////////////////////////////////////////////////////////
// #define __GTREE_DEBUG
// #define ENABLE_OBJECT_COUNT



/////////////////////////////////////////////////////////////////////
// show "writing cached nodes to disc" message
/////////////////////////////////////////////////////////////////////
// #define __GTREE_SHOW_WRITING_CACHED_NODES_MSG



/////////////////////////////////////////////////////////////////////
// print hits/misses statistic for the node cache
/////////////////////////////////////////////////////////////////////
// #define __GTREE_SHOW_CACHE_INFOS



/////////////////////////////////////////////////////////////////////
// node pagesize (must be smaller than the system pagesize)
// - a small amount of bytes per page is reserved for maintenance -
//
// if an error like
//
//         DbEnv: Record size of x too large for page size of y
//
// occurs, the pagesize needs to be reduced!
//
// Warning: changing "PAGESIZE"[4] could propably lead to errors on
// existing trees, which are based on this framework (e.g. m-trees
// and x-trees), thus these trees should be rebuild in this case!
/////////////////////////////////////////////////////////////////////
const unsigned PAGESIZE = (WinUnix::getPageSize() - 60);



/////////////////////////////////////////////////////////////////////
// node cache sizes
//
// (if the size of the node cache exceeds maxNodeCacheSize, it would
// remove nodes from the cache, until the size is smaller than
// minNodeCacheSize)
/////////////////////////////////////////////////////////////////////
const unsigned minNodeCacheSize = 7*1024*1024; // default 7 MB
const unsigned maxNodeCacheSize = 9*1024*1024; // default 9 MB

/////////////////////////////////////////////////////////////////////
// maximum size of a node that should be able to be cached
// (should be smaller or equal or maxNodeCacheSize)
/////////////////////////////////////////////////////////////////////
const unsigned maxCacheableSize = 9*1024*1024; // default 9 MB



} // namespace gtree

// must be included after the ENABLE_OBJECT_COUNT define
#include "ObjCounter.h"

#endif // #define __GTREE_CONFIG_H__
