/* 
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

Oct 2004. M. Spiekermann. The SortByLocalInfo was revised, since it doesn't
work for relations not fitting into memory. Moreover some minor performance tuning
was made (fixed size for the vector of tuples).

Nov 2004. M. Spiekermann. The Algorithm for external sorting was changed. See below 
for details.

Sept. 2005. M. Spiekermann. Sortby altered for usage of class ~TupleBuffer~. Moreover,
a memory leak in the ~sortmergejoin~ value mapping was fixed. 

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

[1] Implementation of the Module Extended Relation Algebra for Persistent storage

[TOC]

1 Includes and defines

*/

#include <vector>
#include <list>
#include <set>
#include <queue>

#include "LogMsg.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "SecondoInterface.h"
#include "StopWatch.h"
#include "Counter.h"
#include "Progress.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.1 Operators ~sort~ and ~sortby~

This operator sorts a stream of tuples by a given list of attributes.
For each attribute it must be specified wether the list should be sorted
in ascending (asc) or descending (desc) order with regard to that attribute.

2.2.1 Auxiliary definitions for value mapping function of operators ~sort~ and ~sortby~

*/

static LexicographicalTupleCompare lexCmp;

class TupleAndRelPos {
public:

  TupleAndRelPos() :
    tuple(0),
    pos(0),
    cmpPtr(0) 
  {};
  
  TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0, 
                 int newPos = 0) :
    tuple(newTuple),
    pos(newPos),
    cmpPtr(cmpObjPtr)
  {}; 

  inline bool operator<(const TupleAndRelPos& ref) const 
  { 
    // by default < is used to define a sort order
    // the priority queue creates a maximum heap, hence
    // we change the result to create a minimum queue.
    // It would be nice to have also an < operator in the class
    // Tuple. Moreover lexicographical comparison should be done by means of
    // TupleCompareBy and an appropriate sort order specification, 

    if (!this->tuple || !ref.tuple) {
      return true;
    }
    if ( cmpPtr ) {
      return !(*(TupleCompareBy*)cmpPtr)( this->tuple, ref.tuple );
    } else {
      return !lexCmp( this->tuple, ref.tuple );
    }
  }

  Tuple* tuple;
  int pos;

private:
  void* cmpPtr;

};


/*
2.2.2 class SortByLocalInfo

An algorithm for external sorting is implemented inside this class. The
constructor creates sorted partitions of the input stream and stores them inside
temporary relations and two heaps in memory.  By calls of ~NextResultTuple~
tuples are returned in sorted order. The sort order must be specified in the
constructor. The memory usage is bounded, hence only a fixed number of tuples
can be hold in memory.

The algorithm roughly works as follows: First all input tuples are stored in a
minimum heap until no more tuples fit into memory.  Then, a new relation is
created and the minimum is stored there.  Afterwards, the tuples are handled as follows:

(a) if the next tuple is less or equal than the minimum of the heap and greater
or equal than the last tuple written to disk, it will be appended to the
current relation

(b) if the next tuple is smaller than the last written it will be stored in a
second heap to be used in the next created relation. 

(c) if the next tuple t is greater than the top of the heap, the minimum will be
written to disk and t will be inserted into the heap.

Finally, the minimum tuple of every temporary relation and the two heaps is
inserted into a probably small heap (containing only one tuple for every
partition) and for every request for tuples this minimum is removed and the
next tuple of the partition of the just returned tuples will be inserted into
the heap.

This algorithm reduces the number of comparisons which are quite costly inside
Secondo (due to usage of C++ Polymorphism) even for ~standard~ attributes.

Moreover, if the input stream is already sorted only one partition will be
created and no costs for merging tuples will occur. Unfortunateley this solution
needs more comparisons than sorting.  


Ideas for future improvement: 

All tuples which are not in order should be collected in a buffer and the
others are written into an relation on disk (maybe also buffered to avoid
writing small results to disk). When the buffer of unsorted tuples is full it
will be sorted and written into a new relation.  While filling the buffer we
can keep track if the inserted tuples are in ascending or descending order.
This algorithm will adapt to sorted streams and will only need N (already sorted)
or 2N (sorted in opposite order) comparisons in that case.  

*/
class SortByLocalInfo : protected ProgressWrapper
{
  public:
    SortByLocalInfo( Word stream, const bool lexicographic, 
		     void *tupleCmp, Progress* p            ):
      ProgressWrapper(p),	    
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
        // does not help us in this case.
        // Hence a new class TupleAndRelPos was defined to define a 
        // '<' operator. 
        TupleQueue* currentRun = &queue[0];
        TupleQueue* nextRun = &queue[1];
       
