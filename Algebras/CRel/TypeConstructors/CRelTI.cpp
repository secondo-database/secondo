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

using namespace CRelAlgebra;

using std::string;

extern NestedList *nl;

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
  if (nl->IsAtom(parameters) || !nl->HasLength(parameters, 3))
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

  ListExpr desiredBlockSizeArg = nl->Second(parameters);
  long desiredBlockSize;

  if (!nl->IsNodeType(IntType, desiredBlockSizeArg) ||
      (desiredBlockSize = nl->IntValue(desiredBlockSizeArg)) < 0)
  {
    error = "TypeInfo's second parameter (block size) is not an int >= 0.";
    return false;
  }

  return TBlockTI::Check(nl->TwoElemList(nl->SymbolAtom(TBlockTC::name),
                                         nl->Third(parameters)),
                         error);
}

CRelTI::CRelTI()
{
}

CRelTI::CRelTI(const TBlockTI &info, size_t cacheSize, size_t blockSize) :
  TBlockTI(info),
  m_cacheSize(cacheSize),
  m_desiredBlockSize(blockSize)
{
}

CRelTI::CRelTI(ListExpr typeExpr)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = CRelTI(nl->Second(typeExpr));
    return;
  }

  ListExpr parameters = nl->Second(typeExpr);

  m_cacheSize = nl->IntValue(nl->First(parameters));
  m_desiredBlockSize = nl->IntValue(nl->Second(parameters));

  AppendAttributeInfos(nl->Third(parameters));
}

size_t CRelTI::GetCacheSize() const
{
  return m_cacheSize;
}

void CRelTI::SetCacheSize(size_t value)
{
  m_cacheSize = value;
}

size_t CRelTI::GetDesiredBlockSize() const
{
  return m_desiredBlockSize;
}

void CRelTI::SetDesiredBlockSize(size_t value)
{
  m_desiredBlockSize = value;
}

ListExpr CRelTI::GetTypeInfo() const
{
  ListExpr attributes = nl->Empty();

  const size_t count = attributeInfos.size();
  if (count > 0)
  {
    attributes = nl->OneElemList(GetListExpr(attributeInfos[0]));

    ListExpr lastAttribute = attributes;

    for (size_t i = 1; i < count; ++i)
    {
      Append(lastAttribute, GetListExpr(attributeInfos[i]));
    }
  }

  return nl->TwoElemList(nl->SymbolAtom(CRelTC::name),
                         nl->ThreeElemList(nl->IntAtom(m_cacheSize),
                                           nl->IntAtom(m_desiredBlockSize),
                                           attributes));
}