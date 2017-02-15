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

#include "GetTuples.h"
#include "ARel.h"
#include "NRel.h"


using namespace nr2a;

GetTuples::Info::Info()
{
  name = "gettuples";
  signature = "stream(tuple(X)) x " + ARel::BasicType()
      + " -> stream(tuple(Y))";
  appendSignature(
      "stream(tuple(X)) x " + NRel::BasicType()
          + " -> stream(tuple(Y))");
  syntax = "_ _ gettuples";
  meaning = "Retrieves tuples from a relation.";
  example = "query IndexDocsByTitle feed Documents gettuples consume2";
}

/*
To store its context while consuming the input stream "gettuples"[2] uses the
following structures.

*/
struct GetTuplesLocalInfoArel : public
    Nr2aLocalInfo<LinearProgressEstimator<GetTuplesLocalInfoArel> >
{
  public:
    ListExpr tupleTypeOut;
    Stream<Tuple>* inputStream;
    ARel* arel;
    ListExpr tupleTypeArel;
    int indexOfTidAttribute;
    int attributeCountStream;
    int attributeCountArel;

    GetTuplesLocalInfoArel()
    : tupleTypeOut(nl->TheEmptyList()), inputStream(NULL), arel(NULL),
      tupleTypeArel(nl->TheEmptyList()),indexOfTidAttribute(0),
      attributeCountStream(0), attributeCountArel(0)
    {
    }

    virtual ~GetTuplesLocalInfoArel() {};
};

struct GetTuplesLocalInfoNrel : public
  Nr2aLocalInfo<LinearProgressEstimator<GetTuplesLocalInfoNrel> >
{
  public:
    ListExpr tupleTypeOut;
    Stream<Tuple>* inputStream;
    NRel* nrel;
    ListExpr tupleTypeNrel;
    int indexOfTidAttribute;
    int attributeCountStream;
    int attributeCountNrel;

    GetTuplesLocalInfoNrel()
    : Nr2aLocalInfo(), tupleTypeOut(nl->TheEmptyList()), inputStream(NULL),
      nrel(NULL), tupleTypeNrel(nl->TheEmptyList()), indexOfTidAttribute(0),
      attributeCountStream(0), attributeCountNrel(0)
    {
    }
    virtual ~GetTuplesLocalInfoNrel() {};
};


GetTuples::~GetTuples()
{
}

/*
The operator expects a stream of tuples containing exactly one attribute of
type "tid"[2].

*/
/*static*/ListExpr GetTuples::MapType(ListExpr args)
{
  if (nl->ListLength(args) != 2)
  {
    return listutils::typeError("Two arguments expected");
  }
  ListExpr indexStream = nl->First(args);
  if (!listutils::isTupleStream(indexStream))
  {
    return listutils::typeError(
        "First argument is expected to be a tuple stream");
  }
  ListExpr nestedRelation = nl->Second(args);
  if (!Nr2aHelper::IsNestedRelation(nestedRelation))
  {
    return listutils::typeError("Second argument is expected to be a "
        "nested relation of type arel2 or nrel2");
  }

  ListExpr attributesIndex = nl->Second(nl->Second(indexStream));
  ListExpr attributesNrel = nl->Second(nl->Second(nestedRelation));
  // tp The maximum number of attributes. Result will contain 1 attribute less
  //    than the sum of the input attributes, because tid is removed.
  int attributesCount = nl->ListLength(attributesIndex)
      + nl->ListLength(attributesNrel)-1;

  std::vector<string> attributesNames(attributesCount);
  string errorMsg;
  int counter = -1;
  int counterTid = 0;
  listForeach(attributesIndex, current)
  {
    if (listutils::isValidAttributeName(nl->First(current), errorMsg))
    {
      if (listutils::isSymbol(nl->Second(current),
          TupleIdentifier::BasicType()))
      {
        ++counterTid;
      }
      else
      {
        attributesNames[++counter] = nl->SymbolValue(nl->First(current));
      }
    }
    else
    {
      return listutils::typeError(errorMsg);
    }
  }
  listForeach(attributesNrel, current)
  {
    if (listutils::isValidAttributeName(nl->First(current), errorMsg))
    {
      attributesNames[++counter] = nl->SymbolValue(nl->First(current));
    }
    else
    {
      return listutils::typeError(errorMsg);
    }
  }
  if (counterTid < 1)
  {
    return listutils::typeError(
        "Input stream is expected to contain an attribute of type tid");
  }
  else if (counterTid > 1)
  {
    return listutils::typeError(
        "Input stream is expected to contain exactly one attribute of type "
        "tid");
  }

  std::sort(attributesNames.begin(), attributesNames.end());
  std::vector<string>::iterator iterator = std::adjacent_find(
      attributesNames.begin(), attributesNames.end());
  if (iterator != attributesNames.end())
  {
    return listutils::typeError(
        "The resulting stream will contain attributes with duplicate names, "
            "which is not allowed");
  }

  ListExpr outStreamType = BuildResultingType(attributesIndex, attributesNrel);
  AutoWrite(outStreamType);

  return outStreamType;
}