        Word wTuple = SetWord(Address(0));
        size_t  c = 0, i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
        bool newRelation = true;


        MAX_MEMORY = qp->MemoryAvailableForOperator();
        cmsg.info("ERA:ShowMemInfo")
          << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
        cmsg.send();

        TupleBuffer *rel=0;
        TupleAndRelPos lastTuple(0, tupleCmpBy);

        qp->Open(stream.addr);
        qp->Request(stream.addr, wTuple);
        TupleAndRelPos minTuple(0, tupleCmpBy);
        while(qp->Received(stream.addr)) // consume the stream completely
        {
          // set progress counter
	  progress->setCtr(10);

          c++; // tuple counter;
          Tuple *t = static_cast<Tuple*>( wTuple.addr );
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
              minTuple.tuple->DecReference();
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
                minTuple.tuple->DecReference();
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
                  minTuple.tuple->DecReference();
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
                  ShowPartitionInfo(c,a,n,m,r);
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
        ShowPartitionInfo(c,a,n,m,r);

        // delete lastTuple and minTuple if allowed
        if ( lastTuple.tuple ) 
        {
          lastTuple.tuple->DeleteIfAllowed();
        }
        if ( (minTuple.tuple != lastTuple.tuple) ) 
        {
          minTuple.tuple->DeleteIfAllowed();
        }

        qp->Close(stream.addr);

        // the lastRun and NextRun partitions in memory having 
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

    ~SortByLocalInfo()
    {
      while( !mergeTuples.empty() ) 
      {
        mergeTuples.top().tuple->DecReference();
        mergeTuples.top().tuple->DeleteIfAllowed();
        mergeTuples.pop();
      }

      for( int i = 0; i < 2; i++ )
      {
        while( !queue[i].empty() )
        {
          queue[i].top().tuple->DecReference();
          queue[i].top().tuple->DeleteIfAllowed();
          queue[i].pop();
        }  
      }

      // delete information about sorted runs
      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        delete relations[i].first;
      }

      delete lexiTupleCmp;
      delete tupleCmpBy;
    }

    Tuple *NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
        return 0;
      else
      {
        // Take the first one.
        TupleAndRelPos p = mergeTuples.top();
        p.tuple->DecReference();
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
            t->DecReference();
            queue[idx].pop();
          } 
          else 
            t = 0;
        }

        if( t != 0 ) 
        { // partition not finished
          p.tuple = t;
          t->IncReference();
          mergeTuples.push( p );
        }
        return result;
      }
    }

  private:

    void ShowPartitionInfo(int c, int a, int n, int m, int r) 
    {
      if ( RTFlag::isActive("ERA:Sort:PartitionInfo") ) 
      {
        cmsg.info() << "Partition finished: processed tuples=" << c 
                    << ", append minimum=" << m 
                    << ", append next=" << n << "," << endl << "  "
                    << "materialized partitions=" << r
                    << ", queue1: " << queue[0].size() 
                    << ", queue2: " << queue[1].size() << endl;
        cmsg.send();  
      }
    }

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

/*
2.1.1 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the first
sort attribute, ~args[4]~ a boolean indicating wether the stream is sorted in
ascending order with regard to the sort first attribute. ~args[5]~ and ~args[6]~
contain these values for the second sort attribute  and so on.

*/

