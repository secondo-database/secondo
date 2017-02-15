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

Nest::LocalInfo::LocalInfo(int groupedAttributesCount,
    int nestedAttributesCount)
    : tupleTypeOut(nl->TheEmptyList()), tupleTypeSubRel(nl->TheEmptyList()),
        groupedAttributesCount(groupedAttributesCount),
        nestedAttributesCount(nestedAttributesCount), inputStream(NULL),
        tupleIn(NULL), groupedAttributesIndices(groupedAttributesCount),
        nestedAttributesIndices(nestedAttributesCount)
{
}

Nest::LocalInfo::~LocalInfo()
{
}

Nest::~Nest()
{
}

/*
 The operator expects three input arguments:

 * A tuple stream to consume

 * A list of attributes not being nested

 * A name for the subrelation containing the other attributes

*/
/*static*/ListExpr Nest::MapType(ListExpr args)
{
  AutoWrite(args);
  if (nl->ListLength(args) != 3)
  {
    return listutils::typeError("Three arguments expected");
  }
  ListExpr inputStreamType = nl->First(args);
  ListExpr ungroupedAttributesList = nl->Second(args);
  ListExpr attributeNest = nl->Third(args);
  AutoWrite(ungroupedAttributesList);

  // tp Check first argument
  if (!listutils::isTupleStream(inputStreamType))
  {
    return nl->TypeError();
  }

  // tp Check second argument
  int ungroupedAttributesCount = nl->ListLength(ungroupedAttributesList);
  if (ungroupedAttributesCount == 0)
  {
    return listutils::typeError(
        "The second argument has to contain at least one attribute");
  }
  // Read next condition as:
  // "If the input stream does not have an attribute count of at elast one more
  // than ungroupedAttributes..."
  else if (!nl->HasMinLength(nl->Second(nl->Second(inputStreamType)),
      ungroupedAttributesCount + 1))
  {
    return listutils::typeError("The second argument must contain less "
        "attributes than the input stream, for at least one attribute has "
        "to remain unnested");
  }
  // ungroupedAttributesCount +1 for the new nrel2 attribute
  std::vector<string> attributesNames(ungroupedAttributesCount + 1);
  string errorMsg;
  int counter = -1;
  listForeach(ungroupedAttributesList, current)
  {
    if (listutils::isValidAttributeName(current, errorMsg))
    {
      attributesNames[++counter] = nl->SymbolValue(current);
    }
    else
    {
      return listutils::typeError(errorMsg);
    }
  }

  // tp Check third argument
  string attributeNestName = "";
  if (listutils::isValidAttributeName(attributeNest, errorMsg))
  {
    attributeNestName = nl->SymbolValue(attributeNest);
    attributesNames[++counter] = attributeNestName;
  }
  else
  {
    return listutils::typeError(errorMsg);
  }

  std::sort(attributesNames.begin(), attributesNames.end());
  std::vector<string>::iterator iterator = std::adjacent_find(
      attributesNames.begin(), attributesNames.end());
  if (iterator != attributesNames.end())
  {
    return listutils::typeError(
        "The resulting stream would contain attributes with duplicate names, "
        "which is not allowed");
  }

  // tp The name of the third argument is checked while building the
  //    resulting type

  // tp Build resulting type
  ListExpr result = nl->TheEmptyList();
  ListExpr outerTypeAttributeList = nl->Second(nl->Second(inputStreamType));
  ListBuilder attributesOuter;
  ListBuilder attributesSub;

  // tp Using the NestedRelationAlgebra the ungrouped attributes are ordered
  //    the way they are mentioned in the second argument of the nest operator.
  //    The NestedRelation2Algebra will behave the same, although this order
  //    is not of any meaning, given the relational calculus.

  std::vector<int> ungroupedAttributeIndexes(ungroupedAttributesCount);
  int currentPosInVector = -1;
  AutoWrite(ungroupedAttributesList);
  listForeach(ungroupedAttributesList, currentUngrouped)
  {
    ListExpr type = nl->TheEmptyList();
    int attributeIndexIn = listutils::findAttribute(outerTypeAttributeList,
        nl->SymbolValue(currentUngrouped), type) - 1;
    if (attributeIndexIn == -1)
    {
      return listutils::typeError("Not all attributes mentioned in the "
          "second argument do exist in the input stream");
    }
    ungroupedAttributeIndexes[++currentPosInVector] = attributeIndexIn;
    ListExpr newAttribute = nl->TwoElemList(currentUngrouped, type);
    attributesOuter.Append(newAttribute);
  }

  int currentIndex = 0;
  currentPosInVector = 0;
  listForeach(outerTypeAttributeList, currentOuter)
  {
    bool isSub = true;
    for(int i =0; i<ungroupedAttributesCount;i++)
    {
      isSub &= (currentIndex != ungroupedAttributeIndexes[i]);
    }
    if(isSub)
    {
      attributesSub.Append(currentOuter);
    }
    currentIndex++;
  }
  attributesOuter.Append(
      nl->TwoElemList(attributeNest, attributesSub.GetARel()));
  result = attributesOuter.GetTupleStream();
  AutoWrite(result);

  return result;
}

