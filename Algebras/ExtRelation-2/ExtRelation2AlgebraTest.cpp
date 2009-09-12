/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 Implementation File ExtRelation2AlgebraTest.cpp

June 2009, Sven Jungnickel. Initial version.

2 Overview

Within this file different test operators have been implemented that were used
during development to test and benchmark the algebra.

3 Includes

*/
#include "LogMsg.h"
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "CPUTimeMeasurer.h"
#include "StandardTypes.h"
#include "SecondoInterface.h"
#include "WinUnix.h"
#include "FileSystem.h"
#include "TupleBuffer.h"
#include "TupleQueue.h"
#include "PriorityQueue.h"

/*
4 Defines

*/

#define HEADLINE_TUPLECOMP "--------------------------- " \
                            "tuplecomp -----------------------------"

#define HEADLINE_HEAPSTL "--------------------------- "\
                          "heapstl -----------------------------"

#define HEADLINE_HEAPSTD "--------------------------- "\
                          "heapstd -----------------------------"

#define HEADLINE_HEAPBUP "--------------------------- "\
                          "heapbup -----------------------------"

#define HEADLINE_HEAPBUP2 "--------------------------- "\
                          "heapbup2 -----------------------------"

#define HEADLINE_HEAPMDR "--------------------------- "\
                          "heapmdr -----------------------------"


/*
4 External linking

*/
extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

/*
5 Utility functions

*/

TupleCompareBy* createTupleCompareBy(Tuple* t)
{
  SortOrderSpecification spec;

  assert(t);

  int nAttrCount = t->GetNoAttributes();

  for(int i = 1; i <= nAttrCount; i++)
  {
    spec.push_back(pair<int, bool>(i, true));
  };

  return new TupleCompareBy(spec);
}

/*
5 Operator ~tuplefiletest~

5.1 LocalInfo class for operator ~tuplefiletest~

*/
namespace extrel2
{

class TupleFileTestLocalInfo
{
public:

  TupleFileTestLocalInfo()
  : file(0)
  , iter(0)
  {}
/*
The constructor. Constructs an empty object.

*/

  ~TupleFileTestLocalInfo()
  {
    if ( file )
    {
      delete file;
    }

    if ( iter )
    {
      delete iter;
    }
  }
/*
The destructor. Deletes the tuple file and iterator instances
if they exist.

*/

  TupleFile* file;
/*
Pointer to ~TupleFile~ instance

*/

