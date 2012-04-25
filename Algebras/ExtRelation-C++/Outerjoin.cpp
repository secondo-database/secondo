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



1.1 Implementation of Outerjoin for Module Extended Relation Algebra

1.1 Using Storage Manager Berkeley DB

January 2008, B. Poneleit 


1.1.1 Includes and defines

*/


#include <vector>
#include <list>
#include <set>
#include <queue>

//#define TRACE_ON
#undef TRACE_ON
#include "LogMsg.h"
#define TRACE_OFF

#include "SortByLocalInfo.h"
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
#include "ListUtils.h"
#include "HashAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;            

/*
2.2.1 Operator ~smouterjoin~

This operator computes the equijoin of two streams. It uses a text book
algorithm as outlined in A. Silberschatz, H. F. Korth, S. Sudarshan,
McGraw-Hill, 3rd. Edition, 1997.

2.2.1.1 Auxiliary definitions for value mapping function of operator ~smouterjoin~

*/

#ifndef USE_PROGRESS
/*
2.2.2.1 Value mapping function of operator ~mergeouterjoin~

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
    progress->readFirst++;
    if (undefA == 0) {
        // create tuple with undefined values        
        result = new Tuple( tupleTypeA );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
          int algId = tupleTypeA->GetAttributeType(i).algId;
          int typeId = tupleTypeA->GetAttributeType(i).typeId;            

          // create an instance of the specified type, which gives
          // us an instance of a subclass of class Attribute.
          Attribute* attr =
            static_cast<Attribute*>( 
              am->CreateObj(algId, typeId)(0).addr );        
          attr->SetDefined( false );
          result->PutAttribute( i, attr );
        }
        undefA = result;
    }
    else {
    result = undefA;
    }
    return result;
}

inline Tuple* NextUndefinedB()
{
    Tuple* result = 0;

    progress->readSecond++;
    if (undefB == 0)  {
        // create tuple with undefined values
        result = new Tuple( tupleTypeB );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
          int algId = tupleTypeB->GetAttributeType(i).algId;
          int typeId = tupleTypeB->GetAttributeType(i).typeId;            

          // create an instance of the specified type, which gives
          // us an instance of a subclass of class Attribute.
          Attribute* attr =
            static_cast<Attribute*>( 
              am->CreateObj(algId, typeId)(0).addr );        
          attr->SetDefined( false );
          result->PutAttribute( i, attr );
        }
        undefB = result;        
    }
    else {
    result = undefB;
    }
    return result;
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

    MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
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

                          if ( cmp == 0)   {
                              // append equal tuples to group  
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else    {
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
    progress->readFirst++;
    if (undefA == 0) {
        // create tuple with undefined values        
        result = new Tuple( tupleTypeA );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
          int algId = tupleTypeA->GetAttributeType(i).algId;
          int typeId = tupleTypeA->GetAttributeType(i).typeId;            

          // create an instance of the specified type, which gives
          // us an instance of a subclass of class Attribute.
          Attribute* attr =
            static_cast<Attribute*>( 
              am->CreateObj(algId, typeId)(0).addr );        
          attr->SetDefined( false );
          result->PutAttribute( i, attr );
        }
        undefA = result;
    }
    else {
    result = undefA;
    }
    return result;
}

inline Tuple* NextUndefinedB()
{
    Tuple* result = 0;

    progress->readSecond++;
    if (undefB == 0)  {
        // create tuple with undefined values
        result = new Tuple( tupleTypeB );
        for (int i = 0; i < result->GetNoAttributes(); i++)
        {
          int algId = tupleTypeB->GetAttributeType(i).algId;
          int typeId = tupleTypeB->GetAttributeType(i).typeId;            

          // create an instance of the specified type, which gives
          // us an instance of a subclass of class Attribute.
          Attribute* attr =
            static_cast<Attribute*>( 
              am->CreateObj(algId, typeId)(0).addr );        
          attr->SetDefined( false );
          result->PutAttribute( i, attr );
        }
        undefB = result;        
    }
    else {
    result = undefB;
    }
    return result;
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
                  tupleCmpA, liA, s);

      liB = new LocalInfo<SortByLocalInfo>();
      progress->secondLocalInfo = liB;
      sliB = new SortByLocalInfo( streamB,
                  false,
                  tupleCmpB, liB, s );

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

    MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
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
    tupleTypeA->DeleteIfAllowed();
    tupleTypeB->DeleteIfAllowed();
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

                          if ( cmp == 0)   {
                              // append equal tuples to group  
                              grpB->AppendTuple(ptB.tuple);

                              // release tuple of input B
                              ptB.setTuple( NextTupleB() );
                          }
                          else    {
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

      if ( li->ptr == 0 )  //first request;
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

          if (li->returned > enoughSuccessesJoin )   // stable state 
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

      pRes->CopyBlocking(p1, p2);     //non-blocking in this case
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


#ifndef USE_PROGRESS
/*
2.2.2.1 Value mapping function of operator ~symmouterjoin~

*/
// standard version


