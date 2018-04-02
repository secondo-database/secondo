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

uint64_t TBlock::GetSaveSize(uint64_t columnCount, bool includeHeader)
{
  uint64_t size = sizeof(uint64_t) + (columnCount * sizeof(ColumnInfo));

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
  m_header(0, sizeof(TBlock), columnFileId, flobFileId),
  m_info(info),
  m_columnCount(m_info->columnCount),
  m_columnInfos(new ColumnInfo[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(columnFileId != 0 ?
    (!columnFile.IsNull() ? columnFile : new SmiRecordFile(false)) : nullptr),
  m_filter(*this),
  m_refCount(1)
{
  for (uint64_t i = 0; i < m_columnCount; i++)
  {
    m_columnInfos[i] = {0, 0};
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
  m_columnInfos(new ColumnInfo[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(header.columnFileId != 0 ?
    (!columnFile.IsNull() ? columnFile : new SmiRecordFile(false)) : nullptr),
  m_filter(*this),
  m_refCount(1)
{
  source.ReadOrThrow((char*)m_columnInfos, m_columnCount * sizeof(ColumnInfo));

  for (uint64_t i = 0; i < m_columnCount; i++)
  {
    m_columns[i] = nullptr;
  }
}

TBlock::TBlock(const TBlock &instance, const uint64_t *columnIndices,
               uint64_t columnCount) :
  m_header(instance.m_header),
  m_columnCount(columnCount),
  m_columnInfos(new ColumnInfo[columnCount]),
  m_columns(new AttrArray*[columnCount]),
  m_columnFile(instance.m_columnFile),
  m_filter(*this, instance.m_filter.m_filter),
  m_refCount(1)
{
  TBlockInfo *info = new TBlockInfo();
  info->columnCount = columnCount;
  info->columnTypes = new ListExpr[columnCount];
  info->columnFactories = new AttrArrayManager*[columnCount];

  SharedArray<const uint64_t> filter = m_filter.m_filter;

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; ++i)
  {
    AttrArray *column = instance.m_columns[columnIndices[i]];
    if (column != nullptr)
    {
      if (filter.IsNull())
      {
        column->IncRef();
      }
      else
      {
        column = column->Filter(filter);
      }
    }

    AttrArrayManager *columnManager =
      instance.m_info->columnFactories[columnIndices[i]];
    columnManager->IncRef();

    m_columnInfos[i] = instance.m_columnInfos[columnIndices[i]];
    m_columns[i] = column;

    size += m_columnInfos[i].size;

    info->columnTypes[i] = instance.m_info->columnTypes[columnIndices[i]];
    info->columnFactories[i] = columnManager;
  }

  m_header.size = size;
  m_info = info;
}

TBlock::TBlock(const TBlock &instance, const uint64_t *projectionIndices,
               uint64_t projectionIndexCount, AttrArray **extensionColumns,
               uint64_t extensionColumnCount,
               const ListExpr extensionColumnTypes) :
  m_header(instance.m_header),
  m_columnCount(projectionIndexCount + extensionColumnCount),
  m_columnInfos(new ColumnInfo[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(instance.m_columnFile),
  m_filter(*this),
  m_refCount(1)
{
  uint64_t columnCount = m_columnCount;

  TBlockInfo *info = new TBlockInfo();
  info->columnCount = columnCount;
  info->columnTypes = new ListExpr[columnCount];
  info->columnFactories = new AttrArrayManager*[columnCount];

  // initialize metadata for projected columns
  for (uint64_t i = 0; i < projectionIndexCount; ++i)
  {
    AttrArrayManager *columnManager =
      instance.m_info->columnFactories[projectionIndices[i]];
    columnManager->IncRef();

    m_columnInfos[i] = instance.m_columnInfos[projectionIndices[i]];

    info->columnTypes[i] = instance.m_info->columnTypes[projectionIndices[i]];
    info->columnFactories[i] = columnManager;
  }

  TBlockInfo extensionInfo(extensionColumnTypes);

  // initialize metadata for extension columns
  for (uint64_t i = 0; i < extensionColumnCount; ++i)
  {
    uint64_t columnIndex = projectionIndexCount + i;

    AttrArrayManager *columnManager = extensionInfo.columnFactories[i];
    columnManager->IncRef();

    info->columnTypes[columnIndex] = extensionInfo.columnTypes[i];
    info->columnFactories[columnIndex] = columnManager;
  }

  SharedArray<const uint64_t> filter = instance.m_filter.m_filter;

  uint64_t size = sizeof(TBlock);

  uint64_t rowCount = m_header.rowCount;

  if (filter.IsNull())
  {
    // unfiltered blocks are used as they are
    for (uint64_t i = 0; i < projectionIndexCount; ++i)
    {
      AttrArray *column = 
        m_columns[i] = instance.m_columns[projectionIndices[i]];

      if (column != nullptr)
      {
        column->IncRef();
      }

      size += m_columnInfos[i].size;
    }
  }
  else
  {
    m_header.columnFileId = 0;
    m_header.flobFileId = 0;

    // filtered blocks must be copied to ensure consistency with extension.
    // this can lead to very small blocks!
    rowCount = m_header.rowCount = filter.GetCapacity();

    for (uint64_t i = 0; i < projectionIndexCount; ++i)
    {
      AttrArray *column = m_columns[i] = info->columnFactories[i]->Create(0);

      AttrArray &sourceColumn = instance.GetAt(projectionIndices[i]);

      for (uint64_t row = 0; row < rowCount; ++row)
      {
        column->Append(sourceColumn, filter[row]);
      }

      m_columnInfos[i].recordId = 0;
      size += m_columnInfos[i].size = column->GetSize();
    }
  }

  for (uint64_t i = 0; i < extensionColumnCount; ++i)
  {
    uint64_t columnIndex = projectionIndexCount + i;

    AttrArray *column = extensionColumns[i];

    const AttrArrayFilter &columnFilter = column->GetFilter();

    if (columnFilter.HasFilter())
    {
      // copy to ensure consistency
      AttrArray *sourceColumn = column;

      column = info->columnFactories[columnIndex]->Create(0);

      for (uint64_t row = 0; row < rowCount; ++row)
      {
        column->Append(*sourceColumn, columnFilter.GetAt(row));
      }
    }
    else
    {
      column->IncRef();
    }

    m_columns[columnIndex] = column;

    size += m_columnInfos[columnIndex].size = column->GetSize();
  }

  m_header.size = size;
  m_info = info;
}

TBlock::TBlock(const TBlock &instance, const uint64_t *columnIndices,
               uint64_t columnCount,
               const SharedArray<const uint64_t> &filter) :
  m_header(instance.m_header),
  m_columnCount(columnCount),
  m_columnInfos(new ColumnInfo[columnCount]),
  m_columns(new AttrArray*[columnCount]),
  m_columnFile(instance.m_columnFile),
  m_filter(*this, filter),
  m_refCount(1)
{
  TBlockInfo *info = new TBlockInfo();
  info->columnCount = columnCount;
  info->columnTypes = new ListExpr[columnCount];
  info->columnFactories = new AttrArrayManager*[columnCount];

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; ++i)
  {
    AttrArray *column = instance.m_columns[columnIndices[i]];
    if (column != nullptr)
    {
      if (filter.IsNull())
      {
        column->IncRef();
      }
      else
      {
        column = column->Filter(filter);
      }
    }

    AttrArrayManager *columnManager =
      instance.m_info->columnFactories[columnIndices[i]];
    columnManager->IncRef();

    m_columnInfos[i] = instance.m_columnInfos[columnIndices[i]];
    m_columns[i] = column;

    size += m_columnInfos[i].size;

    info->columnTypes[i] = instance.m_info->columnTypes[columnIndices[i]];
    info->columnFactories[i] = columnManager;
  }

  m_header.size = size;
  m_info = info;
}

TBlock::TBlock(const TBlock &block, const SharedArray<const uint64_t> &filter) :
  m_header(block.m_header),
  m_info(block.m_info),
  m_columnCount(block.m_columnCount),
  m_columnInfos(new ColumnInfo[m_columnCount]),
  m_columns(new AttrArray*[m_columnCount]),
  m_columnFile(block.m_columnFile),
  m_filter(*this, filter),
  m_refCount(1)
{
  for (uint64_t i = 0; i < m_columnCount; ++i)
  {
    AttrArray *column = block.m_columns[i];

    if (column != nullptr)
    {
      if (filter.IsNull())
      {
        column->IncRef();
      }
      else
      {
        column = column->Filter(filter);
      }
    }

    m_columnInfos[i] = block.m_columnInfos[i];
    m_columns[i] = column;
  }
}

TBlock::~TBlock()
{
  for (uint64_t i = 0; i < m_columnCount; i++)
  {
    if (m_columns[i] != nullptr)
    {
      m_columns[i]->DecRef();
    }
  }

  delete[] m_columns;
  delete[] m_columnInfos;
}

const PTBlockInfo &TBlock::GetInfo() const
{
  return m_info;
}

const TBlockFilter &TBlock::GetFilter() const
{
  return m_filter;
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

  for (uint64_t i = 0; i < m_columnCount; i++)
  {
    SmiRecordId &recordId = m_columnInfos[i].recordId;

    if (recordId == 0)
    {
      AppendOrThrow(*m_columnFile, recordId);
    }
  }

  target.WriteOrThrow((char*)m_columnInfos, m_columnCount * sizeof(ColumnInfo));

  for (uint64_t i = 0; i < m_columnCount; i++)
  {
    AttrArray *column = m_columns[i];

    if (column != nullptr)
    {
      SmiRecord record;
      SelectOrThrow(*m_columnFile, m_columnInfos[i].recordId, SmiFile::Update,
                    record);

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

    for (uint64_t i = 0; i < m_columnCount; i++)
    {
      if (m_columns[i] == nullptr)
      {
        const SmiRecordId recordId = m_columnInfos[i].recordId;

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

          m_columnInfos[i].recordId = 0;
        }
      }
      else
      {
        m_columns[i]->DeleteRecords();

        const SmiRecordId recordId = m_columnInfos[i].recordId;

        if (recordId != 0)
        {
          DeleteOrThrow(*m_columnFile, recordId);

          m_columnInfos[i].recordId = 0;
        }
      }
    }
  }
  else if (m_header.flobFileId != 0)
  {
    for (uint64_t i = 0; i < m_columnCount; i++)
    {
      m_columns[i]->DeleteRecords();

      const SmiRecordId recordId = m_columnInfos[i].recordId;

      if (recordId != 0)
      {
        DeleteOrThrow(*m_columnFile, recordId);

        m_columnInfos[i].recordId = 0;
      }
    }
  }
}

uint64_t TBlock::GetColumnCount() const
{
  return m_columnCount;
}

uint64_t TBlock::GetRowCount() const
{
  return m_header.rowCount;
}

uint64_t TBlock::GetSize() const
{
  return m_header.size;
}

void TBlock::Append(const AttrArrayEntry* tuple)
{
  const uint64_t columnCount = m_columnCount;

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(tuple[i]);

    size += (m_columnInfos[i].size = array.GetSize());
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(Attribute** tuple)
{
  const uint64_t columnCount = m_columnCount;

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(*tuple[i]);

    size += (m_columnInfos[i].size = array.GetSize());
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(const TBlockEntry &tuple)
{
  const uint64_t columnCount = m_columnCount;

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(tuple[i]);

    size += (m_columnInfos[i].size = array.GetSize());
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

void TBlock::Append(const Tuple &tuple)
{
  const uint64_t columnCount = m_columnCount;

  uint64_t size = sizeof(TBlock);

  for (uint64_t i = 0; i < columnCount; i++)
  {
    AttrArray &array = GetAt(i);

    array.Append(*tuple.GetAttribute(i));

    size += (m_columnInfos[i].size = array.GetSize());
  }

  TBlockHeader &header = m_header;

  header.size = size;

  ++header.rowCount;
}

AttrArray &TBlock::GetAt(uint64_t index) const
{
  AttrArray *column = m_columns[index];

  if (column == nullptr)
  {
    SmiRecordId recordId = m_columnInfos[index].recordId;

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

      column = m_info->columnFactories[index]->Load(source, m_header);
    }

    if (!m_filter.m_filter.IsNull())
    {
      AttrArray * filteredColumn = column->Filter(m_filter.m_filter);

      column->DecRef();

      column = filteredColumn;
    }

    m_columns[index] = column;
  }

  return *column;
}

AttrArray &TBlock::operator[](uint64_t index) const
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

uint64_t TBlock::GetRefCount() const
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

  for (uint64_t i = 0; i < columnCount; ++i)
  {
    const ListExpr columnType = nl->First(types);

    types = nl->Rest(types);

    this->columnTypes[i] = columnType;

    AttrArrayTypeConstructor &arrayConstructor =
      *AttrArray::GetTypeConstructor(columnType, false);

    const ListExpr attributeType =
      arrayConstructor.GetAttributeType(columnType, true);

    columnAttributeTypes[i] = attributeType;
    columnFactories[i] = arrayConstructor.CreateManager(attributeType);
  }
}

TBlockInfo::~TBlockInfo()
{
  for (uint64_t i = 0; i < columnCount; ++i)
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

TBlockHeader::TBlockHeader(uint64_t rowCount, uint64_t size,
                           SmiFileId columnFileId, SmiFileId flobFileId) :
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