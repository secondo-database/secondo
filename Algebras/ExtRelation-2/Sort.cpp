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

1 Implementation File Sort.cpp

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#include <algorithm>
#include "stdlib.h"

#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTuple.h"
#include "Sort.h"

#define HLINE "---------------------------------------------------------------"

/*
3 External linking

*/
extern QueryProcessor* qp;

/*
4 Implementation of class ~SortedRunInfo~

*/
namespace extrel2
{

SortedRunInfo::SortedRunInfo(int no)
: No(no)
, RunLength(0)
, RunSize(0)
, MinimumRunLength(0)
, AdditionalRunLength(0)
, TuplesPassedToNextRun(0)
, TuplesInMemory(0)
, TuplesOnDisk(0)
, RunRatio(0)
{
}

SortedRunInfo::SortedRunInfo(extrel2::SortedRunInfo& info)
{
  No = info.No;
  RunLength = info.RunLength;
  RunSize = info.RunSize;
  MinimumRunLength = info.MinimumRunLength;
  AdditionalRunLength = info.AdditionalRunLength;
  TuplesPassedToNextRun = info.TuplesPassedToNextRun;
  TuplesInMemory = info.TuplesInMemory;
  TuplesOnDisk = info.TuplesOnDisk;
  RunRatio = info.RunRatio;
}

SortedRunInfo::~SortedRunInfo()
{
}

float SortedRunInfo::Ratio()
{
  return (float)RunLength / (float)MinimumRunLength;
}

float SortedRunInfo::Size()
{
  return (float)RunSize / (float)( 1024 * 1024 );
}

ostream& operator<<(ostream& stream, SortedRunInfo& info)
{
  stream << setw(2) << info.No << ": " << info.RunLength
         << " Tuples / " << info.Size() << " MB "
         << "(Minimum: " << info.MinimumRunLength
         << ", Added: " << info.AdditionalRunLength << ")"
         << " => Ratio: " << info.Ratio() << endl
         << "    TuplesPassedToNextRun: " << info.TuplesPassedToNextRun
         << " TuplesInMemory: " << info.TuplesInMemory
         << " TuplesOnDisk: " << info.TuplesOnDisk << endl;

  return stream;
}


/*
4 Implementation of class ~SortInfo~

*/

SortInfo::SortInfo(size_t bufferSize, size_t ioBufferSize) :
  BufferSize(bufferSize),
  IOBufferSize(ioBufferSize),
  MaxMergeFanIn(0),
  F0(0),
  W(0),
  IntermediateTuples(0),
  InitialRunsCount(0),
  TotalTupleCount(0),
  TotalTupleSize(0),
  MinTupleSize(INT_MAX),
  MaxTupleSize(0),
  AvgTupleSize(0),
  TotalComparisons(0),
  TimeRunPhase(""),
  TimeMergePhase(""),
  TimeTotal(""),
  RunStatistics(true)
{
}

SortInfo::~SortInfo()
{
  clearAll();
}

SortedRunInfo* SortInfo::CurrentRun()
{
  return this->InitialRunInfo[this->InitialRunsCount-1];
}

void SortInfo::NewRun()
{
  this->InitialRunsCount++;
  this->InitialRunInfo.push_back(new SortedRunInfo(this->InitialRunsCount));
}

void SortInfo::clearAll()
{
  BufferSize = 0;
  IOBufferSize = 0;
  MaxMergeFanIn = 0;
  F0 = 0;
  W = 0;
  IntermediateTuples = 0;
  InitialRunsCount = 0;
  TotalTupleCount = 0;
  TotalTupleSize = 0;
  MinTupleSize = INT_MAX;
  MaxTupleSize = 0;
  AvgTupleSize = 0;
  TotalComparisons = 0;
  TimeRunPhase = "";
  TimeMergePhase = "";
  TimeTotal = "";

  vector<SortedRunInfo*>::iterator iter;

  for( iter = InitialRunInfo.begin(); iter != InitialRunInfo.end(); iter++ )
  {
    delete *iter;
  }
  InitialRunInfo.clear();

  for( iter = FinalRunInfo.begin(); iter != FinalRunInfo.end(); iter++ )
  {
    delete *iter;
  }
  FinalRunInfo.clear();
}

void SortInfo::UpdateStatistics(size_t s)
{
  int n = (int)s;
  this->TotalTupleCount++;
  this->TotalTupleSize += n;
  this->MinTupleSize = n < this->MinTupleSize ? n : this->MinTupleSize;
  this->MaxTupleSize = n > this->MaxTupleSize ? n : this->MaxTupleSize;
}

void SortInfo::RunBuildPhase()
{
  this->StopWatchTotal.start();
}

void SortInfo::MergePhase()
{
  this->AvgTupleSize += (float)this->TotalTupleSize /
                        (float)this->TotalTupleCount;
  this->TimeRunPhase = StopWatchTotal.diffTimes();
  this->StopWatchMerge.start();
}

void SortInfo::Finished()
{
  this->TimeMergePhase = StopWatchMerge.diffTimes();
  this->TimeTotal = StopWatchTotal.diffTimes();
}

ostream& operator<<(ostream& stream, SortInfo& info)
{
  float totalTupleSizeMB = (float)info.TotalTupleSize /
                           (float)(1024.0 * 1024.0);

  stream << endl
         << HLINE << endl
         << "Sort-Operation Statistics" << endl
         << HLINE << endl
         << "BufferSize: \t\t" << info.BufferSize << " ( "
         << info.BufferSize/1024 << " KByte)" << endl
         << "I/O-BufferSize: \t" << info.IOBufferSize << " ( "
         << info.IOBufferSize/1024 << " KByte)" << endl
         << "MaxMergeFanIn: \t\t" << info.MaxMergeFanIn << endl
         << "FirstMergeFanIn: \t" << info.F0 << endl
         << "Intermediate Merges: \t" << info.W+1 << endl
         << "Intermediate Tuples: \t" << info.IntermediateTuples << endl
         << "InitialRunsCount: \t" << info.InitialRunsCount << endl
         << "TotalTupleCount: \t" << info.TotalTupleCount << endl
         << "TotalSize: \t\t" << info.TotalTupleSize << " Byte ( "
         << totalTupleSizeMB << " MByte)" << endl
         << "MinTupleSize: \t\t" << info.MinTupleSize << " Byte" << endl
         << "MaxTupleSize: \t\t" << info.MaxTupleSize << " Byte" << endl
         << "AvgTupleSize: \t\t" << info.AvgTupleSize << " Byte" << endl
         << "TotalComparisons: \t" << info.TotalComparisons << endl
         << "TimeRunPhase: \t\t" << info.TimeRunPhase << endl
         << "TimeMergePhase: \t" << info.TimeMergePhase << endl
         << "TimeTotal: \t\t" << info.TimeTotal << endl;

  if ( info.RunStatistics == true )
  {

  stream << HLINE << endl
         << "Initial run statistics (1-" << info.InitialRunsCount << ")" << endl
         << HLINE << endl;

          for (size_t i = 0; i < info.InitialRunInfo.size(); i++)
          {
            stream << *(info.InitialRunInfo[i]);
          }
  }

  if ( info.RunStatistics == true && !info.FinalRunInfo.empty() )
  {

  stream  << HLINE << endl
          << "Final run statistics" << endl
          << HLINE << endl;

           for (size_t i = 0; i < info.FinalRunInfo.size(); i++)
           {
             stream << *(info.FinalRunInfo[i]);
           }
  }

  stream << endl;

  return stream;
}

ostream& operator<<(ostream& os, SortedRun& run)
{
  Tuple* t;

  os << "-------------------- Run "
     << run.GetRunNumber()
     << " -----------------" << endl;

  while ( ( t = run.GetNextTuple() ) != 0 )
  {
    os << *t << endl;
  }

  return os;
}

/*
5 Implementation of class ~SortedRun~

*/
SortedRun::SortedRun( int runNumber,
                        int attributes,
                        const SortOrderSpecification& spec,
                        size_t ioBufferSize )
: runNumber(runNumber)
, runLength(0)
, tupleCount(0)
, minTupleSize(UINT_MAX)
, maxTupleSize(0)
, buffer(0, ioBufferSize)
, iter(0)
, traceMode(RTFlag::isActive("ERA:TraceSort"))
{
  heap = new TupleQueue(spec, attributes);

  if( traceMode )
  {
    info = new SortedRunInfo(runNumber);
  }
}

SortedRun::~SortedRun()
{
  if ( iter != 0 )
  {
    delete iter;
    iter = 0;
  }

  if ( traceMode )
  {
    // delete info;
    // Delete is handled in SortInfo Destructor
    info = 0;
  }

  if ( !heap->Empty() )
  {
    std::vector<TupleQueueEntry*>& container = heap->GetContainer();

    for(size_t i = 0; i < container.size(); i++)
    {
      container[i]->GetTuple()->DeleteIfAllowed();
    }
  }

  delete heap;
  heap = 0;

  lastTuple.setTuple(0);
}

/*
6 Implementation of class ~SortProgressLocalInfo~

*/
SortProgressLocalInfo::SortProgressLocalInfo()
: ProgressLocalInfo()
, intTuplesTotal(0)
, intTuplesProc(0)
{
}

/*
7 Implementation of class ~SortAlgorithm~

*/
SortAlgorithm::SortAlgorithm( Word stream,
                              const SortOrderSpecification& spec,
                              SortProgressLocalInfo* p,
                              Supplier s,
                              size_t maxFanIn,
                              size_t maxMemSize,
                              size_t ioBufferSize)
: F0(0)
, W(0)
, nextRunNumber(1)
, checkProgressAfter(10)
, stream(stream)
, attributes(-1)
, spec(spec)
//, cmpObj(cmpObj)
, usedMemory(0)
, traceMode(RTFlag::isActive("ERA:TraceSort"))
, traceModeExtended(RTFlag::isActive("ERA:TraceSortExtended"))
, progress(p)
{
  SortedRun *curRun = 0, *nextRun = 0;

  Word wTuple(Address(0));

  // Check specified fan-in for a merge phase
  setMaxFanIn(maxFanIn);

  // Check specified main memory for this operation
  setMemory(maxMemSize, s);

  // Check I/O buffer size for this operation
  setIoBuffer(ioBufferSize);

  if ( traceMode )
  {
    info = new SortInfo(MAX_MEMORY, IO_BUFFER_SIZE);
    info->RunBuildPhase();
    info->MaxMergeFanIn = FMAX;
    TupleQueueCompare::ResetComparisonCounter();
  }

  // Request first tuple and consume the stream completely
  qp->Request(stream.addr, wTuple);

  while( qp->Received(stream.addr) )
  {
    Tuple *t = static_cast<Tuple*>( wTuple.addr );

    progress->read++;

    size_t memSize = t->GetMemSize();

    // Save number of attributes
    if ( attributes == -1 )
    {
      attributes = t->GetNoAttributes();
      curRun = new SortedRun(nextRunNumber++, attributes, spec, IO_BUFFER_SIZE);
      runs.push_back(curRun);
    }

    if ( traceMode )
    {
      info->UpdateStatistics(t->GetSize());
    }

    if( usedMemory <= MAX_MEMORY )
    {
      curRun->AppendToMemory(t);
      usedMemory += memSize;
    }
    else
    {
      if ( curRun->IsFirstTupleOnDisk() )
      {
        // memory is completely used, append to disk
        progress->state = 1;
        curRun->AppendToDisk(t);
      }
      else
      {
        if ( curRun->IsInSortOrder(t) )
        {
          // tuple is on sort order, append to disk
          curRun->AppendToDisk(t);
        }
        else
        {
          if ( traceMode )
          {
            curRun->Info()->TuplesPassedToNextRun++;
          }

          // create next run if necessary
          if ( nextRun == 0 )
          {
            nextRun = new SortedRun( nextRunNumber++, attributes,
                                     spec, IO_BUFFER_SIZE );
            runs.push_back(nextRun);
            updateIntermediateMergeCost();
          }

          // next tuple is smaller, save it for the next relation
          if ( !curRun->IsAtDisk() )
          {
            // push minimum tuple of current run to disk
            curRun->AppendToDisk();

            // append tuple to memory
            nextRun->AppendToMemory(t);
          }
          else
          {
            // finish current run
            curRun->RunFinished();

            // switch runs
            curRun = nextRun;
            nextRun = 0;

            curRun->AppendToDisk(t);
          }
        }
      }
    }

    qp->Request(stream.addr, wTuple);
  }

  if ( traceMode )
  {
    for (size_t i = 0; i < runs.size(); i++)
    {
      info->InitialRunInfo.push_back(runs[i]->InfoCopy());
      info->InitialRunsCount = runs.size();
    }
  }

  mergeInit();
}

/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

SortAlgorithm::~SortAlgorithm()
{
  if ( traceMode )
  {
    info->Finished();
    info->TotalComparisons = TupleQueueCompare::GetComparisonCounter();

    if ( traceModeExtended )
    {
      for (size_t i = 0; i < runs.size(); i++)
      {
        info->FinalRunInfo.push_back(runs[i]->InfoCopy());
      }
    }

    cmsg.info() << *info;
    cmsg.send();

    delete info;
    info = 0;
  }

  while( !mergeQueue->Empty() )
  {
    mergeQueue->Top()->GetTuple()->DeleteIfAllowed();
    mergeQueue->Pop();
  }
  delete mergeQueue;
  mergeQueue = 0;

  // delete information about sorted runs
  while ( !runs.empty() )
  {
    delete (runs.back());
    runs.pop_back();
  }
}

void SortAlgorithm::setMemory(size_t maxMemory, Supplier s)
{
  if ( maxMemory == UINT_MAX )
  {
    MAX_MEMORY = qp->GetMemorySize(s) * 1024 * 1024; // in bytes
  }
  else if ( maxMemory < SORT_MINIMUM_MEMORY )
  {
    MAX_MEMORY = SORT_MINIMUM_MEMORY;
  }
  else if ( maxMemory > SORT_MAXIMUM_MEMORY )
  {
    MAX_MEMORY = SORT_MAXIMUM_MEMORY;
  }
  else
  {
    MAX_MEMORY = maxMemory;
  }
}

void SortAlgorithm::setIoBuffer(size_t size)
{
  if ( size == UINT_MAX && size > 16384 )
  {
    IO_BUFFER_SIZE = WinUnix::getPageSize();
  }
  else
  {
    IO_BUFFER_SIZE = size;
  }
}

void SortAlgorithm::setMaxFanIn(size_t f)
{
  if ( f == UINT_MAX )
  {
    FMAX = SORT_DEFAULT_MAX_FAN_IN;
  }
  else if ( f < SORT_MINIMUM_FAN_IN )
  {
    FMAX = SORT_MINIMUM_FAN_IN;
  }
  else if ( f > SORT_MAXIMUM_FAN_IN )
  {
    FMAX = SORT_MAXIMUM_FAN_IN;
  }
  else
  {
    FMAX = f;
  }
}

bool SortAlgorithm::updateIntermediateMergeCost()
{
  int N = (int)runs.size();

  if ( N > FMAX )
  {
    // calculate merge fan-in for first merge phase so that the
    // following merge phases use the maximum merge fan-in
    this->F0 = ((N-FMAX-1) % (FMAX-1)) + 2;

    // calculate the number of merge phases using the maximum
    // merge fan-in
    this->W = (N-(F0-1)-FMAX) / (FMAX-1);

    if ( traceMode )
    {
      info->F0 = F0;
      info->W = W;
    }

    progress->intTuplesTotal = calcIntermediateMergeCost();

    return true;
  }

  return false;
}

int SortAlgorithm::calcIntermediateMergeCost()
{
  if ( this->F0 > 0 )
  {
    vector<int> runSizes;

    // copy tuple counts of runs to array
    for(size_t i = 0; i < runs.size(); i++)
    {
      runSizes.push_back(runs[i]->GetTupleCount());
    }

    int tuples = 0;

    // Simulate intermediate merge with fan-in F0
    tuples += simulateIntermediateMerge(runSizes, this->F0);

    // Simulate intermediate merge with fan-in FMAX
    for(int i = 0; i < this->W; i++)
    {
      tuples += simulateIntermediateMerge(runSizes, this->FMAX);
    }

    return tuples;
  }

  return 0;
}

int SortAlgorithm::simulateIntermediateMerge(vector<int>& arr, int f)
{
  int cost = 0;

  assert(f <= (int)arr.size());

  // sort run lengths in descending order
  std::sort(arr.begin(), arr.end(), greater<int>());

  // sum up tuples for this phase
  cost = sumLastN(arr, f);

  // cut off f runs
  arr.resize(arr.size() - f);

  // append merged run
  arr.push_back(cost);

  return cost;
}

// sum the last n elements of an integer array up
int SortAlgorithm::sumLastN(vector<int>& arr, int n)
{
  int result = 0;

  assert(n <= (int)arr.size());

  for(size_t i = arr.size() - n; i < arr.size(); i++)
  {
    result += arr[i];
  }

  return result;
}

void SortAlgorithm::mergeInit()
{
  if ( F0 > 0 )
  {
    // final update of intermediate merge costs
    progress->intTuplesTotal = calcIntermediateMergeCost();
  }

  if ( traceMode )
  {
    info->MergePhase();
    info->IntermediateTuples = progress->intTuplesTotal;
  }

  // create heap for merging runs
  mergeQueue = new TupleQueue(spec, attributes);

  // Check if we need intermediate merge phases
  if ( F0 > 0 )
  {
    // Perform first intermediate merge phase with fan-in F0
    mergeNShortest(F0);

    // Perform follow-up intermediate merge phases with fan-in FMAX
    for (int i = 0; i < W; i++)
    {
      mergeNShortest(FMAX);
    }
  }

  // Get first tuple from each run and push it into the merge heap
  for( size_t i = 0; i < runs.size(); i++ )
  {
    Tuple *t = runs[i]->GetNextTuple();

    if( t != 0 )
    {
      mergeQueue->Push(t,i);
    }
  }
}

void SortAlgorithm::mergeNShortest(int n)
{
  assert(n>1);
  assert((int)runs.size() > 1);
  assert((int)runs.size() >= n);

  Tuple* t;
  int counter = 0;

  SortedRunCompareLengthGreater comp;

  // sort the runs according to their length in bytes
  // a minimum heap is created
  make_heap(runs.begin(), runs.end(), comp);

  // get the n shortest runs
  vector<SortedRun*> merge;

  for (int i = 0; i < n; i++)
  {
    // pops first array element and moves it to the end of the array
    // the heap condition is fullfilled afterwards
    pop_heap(runs.begin(), runs.end(), comp);

    // add the popped run to the merge array
    merge.push_back(runs.back());

    // delete run from runs array
    runs.pop_back();
  }

  // create the sorted run for merging
  SortedRun* result = new SortedRun( nextRunNumber++, attributes,
                                     spec, IO_BUFFER_SIZE );

  // append the sorted run to the end of the runs array
  runs.push_back(result);

  if ( traceModeExtended )
  {
    cmsg.info() << HLINE << endl
                << "Intermediate merge phase into Run "
                << result->GetRunNumber() << " with "
                << n << " runs." << endl;
  }

  // Get first tuple from each run and push it into the merge heap.
  for( size_t i = 0; i < merge.size(); i++ )
  {
    if( ( t = merge[i]->GetNextTuple() ) != 0 )
    {
      mergeQueue->Push(t,i);
    }

    if ( traceModeExtended )
    {
      cmsg.info() << merge[i]->GetRunNumber() << ": "
                  << "RunLength: " << merge[i]->GetRunLength() / 1024
                  << " (KBytes), Tuples: " << merge[i]->GetTupleCount() << endl;
    }
  }

  // merge runs
  while ( ( t = this->nextResultTuple(merge) ) != 0 )
  {
    result->AppendToDisk(t);

    progress->intTuplesProc++;

    // Check after a certain amount of tuples if a progress message is necessary
    // If we don't do this progress will freeze, because the query processors
    // eval method isn't called
    if ( ( counter++ % checkProgressAfter ) == 0)
    {
      qp->CheckProgress();
    }
  }

  // delete merged runs
  for ( size_t i = 0; i < merge.size(); i++)
  {
    if ( traceModeExtended )
    {
      cmsg.info() << "Deleting Run " << merge[i]->GetRunNumber() << endl;
      cmsg.send();
    }

    delete merge[i];
  }
  merge.clear();

  return;
}

Tuple* SortAlgorithm::NextResultTuple()
{
  return nextResultTuple(runs);
}

Tuple* SortAlgorithm::nextResultTuple(vector<SortedRun*>& arr)
{
  if( mergeQueue->Empty() ) // stream finished
  {
    return 0;
  }
  else
  {
    // Take the next tuple from the merge queue and set
    // it as the result tuple
    Tuple* result = mergeQueue->Top()->GetTuple();
    int pos = mergeQueue->Top()->GetPosition();
    mergeQueue->Pop();

    // Refill the merge queue with a tuple from result
    // tuple's queue if there are more
    Tuple* t = arr[pos]->GetNextTuple();

    // Push the found tuple into the merge queue
    if( t != 0 )
    {
      // run not finished
      mergeQueue->Push(t, pos);
    }

    return result;
  }
}

/*
8 Implementation of value mapping function of operator ~sort2~ and ~sortby2~

*/

template<int firstArg, bool param>
int SortValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool traceProgress = false;

