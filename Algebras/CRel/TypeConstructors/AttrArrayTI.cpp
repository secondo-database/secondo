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

#include "AttrArrayTC.h"

#include <algorithm>
#include "AlgebraManager.h"
#include <exception>
#include "ListUtils.h"
#include <stdexcept>
#include "SecondoSMI.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "Utility.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::max;
using std::runtime_error;
using std::string;

extern NestedList *nl;
extern AlgebraManager *am;

bool AttrArrayTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "AttrArrayTI's length != 2.";
    return false;
  }

  const ListExpr firstArg = nl->First(typeExpr);
  if (nl->IsEqual(firstArg, Symbols::STREAM()))
  {
    return Check(nl->Second(typeExpr), error);
  }

  if (!nl->IsEqual(firstArg, AttrArrayTC::name))
  {
    error = "AttrArrayTI's first element != " + AttrArrayTC::name + ".";
    return false;
  }

  if (!isDATA(nl->Second(typeExpr)))
  {
    error = "AttrArrayTI's argument (attribute type) is not of kind DATA.";
    return false;
  }

  return true;
}

AttrArrayTI::AttrArrayTI() :
  m_info(NULL),
  m_attributeType(nl->Empty()),
  m_hasCapacity(false)
{
}

AttrArrayTI::AttrArrayTI(ListExpr typeExpr) :
  m_info(NULL)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = AttrArrayTI(nl->Second(typeExpr));
    return;
  }

  SetAttributeType(nl->Second(typeExpr));
}

ListExpr AttrArrayTI::GetAttributeType()
{
  return m_attributeType;
}

void AttrArrayTI::SetAttributeType(ListExpr value)
{
  m_attributeType = value;
  m_info = NULL;
}

GenericAttrArray::PInfo AttrArrayTI::GetPInfo()
{
  if (m_info.IsNull() && !nl->IsEmpty(m_attributeType))
  {
    m_info = new GenericAttrArray::Info(m_attributeType);
  }

  return m_info;
}

ListExpr AttrArrayTI::GetTypeInfo()
{
  return nl->TwoElemList(nl->SymbolAtom(AttrArrayTC::name),
                         m_hasCapacity ?
                          nl->TwoElemList(nl->IntAtom(m_capacity),
                                          m_attributeType) :
                          m_attributeType);
}