template<bool lexicographically> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s)
{
  switch(message)
  {
    case OPEN:
    {
      void *tupleCmp;
      SortOrderSpecification spec;
      bool sortOrderIsAscending;
      int nSortAttrs;
      int sortAttrIndex;

      if(lexicographically)
        tupleCmp = new LexicographicalTupleCompare();
      else
      {
        nSortAttrs = ((CcInt*)args[2].addr)->GetIntval();
        for(int i = 1; i <= nSortAttrs; i++)
        {
          sortAttrIndex = ((CcInt*)args[2 * i + 1].addr)->GetIntval();
          sortOrderIsAscending = ((CcBool*)args[2 * i + 2].addr)->GetBoolval();
          spec.push_back(pair<int, bool>(sortAttrIndex, 
                                         sortOrderIsAscending));
        };

        tupleCmp = new TupleCompareBy( spec );
      }

      // create a ~Progress~ instance
      LocalInfo<SortByLocalInfo>* li = new LocalInfo<SortByLocalInfo>();
      local.addr = li;

      // at this point the local value is well defined
      // afterwards QueryProcessor request calls are
      // allowed.

      li->ptr = new SortByLocalInfo( args[0], 
		                     lexicographically,  
                                     tupleCmp, li       );
      return 0;
    }
    case REQUEST:
    {
      SortByLocalInfo *sli = LocalInfo<SortByLocalInfo>::getPtr( local.addr );
      result = SetWord( sli->NextResultTuple() );
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      SortByLocalInfo *sli = LocalInfo<SortByLocalInfo>::getPtr( local.addr );
      delete sli;
      // The ~Progress~ part of the local value will not be deleted
      // this is an accepted memory leak introduced by progress  
      // delete local.addr !!!!
      return 0;
    }
  }
  return 0;
}

/*
2.2 Operator ~mergejoin~

This operator computes the equijoin of two streams.

2.2.1 Auxiliary definitions for value mapping function of operator ~mergejoin~

*/

static CcInt oneCcInt(true, 1);
static CcBool trueCcBool(true, true);

CPUTimeMeasurer mergeMeasurer;

class MergeJoinLocalInfo
{
private:
  size_t MAX_MEMORY;

  vector<Tuple*> bucketA;
  size_t indexA;

  vector<Tuple*> bucketB;
  size_t indexB;

  TupleBuffer *relationA;
  GenericRelationIterator *iterRelationA;

  TupleBuffer *relationB;
  GenericRelationIterator *iterRelationB;

  Word streamALocalInfo;
  Word streamBLocalInfo;

  Word streamA;
  Word streamB;

  Word aResult;
  Word bResult;

  ArgVector aArgs;
  ArgVector bArgs;

  int attrIndexA;
  int attrIndexB;

  bool expectSorted;

  TupleType *resultTupleType;

  const bool traceFlag; 

  int CompareTuples(Tuple* a, Tuple* b)
  {
    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if(!((Attribute*)a->GetAttribute(attrIndexA))->IsDefined())
    {
      return -1;
    }
    if(!((Attribute*)b->GetAttribute(attrIndexB))->IsDefined())
    {
      return 1;
    }

    return ((Attribute*)a->GetAttribute(attrIndexA))->
             Compare((Attribute*)b->GetAttribute(attrIndexB));
  }

  void SetArgs(ArgVector& args, Word stream, Word attrIndex)
  {
    args[0] = SetWord(stream.addr);
    args[2] = SetWord(&oneCcInt);
    args[3] = SetWord(attrIndex.addr);
    args[4] = SetWord(&trueCcBool);
  }

  Tuple* NextATuple()
  {
    bool yield;

    if(expectSorted)
    {
      qp->Request(streamA.addr, aResult);
      yield = qp->Received(streamA.addr);
    }
    else
    {
      int errorCode = 
        SortBy<false>(aArgs, aResult, REQUEST, streamALocalInfo, 0);
      yield = (errorCode == YIELD);
    }

    if(yield)
    {
      return (Tuple*)aResult.addr;
    }
    else
    {
      aResult = SetWord((void*)0);
      return 0;
    }
  }

