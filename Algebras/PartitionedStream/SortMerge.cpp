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


Dec 2008. M. Spiekermann: Integration of code implemented formerly in the
ExtRelation-Algebra.


[1] Implementation of special variants for sort merge joins

[TOC]

0 Overview

This file contains the implementation of algorithms for external sorting,
merging wich integrate sample techniques to guarantee that the output will
also start with a random prefix.


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
#include "Hashtable.h"
#include "StreamIterator.h"
#include "Tupleorder.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.1 class ~SortByLocalInfo2~

This class contains some big changes compared to the basic implementation, thus for the
sake of simplicity and programming time efficiency we did not try to rearrange the code 
in such a way that we can keep most of them in a base class.

As a result we accept to have many redundant code lines !!!

*/

class SortByLocalInfo2 : protected ProgressWrapper
{
  public:

    SortByLocalInfo2( Word stream, const bool lexicographic,
                     void *tupleCmp, ProgressLocalInfo* p ) :
      ProgressWrapper(p),
      stream( stream ),
      currentIndex( 0 ),
      lexiTupleCmp( lexicographic ?
                    (LexicographicalTupleSmaller*)tupleCmp :
                    0 ),
      tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
      lexicographic( lexicographic),
      pq( UniversalCompare<LexicographicalTupleSmaller>() )     
      {} 

  public:

  void PrepareResultIteration(bool mkRndSubset = false) 
  {
    InitRuns();
    if (!mkRndSubset)
    {    
      CreateRuns();
    }
    else
    {
      MakeRndSubset();
    }    
    FinishRuns();       
    InitMerge();
  }       


  void InitRuns()
  {
    currentRun = &queue[0];
    nextRun = &queue[1];

    c = 0, i = 0, a = 0, n = 0, m = 0, r = 0; // counter variables
    newRelation = true;

    MAX_MEMORY = qp->FixedMemory();
    cmsg.info("ERA:ShowMemInfo")
      << "Sortby.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
    cmsg.send();

    lastTuple.setTuple(0);
    minTuple.setTuple(0);
    rel=0;
  }     

  void CreateRuns()
  {
    StreamIterator<Tuple> is(stream);
    while( is.valid() )   // consume the stream completely
    {
      Tuple* tuple = *is;
      AppendTuple(tuple);
      ++is;
      tuple->DeleteIfAllowed();
    }

  }

  inline StreamIterator<Tuple> GetIterator() 
  {
    return StreamIterator<Tuple>(stream); 
  }  

  void MakeRndSubset()
  {
    // Before a tuple will be passed to the sorting algorithm
    // it may be chosen as member of a random subset.  

    StreamIterator<Tuple> is(stream);
    while( is.valid() )   // consume the stream completely
    {
      // choose random tuples
      size_t i = 0;
      bool replaced = false;
      Tuple* s = rtBuf.ReplacedByRandom(*is, i, replaced);
      
      if ( replaced )
      { 
        // s was replaced by *is 
        if (s != 0) { 
          MAX_MEMORY -= (*is)->GetSize();
          MAX_MEMORY += s->GetSize();     
          AppendTuple(s);
	  s->DeleteIfAllowed();
        }       
      }
      else 
      { 
        assert(s == 0);
        // s == 0, and *is was not stored in buffer
        AppendTuple(*is);
	(*is)->DeleteIfAllowed();
      }

      ++is;
    }
  }  


  HashTable* CreateHashTable(int buckets, int i, int j)
  {       
    // create a hash table for the tuples stored in
    // rtBuf and return a pointer to it.

    ht = new HashTable(buckets, CmpTuples(i,j));
  
    RandomTBuf::iterator tuple = rtBuf.begin();
    for( ; tuple != rtBuf.end(); tuple++) {
      if (*tuple) {         
        ht->add( *tuple, ((*tuple)->HashValue(i) ) );
      } 
    }   
    
    return ht;
  } 

  HashTable* GetHashTable() const { return ht; } 


  void FinishRuns()
  {
    ShowPartitionInfo(c,a,n,m,r,rel);
    Counter::getRef("Sortby:ExternPartitions") = relations.size();

    // copy the lastRun and NextRun runs into tuple buffers
    // which stay in memory.
    CopyQueue2Vector(0);
    CopyQueue2Vector(1);

  }     


