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

#include <stdexcept>
#include "StringUtils.h"
#include "Utility.h"

using std::invalid_argument;
using std::runtime_error;
using std::string;
using std::vector;
using stringutils::any2str;

extern NestedList *nl;

//TBlock------------------------------------------------------------------------

TBlock::Info::Info() :
  columnCount(0),
  columnTypes(NULL),
  columnFactories(NULL)
{
}

TBlock::Info::Info(ListExpr columnTypes) :
  columnCount(nl->ListLength(columnTypes)),
  columnTypes(new ListExpr[columnCount]),
  columnFactories(new AttrArrayFactory*[columnCount])
{
  AttrArrayCatalog &catalog = AttrArrayCatalog::GetInstance();

  ListExpr types = columnTypes;

  for (size_t i = 0; i < columnCount; ++i)
  {
    this->columnTypes[i] = nl->First(types);
    columnFactories[i] = catalog.CreateFactory(this->columnTypes[i]);

    types = nl->Rest(types);
  }
}

TBlock::Info::~Info()
{
  for (size_t i = 0; i < columnCount; ++i)
  {
    columnFactories[i]->DecRef();
  }

  delete[] columnTypes;
  delete[] columnFactories;
}

void* TBlock::Cast(void *pointer)
{
  return new (pointer) TBlock;
}

size_t TBlock::GetSaveSize(size_t columnCount)
{
  return sizeof(size_t) + (2* sizeof(SmiFileId)) +
         (columnCount * sizeof(SmiRecordId));
}

TBlock::TBlock(const PInfo &info, SmiFileId columnFileId,
               SmiFileId flobFileId) :
  m_persistent(0, columnFileId, flobFileId),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(columnFileId != 0 ? new SmiRecordFile(false) : NULL),
  m_flobFile(flobFileId != 0 ? new SmiRecordFile(false) : NULL)
{
  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_recordIds[i] = 0;
    m_columns[i] = NULL;
  }
}

TBlock::TBlock(const PInfo &info, SmiFileId columnFileId, SmiFileId flobFileId,
               Shared<SmiRecordFile> columnFile,
               Shared<SmiRecordFile> flobFile) :
  m_persistent(0, columnFileId, flobFileId),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(columnFileId != 0 ? columnFile : NULL),
  m_flobFile(flobFileId != 0 ? flobFile : NULL)
{
  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_recordIds[i] = 0;
    m_columns[i] = NULL;
  }
}

TBlock::TBlock(const PInfo &info, Reader &source) :
  m_persistent(source),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(m_persistent.columnFileId != 0 ? new SmiRecordFile(false) :
                                                NULL),
  m_flobFile(m_persistent.flobFileId != 0 ? new SmiRecordFile(false) : NULL)
{
  source.ReadOrThrow((char*)m_recordIds, m_columnCount * sizeof(SmiRecordId));

  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_columns[i] = NULL;
  }
}

