/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Module Extended Relation Algebra for Persistent storage

[TOC]

1 Includes and defines

*/
#ifdef RELALG_PERSISTENT

#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"

#include <vector>
#include <list>
#include <set>

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
class LexicographicalPairTupleCompare 
{
  public:
    LexicographicalPairTupleCompare( LexicographicalTupleCompare *lexCmp ):
      lexCmp( lexCmp )
      {}

    bool operator()(const pair<size_t, Tuple*>& aConst, const pair<size_t, Tuple*>& bConst) const
    {
      Tuple *a = aConst.second,
            *b = bConst.second;
      return (*lexCmp)( a, b );
    }

  private:
    LexicographicalTupleCompare *lexCmp;
};

class PairTupleCompareBy
{
  public:
    PairTupleCompareBy( TupleCompareBy *tupCmp ):
      tupCmp( tupCmp )
      {}

    bool operator()(const pair<size_t, Tuple*>& aConst, const pair<size_t, Tuple*>& bConst) const
    {
      Tuple *a = aConst.second,
            *b = bConst.second;
      return (*tupCmp)( a, b );
    }

  private:
    TupleCompareBy *tupCmp;
};

class SortByLocalInfo
{
  public:
    static const size_t MAX_MEMORY_SIZE;

    SortByLocalInfo( Word stream, const bool lexicographic, TupleCompare *tupleCmp ):
      stream( stream ),
      currentIndex( 0 ),
      lexiTupleCmp( lexicographic ? (LexicographicalTupleCompare*)tupleCmp : 0 ),
      tupleCmpBy( lexicographic ? 0 : (TupleCompareBy*)tupleCmp ),
      lexicographic( lexicographic ),
      tupleType( 0 )
      {
        Word wTuple;
        size_t i = 0;

        qp->Open(stream.addr);
        qp->Request(stream.addr, wTuple);

        if(qp->Received(stream.addr))
        {
          tupleType = new TupleType( ((Tuple*)wTuple.addr)->GetTupleType() );
          MAX_TUPLES_IN_MEMORY = MAX_MEMORY_SIZE / ((Tuple*)wTuple.addr)->GetMemorySize();
        }

        while(qp->Received(stream.addr))
        {
          Tuple *t = ((Tuple*)wTuple.addr)->CloneIfNecessary();
          if( t != wTuple.addr )
            ((Tuple*)wTuple.addr)->Delete();

          tuples.push_back( t );
          if( ++i == MAX_TUPLES_IN_MEMORY )
          {
            if( lexicographic )
              sort(tuples.begin(), tuples.end(), *lexiTupleCmp );
            else
              sort(tuples.begin(), tuples.end(), *tupleCmpBy );

            Relation *rel = new Relation( *tupleType, true );
            SaveTo( *rel );
            tuples.clear();
            RelationIterator *iter = rel->MakeScan();
            relations.push_back( pair<Relation*, RelationIterator*>( rel, iter ) );

            i = 0;
          }
          qp->Request(stream.addr, wTuple);
        }
        qp->Close(stream.addr);

        if( lexicographic )
          sort(tuples.begin(), tuples.end(), *lexiTupleCmp );
        else
          sort(tuples.begin(), tuples.end(), *tupleCmpBy );

        if( relations.size() > 0 )
        {
          Relation *rel = new Relation( *tupleType, true );
          SaveTo( *rel );
          tuples.clear();
          RelationIterator *iter = rel->MakeScan();
          relations.push_back( pair<Relation*, RelationIterator*>( rel, iter ) );

          // Get next tuple from each relation and fill the vector.
          for( size_t i = 0; i < relations.size(); i++ )
          {
            Tuple *t = relations[i].second->GetNextTuple();
            if( t != 0 )
            {
              mergeTuples.push_back( pair<size_t, Tuple*>(i, t) );
            }
          }

          // Ordering the merged tuples.
          if( lexicographic )
            sort(mergeTuples.begin(), mergeTuples.end(), LexicographicalPairTupleCompare( lexiTupleCmp ) );
          else
            sort(mergeTuples.begin(), mergeTuples.end(), PairTupleCompareBy( tupleCmpBy ) );
        }
      }

    ~SortByLocalInfo()
    {
      for( size_t i = 0; i < relations.size(); i++ )
      {
        delete relations[i].second;
        relations[i].first->Delete();
      }
      delete tupleType;
    }