struct SymmOuterJoinLocalInfo
{
 SymmOuterJoinLocalInfo(Word _streamRight, Word _streamLeft) 
  {
      streamRight = _streamRight;
      streamLeft  = _streamLeft;
      
      ListExpr typeRight =
        SecondoSystem::GetCatalog()->NumericType(
          qp->GetType( streamRight.addr ) );
      tupleTypeRight = new TupleType( nl->Second( typeRight ) );
      ListExpr typeLeft =
        SecondoSystem::GetCatalog()->NumericType(
          qp->GetType( streamLeft.addr ) );
      tupleTypeLeft = new TupleType( nl->Second( typeLeft ) );

      undefRight = 0;
      undefLeft = 0;
  }
  
  TupleType *resultTupleType;

  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
  Hash *rightHash;
  Hash *leftHash;  
  
  Word streamRight;
  Word streamLeft;
  
  TupleType *tupleTypeRight;
  TupleType *tupleTypeLeft;
  
  bool nullTuples;
  
  TupleBuffer *rightRel2;
  TupleBuffer *leftRel2;  
  
  Tuple *undefRight;
  Tuple *undefLeft;

  
  inline Tuple* NextUndefinedRight()
  {
      Tuple* result = 0;
      readFirst++;
      if (undefRight == 0) {
          // create tuple with undefined values        
          result = new Tuple( tupleTypeRight );
          for (int i = 0; i < result->GetNoAttributes(); i++)
          {
            int algId = tupleTypeRight->GetAttributeType(i).algId;
            int typeId = tupleTypeRight->GetAttributeType(i).typeId;            

            // create an instance of the specified type, which gives
            // us an instance of a subclass of class Attribute.
            Attribute* attr =
              static_cast<Attribute*>( 
                am->CreateObj(algId, typeId)(0).addr );        
            attr->SetDefined( false );
            result->PutAttribute( i, attr );
          }
          undefRight = result;
      }
      else {
      result = undefRight;
      }
      return result;
  }

  inline Tuple* NextUndefinedLeft()
  {
      Tuple* result = 0;

      readSecond++;
      if (undefLeft == 0)  {
          // create tuple with undefined values
          result = new Tuple( tupleTypeLeft );
          for (int i = 0; i < result->GetNoAttributes(); i++)
          {
            int algId = tupleTypeLeft->GetAttributeType(i).algId;
            int typeId = tupleTypeLeft->GetAttributeType(i).typeId;            

            // create an instance of the specified type, which gives
            // us an instance of a subclass of class Attribute.
            Attribute* attr =
              static_cast<Attribute*>( 
                am->CreateObj(algId, typeId)(0).addr );        
            attr->SetDefined( false );
            result->PutAttribute( i, attr );
          }
          undefLeft = result;        
      }
      else {
      result = undefLeft;
      }
      return result;
  }
};