TBlock::TBlock(const PInfo &info, Reader &source,
               Shared<SmiRecordFile> columnFile,
               Shared<SmiRecordFile> flobFile) :
  m_persistent(source),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_recordIds(new SmiRecordId[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(m_persistent.columnFileId != 0 ? columnFile : NULL),
  m_flobFile(m_persistent.flobFileId != 0 ? flobFile : NULL)
{
  source.ReadOrThrow((char*)m_recordIds, m_columnCount * sizeof(SmiRecordId));

  for (size_t i = 0; i < m_columnCount; i++)
  {
    m_columns[i] = NULL;
  }
}

TBlock::TBlock(const TBlock &instance, size_t *columnIndices,
               size_t columnCount) :
  m_persistent(instance.m_persistent),
  m_columnCount(columnCount),
  m_recordIds(new SmiRecordId[columnCount]),
  m_columns(new AttrArray*[columnCount]),
  m_columnFile(instance.m_columnFile),
  m_flobFile(instance.m_flobFile)
{
  Info *info = new Info();
  info->columnCount = columnCount;
  info->columnTypes = new ListExpr[columnCount];
  info->columnFactories = new AttrArrayFactory*[columnCount];

  for (size_t i = 0; i < columnCount; ++i)
  {
    AttrArray *column = instance.m_columns[columnIndices[i]];
    if (column != NULL)
    {
      column->AddRef();
    }

    AttrArrayFactory *columnFactory =
      instance.m_info->columnFactories[columnIndices[i]];
    columnFactory->AddRef();

    m_recordIds[i] = instance.m_recordIds[columnIndices[i]];
    m_columns[i] = column;

    info->columnTypes[i] = instance.m_info->columnTypes[columnIndices[i]];
    info->columnFactories[i] = columnFactory;
  }

  m_info = info;
}

TBlock::~TBlock()
{
  CloseFiles();

  for (size_t i = 0; i < m_columnCount; i++)
  {
    if (m_columns[i] != NULL)
    {
      m_columns[i]->DecRef();
    }
  }

  delete[] m_columns;
  delete[] m_recordIds;
}


const TBlock::PInfo TBlock::GetInfo() const
{
  return m_info;
}

const TBlock::ColumnInfos TBlock::GetColumnInfos() const
{
  return *(ColumnInfos*)NULL;
}

void TBlock::Save(Writer &target)
{
  target.WriteOrThrow(m_persistent);

  if (m_columnFile->GetFileId() != m_persistent.columnFileId)
  {
    CloseOrThrow(*m_columnFile);
  }

  if (!m_columnFile->IsOpen())
  {
    OpenOrThrow(*m_columnFile, m_persistent.columnFileId);
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

    if (column != NULL)
    {
      SmiRecord record;
      SelectOrThrow(*m_columnFile, m_recordIds[i], SmiFile::Update, record);

      SmiWriter columnTarget(record, 0);

      column->Save(columnTarget);
    }
  }
}

void TBlock::DeleteRecords()
{
  if (!m_columnFile.IsNull())
  {
    if (m_columnFile->GetFileId() != m_persistent.columnFileId)
    {
      CloseOrThrow(*m_columnFile);
    }

    if (!m_columnFile->IsOpen())
    {
      OpenOrThrow(*m_columnFile, m_persistent.columnFileId);
    }

    for (size_t i = 0; i < m_columnCount; i++)
    {
      if (m_columns[i] == NULL)
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
}

void TBlock::CloseFiles()
{
  if (!m_columnFile.IsNull() && m_columnFile->IsOpen())
  {
    CloseOrThrow(*m_columnFile);
  }

  if (!m_flobFile.IsNull() && m_flobFile->IsOpen())
  {
    CloseOrThrow(*m_flobFile);
  }

  const size_t columnCount = m_columnCount;
  for (size_t i = 0; i < columnCount; ++i)
  {
    if (m_columns[i] != NULL)
    {
      m_columns[i]->CloseFiles();
    }
  }
}

size_t TBlock::GetColumnCount() const
{
  return m_columnCount;
}

size_t TBlock::GetRowCount() const
{
  return m_persistent.rowCount;
}

size_t TBlock::GetSize() const
{
  size_t size = 0;

  for (size_t i = 0; i < m_columnCount; ++i)
  {
    size += GetAttrArray(i).GetSize();
  }

  return sizeof(TBlock) + size;
}

void TBlock::Append(ArrayAttribute* tuple)
{
  const size_t columnCount = m_columnCount;
  for (size_t i = 0; i < columnCount; i++)
  {
    GetAttrArray(i).Append(tuple[i]);
  }

  ++m_persistent.rowCount;
}

void TBlock::Append(Attribute** tuple)
{
  const size_t columnCount = m_columnCount;
  for (size_t i = 0; i < columnCount; i++)
  {
    GetAttrArray(i).Append(*tuple[i]);
  }

  ++m_persistent.rowCount;
}

void TBlock::Append(const BlockTuple &tuple)
{
  const size_t columnCount = m_columnCount;
  for (size_t i = 0; i < columnCount; i++)
  {
    GetAttrArray(i).Append(tuple[i]);
  }

  ++m_persistent.rowCount;
}

void TBlock::Append(const Tuple &tuple)
{
  const size_t columnCount = m_columnCount;
  for (size_t i = 0; i < columnCount; i++)
  {
    GetAttrArray(i).Append(*tuple.GetAttribute(i));
  }

  ++m_persistent.rowCount;
}

AttrArray &TBlock::GetAt(size_t index) const
{
  return GetAttrArray(index);
}

TBlock::Iterator TBlock::GetIterator() const
{
  return Iterator(this);
}

TBlock::TBlock()
{
}

TBlock::TBlock(const TBlock &instance)
{
}

AttrArray &TBlock::GetAttrArray(size_t index) const
{
  if (m_columns[index] == NULL)
  {
    SmiRecordId recordId = m_recordIds[index];

    if (recordId == 0)
    {
      m_columns[index] =
        m_info->columnFactories[index]->Create(m_persistent.flobFileId);
    }
    else
    {
      if (m_columnFile->GetFileId() != m_persistent.columnFileId)
      {
        CloseOrThrow(*m_columnFile);
      }

      if (!m_columnFile->IsOpen())
      {
        OpenOrThrow(*m_columnFile, m_persistent.columnFileId);
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

      AttrArray *column =  m_info->columnFactories[index]->Load(source),
        *&entry = m_columns[index];

      entry = column;

      column = entry;
    }
  }

  return *(m_columns[index]);
}

//TBlock::TupleIterator---------------------------------------------------------

TBlock::TupleIterator::TupleIterator() :
  m_tuple(NULL, 0),
  m_rowCount(0)
{
}

TBlock::TupleIterator::TupleIterator(const TBlock *block) :
  m_tuple(block, 0),
  m_rowCount(block->GetRowCount())
{
}

bool TBlock::TupleIterator::IsValid() const
{
  return m_tuple.m_row < m_rowCount;
}

const BlockTuple &TBlock::TupleIterator::GetCurrent() const
{
  return m_tuple;
}

bool TBlock::TupleIterator::MoveToNext()
{
  if (m_tuple.m_row < m_rowCount)
  {
    return ++m_tuple.m_row < m_rowCount;
  }

  return false;
}

const BlockTuple &TBlock::TupleIterator::operator * () const
{
  return GetCurrent();
}

TBlock::TupleIterator &TBlock::TupleIterator::operator ++ ()
{
  MoveToNext();

  return *this;
}

bool TBlock::TupleIterator::operator == (const TupleIterator &other) const
{
  if (IsValid())
  {
    if (other.IsValid())
    {
      return m_tuple == other.m_tuple;
    }

    return false;
  }

  return !other.IsValid();
}

bool TBlock::TupleIterator::operator != (const TupleIterator &other) const
{
  return !(*this == other);
}

//BlockTuple--------------------------------------------------------------------

BlockTuple::BlockTuple()
{
}

BlockTuple::BlockTuple(const TBlock *block, size_t row) :
  m_block(block),
  m_row(row)
{
}

ArrayAttribute BlockTuple::operator[](size_t index) const
{
  return m_block->GetAt(index)[m_row];
}

bool BlockTuple::operator == (const BlockTuple &other) const
{
  return m_block == other.m_block && m_row == other.m_row;
}

bool BlockTuple::operator != (const BlockTuple &other) const
{
  return !(*this == other);
}