    Tuple *NextResultTuple()
    {
      if( !tuples.empty() )
      // The tuples fit in memory.
      {
        if( currentIndex < tuples.size() )
          return tuples[currentIndex++];
        else
          return 0;
      }
      else
      {
        if( relations.size() == 0 )
        // No tuples to sort.
          return 0;

        if( mergeTuples.empty() )
        // No tuples to sort.
          return 0;

        // Take the first one.
        size_t relationPos = mergeTuples[0].first;
        Tuple *result = mergeTuples[0].second;
        Tuple *t = relations[relationPos].second->GetNextTuple();
        if( t != 0 )
        {
          mergeTuples[0].second = t;
       
          if( lexicographic )
            sort(mergeTuples.begin(), mergeTuples.end(), LexicographicalPairTupleCompare( lexiTupleCmp ) );
          else
            sort(mergeTuples.begin(), mergeTuples.end(), PairTupleCompareBy( tupleCmpBy ) );
        }
        else
        {
          mergeTuples.erase( mergeTuples.begin() );
        }
        return result;
      }
    }

  private:
    void SaveTo( Relation &rel )
    {
      vector<Tuple*>::iterator iter = tuples.begin();
      while( iter != tuples.end() )
      {
        Tuple *t =  (*iter)->CloneIfNecessary();
        rel.AppendTuple( t );
        t->Delete();
        iter++;
      }
    }

    size_t MAX_TUPLES_IN_MEMORY;
    Word stream;
    vector<Tuple*> tuples;
    vector< pair<size_t, Tuple*> > mergeTuples;
    size_t currentIndex;
    LexicographicalTupleCompare *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;
    TupleType *tupleType;
    vector< pair<Relation*, RelationIterator*> > relations;
};

const size_t SortByLocalInfo::MAX_MEMORY_SIZE = 2097152;

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
      TupleCompare *tupleCmp;
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
        nSortAttrs = (int)((StandardAttribute*)intWord.addr)->GetValue();
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
            (int)((StandardAttribute*)intWord.addr)->GetValue();

          if(requestArgs)
          {
            qp->Request(args[2 * i + 2].addr, boolWord);
          }
          else
          {
            boolWord = SetWord(args[2 * i + 2].addr);
          }
          sortOrderIsAscending =
            (bool*)((StandardAttribute*)boolWord.addr)->GetValue();
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
      rel->AppendTuple( t );
      t->Delete();
      iter++;
    }
  }