  Tuple* NextBTuple()
  {
    bool yield;

    if(expectSorted)
    {
      qp->Request(streamB.addr, bResult);
      yield = qp->Received(streamB.addr);
    }
    else
    {
      int errorCode = 
        SortBy<false>(bArgs, bResult, REQUEST, streamBLocalInfo, 0);
      yield = (errorCode == YIELD);
    }

    if(yield)
    {
      return (Tuple*)bResult.addr;
    }
    else
    {
      bResult = SetWord((void*)0);
      return 0;
    }
  }

  void SaveTo( vector<Tuple*>& bucket, TupleBuffer *rel )
  {
    vector<Tuple*>::iterator iter = bucket.begin();
    while( iter != bucket.end() )
    {
      (*iter)->DecReference();
      rel->AppendTuple( (*iter) );
      (*iter)->DeleteIfAllowed();
      iter++;
    }
    bucket.clear();
  }

  void ReadFrom( GenericRelationIterator *iter, vector<Tuple*>& bucket )
  {
    size_t memory = 0;
    Tuple *t = 0;

    if( (t = iter->GetNextTuple()) != 0 )
    {
      memory += t->GetExtSize();
      while( memory < MAX_MEMORY / 2)
      {
        t->IncReference();
        bucket.push_back( t );
        t = iter->GetNextTuple();
        if( t == 0 )
          break;
        memory += t->GetExtSize();
      }
    }
  }

  void ClearBucket( vector<Tuple*>& bucket )
  {
    vector<Tuple*>::iterator i = bucket.begin();
    while( i != bucket.end() )
    {
      (*i)->DecReference();
      (*i)->DeleteIfAllowed();
      i++;
    }
    bucket.clear();
  }

  size_t MAX_TUPLES_IN_MEMORY;
public:
  MergeJoinLocalInfo( Word streamA, Word attrIndexA,
                      Word streamB, Word attrIndexB, 
                      bool expectSorted, Supplier s  ) :
    traceFlag( RTFlag::isActive("ERA:TraceMergeJoin") )
  {
    this->expectSorted = expectSorted;
    this->streamA = streamA;
    this->streamB = streamB;
    this->attrIndexA = ((CcInt*)attrIndexA.addr)->GetIntval() - 1;
    this->attrIndexB = ((CcInt*)attrIndexB.addr)->GetIntval() - 1;
    this->relationA = 0;
    this->relationB = 0;
    this->aResult = SetWord(Address(0));
    this->bResult = SetWord(Address(0));
    this->MAX_MEMORY = 0;

    if(expectSorted)
    {
      qp->Open(streamA.addr);
      qp->Open(streamB.addr);
    }
    else
    {
      SetArgs(aArgs, streamA, attrIndexA);
      SetArgs(bArgs, streamB, attrIndexB);
      SortBy<false>(aArgs, aResult, OPEN, streamALocalInfo, 0);
      SortBy<false>(bArgs, bResult, OPEN, streamBLocalInfo, 0);
    }

    ListExpr resultType =
                SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    Tuple *tupleA = NextATuple();
    Tuple *tupleB = NextBTuple();

    if( tupleA != 0 && tupleB != 0 )
    {
      MAX_MEMORY = qp->MemoryAvailableForOperator();

      cmsg.info("ERA:ShowMemInfo")
        << "MergeJoin.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
      cmsg.send();
    }
  }

  ~MergeJoinLocalInfo()
  {
    ClearBucket( bucketA );
    ClearBucket( bucketB );

    if(expectSorted)
    {
      qp->Close(streamA.addr);
      qp->Close(streamB.addr);
    }
    else
    {
      SortBy<false>(aArgs, aResult, CLOSE, streamALocalInfo, 0);
      SortBy<false>(bArgs, bResult, CLOSE, streamBLocalInfo, 0);
    }
    resultTupleType->DeleteIfAllowed();
  }

