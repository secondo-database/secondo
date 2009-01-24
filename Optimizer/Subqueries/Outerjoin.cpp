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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]



[1] Implementation of Outerjoin for Module Extended Relation Algebra

[1] Using Storage Manager Berkeley DB

January 2008, B. Poneleit 

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
#include "RTuple.h"
#include "Tupleorder.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#ifndef USE_PROGRESS

//-- begin standard version --//

class SortByLocalInfo
{
  public:
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


        MAX_MEMORY = qp->MemoryAvailableForOperator();
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
          if( MAX_MEMORY > (size_t)t->GetSize() )
          {
            nextTuple.tuple->IncReference();
            currentRun->push(nextTuple);
            i++; // increment Tuples in memory counter
            MAX_MEMORY -= t->GetSize();
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

    ~SortByLocalInfo()
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

    Tuple *NextResultTuple()
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

  private:

    void ShowPartitionInfo( int c, int a, int n, 
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


//-- end standard version --//

#else

//-- begin progress version --//

class SortByLocalInfo : protected ProgressWrapper
{
  public:
    SortByLocalInfo( Word stream, const bool lexicographic,
		     void *tupleCmp, ProgressLocalInfo* p            ):
      ProgressWrapper(p),
      stream( stream ),
      currentIndex( 0 ),
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
        size_t  c = 0, i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
        bool newRelation = true;


        MAX_MEMORY = qp->MemoryAvailableForOperator();
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
          c++; // tuple counter;
          Tuple *t = static_cast<Tuple*>( wTuple.addr );
          TupleAndRelPos nextTuple(t, tupleCmpBy);
          if( MAX_MEMORY > (size_t)t->GetSize() )
          {
            //nextTuple.tuple->IncReference();
            currentRun->push(nextTuple);
            i++; // increment Tuples in memory counter
            MAX_MEMORY -= t->GetSize();
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
                  ShowPartitionInfo(c,a,n,m,r,rel);
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
        ShowPartitionInfo(c,a,n,m,r,rel);

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

    ~SortByLocalInfo()
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

    Tuple *NextResultTuple()
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

  private:

    void ShowPartitionInfo( int c, int a, int n,
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

    Word stream;
    size_t currentIndex;

    // tuple information
    LexicographicalTupleSmaller *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;

    // sorted runs created by in memory heap filtering
    size_t MAX_MEMORY;
    typedef pair<TupleBuffer*, GenericRelationIterator*> SortedRun;
    vector< SortedRun > relations;

    typedef TupleQueue Heap;
    Heap queue[2];
    Heap mergeTuples;



};
//-- end progress version --//

#endif

/*
2.2 Operator ~smouterjoin~

This operator computes the equijoin of two streams. It uses a text book
algorithm as outlined in A. Silberschatz, H. F. Korth, S. Sudarshan,
McGraw-Hill, 3rd. Edition, 1997.

2.2.1 Auxiliary definitions for value mapping function of operator ~smouterjoin~

*/

#ifndef USE_PROGRESS
/*
2.2.2 Value mapping function of operator ~mergejoin~

*/

//-- begin standard version --//

class MergeOuterjoinLocalInfo
{
private:

  // buffer limits	
  size_t MAX_MEMORY;
  size_t MAX_TUPLES_IN_MEMORY;

  // buffer related members
  TupleBuffer *grpB;
  GenericRelationIterator *iter;

  // members needed for sorting the input streams
  LocalInfo<SortByLocalInfo>* liA;
  SortByLocalInfo* sliA;

  LocalInfo<SortByLocalInfo>* liB;
  SortByLocalInfo* sliB;

  Word streamA;
  Word streamB;

  // the current pair of tuples
  Word resultA;
  Word resultB;

  RTuple ptA;
  RTuple ptB;
  RTuple tmpB;

  // the last comparison result
  int cmp;

  // the indexes of the attributes which will
  // be merged and the result type
  int attrIndexA;
  int attrIndexB;

  TupleType *resultTupleType;

  // a flag which indicates if sorting is needed
  bool expectSorted;

  // switch trace messages on/off
  const bool traceFlag; 

  // a flag needed in function NextTuple which tells
  // if the merge with grpB has been finished
  bool continueMerge;

  template<bool BOTH_B>
  int CompareTuples(Tuple* t1, Tuple* t2)
  {

    Attribute* a = 0; 	  
    if (BOTH_B)    
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexB) );
    else
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexA) );

    Attribute* b = static_cast<Attribute*>( t2->GetAttribute(attrIndexB) );

    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if( !a->IsDefined() )
    {
      return -1;
    }
    if( !b->IsDefined() )
    {
      return 1;
    }

    int cmp = a->Compare(b);
    if (traceFlag) 
    { 
          cmsg.info() 
            << "CompareTuples:" << endl
	    << "  BOTH_B = " << BOTH_B << endl
            << "  tuple_1  = " << *t1 << endl
            << "  tuple_2  = " << *t2 << endl 
            << "  cmp(t1,t2) = " << cmp << endl; 
          cmsg.send(); 
    }
    return cmp;
  }

  inline int CompareTuplesB(Tuple* t1, Tuple* t2) 
  {
    return CompareTuples<true>(t1, t2);
  }

  inline int CompareTuples(Tuple* t1, Tuple* t2) 
  {
    return CompareTuples<false>(t1, t2);
  }

  inline Tuple* NextTuple(Word stream, SortByLocalInfo* sli)
  {
    bool yield = false;
    Word result( Address(0) );

    if(!expectSorted)
      return sli->NextResultTuple();

    qp->Request(stream.addr, result);
    yield = qp->Received(stream.addr);

    if(yield)
    {
      return static_cast<Tuple*>( result.addr );
    }
    else
    {
      result.addr = 0;	    
      return static_cast<Tuple*>( result.addr );
    }
  }

  inline Tuple* NextTupleA()
  {
    return NextTuple(streamA, sliA);
  }  

  inline Tuple* NextTupleB()
  {
    return NextTuple(streamB, sliB);
  }  


inline Tuple* NextUndefinedA()
{
    Tuple* result = 0;
    Tuple* r = 0;

    AttributeType attrType;
    ListExpr valueExpr = 0;
    bool correct;
    int errorPos;
    ListExpr errorInfo;	
    progress->readFirst++;
    if (undefA == 0) {
        // create tuple with undefined values
        ListExpr tupleType =
        nl->Second(SecondoSystem::GetCatalog()->NumericType(
            qp->GetType(streamA.addr)));
        result = new Tuple( tupleTypeA );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
            attrType = tupleTypeA->GetAttributeType( i );
            string typeName =
            SecondoSystem::GetCatalog()->GetTypeName(
                attrType.algId, attrType.typeId );

            if ( valueExpr == 0 )
            valueExpr = nl->OneElemList(nl->SymbolAtom("undef"));
            else {
            valueExpr= nl->Cons(nl->SymbolAtom("undef"),valueExpr );
            }
        }
        ListExpr typeInfo = nl->TwoElemList( tupleType, valueExpr );
        r =
        result->In(typeInfo,valueExpr,errorPos,errorInfo,correct);
        undefA = r;
        result->DeleteIfAllowed();
    }
    else {
    r = undefA;
    }
    return r;
}

inline Tuple* NextUndefinedB()
{
    Tuple* result = 0;
    Tuple* r = 0;

    AttributeType attrType;
    ListExpr valueExpr = 0;
    bool correct;
    int errorPos;
    ListExpr errorInfo;	
    progress->readSecond++;
    if (undefB == 0)  {
        // create tuple with undefined values
        ListExpr tupleType =
        nl->Second(SecondoSystem::GetCatalog()->NumericType(
            qp->GetType( streamB.addr )) );
        result = new Tuple( tupleTypeB );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
            attrType = tupleTypeB->GetAttributeType( i );
            string typeName =
              SecondoSystem::GetCatalog()->GetTypeName(
                attrType.algId, attrType.typeId );
            if ( valueExpr == 0 )
            valueExpr = nl->OneElemList(nl->SymbolAtom("undef"));
            else {
            valueExpr = nl->Cons( nl->SymbolAtom("undef"),valueExpr);
            }
        }
        ListExpr typeInfo = nl->TwoElemList( tupleType, valueExpr );
        r = result->In(typeInfo,valueExpr,errorPos,errorInfo,correct);
        undefB = r;
        result->DeleteIfAllowed();
    }
    else {
    r = undefB;
    }
    return r;
}  