/*static*/ValueMapping GetTuples::functions[] = { GetTuplesArel,
    GetTuplesNrel, NULL };

/*static*/int GetTuples::SelectFunction(ListExpr args)
{
  return Nr2aHelper::DefaultSelect(nl->Second(args));
}

/*
Attribute as well as nested relations provide a function to request tuples by
their position in storage, which is also known as ~tuple id~ of type "tid"[2].

*/
/*static*/int GetTuples::GetTuplesArel(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;
  GetTuplesLocalInfoArel *localInfo = NULL;

  switch (message)
  {
    case OPEN:
    {
      localInfo = new GetTuplesLocalInfoArel();
      localInfo->inputStream = new Stream<Tuple>(args[0]);
      localInfo->inputStream->open();
      localInfo->arel = static_cast<ARel*>(args[1].addr);
      localInfo->tupleTypeOut = SecondoSystem::GetCatalog()->NumericType(
          nl->Second(qp->GetSupplierTypeExpr(s)));
      localInfo->tupleTypeArel = SecondoSystem::GetCatalog()->NumericType(
          nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 1))));
      ListExpr indexAttributes = nl->Second(
          nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0))));
      int counter = 0;
      listForeach(indexAttributes, current)
      {
        if (listutils::isSymbol(nl->Second(current),
            TupleIdentifier::BasicType()))
        {
          localInfo->indexOfTidAttribute = counter;
          break;
        }
        else
        {
          ++counter;
        }
      }
      localInfo->attributeCountStream = nl->ListLength(
          nl->Second(nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0)))));
      localInfo->attributeCountArel = nl->ListLength(
          nl->Second(nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 1)))));

      local.addr = localInfo;
    }
      break;

    case REQUEST:
    {
      Tuple *tupleStream = NULL;
      localInfo = static_cast<GetTuplesLocalInfoArel*>(local.addr);
      tupleStream = localInfo->inputStream->request();
      if (tupleStream != NULL)
      {
        Tuple *tupleOut = new Tuple(localInfo->tupleTypeOut);
        Attribute *attributeIn = NULL;
        int writeOffset = 0;
        TupleId tid = 0;
        for (int i = 0; i < localInfo->attributeCountStream; ++i)
        {
          attributeIn = tupleStream->GetAttribute(i);
          if (i != localInfo->indexOfTidAttribute)
          {
            tupleOut->PutAttribute(i + writeOffset, attributeIn->Clone());
          }
          else
          {
            tid = ((TupleIdentifier*) attributeIn)->GetTid();
            writeOffset = -1;
          }

        }
        Tuple *tupleNrel = localInfo->arel->GetTuple(tid);
        for (int i = 0; i < localInfo->attributeCountArel; ++i)
        {
          attributeIn = tupleNrel->GetAttribute(i);
          tupleOut->PutAttribute(i + localInfo->attributeCountStream - 1,
              attributeIn->Clone());
        }
        tupleNrel->DeleteIfAllowed();
        tupleStream->DeleteIfAllowed();
        result.setAddr(tupleOut);
        resultInt = YIELD;
      }
      else
      {
        resultInt = CANCEL;
      }
    }
      break;

    case CLOSE:
      if (local.addr != NULL)
      {
        localInfo = static_cast<GetTuplesLocalInfoArel*>(local.addr);
        localInfo->inputStream->close();
        delete localInfo->inputStream;
        delete localInfo;
        local.addr = NULL;
      }
      break;
  }
  return resultInt;
}

