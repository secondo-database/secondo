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

1 Headerfile "MTreeAlgebra.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This file contains some defines and constants, which could be used to configurate this algebra.

*/
#ifndef __MTREE_ALGEBRA_H__
#define __MTREE_ALGEBRA_H__

#include "Algebras/GeneralTree/GeneralTreeAlgebra.h"




/////////////////////////////////////////////////////////////////////
// enables debugging mode for the mtree-algebra:
/////////////////////////////////////////////////////////////////////
//#define __MTREE_DEBUG


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the insert method:
/////////////////////////////////////////////////////////////////////
#define __MTREE_PRINT_INSERT_INFO


/////////////////////////////////////////////////////////////////////
// enables print of mtree statistics in the out function:
/////////////////////////////////////////////////////////////////////
#define __MTREE_OUTFUN_PRINT_STATISTICS


/////////////////////////////////////////////////////////////////////
// enables print of count of objects in leaf/right node after split:
/////////////////////////////////////////////////////////////////////
// #define MTREE_PRINT_SPLIT_INFO


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the search methods:
/////////////////////////////////////////////////////////////////////
// #define __MTREE_PRINT_SEARCH_INFO


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the search methods
// to file "mtree.log"
/////////////////////////////////////////////////////////////////////
// #define __MTREE_PRINT_STATS_TO_FILE



namespace mtreeAlgebra
{

enum PROMOTE
{ RANDOM, m_RAD, mM_RAD, M_LB_DIST };
/*
Enumeration of the implemented promote functions:

  * "RANDOM"[4] : Promotes two random entries.

  * "m[_]RAD"[4] : Promotes the entries which minimizes the sum of both covering radii.

  * "mM[_]RAD:"[4] Promotes the entries which minimizes the maximum of both covering radii.

  * "M[_]LB[_]DIST"[4] : Promotes as first entry the previously promoted element, which is equal to the parent entry. As second entry, the one with maximum distance to the parent entry would be promoted.

*/

enum PARTITION
{ GENERALIZED_HYPERPLANE, BALANCED };
/*
Enumeration of the implemented partition functions.

Let "p_1"[2], "p_2"[2] be the promoted items and "N_1"[2], "N_2"[2] be the nodes containing "p_1"[2] and "p_2"[2]:

  * "GENERALIZED[_]HYPERPLANE"[4] The algorithm assign an entry "e"[2] as follows: if "d(e,p_1) \leq d(e,p_2)"[2], "e"[2] is assigned to "N_1"[2], otherwhise it is assigned to "N_2"[2].

  * "BALANCED"[4] : This algorithm alternately assigns the nearest neighbour of "p_1"[2] and "p_2"[2], which has not yet been assigned, to "N_1"[2] and "N_2"[2], respectively.

*/

/////////////////////////////////////////////////////////////////////
// en-/disable caching for all node types
/////////////////////////////////////////////////////////////////////
const bool nodeCacheEnabled = true;


/////////////////////////////////////////////////////////////////////
// intevall of printing statistic infos in the insert method
// (only used, if __MTREE_PRINT_INSERT_INFO has been defined)
/////////////////////////////////////////////////////////////////////
const int insertInfoInterval = 100;



/********************************************************************
The following constants are only default values for the mtree-config objects, that could be changed in some configurations. See the initialize method of the "MTreeConfigReg"[4] class for details.

********************************************************************/

/////////////////////////////////////////////////////////////////////
// default split policy
/////////////////////////////////////////////////////////////////////
const PROMOTE defaultPromoteFun = M_LB_DIST;
const PARTITION defaultPartitionFun = BALANCED;


/////////////////////////////////////////////////////////////////////
// en-/disable caching seperately for each node type
/////////////////////////////////////////////////////////////////////
const bool leafCacheable     = true;
const bool internalCacheable = true;


/////////////////////////////////////////////////////////////////////
// max. count of pages for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned maxLeafPages   = 1;
const unsigned maxIntPages    = 1;


/////////////////////////////////////////////////////////////////////
// min. count of entries for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned minLeafEntries = 3;
const unsigned minIntEntries  = 3;


/////////////////////////////////////////////////////////////////////
// max. count of entries for leaf / internal nodes
/////////////////////////////////////////////////////////////////////
const unsigned maxLeafEntries = std::numeric_limits<unsigned>::max();
const unsigned maxIntEntries = std::numeric_limits<unsigned>::max();


/////////////////////////////////////////////////////////////////////
// priorities of the node types
// (higher priorities result into a higher probablility for
// nodes of the respective type to remain in the node cache)
/////////////////////////////////////////////////////////////////////
const unsigned leafPrio     = 0; // default = 0
const unsigned internalPrio = 1; // default = 1


/////////////////////////////////////////////////////////////////////
// constants for the node type id's
/////////////////////////////////////////////////////////////////////
const gtree::NodeTypeId LEAF = 0;
const gtree::NodeTypeId INTERNAL = 1;



// define __MTREE_ANALYSE_STATS if __MTREE_PRINT_SEARCH_INFO or
// __MTREE_PRINT_STATS_TO_FILE has been defined
#ifdef __MTREE_PRINT_SEARCH_INFO
  #define __MTREE_ANALYSE_STATS
#else
  #ifdef __MTREE_PRINT_STATS_TO_FILE
  #define __MTREE_ANALYSE_STATS
  #endif
#endif

} // namespace mtreeAlgebra
#endif // #ifndef __MTREE_ALGEBRA_H__
