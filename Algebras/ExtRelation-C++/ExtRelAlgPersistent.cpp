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
class SortByLocalInfo
{
  public:
    static const size_t MAX_TUPLES_IN_MEMORY;

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
		  tupleType = new TupleType( ((Tuple*)wTuple.addr)->GetTupleType() );

        while(qp->Received(stream.addr))
        {
          tuples.push_back((Tuple*)wTuple.addr);
		  if( ++i == MAX_TUPLES_IN_MEMORY )
		  {
			if( lexicographic )
              sort(tuples.begin(), tuples.end(), *lexiTupleCmp );
            else
              sort(tuples.begin(), tuples.end(), *tupleCmpBy );

            Relation *rel = new Relation( *tupleType, true );
            RelationIterator *iter = rel->MakeScan();
            SaveTo( *rel );
            ClearMemory();
            relations.push_back( pair<Relation*, RelationIterator*>( rel, iter ) );
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
          RelationIterator *iter = rel->MakeScan();
          SaveTo( *rel );
          ClearMemory();
          relations.push_back( pair<Relation*, RelationIterator*>( rel, iter ) );
	    }
      }

    ~SortByLocalInfo()
    {
	  ClearMemory();
	  for( size_t i = 0; i < relations.size(); i++ )
	  {
		delete relations[i].second;
	    relations[i].first->Delete();
      }
	}

    Tuple *NextResultTuple()
    {
	  if( tuples.size() == 0 || currentIndex == tuples.size() )
	  {
	    if( relations.size() == 0 )
	      return 0;

        if( currentIndex == tuples.size() )
          ClearMemory();

	    // Get next tuple from each relation and fill the vector.
       for( size_t i = 0; i < relations.size(); i++ )
       {
         Tuple *t = relations[i].second->GetNextTuple();
         if( t != 0 )
           tuples.push_back( t );
       }
       currentIndex = 0;

       if( lexicographic )
         sort(tuples.begin(), tuples.end(), *lexiTupleCmp );
       else
         sort(tuples.begin(), tuples.end(), *tupleCmpBy );
      }

      assert( currentIndex < tuples.size() );
      return tuples[currentIndex++];
    }

  private:
    void ClearMemory()
    {
//      vector<Tuple*>::iterator iter = tuples.begin();
//      while( iter != tuples.end() )
//        delete *iter;
      tuples.clear();
    }

    void SaveTo( Relation &rel )
    {
      vector<Tuple*>::iterator iter = tuples.begin();
      while( iter != tuples.end() )
	    rel.AppendTuple( (*iter) );
    }

    Word stream;
    vector<Tuple*> tuples;
    size_t currentIndex;
    LexicographicalTupleCompare *lexiTupleCmp;
    TupleCompareBy *tupleCmpBy;
    bool lexicographic;
    TupleType *tupleType;
    vector< pair<Relation*, RelationIterator*> > relations;
};

const size_t SortByLocalInfo::MAX_TUPLES_IN_MEMORY = 1024;

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
      Word result = SetWord( localInfo->NextResultTuple() );
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

2.2.1 Value mapping function of operator ~mergejoin~

*/

template<bool expectSorted> int
MergeJoin(Word* args, Word& result, int message, Word& local, Supplier s)
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
