/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#include "TupleBlockType.h"

#include "SecondoException.h"
#include <string>

using std::string;

TupleBlockType::TupleBlockType(int attributeCount, ListExpr* attributeTypeInfos,
                               int blockSize) : 
  m_attributeCount(attributeCount),
  m_capacity(0),
  m_tupleFlobCount(0),
  m_tupleCoreSize(0),
  m_tupleTotalSize(0),
  m_blockSize(blockSize),
  m_attributeTypes(NULL)
{
  const string errMsg("TupleType: Wrong list format! Line ");

  m_attributeTypes = new AttributeType[m_attributeCount];

  for (int i = 0; i < attributeCount; i++)
  {
    ListExpr current = attributeTypeInfos[i];

    if (nl->ListLength(current) != 2)
    { //( <attr_name> <attr_desc> )
      throw SecondoException(errMsg + int2Str(__LINE__));
    }

    //current = (name type ...)
    ListExpr type = nl->Second(current);

    int algId = 0, typeId = 0, clsSize = 0;
    if (nl->IsAtom(type))
    {
      throw SecondoException(errMsg + int2Str(__LINE__));
    }

    ListExpr t1 = nl->First(type);
    if (nl->IsAtom(t1)) //type = (t1 t2 ...)
    {
      if (nl->ListLength(type) < 2)
      {
        throw SecondoException(errMsg + int2Str(__LINE__));
      }
      //type = (algid typeid ...)
      algId = nl->IntValue(t1),
      typeId = nl->IntValue(nl->Second(type)),
      clsSize = (am->SizeOfObj(algId, typeId))();
    }
    else
    {
      if (nl->ListLength(t1) < 2)
      {
        throw SecondoException(errMsg + int2Str(__LINE__));
      }
      //t1 = ((algid typeid ...) ...)
      algId = nl->IntValue(nl->First(t1));
      typeId = nl->IntValue(nl->Second(t1));
      clsSize = (am->SizeOfObj(algId, typeId))();
    }

    int coreSize = 0;
    TypeConstructor *tc = am->GetTC(algId, typeId);

    int flobCount = tc->NumOfFLOBs();
    bool extStorage = false;
    if (tc->GetStorageType() == Attribute::Extension)
    {
      coreSize = sizeof(uint32_t);
      extStorage = true;
    }
    else if (tc->GetStorageType() == Attribute::Default)
    {
      coreSize = clsSize;
    }
    else
    {
      coreSize = tc->SerializedFixSize();
    }

    m_attributeTypes[i] = AttributeType(algId, typeId, flobCount,
                                        clsSize, coreSize, extStorage);

    m_tupleCoreSize += coreSize;
    m_tupleTotalSize += coreSize;
    m_tupleFlobCount += flobCount;
  }

  m_capacity = m_blockSize / m_tupleCoreSize;
}

TupleBlockType::~TupleBlockType()
{
  if (m_attributeTypes != NULL)
  {
    delete[] m_attributeTypes;
    m_attributeTypes = NULL;
  }
}

PTupleBlockType::PTupleBlockType() : 
  m_instance(NULL),
  m_refCount(NULL)
{
}

PTupleBlockType::PTupleBlockType(int attributeCount, 
                                 ListExpr* attributeTypeInfos, int blockSize) : 
  m_instance(new TupleBlockType(attributeCount, attributeTypeInfos, blockSize)),
  m_refCount(new int(1))
{
}

PTupleBlockType::PTupleBlockType(TupleBlockType *instance) : 
  m_instance(instance),
  m_refCount(instance != NULL ? new int(1) : NULL)
{
}

PTupleBlockType::PTupleBlockType(const PTupleBlockType &instance) : 
  m_instance(instance.m_instance),
  m_refCount(instance.m_refCount)
{
  (*m_refCount)++;
}

PTupleBlockType::~PTupleBlockType()
{
  if (m_instance != NULL)
  {
    (*m_refCount)--;

    if (*m_refCount == 0)
    {
      delete m_instance;
      delete m_refCount;
    }
  }
}