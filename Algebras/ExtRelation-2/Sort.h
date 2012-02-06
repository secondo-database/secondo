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

November 2009, Sven Jungnickel. Changed default value for
default fan-in merge phase to experimentally determined
value of 65.

2 Overview

This file contains the declaration of all classes and functions that
implement the new sort algorithm for operators ~sort2~ and ~sortby2~
and their parameter driven versions ~sort2Param~ and ~sortby2Param~.

3 Includes and defines

*/

#ifndef SORT_H_
#define SORT_H_

#include <limits.h>
#include "RelationAlgebra.h"
#include "Progress.h"
#include "TupleQueue.h"
#include "TupleBuffer2.h"
#include "StopWatch.h"

/*
Operators ~sort2Param~ and ~sortby2Param~ allow to specify the maximum
fan-in for a merge phase. The usable fan-in is limited between a
minimum and maximum value which is specified here. If these limits
are exceeded the fan-in is set to a default value.

*/
#define SORT_MINIMUM_FAN_IN       2
#define SORT_MAXIMUM_FAN_IN       1000
#define SORT_DEFAULT_MAX_FAN_IN   65

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
4 Class ~SortedRunInfo~

Class ~SortedRunInfo~ is used to collect trace data
for a single run during run creation.

All classes and functions have been put into the namespace
~extrel2~ to avoid name conflicts with existing implementations and
to make it later easier to replace existing operators.

*/
namespace extrel2
{
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
5 Class ~SortInfo~

Class ~SortInfo~ is used to collect data during the usage of
the sort operator. The class provides a method for formatted
information output to a string stream. This class is used to
collect the necessary information to do benchmarking between
the different implementations of the sort operator.

*/

class SortInfo {
public:

  SortInfo(size_t bufferSize, size_t ioBufferSize);
/*
The constructor. Constructs a ~SortInfo~ instance and immediately
sets the operators main memory to ~bufferSize~. I/O buffer size
is set to ~ioBufferSize~.

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

  int BufferSize;
/*
Total memory usable by Operator (bytes)

*/

  int IOBufferSize;
/*
I/O Buffer size for writing to disk (bytes)

*/

  int MaxMergeFanIn;
/*
Maximum merge fan-in

*/

  int F0;
/*
Merge fan-in for first intermediate merge phase

*/

  int W;
/*
Number of intermediate merge phases after F0

*/

  int IntermediateTuples;
/*
Number of intermediate tuples to process

*/

  int InitialRunsCount;
/*
Number of initial runs

*/

  int TotalTupleCount;
/*
Total number of processed tuples

*/

  int TotalTupleSize;
/*
Total size of processed tuples (bytes)

*/

  int MinTupleSize;
/*
Minimum tuple size (bytes)

*/

  int MaxTupleSize;
/*
Maximum tuple size (bytes)

*/


  float AvgTupleSize;
/*
Average tuple size (bytes)

*/

  size_t TotalComparisons;
/*
Total number of tuple comparisons during sorting.

*/

  StopWatch StopWatchTotal;
/*
Timer for total sort time

*/

  StopWatch StopWatchMerge;
/*
Timer for merge phase

*/

  string TimeRunPhase;
/*
Used time for building initial runs

*/

  string TimeMergePhase;
/*
Used time for merge phase

*/

  string TimeTotal;
/*
Total time used by sort operation

*/

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
Overloaded operator << of ~SortInfo~. Used for tracing.

*/

/*
6 Class SortedRun

Class ~SortedRun~ represents a sorted run which is created during
run generation. The class is able to store tuples in sorted order in memory an
on disk. For storage in memory a priority queue is used.
For temporary disk storage an instance of class ~TupleBuffer2~
is used. The class offers various methods for sorting and pushing
tuples to disk in sort order. This class is used by sort algorithm
to control run generation.

*/
class SortedRun
{
public:

  SortedRun( int runNumber,
              int attributes,
              const SortOrderSpecification& spec,
              size_t ioBufferSize );
/*
The constructor. Construct a new sorted run with run number ~runNumber~.
The number of tuple attributes is passed in ~attributes~. The sort order
specification ~spec~ defines how the tuple comparison is performed.
For read/write operations on disk the operator uses an I/O buffer with
~ioBufferSize~ bytes.

*/

  ~SortedRun();
/*
The destructor.

*/

  inline bool IsAtDisk()
  {
    return heap->Empty();
  }
/*
Returns true if all tuples from the internal minimum heap have been
moved to disk. Otherwise the method returns false.

*/

  inline bool IsInSortOrder(Tuple* t)
  {
    // Compare object returns true if ordering is ascendant
    return !(heap->Compare(lastTuple.tuple, t));
  }
/*
Returns true if tuple ~t~ is in sort order according to the last tuple
written to disk. Otherwise the method returns false.

*/

