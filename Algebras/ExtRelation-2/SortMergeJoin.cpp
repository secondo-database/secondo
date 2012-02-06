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

1 Implementation File SortMergeJoin.cpp

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#include <algorithm>
#include "stdlib.h"

#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTuple.h"
#include "Sort.h"
#include "SortMergeJoin.h"

/*
3 External linking

*/
extern QueryProcessor* qp;

/*
4 Implementation of class ~SortMergeJoinLocalInfo~

*/
namespace extrel2
{
SortMergeJoinLocalInfo::SortMergeJoinLocalInfo( Word streamA,
                                                int attrIndexA,
                                                Word streamB,
                                                int attrIndexB,
                                                Supplier s,
                                                ProgressLocalInfo* p,
                                                size_t maxMemSize )
: ProgressWrapper(p)
, streamA(streamA)
, streamB(streamB)
, tmpB(0)
, cmp(0)
, attrIndexA(attrIndexA)
, attrIndexB(attrIndexB)
, traceMode(RTFlag::isActive("ERA:TraceMergeJoin"))
, continueMerge(false)
{
  grpB = 0;
  iter = 0;
  liA = 0;
  sliA = 0;
  liB = 0;
  sliB = 0;
  // sort the input streams
  SortOrderSpecification specA;
  SortOrderSpecification specB;

  specA.push_back( pair<int, bool>(attrIndexA, true) );
  specB.push_back( pair<int, bool>(attrIndexB, true) );
  // set available main memory (MAX_MEMORY)
  setMemory(maxMemSize, s);

  liA = new SortProgressLocalInfo();
  progress->firstLocalInfo = liA;
  sliA = new SortAlgorithm(streamA, specA, liA, s, UINT_MAX, MAX_MEMORY / 2 );

  size_t remaining_memory = MAX_MEMORY - sliA->getUsedMemory();

  liB = new SortProgressLocalInfo();
  progress->secondLocalInfo = liB;
  sliB = new SortAlgorithm(streamB, specB, liB, s, UINT_MAX, remaining_memory);

  ListExpr resultType =
              SecondoSystem::GetCatalog()->NumericType( qp->GetType( s ) );
  resultTupleType = new TupleType( nl->Second( resultType ) );

  remaining_memory =  remaining_memory > sliB->getUsedMemory()
                           ?remaining_memory - sliB->getUsedMemory()
                           :0;
  // read in the first tuple of both input streams
  ptA.setTuple( NextTupleA() );
  ptB.setTuple( NextTupleB() );


  grpB = new TupleBuffer2( remaining_memory );

  if ( traceMode )
  {
    cmsg.info() << "-------------------- SortMerge-Join2 ------------------"
                << endl
                << "Memory: \t" << MAX_MEMORY / 1024 << " KByte" << endl;
    cmsg.send();
  }
}

SortMergeJoinLocalInfo::~SortMergeJoinLocalInfo()
{
  if ( sliA )
  {
    delete sliA;
    sliA = 0;
  }

  if ( sliB )
  {
    delete sliB;
    sliB = 0;
  }

  if ( liA )
  {
    delete liA;
    liA = 0;
  }

  if ( liB )
  {
    delete liB;
    liB = 0;
  }

  if ( grpB )
  {
    delete grpB;
    grpB = 0;
  }

  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  resultTupleType->DeleteIfAllowed();
}

void SortMergeJoinLocalInfo::setMemory(size_t maxMemory, Supplier s)
{
  if ( maxMemory == UINT_MAX )
  {
    MAX_MEMORY = qp->GetMemorySize(s) * 1024 * 1024; // in bytes
  }
  else if ( maxMemory < MIN_USER_DEF_MEMORY )
  {
    MAX_MEMORY = MIN_USER_DEF_MEMORY;
  }
  else if ( maxMemory > MAX_USER_DEF_MEMORY )
  {
    MAX_MEMORY = MAX_USER_DEF_MEMORY;
  }
  else
  {
    MAX_MEMORY = maxMemory;
  }
}

Tuple* SortMergeJoinLocalInfo::NextResultTuple()
{
  Tuple* resultTuple = 0;

  if ( !continueMerge && ptB == 0 )
  {
    return 0;
  }

  while( ptA != 0 )
  {
    if ( !continueMerge && ptB != 0 )
    {
      tmpB = ptB;

      grpB->AppendTuple(tmpB.tuple);

      // advance the tuple pointer
      ptB.setTuple( NextTupleB() );

      // collect a group of tuples from B which
      // have the same attribute value
      bool done = false;
      while ( !done && ptB != 0 )
      {
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
        ptA.setTuple( NextTupleA() );

        if (ptA != 0)
        {
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
        {
          return resultTuple;
        }
      }
      else
      {
        // continue merging, create the next result tuple
        resultTuple = NextConcat();
        if (resultTuple)
        {
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
          if (ptA != 0)
          {
            cmp = CompareTuples( ptA.tuple, tmpB.tuple );
          }
        }
      }
    }

    grpB->Clear();

    // tpA > tmpB
    if ( ptB == 0 )
    {
      // short exit
      return 0;
    }

  } // end of main loop

  return 0;
}


/*
5 Value mapping function of operator ~sortmergejoin2~

*/

template<bool param>
int SortMergeJoinValueMap( Word* args, Word& result,
                             int message, Word& local, Supplier s )
{
  // if ( param = false )
  // args[0] : stream A
  // args[1] : stream B
  // args[2] : attribute name of join attribute for stream A
  // args[3] : attribute name join attribute for stream B
  // args[4] : attribute index of join attribute for stream A
  // args[5] : attribute index of join attribute for stream B

  // if ( param = true )
  // args[0] : stream A
  // args[1] : stream B
  // args[2] : attribute name of join attribute for stream A
  // args[3] : attribute name join attribute for stream B
  // args[4] : usable main memory in bytes (only if param is true)
  // args[5] : attribute index of join attribute for stream A
  // args[6] : attribute index of join attribute for stream B

  typedef LocalInfo<SortMergeJoinLocalInfo> LocalType;
  LocalType* li = static_cast<LocalType*>( local.addr );

  switch(message)
  {
    case OPEN:
    {

      if ( li )
      {
        delete li;
      }
      li = new LocalType();
      local.addr = li;

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      li->ptr = 0;

      return 0;
    }

    case REQUEST:
    {

      if ( li->ptr == 0 ) //first request;
        //constructor put here to avoid delays in OPEN
        //which are a problem for progress estimation
      {
        if ( param )
        {
          size_t maxMemSize = (size_t)StdTypes::GetInt( args[4] );
          int attrIndexA = StdTypes::GetInt( args[5] );
          int attrIndexB = StdTypes::GetInt( args[6] );

          li->ptr = new SortMergeJoinLocalInfo( args[0], attrIndexA,
                                                args[1], attrIndexB,
                                                s, li, maxMemSize );
        }
        else
        {
          int attrIndexA = StdTypes::GetInt( args[4] );
          int attrIndexB = StdTypes::GetInt( args[5] );

          li->ptr = new SortMergeJoinLocalInfo( args[0], attrIndexA,
                                                args[1], attrIndexB,
                                                s, li );
        }
      }

      SortMergeJoinLocalInfo* mli = li->ptr;
      result.addr = mli->NextResultTuple();
      li->returned++;

      return result.addr != 0 ? YIELD : CANCEL;
    }

    case CLOSE:
    {
      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      //nothing is deleted on close because the substructures are still
      //needed for progress estimation. Instead, everything is deleted on
      //(repeated) OPEN and on CLOSEPROGRESS

      return 0;
    }

    case CLOSEPROGRESS:
    {
      if (li)
      {
        delete li;
        local.addr = 0;
      }

      return 0;
    }

    case REQUESTPROGRESS:
    {
      ProgressInfo p1, p2;
      ProgressInfo* pRes = static_cast<ProgressInfo*>( result.addr );

      const double uSortBy = 0.00043;   //millisecs per byte read in sort step

      //const double uMergeJoin = 0.0008077;  //millisecs per tuple read
                                        //in merge step (merge)

      const double wMergeJoin = 0.0001738; //millisecs per byte read in
                                          //merge step (sortmerge)

      const double xMergeJoin = 0.0012058; //millisecs per result tuple in
                                          //merge step

      const double yMergeJoin = 0.0001072; //millisecs per result attribute in
                                          //merge step

                                      //see file ConstantsSortmergejoin.txt


      SortProgressLocalInfo* liFirst;
      SortProgressLocalInfo* liSecond;

      if( !li )
      {
        return CANCEL;
      }
      else
      {

        liFirst = static_cast<SortProgressLocalInfo*>
                    (li->firstLocalInfo);
        liSecond = static_cast<SortProgressLocalInfo*>
                    (li->secondLocalInfo);

        if (qp->RequestProgress(args[0].addr, &p1)
         && qp->RequestProgress(args[1].addr, &p2))
        {
          li->SetJoinSizes(p1, p2);

          pRes->CopySizes(li);

          double factor = (double) li->readFirst / p1.Card;

          // Calculate result cardinality
          if ( li->returned > enoughSuccessesJoin )
          {
            double m = (double)li->returned;
            double k1 = (double)li->readFirst;
            double k2 = (double)li->readSecond;

            // estimated selectivity
            double sel = m / ( k1 * k2 );

            // warm state
            if ( qp->GetSelectivity(s) != 0.1 )
            {
              // estimated selectivity from optimizer is used
              // as more tuples are processed the weight of the
              // optimizer estimation is reduced
              pRes->Card = ( p1.Card * p2.Card ) *
                           ( factor * sel +
                             ( 1.0 - factor ) * qp->GetSelectivity(s) );
            }
            else
            {
              // if optimizer is not used use only estimation
              pRes->Card = sel * p1.Card * p2.Card;
            }
          }
          else
          {
            // cold state
            pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
          }

          // total time
          pRes->Time = p1.Time + p2.Time +
                     + p1.Card * p1.Size * uSortBy +
                     + p2.Card * p2.Size * uSortBy +
                     + (p1.Card * p1.Size + p2.Card * p2.Size) * wMergeJoin
                     + pRes->Card * (xMergeJoin + pRes->noAttrs * yMergeJoin);

          long readFirst = (liFirst ? liFirst->read : 0);
          long readSecond = (liSecond ? liSecond->read : 0);

          pRes->Progress = (   p1.Progress * p1.Time
                             + p2.Progress * p2.Time
                             + ((double) readFirst) * p1.Size * uSortBy
                             + ((double) readSecond) * p2.Size * uSortBy
                             + (((double) li->readFirst) * p1.Size
                             + ((double) li->readSecond) * p2.Size)
                               * wMergeJoin
                             + ((double) li->returned)
                               * (xMergeJoin + pRes->noAttrs * yMergeJoin) )
                           / pRes->Time;

          // first result tuple is possible after both input streams
          // deliver the first tuples and both sort algorithm have
          // consumed their streams completely
          pRes->BTime =   p1.BTime
                        + p2.BTime
                        + p1.Card * p1.Size * uSortBy
                        + p2.Card * p2.Size * uSortBy;

          // blocking progress
          pRes->BProgress = (   p1.BProgress * p1.BTime
                              + p2.BProgress * p2.BTime
                              + ((double) readFirst) * p1.Size * uSortBy
                              + ((double) readSecond) * p2.Size * uSortBy )
                            / pRes->BTime;

          return YIELD;
        }
        else
        {
          return CANCEL;
        }
      }
    }
  }

  return 0;
}

/*
6 Instantiation of Template Functions

For some reasons the compiler cannot expand these template functions in
the file ~ExtRelation2Algebra.cpp~, thus the value mapping functions
are instantiated here.

*/

template
int SortMergeJoinValueMap<false>( Word* args, Word& result,
                                   int message, Word& local, Supplier s );

template
int SortMergeJoinValueMap<true>( Word* args, Word& result,
                                  int message, Word& local, Supplier s);

} // end of namespace extrel2
