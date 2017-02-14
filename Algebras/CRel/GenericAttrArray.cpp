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

#include "GenericAttrArray.h"

#include "AlgebraManager.h"
#include <algorithm>
#include <cstring>
#include "SecondoSystem.h"
#include <stdexcept>
#include <string>
#include "StringUtils.h"
#include "TypeConstructor.h"

using std::invalid_argument;
using std::logic_error;
using std::max;
using std::runtime_error;
using std::string;
using std::vector;
using stringutils::any2str;

extern AlgebraManager *am;

enum FlobMode
{
  SingleRecord,
  MultipleRecords,
  Contained
};

const FlobMode flobMode = MultipleRecords;
const size_t containFlobSizeLimit = 100000000;

GenericAttrArray::Info::Info()
{
}

GenericAttrArray::Info::Info(ListExpr attributeTypeExpr) :
  attributeType(attributeTypeExpr)
{
  if (!ResolveTypeInfo(attributeTypeExpr, attributeAlgebraId, attributeTypeId))
  {
    throw runtime_error("");
  }

  TypeConstructor &typeConstructor = *am->GetTC(attributeAlgebraId,
                                                attributeTypeId);

  attributeSize =  typeConstructor.SizeOf();

  attributeInFunction = am->InObj(attributeAlgebraId, attributeTypeId);

  attributeOutFunction = am->OutObj(attributeAlgebraId, attributeTypeId);

  attributeCastFunction = am->Cast(attributeAlgebraId, attributeTypeId);

  attributeFLOBCount = max(typeConstructor.NumOfFLOBs(), 0);
}

void* GenericAttrArray::Cast(void *pointer)
{
  return new (pointer) GenericAttrArray;
}

GenericAttrArray::GenericAttrArray(PInfo info) :
  m_persistent(0, 0, 0, 0),
  m_info(info),
  m_capacity(0),
  m_attributes(NULL),
  m_states(NULL)
{
}

GenericAttrArray::GenericAttrArray(PInfo info, SmiFileId flobFileId) :
  m_persistent(0, 0, flobFileId, 0),
  m_info(info),
  m_capacity(0),
  m_flobFile(flobFileId != 0 ? new SmiRecordFile(false) : NULL),
  m_attributes(NULL),
  m_states(NULL)
{
}

GenericAttrArray::GenericAttrArray(PInfo info, SmiFileId flobFileId,
                                   Shared<SmiRecordFile> file) :
  m_persistent(0, 0, flobFileId, 0),
  m_info(info),
  m_capacity(0),
  m_flobFile(flobFileId != 0 ? file : NULL),
  m_attributes(NULL),
  m_states(NULL)
{
}

GenericAttrArray::GenericAttrArray(PInfo info, Reader &source) :
  m_persistent(source),
  m_info(info),
  m_capacity(m_persistent.count),
  m_flobFile(m_persistent.flobFileId != 0 ? new SmiRecordFile(false) : NULL),
  m_attributes(m_persistent.count > 0 ?
               new char[info->attributeSize * m_persistent.count] : NULL),
  m_states(m_persistent.count > 0 ? new char[m_persistent.count] : NULL)
{
  if (m_persistent.count > 0)
  {
    source.ReadOrThrow(m_attributes, info->attributeSize * m_persistent.count);

    char *state = m_states;
    for (size_t i = 0; i < m_persistent.count; i++)
    {
      *state = Persistent;
      state++;
    }
  }
}

GenericAttrArray::GenericAttrArray(PInfo info, Reader &source,
                                   Shared<SmiRecordFile> file) :
  m_persistent(source),
  m_info(info),
  m_capacity(m_persistent.count),
  m_flobFile(m_persistent.flobFileId != 0 ? file : NULL),
  m_attributes(m_persistent.count > 0 ?
               new char[info->attributeSize * m_persistent.count] : NULL),
  m_states(m_persistent.count > 0 ? new char[m_persistent.count] : NULL)
{
  if (m_persistent.count > 0)
  {
    source.ReadOrThrow(m_attributes, info->attributeSize * m_persistent.count);

    char *state = m_states;
    for (size_t i = 0; i < m_persistent.count; i++)
    {
      *state = Persistent;
      state++;
    }
  }
}

GenericAttrArray::~GenericAttrArray()
{
  CloseFiles();

  if (m_capacity > 0)
  {
    delete[] m_states;
    delete[] m_attributes;
  }
}

const GenericAttrArray::PInfo &GenericAttrArray::GetInfo() const
{
  return m_info;
}