/*static*/int GetTuples::GetTuplesNrel(Word* args, Word& result,
    int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;
  GetTuplesLocalInfoNrel *info = (GetTuplesLocalInfoNrel*)local.addr;

  switch (message)
  {
    case OPEN:
    {
      if (info) delete info;
      info = new GetTuplesLocalInfoNrel();
      info->inputStream = new Stream<Tuple>(args[0]);
      info->inputStream->open();
      info->nrel = static_cast<NRel*>(args[1].addr);
      info->tupleTypeOut = SecondoSystem::GetCatalog()->NumericType(
          nl->Second(qp->GetSupplierTypeExpr(s)));
      info->tupleTypeNrel = SecondoSystem::GetCatalog()->NumericType(
          nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 1))));
      ListExpr indexAttributes = nl->Second(
          nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0))));
      int counter = 0;
      listForeach(indexAttributes, current)
      {
        if (listutils::isSymbol(nl->Second(current),
            TupleIdentifier::BasicType()))
        {
          info->indexOfTidAttribute = counter;
          break;
        }
        else
        {
          ++counter;
        }
      }
      info->attributeCountStream = nl->ListLength(
          nl->Second(nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0)))));
      info->attributeCountNrel = nl->ListLength(
          nl->Second(nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 1)))));
      local.addr = info;
    }
      break;

    case REQUEST:
    {
      info->UnitReceived();
      Tuple *tupleOfStream = NULL;
      info = static_cast<GetTuplesLocalInfoNrel*>(local.addr);
      tupleOfStream = info->inputStream->request();
      if (tupleOfStream != NULL)
      {
        Tuple *tupleOut = new Tuple(info->tupleTypeOut);\
        Attribute *attributeIn = NULL;
        int writeOffset = 0;
        TupleId tid = 0;
        for (int i = 0; i < info->attributeCountStream; ++i)
        {
          attributeIn = tupleOfStream->GetAttribute(i);
          if (i == info->indexOfTidAttribute)
          {
            tid = ((TupleIdentifier*) attributeIn)->GetTid();
            writeOffset = -1;
          }
          else
          {
            tupleOut->PutAttribute(i + writeOffset, attributeIn->Clone());
          }

        }
        Tuple *tupleNrel = info->nrel->GetTuple(info->tupleTypeNrel, tid);
        for (int i = 0; i < info->attributeCountNrel; ++i)
        {
          attributeIn = tupleNrel->GetAttribute(i);
          tupleOut->PutAttribute(i + info->attributeCountStream - 1,
              attributeIn->Clone());
        }
        result.setAddr(tupleOut);
        tupleNrel->DeleteIfAllowed();
        tupleOfStream->DeleteIfAllowed();
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
      delete info->inputStream;
      delete info;
      local.addr = NULL;
      break;
  }
  return resultInt;
}

/*static*/ListExpr GetTuples::BuildResultingType(
    const ListExpr attributesIndex, const ListExpr attributesNrel)
{
  ListBuilder attributesTypesOut;
  listForeach(attributesIndex, current)
  {
    if (!listutils::isSymbol(nl->Second(current), "tid"))
    {
      attributesTypesOut.Append(current);
    }
  }
  listForeach(attributesNrel, current)
  {
    attributesTypesOut.Append(current);
  }
  ListExpr res = attributesTypesOut.GetTupleStream();
  AutoWrite(res);
  return res;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation GetTuples::costEstimators[] =
    { LinearProgressEstimator<GetTuplesLocalInfoArel>::Build,
      LinearProgressEstimator<GetTuplesLocalInfoNrel>::Build };