public:
  MergeOuterjoinLocalInfo( Word _streamA, Word wAttrIndexA,
                      Word _streamB, Word wAttrIndexB, 
                      bool _expectSorted, Supplier s  ) :
    traceFlag( RTFlag::isActive("ERA:TraceMergeOuterjoin") )
  {
    expectSorted = _expectSorted;
    streamA = _streamA;
    streamB = _streamB;
    attrIndexA = StdTypes::GetInt( wAttrIndexA ) - 1;
    attrIndexB = StdTypes::GetInt( wAttrIndexB ) - 1;
    MAX_MEMORY = 0;

    sliA = 0;
    sliB = 0; 

    if( !expectSorted )
    {
      // sort the input streams

      SortOrderSpecification specA;
      SortOrderSpecification specB;
	 
      specA.push_back( pair<int, bool>(attrIndexA + 1, true) ); 
      specB.push_back( pair<int, bool>(attrIndexB + 1, true) ); 


      void* tupleCmpA = new TupleCompareBy( specA );
      void* tupleCmpB = new TupleCompareBy( specB );

      sliA = new SortByLocalInfo( streamA, 
				  false,  
				  tupleCmpA );

      sliB = new SortByLocalInfo( streamB, 
				  false,  
				  tupleCmpB );

    }

    ListExpr resultType =
                SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    // read in the first tuple of both input streams
    ptA = RTuple( NextTupleA() );
    ptB = RTuple( NextTupleB() );

    // initialize the status for the result
    // set iteration   
    tmpB = 0;
    cmp = 0;
    continueMerge = false;

    MAX_MEMORY = qp->MemoryAvailableForOperator();
    grpB = new TupleBuffer( MAX_MEMORY );

    cmsg.info("ERA:ShowMemInfo")
      << "MergeOuterjoin.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
    cmsg.send();

  }

  ~MergeOuterjoinLocalInfo()
  {
    if( !expectSorted )
    {
      // delete the objects instantiated for sorting
      delete sliA;
      delete sliB;
    }

    delete grpB;
    resultTupleType->DeleteIfAllowed();
  }

  Tuple* NextResultTuple()
  {
      Tuple* resultTuple = 0;	

      while ( ptA != 0 || ptB != 0 ) {		
          if (ptA != 0 && (ptB != 0 || tmpB != 0)) {			
              if (tmpB != 0) {
                  cmp = CompareTuples( ptA.tuple, tmpB.tuple );
              }
              else
                  cmp = CompareTuples( ptA.tuple, ptB.tuple );

              /*if ( !outerJoin && tmpB == 0 ) {
                  // create initial group for inner join
                  if (!continueMerge && ptB != 0) {
                      //save ptB in tmpB	
                      tmpB = ptB;

                      grpB->AppendTuple(tmpB.tuple);

                      // advance the tuple pointer
                      ptB.setTuple( NextTupleB() );

                      // collect a group of tuples from B which
                      // have the same attribute value
                      bool done = false;
                      while ( !done && ptB != 0 ) {
                          int cmp = CompareTuplesB( tmpB.tuple, ptB.tuple );

                          if ( cmp == 0) 	{
                              // append equal tuples to group	
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else		{
                              done = true;
                          }	
                      } // end collect group

                      cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                  }
              }*/

              if ( cmp < 0 ) {
                  // ptA < ptB
                  //if ( outerJoin ) {
                      continueMerge = false;
                      resultTuple = NextConcatB();
                      ptA.setTuple( NextTupleA() );
                      if (resultTuple) {
                          return resultTuple;
                      }
                  //}
                  /*else {
                      ptA.setTuple( NextTupleA() );

                      cmp = CompareTuples( ptA.tuple, tmpB.tuple );

                      while ( ptA != 0 && cmp < 0 ) {
                          // skip tuples from A while they are smaller than the
                          // value of the tuples in grpB
                          ptA.setTuple( NextTupleA() );
                          if (ptA != 0)
                              cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                      }
                  }*/
              }
              else if ( cmp == 0 ) {
                  if (!continueMerge && tmpB == 0) {
                      //save ptB in tmpB
                      tmpB = ptB;

                      grpB->AppendTuple(tmpB.tuple);

                      // advance the tuple pointer
                      ptB.setTuple( NextTupleB() );

                      // collect a group of tuples from B which
                      // have the same attribute value
                      bool done = false;
                      while ( !done && ptB != 0 ) {
                          //cout << "grpB" << endl;
                          //logTuples();
                          int cmp = CompareTuplesB( tmpB.tuple, ptB.tuple );

                          if ( cmp == 0) {
                              // append equal tuples to group
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else  {
                              done = true;
                          }	
                      } // end collect group
                  }

                  // continue or start merge
                  if (!continueMerge) {
                      iter = grpB->MakeScan();
                      continueMerge = true;
                      resultTuple = NextConcat();
                      if (resultTuple) {
                          return resultTuple;
                      }
                  } else {
                      //continue merging, create next result tuple
                      resultTuple = NextConcat();
                      if (resultTuple) {
                          return resultTuple;
                      }
                      else {
                          continueMerge = false;
                          delete iter;
                          iter = 0;
                          ptA.setTuple( NextTupleA() );
                          if (ptA != 0) {
                              cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                              if (/*outerJoin && */cmp != 0) {
                                  tmpB = 0;
                                  grpB->Clear();
                              }
                          }

                      }
                  }
              } // end of merge
              else /*if ( outerJoin )*/ {
                  // ptA > ptB
                  continueMerge = false;
                  resultTuple = NextConcatA();
                  ptB.setTuple( NextTupleB() );
                  if (resultTuple) {
                      return resultTuple;
                  }		
              }
              /*else {
                  if ( ptB == 0 )
                      // short exit
                      return 0;

                  grpB->Clear();
                  tmpB = 0;
              }*/
          } // end of
          else if ( /*outerJoin && */ptA != 0 ) {
              // ptB == 0
              resultTuple = NextConcatB();
              ptA.setTuple( NextTupleA() );
              if (resultTuple) {
                  return resultTuple;
              }	
          }
          else /*if ( outerJoin ) / * ptB != 0 */{
              // ptA == 0
              resultTuple = NextConcatA();
              ptB.setTuple( NextTupleB() );
              if (resultTuple) {
                  return resultTuple;
              }	
          }
          else {
              // short exit
              return 0;
          }
      } // end of main loop
      return 0;
  }


  inline Tuple* NextConcat() 
  {
    Tuple* t = iter->GetNextTuple();
    if( t != 0 ) {

     Tuple* result = new Tuple( resultTupleType );
     Concat( ptA.tuple, t, result );
     return result;  
    }
    return 0;
  }
  
  inline Tuple* NextConcatB()
  {
     Tuple* result = new Tuple( resultTupleType );
     Concat( ptA.tuple, NextUndefinedB(), result );
     return result;
  }

  inline Tuple* NextConcatA()
  {	
      Tuple* result = new Tuple( resultTupleType );
      Concat( NextUndefinedA(), ptB.tuple, result );
      return result;
  }  

};