/*static*/ValueMapping Nest::functions[] = { NestValue, NULL };

/*static*/int Nest::SelectFunction(ListExpr args)
{
  return 0;
}

/*
 The value function handles the requests of the successing operator. It calls a
 helper function to get tuples to yield.

*/
/*static*/int Nest::NestValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  int resultInt = 0;

  LocalInfo *localInfo = (LocalInfo*)local.addr;
  string subRelName = "";
  NList nlist(NList(NList("Attr"), NList("string")));
  switch (message)
  {
    case OPEN:
    {
      ListExpr tupleTypeOfInStream = nl->Second(
          qp->GetSupplierTypeExpr(qp->GetSon(s, 0)));
      AutoWrite(tupleTypeOfInStream);
      ListExpr groupedAttributes = qp->GetSupplierTypeExpr(qp->GetSon(s, 1));
      AutoWrite(groupedAttributes);
      string subRelName = nl->SymbolValue(
          qp->GetSupplierTypeExpr(qp->GetSon(s, 2)));
      ListExpr attributesTypesOfInStream = nl->Second(tupleTypeOfInStream);
      int inputAttributesCount = nl->ListLength(attributesTypesOfInStream);
      int groupedAttributesCount = nl->ListLength(groupedAttributes);
      localInfo = new LocalInfo(groupedAttributesCount,
          inputAttributesCount - groupedAttributesCount);
      localInfo->inputStream = new Stream<Tuple>(args[0]);
      localInfo->inputStream->open();
      localInfo->tupleTypeOut = SecondoSystem::GetCatalog()->NumericType(
          nl->Second(qp->GetSupplierTypeExpr(s)));
      localInfo->tupleTypeSubRel = nl->Second(
          nl->Second(
              nl->Nth(groupedAttributesCount + 1,
                  nl->Second(localInfo->tupleTypeOut))));
      std::vector<string> groupedAttributesNames(
          localInfo->groupedAttributesCount);
      int counter = -1;
      listForeach(groupedAttributes, current)
      {
        ListExpr type;
        localInfo->groupedAttributesIndices[++counter] =
            listutils::findAttribute(attributesTypesOfInStream,
                nl->SymbolValue(current), type) - 1;
      }

      int outCounter = -1;
      AutoWrite(attributesTypesOfInStream);
      const unsigned int attrCount = localInfo->groupedAttributesCount
          + localInfo->nestedAttributesCount;
      for (unsigned int inCounter = 0; inCounter < attrCount; ++inCounter)
      {
        if (std::find(localInfo->groupedAttributesIndices.begin(),
            localInfo->groupedAttributesIndices.end(), inCounter)
            == localInfo->groupedAttributesIndices.end())
        {
          localInfo->nestedAttributesIndices[++outCounter] = inCounter;
        }
      }
      break;
    }

    case REQUEST:
    {
      localInfo->UnitReceived();
      Tuple *tupleOut = BuildResultTuple(*localInfo);
      if (tupleOut != NULL)
      {
        result.setAddr(tupleOut);
        resultInt = YIELD;
      }
      else
      {
        result.setAddr(NULL);
        resultInt = CANCEL;
      }
      break;
    }

    case CLOSE:
      if (localInfo != NULL)
      {
        localInfo->inputStream->close();
        delete localInfo->inputStream;
        delete localInfo;
        localInfo = NULL;
      }
      break;
  }
  local.addr = localInfo;
  return resultInt;
}