  inline void AppendTuple(Tuple* t)
  {
    progress->read++;
    c++; // tuple counter;
    TupleAndRelPos nextTuple(t, tupleCmpBy);
    if( MAX_MEMORY > (size_t)t->GetSize() )
    {
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
        currentRun->push(nextTuple);
        minTuple = currentRun->topTuple();
        rel->AppendTuple( minTuple.tuple );
	minTuple.tuple->DeleteIfAllowed();
        lastTuple = minTuple;
        currentRun->pop();
      }
      else
      { // check if nextTuple can be saved in current relation

        if ( nextTuple < TupleAndRelPos(lastTuple.tuple, tupleCmpBy) )
        { // nextTuple is in order
          // Push the next tuple int the heap and append the minimum to
          // the current relation and push

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

          nextRun->push(nextTuple);
          n++;
          if ( !currentRun->empty() )
          {
            // Append the minimum to the current relation
            minTuple.setTuple(currentRun->top().tuple());
            rel->AppendTuple( minTuple.tuple );
	    minTuple.tuple->DeleteIfAllowed();
            lastTuple = minTuple;
            
            currentRun->pop();
          }
          else
          { //create a new run
            newRelation = true;

            // swap queues
            Heap* helpRun = currentRun;
            currentRun = nextRun;
            nextRun = helpRun;
            ShowPartitionInfo(c,a,n,m,r,rel);
            i=n;
            a=0;
            n=0;
            m=0;
          } // end new run
        } // end next tuple is smaller

      } // end of check if nextTuple can be saved in current relation
    }// end of memory is completely used

  }     