/*
2.2.2 Value mapping function of operator ~mergeOuterjoin~

*/


//CPUTimeMeasurer mergeMeasurer;

template<bool expectSorted> int
smouterjoin_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{
  MergeOuterjoinLocalInfo* localInfo;

  switch(message)
  {
    case OPEN:
      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      localInfo = new MergeOuterjoinLocalInfo
        (args[0], args[4], args[1], args[5], expectSorted, s);
      local.setAddr(localInfo);
      return 0;

    case REQUEST:
      //mergeMeasurer.Enter();
      localInfo = (MergeOuterjoinLocalInfo*)local.addr;
      result.setAddr(localInfo->NextResultTuple());
      //mergeMeasurer.Exit();
      return result.addr != 0 ? YIELD : CANCEL;

    case CLOSE:
      //mergeMeasurer.PrintCPUTimeAndReset("CPU Time for Merging Tuples : ");

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      localInfo = (MergeOuterjoinLocalInfo*)local.addr;
      delete localInfo;
      local.addr = 0;
      return 0;
  }
  return 0;
}

//-- end standard version --//

#else

//-- begin progress version --//

class MergeOuterjoinLocalInfo: protected ProgressWrapper
{
private:

  // buffer limits	
  size_t MAX_MEMORY;
  size_t MAX_TUPLES_IN_MEMORY;

