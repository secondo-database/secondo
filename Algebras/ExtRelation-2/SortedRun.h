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

1 Header File SortedRun.h

May 2009. S. Jungnickel. Initial version.

2 Defines and Includes

*/
#ifndef SORTEDRUN_H_
#define SORTEDRUN_H_

#include "RelationAlgebra.h"
#include "Progress.h"
#include "TupleQueue.h"
#include "SortInfo.h"
#include "TupleBuffer.h"

/*
3 Class SortedRun

*/
namespace extrel2
{
  class SortedRun
  {
  public:

    SortedRun(int runNumber, TupleCompareBy* cmp);
/*
The constructor. Construct a new sorted run with run number ~runNumber~
and sets the ~TupleCompareBy~ object pointer to ~cmp~.

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
      return ( (*cmp)(lastTuple.tuple, t) == true );
    }
/*
Returns true tuple ~t~ is in sort order according to the last tuple
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
      lastTuple = heap->Top();
      buffer.AppendTuple( lastTuple.tuple );
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
      lastTuple = heap->Top();
      buffer.AppendTuple( lastTuple.tuple );
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
            t = heap->Top();
            heap->Pop();
          }
        }
      }
      else
      {
        // all tuples in memory
        if ( !heap->Empty() )
        {
          t = heap->Top();
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

    // Run number
    int runNumber;

    // Tuple Compare Object
    TupleCompareBy* cmp;

    // Reference to last tuple written to disk
    RTuple lastTuple;

    // Run length in Bytes
    size_t runLength;

    // Number of tuples in run
    size_t tupleCount;

    // Minimum tuple size
    size_t minTupleSize;

    // Maximum tuple size
    size_t maxTupleSize;

    // Minimum Heap for internal sorting
    extrel2::TupleQueue* heap;

    // Tuple Buffer
    extrel2::TupleBuffer buffer;

    // Iterator for scan
    extrel2::TupleBufferIterator* iter;

    // Statistical information
    extrel2::SortedRunInfo* info;

    // Set Flag to true to enable trace mode
    bool traceMode;
  };

ostream& operator<<(ostream& os, SortedRun& run);

/*
4 Functional comparison classes

4.1 Class SortedRunCompareNumber

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
4.2 Class SortedRunCompareLengthLesser

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
4.3 Class SortedRunCompareLengthGreater

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

}

#endif /* SORTEDRUN_H_ */