/*
It may happen, that the localinfo object will be destroyed
before all internal buffered tuples are delivered stream
upwards, e.g. queries which use a ~head~ operator.
In this case we need to delete also all tuples stored in memory.

*/

    ~SortByLocalInfo2()
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
        //relations[i].first->Clear();
        delete relations[i].first;
        relations[i].first = 0;
      }

      delete lexiTupleCmp;
      lexiTupleCmp = 0;
      delete tupleCmpBy;
      tupleCmpBy = 0;
    }


    Tuple* NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
        return 0;
      else
      {
        // Take the first element out of the merge heap
        TupleAndRelPos p = mergeTuples.top();
        Tuple* result = p.tuple();
        mergeTuples.pop();

        // push next tuple into the merge heap
        Tuple* t = relations[p.pos].second->GetNextTuple();
        if( t != 0 )
        { // run not finished
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, p.pos) );
        }
        return result;
      }
    }

    void InitMerge()
    {       
      for( size_t i = 0; i < relations.size(); i++ )
      {
        if ( relations[i].second != 0 ) {
          delete relations[i].second;
        }  

        relations[i].second = relations[i].first->MakeScan();

        // Get next tuple from each relation and push it into the heap.
        Tuple *t = relations[i].second->GetNextTuple();
        if( t != 0 )
        {
          mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, i) );
        }

      }   
    }

  protected:

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

    void CopyQueue2Vector(int i)
    {
      assert( i == 0 || i == 1 );

      TupleBuffer* tbuf = new TupleBuffer();
      GenericRelationIterator *iter = 0;
      relations.push_back( make_pair( tbuf, iter ) );

      while( !queue[i].empty() )
      {
        Tuple* t = queue[i].top().tuple();
        tbuf->AppendTuple(t);
        queue[i].pop();
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

    // Alternate queue type which can be constructed with a user specific
    // comparison function. Currently, this is only experimental code, the
    // member pq is just instantiated not used.
    priority_queue< TupleAndRelPos, 
                    vector<TupleAndRelPos>, 
                    UniversalCompare<LexicographicalTupleSmaller> > pq;


  private:
    Heap* currentRun;
    Heap* nextRun;

    TupleBuffer* rel;

    size_t  c, i, a, n, m, r; // counter variables

    bool newRelation;

    RTuple lastTuple;
    RTuple minTuple;

    RandomTBuf rtBuf;
    HashTable* ht;
};



/*
2.1.1 Value mapping function of operator ~sortby~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the
first sort attribute, ~args[4]~ a boolean indicating wether the stream should
be sorted in ascending order with regard to the sort first attribute. ~args[5]~
and ~args[6]~ contain these values for the second sort attribute and so on.

*/

template<bool lexicographically> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s)
{
  // args[0] : stream
  // args[1] : ignored
  // args[2] : the number of sort attributes
  // args[3] : the index of the first sort attribute
  // args[4] : a boolean which indicates if sortorder should
  //           be asc or desc.
  // args[5] : Same as 3 but for the second sort attribute
  // args[6] : Same as 4
  // ....
  //

  LocalInfo<SortByLocalInfo2>* li;

  li = static_cast<LocalInfo<SortByLocalInfo2>*>( local.addr );

  switch(message)
  {
    case OPEN:
    {
      if ( li ) delete li;

      li = new LocalInfo<SortByLocalInfo2>();
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
        void *tupleCmp = CompareObject(lexicographically, args).getPtr();

        //Sorting is done in the following constructor. It was moved from
        //OPEN to REQUEST to avoid long delays in the OPEN method, which are
        //a problem for progress estimation 
    
        li->ptr = new SortByLocalInfo2( args[0],
                                     lexicographically,
                                     tupleCmp, li       );

        li->ptr->PrepareResultIteration();
      }

      SortByLocalInfo2* sli = li->ptr;

      result.setAddr( sli->NextResultTuple() );
      li->returned++;
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
      qp->Close(args[0].addr);
      return 0;


    case CLOSEPROGRESS:
      if ( li ) {
         delete li;
         local.addr = 0;
      }
      return 0;


    case REQUESTPROGRESS:

      ProgressInfo p1;
      ProgressInfo *pRes;
      const double uSortBy = 0.000396;   //millisecs per byte input and sort
      const double vSortBy = 0.000194;   //millisecs per byte output
      const double oSortBy = 0.00004;   //offset due to writing to disk
                                    //not yet measurable
      pRes = (ProgressInfo*) result.addr;

      if( !li ) return CANCEL;
      else
      {
        if (qp->RequestProgress(args[0].addr, &p1))
        {
          pRes->Card = li->returned == 0 ? p1.Card : li->read;

          pRes->CopySizes(p1);

          pRes->Time =                       //li->state = 0 or 1
            p1.Time 
            + pRes->Card * p1.Size * (uSortBy + oSortBy * li->state)
            + pRes->Card * p1.Size * vSortBy;

          pRes->Progress =
            (p1.Progress * p1.Time 
             + li->read * p1.Size * (uSortBy + oSortBy * li->state) 
             + li->returned * p1.Size * vSortBy)
            / pRes->Time;

          pRes->BTime = p1.Time + pRes->Card * p1.Size *  
            (uSortBy + oSortBy * li->state);

          pRes->BProgress = 
            (p1.Progress * p1.Time
            + li->read * p1.Size * (uSortBy + oSortBy * li->state))
            / pRes->BTime;

          return YIELD;
        }
        else return CANCEL;
      }

  }
  return 0;
}


/*
2.2 Operator ~mergejoin~

This operator computes the equijoin of two streams. It uses a text book
algorithm as outlined in A. Silberschatz, H. F. Korth, S. Sudarshan,
McGraw-Hill, 3rd. Edition, 1997.

2.2.1 Auxiliary definitions for value mapping function of operator ~mergejoin~

*/

class MergeJoinLocalInfo2: protected ProgressWrapper
{
protected:

  // buffer limits      
  size_t MAX_MEMORY;
  size_t MAX_TUPLES_IN_MEMORY;

  // buffer related members and iterators
  TupleBuffer *grpB;
  GenericRelationIterator *iter;

  // members needed for sorting the input streams
  typedef LocalInfo<SortByLocalInfo2> LocalSRT;
  LocalSRT* liA;
  SortByLocalInfo2* sliA;

  LocalSRT* liB;
  SortByLocalInfo2* sliB;

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

  // Members needed for the random subset option
  bool randomPrefix;
  bool continueHashjoin;
  bool continueProbe;
  bool earlyExit;

  StreamIterator<Tuple> iterB;
  HashTable* ht;

