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
//          MAX_TUPLES_IN_MEMORY = qp->MemoryAvailableForOperator() / ((Tuple*)wTuple.addr)->GetMemorySize();
          MAX_TUPLES_IN_MEMORY = 16 * 1024 * 1024 / ((Tuple*)wTuple.addr)->GetMemorySize();
          cout << "Sort.MAX_TUPLES_IN_MEMORY: " << MAX_TUPLES_IN_MEMORY << endl;
        }

        while(qp->Received(stream.addr))
        {
          Tuple *t = ((Tuple*)wTuple.addr)->CloneIfNecessary();
          if( t != wTuple.addr )
            ((Tuple*)wTuple.addr)->DeleteIfAllowed();

          t->SetFree( false );
          tuples.push_back( t );
          if( ++i == MAX_TUPLES_IN_MEMORY )
          {
            if( lexicographic )
              sort(tuples.begin(), tuples.end(), *lexiTupleCmp );
            else
              sort(tuples.begin(), tuples.end(), *tupleCmpBy );

            Relation *rel = new Relation( *tupleType, true );
            SaveTo( *rel );
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
      if( !tuples.empty() )
      {
        assert( relations.empty() );
        for( size_t i = 0; i < tuples.size(); i++ )
          delete tuples[i];
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
        Tuple *t = (*iter)->CloneIfNecessary();
        if( t != *iter )
          (*iter)->DeleteIfAllowed();
        rel.AppendTuple( t );
        t->DeleteIfAllowed();
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
    Supplier s)
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

      if( cmp == 0 )
      // The tuples are equal. We must store them in a buffer if it fits or in
      // the disk otherwise
      {
        Tuple *equalTupleB = tupleB->Clone(),
              *equalTupleA = tupleA->Clone();

        bucketA.push_back( tupleA );
        bucketB.push_back( tupleB );

        tupleA = NextATuple();
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
    do
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
    } while( tupleA != 0 );

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
