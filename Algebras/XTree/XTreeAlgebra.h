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

1 Headerfile "XTreeAlgebra.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This file contains some defines and constants, which could be used to configurate this algebra.

*/
#ifndef __XTREE_ALGEBRA_H__
#define __XTREE_ALGEBRA_H__

#include "GeneralTreeAlgebra.h"

/////////////////////////////////////////////////////////////////////
// enables debugging mode for the xtree-algebra
/////////////////////////////////////////////////////////////////////
// #define __XTREE_DEBUG


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the insert method
/////////////////////////////////////////////////////////////////////
#define __XTREE_PRINT_INSERT_INFO


/////////////////////////////////////////////////////////////////////
// enables print of mtree statistics in the out function
/////////////////////////////////////////////////////////////////////
#define __XTREE_OUTFUN_PRINT_STATISTICS


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the search methods:
/////////////////////////////////////////////////////////////////////
// #define __XTREE_PRINT_SEARCH_INFO

/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the search methods
// to file "xtree.log"
/////////////////////////////////////////////////////////////////////
// #define __XTREE_PRINT_STATS_TO_FILE


/////////////////////////////////////////////////////////////////////
// use min deadspace to resolve ties instead of minimal area in the
// topologicalSplit and overlapMinimalSplit methods
/////////////////////////////////////////////////////////////////////
#define __XTREE_SPLIT_USE_MIN_DEADSPACE


namespace xtreeAlgebra
{

/////////////////////////////////////////////////////////////////////
// these constants are used within the XTree::split method
/////////////////////////////////////////////////////////////////////
const double MAX_OVERLAP = 0.2;
const double MIN_FANOUT  = 0.35;


/////////////////////////////////////////////////////////////////////
// en-/disable caching for all node types
/////////////////////////////////////////////////////////////////////
const bool nodeCacheEnabled = true;


/////////////////////////////////////////////////////////////////////
// intevall of printing statistic infos in the insert method
// (only used, if __XTREE_PRINT_INSERT_INFO is defined)
/////////////////////////////////////////////////////////////////////
const int insertInfoInterval = 10;



/********************************************************************
The following constants are only default values for the xtree-config objects, that could be changed in some configurations. See the initialize method of the "XTreeConfigReg"[4] class for details.

********************************************************************/

/////////////////////////////////////////////////////////////////////
// en-/disable caching seperately for each node type
/////////////////////////////////////////////////////////////////////
const bool leafCacheable      = true;
const bool internalCacheable  = true;
const bool supernodeCacheable = true;


/////////////////////////////////////////////////////////////////////
// max. count of pages for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned maxLeafPages = 1;
const unsigned maxIntPages  = 1;


/////////////////////////////////////////////////////////////////////
// min. count of entries for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned minLeafEntries = 3;
const unsigned minIntEntries  = 3;


/////////////////////////////////////////////////////////////////////
// max. count of entries for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned maxLeafEntries = std::numeric_limits<unsigned>::max();
const unsigned maxIntEntries  = std::numeric_limits<unsigned>::max();


/////////////////////////////////////////////////////////////////////
// priorities of the defined node types
// (higher priorities result into a higher probablility for
// nodes of the respective type to remain in the node cache)
/////////////////////////////////////////////////////////////////////
const unsigned leafPrio       = 0; // default = 0
const unsigned internalPrio   = 1; // default = 1
const unsigned supernodePrio  = 2; // default = 2


/////////////////////////////////////////////////////////////////////
// constants for the node type id's
/////////////////////////////////////////////////////////////////////
const gtree::NodeTypeId LEAF      = 0;
const gtree::NodeTypeId INTERNAL  = 1;
const gtree::NodeTypeId SUPERNODE = 2;



// define __XTREE_ANALYSE_STATS if __XTREE_PRINT_SEARCH_INFO or
// __XTREE_PRINT_STATS_TO_FILE has been defined
#ifdef __XTREE_PRINT_SEARCH_INFO
  #define __XTREE_ANALYSE_STATS
#else
  #ifdef __XTREE_PRINT_STATS_TO_FILE
  #define __XTREE_ANALYSE_STATS
  #endif
#endif
} // namespace xtreeAlgebra

#endif // #ifndef __XTREE_ALGEBRA_H__