  Tuple *NextResultTuple()
  {
    Tuple *tupleA = 0, *tupleB = 0;
    Tuple *resultTuple = 0;

    if( bucketA.size() > 0 )
    // There are equal tuples that fit in memory for the bucket A.
    {
      if( indexB == bucketB.size() )
      {
        indexB = 0;
        indexA++;
      }

      if( indexA == bucketA.size() )
      {
        if( relationB != 0 )
        {
          ClearBucket( bucketB ); indexB = 0;
          ReadFrom( iterRelationB, bucketB );

          if( bucketB.empty() )
          {
            ClearBucket( bucketA ); indexA = 0;

            if( relationA != 0 )
            {
              ReadFrom( iterRelationA, bucketA );
              if( bucketA.empty() )
              {
                delete iterRelationA;
                delete relationA; relationA = 0;
                delete iterRelationB;
                delete relationB; relationB = 0;
              }
              else
              {
                indexA = 0;
                delete iterRelationB;
                iterRelationB = relationB->MakeScan();
                ReadFrom( iterRelationB, bucketB );
                indexB = 0;
              }
              resultTuple = NextResultTuple();
            }
            else
            {
              delete iterRelationB;
              delete relationB; relationB = 0;
              resultTuple = NextResultTuple();
            }
          }
          else
          {
            indexA = 0; indexB = 0;
            resultTuple = NextResultTuple();
          }
        }
        else if( relationA != 0 )
        {
          ClearBucket( bucketA ); indexA = 0;
          ReadFrom( iterRelationA, bucketA );
          if( bucketA.empty() )
          {
            ClearBucket( bucketB ); indexB = 0;
            delete iterRelationA;
            delete relationA; relationA = 0;
          }
          else
          {
            indexA = 0;
            indexB = 0;
          }
          resultTuple = NextResultTuple();
        }
        else
        {
          ClearBucket( bucketA ); indexA = 0;
          ClearBucket( bucketB ); indexB = 0;

          resultTuple = NextResultTuple();
        }
      }
      else
      {
        resultTuple = new Tuple( resultTupleType );
        Concat( bucketA[indexA], bucketB[indexB++], resultTuple );
      }
    }
    else
    // There are no stored equal tuples.
    {
      tupleA = (Tuple *)aResult.addr;
      tupleB = (Tuple *)bResult.addr;

      if( tupleA == 0 || tupleB == 0 )
      // One of the streams finished.
      {
        if( tupleA != 0 )
          tupleA->DeleteIfAllowed();
        if( tupleB != 0 )
          tupleB->DeleteIfAllowed();
        return 0;
      }

      int cmp = CompareTuples( tupleA, tupleB );
     
      if (traceFlag) 
      { 
        cmsg.info() 
          << "CompareTuples:" << endl
          << "  tupA = " << *tupleA << endl
          << "  tupB = " << *tupleB << endl 
          << "  cmp  = " << cmp << endl; 
        cmsg.send(); 
      }

      if( cmp == 0 )
      // The tuples are equal. We must store them in a buffer if it fits
      // or on disk otherwise
      {
        Tuple *equalTupleB = tupleB;
        Tuple *equalTupleA = tupleA;

        tupleA->IncReference();
        bucketA.push_back( tupleA );
        tupleB->IncReference();
        bucketB.push_back( tupleB );

        size_t bucketASize = tupleA->GetExtSize(),
               bucketBSize = tupleB->GetExtSize();

        tupleA = NextATuple();
        if ( tupleA && traceFlag ) 
        {
          cmsg.info() 
            << "Store tuples from A which are equal to tupB:" << endl;
          cmsg.send();
        }

        while( tupleA != 0 && CompareTuples( tupleA, equalTupleB ) == 0 )
        {
          if (traceFlag) { 
            cmsg.info() << "  nextA: " << *(tupleA) << endl;
                        //<< "   cmpB: " << *(equalTupleB ) << endl;
            cmsg.send();
          }  

          bucketASize += tupleA->GetExtSize();
          if( bucketASize > MAX_MEMORY / 2 )
          {
            relationA = new TupleBuffer( /*tupleA->GetTupleType(), true*/ );
            SaveTo( bucketA, relationA );
            ClearBucket( bucketA );
          }
          if( bucketA.size() > 0 )
          {
            tupleA->IncReference();
            bucketA.push_back( tupleA );
          }
          else
          {
            relationA->AppendTuple( tupleA );
            tupleA->DeleteIfAllowed();
          }
          tupleA = NextATuple();
        }
        indexA = 0;

        if( bucketA.size() == 0 )
        {
          iterRelationA = relationA->MakeScan();
          ReadFrom( iterRelationA, bucketA );
        }

        tupleB = NextBTuple();
        if ( tupleB && traceFlag )
        {
          cmsg.info() 
            << "Store tuples from B which are equal to tupA:" << endl;
          cmsg.send();
        }

        while( tupleB != 0 && CompareTuples( equalTupleA, tupleB ) == 0 )
        {
         if (traceFlag) { 
            cmsg.info() << "  nextB: " << *(tupleB) << endl;
                        //<< "   cmpB: " << *(equalTupleB ) << endl;
            cmsg.send();
          }  

          bucketBSize += tupleB->GetExtSize();
          if( bucketBSize > MAX_MEMORY / 2 )
          {
            relationB = new TupleBuffer( /*tupleB->GetTupleType(), true*/ );
            SaveTo( bucketB, relationB );
            ClearBucket( bucketB );
          }
          if( bucketB.size() > 0 )
          {
            tupleB->IncReference();
            bucketB.push_back( tupleB );
          }
          else
          {
            relationB->AppendTuple( tupleB );
            tupleB->DeleteIfAllowed();
          }
          tupleB = NextBTuple();
        }
        indexB = 0;

        if( bucketB.size() == 0 )
        {
          iterRelationB = relationB->MakeScan();
          ReadFrom( iterRelationB, bucketB );
        }

        if( bucketA.size() == 1 && bucketB.size() == 1 )
        // Only one equal tuple.
        {
          resultTuple = new Tuple( resultTupleType );
          Concat( bucketA[0], bucketB[0], resultTuple );
          ClearBucket( bucketA );
          ClearBucket( bucketB );
        }
        else
        {
          resultTuple = NextResultTuple();
        }
      }
      else if( cmp > 0 )
      {
        tupleB->DeleteIfAllowed();
        tupleB = NextBTuple();
        while( tupleB != 0 && CompareTuples( tupleA, tupleB ) > 0 )
        {
          tupleB->DeleteIfAllowed();
          tupleB = NextBTuple();
        }
        resultTuple = NextResultTuple();
      }
      else if( cmp < 0 )
      {
        tupleA->DeleteIfAllowed();
        tupleA = NextATuple();
        while( tupleA != 0 && CompareTuples( tupleA, tupleB ) < 0 )
        {
          tupleA->DeleteIfAllowed();
          tupleA = NextATuple();
        }
        resultTuple = NextResultTuple();
      }
    }
    return resultTuple;
  }
};

