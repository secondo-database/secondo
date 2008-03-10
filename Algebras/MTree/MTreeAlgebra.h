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

January-February 2008, Mirko Dibbert

1.1 Overview

This file contains some defines and constants, which could be used to configurate the mtree algebra.

1.1 Includes and defines

*/
#ifndef __MTREE_ALGEBRA_H
#define __MTREE_ALGEBRA_H

/////////////////////////////////////////////////////////////////////
// enables debugging mode for the mtree-algebra:
/////////////////////////////////////////////////////////////////////
// #define MTREE_DEBUG


/////////////////////////////////////////////////////////////////////
// enables debugging mode for general tree algebra framework:
/////////////////////////////////////////////////////////////////////
// #define GTAF_DEBUG


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the insert method:
// (should be replaced by progress operator version)
/////////////////////////////////////////////////////////////////////
#define MTREE_PRINT_INSERT_INFO

/////////////////////////////////////////////////////////////////////
// enables print of count of objects in leaf/right node after split:
/////////////////////////////////////////////////////////////////////
// #define MTREE_PRINT_SPLIT_INFO


/////////////////////////////////////////////////////////////////////
// enables print of statistic infos in the search methods:
/////////////////////////////////////////////////////////////////////
// #define MTREE_PRINT_SEARCH_INFO



#include "GTAF.h"
#include "DistfunReg.h" // also includes distdata

namespace mtreeAlgebra {
// en-/disable caching for all node types
const bool nodeCacheEnabled = true;

// en-/disable caching seperately for each node type
const bool leafCacheable = true;
const bool internalCacheable = true;

// intevall of printing statistic infos in the insert method
// (does only work, if MTREE_PRINT_INSERT_INFO has been defined)
const int insertInfoInterval = 100;



/********************************************************************
Default values for the node config objects (used in the MTreeConfig class):

********************************************************************/
// min. count of pages for leaf / internal nodes
const unsigned minLeafPages   = 1;
const unsigned minIntPages    = 1;

// max. count of pages for leaf / internal nodes
const unsigned maxLeafPages   = 1;
const unsigned maxIntPages    = 1;

// min. count of entries for leaf / internal nodes
const unsigned minLeafEntries = 3;
const unsigned minIntEntries  = 3;

// max. count of entries for leaf / internal nodes
const unsigned maxLeafEntries = numeric_limits<unsigned>::max();
const unsigned maxIntEntries = numeric_limits<unsigned>::max();



/********************************************************************
The following constants should not be changed:

********************************************************************/
using namespace generalTree;
using gtaf::NodeConfig;
using gtaf::NodeTypeId;

// constants for the node types
const NodeTypeId Leaf = 0;
const NodeTypeId Internal = 1;

// priorities of the node types
const unsigned leafPrio       = 0; // default = 0
const unsigned internalPrio   = 1; // default = 1

} // namespace mtreeAlgebra
#endif // #ifndef __MTREE_ALGEBRA_H