  // buffer related members
  TupleBuffer *grpB;
  GenericRelationIterator *iter;

  // members needed for sorting the input streams
  LocalInfo<SortByLocalInfo>* liA;
  SortByLocalInfo* sliA;

  LocalInfo<SortByLocalInfo>* liB;
  SortByLocalInfo* sliB;

  Word streamA;
  Word streamB;

  // the current pair of tuples
  Word resultA;
  Word resultB;

  RTuple ptA;
  RTuple ptB;
  RTuple tmpB;
  RTuple lastB;

  Tuple* undefA;
  Tuple* undefB;

  // the last comparison result
  int cmp;

  // the indexes of the attributes which will
  // be merged and the result type
  int attrIndexA;
  int attrIndexB;

  TupleType *resultTupleType;
  TupleType *tupleTypeB;
  TupleType *tupleTypeA;

  // a flag which indicates if sorting is needed
  bool expectSorted;

  // switch trace messages on/off
  const bool traceFlag;

  // a flag needed in function NextTuple which tells
  // if the merge with grpB has been finished
  bool continueMerge;

  bool continueUndefB;
  bool continueUndefA;

  template<bool BOTH_B>
  int CompareTuples(Tuple* t1, Tuple* t2)
  {

    Attribute* a = 0; 	
    if (BOTH_B)
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexB) );
    else
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexA) );

    Attribute* b = static_cast<Attribute*>( t2->GetAttribute(attrIndexB) );

    /* tuples with NULL-Values in the join attributes
       are never matched with other tuples. */
    if( !a->IsDefined() )
    {
      return -1;
    }
    if( !b->IsDefined() )
    {
      return 1;
    }

    int cmp = a->Compare(b);
    if (traceFlag)
    {
          cmsg.info()
            << "CompareTuples:" << endl
        << "  BOTH_B = " << BOTH_B << endl
            << "  tuple_1  = " << *t1 << endl
            << "  tuple_2  = " << *t2 << endl
            << "  cmp(t1,t2) = " << cmp << endl;
          cmsg.send();
    }
    return cmp;
  }

  inline int CompareTuplesB(Tuple* t1, Tuple* t2)
  {
    return CompareTuples<true>(t1, t2);
  }

  inline int CompareTuples(Tuple* t1, Tuple* t2)
  {
    return CompareTuples<false>(t1, t2);
  }

  inline Tuple* NextTuple(Word stream, SortByLocalInfo* sli)
  {
    bool yield = false;
    Word result( Address(0) );

    if(!expectSorted)
      return sli->NextResultTuple();

    qp->Request(stream.addr, result);
    yield = qp->Received(stream.addr);

    if(yield)
    {
      return static_cast<Tuple*>( result.addr );
    }
    else
    {
      result.addr = 0;	
      return static_cast<Tuple*>( result.addr );
    }
  }

  inline Tuple* NextTupleA()
  {
    progress->readFirst++;
    return NextTuple(streamA, sliA);
  }

  inline Tuple* NextTupleB()
  {
    progress->readSecond++;
    return NextTuple(streamB, sliB);
  }

