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
      return 0;

    case REQUEST:
      return CANCEL;

    case CLOSE:
      return 0;
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
