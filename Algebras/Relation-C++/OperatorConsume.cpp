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
#include "OperatorConsume.h"

#include "Progress.h"

#ifdef USE_PROGRESS
#include "../CostEstimation/RelationAlgebraCostEstimation.h"
#endif

using namespace std;

ListExpr OperatorConsume::tconsume_tm(ListExpr args)
{

  if(nl->ListLength(args)!=1){
    return listutils::typeError("one argument expected");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))){
    ErrorReporter::ReportError("stream(tuple(...)) expected");
    return nl->TypeError();
  }

  return nl->Cons(nl->SymbolAtom(TempRelation::BasicType()),
                  nl->Rest(nl->First(args)));
}

/*
5.6.2 Value mapping function of operator ~consume~

*/

#ifndef USE_PROGRESS

// standard version

int
OperatorConsume::Consume(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  Word actual;

  GenericRelation* rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }

  Stream<Tuple> stream(args[0]);

  stream.open;

  Tuple* tup;
  while( (tup = stream.request()) != 0){
      rel->AppendTuple(tup);
      tup->DeleteIfAllowed();
  }
  stream.close();

  result.setAddr(rel);

  return 0;
}

#else
// Version with support for progress queries

CostEstimation* OperatorConsume::ConsumeCostEstimationFunc() {
  return new ConsumeCostEstimation();
}

int
OperatorConsume::Consume(Word* args, Word& result, int message,
       Word& local, Supplier s)
{
  Word actual;
  consumeLocalInfo* cli;

  if ( message <= CLOSE )     //normal evaluation
  {

    cli = (consumeLocalInfo*) local.addr;
    if ( cli ) delete cli;    //needed if consume used in a loop

    cli = new consumeLocalInfo(args[0]);

    local.setAddr(cli);

    GenericRelation* rel = (GenericRelation*)((qp->ResultStorage(s)).addr);
    if(rel->GetNoTuples() > 0)
    {
      rel->Clear();
    }

    cli->stream.open();
    Tuple* tuple;
    while( (tuple = cli->stream.request())!=0){
      rel->AppendTuple(tuple);
      tuple->DeleteIfAllowed();
      cli->current++;
    }
    result.setAddr(rel);

    cli->stream.close();
    cli->state = 1;

    delete cli;
    local.setAddr(0);
    return 0;
  }

  return 0;
}

#endif
