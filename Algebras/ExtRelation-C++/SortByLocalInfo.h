/*

----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
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

*/


// #include <vector>
// #include <list>
// #include <set>
// #include <queue>

// #include "LogMsg.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
// #include "CPUTimeMeasurer.h"
// #include "QueryProcessor.h"
// #include "SecondoInterface.h"
// #include "StopWatch.h"
// #include "Counter.h"
#include "Progress.h"
#include "RTuple.h"
#include "Tupleorder.h"

#ifndef SORTBYLOCALINFO_
#define SORTBYLOCALINFO_
#ifndef USE_PROGRESS

class SortByLocalInfo
{
  public:
    SortByLocalInfo( Word stream,
         const bool lexicographic,
         void *tupleCmp );
		~SortByLocalInfo();
    Tuple *NextResultTuple();
private:
    void ShowPartitionInfo( int c, int a, int n,
                int m, int r, GenericRelation* rel );
    Word stream;
    size_t currentIndex;

    // tuple information
    LexicographicalTupleCompare *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;

    // sorted runs created by in memory heap filtering
    size_t MAX_MEMORY;
    typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    vector< SortedRun > relations;

    typedef priority_queue<TupleAndRelPos> TupleQueue;
    TupleQueue queue[2];
    TupleQueue mergeTuples;
};
#else
class SortByLocalInfo : protected ProgressWrapper
{
  public:
    SortByLocalInfo( Word stream, const bool lexicographic, 
		void *tupleCmp, ProgressLocalInfo* p, Supplier s);
		~SortByLocalInfo();
    Tuple* NextResultTuple();		
    size_t TupleCount(){
      return tupleCount;
    }
	private:
		void ShowPartitionInfo( int c, int a, int n,
		            int m, int r, GenericRelation* rel );
    Word stream;
    size_t tupleCount;

    // tuple information
    LexicographicalTupleSmaller *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;

    // sorted runs created by in memory heap filtering
    size_t MAX_MEMORY;
    typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    vector< SortedRun > relations;

    typedef TupleQueue Heap;
    Heap queue[2];
    Heap mergeTuples;	
};
#endif
#endif