  // switch trace messages on/off
  const bool traceFlag; 

  // a flag needed in function NextTuple which tells
  // if the merge with grpB has been finished
  bool continueMerge;

  template<bool BOTH_B>
  int CompareTuples(Tuple* t1, Tuple* t2)
  {

    Attribute* a = 0;     
    if (BOTH_B) {   
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexB) );
    }  
    else {
      a = static_cast<Attribute*>( t1->GetAttribute(attrIndexA) );
    }

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

  inline Tuple* NextTuple(Word stream, SortByLocalInfo2* sli)
  {
    bool yield = false;
    Word result( Address(0) );

    if(!expectSorted) {
      return sli->NextResultTuple();
    } 

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

    SortByLocalInfo2* SortInput( const Word& stream, int attrIndex, 
                                LocalSRT*& li) 
    {
      // sort the input streams
      SortOrderSpecification spec;
      spec.push_back( pair<int, bool>(attrIndex + 1, true) ); 
      void* tupleCmp = new TupleCompareBy( spec );

      li = new LocalSRT();
      return  new SortByLocalInfo2( stream, false,  tupleCmp, li);
    }



    void SortInputs() 
    {
      // sort the input streams

      SortOrderSpecification specA;
      SortOrderSpecification specB;
         
      specA.push_back( pair<int, bool>(attrIndexA + 1, true) ); 
      specB.push_back( pair<int, bool>(attrIndexB + 1, true) ); 


      void* tupleCmpA = new TupleCompareBy( specA );
      void* tupleCmpB = new TupleCompareBy( specB );

      liA = new LocalInfo<SortByLocalInfo2>();
      progress->firstLocalInfo = liA;
      sliA = new SortByLocalInfo2( streamA, 
                                  false,  
                                  tupleCmpA, liA );

      liB = new LocalInfo<SortByLocalInfo2>();
      progress->secondLocalInfo = liB;
      sliB = new SortByLocalInfo2( streamB, 
                                  false,  
                                  tupleCmpB, liB );

    }


public:
  MergeJoinLocalInfo2( Word _streamA, Word wAttrIndexA,
                      Word _streamB, Word wAttrIndexB, 
                      bool _expectSorted, Supplier s,
                      ProgressLocalInfo* p, 
                      bool _randomPrefix = false, 
                      bool _earlyExit = false )
  :  ProgressWrapper(p), 
     traceFlag( RTFlag::isActive("PSA:TraceMergeJoin") )
  {
    expectSorted = _expectSorted;
    randomPrefix = _randomPrefix;
    earlyExit = _earlyExit;

    streamA = _streamA;
    streamB = _streamB;

    attrIndexA = StdTypes::GetInt( wAttrIndexA ) - 1;
    attrIndexB = StdTypes::GetInt( wAttrIndexB ) - 1;

    ListExpr resultType =
                SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    MAX_MEMORY = qp->FixedMemory();

    cmsg.info("ERA:ShowMemInfo")
      << "MergeJoin.MAX_MEMORY (" << MAX_MEMORY/1024 << " kb)" << endl;
    cmsg.send();

    liA = 0;
    sliA = 0;

    liB = 0;
    grpB = 0;
    sliB = 0; 

    ht = 0;
    continueHashjoin = false;
    continueProbe = false;

    
    if ( randomPrefix ) {
      
      sliA = SortInput(streamA, attrIndexA, liA);
      sliA->PrepareResultIteration(true);
      progress->firstLocalInfo = liA;

      // Now a random subset S1 of 500 tuples is stored in a hash table.
      // Next the tuples of streamB will be joined with S1 and passed to
      // the Sorting-Algorithm for B. Finally, the sorted streams are merged. 

      sliB = SortInput(streamB, attrIndexB, liB);

      if (traceFlag)
	cerr << "Input B sorted" << endl;

      progress->secondLocalInfo = liB;

      iterB = sliB->GetIterator();
      // prime numbers: 503, 701, 1009, 2003
      ht = sliA->CreateHashTable(701, attrIndexA, attrIndexB);

      if (traceFlag)
	cerr << "HashTable created" << endl;

      continueHashjoin = true;
      sliB->InitRuns();

      if (traceFlag)
	cerr << "Input B initialized" << endl;

    }
    else
    {       
      if( !expectSorted ) {         
      
      sliA = SortInput(streamA, attrIndexA, liA);
      sliA->PrepareResultIteration();
      progress->firstLocalInfo = liA;

      sliB = SortInput(streamB, attrIndexB, liB);
      sliB->PrepareResultIteration();
      progress->secondLocalInfo = liB;
      }  
      InitIteration();
    }

  }

  ~MergeJoinLocalInfo2()
  {
    //cerr << "calling ~MergeJoinLocalInfo2()" << endl;    
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


  inline Tuple* NextResultTuple()
  {
    Tuple* resultTuple = 0;

    while ( continueHashjoin ) { // probe hash buckets

      if ( !continueProbe ) // initialize hash bucket iteration
      {
        if (traceFlag)
  	  cerr << "Initialize hash bucket iteration" << endl;

        if ( iterB.valid() ) 
        {
          (*iterB)->IncReference();       
          ht->initProbe( (*iterB)->HashValue(attrIndexB) ); 
          continueProbe = true;
          sliB->AppendTuple(*iterB);

        } 
        else // end of stream B and end of hashjoin                
        {
          if (traceFlag)
  	    cerr << "End of stream B" << endl;

          continueHashjoin = false;             
          continueProbe = false;
          sliB->FinishRuns();
          sliB->InitMerge();
          InitIteration();
        }       
      }       

      if ( continueProbe ) 
      { 

       Tuple* b = *iterB;
       Tuple* a = ht->probe(b);

              if (a != 0) { // concat a and b

                Tuple* result = new Tuple( resultTupleType );
                Concat( a, b, result );		
                return result;        
              } 
              else // switch to next tuple of B
              {
                //cout << "b:refs =" << b->GetNumOfRefs() << endl;      
                b->DeleteIfAllowed();   
                ++iterB;
                continueProbe = false;
              }
      }


    }       

    if (earlyExit) {
          if (traceFlag)
  	    cerr << "Early exit" << endl;
      return 0;     
    }  

    if ( !continueMerge && ptB == 0)
      return 0;     

    while( ptA != 0 ) {
     
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
     
        if ( cmp == 0) 
        {
          // append equal tuples to group       
          grpB->AppendTuple(ptB.tuple);

          // release tuple of input B
          ptB.setTuple( NextTupleB() );
        }
        else
        {
          done = true;  
        }       
      } // end collect group            

      cmp = CompareTuples( ptA.tuple, tmpB.tuple );

      while ( ptA != 0 && cmp < 0 ) 
      {
        // skip tuples from A while they are smaller than the 
        // value of the tuples in grpB        
        ptA.setTuple(  NextTupleA() );
        if (ptA != 0) {
          cmp = CompareTuples( ptA.tuple, tmpB.tuple );
        }  
      }       

      }
      // continue or start a merge with grpB   

      while ( ptA != 0 && cmp == 0 )
      {
        // join ptA with grpB
         
        if (!continueMerge) 
        {      
          iter = grpB->MakeScan();
          continueMerge = true;
          resultTuple = NextConcat();
          if (resultTuple)
            return resultTuple;           
        }  
        else
        {               
          // continue merging, create the next result tuple
          resultTuple = NextConcat();
          if (resultTuple) {
            return resultTuple;
          }         
          else 
          {
            // Iteration over the group finished.         
            // Continue with the next tuple of argument A
            continueMerge = false;
            delete iter;
            iter = 0;
            ptA.setTuple( NextTupleA() );
            if (ptA != 0) {
              cmp = CompareTuples( ptA.tuple, tmpB.tuple );
            }  
          }               
        }         
      }               
      
      grpB->Clear();
      // tpA > tmpB 
      if ( ptB == 0 ) {
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
     t->DeleteIfAllowed();

     return result;  
    }
    return 0;
  }

    void InitIteration()
    { 
    // read in the first tuple of both input streams
    Tuple* tA = NextTupleA();
    ptA = RTuple( tA );
    if(tA) tA->DeleteIfAllowed();
    Tuple* tB = NextTupleB();
    ptB = RTuple( tB );
    if(tB) tB->DeleteIfAllowed();

    // initialize the status for the result
    // set iteration   
    tmpB = 0;
    cmp = 0;
    continueMerge = false;

    if (grpB != 0) 
      delete grpB;

    grpB = new TupleBuffer( MAX_MEMORY );
    }


};