  TupleFileIterator* iter;
/*
Pointer to ~TupleFileIterator~ instance

*/
};

/*

5.2 Value mapping function of operator ~tuplefile~

The argument vector ~args~ contains in the first slot ~args[0]~ the
stream. The value mapping function creates a tuple file when the
OPEN message is received and consumes all tuples onto disk. With
the first REQUEST message a sequential tuple file scan is started.
The operator is finished when all tuples from the tuple file
have been read.

*/
int TupleFileValueMap( Word* args, Word& result,
                        int message, Word& local, Supplier s )
{
  // args[0] : stream

  switch(message)
  {
    case OPEN:
    {
      Word wTuple(Address(0));
      TupleFileTestLocalInfo* li = new TupleFileTestLocalInfo();

      // open stream and request first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);

      while( qp->Received(args[0].addr) )
      {
        Tuple *t = static_cast<Tuple*>( wTuple.addr );

        // create tuple file
        if ( li->file == 0 )
        {
          li->file = new TupleFile(t->GetTupleType(), 4096);
          li->file->Open();
        }

        // append tuple to tuple file on disk
        li->file->Append(t);

        // delete tuple
        t->DeleteIfAllowed();

        qp->Request(args[0].addr, wTuple);
      }

      // close tuple file
      li->file->Close();

      // store pointer to local info
      local.addr = li;

      return 0;
    }
    case REQUEST:
    {
      TupleFileTestLocalInfo* li =
        static_cast<TupleFileTestLocalInfo*>( local.addr );

      // create iterator if not yet done
      if ( li->iter == 0 )
      {
        li->iter = li->file->MakeScan();
      }

      // get next tuple from disk
      Tuple* t = li->iter->GetNextTuple();

      // store tuple address in result
      result.setAddr(t);

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      if( local.addr )
      {
        // close stream
        qp->Close(args[0].addr);

        TupleFileTestLocalInfo* li =
          static_cast<TupleFileTestLocalInfo*>( local.addr );

        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
5 Operator ~tuplebuffer~

5.1 LocalInfo class for operator ~tuplebuffer~

*/

class TupleBufferTestLocalInfo
{
public:
  TupleBufferTestLocalInfo()
  : buffer(0)
  , iter(0)
  {}

  ~TupleBufferTestLocalInfo()
  {
    if ( buffer )
    {
      delete buffer;
    }

    if ( iter )
    {
      delete iter;
    }
  }

  extrel2::TupleBuffer* buffer;
  extrel2::TupleBufferIterator* iter;
};

/*

5.2 Value mapping function of operator ~tuplebuffer~

The argument vector ~args~ contains in the first slot ~args[0]~
the stream. The value mapping function creates a tuple buffer of
16 MB size when the OPEN message is received and consumes
all tuples into this buffer. With the first REQUEST message
a sequential buffer scan is started. The operator is finished
when all tuples from the buffer have been read.

*/

int TupleBufferValueMap( Word* args, Word& result,
                           int message, Word& local, Supplier s )
{
  switch(message)
  {
    case OPEN:
    {
      Word wTuple(Address(0));
      TupleBufferTestLocalInfo* li = new TupleBufferTestLocalInfo();

      // open stream and request first tuple
      qp->Open(args[0].addr);
      qp->Request(args[0].addr, wTuple);

      // Read buffer size in KBytes from second argument
      size_t bufferSize = StdTypes::GetInt( args[1] );

      while( qp->Received(args[0].addr) )
      {
        Tuple *t = static_cast<Tuple*>( wTuple.addr );

        // create tuple file
        if ( li->buffer == 0 )
        {
          li->buffer = new extrel2::TupleBuffer(bufferSize*1024);
        }

        // append tuple to buffer
        li->buffer->AppendTuple(t);

        qp->Request(args[0].addr, wTuple);
      }

      // close tuple file
      li->buffer->CloseDiskBuffer();

      // store pointer to local info
      local.addr = li;

      return 0;
    }
    case REQUEST:
    {
      TupleBufferTestLocalInfo* li =
        static_cast<TupleBufferTestLocalInfo*>( local.addr );

      // create iterator if not yet done
      if ( li->iter == 0 )
      {
        li->iter = li->buffer->MakeScan();
      }

      // get next tuple from disk
      Tuple* t = li->iter->GetNextTuple();

      // store tuple address in result
      result.setAddr(t);

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      if( local.addr )
      {
        // close stream
        qp->Close(args[0].addr);

        TupleBufferTestLocalInfo* li =
          static_cast<TupleBufferTestLocalInfo*>( local.addr );

        delete li;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/*
6 Operator ~tuplecomp~

6.1 Value mapping function of operator ~tuplecomp~

The argument vector ~args~ contains in the first slot ~args[0]~
the stream. The value mapping function consumes all tuples
into an array of type ~TupleAndRelPos~. Afterwards the first
array tuple is compared with all other array tuples (including
itself). The time for this operation is measured and can be used
to determine the cost for a tuple comparison.

*/

int TupleCompValueMap( Word* args, Word& result,
                         int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    StopWatch stopWatch;
    clock_t clockticks;
    int tuples = 0;
    std::vector<RTuple> array;

    // Consume Tuple-Stream into array
    qp->Open(args[0].addr);
    qp->Request(args[0].addr, elem);
    while ( qp->Received(args[0].addr) )
    {
      tuples++;
      Tuple *t = static_cast<Tuple*>( elem.addr );
      array.push_back(RTuple(t));
      qp->Request(args[0].addr, elem);
    }

    bool test;

    TupleCompareAsc* cmp =
      new TupleCompareAsc(createTupleCompareBy(array[0].tuple));
    TupleCompare::ResetComparisonCounter();
    stopWatch.start();
    clockticks = clock();

    // Test-Loop for measurement
    for (size_t i = 0; i < array.size(); i++)
    {
      test = (*cmp)(array[0], array[i]);
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockticks = clock() - clockticks;
    double dCPU = stopWatch.diffSecondsCPU();
    double dReal = stopWatch.diffSecondsReal();

    cmsg.info() << HEADLINE_TUPLECOMP
                << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Time (Real): \t" << dReal
                << " (" << dReal / (double)tuples *1000000.0
                << " usec/tuple)" << endl;
    cmsg.info() << "Time (CPU): \t" << dCPU
                << " (" << dCPU / (double)tuples *1000000.0
                << " usec/tuple)" << endl;
    cmsg.info() << "Clockticks: \t" << clockticks
                << " (" << (float)clockticks / (float)tuples
                << " usec/tuple)" << endl;
    cmsg.info() << "TotalComparisons: \t" << comp << endl;
    cmsg.send();

    array.clear();

    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, 1);
    qp->Close(args[0].addr);
    return 0;
  }

  return 0;
}

/*
6 Operator ~heapstl~

6.1 Value mapping function of operator ~heapstl~

*/

int HeapStlValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    TupleQueue* q;
    StopWatch stopWatchPush;
    StopWatch stopWatchPop;
    clock_t clockPush, clockPop, clockStart;
    int tuples = 0;

    TupleCompare::ResetComparisonCounter();

    stopWatchPush.start();
    clockStart = clock();

    // open stream
    qp->Open(args[0].addr);

    // consume stream into queue
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
      Tuple *t = static_cast<Tuple*>( elem.addr );

      if ( tuples == 0 )
      {
        q = new TupleQueue(createTupleCompareBy(t));
      }

      q->Push(t);
      qp->Request(args[0].addr, elem);
      tuples++;
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockPush = clock() - clockStart;
    double dPushCPU = stopWatchPush.diffSecondsCPU();
    double dPushReal = stopWatchPush.diffSecondsReal();
    stopWatchPop.start();
    clockStart = clock();

    // empty queue and delete tuples
    while ( !q->Empty() )
    {
      Tuple* t = q->Top();
      q->Pop();
      t->DeleteIfAllowed();
    }

    clockPop = clock() - clockStart;
    double dPopCPU = stopWatchPop.diffSecondsCPU();
    double dPopReal = stopWatchPop.diffSecondsReal();
    size_t tcomp = TupleCompare::GetComparisonCounter();

    cmsg.info() << HEADLINE_HEAPSTL << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Push Time: \t\t" << dPushReal
                << " (Real) \t" << dPushCPU << " (CPU)" << endl;
    cmsg.info() << "Pop Time: \t\t" << dPopReal
                << " (Real) \t" << dPopCPU << " (CPU)" << endl;
    cmsg.info() << "Push Clockticks: \t" << clockPush
                << " (" << (float)clockPush / (float)tuples
                << " per push)" << endl;
    cmsg.info() << "Pop Clockticks: \t" << clockPop << " ("
                << (float)clockPop / (float)tuples << " per pop)" << endl;
    cmsg.info() << "TotalComparisons: \t" << tcomp << "\t(Push: "
                << comp << ", Pop: " << tcomp - comp << ")" << endl;
    cmsg.send();

    // store total comparisons as result
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, tcomp);

    // close stream
    qp->Close(args[0].addr);

    return 0;
  }

  return 0;
}

/*
6 Operator ~heapstd~

6.1 Value mapping function of operator ~heapstd~

*/


int HeapStdValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    PriorityQueueStandardHeap<RTuple, TupleCompareAsc>* q;
    StopWatch stopWatchPush;
    StopWatch stopWatchPop;
    clock_t clockPush, clockPop, clockStart;
    int tuples = 0;

    TupleCompare::ResetComparisonCounter();

    stopWatchPush.start();
    clockStart = clock();

    // open stream
    qp->Open(args[0].addr);

    // consume stream into queue
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
      Tuple *t = static_cast<Tuple*>( elem.addr );

      if ( tuples == 0 )
      {
        TupleCompareAsc* cmp = new TupleCompareAsc(createTupleCompareBy(t));
        q = new PriorityQueueStandardHeap<RTuple, TupleCompareAsc>(cmp);
      }

      q->Push(RTuple(t));
      qp->Request(args[0].addr, elem);
      tuples++;
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockPush = clock() - clockStart;
    double dPushCPU = stopWatchPush.diffSecondsCPU();
    double dPushReal = stopWatchPush.diffSecondsReal();
    stopWatchPop.start();
    clockStart = clock();

    // empty queue and delete tuples
    while ( !q->IsEmpty() )
    {
      Tuple* t = q->Top().tuple;
      q->Pop();
      t->DeleteIfAllowed();
    }

    clockPop = clock() - clockStart;
    double dPopCPU = stopWatchPop.diffSecondsCPU();
    double dPopReal = stopWatchPop.diffSecondsReal();
    size_t tcomp = TupleCompare::GetComparisonCounter();

    cmsg.info() << HEADLINE_HEAPSTD << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Push Time: \t\t" << dPushReal
                << " (Real) \t" << dPushCPU << " (CPU)" << endl;
    cmsg.info() << "Pop Time: \t\t" << dPopReal << " (Real) \t"
                << dPopCPU << " (CPU)" << endl;
    cmsg.info() << "Push Clockticks: \t" << clockPush
                << " (" << (float)clockPush / (float)tuples
                << " per push)" << endl;
    cmsg.info() << "Pop Clockticks: \t" << clockPop << " ("
                << (float)clockPop / (float)tuples << " per pop)" << endl;
    cmsg.info() << "TotalComparisons: \t" << tcomp << "\t(Push: "
                << comp << ", Pop: " << tcomp - comp << ")" << endl;
    cmsg.send();

    // store total comparisons as result
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, tcomp);

    // close stream
    qp->Close(args[0].addr);

    return 0;
  }

  return 0;
}

/*
6 Operator ~heapbup~

6.1 Value mapping function of operator ~heapbup~

*/

int HeapBupValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    PriorityQueueBottomUpHeap<RTuple, TupleCompareAsc>* q;
    StopWatch stopWatchPush;
    StopWatch stopWatchPop;
    clock_t clockPush, clockPop, clockStart;
    int tuples = 0;

    TupleCompare::ResetComparisonCounter();

    stopWatchPush.start();
    clockStart = clock();

    // open stream
    qp->Open(args[0].addr);

    // consume stream into queue
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
      Tuple *t = static_cast<Tuple*>( elem.addr );

      if ( tuples == 0 )
      {
        TupleCompareAsc* cmp = new TupleCompareAsc(createTupleCompareBy(t));
        q = new PriorityQueueBottomUpHeap<RTuple, TupleCompareAsc>(cmp);
      }

      q->Push(RTuple(t));
      qp->Request(args[0].addr, elem);
      tuples++;
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockPush = clock() - clockStart;
    double dPushCPU = stopWatchPush.diffSecondsCPU();
    double dPushReal = stopWatchPush.diffSecondsReal();
    stopWatchPop.start();
    clockStart = clock();

    // empty queue and delete tuples
    while ( !q->IsEmpty() )
    {
      Tuple* t = q->Top().tuple;
      q->Pop();
      t->DeleteIfAllowed();
    }

    clockPop = clock() - clockStart;
    double dPopCPU = stopWatchPop.diffSecondsCPU();
    double dPopReal = stopWatchPop.diffSecondsReal();
    size_t tcomp = TupleCompare::GetComparisonCounter();

    cmsg.info() << HEADLINE_HEAPBUP << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Push Time: \t\t" << dPushReal << " (Real) \t"
                << dPushCPU << " (CPU)" << endl;
    cmsg.info() << "Pop Time: \t\t" << dPopReal << " (Real) \t"
                << dPopCPU << " (CPU)" << endl;
    cmsg.info() << "Push Clockticks: \t" << clockPush << " ("
                << (float)clockPush / (float)tuples << " per push)" << endl;
    cmsg.info() << "Pop Clockticks: \t" << clockPop << " ("
                << (float)clockPop / (float)tuples << " per pop)" << endl;
    cmsg.info() << "TotalComparisons: \t" << tcomp << "\t(Push: "
                << comp << ", Pop: " << tcomp - comp << ")" << endl;
    cmsg.send();

    // store total comparison as result
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, tcomp);

