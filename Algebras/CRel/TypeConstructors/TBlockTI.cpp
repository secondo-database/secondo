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

#include "GenericAttrArray.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "StringUtils.h"
#include "TBlockTC.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::string;
using stringutils::any2str;

extern NestedList *nl;

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
    error = "TypeInfo's first element != " + TBlockTC::name + ".";
    return false;
  }

  ListExpr attributeList = nl->Second(typeExpr);
  if (!isAttrList(attributeList))
  {
    error = "TypeInfo's argument isn't a valid attribute list.";
    return false;
  }

  size_t attributeIndex = 0;
  while (!nl->IsEmpty(attributeList))
  {
    const ListExpr attribute = Take(attributeList);

    if (!isValidAttributeName(nl->First(attribute), error))
    {
      error = "Attribute name at position " + any2str(attributeIndex) +
              " isn't valid: " + error;
      return false;
    }

    ++attributeIndex;
  }

  return true;
}

TBlockTI::TBlockTI()
{
}

TBlockTI::TBlockTI(ListExpr typeExpr)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = TBlockTI(nl->Second(typeExpr));
    return;
  }

  AppendAttributeInfos(nl->Second(typeExpr));
}

void TBlockTI::AppendAttributeInfos(ListExpr attributeList)
{
  ListExpr attributeListCopy = attributeList;
  while (!nl->IsEmpty(attributeListCopy))
  {
    attributeInfos.push_back(GetAttributeInfo(Take(attributeListCopy)));
  }

  m_info = NULL;
}

ListExpr TBlockTI::GetTypeInfo() const
{
  ListExpr attributes = nl->Empty();

  size_t count = attributeInfos.size();

  if (count > 0)
  {
    attributes = nl->OneElemList(GetListExpr(attributeInfos[0]));

    ListExpr lastAttribute = attributes;

    for (size_t i = 1; i < count; ++i)
    {
      Append(lastAttribute, GetListExpr(attributeInfos[i]));
    }
  }

  return nl->TwoElemList(nl->SymbolAtom(TBlockTC::name), attributes);
}

TBlock::PInfo TBlockTI::GetBlockInfo() const
{
  if (m_info.IsNull())
  {
    SecondoCatalog &catalog = *SecondoSystem::GetCatalog();

    const ListExpr attributeTypes = nl->OneElemList(nl->Empty());

    ListExpr attributeTypesEnd = attributeTypes;

    for (const AttributeInfo &attributeInfo : attributeInfos)
    {
      attributeTypesEnd = nl->Append(attributeTypesEnd,
                                     catalog.NumericType(attributeInfo.type));
    }

    m_info = new TBlock::Info(nl->Rest(attributeTypes));
  }

  return m_info;
}

TBlockTI::AttributeInfo TBlockTI::GetAttributeInfo(ListExpr value)
{
  AttributeInfo attribute;
  attribute.name = nl->SymbolValue(nl->First(value));
  attribute.type = nl->Second(value);

  return attribute;
}

ListExpr TBlockTI::GetListExpr(const AttributeInfo &value)
{
  return nl->TwoElemList(nl->SymbolAtom(value.name),
                         value.type);
}