template<int dummy>
int
symmouterjoin_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, l;
  SymmOuterJoinLocalInfo* pli;

  switch (message)
  {
    case OPEN :
    {
      long MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
      cmsg.info("ERA:ShowMemInfo") << "SymmOuterJoin.MAX_MEMORY ("
                                   << MAX_MEMORY/1024 << " kB): " << endl;
      cmsg.send();
      pli = new SymmOuterJoinLocalInfo(args[0],args[1]);
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;
      pli->rightHash = new Hash( INT, true );
      pli->leftHash = new Hash( INT, true );
      
      pli->nullTuples = false;
      pli->rightRel2 = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftRel2 = new TupleBuffer( MAX_MEMORY / 2 );  

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      local.setAddr(pli);
      return 0;
    }
    case REQUEST :
    {
      pli = (SymmOuterJoinLocalInfo*)local.addr;

      while( 1 )
        // This loop will end in some of the returns.
      {
          if ( pli->nullTuples )
          {  
          // find all unmatched tuples from right relation
          if ( pli->rightIter == 0 ) 
          {          
            pli->rightIter = pli->rightRel2->MakeScan();
          }
          
          Tuple *rightOuterTuple = pli->rightIter->GetNextTuple();
          
          if ( rightOuterTuple != 0 )
          {
            SmiKeyedFile *file = pli->rightHash->GetFile();
            SmiRecord record;            
            
            while ( rightOuterTuple != 0 && 
              file->SelectRecord( 
                SmiKey((long)rightOuterTuple->GetTupleId()), record ))
            {
              // if we find the tupleid in the hash file, 
              //the tuple is already matched,
              // so we can ignore it.
              // curiosly, record size is 0 when we find the tuple 
              //id in the hashfile
              if ( record.Size() == 0 ) {                
                rightOuterTuple->DeleteIfAllowed();
                rightOuterTuple = 0;                  
                rightOuterTuple = pli->rightIter->GetNextTuple();
              }
              else
                break;
            }
            
            // create a tuple with undefined values for the 
            // attributes of the left relation
            if ( rightOuterTuple != 0 )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( rightOuterTuple, pli->NextUndefinedLeft(), resultTuple );
              rightOuterTuple->DeleteIfAllowed();
              rightOuterTuple = 0;  
              result.setAddr( resultTuple );         
              return YIELD;
            }                  
          }              
          
          if ( pli->leftIter == 0 )
          {
            pli->leftIter = pli->leftRel2->MakeScan();
          }            
          
          Tuple *leftOuterTuple = pli->leftIter->GetNextTuple();
          
          if ( leftOuterTuple != 0 )
          {
            SmiKeyedFile *file = pli->leftHash->GetFile();
            SmiRecord record;
            
            while ( leftOuterTuple != 0 && 
              file->SelectRecord( 
                SmiKey((long)leftOuterTuple->GetTupleId()), record ))
            {
              // if we find the tupleid in the hash file, 
              // the tuple is already matched,
              // so we can ignore it.
              // curiosly, record size is 0 when we find 
              // the tuple id in the hashfile            
              if ( record.Size() == 0 ) {                
                leftOuterTuple->DeleteIfAllowed();
                leftOuterTuple = 0;                  
                leftOuterTuple = pli->leftIter->GetNextTuple(); 
              }   
              else
                break;
            }
            
            // create a tuple with undefined values for the 
            //attributes of the right relation
            if ( leftOuterTuple != 0 )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( pli->NextUndefinedRight(), leftOuterTuple, resultTuple );
              leftOuterTuple->DeleteIfAllowed();
              leftOuterTuple = 0;  
              result.setAddr( resultTuple );                     
              return YIELD;
            }      
            
          }  
          // we're finished
          return CANCEL;
        }      
        if( pli->right )
          // Get the tuple from the right stream and match it with the
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, r);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
              pli->rightRel2->AppendTuple( pli->currTuple );
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished )
              {
                pli->nullTuples = true; // output null-tuples
                continue; 
              }
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then
            // store the current tuple in the right buffer and close the
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            ((*funArgs)[0]).setAddr( pli->currTuple );
            ((*funArgs)[1]).setAddr( leftTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( pli->currTuple, leftTuple, resultTuple );
              pli->leftHash->Append( SmiKey((long)leftTuple->GetTupleId()), 
                (SmiRecordId)leftTuple->GetTupleId());
              pli->rightHash->Append( 
                SmiKey((long)pli->currTuple->GetTupleId()), 
                (SmiRecordId)pli->currTuple->GetTupleId() );              
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              result.setAddr( resultTuple );
              return YIELD;
            }
            else
            {                            
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
        else
          // Get the tuple from the left stream and match it with the
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, l);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
              pli->leftRel2->AppendTuple( pli->currTuple );
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished )
              {
                pli->nullTuples = true;
                continue;
              }
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then
            // store the current tuple in the left buffer and close
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            ((*funArgs)[0]).setAddr( rightTuple );
            ((*funArgs)[1]).setAddr( pli->currTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( rightTuple, pli->currTuple, resultTuple );
              pli->rightHash->Append( SmiKey((long)rightTuple->GetTupleId()), 
                (SmiRecordId)rightTuple->GetTupleId() );
              pli->leftHash->Append( 
                SmiKey((long)pli->currTuple->GetTupleId()), 
                (SmiRecordId)pli->currTuple->GetTupleId() );              
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              result.setAddr( resultTuple );
              return YIELD;
            }
            else
            {
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
      }
    }
    case CLOSE :
    {
      pli = (SymmOuterJoinLocalInfo*)local.addr;
      if(pli)
      {
         if( pli->currTuple != 0 )
           pli->currTuple->DeleteIfAllowed();

         delete pli->leftIter;
         delete pli->rightIter;
         if( pli->resultTupleType != 0 )
           pli->resultTupleType->DeleteIfAllowed();

         if( pli->rightRel != 0 )
         {
           pli->rightRel->Clear();
           delete pli->rightRel;
         }

         if( pli->leftRel != 0 )
         {
           pli->leftRel->Clear();
           delete pli->leftRel;
         }
         
        if ( pli->rightHash != 0 )
        { 
          pli->rightHash->DeleteFile();
          delete pli->rightHash;
          pli->rightHash = 0;
        }
        
        if ( pli->leftHash != 0 )
        {           
          pli->leftHash->DeleteFile();
          delete pli->leftHash;
          pli->leftHash = 0;
        }  

        if( pli->rightRel2 != 0 )
        {
          pli->rightRel2->Clear();
          delete pli->rightRel2;
          pli->rightRel2=0;
        }

        if( pli->leftRel2 != 0 )
        {
          pli->leftRel2->Clear();
          delete pli->leftRel2;
          pli->leftRel2=0;
        }             

         delete pli;
         local.setAddr(0);
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }
  }
  return 0;
}


