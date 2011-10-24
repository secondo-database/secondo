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

#include <vector>
#include <list>
#include <set>
#include <queue>

#include "SortByLocalInfo.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "SecondoInterface.h"
#include "StopWatch.h"
#include "Counter.h"
#include "Progress.h"
#include "RTuple.h"
#include "Tupleorder.h"

#ifndef USE_PROGRESS

//-- begin standard version --//

// class SortByLocalInfo
// {
  // public:
    SortByLocalInfo( Word stream,
		     const bool lexicographic,
		     void *tupleCmp ):
      stream( stream ),
      currentIndex( 0 ),
      lexiTupleCmp( lexicographic ?
                    (LexicographicalTupleCompare*)tupleCmp :
                    0 ),
      tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
      lexicographic( lexicographic )
      {
        // Note: Is is not possible to define a Cmp object using the
        // constructor
        // mergeTuples( PairTupleCompareBy( tupleCmpBy )).
        // It does only work if mergeTuples is a local variable which
        // does not help us in this case. Is it a Compiler bug or C++ feature?
        // Hence a new class TupleAndRelPos was defined which implements
        // the comparison operator '<'.
        TupleQueue* currentRun = &queue[0];
        TupleQueue* nextRun = &queue[1];

        Word wTuple(Address(0));
        size_t  c = 0, i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
        bool newRelation = true;


        MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
        cmsg.info("ERA:ShowMemInfo")
          << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
        cmsg.send();

        TupleBuffer *rel=0;
        TupleAndRelPos lastTuple(0, tupleCmpBy);

        qp->Request(stream.addr, wTuple);
        TupleAndRelPos minTuple(0, tupleCmpBy);
        while(qp->Received(stream.addr)) // consume the stream completely
        {

          c++; // tuple counter;
          Tuple* t = static_cast<Tuple*>( wTuple.addr );
          TupleAndRelPos nextTuple(t, tupleCmpBy);
          if( MAX_MEMORY > (size_t)t->GetExtSize() )
          {
            nextTuple.tuple->IncReference();
            currentRun->push(nextTuple);
            i++; // increment Tuples in memory counter
            MAX_MEMORY -= t->GetExtSize();
          }
          else
          { // memory is completely used
            if ( newRelation )
            { // create new relation
              r++;
              rel = new TupleBuffer( 0 );
              GenericRelationIterator *iter = 0;
              relations.push_back( make_pair( rel, iter ) );
              newRelation = false;

              // get first tuple and store it in an relation
              nextTuple.tuple->IncReference();
              currentRun->push(nextTuple);
              minTuple = currentRun->top();
              //minTuple.tuple->DecReference();
              rel->AppendTuple( minTuple.tuple );
              lastTuple = minTuple;
              currentRun->pop();
            }
            else
            { // check if nextTuple can be saved in current relation
              TupleAndRelPos copyOfLast = lastTuple;
              if ( nextTuple < lastTuple )
              { // nextTuple is in order
                // Push the next tuple int the heap and append the minimum to
                // the current relation and push
                nextTuple.tuple->IncReference();
                currentRun->push(nextTuple);
                minTuple = currentRun->top();
                //minTuple.tuple->DecReference();
                rel->AppendTuple( minTuple.tuple );
                lastTuple = minTuple;
                currentRun->pop();
                m++;
              }
              else
              { // nextTuple is smaller, save it for the next relation
                nextTuple.tuple->IncReference();
                nextRun->push(nextTuple);
                n++;
                if ( !currentRun->empty() )
                {
                  // Append the minimum to the current relation
                  minTuple = currentRun->top();
                  //minTuple.tuple->DecReference();
                  rel->AppendTuple( minTuple.tuple );
                  lastTuple = minTuple;
                  currentRun->pop();
                }
                else
                { //create a new run
                  newRelation = true;

                  // swap queues
                  TupleQueue *helpRun = currentRun;
                  currentRun = nextRun;
                  nextRun = helpRun;
                  ShowPartitionInfo(c,a,n,m,r,rel);
                  i=n;
                  a=0;
                  n=0;
                  m=0;
                } // end new run
              } // end next tuple is smaller

              // delete last tuple if saved to relation and
              // not referenced by minTuple
              if ( copyOfLast.tuple && (copyOfLast.tuple != minTuple.tuple) )
              {
                copyOfLast.tuple->DeleteIfAllowed();
              }

            } // check if nextTuple can be saved in current relation
          }// memory is completely used

          qp->Request(stream.addr, wTuple);
        }
        ShowPartitionInfo(c,a,n,m,r,rel);

        // delete lastTuple and minTuple if allowed
        if ( lastTuple.tuple )
        {
          lastTuple.tuple->DeleteIfAllowed();
        }
        if ( (minTuple.tuple != lastTuple.tuple) )
        {
          minTuple.tuple->DeleteIfAllowed();
        }

        // the lastRun and NextRun runs in memory having
        // less than MAX_TUPLE elements
        if( !queue[0].empty() )
        {
          Tuple* t = queue[0].top().tuple;
          queue[0].pop();
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -2) );
        }
        if( !queue[1].empty() )
        {
          Tuple* t = queue[1].top().tuple;
          queue[1].pop();
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -1) );
        }

        // Get next tuple from each relation and push it into the heap.
        for( size_t i = 0; i < relations.size(); i++ )
        {
          relations[i].second = relations[i].first->MakeScan();
          Tuple *t = relations[i].second->GetNextTuple();
          if( t != 0 )
          {
            t->IncReference();
            mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, i+1) );
          }
        }
        Counter::getRef("Sortby:ExternPartitions") = relations.size();
      }

