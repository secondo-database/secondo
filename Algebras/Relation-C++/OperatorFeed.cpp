/*
----
This file is part of SECONDO.

Copyright (C) 2016,
Faculty of Mathematics and Computer Science,
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


//[$][\$]
//[_][\_]

*/
#include "OperatorFeed.h"

#include "Algebra.h"
#include "Symbols.h"
#include "Progress.h"
#include "RelationAlgebra.h"
#include "Stream.h"

#ifdef USE_PROGRESS
#include "../CostEstimation/RelationAlgebraCostEstimation.h"
#endif

using namespace std;

/*

5.5 Operator ~feed~

Produces a stream from a relation by scanning the relation tuple by
tuple.

5.5.1 Type mapping function of operator ~feed~

A type mapping function takes a nested list as argument. Its
contents are type descriptions of an operator's input parameters.
A nested list describing the output type of the operator is returned.

Result type of feed operation.

----    ((rel x))  -> (stream x)
----

*/
ListExpr OperatorFeed::FeedTypeMap(ListExpr args)
{
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("one argument expected");
    return nl->TypeError();
  }
  ListExpr first = nl->First(args);
  if(listutils::isRelDescription(first,true)||
     listutils::isRelDescription(first,false)){
    return nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(first));
  }
  if(listutils::isOrelDescription(first))
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->Second(first));
  ErrorReporter::ReportError("rel(tuple(...)), trel(tuple(...)) or "
                              "orel(tuple(...)) expected");
  return nl->TypeError();
}
/*

5.5.2 Value mapping function of operator ~feed~

*/


#ifndef USE_PROGRESS

// standard version of code

int
OperatorFeed::Feed(Word* args, Word& result,
                   int message, Word& local, Supplier s)
{
  GenericRelation* r;
  GenericRelationIterator* rit;

  switch (message)
  {
    case OPEN :
      r = (GenericRelation*)args[0].addr;
      rit = r->MakeScan();

      local.addr = rit;
      return 0;

    case REQUEST :
      rit = (GenericRelationIterator*)local.addr;
      Tuple *t;
      if ((t = rit->GetNextTuple()) != 0)
      {
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }

    case CLOSE :
      if(local.addr)
      {
         rit = (GenericRelationIterator*)local.addr;
         delete rit;
         local.addr = 0;
      }
      return 0;
  }
  return 0;
}


#else

// version with support for progress queries
CostEstimation* OperatorFeed::FeedCostEstimationFunc() {
  return new FeedCostEstimation();
}

int
OperatorFeed::Feed(Word* args, Word& result,
                   int message, Word& local, Supplier s)
{
  GenericRelation* r=0;
  FeedLocalInfo* fli=0;

  switch (message)
  {
    case OPEN :{
      r = (GenericRelation*)args[0].addr;

      fli = (FeedLocalInfo*) local.addr;
      if ( fli ) delete fli;

      fli = new FeedLocalInfo();
      fli->returned = 0;
      fli->total = r->GetNoTuples();
      fli->rit = r->MakeScan();
      local.setAddr(fli);
      return 0;
    }
    case REQUEST :{
      fli = (FeedLocalInfo*) local.addr;
      Tuple *t;
      if ((t = fli->rit->GetNextTuple()) != 0)
      {
        fli->returned++;
        result.setAddr(t);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }
    case CLOSE :{
        // Note: object deletion is handled in OPEN and CLOSEPROGRESS
        // keep the local info structure since it may still be
        // needed for handling progress messages.
      fli = static_cast<FeedLocalInfo*>(local.addr);
      if(fli){
        delete fli;
        local.addr = NULL;
      }
      return 0;
    }
  }
  return 0;
}
#endif