public:
  static const size_t MAX_TUPLES_IN_MEMORY;

  MergeJoinLocalInfo(Word streamA, Word attrIndexA,
    Word streamB, Word attrIndexB, bool expectSorted,
    Supplier s)
  {
    assert(streamA.addr != 0);
    assert(streamB.addr != 0);
    assert(attrIndexA.addr != 0);
    assert(attrIndexB.addr != 0);
    assert((int)((StandardAttribute*)attrIndexA.addr)->GetValue() > 0);
    assert((int)((StandardAttribute*)attrIndexB.addr)->GetValue() > 0);

    this->expectSorted = expectSorted;
    this->streamA = streamA;
    this->streamB = streamB;
    this->attrIndexA = (int)((StandardAttribute*)attrIndexA.addr)->GetValue() - 1;
    this->attrIndexB = (int)((StandardAttribute*)attrIndexB.addr)->GetValue() - 1;
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

    NextATuple();
    NextBTuple();
  }

  ~MergeJoinLocalInfo()
  {
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
      assert( bucketB.size() > 0 || relationB != 0 );
      resultTuple = new Tuple( *resultTupleType, false );

      if( bucketB.size() > 0 )
      {
        if( indexB == bucketB.size() )
        {
          indexB = 0;
          indexA++;
        }
        if( indexA == bucketA.size() )
        {
          bucketA.clear(); indexA = 0;
          bucketB.clear(); indexB = 0;
  
          resultTuple = NextResultTuple();
        }
        else
        {
          resultTuple = new Tuple( *resultTupleType, false );
          Concat( bucketA[indexA], bucketB[indexB], resultTuple );
        }
      }
      else // ( relationB != 0 )
      {
        if( (tupleB = iterRelationB->GetNextTuple()) == 0 )
        {
          delete iterRelationB;
          iterRelationB = relationB->MakeScan();
          tupleB = iterRelationB->GetNextTuple();
          indexA++;
        }
        if( indexA == bucketA.size() )
        {
          bucketA.clear(); indexA = 0;
          delete iterRelationB;
          relationB->Delete(); relationB = 0;
          resultTuple = NextResultTuple();
        }
        else
        {
          resultTuple = new Tuple( *resultTupleType, false );
          Concat( bucketA[indexA], tupleB, resultTuple );
        }
      }
    }
    else if( relationA != 0 )
    // There are equal tuples that did not fit in memory for bucket A.
    {
      assert( bucketB.size() > 0 || relationB != 0 );

      if( relationB != 0 )
      {
        if( (tupleB = iterRelationB->GetNextTuple()) == 0 )
        {
          delete iterRelationB;
          iterRelationB = relationB->MakeScan();
          tupleB = iterRelationB->GetNextTuple();
          tupleA = iterRelationA->GetNextTuple();
        }
        if( tupleA == 0 )
        {
          delete iterRelationA;
          relationA->Delete(); relationA = 0;
          delete iterRelationB;
          relationB->Delete(); relationB = 0;

          resultTuple = NextResultTuple();
        }
        else
        {
          resultTuple = new Tuple( *resultTupleType, false );
          Concat( tupleA, tupleB, resultTuple );
        }
      }
      else
      {
        if( indexB == bucketB.size() )
        {
          indexB = 0;
          tupleA = iterRelationA->GetNextTuple();
        }
        if( tupleA == 0 )
        {
          delete iterRelationA;
          relationA->Delete(); relationA = 0;
          bucketB.clear(); indexB = 0;

          resultTuple = NextResultTuple();
        }
        else
        {
          resultTuple = new Tuple( *resultTupleType, false );
          Concat( tupleA, bucketB[indexB], resultTuple );
        }
      }
    }
    else
    // There are no stored equal tuples.
    {
      if( aResult.addr == 0 || bResult.addr == 0 )
      // One of the streams finished.
        return 0;

      tupleA = (Tuple *)aResult.addr;
      tupleB = (Tuple *)bResult.addr;

      int cmp = CompareTuples( tupleA, tupleB );

      if( cmp == 0 )
      // The tuples are equal. We must store them in a buffer if it fits or in 
      // the disk otherwise
      {
        Tuple *equalTuple = tupleA;
        bucketA.push_back( tupleA );
        bucketB.push_back( tupleB );
 
        tupleA = NextATuple();
        while( tupleA != 0 && CompareTuples( equalTuple, tupleA ) == 0 )
        {
          if( bucketA.size() == MAX_TUPLES_IN_MEMORY )
          {
            relationA = new Relation( tupleA->GetTupleType(), true );
            SaveTo( bucketA, relationA );
            bucketA.clear();
          }
          if( bucketA.size() > 0 )
            bucketA.push_back( tupleA );
          else
            relationA->AppendTuple( tupleA );
          tupleA = NextATuple();
        } 

        if( bucketA.size() > 0 )
          indexA = 0;
        else
        {
          assert( relationA != 0 );
          iterRelationA = relationA->MakeScan();
        }

        tupleB = NextBTuple();
        while( tupleB != 0 && CompareTuples( equalTuple, tupleB ) == 0 )
        {
          if( bucketB.size() == MAX_TUPLES_IN_MEMORY )
          {
            relationB = new Relation( tupleB->GetTupleType(), true );
            SaveTo( bucketB, relationB );
            bucketB.clear();
          }
          if( bucketB.size() > 0 )
            bucketB.push_back( tupleB );
          else
            relationB->AppendTuple( tupleB );
          tupleB = NextBTuple();
        } 

        if( bucketB.size() > 0 )
          indexB = 0;
        else
        {
          assert( relationB != 0 );
          iterRelationB = relationB->MakeScan();
        }

        if( bucketA.size() == 1 && bucketB.size() == 1 )
        // Only one equal tuple.
        {
          resultTuple = new Tuple( *resultTupleType, false );
          Concat( bucketA[0], bucketB[0], resultTuple );
          bucketA.clear(); 
          bucketB.clear(); 
        } 
        else
        {
          resultTuple = NextResultTuple();
        }
      }
      else if( cmp > 0 )
      {
        tupleB = NextBTuple();
        while( CompareTuples( tupleA, tupleB ) > 0 )
          tupleB = NextBTuple();

        resultTuple = NextResultTuple();
      }
      else if( cmp < 0 )
      {
        tupleA = NextATuple();
        while( CompareTuples( tupleA, tupleB ) < 0 )
          tupleA = NextATuple();

        resultTuple = NextResultTuple();
      }
    }
    return resultTuple;
  }
};

const size_t MergeJoinLocalInfo::MAX_TUPLES_IN_MEMORY = 2;

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
2.3 Operator ~hashjoin~

This operator computes the equijoin two streams via a hash join.
The user can specify the number of hash buckets.

2.3.2 Value Mapping Function of Operator ~hashjoin~

*/
int HashJoin(Word* args, Word& result, int message, Word& local, Supplier s)
{
  switch(message)
  {
    case OPEN:
      return 0;

    case REQUEST:
      return CANCEL;

    case CLOSE:
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
