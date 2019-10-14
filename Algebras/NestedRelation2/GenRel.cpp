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
#include "../FText/FTextAlgebra.h"
#include "../NestedRelation/NestedRelationAlgebra.h"
#include "../Collection/CollectionAlgebra.h"
#include "../Record/Record.h"

using namespace nr2a;
using namespace std;

const string cNrel2 = "nrel2";

GenRel::~GenRel()
{
}

/*
The operator expects eight parameters. The resulting type depends on several
of these, so the operator is marked as using values for type mapping, which
results in the arguments being nested lists of length two caontaining the type
of the argument first and its value second.

Some of the arguments have to be evaluated, to determine the resulting type.
The type of the subrelation is a special case, for it influences the resulting
type but need not be evaluated. It is

*/
/*static*/ListExpr GenRel::MapType(ListExpr args)
{
  AutoWrite(args);

  if (nl->ListLength(args) != 8)
  {
    return listutils::typeError("eight arguments expected");
  }
  for (int i = 1; i <= 8; i++)
  {
    if (!nl->HasLength(nl->Nth(i, args), 2))
    {
      return listutils::typeError(
          "eight arguments with two elements each expected");
    }
  }

  ListExpr result = nl->TheEmptyList();

  ListExpr relTypeList = nl->First(args);
  ListExpr namePrefixList = nl->Second(args);
  ListExpr attrCountList = nl->Third(args);
  //ListExpr tupleCountList = nl->Fourth(args);
  ListExpr attrTypeList = nl->Fifth(args);
  //ListExpr dataParamList = nl->Sixth(args);
  //ListExpr dataParam2List = nl->Seventh(args);
  ListExpr subRelGenList = nl->Eigth(args);

  Word relTypeWord;
  Word namePrefixWord;
  Word attrCountWord;
  Word attrTypeWord;
  bool correct = false;
  bool evaluable = false;
  bool defined = false;
  bool isFunction = false;
  OpTree opTree1 = NULL;
  OpTree opTree2 = NULL;
  OpTree opTree3 = NULL;
  OpTree opTree4 = NULL;
  ListExpr resultType1;
  ListExpr resultType2;
  ListExpr resultType3;
  ListExpr resultType4;

  QueryProcessor *qpp = new QueryProcessor
      ( nl, SecondoSystem::GetAlgebraManager() );
  qpp->Construct(nl->Second(relTypeList), correct, evaluable, defined,
      isFunction, opTree1, resultType1);
  qpp->Request(opTree1, relTypeWord);
  qpp->Construct(nl->Second(namePrefixList), correct, evaluable, defined,
      isFunction, opTree2, resultType2);
  qpp->Request(opTree2, namePrefixWord);
  qpp->Construct(nl->Second(attrCountList), correct, evaluable, defined,
      isFunction, opTree3, resultType3);
  qpp->Request(opTree3, attrCountWord);
  qpp->Construct(nl->Second(attrTypeList), correct, evaluable, defined,
      isFunction, opTree4, resultType4);
  qpp->Request(opTree4, attrTypeWord);

  string relType = ((CcString*)relTypeWord.addr)->GetValue();
  string namePrefix = ((CcString*)namePrefixWord.addr)->GetValue();
  int attrCount = ((CcInt*)attrCountWord.addr)->GetValue();
  //int tupleCount = nl->IntValue(nl->Second(tupleCountList));
  string attrType = ((CcString*)attrTypeWord.addr)->GetValue();
  //int dataParam = nl->IntValue(nl->Second(dataParamList));
  //int dataParam2 = nl->IntValue(nl->Second(dataParamList));

  // Generate relation type
  ListBuilder attributeTypes;
  // String (MAX_STRINGSIZE bytes)
  // an positive integer represented using decimal system (up to 10 bytes)
  // and an null termination (1 byte)
  char attrName[MAX_STRINGSIZE+10+1];
  for (int a = 0; a < attrCount; a++)
  {
    sprintf(attrName, "%s%d", namePrefix.c_str(), a);
    attributeTypes.Append(
        nl->TwoElemList(nl->SymbolAtom(attrName), nl->SymbolAtom(attrType)));
  }
  if (!listutils::isSymbolUndefined(nl->First(subRelGenList)))
  {
    attributeTypes.Append(
        nl->TwoElemList(nl->SymbolAtom("SubRel"), nl->First(subRelGenList)));
  }
  result = nl->TwoElemList(nl->SymbolAtom(relType), (relType=="vector")?
      attributeTypes.GetRecord():attributeTypes.GetTuple());

  AutoWrite(result);
  qpp->Destroy(opTree1, true);
  qpp->Destroy(opTree2, true);
  qpp->Destroy(opTree3, true);
  qpp->Destroy(opTree4, true);
  delete qpp;
  return result;
}

