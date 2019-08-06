/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{2}
\tableofcontents


1 includes

*/

#pragma once

#include "IOData.h"

namespace cdacspatialjoin {
/*
2 MergerStats struct

Encapsulates counters for the usage of the Merger::reportPairs...() functions
and the iteration frequency of the while loops in Merger::reportPairsSub().

*/
struct MergerStats {
#ifdef CDAC_SPATIAL_JOIN_METRICS
   /* statistics on the usage of SelfMerger functions and the
    * iteration frequency of the while loops in (Self)Merger::reportPairsSub().
    * Using a static instance is not a "pretty" solution but saves us from
    * injecting a MergerStats instance into millions of (Self)Merger
    * instances */
   static std::unique_ptr<MergerStats> onlyInstance;
#endif

   /* size of loopStats array (see below). 32 allows for up to
    * 2^32 - 1 = approx. 4 billion cycles in while loops */
   static constexpr unsigned LOOP_STATS_COUNT = 32;

   /* the number of times reportPairs() was called */
   uint64_t reportPairsCount;

   /* the number of times reportPairsSub() was called (excluding special cases
    * with only 1 edge in one of the sets) */
   uint64_t reportPairsSubCount;

   /* the number of times reportPairsSub() was called with only 1 edge in
    * either (but not both) of the sets */
   uint64_t reportPairsSub1Count;

   /* the number of times reportPairsSub() was called with only 1 edge in
    * both sets */
   uint64_t reportPairsSub11Count;

   /* statistics on the while loops in Merger::reportPairsSub():
    * for each while loop with n cycles, the entry loopStats[lb n] is
    * incremented by 1. For instance, loopStats[5] holds the number of while
    * loops that had 16-31 (i.e. between 2^4 and 2^5 - 1) cycles */
   uint64_t loopStats[LOOP_STATS_COUNT];

public:
   /* updates the loop statistics, adding a loop with the given count of
    * cycles */
   void add(size_t cycleCount);

   void reset();

   /* constructor, setting all counters to 0 */
   MergerStats();

   /* destructor */
   ~MergerStats() = default;

   /* reports the loop statistics (while loops in Merger::reportPairsSub())
    * to the given out stream */
   void report(std::ostream& out) const;
};


}