/*
 This helper function is called once per top level tuple requested. The values
 requested from the stream are compared to their predecessor to
 decide when to finish collecting tuples for a subrelation and to create a new
 top level tuple. In other words: It is assumed that the input stream is
 sorted by the attributes not being nested (the ones given as second argument).

 The built top level tuple is then returned.

*/
/*static*/Tuple * Nest::BuildResultTuple(LocalInfo & localInfo)
{
  AutoWrite(localInfo.tupleTypeOut);
  Tuple *result = NULL;
  if (localInfo.tupleIn == NULL)
  {
    localInfo.tupleIn = localInfo.inputStream->request();
  }

  if (localInfo.tupleIn != NULL)
  {
    Attribute *attributeIn = NULL;
    result = new Tuple(localInfo.tupleTypeOut);
    for (unsigned int i = 0; i < localInfo.groupedAttributesCount; i++)
    {
      attributeIn = localInfo.tupleIn->GetAttribute(
          localInfo.groupedAttributesIndices[i]);
      result->PutAttribute(i, attributeIn->Clone());
    }

    ARel *nestedRelation = new ARel(nl->TwoElemList(nl->SymbolAtom(
        ARel::BasicType()), localInfo.tupleTypeSubRel));

    bool continueRequest = true;
    while (continueRequest)
    {
      //Check if the not nested attributes of the new tuple are still equal to
      //those of the previous tuple. If so the nesting is still not finished.
      Tuple *nestedTuple = new Tuple(localInfo.tupleTypeSubRel);

      for (unsigned int i = 0; i < localInfo.nestedAttributesCount; i++)
      {
        attributeIn = localInfo.tupleIn->GetAttribute(
            localInfo.nestedAttributesIndices[i]);
        nestedTuple->PutAttribute(i, attributeIn->Clone());
      }

      nestedRelation->AppendTuple(nestedTuple);
      localInfo.tupleIn->DeleteIfAllowed();

      //Now check, if it is neccessary to continue nesting at the next tuple
      localInfo.tupleIn = localInfo.inputStream->request();

      //There is no need to check for continuation, if no more tuples are
      //available to nest
      continueRequest = (localInfo.tupleIn != NULL);

      //If continuation is not finished yet, continue if all attributes not
      //getting nested are equal to those of the other tuples processed yet
      unsigned int i = 0;
      while(continueRequest && (i < localInfo.groupedAttributesCount))
      {
        const Attribute *a1 = result->GetAttribute(i);
        const Attribute *a2 = localInfo.tupleIn->GetAttribute(
            localInfo.groupedAttributesIndices[i]);
        //Equality is checked iff hashes are equal
        continueRequest &= ((a1->HashValue() == a2->HashValue())
            && (a1->Compare(a2) == 0));
        i++;
      }
      localInfo.UnitProcessed();
    }
    result->PutAttribute(localInfo.groupedAttributesCount, nestedRelation);
  }
  return result;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation Nest::costEstimators[] =
  { LinearProgressEstimator<LocalInfo>::Build };
