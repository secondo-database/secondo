/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
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

1 Header File SortMergeJoin.h

Sept 2009, Sven Jungnickel. Initial version

2 Overview

This file contains the declaration of all classes and functions that
implement the ~sortmergejoin2~ operator which uses the new sort
algorithm implemented by operator ~sort2~.

3 Includes and defines

*/

#ifndef SORTMERGEJOIN_H_
#define SORTMERGEJOIN_H_

#include "RelationAlgebra.h"
#include "Progress.h"
#include "TupleBuffer2.h"
#include "Sort.h"

/*
3 Class ~SortMergeJoinLocalInfo~

*/

namespace extrel2
{

class SortMergeJoinLocalInfo : protected ProgressWrapper
{
  public:
    SortMergeJoinLocalInfo( Word streamA, int attrIndexA,
                              Word streamB, int attrIndexB,
                              Supplier s, ProgressLocalInfo* p,
                              size_t maxMemSize = UINT_MAX );
/*
The constructor. Consumes all tuples of the tuple stream ~stream~
immediately into sorted runs of approximately two times the size of the
operators main memory by making use of the replacement selection
algorithm for run generation.The internal main memory sort routine
uses a minimum heap for sorting. For tuple comparisons the algorithm
uses the compare object ~cmpObj~. All progress information will be
stored in ~p~. Additionally the constructor allows to specify the
maximum number of open temporary files during a merge phase, the so
called maximum fan-in ~maxFanIn~ for a merge phase and the maximum
memory ~maxMemSize~ the operator uses for the sort operation. If
~maxMemSize~ is set to UINT\_MAX the sort algorithm uses the maximum
main memory which the operators has been assigned by the query processor.

*/

    ~SortMergeJoinLocalInfo();
/*
The destructor. Frees all resources of the algorithm.

*/

    Tuple* NextResultTuple();
/*
Returns the pointer of the next result tuple. If all tuples
have been processed the method returns 0.

*/

  private:

    inline Tuple* NextConcat()
    {
      Tuple* t = iter->GetNextTuple();
      if( t != 0 )
      {
        Tuple* result = new Tuple( resultTupleType );
        Concat( ptA.tuple, t, result );
        t->DeleteIfAllowed();

        return result;
      }
      return 0;
    }
/*
Returns the pointer of the next result tuple in sort order. If all tuples
have been processed the method returns 0.

*/

    template<bool BOTH_B>
    int CompareTuples(Tuple* t1, Tuple* t2)
    {
      Attribute* a = 0;
      if (BOTH_B)
      {
        a = static_cast<Attribute*>
              ( t1->GetAttribute(attrIndexB-1) );
      }
      else
      {
        a = static_cast<Attribute*>
              ( t1->GetAttribute(attrIndexA-1) );
      }

      Attribute* b = static_cast<Attribute*>
                        ( t2->GetAttribute(attrIndexB-1) );

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

      if (traceMode)
      {
        cmsg.info()
          << "CompareTuples:" << endl
          << "  BOTH_B = " << BOTH_B << endl
          << "  tuple_1  = " << *t1 << " (Ref: "
          << t1->GetNumOfRefs() << ")" << endl
          << "  tuple_2  = " << *t2 << " (Ref: "
          << t2->GetNumOfRefs() << "(" << endl
          << "  cmp(t1,t2) = " << cmp << endl;
        cmsg.send();
      }
      return cmp;
    }
/*
Returns the pointer of the next result tuple in sort order.
If all tuples have been processed the method returns 0.

*/

    inline int CompareTuplesB(Tuple* t1, Tuple* t2)
    {
      return CompareTuples<true>(t1, t2);
    }
/*
Returns the pointer of the next result tuple in sort order.
If all tuples have been processed the method returns 0.

*/

    inline int CompareTuples(Tuple* t1, Tuple* t2)
    {
      return CompareTuples<false>(t1, t2);
    }
/*
Returns the pointer of the next result tuple in sort order.
If all tuples have been processed the method returns 0.

*/

    inline Tuple* NextTupleA()
    {
      progress->readFirst++;
      return sliA->NextResultTuple();
    }
/*
Returns the pointer of the next result tuple in sort order.
If all tuples have been processed the method returns 0.

*/

    inline Tuple* NextTupleB()
    {
      progress->readSecond++;
      return sliB->NextResultTuple();
    }
/*
Returns the pointer of the next result tuple in sort order.
If all tuples have been processed the method returns 0.

*/

    void setMemory(size_t maxMemory, Supplier s);
/*
Sets the usable main memory for the operator in bytes. If ~maxMemory~
has value ~UINT\_MAX~ the usable main memory is requested from the
query processor which reads it from the operator's node.

*/

    static const size_t MIN_USER_DEF_MEMORY = 1024;
/*
Minimum amount of user defined memory for the operator.

*/

    static const size_t MAX_USER_DEF_MEMORY = ( 64 * 1024 * 1024 );
/*
Maximum amount of user defined memory for the operator.

*/

    // buffer limits
    size_t MAX_MEMORY;
    size_t MAX_TUPLES_IN_MEMORY;

    // buffer related members
    TupleBuffer2 *grpB;
    TupleBuffer2Iterator *iter;

    // members needed for sorting the input streams
    SortProgressLocalInfo* liA;
    SortAlgorithm* sliA;

    SortProgressLocalInfo* liB;
    SortAlgorithm* sliB;

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

    // switch trace messages on/off
    const bool traceMode;

    // a flag needed in function NextTuple which tells
    // if the merge with grpB has been finished
    bool continueMerge;

};

} // end of namespace extrel2

#endif /* SORTMERGEJOIN_H_ */
