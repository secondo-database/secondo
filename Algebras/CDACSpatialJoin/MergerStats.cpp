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


*/

#include <assert.h>
#include <iostream>
#include <iomanip>

#include "MergerStats.h"
#include "Utils.h"

using namespace cdacspatialjoin;
using namespace std;


/*
1 MergerStats struct

*/
MergerStats::MergerStats() :
        reportPairsCount(0),
        reportPairsSubCount(0),
        reportPairsSub1Count(0),
        reportPairsSub11Count(0),
        loopStats {0} {
}

void MergerStats::add(size_t cycleCount) {
   unsigned lbCycleCount = 0; // the binary logarithm of cycleCount
   while (cycleCount != 0) {
      ++lbCycleCount;
      cycleCount >>= 1U;
   }
   assert (lbCycleCount < LOOP_STATS_COUNT);
   ++loopStats[lbCycleCount];
}

void MergerStats::report(std::ostream& out) const {
   unsigned lastEntry = 0;
   uint64_t totalLoopCount = 0;
   for (unsigned i = 0; i < LOOP_STATS_COUNT; ++i) {
      if (loopStats[i] > 0) {
         lastEntry = i;
         totalLoopCount += loopStats[i];
      }
   }
   if (lastEntry > 0) {
      cout << endl << "Statistics for Merger::reportPairs(): "
           << formatInt(reportPairsCount) << " calls; " << endl;

      const uint64_t subSum = reportPairsSubCount + reportPairsSub1Count +
                              reportPairsSub11Count;
      cout << "Statistics for Merger::reportPairsSub(): "
           << formatInt(subSum) << " calls total: " << endl;
      cout << setw(14) << formatInt(reportPairsSubCount) << " calls "
           << "(" << reportPairsSubCount * 100.0 / subSum << " %) "
           << "with multiple edges in both sets + " << endl;
      cout << setw(14) << formatInt(reportPairsSub1Count) << " calls "
           << "(" << reportPairsSub1Count * 100.0 / subSum << " %) "
           << "with only one edge in either of the sets + " << endl;
      cout << setw(14) << formatInt(reportPairsSub11Count) << " calls "
           << "(" << reportPairsSub11Count * 100.0 / subSum << " %) "
           << "with only one edge in both of the sets" << endl;

      cout << "In a total of   " << formatInt(totalLoopCount) << " loops "
           << "in reportPairsSub() (multiple edges), the cycle count was ..."
           << endl;

      uint64_t cycleCount = 1;
      for (unsigned i = 0; i <= lastEntry; ++i) {
         if (i == 0 && loopStats[i] == 0)
            continue; // skip this if such while loops are prevented
         cout << "< " << setw(7) << formatInt(cycleCount) << ": "
              << setw(14) << formatInt(loopStats[i]) << " loops"
              << " (" << loopStats[i] * 100.0 / totalLoopCount << " %)" << endl;
         cycleCount <<= 1U;
      }
      cout << endl;
   }
}