    // close stream
    qp->Close(args[0].addr);

    return 0;
  }

  return 0;
}

/*
6 Operator ~heapbup2~

6.1 Value mapping function of operator ~heapbup2~

*/

int HeapBup2ValueMap( Word* args, Word& result,
                        int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    PriorityQueueBottomUpHeap2<RTuple, TupleCompareAsc>* q;
    StopWatch stopWatchPush;
    StopWatch stopWatchPop;
    clock_t clockPush, clockPop, clockStart;
    int tuples = 0;

    TupleCompare::ResetComparisonCounter();

    stopWatchPush.start();
    clockStart = clock();

    // open stream
    qp->Open(args[0].addr);

    // consume stream into queue
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
      Tuple *t = static_cast<Tuple*>( elem.addr );

      if ( tuples == 0 )
      {
        TupleCompareAsc* cmp = new TupleCompareAsc(createTupleCompareBy(t));
        q = new PriorityQueueBottomUpHeap2<RTuple, TupleCompareAsc>(cmp);
      }

      q->Push(RTuple(t));
      qp->Request(args[0].addr, elem);
      tuples++;
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockPush = clock() - clockStart;
    double dPushCPU = stopWatchPush.diffSecondsCPU();
    double dPushReal = stopWatchPush.diffSecondsReal();
    stopWatchPop.start();
    clockStart = clock();

    // empty queue and delete tuples
    while ( !q->IsEmpty() )
    {
      Tuple* t = q->Top().tuple;
      q->Pop();
      t->DeleteIfAllowed();
    }

    clockPop = clock() - clockStart;
    double dPopCPU = stopWatchPop.diffSecondsCPU();
    double dPopReal = stopWatchPop.diffSecondsReal();
    size_t tcomp = TupleCompare::GetComparisonCounter();

    cmsg.info() << HEADLINE_HEAPBUP2 << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Push Time: \t\t" << dPushReal
                << " (Real) \t" << dPushCPU << " (CPU)" << endl;
    cmsg.info() << "Pop Time: \t\t" << dPopReal << " (Real) \t"
                << dPopCPU << " (CPU)" << endl;
    cmsg.info() << "Push Clockticks: \t" << clockPush
                << " (" << (float)clockPush / (float)tuples
                << " per push)" << endl;
    cmsg.info() << "Pop Clockticks: \t" << clockPop
                << " (" << (float)clockPop / (float)tuples
                << " per pop)" << endl;
    cmsg.info() << "TotalComparisons: \t" << tcomp
                << "\t(Push: " << comp << ", Pop: "
                << tcomp - comp << ")" << endl;
    cmsg.send();

    // store total comparisons as result
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, tcomp);

    // close stream
    qp->Close(args[0].addr);

    return 0;
  }

  return 0;
}