  inline bool IsFirstTupleOnDisk()
  {
    return ( lastTuple.tuple == 0 );
  }
/*
Returns true if the next tuple that is appended to the run
is the first tuple written to disk. Otherwise the method returns false.

*/

  inline void AppendToDisk()
  {
    lastTuple = heap->Top()->GetTuple();
    buffer.AppendTuple( lastTuple.tuple );
    lastTuple.tuple->DeleteIfAllowed();
    heap->Pop();

    if ( traceMode )
    {
      info->TuplesInMemory--;
      info->TuplesOnDisk++;
    }
  }
/*
Appends the minimum tuple from the internal heap to disk.

*/

  inline void AppendToDisk(Tuple* t)
  {
    heap->Push(t);
    lastTuple = heap->Top()->GetTuple();
    buffer.AppendTuple( lastTuple.tuple );
    lastTuple.tuple->DeleteIfAllowed();
    heap->Pop();

    size_t s = t->GetSize();
    tupleCount++;
    runLength += s;
    minTupleSize = s < minTupleSize ? s : minTupleSize;
    maxTupleSize = s > maxTupleSize ? s : maxTupleSize;

    if ( traceMode )
    {
      info->RunLength++;
      info->RunSize += s;
      info->TuplesOnDisk++;
      info->AdditionalRunLength++;
    }
  }
/*
Inserts tuple ~t~ into the minimum heap, takes the
minimum tuple from the heap and appends it to disk.

*/

  inline void AppendToMemory(Tuple* t)
  {
    heap->Push(t);

    size_t s = t->GetSize();
    tupleCount++;
    runLength += s;
    minTupleSize = s < minTupleSize ? s : minTupleSize;
    maxTupleSize = s > maxTupleSize ? s : maxTupleSize;

    if ( traceMode )
    {
      info->RunLength++;
      info->RunSize += s;
      info->MinimumRunLength++;
      info->TuplesInMemory++;
    }
  }
/*
Inserts tuple ~t~ into the minimum heap in memory.

*/

  inline Tuple* GetNextTuple()
  {
    Tuple* t = 0;

    // Free reference to last tuple saved on disk
    if ( lastTuple.tuple != 0 )
    {
      lastTuple.setTuple(0);
    }

    if ( buffer.GetNoTuples() > 0 )
    {
      // tuples on disk
      if ( iter == 0 )
      {
        iter = buffer.MakeScan();
      }

      t = iter->GetNextTuple();

      if ( t == 0 )
      {
        // tuples on disk finished, proceed with tuples in memory
        if ( !heap->Empty() )
        {
          t = heap->Top()->GetTuple();
          heap->Pop();
        }
      }
    }
    else
    {
      // all tuples in memory
      if ( !heap->Empty() )
      {
        t = heap->Top()->GetTuple();
        heap->Pop();
      }
    }

    return t;
  }

/*
Returns the next tuple in sort order from the run. The sequential scan
starts with the tuples located on disk. After the tuple buffer has been
processed the remaining tuples in memory are scanned. After the scan
the internal heap will be empty.

*/

  inline void RunFinished()
  {
    buffer.CloseDiskBuffer();
  }
/*
Signals that the run creation is finished. This method performs some
cleanup operations, like closing the disk buffer.

*/

  inline int GetRunNumber()
  {
    return runNumber;
  }
/*
Returns the run number

*/

  inline int GetRunLength()
  {
    return runLength;
  }
/*
Returns the run length in bytes

*/

  inline size_t GetTupleCount()
  {
    return tupleCount;
  }
/*
Returns the total number of tuples in the run

*/

  inline size_t GetMinimumTupleSize()
  {
    return minTupleSize;
  }
/*
Returns the minimum tuple size of all tuple in the run

*/

  inline size_t GetMaximumTupleSize()
  {
    return maxTupleSize;
  }
/*
Returns the maximum tuple size of all tuple in the run

*/

  inline double GetAverageTupleSize()
  {
    return (double)runLength / (double)tupleCount;
  }
/*
Returns the average tuple size of all tuple in the run

*/

  inline extrel2::SortedRunInfo* Info()
  {
    return info;
  }
/*
Returns the ~SortedRunInfo~ instance if ~traceMode~ is true.
Otherwise 0 is returned.

*/

  inline extrel2::SortedRunInfo* InfoCopy()
  {
    if ( info )
    {
      return new SortedRunInfo(*info);
    }
    else
    {
      return 0;
    }
  }
/*
Returns a copy of the ~SortedRunInfo~ instance if ~traceMode~ is true.
Otherwise 0 is returned.

*/

private:

