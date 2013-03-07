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

//paragraph [1] title: [{\Large \bf ]   [}]
//[->] [$\rightarrow$]



[1] Stream Example Algebra

July 2002 RHG

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hybrid~). Only the executable
level remains. Models are also removed from type constructors.

October 2007, M. Spiekermann revised the code in order to improve the example
code, e.g. limiting the scope of variables, careful value assignments,
C++-style cast operations, etc., and to use newer programming interfaces which
simplifies type mappings and operator registration.


0 Overview

This little algebra demonstrates the use of streams and parameter functions
in algebra operators. It does not introduce any type constructors, but has
several operators to manipulate streams. It provides the following operators:

  * intstream: int x int [->] (stream int)

    Creates a stream of integers containing all integers between the first and
the second argument. If the second argument is smaller than the first, the
stream will be empty.

  * countintstream: (stream T) [->] int

    Returns the number of elements in an int stream.

  * printintstream: (stream int) [->] (stream int)

    Prints out all elements of the int stream

  * filterintstream: (stream int) x (int [->] bool) [->] (stream int)

    Filters the elements of an int stream by a predicate.

1 Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"  //We need the SECONDO type int, for example
#include "Symbols.h"
#include "Stream.h"
#include "ListUtils.h"

#include <string>
#include <iostream>    //

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace ste {

/*

2 Algebra Implementation

2.2 Type Mapping Functions

These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.


Type mapping for ~intstream~ is

----    (int int) -> (stream int)
----

*/
ListExpr
intstreamType( ListExpr args ) {
  string err = "int x int expected";
  if(!nl->HasLength(args,2)){
    return listutils::typeError(err);
  }
  if(!CcInt::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args))){
    return listutils::typeError(err);
  }  
  return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType()),
                         nl->SymbolAtom(CcInt::BasicType()));
}

/*
Type mapping for ~count~ is

----    ((stream int)) -> int
----

*/
ListExpr
countType( ListExpr args )
{
  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected");
  }
  if(!Stream<CcInt>::checkType(nl->First(args))){
    return listutils::typeError("stream(int) expected");
  }
  return nl->SymbolAtom(CcInt::BasicType());
}

/*
Type mapping for ~printintstream~ is

----    ((stream int)) -> (stream int)
----

*/
ListExpr
printintstreamType( ListExpr args )
{

  if(!nl->HasLength(args,1)){
    return listutils::typeError("One argument expected");
  }
  if(!Stream<CcInt>::checkType(nl->First(args))){
    return listutils::typeError("stream(int) expected");
  }
  return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType()),
                         nl->SymbolAtom(CcInt::BasicType()));

}


/*
Type mapping for ~filter~ is

----    ((stream int) (map int bool)) -> (stream int)
----

*/
ListExpr
filterType( ListExpr args )
{
  if(!nl->HasLength(args,2)){
    return listutils::typeError("2 arguments expected");
  }
  ListExpr arg1 = nl->First(args);
  if(!Stream<CcInt>::checkType(arg1)){
    return listutils::typeError("first argument must be a stream(int)");
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->HasLength(arg2,3)){
    return listutils::typeError("second argument must be map: int -> bool");
  }

  ListExpr arg2_1 = nl->First(arg2);
  ListExpr arg2_2 = nl->Second(arg2);
  ListExpr arg2_3 = nl->Third(arg2);

  if(!listutils::isSymbol(arg2_1,Symbol::MAP()) ||
     !listutils::isSymbol(arg2_2, CcInt::BasicType()) ||
     !listutils::isSymbol(arg2_3, CcBool::BasicType())){
    return listutils::typeError("second argument must be map: int -> bool");
  }
  return arg1;
}

/*
2.3 Value Mapping Functions

2.3.1 Operator ~intstream~

Creates an integer stream. An example for creating a stream.

*/

int
intstreamFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  // An auxiliary type which keeps the state of this
  // operation during two requests
  struct Range {
    int current;
    int last;

    Range(CcInt* i1, CcInt* i2) {

      // Do a proper initialization even if one of the
      // arguments has an undefined value
      if (i1->IsDefined() && i2->IsDefined())
      {
        current = i1->GetIntval();
        last = i2->GetIntval();
      }
      else
      {
	// this initialization will create an empty stream
        current = 1;
        last = 0;
      }
    }
  };

  Range* range = static_cast<Range*>(local.addr);

  switch( message )
  {
    case OPEN: { // initialize the local storage

      CcInt* i1 = static_cast<CcInt*>( args[0].addr );
      CcInt* i2 = static_cast<CcInt*>( args[1].addr );
      range = new Range(i1, i2);
      local.addr = range;

      return 0;
    }
    case REQUEST: { // return the next stream element

      if ( range->current <= range->last )
      {
        CcInt* elem = new CcInt(true, range->current++);
        result.addr = elem;
        return YIELD;
      }
      else
      {
	// you should always set the result to null
	// before you return a CANCEL
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: { // free the local storage

      if (range != 0) {
        delete range;
        local.addr = 0;
      }

      return 0;
    }
    default: {
      /* should never happen */
      return -1;
    }
  }
}

/*
2.3.2 Value mapping for ~count~

Count the number of elements in a stream. An example for consuming a stream.

*/
int
countFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  qp->Open(args[0].addr); // open the argument stream

  Stream<CcInt> stream(args[0]);
  stream.open();

  CcInt* next = stream.request();
  int count =0;
  while(next!=0){
    count++;
    next->DeleteIfAllowed();
    next = stream.request();
  }
  stream.close();

  // Assign a value to the operations result object which is provided
  // by the query processor
  result = qp->ResultStorage(s);
  static_cast<CcInt*>(result.addr)->Set(true, count);

  return 0;
}

