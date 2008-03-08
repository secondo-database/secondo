/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This file belongs to the GeneralTreeAlgebra framework (GTAF)           %
% Class descriptions and usage details could be found in gtaf.pdf        %
%                                                                        %
% (if this file does not exist, use "make docu" in the parent directory) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

1.1 Headerfile "GTAF[_]Config.h"[4]

January-February 2008, Mirko Dibbert

*/
#ifndef __GTAF_CONFIG_H
#define __GTAF_CONFIG_H

#include <vector>
#include <stack>
#include <list>
#include "GTAF_Types.h"
#include "WinUnix.h"

#ifdef GTAF_DEBUG
    #define ENABLE_OBJECT_COUNT
    // enable counter for open objects

    #define GTAF_SHOW_CACHE_INFOS
    // print hits/misses statistic of the node cache
#endif

namespace gtaf
{

const unsigned PAGESIZE = (WinUnix::getPageSize() - 60);
/*
Size of a page in the tree file. If an error like

----
DbEnv: Record size of x too large for page size of y
----
occurs, the pagesize needs to be reduced!

Warning: Changing "PAGESIZE"[4] could propably lead to errors on existing trees,
which are based on this framework (e.g. m-trees and x-trees), thus all these
trees should be rebuild in this case!

*/

const unsigned minNodeCacheSize = 7*1024*1024;
const unsigned maxNodeCacheSize = 9*1024*1024;
/*
Minimum / maximum node cache size (per tree). If the size of the node cache exceeds "maxNode-"[4] "CacheSize"[4], it would remove nodes from the cache, until the size is smaller than "minNode-"[4] "CacheSize"[4].

*/

} // namespace gtaf
#endif // #ifndef __GTAF_CONFIG_H