  int runNumber;
/*
Run number

*/

  RTuple lastTuple;
/*
Reference to last tuple written to disk

*/

  size_t runLength;
/*
Run length in Bytes

*/

  size_t tupleCount;
/*
Number of tuples in run

*/

  size_t minTupleSize;
/*
Minimum tuple size

*/

  size_t maxTupleSize;
/*
Maximum tuple size

*/

  extrel2::TupleQueue* heap;
/*
Minimum Heap for internal sorting

*/

  extrel2::TupleBuffer2 buffer;
/*
Tuple Buffer

*/

  extrel2::TupleBuffer2Iterator* iter;
/*
Iterator for scan

*/

  extrel2::SortedRunInfo* info;
/*
Statistical information

*/

  bool traceMode;
/*
Set Flag to true to enable trace mode

*/
};

ostream& operator<<(ostream& os, SortedRun& run);
/*
Overloaded operator << of ~SortedRun~. Used for for debugging purposes.

*/

/*
7 Functional comparison classes

7.1 Class SortedRunCompareNumber

Derived functional STL class for lesser comparison of two ~SortedRun~
instances according to their run number.

*/
class SortedRunCompareNumber :
  public binary_function<SortedRun*, SortedRun*, bool >
{
  public:

  inline bool operator()(SortedRun* x, SortedRun* y)
  {
    return ( x->GetRunNumber() < y->GetRunNumber() );
  }
};

/*
7.2 Class SortedRunCompareLengthLesser

Derived functional STL class for lesser comparison of two ~SortedRun~
instances according to their tuple count.

*/
class SortedRunCompareLengthLesser :
  public binary_function<SortedRun*, SortedRun*, bool >
{
  public:

  inline bool operator()(SortedRun* x, SortedRun* y)
  {
    return ( x->GetTupleCount() < y->GetTupleCount() );
  }
};

/*
7.3 Class SortedRunCompareLengthGreater

Derived functional STL class for greater comparison of two ~SortedRun~
instances according to their tuple count.

*/
class SortedRunCompareLengthGreater :
  public binary_function<SortedRun*, SortedRun*, bool >
{
  public:

  inline bool operator()(SortedRun* x, SortedRun* y)
  {
    return ( x->GetTupleCount() >= y->GetTupleCount() );
  }
};

/*
8 Class ~SortProgressLocalInfo~

This class contains the progress information for the implementation
of the ~sort2~, ~sortby2~, ~sort2Param~ and ~sortby2Param~ operators.
As the new operator implementation need to provide some more progress
information during intermediate merge phases this class has been derived
from ~ProgressLocalInfo~. The class contains two additional counters
that represent the total and current amount of tuples that must
be processed in intermediate merge phases.

*/
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
9 Class ~SortAlgorithm~

This class implements an algorithm for external sorting for operators
~sort2~, ~sortby2~, ~sort2Param~ and ~sortby2Param~. The constructor
creates sorted partitions of the input stream and stores them inside
temporary tuple files and two minimum heaps in memory. By calls of
~NextResultTuple~ tuples are returned in sorted order. The sort order
must be specified in the constructor. The memory usage is bounded,
hence only a fixed number of tuples can be hold in memory.

The algorithm roughly works as follows: First all input tuples are stored in a
minimum heap until no more tuples fit into memory. Then, a new tuple file is
created and the minimum is stored there. Afterwards, the tuples are handled as
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
                   const SortOrderSpecification& spec,
                   SortProgressLocalInfo* p,
                   Supplier s,
                   size_t maxFanIn = UINT_MAX,
                   size_t maxMemSize = UINT_MAX,
                   size_t ioBufferSize = UINT_MAX);
/*
The constructor. Consumes all tuples of the tuple stream ~stream~ immediately
into sorted runs of approximately two times the size of the operators main
memory by making use of the replacement selection algorithm for run generation.
The internal main memory sort routine uses a minimum heap for sorting. For
tuple comparisons the algorithm uses the compare object ~cmpObj~. All
progress information will be stored in ~p~. Additionally the constructor
allows to specify the maximum number of open temporary files during a merge
phase, the so called maximum fan-in ~maxFanIn~. If ~maxFanIn~ is set to
UINT\_MAX the sort algorithm uses the default value
SORT\_DEFAULT\_MAX\_FAN\_IN. The main memory that the operator uses can be
specified by parameter ~maxMemSize~. If ~maxMemSize~
is set to UINT\_MAX the sort algorithm uses the maximum main memory assigned
by the query processor. Parameter ~ioBufferSize~ is used to specify the
I/O buffer size in bytes for read/write operations on disk. If ~ioBufferSize~
is set to UINT\_MAX the sort algorithm uses the systems page size as a default
value.

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

  size_t getUsedMemory(){
     return usedMemory;
  }


  private:

    void setMemory(size_t maxMemory, Supplier s);
/*
Sets the usable main memory for the operator in bytes. If ~maxMemory~
has value ~UINT\_MAX~ the usable main memory is requested from the
query processor which reads it from the operator's node.

*/

