/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

Oct 2004. M. Spiekermann. The SortByLocalInfo was revised, since it doesn't
work for relations not fitting into memory. Moreover some minor performance tuning
was made (fixed size for the vector of tuples).

Nov 2004. M. Spiekermann. The Algorithm for external sorting was changed. See below 
for details.

[1] Implementation of the Module Extended Relation Algebra for Persistent storage

[TOC]

1 Includes and defines

*/
#ifdef RELALG_PERSISTENT

#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "StopWatch.h"

#include <vector>
#include <list>
#include <set>
#include <queue>

#include "Counter.h"
#include "LogMsg.h"

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
  
  TupleAndRelPos(Tuple* newTuple, TupleCompareBy* cmpObjPtr = 0, size_t newPos = 0) :
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
    Counter::getRef("TupleAndRelPos::less")++;
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
or 2N (sorted in opposite order) comparisons.  

*/

template<class T> 
class PrioQueue : public priority_queue<T> {

public:
  PrioQueue() : priority_queue<T>() {};
  ~PrioQueue() {};
  
  size_t capacity() { return c.capacity(); }
  void reserve(size_t n) { c.reserve(n); }
};



class SortByLocalInfo
{
  public:
    SortByLocalInfo( Word stream, const bool lexicographic, void *tupleCmp ):
      stream( stream ),
      currentIndex( 0 ),
      lexiTupleCmp( lexicographic ? (LexicographicalTupleCompare*)tupleCmp : 0 ),
      tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
      lexicographic( lexicographic ),
      tupleType( 0 )
      {
        // Note: Is is not possible to define a Cmp object using the constructor 
        // mergeTuples( PairTupleCompareBy( tupleCmpBy )). It does only work if
        // mergeTuples is a local variable which does not help us in this case.
        // Hence a new class TupleAndRelPos was defined to define a '<' operator. 
        PrioQueue<TupleAndRelPos>* currentRun = &queue[0];
        PrioQueue<TupleAndRelPos>* nextRun = &queue[1];
       
        Word wTuple = SetWord(Address(0));
        size_t i = 0, a = 0, n = 0, r = 0; // counter variables
        bool newRelation = true;

        qp->Open(stream.addr);
        qp->Request(stream.addr, wTuple);

        if(qp->Received(stream.addr))
        {
          tupleType = new TupleType( ((Tuple*)wTuple.addr)->GetTupleType() );
          // MAX_TUPLES_IN_MEMORY = qp->MemoryAvailableForOperator() / ((Tuple*)wTuple.addr)->GetMemorySize();
          char* memSizeStr = getenv("SECONDO_SORT_MAXTUPLES");
          if ( memSizeStr ) {
            MAX_TUPLES_IN_MEMORY = atoi(memSizeStr);
          } else { 
            MAX_TUPLES_IN_MEMORY = 16 * 1024 * 1024 / ((Tuple*)wTuple.addr)->GetMemorySize();
          }
          cmsg.info() << "Sort.MAX_TUPLES_IN_MEMORY: " << MAX_TUPLES_IN_MEMORY << endl;
          cmsg.send();
        }
        // Reserving memory does not work correctly in all situations
        //currentRun->reserve(MAX_TUPLES_IN_MEMORY+100); 
        //nextRun->reserve(MAX_TUPLES_IN_MEMORY+100);

        Relation *rel=0;
        TupleAndRelPos lastTuple(0, tupleCmpBy);
        TupleAndRelPos minTuple(0, tupleCmpBy);
        while(qp->Received(stream.addr)) // consume the stream completely
        {
          Tuple *t = ((Tuple*)wTuple.addr)->CloneIfNecessary();
          if( t != wTuple.addr ) {
            ((Tuple*)wTuple.addr)->DeleteIfAllowed();
          }
          t->SetFree( false );
          assert( (t!=0) );                 
          
          TupleAndRelPos nextTuple(t, tupleCmpBy); 
           
          if ( i < MAX_TUPLES_IN_MEMORY ){ 
           
            currentRun->push(nextTuple);
            i++; // increment Tuples in memory counter   
            
          } else { // memory is completely used 
          
            if ( newRelation ) { // create new relation
            
              r++;
              rel = new Relation( *tupleType, true );
              RelationIterator *iter = rel->MakeScan();
              relations.push_back( pair<Relation*, RelationIterator*>( rel, iter ) );
              newRelation = false;
              
              // get first tuple and store it in an relation
              currentRun->push(nextTuple);
              minTuple = currentRun->top();
              lastTuple = minTuple;
              AppendToRel(*rel, nextTuple);
              currentRun->pop();              
              
            } else { // check if nextTuple can be saved in current relation
              
              TupleAndRelPos copyOfLast = lastTuple;
              if ( nextTuple < lastTuple ) { // nextTuple is in order              

                if ( minTuple < nextTuple ) { 
                  // nextTuple smaller than min, append it to the
                  // current relation 
                  AppendToRel(*rel, nextTuple);
                  lastTuple = nextTuple;
                  a++;
                } else { 
                  // Append the minimum to the next relation and push
                  // the next tuple into the heap. 
                  AppendToRel(*rel, minTuple);
                  lastTuple = minTuple;
                  minTuple = currentRun->top();
                  currentRun->pop();
                  currentRun->push(nextTuple);
                } 
                     
              } else { // nextTuple is smaller, save it for the next relation
                
                nextRun->push(nextTuple);
                n++;

                if ( !currentRun->empty() ) {
                
                  lastTuple = currentRun->top();
                  AppendToRel(*rel, lastTuple);
                  currentRun->pop();
                  
                } else { //create a new run 

                  newRelation = true;
                  
                  // swap queues
                  PrioQueue<TupleAndRelPos> *helpRun = currentRun;
                  currentRun = nextRun;
                  nextRun = helpRun;
                  ShowPartitionInfo(i,a,n,r);
                  i=n;
                  a=0;
                  n=0;
                } // end new run               
                
              } // end next tuple is smaller

              // delete last tuple if saved to relation and
              // not referenced by minTuple
              if ( copyOfLast.tuple 
                   && (copyOfLast.tuple != minTuple.tuple) 
                   && (copyOfLast.pos == 1) ) 
              {
                copyOfLast.tuple->SetFree(true);
                copyOfLast.tuple->DeleteIfAllowed();
              }

            } // check if nextTuple can be saved in current relation
          } // memory is completely used
          
          qp->Request(stream.addr, wTuple);
        }
        ShowPartitionInfo(i,a,n,r);

        // delete lastTuple and minTuple if allowed
        if (lastTuple.pos) {
          lastTuple.tuple->SetFree(true);
          lastTuple.tuple->DeleteIfAllowed();
        }
        if (minTuple.pos) {
          minTuple.tuple->SetFree(true);
          minTuple.tuple->DeleteIfAllowed();
        }

        qp->Close(stream.addr);

        // the lastRun and NextRun partitions in memory having 
        // less than MAX_TUPLE elements
        if( !queue[0].empty() ) {
          TupleAndRelPos t = queue[0].top();
          queue[0].pop();
          t.pos = -2;
          mergeTuples.push( t );
        } 
        if( !queue[1].empty() ) {
          TupleAndRelPos t = queue[1].top();
          queue[1].pop();
          t.pos = -1;
          mergeTuples.push( t );
        } 
               
        // Get next tuple from each relation and push it into the heap.
        for( size_t i = 0; i < relations.size(); i++ )
        {
          Tuple *t = relations[i].second->GetNextTuple();
          if( t != 0 ) {
             mergeTuples.push( TupleAndRelPos(t, tupleCmpBy, i+1) );
          }
        }
      }

