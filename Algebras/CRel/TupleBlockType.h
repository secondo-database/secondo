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

#ifndef TUPLEBLOCKTYPE
#define TUPLEBLOCKTYPE

#include "RelationAlgebra.h"

class TupleBlockType
{
public:
  TupleBlockType(int attributeCount, ListExpr* attributeTypeInfos, 
                 int blockSize);
  ~TupleBlockType();

  inline int GetAttributeCount() const
  {
    return m_attributeCount;
  }

  inline const AttributeType &GetAttributeType(int index) const
  {
    return m_attributeTypes[index];
  }

  inline int GetCapacity() const
  {
    return m_capacity;
  }

  inline int GetBlockSize() const
  {
    return m_blockSize;
  }

private:
  int m_attributeCount,
      m_capacity,
      m_tupleFlobCount,
      m_tupleCoreSize,
      m_tupleTotalSize,
      m_blockSize;

  AttributeType *m_attributeTypes;
};

struct PTupleBlockType
{
public:
  PTupleBlockType();
  PTupleBlockType(int attributeCount, ListExpr* attributeTypeInfos,
                  int blockSize);
  PTupleBlockType(TupleBlockType *instance);
  PTupleBlockType(const PTupleBlockType &instance);
  ~PTupleBlockType();

  inline TupleBlockType *GetPointer() const
  {
    return m_instance;
  }

  inline PTupleBlockType &operator=(PTupleBlockType instance)
  {
    TupleBlockType *swapInstance = m_instance;
    int *swapRefCount = m_refCount;

    m_instance = instance.m_instance;
    m_refCount = instance.m_refCount;

    instance.m_instance = swapInstance;
    instance.m_refCount = swapRefCount;

    return *this;
  }

  inline TupleBlockType *operator->()
  {
    return m_instance;
  }

private:
  TupleBlockType *m_instance;

  int *m_refCount;
};

#endif