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

#include "Include.h"



using namespace nr2a;

/*
 Structure to store the context of the operator.

*/
struct UnnestLocalInfo :
    public Nr2aLocalInfo<LinearProgressEstimator<UnnestLocalInfo> >
{
  public:
    UnnestLocalInfo();
    virtual ~UnnestLocalInfo();

    Tuple *tupleOut;
    TupleType* tupleTypeOut;
    Stream<Tuple>* inputStream;
    ARel * subRel;
    ARelIterator *subRelIterator;
    TupleType* tupleTypeSubRel;
    ListExpr tupleTypeListSubRel;
    int attributeIndex;
};

UnnestLocalInfo::UnnestLocalInfo()
    : tupleOut(NULL), tupleTypeOut(0), inputStream(NULL),
        subRel(NULL), subRelIterator(NULL),
        tupleTypeSubRel(0), attributeIndex(-1)
{
}

UnnestLocalInfo::~UnnestLocalInfo()
{

}

Unnest::~Unnest()
{
}

/*static*/ListExpr Unnest::MapType(ListExpr args)
{
  AutoWrite(args);
  if (nl->ListLength(args) != 2)
  {
    return listutils::typeError("Two arguments expected");
  }
  ListExpr inStreamType = nl->First(args);
  ListExpr attributeToUnnest = nl->Second(args);
  if (!listutils::isTupleStream(inStreamType))
  {
    return nl->TypeError();
  }
  string attributeUnnestName = "";
  if (listutils::isSymbol(attributeToUnnest))
  {
    ListExpr type = nl->Empty();
    attributeUnnestName = nl->SymbolValue(attributeToUnnest);
    if (listutils::findAttribute(nl->Second(nl->Second(inStreamType)),
        attributeUnnestName, type) == 0)
    {
      return listutils::typeError("The attribute mentioned in the "
          "second argument does not exist in the input stream");
    }
    else
    {
      if (!nl->IsEqual(nl->First(type), ARel::BasicType()))
      {
        return listutils::typeError("The attribute mentioned in the "
            "second argument is not a nested relation of type arel2");
      }
    }
  }
  else
  {
    return nl->TypeError();
  }

  ListExpr attributesTypesIn = nl->Second(nl->Second(inStreamType));
  ListExpr result = BuildResultingType(attributesTypesIn,
      attributeUnnestName);
  AutoWrite(result);

  return result;
}

/*static*/ValueMapping Unnest::functions[] = { UnnestValue, NULL };

/*static*/int Unnest::SelectFunction(ListExpr args)
{
  return 0;
}

