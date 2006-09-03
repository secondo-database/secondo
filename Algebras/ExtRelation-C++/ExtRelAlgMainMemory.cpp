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

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

[1] Implementation of the Module Extended Relation Algebra for Main Memory

[TOC]

1 Includes and defines

*/
#ifndef RELALG_PERSISTENT

#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "QueryProcessor.h"
#include "vector"
#include "deque"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
2 Operators

2.1 Operators ~sort~ and ~sortby~

This operator sorts a stream of tuples by a given list of attributes.
For each attribute it must be specified wether the list should be sorted
in ascending (asc) or descending (desc) order with regard to that attribute.

2.1.1 Value mapping function of operator ~sortBy~

The argument vector ~args~ contains in the first slot ~args[0]~ the stream and
in ~args[2]~ the number of sort attributes. ~args[3]~ contains the index of the first
sort attribute, ~args[4]~ a boolean indicating wether the stream is sorted in
ascending order with regard to the sort first attribute. ~args[5]~ and ~args[6]~
contain these values for the second sort attribute  and so on.


*/
struct SortByLocalInfo
{
  vector<Tuple*>* tuples;
  size_t currentIndex;
};

CPUTimeMeasurer sortMeasurer;

template<bool lexicographically, bool requestArgs> int
SortBy(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word tuple;
  vector<Tuple*>* tuples;
  SortByLocalInfo* localInfo;
  SortOrderSpecification spec;
  int i;
  Word intWord;
  Word boolWord;
  size_t j;
  int sortAttrIndex;
  int nSortAttrs;
  bool sortOrderIsAscending;
  TupleCompareBy *tupCmpBy;
  LexicographicalTupleCompare lexCmp;
  Tuple* t;

  switch(message)
  {
    case OPEN:
      tuples = new vector<Tuple*>;
      qp->Open(args[0].addr);
      qp->Request(args[0].addr,tuple);
      while(qp->Received(args[0].addr))
      {
        t =(Tuple*)tuple.addr;
        t->IncReference();
        tuples->push_back(t);
        qp->Request(args[0].addr,tuple);
      }
      qp->Close(args[0].addr);

      if(lexicographically)
      {
        sortMeasurer.Enter();
        sort(tuples->begin(), tuples->end(), lexCmp);
        sortMeasurer.Exit();
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
        for(i = 1; i <= nSortAttrs; i++)
        {
	  if(requestArgs)
          {
            qp->Request(args[2 * i + 1].addr, intWord);
          }
          else
          {
            intWord = SetWord(args[2 * i + 1].addr);
          }
          sortAttrIndex = ((CcInt*)intWord.addr)->GetIntval();

          if(requestArgs)
          {
            qp->Request(args[2 * i + 2].addr, boolWord);
          }
          else
          {
            boolWord = SetWord(args[2 * i + 2].addr);
          }
          sortOrderIsAscending = ((CcBool*)boolWord.addr)->GetBoolval();
          spec.push_back(pair<int, bool>(sortAttrIndex, sortOrderIsAscending));
        };

        tupCmpBy = new TupleCompareBy( spec );
        sortMeasurer.Enter();
        sort(tuples->begin(), tuples->end(), *tupCmpBy);
        sortMeasurer.Exit();
        delete tupCmpBy;
      }

      sortMeasurer.PrintCPUTimeAndReset("CPU Time for Sorting Tuples : ");

      localInfo = new SortByLocalInfo;
      localInfo->tuples = tuples;
      localInfo->currentIndex = 0;
      local = SetWord(localInfo);
      return 0;
    case REQUEST:
      localInfo = (SortByLocalInfo*)local.addr;
      tuples = localInfo->tuples;
      if(localInfo->currentIndex  + 1 <= tuples->size())
      {
        result = SetWord((*tuples)[localInfo->currentIndex]);
        localInfo->currentIndex++;
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    case CLOSE:
      localInfo = (SortByLocalInfo*)local.addr;

      for(j = localInfo->currentIndex;
        j + 1 <= localInfo->tuples->size(); j++)
      {
        (*(localInfo->tuples))[j]->DecReference();
        (*(localInfo->tuples))[j]->DeleteIfAllowed();
      }

      delete localInfo->tuples;
      delete localInfo;
      return 0;
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
  vector<Tuple*> bucketB;
  deque<Tuple*> resultBucket;

  Word aResult;
  Word bResult;

  Word streamALocalInfo;
  Word streamBLocalInfo;

  Word streamA;
  Word streamB;

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

    return 
      ((Attribute*)a->GetAttribute(attrIndexA))->
        Compare((Attribute*)b->GetAttribute(attrIndexB));
  }

  void SetArgs(ArgVector& args, Word stream, Word attrIndex)
  {
    args[0] = SetWord(stream.addr);
    args[2] = SetWord(&oneCcInt);
    args[3] = SetWord(attrIndex.addr);
    args[4] = SetWord(&trueCcBool);
  }

  Tuple* nextATuple()
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
        SortBy<false, false>(aArgs, aResult, REQUEST, streamALocalInfo, 0);
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

  Tuple* nextBTuple()
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
        SortBy<false, false>(bArgs, bResult, REQUEST, streamBLocalInfo, 0);
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

  bool FetchNextMatch()
  {
    Tuple* aTuple = (Tuple*)aResult.addr;
    Tuple* bTuple = (Tuple*)bResult.addr;
    if(aTuple == 0 || bTuple == 0)
    {
      if(aTuple != 0)
        aTuple->DeleteIfAllowed();
      if(bTuple != 0)
        bTuple->DeleteIfAllowed();

      return false;
    }
    else
    {
      int cmpResult = CompareTuples((Tuple*)aResult.addr, (Tuple*)bResult.addr);
      while(cmpResult != 0)
      {
        if(cmpResult < 0)
        {
          ((Tuple*)aResult.addr)->DeleteIfAllowed();
          if(nextATuple() == 0)
          {
            ((Tuple*)bResult.addr)->DeleteIfAllowed();
            return false;
          }
        }
        else
        {
          ((Tuple*)bResult.addr)->DeleteIfAllowed();
          if(nextBTuple() == 0)
          {
            ((Tuple*)aResult.addr)->DeleteIfAllowed();
            return false;
          }
        }
        cmpResult = CompareTuples((Tuple*)aResult.addr, (Tuple*)bResult.addr);
      }
      return true;
    }
  }

  void ComputeProductOfBuckets()
  {
    assert(!bucketA.empty());
    assert(!bucketB.empty());

    vector<Tuple*>::iterator iterA = bucketA.begin();
    vector<Tuple*>::iterator iterB = bucketB.begin();
    for(; iterA != bucketA.end(); iterA++)
    {
      for(iterB = bucketB.begin(); iterB != bucketB.end(); iterB++)
      {
        Tuple* resultTuple = new Tuple( resultTupleType );
        Concat(*iterA, *iterB, resultTuple);
        resultBucket.push_back(resultTuple);
      }
    }
  }

  void ClearBuckets()
  {
    vector<Tuple*>::iterator iterA = bucketA.begin();
    vector<Tuple*>::iterator iterB = bucketB.begin();

    for(; iterA != bucketA.end(); iterA++)
    {
      (*iterA)->DeleteIfAllowed();
    }

    for(; iterB != bucketB.end(); iterB++)
    {
      (*iterB)->DeleteIfAllowed();
    }

    bucketA.clear();
    bucketB.clear();
  }

  void FillResultBucket()
  {
    assert((Tuple*)aResult.addr != 0);
    assert((Tuple*)bResult.addr != 0);

    Tuple* aMatch = (Tuple*)aResult.addr;
    Tuple* bMatch = (Tuple*)bResult.addr;
    assert(CompareTuples(aMatch, bMatch) == 0);

    Tuple* currentA = aMatch;
    Tuple* currentB = bMatch;

    while(currentA != 0 && CompareTuples(currentA, bMatch) == 0)
    {
      bucketA.push_back(currentA);
      currentA = nextATuple();
    }

    while(currentB != 0 && CompareTuples(aMatch, currentB) == 0)
    {
      bucketB.push_back(currentB);
      currentB = nextBTuple();
    }

    ComputeProductOfBuckets();
    ClearBuckets();
  }

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

    aResult = SetWord(0);
    bResult = SetWord(0);

    this->expectSorted = expectSorted;
    this->streamA = streamA;
    this->streamB = streamB;
    this->attrIndexA = ((CcInt*)attrIndexA.addr)->GetIntval() - 1;
    this->attrIndexB = ((CcInt*)attrIndexB.addr)->GetIntval() - 1;

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

    ListExpr resultType = 
      SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    nextATuple();
    nextBTuple();
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
    resultTupleType->DeleteIfAllowed();
  }

  Tuple* NextResultTuple()
  {
    if(resultBucket.empty())
    {
      if(FetchNextMatch())
      {
        FillResultBucket();
      }
      else
      {
        return 0;
      }
    }
    Tuple* next = resultBucket.front();
    resultBucket.pop_front();
    return next;
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

CPUTimeMeasurer hashMeasurer;  // measures cost of distributing into buckets and
                               // of computing products of buckets
CPUTimeMeasurer bucketMeasurer;// measures the cost of producing the tuples in
                               // the result set

class HashJoinLocalInfo
{
private:
  static const size_t MAX_BUCKETS = 6151;
  static const size_t MIN_BUCKETS = 1;
  static const size_t DEFAULT_BUCKETS = 97;
  size_t nBuckets;

  int attrIndexA;
  int attrIndexB;

  Word streamA;
  Word streamB;

  Word tupleA;
  vector<vector< Tuple*> > bucketsB;

  vector<Tuple*>::iterator iterTuplesBucketB;
  size_t hashA;
  bool endquery;

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

  void FillHashBucketsB()
  {
    Word tupleWord;
    qp->Open(streamB.addr);
    qp->Request(streamB.addr, tupleWord);
    while(qp->Received(streamB.addr))
    {
      hashMeasurer.Enter();

      Tuple* tuple = (Tuple*)tupleWord.addr;
      size_t hashB = HashTuple(tuple, attrIndexB);
      bucketsB[hashB].push_back(tuple);

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
      vector<Tuple*>::iterator iter = (*iterBuckets).begin();
      while(iter != (*iterBuckets).end())
      {
        (*iter)->DeleteIfAllowed();
        iter++;
      }
      iterBuckets++;
    }
  }

public:
  HashJoinLocalInfo(Word streamA, Word attrIndexAWord,
    Word streamB, Word attrIndexBWord, Word nBucketsWord,
    Supplier s)
  {
    this->streamA = streamA;
    this->streamB = streamB;

    ListExpr resultType = 
      SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
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

    hashMeasurer.Exit();

    FillHashBucketsB();

    qp->Open(streamA.addr);
    qp->Request( streamA.addr, tupleA );
    if( qp->Received(streamA.addr) )
    {
      hashA = HashTuple((Tuple*)tupleA.addr, attrIndexA);
      iterTuplesBucketB = bucketsB[hashA].begin();
      endquery = false;
    }
    else
      endquery = true;
  }

  ~HashJoinLocalInfo()
  {
    ClearBucketsB();
    qp->Close(streamA.addr);
    resultTupleType->DeleteIfAllowed();
  }

  Tuple* NextResultTuple()
  {
    Tuple *result;

    while( !endquery )
    {
      assert( tupleA.addr != 0 );
      while( iterTuplesBucketB != bucketsB[hashA].end() )
      {
        if( CompareTuples( (Tuple *)tupleA.addr, *iterTuplesBucketB ) == 0 )
        {
          result = new Tuple( resultTupleType );
          Concat( (Tuple *)tupleA.addr, *iterTuplesBucketB, result );
          iterTuplesBucketB++;
          return result;
        }
        iterTuplesBucketB++;
      }
      ((Tuple*)tupleA.addr)->DeleteIfAllowed();
      qp->Request( streamA.addr, tupleA );
      if( qp->Received(streamA.addr) )
      {
        hashA = HashTuple((Tuple*)tupleA.addr, attrIndexA);
        iterTuplesBucketB = bucketsB[hashA].begin();
      }
      else
        endquery = true;
    }
    return 0;
  }
};

/*
2.3.2 Value Mapping Function of Operator ~oldhashjoin~

*/
int OldHashJoin(Word* args, Word& result, int message, Word& local, Supplier s)
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
      hashMeasurer.PrintCPUTimeAndReset("CPU Time for Hashing Tuples : ");
      bucketMeasurer.PrintCPUTimeAndReset(
        "CPU Time for Computing Products of Buckets : ");

      localInfo = (HashJoinLocalInfo*)local.addr;
      delete localInfo;
      return 0;
  }
  return 0;
}

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
      hashMeasurer.PrintCPUTimeAndReset("CPU Time for Hashing Tuples : ");
      bucketMeasurer.PrintCPUTimeAndReset(
        "CPU Time for Computing Products of Buckets : ");

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
SortBy<false, true>(Word* args, Word& result, int message, 
                    Word& local, Supplier s);
template int
SortBy<true, true>(Word* args, Word& result, int message, 
                   Word& local, Supplier s);
template int
MergeJoin<true>(Word* args, Word& result, int message, 
                Word& local, Supplier s);
template int
MergeJoin<false>(Word* args, Word& result, int message, 
                 Word& local, Supplier s);

#endif // RELALG_PERSISTENT
