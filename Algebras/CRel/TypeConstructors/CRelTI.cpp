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
#include "CRelTI.h"

#include "CRelTC.h"
#include "TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

bool CRelTI::Check(ListExpr typeExpr)
{
  string error;

  return Check(typeExpr, error);
}

bool CRelTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "TypeInfo's length != 2.";
    return false;
  }

  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    typeExpr = nl->Second(typeExpr);
  }

  if (!nl->IsEqual(nl->First(typeExpr), CRelTC::name))
  {
    error = "TypeInfo's first element != " + CRelTC::name + ".";
    return false;
  }

  ListExpr parameters = nl->Second(typeExpr);
  if (nl->IsAtom(parameters) || !nl->HasLength(parameters, 2))
  {
    error = "TypeInfo's second element isn't a three element list.";
    return false;
  }

  ListExpr cacheSizePara = nl->First(parameters);
  long cacheSize;

  if (!nl->IsNodeType(IntType, cacheSizePara) ||
      (cacheSize = nl->IntValue(cacheSizePara)) < 1)
  {
    error = "TypeInfo's first parameter (cache size) is not an int > 0.";
    return false;
  }

  return TBlockTI::Check(nl->Second(parameters), error);
}

CRelTI::CRelTI(bool numeric) :
  TBlockTI(numeric)
{
}

CRelTI::CRelTI(const TBlockTI &info, size_t cacheSize) :
  TBlockTI(info),
  m_cacheSize(cacheSize)
{
}

CRelTI::CRelTI(ListExpr typeExpr, bool numeric) :
  TBlockTI(numeric)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = CRelTI(nl->Second(typeExpr), numeric);
    return;
  }

  const ListExpr parameters = nl->Second(typeExpr);

  *this = CRelTI(TBlockTI(nl->Second(parameters), numeric),
                 nl->IntValue(nl->First(parameters)));
}

size_t CRelTI::GetCacheSize() const
{
  return m_cacheSize;
}

void CRelTI::SetCacheSize(size_t value)
{
  m_cacheSize = value;
}

ListExpr CRelTI::GetTypeExpr() const
{
  if (IsNumeric())
  {
    return nl->TwoElemList(GetNumericType(CRelTC::name),
                           nl->TwoElemList(nl->IntAtom(m_cacheSize),
                                           TBlockTI::GetTypeExpr()));
  }
  else
  {
    return nl->TwoElemList(nl->SymbolAtom(CRelTC::name),
                           nl->TwoElemList(nl->IntAtom(m_cacheSize),
                                           TBlockTI::GetTypeExpr()));
  }
}