/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

    SortByLocalInfo::~SortByLocalInfo()
    {
      while( !mergeTuples.empty() )
      {
        mergeTuples.top().tuple->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].top().tuple->DeleteIfAllowed();
          queue[i].pop();
        }
      }

      // delete information about sorted runs
      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        relations[i].second = 0;
        delete relations[i].first;
        relations[i].first = 0;
      }

      delete lexiTupleCmp;
      lexiTupleCmp = 0;
      delete tupleCmpBy;
      tupleCmpBy = 0;
    }

    Tuple* SortByLocalInfo::NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
        return 0;
      else
      {
        // Take the first one.
        TupleAndRelPos p = mergeTuples.top();
        //p.tuple->DecReference();
        mergeTuples.pop();
        Tuple *result = p.tuple;
        Tuple *t = 0;

        if (p.pos > 0)
          t = relations[p.pos-1].second->GetNextTuple();
        else
        {
          int idx = p.pos+2;
          if ( !queue[idx].empty() )
          {
            t = queue[idx].top().tuple;
            //t->DecReference();
            queue[idx].pop();
          }
          else
            t = 0;
        }

        if( t != 0 )
        { // run not finished
          p.tuple = t;
          t->IncReference();
          mergeTuples.push( p );
        }
        return result;
      }
    }

    void SortByLocalInfo::ShowPartitionInfo( int c, int a, int n,
		            int m, int r, GenericRelation* rel )
    {
      int rs = (rel != 0) ? rel->GetNoTuples() : 0;
      if ( RTFlag::isActive("ERA:Sort:PartitionInfo") )
      {
        cmsg.info() << "Current run finished: "
		    << "  processed tuples=" << c
                    << ", append minimum=" << m
                    << ", append next=" << n << endl
                    << "  materialized runs=" << r
                    << ", last partition's tuples=" << rs << endl
                    << "  Runs in memory: queue1= " << queue[0].size()
                    << ", queue2= " << queue[1].size() << endl;
        cmsg.send();
      }
    }

    // Word stream;
    // size_t currentIndex;

    // // tuple information
    // LexicographicalTupleCompare *lexiTupleCmp;
    // TupleCompareBy *tupleCmpBy;
    // bool lexicographic;

    // // sorted runs created by in memory heap filtering
    // size_t MAX_MEMORY;
    // typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    // vector< SortedRun > relations;

    // typedef priority_queue<TupleAndRelPos> TupleQueue;
    // TupleQueue queue[2];
    // TupleQueue mergeTuples;
