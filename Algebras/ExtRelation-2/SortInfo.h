/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

1 Header File SortInfo.h

May 2009. S. Jungnickel. Initial version.

2 Defines and Includes

*/

#ifndef SEC_SORTINFO_H
#define SEC_SORTINFO_H

#include <queue>

#include "StopWatch.h"
#include "RelationAlgebra.h"

using namespace std;

namespace extrel2
{
/*
3 Class ~SortedRunInfo~

Class ~SortedRunInfo~ is used to collect trace data
for a single run during run creation.

*/
  class SortedRunInfo {
  public:

    SortedRunInfo(int no);
/*
The constructor. Constructs a ~SortedRunInfo~ instance using
the specified run number ~no~.

*/

    SortedRunInfo(SortedRunInfo& info);
/*
Copy constructor.

*/

    ~SortedRunInfo();
/*
The destructor.

*/

    float Ratio();
/*
Returns the ratio between the total number of tuples in the run
and the number of tuples which fit into the operator main memory.

*/

    float Size();
/*
Returns the total run size in MByte.

*/

    int No;
/*
Run number

*/

    int RunLength;
/*
Number of tuples in Run

*/

    int RunSize;
/*
Run size in bytes

*/

    int MinimumRunLength;
/*
Number of tuples from operators main memory

*/

    int AdditionalRunLength;
/*
Number of tuples added to run by replacement selection

*/

    int TuplesPassedToNextRun;
/*
Number of tuples which were moved to the next run

*/

    int TuplesInMemory;
/*
Number of tuples in memory (after initial run phase)

*/

    int TuplesOnDisk;
/*
Number of tuples on disk (after initial run phase)

*/

    float RunRatio;
/*
Ratio between ~MinimumRunLength~ and ~RunLength~
(benefit of replacement selection)

*/
  };

  ostream& operator <<(ostream& stream, SortedRunInfo& info);
/*
Overloaded operator << of ~SortedRunInfo~

*/


/*
4 Class ~SortInfo~

Class ~SortInfo~ is used to collect data during the usage of
the sort operator. The class provides a method for formatted
information output to a string stream. This class is used to
collect the necessary information to do benchmarking between
the different implementations of the sort operator.

*/

  class SortInfo {
  public:

    SortInfo(size_t bufferSize);
/*
The constructor. Constructs a ~SortInfo~ instance and immediately
sets the operators main memory to ~bufferSize~.

*/

    ~SortInfo();
/*
The destructor.

*/

    void NewRun();
/*
Creates a new ~SortedRunInfo~ instance which may be accessed
using ~CurrentRun()~.

*/

    SortedRunInfo* CurrentRun();
/*
Returns the current ~SortedRunInfo~ instance.

*/

    void clearAll();
/*
Clears all collected sort information.

*/

    void UpdateStatistics(size_t s);
/*
Updates the tuple statistics

*/
    void RunBuildPhase();
/*
Starts time measurement for run generation phase and
total time.

*/

    void MergePhase();
/*
Stops time measurement for the run generation phase and
starts time measurement for merge phase.

*/

    void Finished();
/*
Stops total time measurement

*/

    int BufferSize;          // Total memory usable by Operator (bytes)
    int IOBufferSize;        // I/O Buffer size for writing to disk (bytes)
    int MaxMergeFanIn;       // Maximum merge fan-in
    int F0;                  // Merge fan-in for first intermediate merge phase
    int W;                   // Number of intermediate merge phases after F0
    int IntermediateTuples;  // Number of intermediate tuples to process
    int InitialRunsCount;    // Number of initial runs
    int TotalTupleCount;     // Total number of processed tuples
    int TotalTupleSize;      // Total size of processed tuples (bytes)
    int MinTupleSize;        // Minimum tuple size (bytes)
    int MaxTupleSize;        // Maximum tuple size (bytes)
    float AvgTupleSize;        // Average tuple size (bytes)
    size_t TotalComparisons;

    StopWatch StopWatchTotal; // Timer for total sort time
    StopWatch StopWatchMerge; // Timer for merge phase
    string TimeRunPhase;     // Used time for building initial runs
    string TimeMergePhase;   // Used time for merge phase
    string TimeTotal;        // Total time used by sort operation

    bool RunStatistics;
/*
Flag which controls if detailed run statistics are shown. If set to true
detailed run statistics are shown. Default value is true.

*/

    vector<SortedRunInfo*> InitialRunInfo;
/*
Array for collecting the initial run information

*/

    vector<SortedRunInfo*> FinalRunInfo;
/*
Array for collecting the final run information for the last merge phase.
Initial runs may be delete and be combined into new runs during intermediate
merge phases.

*/
  };

  ostream& operator <<(ostream& stream, SortInfo& info);
/*
Overloaded operator << of ~SortInfo~

*/
}

#endif
