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

#include "TBlockTC.h"

#include "AlgebraManager.h"
#include "GenericAttrArray.h"
#include <exception>
#include "LogMsg.h"
#include <memory>
#include "SecondoSystem.h"
#include "StringUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using std::unique_ptr;
using stringutils::any2str;

extern NestedList *nl;
extern AlgebraManager *am;

const string TBlockTC::name = "tblock";

ListExpr TBlockTC::TypeProperty()
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

bool TBlockTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  string error;
  if (!TBlockTI::Check(typeExpr, error))
  {
    errorInfo = listutils::simpleMessage(error);
    return false;
  }

  return true;
}

Word TBlockTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                  ListExpr &errorInfo, bool &correct)
{
  correct = true;

  try
  {
    const TBlockTI typeInfo = TBlockTI(typeExpr);
    const size_t columnCount = typeInfo.attributeInfos.size();
    const TBlock::PInfo blockInfo = typeInfo.GetBlockInfo();

    unique_ptr<AttributeInFunction[]> inFunctions(
      new AttributeInFunction[columnCount]);

    for (size_t i = 0; i < columnCount; i++)
    {
      const ListExpr columnType = typeInfo.attributeInfos[i].type;

      int algebraId,
        typeId;

      ResolveTypeInfo(columnType, algebraId, typeId);

      inFunctions[i] = AttributeInFunction(am->InObj(algebraId, typeId),
                                          columnType);
    }

    SmiFileId fileId = SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

    TBlock *result(new TBlock(blockInfo, fileId, fileId));

    if (!nl->IsEmpty(value))
    {
      ListExpr tupleValues = value,
        error = nl->Empty();

      unique_ptr<Attribute*[]> tuple(new Attribute*[columnCount]);

      size_t tupleIndex = 0;

      do
      {
        ListExpr attributeValues = nl->First(tupleValues);
        tupleValues = nl->Rest(tupleValues);

        for (size_t i = 0; i < columnCount; ++i)
        {
          if (nl->IsEmpty(attributeValues))
          {
            ErrorReporter::ReportError("Not enough attributes in tuple " +
                                       any2str(tupleIndex) + ".");
            correct = false;
            return Word();
          }

          tuple[i] = inFunctions[i](nl->First(attributeValues), i, error,
                                    correct);

          attributeValues = nl->Rest(attributeValues);

          if (!correct)
          {
            ErrorReporter::ReportError("Invalid attribute-value in tuple" +
                                       any2str(tupleIndex) + ": " +
                                       nl->ToString(error));
            return Word();
          }
        }

        result->Append(tuple.get());

        for (size_t i = 0; i < columnCount; ++i)
        {
          tuple[i]->DeleteIfAllowed();
        }
      }
      while (!nl->IsEmpty(tupleValues));
    }

    return Word(result);
  }
  catch (const exception &e)
  {
    errorInfo = simpleMessage(e.what());
    correct = false;
    return Word();
  }
}

ListExpr TBlockTC::Out(ListExpr typeExpr, Word value)
{
  const TBlock &instance = *(TBlock*)value.addr;
  const size_t columnCount = instance.GetColumnCount();

  const ListExpr tupleExprList = nl->OneElemList(nl->Empty());
  ListExpr tupleExprListEnd = tupleExprList;

  for (const BlockTuple &tuple : instance)
  {
    const ListExpr tupleExpr = nl->OneElemList(nl->Empty());
    ListExpr tupleExprEnd = tupleExpr;

    for (size_t i = 0; i < columnCount; i++)
    {
      tupleExprEnd = nl->Append(tupleExprEnd, tuple[i].GetListExpr());
    }

    tupleExprListEnd = nl->Append(tupleExprListEnd, nl->Rest(tupleExpr));
  }

  return nl->Rest(tupleExprList);
}

Word TBlockTC::Create(const ListExpr typeExpr)
{
  const SmiFileId fileId =
    SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  return Word(new TBlock(TBlockTI(typeExpr).GetBlockInfo(), fileId, fileId));
}

void TBlockTC::Delete(const ListExpr, Word &value)
{
  TBlock &instance = *(TBlock*)value.addr;

  instance.DeleteRecords();
  instance.DecRef();

  value.addr = NULL;
}

bool TBlockTC::Open(SmiRecord &valueRecord, size_t &offset,
                    const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiReader source = SmiReader(valueRecord, offset);

    value = Word(new TBlock(TBlockTI(typeExpr).GetBlockInfo(), source));

    offset = source.GetPosition();

    return true;
  }
  catch (const exception &e)
  {
    value = Word();

    return false;
  }
}

bool TBlockTC::Save(SmiRecord &valueRecord, size_t &offset, const ListExpr,
                    Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    ((TBlock*)value.addr)->Save(target);

    offset = target.GetPosition();

    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

void TBlockTC::Close(const ListExpr, Word &value)
{
  ((TBlock*)value.addr)->DecRef();

  value.addr = NULL;
}

void *TBlockTC::Cast(void *addr)
{
  return TBlock::Cast(addr);
}

int TBlockTC::SizeOf()
{
  return sizeof(TBlock);
}

Word TBlockTC::Clone(const ListExpr, const Word &value)
{
  SmiFileId fileId = SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  TBlock &instance = *(TBlock*)value.addr,
    *clone = new TBlock(instance.GetInfo(), fileId, fileId);

  for (const BlockTuple &tuple : instance)
  {
    clone->Append(tuple);
  }

  return Word(clone);
}

TBlockTC::TBlockTC() :
  TypeConstructor(name, TypeProperty, Out, In, NULL, NULL, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}