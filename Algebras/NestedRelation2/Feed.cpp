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

*/

#include <unistd.h> //todo Remove

#include "Include.h"
#include "LinearProgressEstimator.h"

using namespace nr2a;

Feed::Info::Info()
{
  name = "feed";
  signature = ARel::BasicType() + "(tuple(X)) -> "
      + "stream(tuple(X))";
  appendSignature(NRel::BasicType() + "(tuple(X)) -> "
      + "stream(tuple(X))");
  syntax = "_ feed";
  meaning = "Produces a stream from a nested relation by scanning "
      "the relation tuple by tuple.";
  example = "query Documents feed project[Title] consume";
}

Feed::~Feed()
{
}

/*
The operator accepts "nrel2"[2] and "arel2"[2]. The output type is a tuple
stream whose tuple type equals the type of the relation's tuples.

*/
/*static*/ListExpr Feed::MapType(ListExpr args)
{
  AutoWrite(args);
  if (!nl->HasLength(args,1))
  {
    return listutils::typeError("One argument expected");
  }
  ListExpr relType = nl->First(args);
  ListExpr result = nl->TheEmptyList();
  if (Nr2aHelper::IsNestedRelation(relType))
  {
    ListExpr tupleType = nl->Second(relType);
    result = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), tupleType);
  }
  else
  {
    result = listutils::typeError("Input is expected to be of type nrel2 "
        "or arel2");
  }

  return result;
}

ValueMapping Feed::functions[] = { FeedArel, FeedNrel, NULL };

/*static*/int Feed::SelectFunction(ListExpr args)
{
  return Nr2aHelper::DefaultSelect(nl->First(args));
}

/*
The relations are iterated on request until the stream is closed or the
relation is iterated to its end.

*/
/*static*/int Feed::FeedArel(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  int resultInt = 0;
  result = qp->ResultStorage(s);
  LocalInfoArel *info = (LocalInfoArel *)local.addr;
  switch (message)
  {
    case OPEN:
    {
      ARel *arel = static_cast<ARel*>(args[0].addr);
      ListExpr tupleType = nl->TheEmptyList();
      if (qp->GetNoSons(s) == 1)
      {
        tupleType = qp->GetSupplierTypeExpr(s);
        tupleType = nl->Second(tupleType);
      }
      ProgressInfo progressInfo;
      progressInfo.Card = arel->GetTupleCount();
      ARelIterator *iter = new ARelIterator(arel,0,0);
      info = new LocalInfoArel(iter, args[0].addr, progressInfo);
      local.addr = info;
    }
      break;
    case REQUEST:
    {
      info->UnitReceived();
      Tuple *t = NULL;
      t = info->iter->getNextTuple();
      if (t != NULL)
      {
        result.setAddr(t);
        info->UnitProcessed();
        resultInt = YIELD;
      }
      else
      {
        resultInt = CANCEL;
      }
    }
      break;

    case CLOSE:
      if (local.addr)
      {
        delete info;
        local.addr = NULL;
      }
      break;
  }
  return resultInt;
}

/*static*/int Feed::FeedNrel(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;
  LocalInfoNrel *info = (LocalInfoNrel *)local.addr;
  switch (message)
  {
    case OPEN:
    {
      ListExpr tupleType = nl->TheEmptyList();
      NRel *nrel = static_cast<NRel*>(args[0].addr);
      if (qp->GetNoSons(s) == 1)
      {
        tupleType = qp->GetSupplierTypeExpr(s);
        tupleType = nl->Second(tupleType);
      }
      ProgressInfo progressInfo;
      progressInfo.Card = nrel->GetTupleCount();
      info = new LocalInfoNrel(nrel->getIterator(tupleType), args[0].addr,
          progressInfo);
      local.addr = info;
    }
      break;
    case REQUEST:
    {
      info->UnitReceived();
      Tuple *t = NULL;
      t = info->iter->getNextTuple();
      if (t != NULL)
      {
        result.setAddr(t);
        resultInt = YIELD;
        info->UnitProcessed();
      }
      else
      {
        resultInt = CANCEL;
      }
    }
      break;

    case CLOSE:
      if (local.addr)
      {
        delete info;
        local.addr = NULL;
      }
      break;
  }
  return resultInt;
}

CreateCostEstimation Feed::costEstimators[] =
  { GetCostEstimator<Feed::LocalInfoArel>,
    GetCostEstimator<Feed::LocalInfoNrel> };

template <class T>
/*static*/ CostEstimation * Feed::GetCostEstimator()
{
  return new LinearProgressEstimator<T>();
}


Feed::LocalInfoArel::LocalInfoArel(ARelIterator * iter,
    void * predecessor, const ProgressInfo base)
: iter(iter)
{
  //Intentionally left blank
}

Feed::LocalInfoArel::~LocalInfoArel()
{
  delete iter;
}

Feed::LocalInfoNrel::LocalInfoNrel(NRelIterator * iter,
    void * predecessor, const ProgressInfo base)
    : iter(iter)
{
  Nr2aLocalInfo<LinearProgressEstimator<LocalInfoNrel> >::base = base;
}

Feed::LocalInfoNrel::~LocalInfoNrel()
{
  delete iter;
}
