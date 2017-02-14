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
#include "CRelTC.h"

#include "AlgebraManager.h"
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include <memory>
#include "StringUtils.h"
#include "TBlock.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using std::unique_ptr;
using stringutils::any2str;

extern NestedList *nl;
extern AlgebraManager *am;

const string CRelTC::name = "crel";

ListExpr CRelTC::TypeProperty()
{
  return nl->TwoElemList(nl->FourElemList(nl->StringAtom("Signature"),
                                          nl->StringAtom("Example Type List"),
                                          nl->StringAtom("List Rep"),
                                          nl->StringAtom("Example List")),
                         nl->FourElemList(nl->StringAtom(""),
                                          nl->StringAtom(""),
                                          nl->TextAtom(""),
                                          nl->TextAtom("")));
}

bool CRelTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  string error;
  if (!CRelTI::Check(typeExpr, error))
  {
    errorInfo = listutils::simpleMessage(error);
    return false;
  }

  return true;
}

Word CRelTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                ListExpr &errorInfo, bool &correct)
{
  correct = true;

  try
  {
    const CRelTI typeInfo = CRelTI(typeExpr);
    const TBlock::PInfo blockInfo = typeInfo.GetBlockInfo();
    const size_t columnCount = blockInfo->columnCount;

    unique_ptr<AttributeInFunction[]> inFunctions(
      new AttributeInFunction[columnCount]);

    for (size_t i = 0; i < columnCount; i++)
    {
      const ListExpr type = blockInfo->columnTypes[i];

      int algebraId,
        typeId;

      ResolveTypeInfo(type, algebraId, typeId);

      inFunctions[i] = AttributeInFunction(am->InObj(algebraId, typeId), type);
    }

    CRel *relation = new CRel(blockInfo, typeInfo.GetDesiredBlockSize(),
                              typeInfo.GetCacheSize());

    ListExpr tupleValues = value;

    unique_ptr<Attribute*[]> tuple(new Attribute*[columnCount]);

    size_t index = 0;

    while (!nl->IsEmpty(tupleValues))
    {
      ListExpr attributeValues = nl->First(tupleValues);
      tupleValues = nl->Rest(tupleValues);

      for (size_t i = 0; i < columnCount; ++i)
      {
        if (nl->IsEmpty(attributeValues))
        {
          correct = false;
          ErrorReporter::ReportError("Not enough values in tuple " +
                                     any2str(index / columnCount) + '\n');
          return Word();
        }

        tuple[i] = inFunctions[i](nl->First(attributeValues), index, errorInfo,
                                  correct);

        attributeValues = nl->Rest(attributeValues);

        if (!correct)
        {
          ErrorReporter::ReportError(nl->ToString(errorInfo));
          return Word();
        }

        ++index;
      }

      relation->Append(tuple.get());

      for (size_t i = 0; i < columnCount; ++i)
      {
        tuple[i]->DeleteIfAllowed();
      }
    }

    return Word(relation);
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
    correct = false;
    return Word();
  }
}

ListExpr CRelTC::Out(ListExpr typeExpr, Word value)
{
  const CRel &instance = *(CRel*)value.addr;

  const size_t columnCount = instance.GetColumnCount();

  if (columnCount > 0)
  {
    const CRelTI typeInfo = CRelTI(typeExpr);

    const ListExpr tupleExprList = nl->OneElemList(nl->Empty());
      ListExpr tupleExprListEnd = tupleExprList;

    for (const BlockTuple &tuple : instance)
    {
      const ListExpr tupleExpr = nl->OneElemList(nl->Empty());
      ListExpr tupleExprEnd = tupleExpr;

      for (size_t i = 0; i < columnCount; ++i)
      {
        tupleExprEnd = nl->Append(tupleExprEnd, tuple[i].GetListExpr());
      }

      tupleExprListEnd = nl->Append(tupleExprListEnd, nl->Rest(tupleExpr));
    }

    return nl->Rest(tupleExprList);
  }

  return nl->Empty();
}

Word CRelTC::Create(const ListExpr typeExpr)
{
  const CRelTI typeInfo = CRelTI(typeExpr);

  return Word(new CRel(typeInfo.GetBlockInfo(), typeInfo.GetDesiredBlockSize(),
                       typeInfo.GetCacheSize()));
}

void CRelTC::Delete(const ListExpr, Word &value)
{
  CRel *instance = (CRel*)value.addr;

  instance->DeleteFiles();

  delete instance;

  value.addr = NULL;
}

bool CRelTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  try
  {
    const CRelTI typeInfo = CRelTI(typeExpr);

    SmiReader source = SmiReader(valueRecord, offset);

    value = Word(new CRel(typeInfo.GetBlockInfo(), typeInfo.GetCacheSize(),
                          source));

    offset = source.GetPosition();

    return true;
  }
  catch (const exception &e)
  {
    value = Word();

    return false;
  }
}

bool CRelTC::Save(SmiRecord &valueRecord, size_t &offset, const ListExpr,
                  Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    ((CRel*)value.addr)->Save(target);

    offset = target.GetPosition();

    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

void CRelTC::Close(const ListExpr, Word &value)
{
  delete (CRel*)value.addr;

  value.addr = NULL;
}

void *CRelTC::Cast(void *addr)
{
  return CRel::Cast(addr);
}

int CRelTC::SizeOf()
{
  return sizeof(CRel);
}

Word CRelTC::Clone(const ListExpr, const Word &value)
{
  CRel &instance = *(CRel*)value.addr,
    *clone = new CRel(instance.GetBlockInfo(), instance.GetDesiredBlockSize(),
                      instance.GetCacheSize());

  for (const BlockTuple &tuple : instance)
  {
    clone->Append(tuple);
  }

  return Word(clone);
}

CRelTC::CRelTC() :
  TypeConstructor(name, TypeProperty, Out, In, NULL, NULL, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}