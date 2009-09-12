/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]


1 Header File Sort.h

May 2009, Sven Jungnickel. Initial version

2 Overview

This file contains the declaration of all classes and functions that
implement the new sort algorithm for operators ~sort2~ and ~sortby2~.

3 Includes and defines

*/

#ifndef SORT_H_
#define SORT_H_

#include "RelationAlgebra.h"
#include "Progress.h"
#include "TupleQueue.h"
#include "SortInfo.h"
#include "TupleBuffer.h"
#include "SortedRun.h"

using namespace symbols;

/*
Operators ~sort2with~ and ~sortby2with~ allow to specify the maximum
fan-in for a merge phase. The usable fan-in is limited between a
minimum and maximum value which is specified here. If these limits
are exceeded the fan-in is set to a default value.

*/
#define SORT_MINIMUM_FAN_IN       2
#define SORT_MAXIMUM_FAN_IN       1000
#define SORT_DEFAULT_MAX_FAN_IN   50

/*
To simplify tests for the new sort operators we can also set the usable
main memory size in bytes for the sort operation. The usable memory is
limited to a minimum and maximum value which are specified here. If these
limits are exceeded the main memory size is set to a default value.

*/
#define SORT_MINIMUM_MEMORY       1024
#define SORT_MAXIMUM_MEMORY       ( 64 * 1024 * 1024 )
#define SORT_DEFAULT_MEMORY       ( 16 * 1024 * 1024 )

/*
2 Class ~SortProgressLocalInfo~

All classes and functions have been put into the namespace
~extrel2~ to avoid name conflicts with existing implementations and
to make it later easier to replace existing operators.

This class contains the progress information for the implementation
of the ~sort2~ and ~sortby2~ operators. As the new operator
implementation need to provide some more progress information
during intermediate merge phases this class has been derived from
~ProgressLocalInfo~. The class contains two additional counters
that represent the total and current amount of tuples that must
be processed in intermediate merge phases.

*/
namespace extrel2
{
class SortProgressLocalInfo : public ProgressLocalInfo
{
  public:

    SortProgressLocalInfo();
/*
The constructor.

*/

    int intTuplesTotal;
/*
Total number of tuples to be processed during intermediate merge phases

*/

    int intTuplesProc;
/*
Current number of processed tuples during intermediate merge phases

*/
};

/*
3 Class ~SortAlgorithm~

This class implements an algorithm for external sorting for both
operators ~sort2~ and ~sortby2~. The constructor creates sorted
partitions of the input stream and stores them inside temporary tuple
files and two minimum heaps in memory.  By calls of ~NextResultTuple~
tuples are returned in sorted order. The sort order must be specified
in the constructor. The memory usage is bounded, hence only a fixed
number of tuples can be hold in memory.

The algorithm roughly works as follows: First all input tuples are stored in a
minimum heap until no more tuples fit into memory.  Then, a new tuple file is
created and the minimum is stored there.  Afterwards, the tuples are handled as
follows:

(a) if the next tuple is less or equal than the minimum of the heap and greater
or equal than the last tuple written to disk, it will be appended to the
current file

(b) if the next tuple is smaller than the last written it will be stored in a
second heap to be used in the next created relation.

(c) if the next tuple t is greater than the top of the heap, the minimum will be
written to disk and t will be inserted into the heap.

Finally, the minimum tuple of every run is inserted into a probably small
heap (containing only one tuple for every partition) and for every request
for tuples this minimum is removed and the next tuple of the partition
of the just returned tuples will be inserted into the heap.

This algorithm reduces the number of comparisons which are quite costly inside
Secondo (due to usage of C++ Polymorphism) even for ~standard~ attributes.

Moreover, if the input stream is already sorted only one partition will be
created and no costs for merging tuples will occur. Unfortunateley this solution
needs more comparisons than sorting.

*/
class SortAlgorithm
{
  public:

    SortAlgorithm( Word stream,
                   TupleCompareBy* cmpObj,
                   SortProgressLocalInfo* p,
                   int maxFanIn = SORT_DEFAULT_MAX_FAN_IN,
                   size_t maxMemSize = UINT_MAX );
/*
The constructor. Consumes all tuples of the tuple stream ~stream~ immediately
into sorted runs of approximately two times the size of the operators main
memory by making use of the replacement selection algorithm for run generation.
The internal main memory sort routine uses a minimum heap for sorting. For
tuple comparisons the the algorithm uses the compare object ~cmpObj~. All
progress information will be stored in ~p~. Additionally the constructor
allows to specify the maximum number of open temporary files during a merge
phase, the so called maximum fan-in ~maxFanIn~ for a merge phase and the
maximum memory ~maxMemSize~ the operator uses for the sort operation. If
~maxMemSize~ is set to UINT\_MAX the sort algorithm uses the maximum
main memory which the operators has been assigned by the query processor.

*/

    ~SortAlgorithm();
/*
The destructor. Frees all resources of the algorithm.

*/

    Tuple* NextResultTuple();
/*
Returns the pointer of the next result tuple in sort order. If all tuples
have been processed the method returns 0.

*/

  private:

    void mergeInit();
/*
Initializes the merge phase. If the number of runs exceeds FMAX intermediate
merge phases will be performed. After this method has been called the
minimum heap used for merging tuples will be filled with one tuple from each
run. The number of runs will be limited by FMAX, so that all tuples may be
processed within one final merge phase.

*/