#else

// with support for progress queries

class SymmOuterJoinLocalInfo: public ProgressLocalInfo
{
public:
  SymmOuterJoinLocalInfo(Word _streamRight, Word _streamLeft) 
  {
      streamRight = _streamRight;
      streamLeft  = _streamLeft;
      
      ListExpr typeRight =
        SecondoSystem::GetCatalog()->NumericType(
          qp->GetType( streamRight.addr ) );
      tupleTypeRight = new TupleType( nl->Second( typeRight ) );
      ListExpr typeLeft =
        SecondoSystem::GetCatalog()->NumericType(
          qp->GetType( streamLeft.addr ) );
      tupleTypeLeft = new TupleType( nl->Second( typeLeft ) );

      undefRight = 0;
      undefLeft = 0;
  }


  ~SymmOuterJoinLocalInfo(){
     if(undefRight){
        undefRight->DeleteIfAllowed();
        undefRight=0;
     }
     if(undefLeft){
       undefLeft->DeleteIfAllowed();
       undefLeft=0;
     }
     tupleTypeRight->DeleteIfAllowed();
     tupleTypeLeft->DeleteIfAllowed();
  }
  
  Word streamRight;
  Word streamLeft;
  
  TupleType *resultTupleType;
  TupleType *tupleTypeRight;
  TupleType *tupleTypeLeft;
  
  TupleBuffer *rightRel;
  GenericRelationIterator *rightIter;
  TupleBuffer *leftRel;
  GenericRelationIterator *leftIter;
  bool right;
  Tuple *currTuple;
  bool rightFinished;
  bool leftFinished;
  
  Hash *rightHash;
  Hash *leftHash;
  bool nullTuples;
  
  TupleBuffer *rightRel2;
  TupleBuffer *leftRel2;  
  
  Tuple *undefRight;
  Tuple *undefLeft;

  
  inline Tuple* NextUndefinedRight()
  {
      Tuple* result = 0;
      readFirst++;
      if (undefRight == 0) {
          // create tuple with undefined values        
          result = new Tuple( tupleTypeRight );
          for (int i = 0; i < result->GetNoAttributes(); i++)
          {
            int algId = tupleTypeRight->GetAttributeType(i).algId;
            int typeId = tupleTypeRight->GetAttributeType(i).typeId;            

            // create an instance of the specified type, which gives
            // us an instance of a subclass of class Attribute.
            Attribute* attr =
              static_cast<Attribute*>( 
                am->CreateObj(algId, typeId)(0).addr );        
            attr->SetDefined( false );
            result->PutAttribute( i, attr );
          }
          undefRight = result;
      }
      else {
      result = undefRight;
      }
      return result;
  }

