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

#include "IndicesTI.h"

#include "IndicesTC.h"

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

bool IndicesTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "TypeInfo's length != 2.";
    return false;
  }

  if (!nl->IsEqual(nl->First(typeExpr), IndicesTC::name))
  {
    error = "TypeInfo's first element != " + IndicesTC::name + ".";
    return false;
  }

  const ListExpr capacity = nl->Second(typeExpr);
  if (!nl->IsNodeType(IntType, capacity) || !nl->IntValue(capacity) < 0)
  {
    error = "TypeInfo's second element isn't an int >= 0.";
    return false;
  }

  return true;
}

IndicesTI::IndicesTI() :
  m_capacity(0)
{
}

IndicesTI::IndicesTI(ListExpr typeExpr) :
  m_capacity(nl->IntValue(nl->Second(typeExpr)))
{
}

size_t IndicesTI::GetCapacity() const
{
  return m_capacity;
}

void IndicesTI::SetCapacity(size_t value)
{
  m_capacity = value;
}

ListExpr IndicesTI::GetTypeInfo() const
{
  return nl->TwoElemList(nl->SymbolAtom(IndicesTC::name),
                         nl->IntAtom(m_capacity));
}