/*
2.3.3 Value mapping ~printintstream~

The next function prints the elements of a "stream(int)".
An example for a pure stream operator (input and output are streams).

*/

int
printintstreamFun (Word* args, Word& result,
                   int message, Word& local, Supplier s)
{
  switch( message )
  {
    case OPEN: {
      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST: {

      Word elem(Address(0));
      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
      {
        cout << static_cast<CcInt*>(elem.addr)->GetIntval() << endl;
        result = elem;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {

      qp->Close(args[0].addr);
      return 0;
    }
    default: {
      /* should not happen */
      return -1;
    }
  }
}

/*
2.3.4 Value mapping for ~filter~

Filter the elements of a stream by a predicate. An example for a stream
operator and also for one calling a parameter function.

*/
int
filterFun (Word* args, Word& result, int message, Word& local, Supplier s)
{

  switch( message )
  {
    case OPEN: {

      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST: {

      // Get the argument vector for the parameter function.
      ArgVectorPointer funargs = qp->Argument(args[1].addr);

      // Loop over stream elements until the function yields true.
      Word elem(Address(0));
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
      {
        // Supply the argument for the parameter function.
        (*funargs)[0] = elem;

        // Instruct the parameter function to be evaluated.
        Word funresult(Address(0));
        qp->Request(args[1].addr, funresult);
        CcBool* b = static_cast<CcBool*>( funresult.addr );

        bool funRes = b->IsDefined() && b->GetBoolval();

        if ( funRes )
        {
    // TRUE: Element passes the filter condition
          result = elem;
          return YIELD;
        }
        else
        {
           // FALSE: Element is rejected by the filter condition

          // consume the stream object (allow deletion)
          static_cast<CcInt*>( elem.addr )->DeleteIfAllowed();

    // Get next stream element
          qp->Request(args[0].addr, elem);
        }
      }

      // End of Stream reached
      result.addr = 0;
      return CANCEL;
    }
    case CLOSE: {

      qp->Close(args[0].addr);
      return 0;
    }
    default: {
      /* should never happen */
      return -1;
    }
  }
}

/*
2.4 Description of Operators

*/

struct intstreamInfo : OperatorInfo
{
  intstreamInfo() : OperatorInfo()
  {
    name      = "intstream";
    signature = CcInt::BasicType() + " x " + CcInt::BasicType()
                + " -> stream(int)";
    syntax    = "intstream (_ , _)";
    meaning   = "Creates a stream of integers containing the numbers "
                "between the first and the second argument.";
  }
};


struct countInfo :  OperatorInfo
{
  countInfo() : OperatorInfo()
  {
    name      = "countintstream";
    signature = "stream(int)  -> " + CcInt::BasicType();
    syntax    = "_ countintstream";
    meaning   = "Counts the number of elements of an int stream.";
  }
};

struct printintSInfo :  OperatorInfo
{
  printintSInfo() : OperatorInfo()
  {
    name      = "printintstream";
    signature = "stream(int)  -> stream(int)";
    syntax    = "_ printintstream";
    meaning   = "Prints the elements int stream.";
  }
};

struct filterInfo :  OperatorInfo
{
  filterInfo() : OperatorInfo()
  {
    name      = "filterintstream";
    signature = "stream(int) x (int -> bool) -> stream(int)";
    syntax    = "_ filterintstream[ function ]";
    meaning   = "Filters the elements of an int stream by a predicate.";
  }
};


/*
2.4 The algebra class

*/

class StreamExampleAlgebra : public Algebra
{
 public:
  StreamExampleAlgebra() : Algebra()
  {
    AddOperator( intstreamInfo(), intstreamFun, intstreamType );
    AddOperator( countInfo(), countFun, countType);
    AddOperator( printintSInfo(), printintstreamFun, printintstreamType );
    AddOperator( filterInfo(), filterFun, filterType );
  }
  ~StreamExampleAlgebra() {};
};

} // end of namespace ste

/*
3 Initialization

*/

extern "C"
Algebra*
InitializeStreamExampleAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  return (new ste::StreamExampleAlgebra);
}