/*
2.2.2  MergeJoinLocalInfoSHF

A variant of a sortmergejoin which produces an output stream which starts with
a random sample of 500 tuples.

*/


class MergeJoinLocalInfoSHF : protected MergeJoinLocalInfo2
{

  public:       
  MergeJoinLocalInfoSHF( Word _streamA, 
                             Word wAttrIndexA,
                             Word _streamB, 
                             Word wAttrIndexB, 
                             bool _expectSorted, 
                             Supplier s,
                             ProgressLocalInfo* p, 
                             bool rnd = false, bool earlyexit = false )
  : MergeJoinLocalInfo2( _streamA, wAttrIndexA,
                        _streamB, wAttrIndexB, 
                        _expectSorted, s, p, rnd, earlyexit ),
    streamPos(0),
    positions(500,0),   
    memBufIter(0),     
    memBufFinished(false),
    firstScanFinished(false),
    trace(true)
  {}              

  ~MergeJoinLocalInfoSHF() {

    cerr << "calling ~MergeJoinLocalInfoSHF()" << endl;   
  }

  inline Tuple* NextResultTuple() 
  {
     Tuple* res = 0;

     if (!firstScanFinished) 
     {
       res = MergeJoinLocalInfo2::NextResultTuple();

       while (res != 0)
       {
         // decide if tuple replaces one of the buffer
         streamPos++;

            size_t i = 0;
            bool replaced = false;
            Tuple* v = rtBuf.ReplacedByRandom(res, i, replaced);
            
            if ( replaced )
            { 
              positions[i] = streamPos;     
              // v was replaced by res
              if (v != 0) {
                //persBuf.AppendTuple(v);           
                v->DeleteIfAllowed();
              } 
            }
            else 
            { 
              assert(v == 0);
              // v == 0, and t was not stored in buffer
              res->DeleteIfAllowed();
            }

         res = MergeJoinLocalInfo2::NextResultTuple();
       } 
       if (trace)
         cerr << "copy2TupleBuf" << endl;

       rtBuf.copy2TupleBuf( memBuf );

         // reset scan       
         firstScanFinished = true;
         sliA->InitMerge();
         sliB->InitMerge(); 
         InitIteration();

         sort(positions.begin(), positions.end());
         posIter = positions.begin();
         memBufIter = memBuf.MakeScan();

         streamPos = 0;
        
         if (trace)
           cerr << "Start 2nd run" << endl;          
     }

     if (firstScanFinished)
     {  
       if (!memBufFinished)
       {
         res = memBufIter->GetNextTuple();

         if (res == 0) {              

           if (trace) {      
             cerr << endl;
             cerr << "streamPos: " << streamPos << endl;            
             cerr << "memBuf   : " << memBuf.GetNoTuples() << endl;         
           }  
           memBufFinished = true;
           delete memBufIter;
           memBufIter = 0;
         }      
       }

       if ( memBufFinished == true)
       { 
         res = MergeJoinLocalInfo2::NextResultTuple();
         streamPos++;
         while (streamPos == *posIter) 
         {
            res->DeleteIfAllowed();
            res = MergeJoinLocalInfo2::NextResultTuple();
            streamPos++;
            posIter++;
         } 
       }         
     }       
     return res;
  }       