ValueMapping GenRel::functions[] = { GenRelValue, NULL };

/*static*/int GenRel::SelectFunction(ListExpr args)
{
  return 0;
}

/*
The value mapping first determines if a subrelation is to build. It then
differentiates by the type of relation to build. The inner loop building the
attributes differentiates by the type of attributes specified. At the end of
both loops the built attributes and the built tuples are appended respectively.

*/
/*static*/int GenRel::GenRelValue(Word* args, Word& result, int message,
    Word& local, Supplier s)
{
  string relType = static_cast<CcString*>(args[0].addr)->GetValue();
  //string namePrefix = static_cast<CcString*>(args[1].addr)->GetValue();
  int attrCount = static_cast<CcInt*>(args[2].addr)->GetValue();
  int tupleCount = static_cast<CcInt*>(args[3].addr)->GetValue();
  string dataType = static_cast<CcString*>(args[4].addr)->GetValue();
  int dataParam = static_cast<CcInt*>(args[5].addr)->GetValue();
  int dataParam2 = static_cast<CcInt*>(args[6].addr)->GetValue();
  Attribute *subRel = static_cast<Attribute*>(args[7].addr);

  ListExpr relTypeList = SecondoSystem::GetCatalog()->NumericType(
      qp->GetSupplierTypeExpr(s));
  AutoWrite(relTypeList);
  bool hasSubRel = !listutils::isSymbolUndefined(
      qp->GetSupplierTypeExpr(qp->GetSon(s, 7)));

  //Helper variables for Record/Vector
  int recordSize = 0;
  string *recordAttrTypes=0;
  string *recordAttrNames=0;

  void *res = NULL;
  if (relType == NRel::BasicType())
  {
    res = qp->ResultStorage(s).addr;
  }
  else if (relType == ARel::BasicType())
  {
    res = qp->ResultStorage(s).addr;
  }
  else if (relType == "nrel")
  {
    res = qp->ResultStorage(s).addr;
  }
  else if (relType == "arel")
  {
    ListExpr arelRelTypeList = SecondoSystem::GetCatalog()->NumericType(
        nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()),
            nl->Second(relTypeList)));
    AutoWrite(arelRelTypeList);
    AutoWrite(relTypeList);
    Relation *rel = new Relation(arelRelTypeList);
    AttributeRelation *arel =
        (AttributeRelation *)qp->ResultStorage(s).addr;
    arel->setRel(rel);
    arel->setRelId(rel->GetFileId());
    arel->setPartOfNrel(false);
    res = arel;
  }
  else if (relType == "vector")
  {
    //Skip the "record" symbol at first place in the given list
    ListExpr recordDescription =
        nl->Rest(nl->Second(qp->GetSupplierTypeExpr(s)));
    recordSize = nl->ListLength(recordDescription);
    recordAttrNames = new string[recordSize];
    recordAttrTypes = new string[recordSize];
    int i = 0;
    listForeach(recordDescription, current)
    {
      AutoWrite(current);
      recordAttrNames[i] = nl->SymbolValue(nl->First(current));
      recordAttrTypes[i] = nl->SymbolValue((nl->IsAtom(nl->Second(current)))?
          nl->Second(current):nl->First(nl->Second(current)));
      i++;
    }
    res = qp->ResultStorage(s).addr;
    ((collection::Collection*)res)->SetDefined(true);
  }

  for (int tupleIndex = 0; tupleIndex < tupleCount; tupleIndex++)
  {
    void *tuple = NULL;
    if (relType == "vector")
    {
      tuple = new Record(recordSize);
    }
    else
    {
      tuple = new Tuple(nl->Second(relTypeList));
    }

    for (int attrIndex = 0; attrIndex < attrCount; attrIndex++)
    {
      Attribute *attr = NULL;
      if (dataType == "bool")
      {
        attr = new CcBool(true, (tupleIndex + attrIndex) % dataParam2 == 0);
      }
      else if (dataType == "int")
      {
        attr = new CcInt(true, (tupleIndex + attrIndex) % dataParam2);
      }
      else if (dataType == "real")
      {
        attr = new CcReal(true, (tupleIndex + attrIndex) % dataParam2);
      }
      else if (dataType == "string")
      {
        char buff[dataParam + 1];
        memset(buff, ((tupleIndex + attrIndex) % dataParam2) ? 'a' : 'b',
            dataParam);
        memset(buff + dataParam, '\0', 1);
        attr = new CcString(true, buff);
      }
      else if (dataType == "text")
      {
        char buff[dataParam + 1];
        memset(buff, ((tupleIndex + attrIndex) % dataParam2) ? 'a' : 'b',
            dataParam);
        memset(buff + dataParam, '\0', 1);
        attr = new FText(true, buff);
      }

      if (relType == "vector")
      {
        if(!((Record*)tuple)->AppendElement(attr, recordAttrTypes[attrIndex],
            recordAttrNames[attrIndex]))
        {
          assert(false);
        }
        attr->DeleteIfAllowed();
      }
      else
      {
        ((Tuple*)tuple)->PutAttribute(attrIndex, attr);
      }
    }
    if (hasSubRel)
    {
      if (relType == "vector")
      {
        if (!((Record*)tuple)->AppendElement(subRel,
            recordAttrTypes[attrCount], recordAttrNames[attrCount]))
        {
          assert(false);
        }

      }
      else
      {
        ((Tuple*)tuple)->PutAttribute(attrCount, subRel->Clone());
      }

    }

    if (relType == NRel::BasicType())
    {
      ((NRel*) res)->AppendTuple((Tuple*)tuple);
    }
    else if (relType == ARel::BasicType())
    {
      ((ARel*) res)->AppendTuple((Tuple*)tuple);
    }
    else if (relType == "nrel")
    {
      ((NestedRelation*) res)->AppendTuple((Tuple*)tuple);
      ((Tuple*)tuple)->DeleteIfAllowed();
    }
    else if (relType == "arel")
    {
      ((AttributeRelation*) res)->getRel()->AppendTuple((Tuple*)tuple);
      ((AttributeRelation*) res)->Append(((Tuple*)tuple)->GetTupleId());
      ((Tuple*)tuple)->DeleteIfAllowed();
    }
    else if (relType == "vector")
    {
      ((collection::Collection*) res)->Insert((Record*)tuple, 1);
      ((Record*)tuple)->DeleteIfAllowed();
    }
  }
  if (relType == "vector")
  {
    ((collection::Collection*) res)->Finish();
    if(recordAttrNames){
       delete[] recordAttrNames;
    }
    if(recordAttrTypes){
      delete[] recordAttrTypes;
    }
  }

  result.setAddr(res);
  return 0;
}

/*
List of functions for cost estimation.

*/
CreateCostEstimation GenRel::costEstimators[] =
  { BlockingProgressEstimator::Build };
