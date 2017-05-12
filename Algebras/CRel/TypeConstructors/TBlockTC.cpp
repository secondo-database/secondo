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
#include "AttrArray.h"
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "RelationAlgebra.h"
#include <set>
#include "Shared.h"
#include "SecondoCatalog.h"
#include "SecondoException.h"
#include "SecondoSystem.h"
#include "StringUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::set;
using std::string;
using stringutils::any2str;

extern NestedList *nl;
extern AlgebraManager *am;
extern CMsg cmsg;

//TBlockTI----------------------------------------------------------------------

const size_t TBlockTI::blockSizeFactor = 1024 * 1024; //MiB

bool TBlockTI::Check(ListExpr typeExpr)
{
  string error;

  return Check(typeExpr, error);
}

bool TBlockTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "TypeInfo's length != 2.";
    return false;
  }

  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    return Check(nl->Second(typeExpr), error);
  }

  if (!nl->IsEqual(nl->First(typeExpr), TBlockTC::name))
  {
    error = "TypeInfo's first element (type name) != " + TBlockTC::name + ".";
    return false;
  }

  const ListExpr parameters = nl->Second(typeExpr);

  if (!nl->HasLength(parameters, 2))
  {
    error = "TypeInfo's second element (parameters) isn't a two element list.";
    return false;
  }

  const ListExpr blockSizeExpr = nl->First(parameters);

  if (!nl->IsNodeType(IntType, blockSizeExpr))
  {
    error = "TypeInfo's first parameter (desired size) isn't a int.";
    return false;
  }

  ListExpr columnList = nl->Second(parameters);

  if (nl->IsEmpty(columnList) || nl->AtomType(columnList) != NoAtom)
  {
    error = "TypeInfo's second parameter (column list) is empty / no list.";
    return false;
  }

  ListExpr errorExpr = emptyErrorInfo();
  set<string> names;
  size_t i = 0;

  while (!nl->IsEmpty(columnList))
  {
    const ListExpr current = nl->First(columnList);

    columnList = nl->Rest(columnList);

    if (nl->ListLength(current) != 2)
    {
      error = "Column list element at " + any2str(i) +
              ": Element isn't a two element list.";
      return false;
    }

    const ListExpr nameExpr = nl->First(current);

    if(nl->AtomType(nameExpr) != SymbolType)
    {
      error = "Column list element at " + any2str(i) +
              ": Element's first element (name) isn't a symbol.";
      return false;
    }

    if (!isValidAttributeName(nameExpr, error))
    {
      return false;
    }

    if(!names.insert(nl->SymbolValue(nameExpr)).second)
    {
      error = "Column list element at " + any2str(i) +
              ": Element's first element (name) isn't unique.";
      return false;
    }

    if (!am->CheckKind(Kind::ATTRARRAY(), nl->Second(current), errorExpr))
    {
      error = "Column list element at " + any2str(i) +
              ": Element's second element (type) isn't of kind ATTRARRAY.";
      return false;
    }
  }

  return true;
}

TBlockTI::TBlockTI(bool numeric) :
  m_isNumeric(numeric),
  m_info(nullptr)
{
}

TBlockTI::TBlockTI(ListExpr typeExpr, bool numeric) :
  m_isNumeric(numeric),
  m_info(nullptr)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = TBlockTI(nl->Second(typeExpr), numeric);
    return;
  }

  const ListExpr parameters = nl->Second(typeExpr);

  m_desiredBlockSize = nl->IntValue(nl->First(parameters));

  AppendColumnInfos(nl->Second(parameters));
}

bool TBlockTI::IsNumeric() const
{
  return m_isNumeric;
}

size_t TBlockTI::GetDesiredBlockSize() const
{
  return m_desiredBlockSize;
}

void TBlockTI::SetDesiredBlockSize(size_t value)
{
  m_desiredBlockSize = value;
  m_info = nullptr;
}

void TBlockTI::AppendColumnInfos(ListExpr columnList)
{
  ListExpr columnListCopy = columnList;

  while (!nl->IsEmpty(columnListCopy))
  {
    const ListExpr columnExpr = nl->First(columnListCopy);

    ColumnInfo columnInfo;
    columnInfo.name = nl->SymbolValue(nl->First(columnExpr));
    columnInfo.type = nl->Second(columnExpr);

    columnInfos.push_back(columnInfo);

    columnListCopy = nl->Rest(columnListCopy);
  }

  m_info = nullptr;
}

ListExpr TBlockTI::GetColumnList() const
{
  ListExpr columnList = nl->Empty(),
    columnListEnd = columnList;

  for (const ColumnInfo &columnInfo : columnInfos)
  {
    if (nl->IsEmpty(columnList))
    {
      columnList =
        nl->OneElemList(nl->TwoElemList(nl->SymbolAtom(columnInfo.name),
                                        columnInfo.type));
      columnListEnd = columnList;
    }
    else
    {
      columnListEnd =
        nl->Append(columnListEnd,
                   nl->TwoElemList(nl->SymbolAtom(columnInfo.name),
                                   columnInfo.type));
    }
  }

  return columnList;
}

ListExpr TBlockTI::GetTypeExpr(bool stream) const
{
  const ListExpr type =
    nl->TwoElemList(m_isNumeric ? GetNumericType(TBlockTC::name) :
                                  nl->SymbolAtom(TBlockTC::name),
                    nl->TwoElemList(nl->IntAtom(m_desiredBlockSize),
                                    GetColumnList()));

  return stream ? nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), type) :
                  type;
}

ListExpr TBlockTI::GetTupleTypeExpr(bool stream) const
{
  ListExpr attributeList = nl->Empty(),
    attributeListEnd = attributeList;

  for (const ColumnInfo &columnInfo : columnInfos)
  {
    AttrArrayTypeConstructor &arrayConstructor =
      *AttrArray::GetTypeConstructor(columnInfo.type, false);

    const ListExpr columnName = nl->SymbolAtom(columnInfo.name),
      attributeType = arrayConstructor.GetAttributeType(columnInfo.type,
                                                        m_isNumeric);

    if (nl->IsEmpty(attributeList))
    {
      attributeList = nl->OneElemList(nl->TwoElemList(columnName,
                                                      attributeType));

      attributeListEnd = attributeList;
    }
    else
    {
      attributeListEnd = nl->Append(attributeListEnd,
                                    nl->TwoElemList(columnName,
                                                    attributeType));
    }
  }

  const ListExpr type =
    nl->TwoElemList(m_isNumeric ? GetNumericType(Tuple::BasicType()) :
                                  nl->SymbolAtom(Tuple::BasicType()),
                    attributeList);

  return stream ? nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()), type) :
                  type;
}

const PTBlockInfo &TBlockTI::GetBlockInfo() const
{
  if (m_info.IsNull())
  {
    SecondoCatalog &catalog = *SecondoSystem::GetCatalog();

    ListExpr columnTypes = nl->Empty(),
      columnTypesEnd = columnTypes;

    for (const ColumnInfo &columnInfo : columnInfos)
    {
      if (nl->IsEmpty(columnTypes))
      {
        columnTypes = nl->OneElemList(catalog.NumericType(columnInfo.type));
        columnTypesEnd = columnTypes;
      }
      else
      {
        columnTypesEnd = nl->Append(columnTypesEnd,
                                    catalog.NumericType(columnInfo.type));
      }
    }

    m_info = new TBlockInfo(columnTypes);
  }

  return m_info;
}

//TBlockTC----------------------------------------------------------------------

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

  for (const TBlockEntry &tuple : instance.GetFilter())
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

  for (const TBlockEntry &tuple : instance.GetFilter())
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