inline Tuple* NextUndefinedA()
{
    Tuple* result = 0;
    Tuple* r = 0;

    AttributeType attrType;
    ListExpr valueExpr = 0;
    bool correct;
    int errorPos;
    ListExpr errorInfo;	
    progress->readFirst++;
    if (undefA == 0) {
        // create tuple with undefined values
        ListExpr tupleType =
        nl->Second(SecondoSystem::GetCatalog()->NumericType(
            qp->GetType(streamA.addr)));
        result = new Tuple( tupleTypeA );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
            attrType = tupleTypeA->GetAttributeType( i );
            string typeName =
            SecondoSystem::GetCatalog()->GetTypeName(
                attrType.algId, attrType.typeId );

            if ( valueExpr == 0 )
            valueExpr = nl->OneElemList(nl->SymbolAtom("undef"));
            else {
            valueExpr= nl->Cons(nl->SymbolAtom("undef"),valueExpr );
            }
        }
        ListExpr typeInfo = nl->TwoElemList( tupleType, valueExpr );
        r =
        result->In(typeInfo,valueExpr,errorPos,errorInfo,correct);
        undefA = r;
        result->DeleteIfAllowed();
    }
    else {
    r = undefA;
    }
    return r;
}

inline Tuple* NextUndefinedB()
{
    Tuple* result = 0;
    Tuple* r = 0;

    AttributeType attrType;
    ListExpr valueExpr = 0;
    bool correct;
    int errorPos;
    ListExpr errorInfo;	
    progress->readSecond++;
    if (undefB == 0)  {
        // create tuple with undefined values
        ListExpr tupleType =
        nl->Second(SecondoSystem::GetCatalog()->NumericType(
            qp->GetType( streamB.addr )) );
        result = new Tuple( tupleTypeB );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
            attrType = tupleTypeB->GetAttributeType( i );
            string typeName =
              SecondoSystem::GetCatalog()->GetTypeName(
                attrType.algId, attrType.typeId );
            if ( valueExpr == 0 )
            valueExpr = nl->OneElemList(nl->SymbolAtom("undef"));
            else {
            valueExpr = nl->Cons( nl->SymbolAtom("undef"),valueExpr);
            }
        }
        ListExpr typeInfo = nl->TwoElemList( tupleType, valueExpr );
        r = result->In(typeInfo,valueExpr,errorPos,errorInfo,correct);
        undefB = r;
        result->DeleteIfAllowed();
    }
    else {
    r = undefB;
    }
    return r;
}