    ~SortByLocalInfo()
    {
      if ( !mergeTuples.empty() ) {
        cmsg.warning() << "Warning SortByLocalInfo contains tuples!" << endl;
        cmsg.send();
      }

      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        delete relations[i].first;
      }
      delete tupleType;
    }

    Tuple *NextResultTuple()
    {
      if( mergeTuples.empty() ) // stream finished
      {
        assert( queue[0].empty() && queue[1].empty() );
        return 0;
      }
      else
      {
        // Take the first one.
        TupleAndRelPos p = mergeTuples.top();
        mergeTuples.pop();
        Tuple *result = p.tuple;
        Tuple *t = 0;

        assert( (p.pos != 0) && (p.pos >= -2) && (p.pos <= (int)relations.size()+1) );
        assert( result != 0);

        if (p.pos > 0) {
          t = relations[p.pos-1].second->GetNextTuple();
        }
        if (p.pos < 0) {

          int idx = p.pos+2;
          if ( !queue[idx].empty() ) {
            t = queue[idx].top().tuple;
            queue[idx].pop();
          } else {
            t = 0;
          } 
        }

        if( t != 0 ) {
          p.tuple = t;
          mergeTuples.push( p );
        }
        result->SetFree(true);
        return result;
      }
    }

  private:
    inline void AppendToRel( Relation &rel, TupleAndRelPos& t )
    {
        Tuple *tp = t.tuple->CloneIfNecessary();
        rel.AppendTuple( tp );

        if( tp != t.tuple ) {
          t.pos=1; 
          // this indicates that the tuple was saved to disk
          // since it may be needed for further comparisons
          // it will be deleted on other places
        }
        tp->DeleteIfAllowed();
    }

    void ShowPartitionInfo(int i, int a, int n, int r) {

      if ( RTFlag::isActive("Stat:SortBy") ) {

        cmsg.info() << "Partition finished: i=" << i 
                    << ", appended=" << a 
                    << ", next=" << n
                    << ", partitions=" << r
                    << ", queue1: " << queue[1].size() 
                    << ", queue2: " << queue[2].size() << endl;
        cmsg.send();  
      }
    }

    size_t MAX_TUPLES_IN_MEMORY;
    Word stream;
    vector<TupleAndRelPos> tuples;
    size_t currentIndex;
    LexicographicalTupleCompare *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;
    TupleType *tupleType;
    vector< pair<Relation*, RelationIterator*> > relations;
    PrioQueue<TupleAndRelPos> queue[2];
    priority_queue<TupleAndRelPos> mergeTuples;
};

