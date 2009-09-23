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
4 Implementation of class ~SortProgressLocalInfo~

*/
namespace extrel2
{
SortProgressLocalInfo::SortProgressLocalInfo()
: ProgressLocalInfo()
, intTuplesTotal(0)
, intTuplesProc(0)
{
}

/*
4 Implementation of class ~SortAlgorithm~

*/
SortAlgorithm::SortAlgorithm( Word stream,
                              TupleCompareBy* cmpObj,
                              SortProgressLocalInfo* p,
                              int maxFanIn,
                              size_t maxMemSize )
: F0(0)
, W(0)
, nextRunNumber(1)
, checkProgressAfter(10)
, stream(stream)
, cmpObj(cmpObj)
, usedMemory(0)
, traceMode(RTFlag::isActive("ERA:TraceSort"))
, traceModeExtended(RTFlag::isActive("ERA:TraceSortExtended"))
, progress(p)
{
  Word wTuple(Address(0));
  bool intermediate = false;

  // Check specified fan-in for a merge phase
  if ( maxFanIn < SORT_MINIMUM_FAN_IN )
  {
    FMAX = SORT_MINIMUM_FAN_IN;
  }
  else if ( maxFanIn > SORT_MAXIMUM_FAN_IN )
  {
    FMAX = SORT_MAXIMUM_FAN_IN;
  }
  else
  {
    FMAX = maxFanIn;
  }

  // Check specified main memory for this operation
  if ( maxMemSize == UINT_MAX )
  {
    MAX_MEMORY = qp->MemoryAvailableForOperator();
  }
  else if ( maxMemSize < SORT_MINIMUM_MEMORY )
  {
    MAX_MEMORY = SORT_MINIMUM_MEMORY;
  }
  else if ( maxMemSize > SORT_MAXIMUM_MEMORY )
  {
    MAX_MEMORY = SORT_MAXIMUM_MEMORY;
  }
  else
  {
    MAX_MEMORY = maxMemSize;
  }

  if ( traceMode )
  {
    info = new SortInfo(MAX_MEMORY);
    info->RunBuildPhase();
    info->MaxMergeFanIn = FMAX;
    TupleCompare::ResetComparisonCounter();
  }

  // Create current run
  SortedRun* curRun = new SortedRun(nextRunNumber++, cmpObj);
  SortedRun* nextRun = 0;
  runs.push_back(curRun);

  // Request first tuple and consume the stream completely
  qp->Request(stream.addr, wTuple);

  while( qp->Received(stream.addr) )
  {
    progress->read++;
    Tuple *t = static_cast<Tuple*>( wTuple.addr );
    size_t s = (int)t->GetExtSize();

    if ( traceMode )
    {
      info->UpdateStatistics(s);
    }

    if( usedMemory + s <= MAX_MEMORY )
    {
      curRun->AppendToMemory(t);
      usedMemory += s;
      //cmsg.info() << "Progress->read: " << progress->read
                  //<< "(UsedMem: " << usedMemory << endl;
      //cmsg.send();
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
            nextRun = new SortedRun(nextRunNumber++, cmpObj);
            runs.push_back(nextRun);
            intermediate = updateIntermediateMerges();
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

    // update intermediate merge cost every 500 tuples
    if ( intermediate == true && !progress->read % 500 )
    {
      progress->intTuplesTotal = calcIntermediateMergeCost();
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
    info->TotalComparisons = TupleCompare::GetComparisonCounter();

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

  delete cmpObj;
  cmpObj = 0;
}

bool SortAlgorithm::updateIntermediateMerges()
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
  // final update of intermediate merge costs
  progress->intTuplesTotal = calcIntermediateMergeCost();

  if ( traceMode )
  {
    info->MergePhase();
    info->IntermediateTuples = progress->intTuplesTotal;
  }

  // create heap for merging runs
  mergeQueue = new TupleAndRelPosQueue(cmpObj);

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
      TupleAndRelPos p(t,i);
      mergeQueue->Push(p);
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
  make_heap(runs.begin(), runs.end(), comp);

  // get the n shortest runs
  vector<SortedRun*> merge;

  for (int i = 0; i < n; i++)
  {
    pop_heap(runs.begin(), runs.end(), comp);
    merge.push_back(runs.back());
    runs.pop_back();
  }

  // create the sorted run for merging
  SortedRun* result = new SortedRun(nextRunNumber++, cmpObj);
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
      TupleAndRelPos p(t,i);
      mergeQueue->Push(p);
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
    TupleAndRelPos p = mergeQueue->Top();
    mergeQueue->Pop();
    Tuple* result = p.tuple();

    // Refill the merge queue with a tuple from result
    // tuple's queue if there are more
    Tuple* t = arr[p.pos]->GetNextTuple();

    // Push the found tuple into the merge queue
    if( t != 0 )
    {
      // run not finished
      p.ref = t;
      mergeQueue->Push(p);
    }

    return result;
  }
}

/*
3 Implementation of value mapping function of operator ~sort2~ and ~sortby2~

*/

template<int firstArg, bool param>
int SortValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  // Operator sort2 (firstArg = 1, param = false)
  // args[0] : stream
  // args[1] : the number of sort attributes
  // args[3] : the index of the first sort attribute
  // args[4] : a boolean which indicates if sortorder should
  //           be asc (true) or desc (false)
  // args[5] : Same as 3 but for the second sort attribute
  // args[6] : Same as 4 but for the second sort attribute
  // ....

  // Operator sort2with (firstArg = 4, param = true)
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

  // Operator sortby2with (firstArg = 5, param = true)
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

          // set I/O buffer size in bytes
          if ( ioBufferSize >= 0 && ioBufferSize <= 16384 )
          {
            li->oldIoBufferSize = TupleBuffer::GetIoBufferSize();
            TupleBuffer::SetIoBufferSize((size_t)ioBufferSize);
          }

          li->ptr = new SortAlgorithm( args[0],
                                       new TupleCompareBy(spec), li,
                                       maxFanIn, maxMemSize);
        }
        else
        {
          li->ptr = new SortAlgorithm( args[0],
                                       new TupleCompareBy(spec), li);
        }
      }

      SortAlgorithm* sa = li->ptr;

      result.setAddr( sa->NextResultTuple() );
      li->returned++;
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      qp->Close(args[0].addr);

      // Note: object deletion is handled in OPEN and CLOSEPROGRESS
      return 0;
    }


    case CLOSEPROGRESS:
    {
      if (li)
      {
        // restore old I/O buffer size if necessary
        if ( li->oldIoBufferSize != UINT_MAX )
        {
          TupleBuffer::SetIoBufferSize(li->oldIoBufferSize);
        }
        delete li;
        local.addr = 0;
      }
      return 0;
    }

    case REQUESTPROGRESS:
    {
      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uSortBy = 0.000396;   //millisecs per byte input and sort
      const double vSortBy = 0.000194;   //millisecs per byte output
      const double oSortBy = 0.00004;   //offset due to writing to disk
            //not yet measurable
      pRes = (ProgressInfo*) result.addr;

      if( !li )
      {
        return CANCEL;
      }
      else
      {
        if (qp->RequestProgress(args[0].addr, &p1))
        {
          pRes->Card = li->returned == 0 ? p1.Card : li->read;
          pRes->CopySizes(p1);

          pRes->Time = p1.Time
                       + pRes->Card * p1.Size * (uSortBy + oSortBy * li->state)
                       + pRes->Card * p1.Size * vSortBy
                       + li->intTuplesTotal * p1.Size * (2 * vSortBy);

          pRes->Progress =
            (p1.Progress * p1.Time
             + li->read * p1.Size * (uSortBy + oSortBy * li->state)
             + li->returned * p1.Size * vSortBy
             + li->intTuplesProc * p1.Size * (2 * vSortBy) )
            / pRes->Time;

          // Estimated time until first result tuple is ready
          pRes->BTime = p1.Time
                        + pRes->Card * p1.Size * (uSortBy + oSortBy * li->state)
                        + li->intTuplesTotal * p1.Size * (2 * vSortBy);

          // Current blocking op progress
          pRes->BProgress =
            (p1.Progress * p1.Time
                + li->read * p1.Size * (uSortBy + oSortBy * li->state)
                + li->intTuplesProc * p1.Size * (2 * vSortBy))
                / pRes->BTime;

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
3 Instantiation of Template Functions

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