  // Operator sort2 (firstArg = 1, param = false)
  // args[0] : stream
  // args[1] : the number of sort attributes
  // args[3] : the index of the first sort attribute
  // args[4] : a boolean which indicates if sortorder should
  //           be asc (true) or desc (false)
  // args[5] : Same as 3 but for the second sort attribute
  // args[6] : Same as 4 but for the second sort attribute
  // ....

  // Operator sort2Param (firstArg = 4, param = true)
  // args[0] : stream
  // args[1] : operator's main memory in bytes
  // args[2] : maximum fan-in of merge phase
  // args[3] : I/O buffer size in bytes
  // args[4] : the number of sort attributes
  // args[5] : the index of the first sort attribute
  // args[6] : a boolean which indicates if sortorder should
  //           be asc (true) or desc (false)
  // args[7] : Same as 3 but for the second sort attribute
  // args[8] : Same as 4 but for the second sort attribute
  // ....

  // Operator sortby2 (firstArg = 2, param = false)
  // args[0] : stream
  // args[1] : sort attributes specification as list (ignored)
  // args[2] : the number of sort attributes
  // args[3] : the index of the first sort attribute
  // args[4] : a boolean which indicates if sortorder should
  //           be asc (true) or desc (false)
  // args[5] : Same as 3 but for the second sort attribute
  // args[6] : Same as 4 but for the second sort attribute
  // ....

