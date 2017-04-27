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

#include "TBlock.h"

#include "SmiUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

extern NestedList *nl;

//TBlock------------------------------------------------------------------------

size_t TBlock::GetSaveSize(size_t columnCount, bool includeHeader)
{
  size_t size = sizeof(size_t) + (columnCount * sizeof(SmiRecordId));

  if (includeHeader)
  {
    size += 2 * sizeof(SmiFileId);
  }

  return size;
}

TBlock::TBlock(const PTBlockInfo &info, SmiFileId columnFileId,
               SmiFileId flobFileId) :
  TBlock(info, columnFileId, flobFileId, Shared<SmiRecordFile>())
{
}

TBlock::TBlock(const PTBlockInfo &info, SmiFileId columnFileId,
               SmiFileId flobFileId, Shared<SmiRecordFile> columnFile) :
  m_header(0, 0, columnFileId, flobFileId),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(columnFileId != 0 ?
    (!columnFile.IsNull() ? columnFile : new SmiRecordFile(false)) : nullptr),
  m_refCount(1)
{
  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_recordIds[i] = 0;
    m_columns[i] = nullptr;
  }
}

TBlock::TBlock(const PTBlockInfo &info, Reader &source) :
  TBlock(info, source.ReadOrThrow<TBlockHeader>(), source)
{
}

TBlock::TBlock(const PTBlockInfo &info, Reader &source,
               Shared<SmiRecordFile> columnFile) :
  TBlock(info, source.ReadOrThrow<TBlockHeader>(), source, columnFile)
{
}

TBlock::TBlock(const PTBlockInfo &info, const TBlockHeader &header,
               Reader &source) :
  TBlock(info, header, source, Shared<SmiRecordFile>())
{
}

TBlock::TBlock(const PTBlockInfo &info, const TBlockHeader &header,
               Reader &source, Shared<SmiRecordFile> columnFile) :
  m_header(header),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(header.columnFileId != 0 ?
    (!columnFile.IsNull() ? columnFile : new SmiRecordFile(false)) : nullptr),
  m_refCount(1)
{
  source.ReadOrThrow((char*)m_recordIds, m_columnCount * sizeof(SmiRecordId));

  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_columns[i] = nullptr;
  }
}

TBlock::TBlock(const TBlock &instance, size_t *columnIndices,
               size_t columnCount) :
  m_header(instance.m_header),
  m_columnCount(columnCount),
  m_recordIds(new SmiRecordId[columnCount]),
  m_columns(new AttrArray*[columnCount]),
  m_columnFile(instance.m_columnFile),
  m_refCount(1)
{
  TBlockInfo *info = new TBlockInfo();
  info->columnCount = columnCount;
  info->columnTypes = new ListExpr[columnCount];
  info->columnFactories = new AttrArrayManager*[columnCount];

  for (size_t i = 0; i < columnCount; ++i)
  {
    AttrArray *column = instance.m_columns[columnIndices[i]];
    if (column != nullptr)
    {
      column->IncRef();
    }

    AttrArrayManager *columnManager =
      instance.m_info->columnFactories[columnIndices[i]];
    columnManager->IncRef();

    m_recordIds[i] = instance.m_recordIds[columnIndices[i]];
    m_columns[i] = column;

    info->columnTypes[i] = instance.m_info->columnTypes[columnIndices[i]];
    info->columnFactories[i] = columnManager;
  }

  m_info = info;
}

TBlock::~TBlock()
{
  for (size_t i = 0; i < m_columnCount; i++)
  {
    if (m_columns[i] != nullptr)
    {
      m_columns[i]->DecRef();
    }
  }

  delete[] m_columns;
  delete[] m_recordIds;
}

const PTBlockInfo &TBlock::GetInfo() const
{
  return m_info;
}

