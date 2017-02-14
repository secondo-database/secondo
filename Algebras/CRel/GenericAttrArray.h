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

#pragma once

#include "AlgebraTypes.h"
#include "AttrArray.h"
#include "Attribute.h"
#include <cstddef>
#include <iterator>
#include <limits>
#include "NestedList.h"
#include "ReadWrite.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include "Utility.h"
#include <vector>
#include <string>

class GenericAttrArray : public AttrArray
{
public:
  class Iterator
  {
  public:
    Iterator() :
      m_instance(NULL),
      m_index(0),
      m_count(0)
    {
    }

    Iterator(const GenericAttrArray &instance) :
      m_instance(&instance),
      m_index(0),
      m_count(instance.GetCount())
    {
    }

    bool IsValid() const
    {
      return m_index < m_count;
    }

    Attribute &GetAttribute()
    {
      return m_instance->GetAt(m_index);
    }

    bool MoveToNext()
    {
      if (IsValid())
      {
        ++m_index;

        return IsValid();
      }

      return false;
    }

    Attribute &operator * ()
    {
      return GetAttribute();
    }

    Iterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    bool operator == (const Iterator &other) const
    {
      if (IsValid())
      {
        if (other.IsValid())
        {
          return m_instance == other.m_instance &&
                 m_index == other.m_index;
        }

        return false;
      }

      return !other.IsValid();
    }

    bool operator != (const Iterator &other) const
    {
      return !(*this == other);
    }

  private:
    const GenericAttrArray *m_instance;

    size_t m_index,
      m_count;
  };

  class Info
  {
  public:
    int attributeAlgebraId,
      attributeTypeId;

    size_t attributeSize,
      attributeFLOBCount;

    InObject attributeInFunction;

    OutObject attributeOutFunction;

    ObjectCast attributeCastFunction;

    ListExpr attributeType;

    Info();

    Info(ListExpr attributeTypeExpr);
  };

  typedef Shared<const Info> PInfo;

  static void* Cast(void *pointer);


  GenericAttrArray(PInfo info);

  GenericAttrArray(PInfo info, SmiFileId flobFileId);

  GenericAttrArray(PInfo info, SmiFileId flobFileId,
                   Shared<SmiRecordFile> file);

  GenericAttrArray(PInfo info, Reader &source);

  GenericAttrArray(PInfo info, Reader &source, Shared<SmiRecordFile> file);

  virtual ~GenericAttrArray();


  const PInfo &GetInfo() const;

  virtual size_t GetCount() const;

  virtual size_t GetSize() const;

  virtual void Save(Writer &target) const;

  virtual void DeleteRecords();

  virtual void CloseFiles();

  void Clear();

  virtual void Append(const AttrArray &block, size_t row);

  virtual void Append(ListExpr value);

  virtual void Append(Attribute &value);

  virtual void Remove();

  Attribute &GetAt(size_t index) const;

  Attribute &operator[](size_t index) const
  {
    return GetAt(index);
  }

  virtual int Compare(size_t rowA, const AttrArray &blockB, size_t rowB) const;

  virtual int Compare(size_t row, Attribute &value) const;

  virtual size_t GetHash(size_t row) const;

  virtual ListExpr GetListExpr(size_t row) const;

  Iterator GetIterator() const;

  Iterator begin() const
  {
    return GetIterator();
  }

  Iterator end() const
  {
    return Iterator();
  }

private:
  class PersistentMembers
  {
  public:
    size_t count,
      flobRecordSize;

    SmiFileId flobFileId;

    SmiRecordId flobRecordId;

    PersistentMembers()
    {
    }

    PersistentMembers(size_t count, size_t flobRecordSize, SmiFileId flobFileId,
                      SmiRecordId flobRecordId) :
      count(count),
      flobRecordSize(flobRecordSize),
      flobFileId(flobFileId),
      flobRecordId(flobRecordId)
    {
    }

    PersistentMembers(Reader &source)
    {
      source.ReadOrThrow((char*)this, sizeof(PersistentMembers));
    }
  };

  static const char Undefined = 0,
    Persistent = 1,
    Casted = 2;

  PersistentMembers m_persistent;

  PInfo m_info;

  size_t m_capacity;
    //m_flobCapacity;

  Shared<SmiRecordFile> m_flobFile;

  char *m_attributes,
    *m_states;
    //*m_flobs;

  GenericAttrArray();
  GenericAttrArray(const GenericAttrArray &instance);

  char &GetState(size_t index) const;

  Attribute &GetAttribute(size_t index) const;
};