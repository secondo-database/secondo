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

#include "TBlockTI.h"

#include "AlgebraManager.h"
#include "AttrArray.h"
#include "ListUtils.h"
#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include <set>
#include "StringUtils.h"
#include "TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::set;
using std::string;
using stringutils::any2str;

extern AlgebraManager *am;
extern NestedList *nl;

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
      (AttrArrayTypeConstructor&)*GetTypeConstructor(columnInfo.type);

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