size_t GenericAttrArray::GetCount() const
{
  return m_persistent.count;
}

size_t GenericAttrArray::GetSize() const
{
  return sizeof(GenericAttrArray) +
         (m_persistent.count * (m_info->attributeSize + sizeof(char)));
}

void GenericAttrArray::Save(Writer &target) const
{
  target.WriteOrThrow(m_persistent);
  target.WriteOrThrow(m_attributes, m_persistent.count * m_info->attributeSize);
}

void GenericAttrArray::DeleteRecords()
{
  switch (flobMode)
  {
  case FlobMode::SingleRecord:
  {
    if (m_persistent.flobRecordId != 0)
    {
      if (m_flobFile->GetFileId() != m_persistent.flobFileId)
      {
        CloseOrThrow(*m_flobFile);
      }

      if (!m_flobFile->IsOpen())
      {
        OpenOrThrow(*m_flobFile, m_persistent.flobFileId);
      }

      DeleteOrThrow(*m_flobFile, m_persistent.flobRecordId);

      m_persistent.flobRecordId = 0;
    }
    break;
  }
  case FlobMode::MultipleRecords:
  {
    if (m_persistent.flobFileId != 0 && m_info->attributeFLOBCount > 0)
    {
      for (size_t i = 0; i < m_persistent.count; ++i)
      {
        GetAttribute(i).DestroyFlobs();
      }
    }
    break;
  }
  case FlobMode::Contained:
  {
    break;
  }
  }
}

void GenericAttrArray::CloseFiles()
{
  switch (flobMode)
  {
  case FlobMode::SingleRecord:
  {
    if (m_flobFile->IsOpen())
    {
      CloseOrThrow(*m_flobFile);
    }
    break;
  }
  }
}

void GenericAttrArray::Clear()
{
  switch (flobMode)
  {
  case FlobMode::SingleRecord:
  {
    if (m_persistent.count > 0)
    {
      m_persistent.count = 0;

      if (m_persistent.flobRecordId != 0)
      {
        if (m_flobFile->GetFileId() != m_persistent.flobFileId)
        {
          CloseOrThrow(*m_flobFile);
        }

        if (!m_flobFile->IsOpen())
        {
          OpenOrThrow(*m_flobFile, m_persistent.flobFileId);
        }

        SmiRecord record;
        SelectOrThrow(*m_flobFile, m_persistent.flobRecordId, SmiFile::Update,
                      record);

        if (!record.Truncate(0))
        {
          throw runtime_error("Failed to truncate FLOB record. fileId: " +
                              stringutils::any2str(m_flobFile->GetFileId()) +
                              ", recordId: " +
                              stringutils::any2str(m_persistent.flobRecordId));
        }
      }
    }
    break;
  }
  case FlobMode::MultipleRecords:
  {
    if (m_persistent.count > 0)
    {
      DeleteRecords();
      m_persistent.count = 0;
    }
    break;
  }
  }
}

void GenericAttrArray::Append(const AttrArray &block, size_t row)
{
  Append(((const GenericAttrArray&)block)[row]);
}

void GenericAttrArray::Append(ListExpr value)
{
  ListExpr errorInfo = nl->Empty();
  bool correct = true;

  Word attribute = m_info->attributeInFunction(m_info->attributeType, value,
                                               m_persistent.count, errorInfo,
                                               correct);

  if (!correct)
  {
    throw runtime_error(nl->ToString(errorInfo));
  }

  Append(*(Attribute*)attribute.addr);

  ((Attribute*)attribute.addr)->DeleteIfAllowed();
}