    Tuple* nextResultTuple(vector<SortedRun*>& arr);
/*
Returns the pointer of the next result tuple in sort order. This is an internal
method which is called by ~NextResultTuple~ and during intermediate merge phases.
In the latter case the method uses a temporary array of sorted runs instead of
using ~runs~. If all tuples in ~arr~ have been processed the method returns 0.

*/

    void mergeNShortest(int n);
/*
Merges the ~n~ shortest runs of array ~runs~ into a new run. This method is used
for intermediate merge phases. The total number of runs will be reduced by n-1.
The run length is determined by the number of tuples in a run.

*/

    bool updateIntermediateMerges();
/*
Calculates if any intermediate merge phases are necessary. Member variables
F0 and W are updated if the number of runs exceeds FMAX.

*/

    int simulateIntermediateMerge(vector<int>& arr, int f);
/*
Simulates and the determines the cost for an intermediate merge phase.
~arr~ is an array containing the run length in tuples. ~f~ is the fan-in
of the simulated merge phase. The method returns the number of tuples which
must be read and written once. This method is used by ~calcIntermediateMergeCost~
to determine the total costs for the intermediate merge phases.

*/

    int sumLastN(vector<int>& arr, int n);
/*
Sums the last ~n~ elements of the integer array ~arr~ and returns it as the
result.

*/

    int calcIntermediateMergeCost();
/*
Calculates the intermediate merge costs in number of tuples that must
be read and written once during intermediate merge phases. The effective cost
in ms is obtained by multiplication by factor 2 and a constant for
read/write cost or the sum of two constants (one for write and one for read)

*/

    int FMAX;
/*
Maximum merge fan-in for a merge phase. If the number of sorted runs exceeds
~FMAX~ runs, additional intermediate merge phases have to be performed after
all runs have been created. FMAX thus limits the number of open temporary
files during the merge phase. The number of open files during a merge phase
is maximal FMAX+1 (FMAX files for the runs and one file for the merged run).

*/

    int F0;
/*
Merge fan-in of first intermediate merge phase. The fan-in of the first
intermediate merge phase is calculated so that the last merge-phase has
a fan-in of exactly FMAX runs. N is the number of runs (N > FMAX).

$F_0=((N-FMAX-1) mod (FMAX-1))+2$

*/

    int W;
/*
Additional intermediate merge phases with fan-in FMAX after F0 merge phase.
(W does not include the final merge phase!). N is the number of runs (N > FMAX).

$W=\frac{N-(F_0-1)-FMAX}{FMAX-1}$

*/

    int nextRunNumber;
/*
Run number sequence. Used to number the runs.

*/

    int checkProgressAfter;
/*
During an intermediate merge phase after ~checkProgressAfter~ tuples
have been processed a progress message will be propagated by the
query processor (but only if enough time since the last progress
message has passed by, the query processor will insure this)

*/

    Word stream;
/*
Word which contains the address of the processed tuple stream

*/

    TupleCompareBy *cmpObj;
/*
Compare Object used for tuple comparison. Contains the sort order
specification (also used for lexicographical comparison).

*/

    size_t MAX_MEMORY;
/*
Maximum memory available for the sort operation [bytes]

*/

    size_t usedMemory;
/*
Used memory in bytes

*/

    vector<SortedRun*> runs;
/*
Array which contains the references to the external runs located on disk

*/

    TupleAndRelPosQueue* mergeQueue;
/*
Priority Queue used to merge the runs in the merge phase

*/

    bool traceMode;
/*
Flag which decides if tracing information is generated.
To enable tracing set the flag ~RTF::ERA:TraceSort~ in
SecondoConfig.ini.

*/

    bool traceModeExtended;
/*
Flag which decides if more detailed tracing information
is generated. To enable detailed tracing set the flag
~RTF::ERA:TraceSortExtended~ in SecondoConfig.ini.

*/

    SortInfo* info;
/*
Statistical information if traceMode is true

*/

    SortProgressLocalInfo* const progress;
/*
Local progress information

*/
};

/*
4 Class ~SortLocalInfo~

An instance of this class is used in the value mapping function of both
sort operators to save the state of the sort algorithm between multiple
message calls. This class simplifies access to the progress information.
A pointer to this instance of type ~SortProgressLocalInfo~ will be passed
to the ~SortAlgorithm~ object ~ptr~.

*/
class SortLocalInfo: public SortProgressLocalInfo
{
  public:

    SortLocalInfo() : SortProgressLocalInfo(), ptr(0) {}
/*
The constructor. Construct an empty instance.

*/

    ~SortLocalInfo() { if (ptr) delete ptr; }
/*
The destructor. Frees the sort algorithm object.

*/

    SortAlgorithm * ptr;
/*
Pointer to sort algorithm

*/
};

/*
5 Value mapping function of ~sort2~, ~sortby2~ operator

This value mapping function is also used for operators ~sort2with~
and ~sortby2with~. The template parameter ~firstArg~ specifies the
argument index of the first argument that belongs to the sort order
specification. For both operators ~sort2~ and ~sortby2~ the
corresponding type mapping function appends a sort order specification
to the argument list. The only difference is the argument index of the
first sort order specification argument, as ~sort2~ has no additional
arguments besides the stream. ~sortby2~ has a nested list of sort order
arguments as second argument. The template parameter ~param~ is used
to specify whether the operator receives two additional arguments. The
first one is the operators main memory size in KBytes and the second
one the maximum fan-in for a merge phase.

*/
  template<int firstArg, bool param>
  int SortValueMap( Word* args, Word& result,
                     int message, Word& local, Supplier s );

}

#endif /* SORT_H_ */