  // Operator sortby2Param (firstArg = 5, param = true)
  // args[0] : stream
  // args[1] : sort attributes specification as list (ignored)
  // args[2] : operator's main memory in bytes
  // args[3] : maximum fan-in of merge phase
  // args[4] : I/O buffer size in bytes
  // args[5] : the number of sort attributes
  // args[6] : the index of the first sort attribute
  // args[7] : a boolean which indicates if sortorder should
  //           be asc (true) or desc (false)
  // args[8] : Same as 3 but for the second sort attribute
  // args[9] : Same as 4 but for the second sort attribute
  // ....

  SortLocalInfo* li;
  li = static_cast<SortLocalInfo*>( local.addr );

  switch(message)
  {
    case OPEN:
    {
      if (li)
      {
        delete li;
      }

      li = new SortLocalInfo();
      local.addr = li;

      // at this point the local value is well defined
      // afterwards progress request calls are
      // allowed.

      li->ptr = 0;

      qp->Open(args[0].addr);

      return 0;
    }

    case REQUEST:
    {
      if ( li->ptr == 0 )
      {
        SortOrderSpecification spec;
        int nAttrCount = StdTypes::GetInt( args[firstArg] );
        for(int i = 1; i <= nAttrCount; i++)
        {
          int j = firstArg + (2*i - 1);
          int attrIndex = StdTypes::GetInt( args[j] );
          bool isAsc = StdTypes::GetBool( args[j+1] );
          spec.push_back(pair<int, bool>(attrIndex, isAsc));
        };

        //Sorting is done in the following constructor. It was moved from
        //OPEN to REQUEST to avoid long delays in the OPEN method, which are
        //a problem for progress estimation
        if ( param )
        {
          int idx = firstArg - 3;
          int maxMemSize = StdTypes::GetInt( args[idx] );
          int maxFanIn = StdTypes::GetInt( args[idx+1] );
          int ioBufferSize = StdTypes::GetInt( args[idx+2] );

          size_t fan = maxFanIn < 0 ? UINT_MAX : (size_t)maxFanIn;
          size_t mem = maxMemSize < 0 ? UINT_MAX : (size_t)maxMemSize;
          size_t buf = ioBufferSize < 0 ? UINT_MAX : (size_t)ioBufferSize;

          li->ptr = new SortAlgorithm(args[0], spec, li, s, fan, mem, buf);
        }
        else
        {
          li->ptr = new SortAlgorithm(args[0], spec, li, s);
        }
      }

      SortAlgorithm* sa = li->ptr;

      result.setAddr( sa->NextResultTuple() );
      li->returned++;
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      // Note: object deletion is handled in OPEN and CLOSEPROGRESS
      qp->Close(args[0].addr);
      return 0;
    }


    case CLOSEPROGRESS:
    {
      if (li)
      {
        delete li;
        local.addr = 0;
      }
      return 0;
    }

    case REQUESTPROGRESS:
    {
      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uSortBy = 0.0000873; //millisecs per byte input and sort
	// old constant 0.000396;   
      const double vSortBy = 0.0000243;  //millisecs per byte output
        // old constant 0.000194;  
      const double oSortBy = 0.00004;   //offset due to writing to disk
            //not yet measurable

      // constants determined in file ExtRelation-C++/ConstantsExtendStream



      pRes = (ProgressInfo*) result.addr;

      if( !li )
      {
        return CANCEL;
      }
      else
      {
        if (qp->RequestProgress(args[0].addr, &p1))
        {
          // -------------------------------------------
          // Result cardinality
          // -------------------------------------------

          pRes->Card = li->returned == 0 ? p1.Card : li->read;
          pRes->CopySizes(p1);

          // -------------------------------------------
          // Total time
          // -------------------------------------------

          pRes->Time = p1.Time
            + pRes->Card * p1.SizeExt * (uSortBy + oSortBy * li->state)
            + pRes->Card * p1.SizeExt * vSortBy
            + li->intTuplesTotal * p1.Size * (2 * vSortBy);

          // -------------------------------------------
          // Total progress
          // -------------------------------------------

          pRes->Progress =
            (p1.Progress * p1.Time
             + li->read * p1.SizeExt * (uSortBy + oSortBy * li->state)
             + li->returned * p1.SizeExt * vSortBy
             + li->intTuplesProc * p1.SizeExt * (2 * vSortBy) )
            / pRes->Time;

          // -------------------------------------------
          // Blocking time
          // -------------------------------------------

          // Estimated time until first result tuple is ready
          pRes->BTime = p1.Time
            + pRes->Card * p1.SizeExt * (uSortBy + oSortBy * li->state)
            + li->intTuplesTotal * p1.SizeExt * (2 * vSortBy);

          // -------------------------------------------
          // Blocking progress
          // -------------------------------------------

          pRes->BProgress =
            (p1.Progress * p1.Time
                + li->read * p1.SizeExt * (uSortBy + oSortBy * li->state)
                + li->intTuplesProc * p1.SizeExt * (2 * vSortBy))
                / pRes->BTime;

          if ( traceProgress )
          {
            cmsg.info() << "p1->Card: " << p1.Card
                        << ", p1.Time: " << p1.Time
                        << ", p1.Progress: " << p1.Progress
                        << endl
                        << "pRes->Time: " << pRes->Time
                        << ", pRes->Progress: " << pRes->Progress
                        << endl
                        << ", intTuples: " << li->intTuplesProc
                        << "/" << li->intTuplesTotal
                        << endl;
            cmsg.send();
          }

          return YIELD;
        }
        else
        {
          return CANCEL;
        }
      }
    }
  }
  return 0;
}

/*
9 Instantiation of Template Functions

For some reasons the compiler cannot expand these template functions in
the file ~ExtRelation2Algebra.cpp~, thus the value mapping functions
are instantiated here.

*/

template
int SortValueMap<1, false>( Word* args, Word& result,
                             int message, Word& local, Supplier s );

template
int SortValueMap<2, false >( Word* args, Word& result,
                              int message, Word& local, Supplier s );

template
int SortValueMap<4, true>( Word* args, Word& result,
                            int message, Word& local, Supplier s );

template
int SortValueMap<5, true>( Word* args, Word& result,
                            int message, Word& local, Supplier s );

} // end of namespace extrel2