// };

//-- end standard version --//

#else

//-- begin progress version --//

// class SortByLocalInfo : protected ProgressWrapper
// {
  // public:
    SortByLocalInfo::SortByLocalInfo( Word stream, const bool lexicographic,
		     void *tupleCmp, ProgressLocalInfo* p, Supplier s ):
      ProgressWrapper(p),
      stream( stream ),
      currentIndex( 0 ),
      tupleCount(0),
      lexiTupleCmp( lexicographic ?
                    (LexicographicalTupleSmaller*)tupleCmp :
                    0 ),
      tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
      lexicographic( lexicographic )
      {
        // Note: It is not possible to define a Cmp object using the
        // constructor
        // mergeTuples( PairTupleCompareBy( tupleCmpBy )).
        // It does only work if mergeTuples is a local variable which
        // does not help us in this case. Is it a Compiler bug or C++ feature?
        // Hence a new class TupleAndRelPos was defined which implements
        // the comparison operator '<'.
        Heap* currentRun = &queue[0];
        Heap* nextRun = &queue[1];

        Word wTuple(Address(0));
        size_t  i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
        bool newRelation = true;


        MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
        cmsg.info("ERA:ShowMemInfo")
          << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
        cmsg.send();

        TupleBuffer *rel=0;
        RTuple lastTuple;

        qp->Request(stream.addr, wTuple);
        RTuple minTuple;
        while(qp->Received(stream.addr)) // consume the stream completely
        {
          progress->read++;
          tupleCount++; // tuple counter;
          Tuple *t = static_cast<Tuple*>( wTuple.addr );
          TupleAndRelPos nextTuple(t, tupleCmpBy);
          if( MAX_MEMORY > (size_t)t->GetExtSize() )
          {
            //nextTuple.tuple->IncReference();
            currentRun->push(nextTuple);
            i++; // increment Tuples in memory counter
            MAX_MEMORY -= t->GetExtSize();
          }
          else
          { // memory is completely used
            progress->state = 1;
            if ( newRelation )
            { // create new relation
              r++;
              rel = new TupleBuffer( 0 );
              GenericRelationIterator *iter = 0;
              relations.push_back( make_pair( rel, iter ) );
              newRelation = false;

              // get first tuple and store it in an relation
              //nextTuple.ref.tuple->IncReference();
              currentRun->push(nextTuple);
              minTuple = currentRun->topTuple();
              rel->AppendTuple( minTuple.tuple );
              lastTuple = minTuple;
              minTuple.tuple->DeleteIfAllowed(); // remove from input stream
              currentRun->pop();
            }
            else
            { // check if nextTuple can be saved in current relation
              if ( nextTuple < TupleAndRelPos(lastTuple.tuple, tupleCmpBy) )
              { // nextTuple is in order
                // Push the next tuple int the heap and append the minimum to
                // the current relation and push
                //##nextTuple.ref.tuple->IncReference();
                currentRun->push(nextTuple);
                minTuple = currentRun->topTuple();
                rel->AppendTuple( minTuple.tuple );
                minTuple.tuple->DeleteIfAllowed();
                lastTuple = minTuple;
                currentRun->pop();
                m++;
              }
              else
              { // nextTuple is smaller, save it for the next relation
                //##nextTuple.ref.tuple->IncReference();
                nextRun->push(nextTuple);
                n++;
                if ( !currentRun->empty() )
                {
                  // Append the minimum to the current relation
                  minTuple = currentRun->topTuple();
                  rel->AppendTuple( minTuple.tuple );
                  minTuple.tuple->DeleteIfAllowed();
                  lastTuple = minTuple;
                  currentRun->pop();
                }
                else
                { //create a new run
                  newRelation = true;

                  // swap queues
                  TupleQueue *helpRun = currentRun;
                  currentRun = nextRun;
                  nextRun = helpRun;
                  ShowPartitionInfo(tupleCount,a,n,m,r,rel);
                  i=n;
                  a=0;
                  n=0;
                  m=0;
                } // end new run
              } // end next tuple is smaller

              // delete last tuple if saved to relation and
              // not referenced by minTuple
              /*if ( copyOfLast.tuple && (copyOfLast.tuple != minTuple.tuple) )
              {
                copyOfLast.tuple->DeleteIfAllowed();
              }*/

            } // check if nextTuple can be saved in current relation
          }// memory is completely used

          qp->Request(stream.addr, wTuple);
        }
        ShowPartitionInfo(tupleCount,a,n,m,r,rel);

        // delete lastTuple and minTuple if allowed
        /*##if ( lastTuple.tuple )
        {
          lastTuple.tuple->DeleteIfAllowed();
        }
        if ( (minTuple.tuple != lastTuple.tuple) )
        {
          minTuple.tuple->DeleteIfAllowed();
        }*/

        // the lastRun and NextRun runs in memory having
        // less than MAX_TUPLE elements
        if( !queue[0].empty() )
        {
          Tuple* t = queue[0].topTuple();
          queue[0].pop();
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -2) );
        }
        if( !queue[1].empty() )
        {
          Tuple* t = queue[1].topTuple();
          queue[1].pop();
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, -1) );
        }

        // Get next tuple from each relation and push it into the heap.
        for( size_t i = 0; i < relations.size(); i++ )
        {
          relations[i].second = relations[i].first->MakeScan();
          Tuple *t = relations[i].second->GetNextTuple();
          if( t != 0 )
          {
            //t->IncReference();
            mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, i+1) );
          }
        }
        Counter::getRef("Sortby:ExternPartitions") = relations.size();
      }

