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

#include "GAttrArrayTI.h"

#include "AlgebraManager.h"
#include "GAttrArrayTC.h"
#include "ListUtils.h"
#include "SecondoSystem.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using listutils::isDATA;
using listutils::isStream;
using std::string;

extern AlgebraManager *am;
extern NestedList *nl;

bool GAttrArrayTI::Check(ListExpr typeExpr)
{
  string error;

  return Check(typeExpr, error);
}

bool GAttrArrayTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "GAttrArrayTI's length != 2.";
    return false;
  }

  const ListExpr firstArg = nl->First(typeExpr);
  if (isStream(typeExpr))
  {
    return Check(GetStreamType(typeExpr), error);
  }

  if (!nl->IsEqual(firstArg, GAttrArrayTC::name))
  {
    error = "GAttrArrayTI's first element != " + GAttrArrayTC::name + ".";
    return false;
  }

  if (!isDATA(nl->Second(typeExpr)))
  {
    error = "GAttrArrayTI's argument (attribute type) is not of kind DATA.";
    return false;
  }

  return true;
}

GAttrArrayTI::GAttrArrayTI(bool numeric) :
  m_attributeType(nl->Empty()),
  m_isNumeric(numeric),
  m_info(nullptr)
{
}

GAttrArrayTI::GAttrArrayTI(ListExpr typeExpr, bool numeric) :
  m_attributeType(isStream(typeExpr) ?
                    nl->Second(GetStreamType(typeExpr)) :
                    nl->Second(typeExpr)),
  m_isNumeric(numeric),
  m_info(nullptr)
{
}

bool GAttrArrayTI::IsNumeric() const
{
  return m_isNumeric;
}

ListExpr GAttrArrayTI::GetAttributeType() const
{
  return m_attributeType;
}

void GAttrArrayTI::SetAttributeType(ListExpr value)
{
  m_attributeType = value;
  m_info = nullptr;
}

const PGAttrArrayInfo &GAttrArrayTI::GetPInfo() const
{
  ListExpr attributeType = m_attributeType;

  if (m_info.IsNull() && !nl->IsEmpty(attributeType))
  {
    if (!m_isNumeric)
    {
      attributeType = SecondoSystem::GetCatalog()->NumericType(attributeType);
    }

    m_info = new GAttrArrayInfo(attributeType);
  }

  return m_info;
}

ListExpr GAttrArrayTI::GetTypeExpr() const
{
  return nl->TwoElemList(m_isNumeric ? GetNumericType(GAttrArrayTC::name) :
                                       nl->SymbolAtom(GAttrArrayTC::name),
                         m_attributeType);
}