void GenericAttrArray::Append(Attribute &attribute)
{
  if (m_capacity == m_persistent.count)
  {
    if (m_capacity == 0)
    {
      m_capacity = 1;
      m_states = new char[1];
      m_attributes = new char[m_info->attributeSize];
    }
    else
    {
      size_t capacity = m_capacity * 2;

      char *states = new char[capacity],
        *attributes = new char[capacity * m_info->attributeSize];

      memcpy(states, m_states, m_capacity);
      memcpy(attributes, m_attributes, m_capacity * m_info->attributeSize);

      delete[] m_states;
      delete[] m_attributes;

      m_capacity = capacity;
      m_states = states;
      m_attributes = attributes;
    }
  }

  GetState(m_persistent.count) = Casted;
  Attribute &storedAttribute(GetAttribute(m_persistent.count));

  memcpy((char*)&storedAttribute, (char*)&attribute, m_info->attributeSize);

  switch (flobMode)
  {
  case FlobMode::SingleRecord:
  {
    if (m_persistent.flobFileId != 0)
    {
      const size_t flobCount = m_info->attributeFLOBCount;

      if (flobCount > 0)
      {
        if (m_flobFile->GetFileId() != m_persistent.flobFileId)
        {
          CloseOrThrow(*m_flobFile);
        }

        if (!m_flobFile->IsOpen())
        {
          OpenOrThrow(*m_flobFile, m_persistent.flobFileId);

          if (m_persistent.flobRecordId == 0)
          {
            AppendOrThrow(*m_flobFile, m_persistent.flobRecordId);
          }
        }

        size_t flobOffset = m_persistent.flobRecordSize;

        const SmiRecordId flobRecordId = m_persistent.flobRecordId;

        for (size_t j = 0; j < flobCount; j++)
        {
          Flob &sourceFlob = *attribute.GetFLOB(j),
            *targetFlob = storedAttribute.GetFLOB(j);

          new (targetFlob) Flob(0);

          sourceFlob.saveToFile(m_flobFile.GetPointer(), flobRecordId,
                                flobOffset, *targetFlob);

          flobOffset += targetFlob->getSize();
        }

        m_persistent.flobRecordSize = flobOffset;
      }
    }

    break;
  }
  case FlobMode::MultipleRecords:
  {
    if (m_persistent.flobFileId != 0)
    {
      size_t flobCount = m_info->attributeFLOBCount;

      if (flobCount > 0)
      {
        for (size_t j = 0; j < flobCount; j++)
        {
          Flob &sourceFlob = *attribute.GetFLOB(j),
            *targetFlob = storedAttribute.GetFLOB(j);

          new (targetFlob) Flob(0);

          sourceFlob.saveToFile(m_persistent.flobFileId, 0, *targetFlob);
        }
      }
    }
    break;
  }
  /*case FlobMode::Contained:
  {
    const size_t flobCount = m_info->attributeFLOBCount;

    if (flobCount > 0)
    {
      for (size_t j = 0; j < flobCount; j++)
      {
        Flob *targetFlob = storedAttribute.GetFLOB(j);

        const size_t flobSize = targetFlob->getSize();

        if (flobSize <= containFlobSizeLimit)
        {
          targetFlob->bringToMemory();
        }
        else
        {
          Flob &sourceFlob = *attribute.GetFLOB(j);

          new (targetFlob) Flob(0);

          sourceFlob.saveToFile(m_persistent.flobFileId, 0, *targetFlob);
        }
      }
    }
  }*/
  }

  while (storedAttribute.NoRefs() > 1)
  {
    storedAttribute.DeleteIfAllowed();
  }

  ++m_persistent.count;
}

void GenericAttrArray::Remove()
{
  --m_persistent.count;

  if (m_persistent.flobFileId != 0 && m_info->attributeFLOBCount > 0)
  {
    GetAttribute(m_persistent.count).DestroyFlobs();
  }
}

Attribute &GenericAttrArray::GetAt(size_t index) const
{
  char &state = GetState(index);
  Attribute &attribute = GetAttribute(index);

  if (state == Persistent)
  {
    m_info->attributeCastFunction(&attribute);
    state = Casted;
  }

  return attribute;
}

int GenericAttrArray::Compare(size_t rowA, const AttrArray &blockB,
                              size_t rowB) const
{
  return GetAt(rowA).Compare(&((const GenericAttrArray&)blockB).GetAt(rowB));
}

int GenericAttrArray::Compare(size_t row, Attribute &value) const
{
  return GetAt(row).Compare(&value);
}

size_t GenericAttrArray::GetHash(size_t row) const
{
  return GetAt(row).HashValue();
}

size_t GenericAttrArray::GetListExpr(size_t row) const
{
  return m_info->attributeOutFunction(m_info->attributeType, &GetAt(row));
}

GenericAttrArray::Iterator GenericAttrArray::GetIterator() const
{
  return Iterator(*this);
}

GenericAttrArray::GenericAttrArray()
{
}

GenericAttrArray::GenericAttrArray(const GenericAttrArray &instance)
{
  throw runtime_error("GenericAttrArray's default copy-constructor should never"
                      " be called.");
}

char &GenericAttrArray::GetState(size_t index) const
{
  return *(m_states + index);
}

Attribute &GenericAttrArray::GetAttribute(size_t index) const
{
  return *(Attribute*)(m_attributes + (index * m_info->attributeSize));
}