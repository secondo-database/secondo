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

#include "AlgebraTypes.h"
#include "AlgebraManager.h"
#include <exception>
#include "LogMsg.h"
#include "Shared.h"
#include "SecondoException.h"
#include "SecondoSystem.h"
#include "StringUtils.h"
#include "TBlockTI.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using stringutils::any2str;

extern NestedList *nl;
extern AlgebraManager *am;
extern CMsg cmsg;

const string TBlockTC::name = "tblock";

ListExpr TBlockTC::TypeProperty()
{
  return ConstructorInfo(name,
    "int x (string DATA)* -> " + name,
    "(" + name + " (1000 ((Name string)(Age int))))",
    "((<attr1> ... <attrn>)*)",
    "((\"Myers\" 53)(\"Smith\" 21))",
    "The int parameter defines the desired size.").list();
}

bool TBlockTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  string error;
  if (!TBlockTI::Check(typeExpr, error))
  {
    cmsg.typeError(error);
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
    if (nl->IsAtom(value))
    {
      throw SecondoException("Value isn't a list of tuples.");
    }

    const PTBlockInfo blockInfo = TBlockTI(typeExpr, true).GetBlockInfo();
    const size_t columnCount = blockInfo->columnCount;

    SharedArray<InObject> inFunctions(columnCount);
    SharedArray<ListExpr> attributeTypes(columnCount);

    for (size_t i = 0; i < columnCount; i++)
    {
      const ListExpr columnType = blockInfo->columnAttributeTypes[i];

      int algebraId,
        typeId;

      ResolveTypeOrThrow(columnType, algebraId, typeId);

      inFunctions[i] = am->InObj(algebraId, typeId);
      attributeTypes[i] = columnType;
    }

    SmiFileId fileId = SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

    TBlock *result(new TBlock(blockInfo, fileId, fileId));

    if (!nl->IsEmpty(value))
    {
      ListExpr tupleValues = value;

      SharedArray<Attribute*> tuple(columnCount);

      size_t tupleIndex = 0;

      do
      {
        ListExpr attributeValues = nl->First(tupleValues);

        if (nl->IsAtom(attributeValues))
        {
          result->DecRef();
          throw SecondoException("Tuple value isn't a list of attributes.");
        }

        for (size_t i = 0; i < columnCount; ++i)
        {
          if (nl->IsEmpty(attributeValues))
          {
            for (size_t j = 0; j < i; ++j)
            {
              tuple[j]->DeleteIfAllowed();
            }

            result->DecRef();
            throw SecondoException("Not enough attributes in tuple value " +
                                   any2str(tupleIndex) + ".");
          }

          tuple[i] = (Attribute*)inFunctions[i](attributeTypes[i],
                                                nl->First(attributeValues), i,
                                                errorInfo, correct).addr;

          attributeValues = nl->Rest(attributeValues);

          if (!correct)
          {
            for (size_t j = 0; j < i; ++j)
            {
              tuple[j]->DeleteIfAllowed();
            }

            if (tuple[i] != nullptr)
            {
              tuple[i]->DeleteIfAllowed();
            }

            result->DecRef();
            throw SecondoException("Invalid attribute-value in tuple" +
                                   any2str(tupleIndex) + ": " +
                                   nl->ToString(errorInfo));
          }
        }

        result->Append(tuple.GetPointer());

        for (size_t i = 0; i < columnCount; ++i)
        {
          tuple[i]->DeleteIfAllowed();
        }
      }
      while (!nl->IsEmpty(tupleValues = nl->Rest(tupleValues)));
    }

    return Word(result);
  }
  catch (const exception &e)
  {
    cmsg.inFunError(e.what());
    correct = false;
    return Word();
  }
}

ListExpr TBlockTC::Out(ListExpr typeExpr, Word value)
{
  const TBlock &instance = *(TBlock*)value.addr;
  const size_t columnCount = instance.GetColumnCount();

  OutObject *attrOuts = new OutObject[columnCount];

  const ListExpr *attrTypes = instance.GetInfo()->columnAttributeTypes;

  for (size_t i = 0; i < columnCount; ++i)
  {
    attrOuts[i] = GetOutFunction(attrTypes[i]);
  }

  const ListExpr tupleExprList = nl->OneElemList(nl->Empty());
  ListExpr tupleExprListEnd = tupleExprList;

  for (const TBlockEntry &tuple : instance)
  {
    const ListExpr tupleExpr = nl->OneElemList(nl->Empty());
    ListExpr tupleExprEnd = tupleExpr;

    for (size_t i = 0; i < columnCount; i++)
    {
      Attribute *attr = tuple[i].GetAttribute();

      tupleExprEnd = nl->Append(tupleExprEnd, attrOuts[i](attrTypes[i], attr));

      attr->DeleteIfAllowed();
    }

    tupleExprListEnd = nl->Append(tupleExprListEnd, nl->Rest(tupleExpr));
  }

  delete[] attrOuts;

  return nl->Rest(tupleExprList);
}

Word TBlockTC::Create(const ListExpr typeExpr)
{
  const SmiFileId fileId =
    SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  return Word(new TBlock(TBlockTI(typeExpr, true).GetBlockInfo(), fileId,
                         fileId));
}

void TBlockTC::Delete(const ListExpr, Word &value)
{
  TBlock &instance = *(TBlock*)value.addr;

  instance.DeleteRecords();
  instance.DecRef();

  value.addr = nullptr;
}

bool TBlockTC::Open(SmiRecord &valueRecord, size_t &offset,
                    const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiReader source = SmiReader(valueRecord, offset);

    value = Word(new TBlock(TBlockTI(typeExpr, true).GetBlockInfo(), source));

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

  value.addr = nullptr;
}

void *TBlockTC::Cast(void *addr)
{
  return nullptr;
}

int TBlockTC::SizeOf()
{
  return 0;
}

Word TBlockTC::Clone(const ListExpr, const Word &value)
{
  SmiFileId fileId = SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  TBlock &instance = *(TBlock*)value.addr,
    *clone = new TBlock(instance.GetInfo(), fileId, fileId);

  for (const TBlockEntry &tuple : instance)
  {
    clone->Append(tuple);
  }

  return Word(clone);
}

TBlockTC::TBlockTC() :
  TypeConstructor(name, TypeProperty, Out, In, nullptr, nullptr, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}