  private:
  size_t streamPos;

  TupleBuffer memBuf;
  vector<size_t> positions;
  vector<size_t>::const_iterator posIter;
  RandomTBuf rtBuf;
  
  GenericRelationIterator* memBufIter;
  bool memBufFinished;
  bool firstScanFinished;
  const bool trace;

};  


/*
2.2.3 Value mapping function of operator ~mergejoin~

*/

//CPUTimeMeasurer mergeMeasurer;

template<class T, bool SRT, bool RND, bool R3> int
MergeJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  typedef LocalInfo<T> LocalType;
  LocalType* li = static_cast<LocalType*>( local.addr );

  switch(message)
  {
    case OPEN:

      if ( li ) {
        delete li->ptr;
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

      if ( li->ptr == 0 )       //first request;
                                //constructor put here to avoid delays in OPEN
                                //which are a problem for progress estimation
      {
        li->ptr = new T( args[0], args[4], args[1], 
                         args[5], SRT, s, li, RND, R3 );
      }

      T* mli = li->ptr;
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
      //needed for progress estimation. Instea/*
      //(repeated) OPEN and on CLOSEPROGRESS

      return 0;


    case CLOSEPROGRESS:

      if ( li ) {
        delete li;
        local.addr = 0;
      }
      return 0;


    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo* pRes = static_cast<ProgressInfo*>( result.addr );
      const double uMergeJoin = 0.041;  //millisecs per tuple merge (merge)
      const double vMergeJoin = 0.000076; //millisecs per byte merge (sortmerge)
      const double uSortBy = 0.00043;     //millisecs per byte sort

      if( !li ) 
      {
        return CANCEL;
      } 
      else
      {

        if (qp->RequestProgress(args[0].addr, &p1)
         && qp->RequestProgress(args[1].addr, &p2))
        {
          li->SetJoinSizes(p1, p2);

          pRes->CopySizes(li);

          if ( SRT ) // already sorted inputes
          {
            pRes->Time = p1.Time + p2.Time +
              (p1.Card + p2.Card) * uMergeJoin;

            pRes->Progress =
              (p1.Progress * p1.Time + p2.Progress * p2.Time +
                (((double) li->readFirst) + ((double) li->readSecond)) 
                * uMergeJoin)
              / pRes->Time;

            pRes->CopyBlocking(p1, p2);    //non-blocking in this case
          }
          else // unsorted inputs
          {
            pRes->Time =
              p1.Time + 
              p2.Time +
              p1.Card * p1.Size * uSortBy + 
              p2.Card * p2.Size * uSortBy +
              (p1.Card * p1.Size + p2.Card * p2.Size) * vMergeJoin;
        
            typedef LocalInfo<SortByLocalInfo2> LocalSRT;      
            LocalSRT* liFirst = 0;
            LocalSRT* liSecond = 0;

            liFirst = static_cast<LocalSRT*>( li->firstLocalInfo );
            liSecond = static_cast<LocalSRT*>( li->secondLocalInfo );

            long readFirst = (liFirst ? liFirst->read : 0);
            long readSecond = (liSecond ? liSecond->read : 0);

            pRes->Progress =
              (p1.Progress * p1.Time + 
              p2.Progress * p2.Time +
              ((double) readFirst) * p1.Size * uSortBy + 
              ((double) readSecond) * p2.Size * uSortBy +
              (((double) li->readFirst) * p1.Size + 
                ((double) li->readSecond) * p2.Size) 
                * vMergeJoin)
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



          if (li->returned > enoughSuccessesJoin )      // stable state 
          {
            pRes->Card = ((double) li->returned * (p1.Card + p2.Card)
           /  ((double) li->readFirst + (double) li->readSecond));

          }
          else
          {
            pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
          }
          return YIELD;
        }
        else return CANCEL;

      }
    }
  }
  return 0;
}


/*
3 Instantiation of Template Functions

*/


// sortmergejoin_r
template int
MergeJoin<MergeJoinLocalInfoSHF, false, false, false>( Word* args, 
                                                       Word& result, 
                                                       int message, 
                                                       Word& local, 
                                                       Supplier s   );

int 
sortmergejoinr_vm( Word* args, Word& result, 
                   int message, Word& local, Supplier s )
{
  return MergeJoin<MergeJoinLocalInfoSHF, false, false, false>(args, result, 
                                                 message, local, s);    
}


int 
sortmergejoinr2_vm( Word* args, Word& result, 
                   int message, Word& local, Supplier s )
{
  return MergeJoin<MergeJoinLocalInfo2, false, true, false>(args, result, 
                                              message, local, s);       
}

int 
sortmergejoinr3_vm( Word* args, Word& result, 
                   int message, Word& local, Supplier s )
{
  return MergeJoin<MergeJoinLocalInfo2, false, true, true>(args, result, 
                                              message, local, s);       
}