/*
2.1.1 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the first
sort attribute, ~args[4]~ a boolean indicating wether the stream is sorted in
ascending order with regard to the sort first attribute. ~args[5]~ and ~args[6]~
contain these values for the second sort attribute  and so on.

*/

template<bool lexicographically, bool requestArgs> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s)
{
  switch(message)
  {
    case OPEN:
    {
      void *tupleCmp;
      SortOrderSpecification spec;
      Word intWord;
      Word boolWord;
      bool sortOrderIsAscending;
      int nSortAttrs;
      int sortAttrIndex;

      if(lexicographically)
      {
	tupleCmp = new LexicographicalTupleCompare();
      }
      else
      {
	if(requestArgs)
        {
          qp->Request(args[2].addr, intWord);
        }
        else
        {
          intWord = SetWord(args[2].addr);
        }
        nSortAttrs = ((CcInt*)intWord.addr)->GetIntval();
        for(int i = 1; i <= nSortAttrs; i++)
        {
	  if(requestArgs)
          {
            qp->Request(args[2 * i + 1].addr, intWord);
          }
          else
          {
            intWord = SetWord(args[2 * i + 1].addr);
          }
          sortAttrIndex =
            ((CcInt*)intWord.addr)->GetIntval();

          if(requestArgs)
          {
            qp->Request(args[2 * i + 2].addr, boolWord);
          }
          else
          {
            boolWord = SetWord(args[2 * i + 2].addr);
          }
          sortOrderIsAscending =
            ((CcBool*)boolWord.addr)->GetBoolval();
          spec.push_back(pair<int, bool>(sortAttrIndex, sortOrderIsAscending));
        };

        tupleCmp = new TupleCompareBy( spec );
      }

      local = SetWord(new SortByLocalInfo( args[0], lexicographically, tupleCmp ));
      return 0;
    }
    case REQUEST:
    {
      SortByLocalInfo *localInfo = (SortByLocalInfo*)local.addr;
      result = SetWord( localInfo->NextResultTuple() );
      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      SortByLocalInfo *localInfo = (SortByLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

/*
2.2 Operator ~mergejoin~

This operator computes the equijoin two streams.

2.2.1 Auxiliary definitions for value mapping function of operator ~mergejoin~

*/

static CcInt oneCcInt(true, 1);
static CcBool trueCcBool(true, true);

CPUTimeMeasurer mergeMeasurer;

class MergeJoinLocalInfo
{
private:
  vector<Tuple*> bucketA;
  size_t indexA;

  vector<Tuple*> bucketB;
  size_t indexB;

  Relation *relationA;
  RelationIterator *iterRelationA;

  Relation *relationB;
  RelationIterator *iterRelationB;

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

  const string traceFlag; 

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

    return ((Attribute*)a->GetAttribute(attrIndexA))->Compare((Attribute*)b->GetAttribute(attrIndexB));
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
      int errorCode = SortBy<false, false>(aArgs, aResult, REQUEST, streamALocalInfo, 0);
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
      int errorCode = SortBy<false, false>(bArgs, bResult, REQUEST, streamBLocalInfo, 0);
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

  void SaveTo( vector<Tuple*>& bucket, Relation *rel )
  {
    vector<Tuple*>::iterator iter = bucket.begin();
    while( iter != bucket.end() )
    {
      Tuple *t =  (*iter)->CloneIfNecessary();
      if( t != *iter )
        (*iter)->DeleteIfAllowed();
      rel->AppendTuple( t );
      t->DeleteIfAllowed();
      iter++;
    }
  }

  void ReadFrom( RelationIterator *iter, vector<Tuple*>& bucket )
  {
    size_t i = 0;
    Tuple *t;

    while( i < MAX_TUPLES_IN_MEMORY && (t = iter->GetNextTuple()) != 0 )
    {
      t->SetFree( false );
      bucket.push_back( t );
      i++;
    }
  }

  void ClearBucket( vector<Tuple*>& bucket )
  {
    vector<Tuple*>::iterator i = bucket.begin();
    while( i != bucket.end() )
    {
      Tuple *t = *i;
      t->DeleteIfAllowed();
      i++;
    }
    bucket.clear();
  }

  size_t MAX_TUPLES_IN_MEMORY;
public:
  MergeJoinLocalInfo(Word streamA, Word attrIndexA,
    Word streamB, Word attrIndexB, bool expectSorted,
    Supplier s) :
    traceFlag("Merge:Trace")
  {
     assert(streamA.addr != 0);
    assert(streamB.addr != 0);
    assert(attrIndexA.addr != 0);
    assert(attrIndexB.addr != 0);
    assert(((CcInt*)attrIndexA.addr)->GetIntval() > 0);
    assert(((CcInt*)attrIndexB.addr)->GetIntval() > 0);

    this->expectSorted = expectSorted;
    this->streamA = streamA;
    this->streamB = streamB;
    this->attrIndexA = ((CcInt*)attrIndexA.addr)->GetIntval() - 1;
    this->attrIndexB = ((CcInt*)attrIndexB.addr)->GetIntval() - 1;
    this->relationA = 0;
    this->relationB = 0;
    this->aResult = SetWord(Address(0));
    this->bResult = SetWord(Address(0));

    if(expectSorted)
    {
      qp->Open(streamA.addr);
      qp->Open(streamB.addr);
    }
    else
    {
      SetArgs(aArgs, streamA, attrIndexA);
      SetArgs(bArgs, streamB, attrIndexB);
      SortBy<false, false>(aArgs, aResult, OPEN, streamALocalInfo, 0);
      SortBy<false, false>(bArgs, bResult, OPEN, streamBLocalInfo, 0);
    }

    ListExpr resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    Tuple *tupleA = NextATuple(),
          *tupleB = NextBTuple();

    if( tupleA != 0 && tupleB != 0 )
    {
      long sizeTupleA = tupleA->GetMemorySize(),
           sizeTupleB = tupleB->GetMemorySize(),
           tupleSize = sizeTupleA > sizeTupleB ? sizeTupleA : sizeTupleB;
//      MAX_TUPLES_IN_MEMORY = qp->MemoryAvailableForOperator() / ( 2 * tupleSize );
      MAX_TUPLES_IN_MEMORY = 16 * 1024 * 1024 / ( 2 * tupleSize );
      cout << "Merge.MAX_TUPLES_IN_MEMORY: " << MAX_TUPLES_IN_MEMORY << endl;
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
      SortBy<false, false>(aArgs, aResult, CLOSE, streamALocalInfo, 0);
      SortBy<false, false>(bArgs, bResult, CLOSE, streamBLocalInfo, 0);
    }
    delete resultTupleType;
  }

  Tuple *NextResultTuple()
  {
    Tuple *tupleA, *tupleB;
    Tuple *resultTuple = 0;

    if( bucketA.size() > 0 )
    // There are equal tuples that fit in memory for the bucket A.
    {
      assert( bucketB.size() > 0 );

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
                relationA->Delete(); relationA = 0;
                delete iterRelationB;
                relationB->Delete(); relationB = 0;
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
              relationB->Delete(); relationB = 0;
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
            relationA->Delete(); relationA = 0;
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
        resultTuple = new Tuple( *resultTupleType, false );
        Concat( bucketA[indexA], bucketB[indexB++], resultTuple );
      }
    }
    else
    // There are no stored equal tuples.
    {
      assert( relationA == 0 && relationB == 0 );
      assert( bucketA.empty() && bucketB.empty() );

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
      
      if ( RTFlag::isActive( traceFlag ) ) {

        cout << "Comp A: " << *tupleA << " - B: " << *tupleB 
             << " = " << cmp << endl; 
      }

      if( cmp == 0 )
      // The tuples are equal. We must store them in a buffer if it fits or in
      // the disk otherwise
      {
        Tuple *equalTupleB = tupleB->Clone(),
              *equalTupleA = tupleA->Clone();

        bucketA.push_back( tupleA );
        bucketB.push_back( tupleB );

        tupleA = NextATuple();
        if ( tupleA && RTFlag::isActive( traceFlag ) ) {
          cout << "    A: " << *(tupleA) << endl;
          cout << "  eqB: " << *(equalTupleB ) << endl;
        }
        while( tupleA != 0 && CompareTuples( tupleA, equalTupleB ) == 0 )
        {
          if( bucketA.size() == MAX_TUPLES_IN_MEMORY )
          {
            relationA = new Relation( tupleA->GetTupleType(), true );
            SaveTo( bucketA, relationA );
            ClearBucket( bucketA );
          }
          if( bucketA.size() > 0 )
            bucketA.push_back( tupleA );
          else
          {
            Tuple *t = tupleA->CloneIfNecessary();
            if( t != tupleA )
              tupleA->DeleteIfAllowed();
            relationA->AppendTuple( t );
            t->DeleteIfAllowed();
          }
          tupleA = NextATuple();
          if ( tupleA && RTFlag::isActive( traceFlag ) ) {
            cout << "    joined A: " << *(tupleA) << endl;
          }
        }
        equalTupleB->DeleteIfAllowed();
        indexA = 0;

        if( bucketA.size() == 0 )
        {
          assert( relationA != 0 );
          iterRelationA = relationA->MakeScan();
          ReadFrom( iterRelationA, bucketA );
        }

        tupleB = NextBTuple();
        if ( tupleB && RTFlag::isActive( traceFlag ) ) { 
          cout << "    B: " << *(tupleB) << endl;
          cout << "  eqA: " << *(equalTupleA) << endl;
        }
        while( tupleB != 0 && CompareTuples( equalTupleA, tupleB ) == 0 )
        {
          if( bucketB.size() == MAX_TUPLES_IN_MEMORY )
          {
            relationB = new Relation( tupleB->GetTupleType(), true );
            SaveTo( bucketB, relationB );
            ClearBucket( bucketB );
          }
          if( bucketB.size() > 0 )
            bucketB.push_back( tupleB );
          else
          {
            Tuple *t = tupleB->CloneIfNecessary();
            if( t != tupleB )
              tupleB->DeleteIfAllowed();
            relationB->AppendTuple( t );
            t->DeleteIfAllowed();
          }
          tupleB = NextBTuple();
          if ( tupleB && RTFlag::isActive( traceFlag )) {
            cout << "    joined B: " << *(tupleB) << endl;
          }
        }
        equalTupleA->DeleteIfAllowed();
        indexB = 0;

        if( bucketB.size() == 0 )
        {
          assert( relationB != 0 );
          iterRelationB = relationB->MakeScan();
          ReadFrom( iterRelationB, bucketB );
        }

        if( bucketA.size() == 1 && bucketB.size() == 1 )
        // Only one equal tuple.
        {
          assert( relationA == 0 && relationB == 0 );
          resultTuple = new Tuple( *resultTupleType, false );
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
  Word attrIndexA;
  Word attrIndexB;

  switch(message)
  {
    case OPEN:
      qp->Request(args[4].addr, attrIndexA);
      qp->Request(args[5].addr, attrIndexB);
      localInfo = new MergeJoinLocalInfo
        (args[0], attrIndexA, args[1], attrIndexB, expectSorted, s);
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
2.3 Operator ~oldhashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.3.1 Auxiliary definitions for value mapping function of operator ~hashjoin~

*/

CPUTimeMeasurer hashMeasurer;  // measures cost of distributing into buckets and
                               // of computing products of buckets
CPUTimeMeasurer bucketMeasurer;// measures the cost of producing the tuples in
                               // the result set

class OldHashJoinLocalInfo
{
private:
  size_t nBuckets;
  size_t MAX_TUPLES_IN_BUCKET;

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;

  Word tupleA;
  vector< vector<Tuple*> > bucketsB;
  vector<Tuple*>::iterator iterTuplesBucketB;

  vector<Relation*> relBucketsB;
  RelationIterator* iterTuplesRelBucketB;

  size_t hashA;

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
    return (((StandardAttribute*)tuple->GetAttribute(attrIndex))->HashValue() % nBuckets);
  }

  void SaveTo( vector<Tuple*>& bucket, Relation *rel )
  {
    vector<Tuple*>::iterator iter = bucket.begin();
    while( iter != bucket.end() )
    {
      Tuple *t = (*iter)->CloneIfNecessary();
      rel->AppendTuple( t );
      t->DeleteIfAllowed();
      iter++;
    }
  }

  int ReadFrom( RelationIterator *iter, vector<Tuple*>& bucket )
  {
    size_t i = 0;
    Tuple *t;

    while( i < MAX_TUPLES_IN_BUCKET && (t = iter->GetNextTuple()) != 0 )
    {
      t->SetFree( false );
      bucket.push_back( t );
      i++;
    }
    return i;
  }

  void ClearBucket( vector<Tuple*>& bucket )
  {
    vector<Tuple*>::iterator i = bucket.begin();
    while( i != bucket.end() )
    {
      delete (*i);
      i++;
    }
    bucket.clear();
  }

  void FillHashBucketsB()
  {
    Word tupleWord;
    qp->Open(streamB.addr);
    qp->Request(streamB.addr, tupleWord);

    if(qp->Received(streamB.addr))
    {
      Tuple *tupleB = (Tuple*)tupleWord.addr;
//      MAX_TUPLES_IN_BUCKET = qp->MemoryAvailableForOperator() / ( nBuckets * tupleB->GetMemorySize() );
      MAX_TUPLES_IN_BUCKET = 16 * 1024 * 1024 / ( nBuckets * tupleB->GetMemorySize() );
      cout << "HashJoin.MAX_TUPLES_IN_BUCKET: " << MAX_TUPLES_IN_BUCKET << endl;
    }

    while(qp->Received(streamB.addr))
    {
      hashMeasurer.Enter();

      Tuple* tupleB = ((Tuple*)tupleWord.addr)->CloneIfNecessary();
      ((Tuple*)tupleWord.addr)->DeleteIfAllowed();
      size_t hashB = HashTuple(tupleB, attrIndexB);

      if( bucketsB[hashB].size() == MAX_TUPLES_IN_BUCKET )
      {
        relBucketsB[hashB] = new Relation( tupleB->GetTupleType(), true );
        SaveTo( bucketsB[hashB], relBucketsB[hashB] );
        ClearBucket( bucketsB[hashB] );
      }

      if( relBucketsB[hashB] == 0 )
      {
        tupleB->SetFree( false );
        bucketsB[hashB].push_back( tupleB );
      }
      else
      {
        relBucketsB[hashB]->AppendTuple( tupleB );
        tupleB->DeleteIfAllowed();
      }
      hashMeasurer.Exit();

      qp->Request(streamB.addr, tupleWord);
    }
    qp->Close(streamB.addr);
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

  void ClearRelationsB()
  {
    delete iterTuplesRelBucketB;

    vector< Relation* >::iterator iterBuckets = relBucketsB.begin();

    while(iterBuckets != relBucketsB.end() )
    {
      if( (*iterBuckets) != 0 )
        delete *iterBuckets;
      iterBuckets++;
    }

  }

public:
  static const size_t MAX_BUCKETS = 257;
  static const size_t MIN_BUCKETS = 1;
  static const size_t DEFAULT_BUCKETS = 97;

  OldHashJoinLocalInfo(Word streamA, Word attrIndexAWord,
    Word streamB, Word attrIndexBWord, Word nBucketsWord,
    Supplier s)
  {
    this->streamA = streamA;
    this->streamB = streamB;

    ListExpr resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    attrIndexA = ((CcInt*)attrIndexAWord.addr)->GetIntval() - 1;
    attrIndexB = ((CcInt*)attrIndexBWord.addr)->GetIntval() - 1;
    nBuckets = ((CcInt*)nBucketsWord.addr)->GetIntval();
    if(nBuckets < MIN_BUCKETS)
    {
      nBuckets = MIN_BUCKETS;
    }
    else if(nBuckets > MAX_BUCKETS)
    {
      nBuckets = MAX_BUCKETS;
    }

    hashMeasurer.Enter();

    bucketsB.resize(nBuckets);
    relBucketsB.resize(nBuckets);

    for(size_t i = 0; i < nBuckets; i++ )
      relBucketsB[i] = 0;

    iterTuplesRelBucketB = 0;

    hashMeasurer.Exit();

    FillHashBucketsB();

    qp->Open(streamA.addr);
    qp->Request( streamA.addr, tupleA );
    if( qp->Received(streamA.addr) )
    {
      hashA = HashTuple((Tuple*)tupleA.addr, attrIndexA);
      iterTuplesBucketB = bucketsB[hashA].begin();
    }
  }

  ~OldHashJoinLocalInfo()
  {
    ClearBucketsB();
    ClearRelationsB();
    qp->Close(streamA.addr);
    delete resultTupleType;
  }

  Tuple* NextTupleB( size_t hashA )
  {
    if( iterTuplesBucketB != bucketsB[hashA].end() )
    {
      Tuple *result = *iterTuplesBucketB;
      iterTuplesBucketB++;
      return result;
    }

    if( relBucketsB[hashA] != 0 )
    {
      if( iterTuplesRelBucketB == 0 )
        iterTuplesRelBucketB = relBucketsB[hashA]->MakeScan();

      if( !bucketsB[hashA].empty() )
        ClearBucket( bucketsB[hashA] );

      if( ReadFrom( iterTuplesRelBucketB, bucketsB[hashA] ) == 0 )
      {
        delete iterTuplesRelBucketB;
        iterTuplesRelBucketB = 0;
        return 0;
      }

      iterTuplesBucketB = bucketsB[hashA].begin();

      return NextTupleB( hashA );
    }

    iterTuplesRelBucketB = 0;
    return 0;
  }

  Tuple* NextResultTuple()
  {
    Tuple *result;

    while( tupleA.addr != 0 )
    {
      Tuple *tupleB;
      while( (tupleB = NextTupleB( hashA )) != 0 )
      {
        if( CompareTuples( (Tuple *)tupleA.addr, tupleB ) == 0 )
        {
          result = new Tuple( *resultTupleType, true );
          Concat( (Tuple *)tupleA.addr, tupleB, result );

          return result;
        }
      }
      ((Tuple*)tupleA.addr)->DeleteIfAllowed();

      qp->Request( streamA.addr, tupleA );
      if( qp->Received(streamA.addr) )
      {
        hashA = HashTuple((Tuple*)tupleA.addr, attrIndexA);
        iterTuplesBucketB = bucketsB[hashA].begin();
      }
    }
    return 0;
  }
};

/*
2.3.2 Value Mapping Function of Operator ~hashjoin~

*/
int OldHashJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  OldHashJoinLocalInfo* localInfo;
  Word attrIndexA;
  Word attrIndexB;
  Word nHashBuckets;

  switch(message)
  {
    case OPEN:
      qp->Request(args[5].addr, attrIndexA);
      qp->Request(args[6].addr, attrIndexB);
      qp->Request(args[4].addr, nHashBuckets);
      localInfo = new OldHashJoinLocalInfo(args[0], attrIndexA,
        args[1], attrIndexB, nHashBuckets, s);
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (OldHashJoinLocalInfo*)local.addr;
      result = SetWord(localInfo->NextResultTuple());
      return result.addr != 0 ? YIELD : CANCEL;
    case CLOSE:
      hashMeasurer.PrintCPUTimeAndReset("CPU Time for Hashing Tuples : ");
      bucketMeasurer.PrintCPUTimeAndReset(
        "CPU Time for Computing Products of Buckets : ");

      localInfo = (OldHashJoinLocalInfo*)local.addr;
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
  size_t MAX_TUPLES_IN_MEMORY;

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;

  Tuple *tupleA;
  TupleBuffer* relA;
  TupleBufferIterator* iterTuplesRelA;
  bool firstPassA;
  size_t hashA;

  vector< vector<Tuple*> > bucketsB;
  vector<Tuple*>::iterator iterTuplesBucketB;
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
    return (((StandardAttribute*)tuple->GetAttribute(attrIndex))->HashValue() % nBuckets);
  }

  void ClearBucket( vector<Tuple*>& bucket )
  {
    vector<Tuple*>::iterator i = bucket.begin();
    while( i != bucket.end() )
    {
      delete (*i);
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
        Tuple *tupleB = (Tuple*)wTupleB.addr;
//        MAX_TUPLES_IN_MEMORY = qp->MemoryAvailableForOperator() / ( tupleB->GetMemorySize() );
        MAX_TUPLES_IN_MEMORY = 12 * 1024 * 1024 / tupleB->GetMemorySize();
        cout << "HashJoin.MAX_TUPLES_IN_MEMORY: " << MAX_TUPLES_IN_MEMORY << endl;
      }
    }

    size_t i = 0;
    while(qp->Received(streamB.addr) && i++ < MAX_TUPLES_IN_MEMORY)
    {
      Tuple* tupleB = ((Tuple*)wTupleB.addr)->CloneIfNecessary();
      if( tupleB != wTupleB.addr )
        ((Tuple*)wTupleB.addr)->DeleteIfAllowed();
      size_t hashB = HashTuple(tupleB, attrIndexB);

      tupleB->SetFree( false );
      bucketsB[hashB].push_back( tupleB );

      qp->Request(streamB.addr, wTupleB);
    }

    bool remainTuples = false;
    if( i >= MAX_TUPLES_IN_MEMORY && qp->Received(streamB.addr) )
      remainTuples = true;

    if( !remainTuples )
      qp->Close(streamB.addr);

    return remainTuples;
  }

public:
  static const size_t MIN_BUCKETS = 3;
  static const size_t DEFAULT_BUCKETS = 97;

  HashJoinLocalInfo(Word streamA, Word attrIndexAWord,
    Word streamB, Word attrIndexBWord, Word nBucketsWord,
    Supplier s)
  {
    this->streamA = streamA;
    this->streamB = streamB;

    ListExpr resultType = SecondoSystem::GetCatalog( ExecutableLevel )->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    attrIndexA = ((CcInt*)attrIndexAWord.addr)->GetIntval() - 1;
    attrIndexB = ((CcInt*)attrIndexBWord.addr)->GetIntval() - 1;
    nBuckets = ((CcInt*)nBucketsWord.addr)->GetIntval();
    if(nBuckets < MIN_BUCKETS)
    {
      nBuckets = MIN_BUCKETS;
    }

    bucketsB.resize(nBuckets);
    relA = 0;
    iterTuplesRelA = 0;
    firstPassA = true;
    tupleA = 0;

    qp->Open(streamB.addr);
    remainTuplesB = FillHashBucketsB();
    bFitsInMemory  = !remainTuplesB;

    if( !bFitsInMemory )
      relA = new TupleBuffer( 4 * 1024 * 1024 );

    qp->Open(streamA.addr);
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
    if( !bFitsInMemory )
    {
      assert( relA != 0 );
      relA->Clear();
      delete relA;
    }
    delete resultTupleType;
  }

  bool NextTupleA()
  {
    if( tupleA != 0 )
    {
      if( firstPassA && !bFitsInMemory )
        relA->AppendTuple( tupleA );
      tupleA->DeleteIfAllowed();
    }

    if( firstPassA )
    {
      Word wTupleA;
      qp->Request( streamA.addr, wTupleA );
      if( qp->Received(streamA.addr) )
      {
        tupleA = ((Tuple*)wTupleA.addr)->CloneIfNecessary();
        if( tupleA != wTupleA.addr )
          ((Tuple*)wTupleA.addr)->DeleteIfAllowed();
      }
      else
      {
        tupleA = 0;
        qp->Close(streamA.addr);
        return false;
      }
    }
    else
    {
      assert( !bFitsInMemory );
      assert( iterTuplesRelA != 0 );

      if( (tupleA = iterTuplesRelA->GetNextTuple()) == 0 )
      {
	delete iterTuplesRelA;
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
          Tuple *result = new Tuple( *resultTupleType, true );
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
          assert( relA != 0 );
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
  Word attrIndexA;
  Word attrIndexB;
  Word nHashBuckets;

  switch(message)
  {
    case OPEN:
      qp->Request(args[5].addr, attrIndexA);
      qp->Request(args[6].addr, attrIndexB);
      qp->Request(args[4].addr, nHashBuckets);
      localInfo = new HashJoinLocalInfo(args[0], attrIndexA,
        args[1], attrIndexB, nHashBuckets, s);
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
SortBy<false, true>(Word* args, Word& result, int message, Word& local, Supplier s);
template int
SortBy<true, true>(Word* args, Word& result, int message, Word& local, Supplier s);
template int
MergeJoin<true>(Word* args, Word& result, int message, Word& local, Supplier s);
template int
MergeJoin<false>(Word* args, Word& result, int message, Word& local, Supplier s);

#endif // RELALG_PERSISTENT