/*
6 Operator ~heapmdr~

6.1 Value mapping function of operator ~heapmdr~

*/

int HeapMdrValueMap( Word* args, Word& result,
                      int message, Word& local, Supplier s )
{
  if ( message <= CLOSE )
  {
    Word elem;
    PriorityQueueMDRHeap<RTuple, TupleCompareAsc>* q;
    StopWatch stopWatchPush;
    StopWatch stopWatchPop;
    clock_t clockPush, clockPop, clockStart;
    int tuples = 0;

    TupleCompare::ResetComparisonCounter();

    stopWatchPush.start();
    clockStart = clock();

    // open stream
    qp->Open(args[0].addr);

    // consume stream into queue
    qp->Request(args[0].addr, elem);

    while ( qp->Received(args[0].addr) )
    {
      Tuple *t = static_cast<Tuple*>( elem.addr );

      if ( tuples == 0 )
      {
        TupleCompareAsc* cmp = new TupleCompareAsc(createTupleCompareBy(t));
        q = new PriorityQueueMDRHeap<RTuple, TupleCompareAsc>(cmp);
      }

      q->Push(RTuple(t));
      qp->Request(args[0].addr, elem);
      tuples++;
    }

    size_t comp = TupleCompare::GetComparisonCounter();
    clockPush = clock() - clockStart;
    double dPushCPU = stopWatchPush.diffSecondsCPU();
    double dPushReal = stopWatchPush.diffSecondsReal();
    stopWatchPop.start();
    clockStart = clock();

    // empty queue and delete tuples
    while ( !q->IsEmpty() )
    {
      Tuple* t = q->Top().tuple;
      q->Pop();
      t->DeleteIfAllowed();
    }

    clockPop = clock() - clockStart;
    double dPopCPU = stopWatchPop.diffSecondsCPU();
    double dPopReal = stopWatchPop.diffSecondsReal();
    size_t tcomp = TupleCompare::GetComparisonCounter();

    cmsg.info() << HEADLINE_HEAPMDR << endl;
    cmsg.info() << "Tuples: " << tuples << endl;
    cmsg.info() << "Push Time: \t\t" << dPushReal
                << " (Real) \t" << dPushCPU << " (CPU)" << endl;
    cmsg.info() << "Pop Time: \t\t" << dPopReal
                << " (Real) \t" << dPopCPU << " (CPU)" << endl;
    cmsg.info() << "Push Clockticks: \t" << clockPush
                << " (" << (float)clockPush / (float)tuples
                << " per push)" << endl;
    cmsg.info() << "Pop Clockticks: \t" << clockPop
                << " (" << (float)clockPop / (float)tuples
                << " per pop)" << endl;
    cmsg.info() << "TotalComparisons: \t" << tcomp
                << "\t(Push: " << comp << ", Pop: "
                << tcomp - comp << ")" << endl;
    cmsg.send();

    // store total comparison as result
    result = qp->ResultStorage(s);
    ((CcInt*) result.addr)->Set(true, tcomp);

    // close stream
    qp->Close(args[0].addr);

    return 0;
  }

  return 0;
}

} // end of namespace extrel2