    void setIoBuffer(size_t size);
/*
Sets the I/O buffer to ~size~ bytes. If ~size~ has value
~UINT\_MAX~ the default I/O buffer size is used (system's page size).

*/

    void setMaxFanIn(size_t f);
/*
Sets the maximum fan-in for the merge phase to ~f~. The method verifies
that ~f~ lies between 2 and 1000. If ~f~ exceeds these limits the corresponding
boundary is set.

*/

    void mergeInit();
/*
Initializes the merge phase. If the number of runs exceeds ~FMAX~ intermediate
merge phases will be performed. After this method has been called the
minimum heap used for merging tuples will be filled with one tuple from each
run. The number of runs will be limited by ~FMAX~, so that all tuples may be
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

    bool updateIntermediateMergeCost();
/*
Calculates if any intermediate merge phases are necessary. Member variables
~F0~ and ~W~ are updated if the number of runs exceeds ~FMAX~.

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
read/write cost or the sum of two constants (one for write and one for read).

*/

    int FMAX;
/*
Maximum merge fan-in for a merge phase. If the number of sorted runs exceeds
~FMAX~ runs, additional intermediate merge phases have to be performed after
all runs have been created. ~FMAX~ thus limits the number of open temporary
files during the merge phase. The number of open files during a merge phase
is maximal ~FMAX+1~ (~FMAX~ files for the runs and one file for the
merged run).

*/

    int F0;
/*
Merge fan-in of first intermediate merge phase. The fan-in of the first
intermediate merge phase is calculated so that the last merge-phase has
a fan-in of exactly ~FMAX~ runs. ~N~ is the number of runs ($N > FMAX$).
$F_0=((N-FMAX-1) mod (FMAX-1))+2$

*/

    int W;
/*
Additional intermediate merge phases with fan-in ~FMAX~ after ~F0~ merge
phase. (~W~ does not include the final merge phase!).
~N~ is the number of runs ($N > FMAX$). $W=\frac{N-(F_0-1)-FMAX}{FMAX-1}$

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

    int attributes;
/*
Number of attributes of a tuple in the tuple stream. This value is determined
when the first tuple has been read. This information is needed by class
~TupleQueue~ to automatically determine whether the the sort order description
is lexicographical (asc/desc). In this case some performance improvements can
be made during tuple comparison.

*/

    const SortOrderSpecification& spec;
/*
Sort order specification for tuple comparison.

*/

    size_t MAX_MEMORY;
/*
Maximum memory available for the sort operation [bytes]

*/

    size_t IO_BUFFER_SIZE;
/*
I/O buffer size in bytes for read/write operations on disk.

*/

    size_t usedMemory;
/*
Used memory in bytes

*/

    vector<SortedRun*> runs;
/*
Array which contains the references to the external runs located on disk

*/

    TupleQueue* mergeQueue;
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
10 Class ~SortLocalInfo~

An instance of this class is used in the value mapping function of all
sort operators to save the state of the sort algorithm between multiple
message calls. This class simplifies access to the progress information.
A pointer to this instance of type ~SortProgressLocalInfo~ will be passed
to the ~SortAlgorithm~ object ~ptr~.

*/
class SortLocalInfo: public SortProgressLocalInfo
{
  public:

    SortLocalInfo()
    : SortProgressLocalInfo()
    , ptr(0)
    {}
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
11 Value mapping function of ~sort2~ and ~sortby2~ operator

This value mapping function is also used for operators ~sort2Param~
and ~sortby2Parm~. The template parameter ~firstArg~ specifies the
argument index of the first argument that belongs to the sort order
specification. For both operators ~sort2~ and ~sortby2~ the
corresponding type mapping function appends a sort order specification
to the argument list. The only difference is the argument index of the
first sort order specification argument, as ~sort2~ has no additional
arguments besides the stream. ~sortby2~ has a nested list of sort order
arguments as second argument. The template parameter ~param~ is used
to specify whether the operator receives three additional arguments. The
first one is the operators main memory size in KBytes, the second
one the maximum fan-in for a merge phase and the third one the I/O
buffer size for read/write operations to disc. ~param~ is set to true
for operators ~sort2Param~ and ~sortby2Param~.

*/
  template<int firstArg, bool param>
  int SortValueMap( Word* args, Word& result,
                     int message, Word& local, Supplier s );

} // end of namespace extrel2

#endif /* SORT_H_ */