void TBlock::Save(Writer &target, bool includeHeader)
{
  if (m_columnFile->GetFileId() != m_header.columnFileId)
  {
    CloseOrThrow(*m_columnFile);
  }

  if (!m_columnFile->IsOpen())
  {
    OpenOrThrow(*m_columnFile, m_header.columnFileId);
  }

  if (includeHeader)
  {
    target.WriteOrThrow(m_header);
  }

  for (size_t i = 0; i < m_columnCount; i++)
  {
    SmiRecordId &recordId = m_recordIds[i];
    if (recordId == 0)
    {
      AppendOrThrow(*m_columnFile, recordId);
    }
  }

  target.WriteOrThrow((char*)m_recordIds, m_columnCount * sizeof(SmiRecordId));

  for (size_t i = 0; i < m_columnCount; i++)
  {
    AttrArray *column = m_columns[i];

    if (column != nullptr)
    {
      SmiRecord record;
      SelectOrThrow(*m_columnFile, m_recordIds[i], SmiFile::Update, record);

      SmiWriter columnTarget(record, 0);

      column->Save(columnTarget, false);
    }
  }
}

void TBlock::DeleteRecords()
{
  if (m_header.columnFileId != 0)
  {
    if (m_columnFile->GetFileId() != m_header.columnFileId)
    {
      CloseOrThrow(*m_columnFile);
    }

    if (!m_columnFile->IsOpen())
    {
      OpenOrThrow(*m_columnFile, m_header.columnFileId);
    }

    for (size_t i = 0; i < m_columnCount; i++)
    {
      if (m_columns[i] == nullptr)
      {
        SmiRecordId recordId = m_recordIds[i];

        if (recordId != 0)
        {
          SmiRecord record;
          SelectOrThrow(*m_columnFile, recordId, SmiFile::ReadOnly, record);

          SmiReader source(record, 0);

          AttrArray *column = m_info->columnFactories[i]->Load(source);
          column->DeleteRecords();
          column->DecRef();

          record.Finish();
          DeleteOrThrow(*m_columnFile, recordId);

          m_recordIds[i] = 0;
        }
      }
      else
      {
        m_columns[i]->DeleteRecords();

        if (m_recordIds[i] != 0)
        {
          DeleteOrThrow(*m_columnFile, m_recordIds[i]);

          m_recordIds[i] = 0;
        }
      }
    }
  }
  else if (m_header.flobFileId != 0)
  {
    for (size_t i = 0; i < m_columnCount; i++)
    {
      m_columns[i]->DeleteRecords();

      if (m_recordIds[i] != 0)
      {
        DeleteOrThrow(*m_columnFile, m_recordIds[i]);

        m_recordIds[i] = 0;
      }
    }
  }
}

size_t TBlock::GetColumnCount() const
{
  return m_columnCount;
}

size_t TBlock::GetRowCount() const
{
  return m_header.rowCount;
}

size_t TBlock::GetSize() const
{
  return m_header.size;
}