/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

    SortByLocalInfo::~SortByLocalInfo()
    {
      while( !mergeTuples.empty() )
      {
        mergeTuples.topTuple()->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].topTuple()->DeleteIfAllowed();
          queue[i].pop();
        }
      }

      // delete information about sorted runs
      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        relations[i].second = 0;
        delete relations[i].first;
        relations[i].first = 0;
      }

      delete lexiTupleCmp;
      lexiTupleCmp = 0;
      delete tupleCmpBy;
      tupleCmpBy = 0;
    }

    Tuple* SortByLocalInfo::NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
        return 0;
      else
      {
        // Take the first one.
        TupleAndRelPos p = mergeTuples.top();
        //p.tuple->DecReference();
        mergeTuples.pop();
        Tuple* result = p.tuple();
        Tuple* t = 0;

        if (p.pos > 0)
          t = relations[p.pos-1].second->GetNextTuple();
        else
        {
          int idx = p.pos+2;
          if ( !queue[idx].empty() )
          {
            t = queue[idx].topTuple();
            //t->DecReference();
            queue[idx].pop();
          }
          else
            t = 0;
        }

        if( t != 0 )
        { // run not finished
          p.ref = t;
          //t->IncReference();
          mergeTuples.push( p );
        }
        return result;
      }
    }

  // private:

    void SortByLocalInfo::ShowPartitionInfo( int c, int a, int n,
		            int m, int r, GenericRelation* rel )
    {
      int rs = (rel != 0) ? rel->GetNoTuples() : 0;
      if ( RTFlag::isActive("ERA:Sort:PartitionInfo") )
      {
        cmsg.info() << "Current run finished: "
		    << "  processed tuples=" << c
                    << ", append minimum=" << m
                    << ", append next=" << n << endl
                    << "  materialized runs=" << r
                    << ", last partition's tuples=" << rs << endl
                    << "  Runs in memory: queue1= " << queue[0].size()
                    << ", queue2= " << queue[1].size() << endl;
        cmsg.send();
      }
    }

    // Word stream;
    // size_t currentIndex;

    // // tuple information
    // LexicographicalTupleSmaller *lexiTupleCmp;
    // TupleCompareBy *tupleCmpBy;
    // bool lexicographic;

    // // sorted runs created by in memory heap filtering
    // size_t MAX_MEMORY;
    // typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    // vector< SortedRun > relations;

    // typedef TupleQueue Heap;
    // Heap queue[2];
    // Heap mergeTuples;



// };


//-- end progress version --//

#endif