public:
  MergeOuterjoinLocalInfo( Word _streamA, Word wAttrIndexA,
                      Word _streamB, Word wAttrIndexB,
                      bool _expectSorted, Supplier s,
                      ProgressLocalInfo* p ) :
    ProgressWrapper(p),
    traceFlag( RTFlag::isActive("ERA:TraceMergeOuterjoin") )
  {
    expectSorted = _expectSorted;
    streamA = _streamA;
    streamB = _streamB;
    attrIndexA = StdTypes::GetInt( wAttrIndexA ) - 1;
    attrIndexB = StdTypes::GetInt( wAttrIndexB ) - 1;
    MAX_MEMORY = 0;

    liA = 0;
    sliA = 0;

    liB = 0;
    sliB = 0;

    if( !expectSorted )
    {
      // sort the input streams

      SortOrderSpecification specA;
      SortOrderSpecification specB;

      specA.push_back( pair<int, bool>(attrIndexA + 1, true) );
      specB.push_back( pair<int, bool>(attrIndexB + 1, true) );


      void* tupleCmpA = new TupleCompareBy( specA );
      void* tupleCmpB = new TupleCompareBy( specB );

      liA = new LocalInfo<SortByLocalInfo>();
      progress->firstLocalInfo = liA;
      sliA = new SortByLocalInfo( streamA,
                  false,
                  tupleCmpA, liA );

      liB = new LocalInfo<SortByLocalInfo>();
      progress->secondLocalInfo = liB;
      sliB = new SortByLocalInfo( streamB,
                  false,
                  tupleCmpB, liB );

    }

    ListExpr resultType =
      SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    // read in the first tuple of both input streams
    ptA.setTuple( NextTupleA() );
    ptB.setTuple( NextTupleB() );


    ListExpr typeA =
      SecondoSystem::GetCatalog()->NumericType(
        qp->GetType( streamA.addr ) );
    ListExpr typeB =
      SecondoSystem::GetCatalog()->NumericType(
        qp->GetType( streamB.addr ) );
    tupleTypeA = new TupleType( nl->Second( typeA ) );
    tupleTypeB = new TupleType( nl->Second( typeB ) );

    undefA = 0;
    undefB = 0;

    // initialize the status for the result
    // set iteration
    tmpB = 0;
    cmp = 0;

    lastB = 0;
    continueMerge = false;

    MAX_MEMORY = qp->MemoryAvailableForOperator();
    grpB = new TupleBuffer( MAX_MEMORY );

    cmsg.info("ERA:ShowMemInfo")
      << "MergeOuterjoin.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
    cmsg.send();

  }

  ~MergeOuterjoinLocalInfo()
  {
    if ( undefA != 0 )
      undefA->DeleteIfAllowed();
    if ( undefB != 0 )
      undefB->DeleteIfAllowed();
    if( !expectSorted )
    {
      // delete the objects instantiated for sorting
      delete sliA;
      delete sliB;
      delete liA;
      delete liB;
    }

    delete grpB;
    resultTupleType->DeleteIfAllowed();
  }

  Tuple* NextResultTuple()
  {
      Tuple* resultTuple = 0;	

      while ( ptA != 0 || ptB != 0 ) {		
          if (ptA != 0 && (ptB != 0 || tmpB != 0)) {			
              if (tmpB != 0) {
                  cmp = CompareTuples( ptA.tuple, tmpB.tuple );
              }
              else
                  cmp = CompareTuples( ptA.tuple, ptB.tuple );

              /*if ( !outerJoin && tmpB == 0 ) {
                  // create initial group for inner join
                  if (!continueMerge && ptB != 0) {
                      //save ptB in tmpB	
                      tmpB = ptB;

                      grpB->AppendTuple(tmpB.tuple);

                      // advance the tuple pointer
                      ptB.setTuple( NextTupleB() );

                      // collect a group of tuples from B which
                      // have the same attribute value
                      bool done = false;
                      while ( !done && ptB != 0 ) {
                          int cmp = CompareTuplesB( tmpB.tuple, ptB.tuple );

                          if ( cmp == 0) 	{
                              // append equal tuples to group	
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else		{
                              done = true;
                          }	
                      } // end collect group

                      cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                  }
              }*/

              if ( cmp < 0 ) {
                  // ptA < ptB
                  //if ( outerJoin ) {
                      continueMerge = false;
                      resultTuple = NextConcatB();
                      ptA.setTuple( NextTupleA() );
                      if (resultTuple) {
                          return resultTuple;
                      }
                  /*}
                  else {
                      ptA.setTuple( NextTupleA() );

                      cmp = CompareTuples( ptA.tuple, tmpB.tuple );

                      while ( ptA != 0 && cmp < 0 ) {
                          // skip tuples from A while they are smaller than the
                          // value of the tuples in grpB
                          ptA.setTuple( NextTupleA() );
                          if (ptA != 0)
                              cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                      }
                  }*/
              }
              else if ( cmp == 0 ) {
                  if (!continueMerge && tmpB == 0) {
                      //save ptB in tmpB
                      tmpB = ptB;

                      grpB->AppendTuple(tmpB.tuple);

                      // advance the tuple pointer
                      ptB.setTuple( NextTupleB() );

                      // collect a group of tuples from B which
                      // have the same attribute value
                      bool done = false;
                      while ( !done && ptB != 0 ) {
                          //cout << "grpB" << endl;
                          //logTuples();
                          int cmp = CompareTuplesB( tmpB.tuple, ptB.tuple );

                          if ( cmp == 0) {
                              // append equal tuples to group
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else  {
                              done = true;
                          }	
                      } // end collect group
                  }

                  // continue or start merge
                  if (!continueMerge) {
                      iter = grpB->MakeScan();
                      continueMerge = true;
                      resultTuple = NextConcat();
                      if (resultTuple) {
                          return resultTuple;
                      }
                  } else {
                      //continue merging, create next result tuple
                      resultTuple = NextConcat();
                      if (resultTuple) {
                          return resultTuple;
                      }
                      else {
                          continueMerge = false;
                          delete iter;
                          iter = 0;
                          ptA.setTuple( NextTupleA() );
                          if (ptA != 0) {
                              cmp = CompareTuples( ptA.tuple, tmpB.tuple );
                              if (/*outerJoin && */cmp != 0) {
                                  tmpB = 0;
                                  grpB->Clear();
                              }
                          }

                      }
                  }
              } // end of merge
              else /*if ( outerJoin )*/ {
                  // ptA > ptB
                  continueMerge = false;
                  resultTuple = NextConcatA();
                  ptB.setTuple( NextTupleB() );
                  if (resultTuple) {
                      return resultTuple;
                  }		
              }
              /*else {
                  if ( ptB == 0 )
                      // short exit
                      return 0;

                  grpB->Clear();
                  tmpB = 0;
              }*/
          } // end of
          else if ( /*outerJoin && */ptA != 0 ) {
              // ptB == 0
              resultTuple = NextConcatB();
              ptA.setTuple( NextTupleA() );
              if (resultTuple) {
                  return resultTuple;
              }	
          }
          else /*if ( outerJoin ) / * ptB != 0 */{
              // ptA == 0
              resultTuple = NextConcatA();
              ptB.setTuple( NextTupleB() );
              if (resultTuple) {
                  return resultTuple;
              }	
          }
          /*else {
              // short exit
              return 0;
          }*/
      } // end of main loop
      return 0;
  }

  inline Tuple* NextConcat()
  {
    Tuple* t = iter->GetNextTuple();
    if( t != 0 ) {
     Tuple* result = new Tuple( resultTupleType );
     Concat( ptA.tuple, t, result );
     t->DeleteIfAllowed();

     return result;
    }
    return 0;
  }

  inline Tuple* NextConcatB()
  {
     Tuple* result = new Tuple( resultTupleType );
     Concat( ptA.tuple, NextUndefinedB(), result );
     return result;
  }

  inline Tuple* NextConcatA()
  {	
      Tuple* result = new Tuple( resultTupleType );
      Concat( NextUndefinedA(), ptB.tuple, result );
      return result;
  }

};