/*
2.2.2 Value mapping function of operator ~mergejoin~

*/

template<bool expectSorted> int
MergeJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  MergeJoinLocalInfo* localInfo;

  switch(message)
  {
    case OPEN:
      localInfo = new MergeJoinLocalInfo
        (args[0], args[4], args[1], args[5], expectSorted, s);
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      mergeMeasurer.Enter();
      localInfo = (MergeJoinLocalInfo*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      mergeMeasurer.Exit();
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      mergeMeasurer.PrintCPUTimeAndReset("CPU Time for Merging Tuples : ");

      localInfo = (MergeJoinLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

/*
2.3 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.3.1 Auxiliary definitions for value mapping function of operator ~hashjoin~

*/
class HashJoinLocalInfo
{
private:
  size_t nBuckets;

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;
  bool streamAClosed;
  bool streamBClosed;

  Tuple *tupleA;
  TupleBuffer* relA;
  GenericRelationIterator* iterTuplesRelA;
  size_t relA_Mem;
  bool firstPassA;
  bool memInfoShown;
  size_t hashA;

  vector< vector<Tuple*> > bucketsB;
  vector<Tuple*>::iterator iterTuplesBucketB;
  size_t bucketsB_Mem;
  bool remainTuplesB, bFitsInMemory;
  Word wTupleB;

  TupleType *resultTupleType;

  int CompareTuples(Tuple* a, Tuple* b)
  {
    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if(!((Attribute*)a->GetAttribute(attrIndexA))->IsDefined())
    {
      return -1;
    }
    if(!((Attribute*)b->GetAttribute(attrIndexB))->IsDefined())
    {
      return 1;
    }

    return ((Attribute*)a->GetAttribute(attrIndexA))->
      Compare((Attribute*)b->GetAttribute(attrIndexB));
  }

  size_t HashTuple(Tuple* tuple, int attrIndex)
  {
    return 
      (((StandardAttribute*)tuple->GetAttribute(attrIndex))->HashValue() % 
      nBuckets);
  }

  void ClearBucket( vector<Tuple*>& bucket )
  {
    vector<Tuple*>::iterator i = bucket.begin();
    while( i != bucket.end() )
    {
      (*i)->DecReference();
      (*i)->DeleteIfAllowed();
      i++;
    }
    bucket.clear();
  }

  void ClearBucketsB()
  {
    vector< vector<Tuple*> >::iterator iterBuckets = bucketsB.begin();

    while(iterBuckets != bucketsB.end() )
    {
      ClearBucket( *iterBuckets );
      iterBuckets++;
    }
  }

  bool FillHashBucketsB()
  {
    if( firstPassA )
    {
      qp->Request(streamB.addr, wTupleB);
      if(qp->Received(streamB.addr))
      {
        // reserve 3/4 of memory for buffering tuples of B;
        // Before retrieving the allowed memory size from the
        // configuration file it was set to 12MB for B and 4MB for A (see below)
        bucketsB_Mem = (3 * qp->MemoryAvailableForOperator())/4;
        relA_Mem = qp->MemoryAvailableForOperator()/4;

        cmsg.info("ERA:ShowMemInfo")
          << "HashJoin.MAX_MEMORY ("
          << qp->MemoryAvailableForOperator()/1024
          << " kb - A: " << relA_Mem/1024 << "kb B: "
          << bucketsB_Mem/1024 << "kb)" << endl
          << "Stream A is stored in a Tuple Buffer" << endl;
        cmsg.send();
      }
    }

    size_t b = 0, i = 0;
    while(qp->Received(streamB.addr) )
    {
      Tuple* tupleB = (Tuple*)wTupleB.addr;
      b += tupleB->GetExtSize();
      i++;
      if( b > bucketsB_Mem )
      {
        cmsg.info("ERA:ShowMemInfo")
          << "HashJoin - Stream B does not fit in memory" << endl
          << "Memory used up to now: " << b / 1024 << "kb" << endl
          << "Tuples in memory: " << i << endl;
        cmsg.send();

        break;
      }

      size_t hashB = HashTuple(tupleB, attrIndexB);
      tupleB->IncReference();
      bucketsB[hashB].push_back( tupleB );
      qp->Request(streamB.addr, wTupleB);
    }

    bool remainTuples = false;
    if( b > bucketsB_Mem && qp->Received(streamB.addr) )
      remainTuples = true;

    if( !remainTuples )
    {
      qp->Close(streamB.addr);
      streamBClosed = true;
    }

    return remainTuples;
  }

public:
  static const size_t MIN_BUCKETS = 3;
  static const size_t DEFAULT_BUCKETS = 97;

  HashJoinLocalInfo(Word streamA, Word attrIndexAWord,
    Word streamB, Word attrIndexBWord, Word nBucketsWord,
    Supplier s)
  {
    memInfoShown = false;
    this->streamA = streamA;
    this->streamB = streamB;

    ListExpr resultType = 
      SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    attrIndexA = ((CcInt*)attrIndexAWord.addr)->GetIntval() - 1;
    attrIndexB = ((CcInt*)attrIndexBWord.addr)->GetIntval() - 1;
    nBuckets = ((CcInt*)nBucketsWord.addr)->GetIntval();
    if(nBuckets > qp->MemoryAvailableForOperator() / 1024)
      nBuckets = qp->MemoryAvailableForOperator() / 1024;
    if(nBuckets < MIN_BUCKETS)
      nBuckets = MIN_BUCKETS;

    bucketsB.resize(nBuckets);
    relA = 0;
    iterTuplesRelA = 0;
    firstPassA = true;
    tupleA = 0;

    qp->Open(streamB.addr);
    streamBClosed = false;
    remainTuplesB = FillHashBucketsB();
    bFitsInMemory  = !remainTuplesB;

    if( !bFitsInMemory ) 
      // reserve 1/4 of the allowed memory for buffering tuples of A
      relA = new TupleBuffer( relA_Mem );

    qp->Open(streamA.addr);
    streamAClosed = false;
    NextTupleA();
/*
At this moment we have a tuple of the stream A and a hash table in memory
of the stream B. There is a possibility that the stream B does not fit in
memory, which is kept in the variable ~bFitsInMemory~. The iterator for the
bucket that the tuple coming from A hashes is also initialized.

*/
  }

  ~HashJoinLocalInfo()
  {
    ClearBucketsB();

    // delete tuple buffer and its iterator if necessary
    if( !bFitsInMemory )
    {
      if ( iterTuplesRelA )
        delete iterTuplesRelA;
      relA->Clear();
      delete relA;
    }

    // close open streams if necessary
    if ( !streamAClosed )
      qp->Close(streamA.addr);
    if ( !streamBClosed )
      qp->Close(streamB.addr);

    resultTupleType->DeleteIfAllowed();
  }

  bool NextTupleA()
  {
    if( tupleA != 0 )
    {
      if( firstPassA && !bFitsInMemory ) {
        relA->AppendTuple( tupleA );
      }	
      tupleA->DeleteIfAllowed();
    }

    if( firstPassA )
    {
      Word wTupleA;
      qp->Request( streamA.addr, wTupleA );
      if( qp->Received(streamA.addr) )
      {
        tupleA = (Tuple*)wTupleA.addr;
        if (!memInfoShown) 
        {
          cmsg.info("ERA:ShowMemInfo") 
            << "TupleBuffer for relA can hold " 
            << relA_Mem / tupleA->GetExtSize() << " tuples" << endl;
          cmsg.send();
          memInfoShown = true;
        }
      }
      else
      {
        tupleA = 0;
        qp->Close(streamA.addr);
        streamAClosed = true;
        return false;
      }
    }
    else
    {
      if( (tupleA = iterTuplesRelA->GetNextTuple()) == 0 )
      {
        delete iterTuplesRelA;
        iterTuplesRelA = 0;
        return false;
      }
    }

    hashA = HashTuple( tupleA, attrIndexA );
    iterTuplesBucketB = bucketsB[hashA].begin();
    return true;
  }

  Tuple* NextResultTuple()
  {
    while( tupleA != 0 )
    {
      while( iterTuplesBucketB != bucketsB[hashA].end() )
      {
        Tuple *tupleB = *iterTuplesBucketB++;

        if( CompareTuples( tupleA, tupleB ) == 0 )
        {
          Tuple *result = new Tuple( resultTupleType );
          Concat( tupleA, tupleB, result );
          return result;
        }
      }

      if( !NextTupleA() )
      {
        if( remainTuplesB )
        {
          firstPassA = false;
          ClearBucketsB();
          remainTuplesB = FillHashBucketsB();
          iterTuplesRelA = relA->MakeScan();
          NextTupleA();
        }
      }
    }

    return 0;
  }
};

/*
2.3.2 Value Mapping Function of Operator ~hashjoin~

*/
int HashJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  HashJoinLocalInfo* localInfo;

  switch(message)
  {
    case OPEN:
      localInfo = 
        new HashJoinLocalInfo(args[0], args[5], args[1], args[6], args[4], s);
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (HashJoinLocalInfo*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      localInfo = (HashJoinLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}


/*
3 Initialization of the templates

The compiler cannot expand these template functions.

*/
template int
SortBy<false>(Word* args, Word& result, int message, 
              Word& local, Supplier s);
template int
SortBy<true>(Word* args, Word& result, int message, 
             Word& local, Supplier s);
template int
MergeJoin<true>(Word* args, Word& result, int message, 
                Word& local, Supplier s);
template int
MergeJoin<false>(Word* args, Word& result, int message, 
                 Word& local, Supplier s);