void TBlock::Append(const AttrArrayEntry* tuple)
{
  const size_t columnCount = m_columnCount;

  size_t size = 0;

  for (size_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(tuple[i]);

    size += array.GetSize();
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(Attribute** tuple)
{
  const size_t columnCount = m_columnCount;

  size_t size = 0;

  for (size_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(*tuple[i]);

    size += array.GetSize();
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(const TBlockEntry &tuple)
{
  const size_t columnCount = m_columnCount;

  size_t size = 0;

  for (size_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(tuple[i]);

    size += array.GetSize();
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(const Tuple &tuple)
{
  const size_t columnCount = m_columnCount;

  size_t size = 0;

  for (size_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(*tuple.GetAttribute(i));

    size += array.GetSize();
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

AttrArray &TBlock::GetAt(size_t index) const
{
  AttrArray *column = m_columns[index];

  if (column == nullptr)
  {
    SmiRecordId recordId = m_recordIds[index];

    if (recordId == 0)
    {
      column = m_info->columnFactories[index]->Create(m_header.flobFileId);
    }
    else
    {
      if (m_columnFile->GetFileId() != m_header.columnFileId)
      {
        CloseOrThrow(*m_columnFile);
      }

      if (!m_columnFile->IsOpen())
      {
        OpenOrThrow(*m_columnFile, m_header.columnFileId);
      }

      SmiRecord columnRecord;
      SelectOrThrow(*m_columnFile, recordId, SmiFile::ReadOnly, columnRecord);

      SmiReader source(columnRecord, 0);

      /*class RecordFileReader : public Reader
      {
      public:
        RecordFileReader(SmiRecordFile &file, SmiRecordId recordId,
                         size_t position) :
          m_file(file),
          m_recordId(recordId),
          m_position(position)
        {
        }

        virtual size_t GetPosition()
        {
          return m_position;
        }

        virtual void SetPosition(size_t position)
        {
          m_position = position;
        }

        virtual bool Read(char *target, size_t count)
        {
          size_t read;

          if (m_file.Read(m_recordId, target, count, m_position, read))
          {
            m_position += read;

            return read == count;
          }

          return false;
        }

      private:
        SmiRecordFile &m_file;

        SmiRecordId m_recordId;

        size_t m_position;
      } source = RecordFileReader(*m_columnFile, recordId, 0);*/

      column =  m_info->columnFactories[index]->Load(source, m_header);
    }

    m_columns[index] = column;
  }

  return *column;
}

AttrArray &TBlock::operator[](size_t index) const
{
  return GetAt(index);
}

TBlockIterator TBlock::GetIterator() const
{
  return TBlockIterator(this);
}

TBlockIterator TBlock::begin() const
{
  return GetIterator();
}

TBlockIterator TBlock::end() const
{
  return TBlockIterator();
}

void TBlock::IncRef() const
{
  ++m_refCount;
}

void TBlock::DecRef() const
{
  if (--m_refCount == 0)
  {
    delete this;
  }
}

size_t TBlock::GetRefCount() const
{
  return m_refCount;
}

//TBlockInfo------------------------------------------------------------------

TBlockInfo::TBlockInfo() :
  columnCount(0),
  columnTypes(nullptr),
  columnAttributeTypes(nullptr),
  columnFactories(nullptr)
{
}

TBlockInfo::TBlockInfo(ListExpr columnTypes) :
  columnCount(nl->ListLength(columnTypes)),
  columnTypes(new ListExpr[columnCount]),
  columnAttributeTypes(new ListExpr[columnCount]),
  columnFactories(new AttrArrayManager*[columnCount])
{
  ListExpr types = columnTypes;

  for (size_t i = 0; i < columnCount; ++i)
  {
    const ListExpr columnType = nl->First(types);

    types = nl->Rest(types);

    this->columnTypes[i] = columnType;

    AttrArrayTypeConstructor &arrayConstructor =
      (AttrArrayTypeConstructor&)*GetTypeConstructor(columnType);

    const ListExpr attributeType =
      arrayConstructor.GetAttributeType(columnType, true);

    columnAttributeTypes[i] = attributeType;
    columnFactories[i] = arrayConstructor.CreateManager(attributeType);
  }
}

TBlockInfo::~TBlockInfo()
{
  for (size_t i = 0; i < columnCount; ++i)
  {
    columnFactories[i]->DecRef();
  }

  if (columnTypes != nullptr)
  {
    delete[] columnTypes;
  }

  if (columnAttributeTypes != nullptr)
  {
    delete[] columnAttributeTypes;
  }

  if (columnFactories != nullptr)
  {
    delete[] columnFactories;
  }
}

//TBlockHeader-----------------------------------------------------

TBlockHeader::TBlockHeader()
{
}

TBlockHeader::TBlockHeader(size_t rowCount, size_t size, SmiFileId columnFileId,
                           SmiFileId flobFileId) :
  rowCount(rowCount),
  size(size),
  columnFileId(columnFileId),
  flobFileId(flobFileId)
{
}

TBlockHeader::operator AttrArrayHeader() const
{
  return AttrArrayHeader(rowCount, flobFileId);
}