/*static*/int Unnest::UnnestValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;

  UnnestLocalInfo *localInfo = NULL;
  switch (message)
  {
    case OPEN:
    {
      ListExpr attributesTypesIn = nl->Second(
            nl->Second(qp->GetSupplierTypeExpr(qp->GetSon(s, 0))));
      ListExpr attributeType_unused;
      string subRelName = nl->SymbolValue(
          qp->GetSupplierTypeExpr(qp->GetSon(s, 1)));
      ListExpr tupleTypeOut = nl->Second(qp->GetSupplierTypeExpr(s));

      localInfo = new UnnestLocalInfo();
      localInfo->inputStream = new Stream<Tuple>(args[0]);
      localInfo->inputStream->open();
      localInfo->attributeIndex = listutils::findAttribute(attributesTypesIn,
          subRelName, attributeType_unused) - 1;
      localInfo->tupleTypeListSubRel = nl->Second(
                      nl->Second(
                        nl->Nth((localInfo->attributeIndex + 1),
                                 attributesTypesIn)));
      localInfo->tupleTypeSubRel = new TupleType( 
                   SecondoSystem::GetCatalog()->NumericType(nl->Second(
                     nl->Second(
                       nl->Nth((localInfo->attributeIndex + 1), 
                                attributesTypesIn)))));
      localInfo->tupleTypeOut = new TupleType(
              SecondoSystem::GetCatalog()->NumericType(tupleTypeOut));
      local.addr = localInfo;
    }
      break;
    case REQUEST:
    {
      localInfo = static_cast<UnnestLocalInfo*>(local.addr);
      if(!localInfo) return CANCEL;

      Tuple *tupleSubRel = NULL;
      // Test if there is a tuple of the sub-relation available, ...
      tupleSubRel =
          (localInfo->subRelIterator != NULL) ?
              localInfo->subRelIterator->getNextTuple() : NULL;
      // ...if not advance to a new tuple of the input stream containing
      // another attribute relation
      if (tupleSubRel == NULL)
      {
        resultInt = -1;
        while(resultInt == -1)
        {
          Tuple *tupleRel = localInfo->inputStream->request();
          if (tupleRel != NULL)
          {
            if (localInfo->subRel != NULL)
            {
              localInfo->subRel->DeleteIfAllowed();
            }
            localInfo->subRel = static_cast<ARel*>(tupleRel->GetAttribute(
                localInfo->attributeIndex)->Copy());
            if (localInfo->subRelIterator != NULL)
            {
              delete localInfo->subRelIterator;
            }
            localInfo->subRelIterator = new ARelIterator(localInfo->subRel,
                                                localInfo->tupleTypeSubRel,
                                                localInfo->tupleTypeListSubRel);
            tupleSubRel = localInfo->subRelIterator->getNextTuple();

            if (localInfo->tupleOut != NULL)
            {
              localInfo->tupleOut->DeleteIfAllowed();
              localInfo->tupleOut = NULL;
            }
            if (tupleSubRel != NULL)
            {
              localInfo->tupleOut = BuildTuple(localInfo->tupleTypeOut,
                  tupleRel, tupleSubRel, localInfo->attributeIndex);
              resultInt = 0;
            }
            //else
            {
              //Here empty subrelations could be handled by producing a tuple
              //with the attributes of the subrelation set to undefined. This
              //is a little less intuitive, but will avoid losing information
              //of the outer relation by unnesting its empty subrelations.
              //If this is the only way handling empty subrelations the while
              //loop is not needed anymore, for it will then always be used
              //only once each time called.
            }
            tupleRel->DeleteIfAllowed();
          }
          else
          {
            resultInt = CANCEL;
          }
        }
      }
      if (resultInt == 0)
      {
        Tuple *tupleRet = localInfo->tupleOut->Clone();
        TupleSetValuesOfSubRel(tupleRet, tupleSubRel,
            localInfo->attributeIndex);
        result.setAddr(tupleRet);
        resultInt = YIELD;
      }
      if (tupleSubRel != NULL)
      {
        delete tupleSubRel;
      }
    }
      break;

    case CLOSE:
      localInfo = static_cast<UnnestLocalInfo*>(local.addr);
      if(localInfo){
        localInfo->inputStream->close();
        delete localInfo->inputStream;
        delete localInfo->subRelIterator;
        localInfo->tupleTypeOut->DeleteIfAllowed();
        localInfo->tupleTypeSubRel->DeleteIfAllowed();
        if (localInfo->tupleOut != NULL)
        {
          localInfo->tupleOut->DeleteIfAllowed();
        }
        if (localInfo->subRel != NULL)
        {
          localInfo->subRel->DeleteIfAllowed();
        }
        delete localInfo;
        local.addr = NULL;
      }
      break;
  }
  return resultInt;
}

/*static*/Tuple * Unnest::BuildTuple(
    TupleType* tupleTypeOut,
    const Tuple * const tupleInStream, 
    const Tuple * const tupleSubRel,
    const int attributeIndex)
{
  Tuple* result = new Tuple(tupleTypeOut);
  int attributeCountInStream =
      tupleInStream->GetTupleType()->GetNoAttributes();
  //int attributeCountSubRel = tupleSubRel->GetTupleType()->GetNoAttributes();
  int attributeCountSubRel =
      result->GetTupleType()->GetNoAttributes() - (attributeCountInStream -1);
  for (int i = 0; i < attributeCountInStream; i++)
  {
    if (i != attributeIndex)
    {
      const int resultIndex =
          (i < attributeIndex) ? i : i + attributeCountSubRel - 1;
      result->PutAttribute(resultIndex,
          tupleInStream->GetAttribute(i)->Clone());
    }
  }
  return result;
}

/*static*/void Unnest::TupleSetValuesOfSubRel(Tuple * tupleOut,
    const Tuple * const tupleSubRel, const int attributeIndex)
{
  int attributeCountSubRel = tupleSubRel->GetTupleType()->GetNoAttributes();
  for (int i = 0; i < attributeCountSubRel; i++)
  {
    int resultIndex = i + attributeIndex;
    tupleOut->PutAttribute(resultIndex, tupleSubRel->GetAttribute(i)->Copy());
  }
}

/*static*/ListExpr Unnest::BuildResultingType
    (const ListExpr attributesTypes, const string attributeUnnestName)
{
  ListBuilder attributesTypesOut;
  listForeach(attributesTypes, current)
  {
    AutoWrite(current);
    if (nl->SymbolValue(nl->First(current)) == attributeUnnestName)
    {
      if ((nl->IsAtom(nl->First(nl->Second(current))))
          && (nl->SymbolValue(nl->First(nl->Second(current)))
              == ARel::BasicType())
          && (listutils::isTupleDescription
              (nl->Second(nl->Second(current)))))
      {
        ListExpr sub = nl->Second(nl->Second(nl->Second(current)));
        listForeach(sub, currentSub)
        {
          AutoWrite(currentSub);
          attributesTypesOut.Append(currentSub);
        }
      }
    }
    else
    {
      attributesTypesOut.Append(current);
    }
  }
  ListExpr res = attributesTypesOut.GetTupleStream();
  AutoWrite(res);
  return res;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Unnest::costEstimators[] =
  { LinearProgressEstimator<UnnestLocalInfo>::Build };