/*
2.2.2 Value mapping function of operator ~mergeouterjoin~

*/


//CPUTimeMeasurer mergeMeasurer;

template<bool expectSorted> int
smouterjoin_vm(Word* args, Word& result, 
                  int message, Word& local, Supplier s)
{
  typedef LocalInfo<MergeOuterjoinLocalInfo> LocalType;
  LocalType* li = static_cast<LocalType*>( local.addr );

  switch(message)
  {
    case OPEN:

      if ( li ) {
        delete li;
      }
      li = new LocalType();
      local.addr = li;

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      li->ptr = 0;

      return 0;

    case REQUEST: {
      //mergeMeasurer.Enter();

      if ( li->ptr == 0 )	//first request;
				//constructor put here to avoid delays in OPEN
				//which are a problem for progress estimation
      {
        li->ptr = new MergeOuterjoinLocalInfo
          (args[0], args[4], args[1], args[5], expectSorted, s, li);
      }

      MergeOuterjoinLocalInfo* mli = li->ptr;
      result.addr = mli->NextResultTuple();
      li->returned++;

      //mergeMeasurer.Exit();

      return result.addr != 0 ? YIELD : CANCEL;

    }

    case CLOSE:
      //mergeMeasurer.PrintCPUTimeAndReset("CPU Time for Merging Tuples : ");

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      //nothing is deleted on close because the substructures are still 
      //needed for progress estimation. Instead, everything is deleted on 
      //(repeated) OPEN and on CLOSEPROGRESS

      return 0;

    case CLOSEPROGRESS:
      if (li) {
        delete li;
	local.addr = 0;
      }

      return 0;


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo* pRes = static_cast<ProgressInfo*>( result.addr );

      const double uSortBy = 0.00043;   //millisecs per byte read in sort step

      const double uMergeOuterjoin = 0.0008077;  //millisecs per tuple read
                                        //in merge step (merge)

      const double wMergeOuterjoin = 0.0001738; //millisecs per byte read in 
                                          //merge step (sortmerge)

      const double xMergeOuterjoin = 0.0012058; //millisecs per result tuple in 
                                          //merge step 

      const double yMergeOuterjoin = 0.0001072; //millisecs per result 
                                          //attribute in merge step 

                                      //see file ConstantsSortmergeouterjoin.txt


      LocalInfo<SortByLocalInfo>* liFirst;
      LocalInfo<SortByLocalInfo>* liSecond;

      if( !li ) return CANCEL;
      else
      {

        liFirst = static_cast<LocalInfo<SortByLocalInfo>*> 
	  (li->firstLocalInfo);
        liSecond = static_cast<LocalInfo<SortByLocalInfo>*> 
	  (li->secondLocalInfo);

        if (qp->RequestProgress(args[0].addr, &p1)
         && qp->RequestProgress(args[1].addr, &p2))
        {
	  li->SetJoinSizes(p1, p2);

	  pRes->CopySizes(li);

          if (li->returned > enoughSuccessesJoin ) 	// stable state 
          {
            pRes->Card = ((double) li->returned) * p1.Card
            /  ((double) li->readFirst);
          }
          else
          {
            pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
          }


          if ( expectSorted )   
          {
            pRes->Time = p1.Time + p2.Time +
              (p1.Card + p2.Card) * uMergeOuterjoin +
              pRes->Card * (xMergeOuterjoin + pRes->noAttrs * yMergeOuterjoin);

            pRes->Progress =
              (p1.Progress * p1.Time + p2.Progress * p2.Time +
                (((double) li->readFirst) + ((double) li->readSecond)) 
                * uMergeOuterjoin +
              ((double) li->returned) 
                * (xMergeOuterjoin + pRes->noAttrs * yMergeOuterjoin))
              / pRes->Time;

	    pRes->CopyBlocking(p1, p2);	   //non-blocking in this case
          }
          else
          {
            pRes->Time =
              p1.Time + 
	      p2.Time +
              p1.Card * p1.Size * uSortBy + 
              p2.Card * p2.Size * uSortBy +
              (p1.Card * p1.Size + p2.Card * p2.Size) * wMergeOuterjoin +
              pRes->Card * (xMergeOuterjoin + pRes->noAttrs * yMergeOuterjoin);

            long readFirst = (liFirst ? liFirst->read : 0);
            long readSecond = (liSecond ? liSecond->read : 0);

            pRes->Progress =
              (p1.Progress * p1.Time + 
              p2.Progress * p2.Time +
              ((double) readFirst) * p1.Size * uSortBy + 
              ((double) readSecond) * p2.Size * uSortBy +
              (((double) li->readFirst) * p1.Size + 
               ((double) li->readSecond) * p2.Size) * wMergeOuterjoin +
              ((double) li->returned) 
                * (xMergeOuterjoin + pRes->noAttrs * yMergeOuterjoin))
              / pRes->Time;

            pRes->BTime = p1.Time + p2.Time               
	      + p1.Card * p1.Size * uSortBy 
              + p2.Card * p2.Size * uSortBy;

	    pRes->BProgress = 
	      (p1.Progress * p1.Time + p2.Progress * p2.Time 
              + ((double) readFirst) * p1.Size * uSortBy
              + ((double) readSecond) * p2.Size * uSortBy)
	      / pRes->BTime;
          }
       
          return YIELD;
        }
        else return CANCEL;

      }
    }
  }
  return 0;
}

//-- end progress version --//

#endif

template int
smouterjoin_vm<false>(Word* args, Word& result, int message, 
                 Word& local, Supplier s);