  inline Tuple* NextUndefinedLeft()
  {
      Tuple* result = 0;

      readSecond++;
      if (undefLeft == 0)  {
          // create tuple with undefined values
          result = new Tuple( tupleTypeLeft );
          for (int i = 0; i < result->GetNoAttributes(); i++)
          {
            int algId = tupleTypeLeft->GetAttributeType(i).algId;
            int typeId = tupleTypeLeft->GetAttributeType(i).typeId;            

            // create an instance of the specified type, which gives
            // us an instance of a subclass of class Attribute.
            Attribute* attr =
              static_cast<Attribute*>( 
                am->CreateObj(algId, typeId)(0).addr );        
            attr->SetDefined( false );
            result->PutAttribute( i, attr );
          }
          undefLeft = result;        
      }
      else {
      result = undefLeft;
      }
      return result;
  }
};

template<int dummy>
int
symmouterjoin_vm(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word r, l;
  SymmOuterJoinLocalInfo* pli;

  pli = (SymmOuterJoinLocalInfo*) local.addr;

  switch (message)
  {
    case OPEN :
    {

      long MAX_MEMORY = (qp->GetMemorySize(s) * 1024 * 1024);
      cmsg.info("ERA:ShowMemInfo") << "SymmOuterJoin.MAX_MEMORY ("
                                   << MAX_MEMORY/1024 << " kB): " << endl;
      cmsg.send();


      if ( pli ) delete pli;

      pli = new SymmOuterJoinLocalInfo(args[0], args[1]);
      pli->rightRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->rightIter = 0;
      pli->leftRel = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftIter = 0;
      pli->right = true;
      pli->currTuple = 0;
      pli->rightFinished = false;
      pli->leftFinished = false;
      pli->rightHash = new Hash( SmiKey::Integer, true );
      pli->leftHash = new Hash( SmiKey::Integer, true );
      pli->nullTuples = false;
      pli->rightRel2 = new TupleBuffer( MAX_MEMORY / 2 );
      pli->leftRel2 = new TupleBuffer( MAX_MEMORY / 2 );

      ListExpr resultType = GetTupleResultType( s );
      pli->resultTupleType = new TupleType( nl->Second( resultType ) );

      qp->Open(args[0].addr);
      qp->Open(args[1].addr);

      pli->readFirst = 0;
      pli->readSecond = 0;
      pli->returned = 0;

      local.setAddr(pli);
      return 0;
    }

    case REQUEST :
    {
      while( 1 )
        // This loop will end in some of the returns.
      {
          if ( pli->nullTuples )
          {  
          // find all unmatched tuples from right relation
          if ( pli->rightIter == 0 ) 
          {          
            pli->rightIter = pli->rightRel2->MakeScan();
          }
          
          Tuple *rightOuterTuple = pli->rightIter->GetNextTuple();
          
          if ( rightOuterTuple != 0 )
          {
            SmiKeyedFile *file = pli->rightHash->GetFile();
            SmiRecord record;            
            
            while ( rightOuterTuple != 0 && 
              file->SelectRecord( 
                SmiKey((long)rightOuterTuple->GetTupleId()), record ))
            {
              // if we find the tupleid in the hash file, 
              //the tuple is already matched,
              // so we can ignore it.                
                rightOuterTuple->DeleteIfAllowed();
                rightOuterTuple = 0;                  
                rightOuterTuple = pli->rightIter->GetNextTuple();
            }
            
            // create a tuple with undefined values for the 
            // attributes of the left relation
            if ( rightOuterTuple != 0 )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Tuple* nextUndefinedLeft = pli->NextUndefinedLeft();
              Concat( rightOuterTuple, nextUndefinedLeft, resultTuple );
              rightOuterTuple->DeleteIfAllowed();
              rightOuterTuple = 0;  
              result.setAddr( resultTuple );
              pli->returned++;            
              return YIELD;
            }                  
          }              
          
          if ( pli->leftIter == 0 )
          {
            pli->leftIter = pli->leftRel2->MakeScan();
          }            
          
          Tuple *leftOuterTuple = pli->leftIter->GetNextTuple();
          
          if ( leftOuterTuple != 0 )
          {
            SmiKeyedFile *file = pli->leftHash->GetFile();
            SmiRecord record;
            
            while ( leftOuterTuple != 0 && 
              file->SelectRecord( 
                SmiKey((long)leftOuterTuple->GetTupleId()), record ))
            {
              // if we find the tupleid in the hash file, 
              // the tuple is already matched,
              // so we can ignore it.       
                leftOuterTuple->DeleteIfAllowed();
                leftOuterTuple = 0;                  
                leftOuterTuple = pli->leftIter->GetNextTuple(); 
            }
            
            // create a tuple with undefined values for the 
            //attributes of the right relation
            if ( leftOuterTuple != 0 )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Tuple* nextUndefinedRight = pli->NextUndefinedRight();
              Concat( nextUndefinedRight , leftOuterTuple, resultTuple );
              leftOuterTuple->DeleteIfAllowed();
              leftOuterTuple = 0;  
              result.setAddr( resultTuple );
              pli->returned++;            
              return YIELD;
            }      
            
          }  
          // we're finished
          return CANCEL;
        }
        if( pli->right )
          // Get the tuple from the right stream and match it with the
          // left stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[0].addr, r);
            if( qp->Received( args[0].addr ) )
            {
              pli->currTuple = (Tuple*)r.addr;
              pli->leftIter = pli->leftRel->MakeScan();
              pli->readFirst++;              
              pli->rightRel2->AppendTuple( pli->currTuple );
            }
            else
            {
              pli->rightFinished = true;
              if( pli->leftFinished ) 
              {
                pli->nullTuples = true;
                continue;                
              }
              else
              {
                pli->right = false;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the right stream in currTuple
          // and an open iterator on the left stored buffer.
          Tuple *leftTuple = pli->leftIter->GetNextTuple();

          if( leftTuple == 0 )
            // There are no more tuples in the left iterator. We then
            // store the current tuple in the right buffer and close the
            // left iterator.
          {
            if( !pli->leftFinished )
              // We only need to keep track of the right tuples if the
              // left stream is not finished.
            {
              pli->rightRel->AppendTuple( pli->currTuple );
              pli->right = false;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->leftIter;
            pli->leftIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            ((*funArgs)[0]).setAddr( pli->currTuple );
            ((*funArgs)[1]).setAddr( leftTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( pli->currTuple, leftTuple, resultTuple );
              pli->leftHash->Append( SmiKey((long)leftTuple->GetTupleId()), 
                (SmiRecordId)leftTuple->GetTupleId());
              pli->rightHash->Append( 
                SmiKey((long)pli->currTuple->GetTupleId()), 
                (SmiRecordId)pli->currTuple->GetTupleId() );
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              result.setAddr( resultTuple );
              pli->returned++;            
              return YIELD;
            }
            else
            {        
              leftTuple->DeleteIfAllowed();
              leftTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
        else
          // Get the tuple from the left stream and match it with the
          // right stored buffer
        {
          if( pli->currTuple == 0 )
          {
            qp->Request(args[1].addr, l);
            if( qp->Received( args[1].addr ) )
            {
              pli->currTuple = (Tuple*)l.addr;
              pli->rightIter = pli->rightRel->MakeScan();
              pli->readSecond++;
              pli->leftRel2->AppendTuple( pli->currTuple );
            }
            else
            {
              pli->leftFinished = true;
              if( pli->rightFinished ) 
              { 
                pli->nullTuples = true;
                continue;
              }
              else
              {
                pli->right = true;
                continue; // Go back to the loop
              }
            }
          }

          // Now we have a tuple from the left stream in currTuple and
          // an open iterator on the right stored buffer.
          Tuple *rightTuple = pli->rightIter->GetNextTuple();

          if( rightTuple == 0 )
            // There are no more tuples in the right iterator. We then
            // store the current tuple in the left buffer and close
            // the right iterator.
          {
            if( !pli->rightFinished )
              // We only need to keep track of the left tuples if the
              // right stream is not finished.
            {
              pli->leftRel->AppendTuple( pli->currTuple );
              pli->right = true;
            }

            pli->currTuple->DeleteIfAllowed();
            pli->currTuple = 0;

            delete pli->rightIter;
            pli->rightIter = 0;

            continue; // Go back to the loop
          }
          else
            // We match the tuples.
          {
            ArgVectorPointer funArgs = qp->Argument(args[2].addr);
            ((*funArgs)[0]).setAddr( rightTuple );
            ((*funArgs)[1]).setAddr( pli->currTuple );
            Word funResult;
            qp->Request(args[2].addr, funResult);
            CcBool *boolFunResult = (CcBool*)funResult.addr;

            if( boolFunResult->IsDefined() &&
                boolFunResult->GetBoolval() )
            {
              Tuple *resultTuple = new Tuple( pli->resultTupleType );
              Concat( rightTuple, pli->currTuple, resultTuple );
              pli->rightHash->Append( SmiKey((long)rightTuple->GetTupleId()), 
                (SmiRecordId)rightTuple->GetTupleId() );
              pli->leftHash->Append( 
                SmiKey((long)pli->currTuple->GetTupleId()), 
                (SmiRecordId)pli->currTuple->GetTupleId() );
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              result.setAddr( resultTuple );
              pli->returned++;                
              return YIELD;
            }
            else
            {        
              rightTuple->DeleteIfAllowed();
              rightTuple = 0;
              continue; // Go back to the loop
            }
          }
        }
      }
    }
    case CLOSE :
    {
      if(pli)
      {
        if( pli->currTuple != 0 ){
          pli->currTuple->DeleteIfAllowed();
          pli->currTuple=0;
        }

        delete pli->leftIter;
        delete pli->rightIter;
        if( pli->resultTupleType != 0 ){
          pli->resultTupleType->DeleteIfAllowed();
          pli->resultTupleType=0;
        }

        if( pli->rightRel != 0 )
        {
          pli->rightRel->Clear();
          delete pli->rightRel;
          pli->rightRel=0;
        }

        if( pli->leftRel != 0 )
        {
          pli->leftRel->Clear();
          delete pli->leftRel;
          pli->leftRel=0;
        }
        
        if ( pli->rightHash != 0 )
        { 
          pli->rightHash->DeleteFile();
          delete pli->rightHash;
          pli->rightHash = 0;
        }
        
        if ( pli->leftHash != 0 )
        {           
          pli->leftHash->DeleteFile();
          delete pli->leftHash;
          pli->leftHash = 0;
        }  

        if( pli->rightRel2 != 0 )
        {
          pli->rightRel2->Clear();
          delete pli->rightRel2;
          pli->rightRel2=0;
        }

        if( pli->leftRel2 != 0 )
        {
          pli->leftRel2->Clear();
          delete pli->leftRel2;
          pli->leftRel2=0;
        }        
      }

      qp->Close(args[0].addr);
      qp->Close(args[1].addr);

      return 0;
    }


    case CLOSEPROGRESS:
      if ( pli )
      {
         delete pli;
         local.setAddr(0);
      }
      return 0;


    case REQUESTPROGRESS :
    {
      ProgressInfo p1, p2;
      ProgressInfo *pRes;
      const double uSymmOuterJoin = 0.2;  //millisecs per tuple pair


      pRes = (ProgressInfo*) result.addr;

      if (!pli) return CANCEL;

      if (qp->RequestProgress(args[0].addr, &p1)
       && qp->RequestProgress(args[1].addr, &p2))
      {
        pli->SetJoinSizes(p1, p2);

        pRes->CopySizes(pli);

        double predCost =
          (qp->GetPredCost(s) == 0.1 ? 0.004 : qp->GetPredCost(s));

        //the default value of 0.1 is only suitable for selections

        pRes->Time = p1.Time + p2.Time +
          p1.Card * p2.Card * predCost * uSymmOuterJoin;

        pRes->Progress =
          (p1.Progress * p1.Time + p2.Progress * p2.Time +
          pli->readFirst * pli->readSecond *
          predCost * uSymmOuterJoin)
          / pRes->Time;

        if (pli->returned > enoughSuccessesJoin )   // stable state assumed now
        {
          pRes->Card = p1.Card * p2.Card *
            ((double) pli->returned /
              (double) (pli->readFirst * pli->readSecond));
        }
        else
        {
          pRes->Card = p1.Card * p2.Card * qp->GetSelectivity(s);
        }

        pRes->CopyBlocking(p1, p2);  //non-blocking oprator

        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
  }
  return 0;
}

#endif


template int
smouterjoin_vm<false>(Word* args, Word& result, int message, 
                 Word& local, Supplier s);
                 
template int
symmouterjoin_vm<1>(Word* args, Word& result, int message, 
                 Word& local, Supplier s);                 
