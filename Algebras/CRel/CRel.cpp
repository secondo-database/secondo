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

#include "CRel.h"

#include "SmiUtils.h"

using namespace CRelAlgebra;

using std::string;

//CRel--------------------------------------------------------------------------

CRel::CRel(const PTBlockInfo &blockInfo, size_t desiredBlockSize,
           size_t cacheSize) :
  m_blockInfo(blockInfo),
  m_columnCount(blockInfo->columnCount),
  m_desiredBlockSize(desiredBlockSize),
  m_blockSaveSize(TBlock::GetSaveSize(m_columnCount)),
  m_rowCount(0),
  m_blockCount(0),
  m_nextBlockIndex(0),
  m_recordCount(0),
  m_iteratorPosition(0),
  m_blockIterator(nullptr),
  m_blockRecordEntrySize(TBlock::GetSaveSize(m_columnCount, false) +
                         sizeof(BlockHeader)),
  m_blockRecordEntryCount((BlockRecord::sizeLimit + m_blockRecordEntrySize - 1)
                          / m_blockRecordEntrySize),
  m_blockRecordSize(m_blockRecordEntryCount * m_blockRecordEntrySize),
  m_blockFile(false),
  m_columnFile(new SmiRecordFile(false)),
  m_flobFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  CreateOrThrow(m_blockFile);
  CreateOrThrow(*m_columnFile);
  CreateOrThrow(*m_flobFile);

  m_blockFileId = m_blockFile.GetFileId();
  m_columnFileId = m_columnFile->GetFileId();
  m_flobFileId = m_flobFile->GetFileId();
}

CRel::CRel(const PTBlockInfo &blockInfo, size_t cacheSize, Reader &source) :
  m_blockInfo(blockInfo),
  m_columnCount(blockInfo->columnCount),
  m_desiredBlockSize(source.ReadOrThrow<size_t>()),
  m_blockSaveSize(TBlock::GetSaveSize(m_columnCount)),
  m_rowCount(source.ReadOrThrow<size_t>()),
  m_blockCount(source.ReadOrThrow<size_t>()),
  m_nextBlockIndex(source.ReadOrThrow<size_t>()),
  m_recordCount(m_blockCount),
  m_iteratorPosition(0),
  m_blockIterator(nullptr),
  m_blockRecordEntrySize(TBlock::GetSaveSize(m_columnCount, false) +
                         sizeof(BlockHeader)),
  m_blockRecordEntryCount((BlockRecord::sizeLimit + m_blockRecordEntrySize - 1)
                          / m_blockRecordEntrySize),
  m_blockRecordSize(m_blockRecordEntryCount * m_blockRecordEntrySize),
  m_blockFileId(source.ReadOrThrow<SmiFileId>()),
  m_columnFileId(source.ReadOrThrow<SmiFileId>()),
  m_flobFileId(source.ReadOrThrow<SmiFileId>()),
  m_blockFile(false),
  m_columnFile(new SmiRecordFile(false)),
  m_flobFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  OpenOrThrow(m_blockFile, m_blockFileId);
  OpenOrThrow(*m_columnFile, m_columnFileId);
  OpenOrThrow(*m_flobFile, m_flobFileId);
}

CRel::~CRel()
{
  if (m_blockRecord.data != nullptr)
  {
    if (m_blockRecord.modified)
    {
      size_t offset = 0;

      if (m_recordCount > m_blockRecord.index)
      {
        WriteOrThrow(m_blockFile, m_blockRecord.index + 1, m_blockRecord.data,
                     m_blockRecordSize, offset);
      }
      else
      {
        SmiRecord record;

        do
        {
          AppendOrThrow(m_blockFile, record);
        }
        while (++m_recordCount <= m_blockRecord.index);

        WriteOrThrow(record, m_blockRecord.data, m_blockRecordSize, offset);
      }
    }

    delete[] m_blockRecord.data;
  }

  if (m_blockFileId != 0 && m_flobFileId != 0)
  {
    LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

    if (iterator.IsValid())
    {
      do
      {
        size_t index;
        BlockCacheEntry &entry = iterator.GetValue(index);

        if (entry.modified)
        {
          SaveBlock(index, *entry.block);
        }

        entry.block->DecRef();
      }
      while (iterator.MoveToNext());
    }
  }
  else
  {
    LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

    if (iterator.IsValid())
    {
      do
      {
        size_t index;
        iterator.GetValue(index).block->DecRef();
      }
      while (iterator.MoveToNext());
    }
  }

  if (m_blockFile.IsOpen())
  {
    CloseOrThrow(m_blockFile);
  }

  if (m_flobFile->IsOpen())
  {
    CloseOrThrow(*m_flobFile);
  }
}

const PTBlockInfo &CRel::GetBlockInfo() const
{
  return m_blockInfo;
}

void CRel::Save(Writer &target)
{
  target.WriteOrThrow(m_desiredBlockSize);
  target.WriteOrThrow(m_rowCount);
  target.WriteOrThrow(m_blockCount);
  target.WriteOrThrow(m_nextBlockIndex);
  target.WriteOrThrow(m_blockFileId);
  target.WriteOrThrow(m_columnFileId);
  target.WriteOrThrow(m_flobFileId);

  LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

  if (iterator.IsValid())
  {
    do
    {
      size_t index;
      BlockCacheEntry &entry = iterator.GetValue(index);

      if (entry.modified)
      {
        SaveBlock(index, *entry.block);
        entry.modified = false;
      }
    }
    while (iterator.MoveToNext());
  }
}

void CRel::DeleteFiles()
{
  DropOrThrow(m_blockFile);

  if (m_columnFile->GetFileId() != m_columnFileId)
  {
    CloseOrThrow(*m_columnFile);
  }

  if (!m_columnFile->IsOpen())
  {
    OpenOrThrow(*m_columnFile, m_columnFileId);
  }

  DropOrThrow(*m_columnFile);


  if (m_flobFile->GetFileId() != m_flobFileId)
  {
    CloseOrThrow(*m_flobFile);
  }

  if (!m_flobFile->IsOpen())
  {
    OpenOrThrow(*m_flobFile, m_flobFileId);
  }

  DropOrThrow(*m_flobFile);

  m_blockFileId = 0;
  m_columnFileId = 0;
  m_flobFileId = 0;
}

size_t CRel::GetColumnCount() const
{
  return m_columnCount;
}

size_t CRel::GetRowCount() const
{
  return m_rowCount;
}

size_t CRel::GetBlockCount() const
{
  return m_blockCount;
}

size_t CRel::GetDesiredBlockSize() const
{
  return m_desiredBlockSize;
}

size_t CRel::GetCacheSize() const
{
  return m_blockCache.GetSize();
}

void CRel::Append(const AttrArrayEntry* tuple)
{
  TAppend(tuple);
}

void CRel::Append(Attribute** tuple)
{
  TAppend(tuple);
}

void CRel::Append(const TBlockEntry &tuple)
{
  TAppend<const TBlockEntry&>(tuple);
}

void CRel::Append(const Tuple &tuple)
{
  TAppend<const Tuple&>(tuple);
}

TBlock &CRel::GetBlock(size_t index) const
{
  size_t replacedIndex;
  BlockCacheEntry &entry = m_blockCache.Get(index, replacedIndex);

  if (replacedIndex != index)
  {
    if (entry.modified)
    {
      SaveBlock(replacedIndex, *entry.block);
    }

    entry.modified = false;

    entry.block->DecRef();
    entry.block = nullptr;
  }

  if (entry.block == nullptr)
  {
    entry.block = LoadBlock(index);
  }

  return *entry.block;
}

CRelIterator CRel::GetIterator() const
{
  return CRelIterator(this);
}

CRelBlockIterator CRel::GetBlockIterator() const
{
  return CRelBlockIterator(this);
}

CRelIterator CRel::begin() const
{
  return GetIterator();
}

CRelIterator CRel::end() const
{
  return CRelIterator();
}

template<class T>
void CRel::TAppend(T tuple)
{
  TBlock *block;

  size_t index = m_nextBlockIndex,
    replacedIndex;

  if (index < m_blockCount)
  {
    block = &GetBlock(index);
  }
  else
  {
    block = new TBlock(m_blockInfo, m_columnFileId, m_flobFileId, m_columnFile);

    ++m_blockCount;

    BlockCacheEntry &entry = m_blockCache.Get(index, replacedIndex);

    if (replacedIndex != index)
    {
      if (entry.modified)
      {
        SaveBlock(replacedIndex, *entry.block);
      }

      entry.block->DecRef();
    }

    entry.modified = true;
    entry.block = block;
  }

  block->Append(tuple);

  if (block->GetSize() >= m_desiredBlockSize)
  {
    ++m_nextBlockIndex;
  }

  ++m_rowCount;
}

TBlock *CRel::LoadBlock(size_t index) const
{
  BlockRecord &blockRecord = GetBlockRecord(index);

  const size_t offset =
    (index % m_blockRecordEntryCount) * m_blockRecordEntrySize;

  BufferReader source(blockRecord.data, offset);

  return new TBlock(m_blockInfo,
                    source.ReadOrThrow<BlockHeader>().ToHeader(m_columnFileId,
                                                               m_flobFileId),
                    source, m_columnFile);
}

void CRel::SaveBlock(size_t index, TBlock &block) const
{
  BlockRecord &blockRecord = GetBlockRecord(index);

  blockRecord.modified = true;

  const size_t offset =
    (index % m_blockRecordEntryCount) * m_blockRecordEntrySize;

  BufferWriter target(blockRecord.data, offset);

  target.WriteOrThrow(BlockHeader(block));

  block.Save(target, false);
}


CRel::BlockRecord &CRel::GetBlockRecord(size_t blockIndex) const
{
  const size_t recordIndex = blockIndex / m_blockRecordEntryCount;

  BlockRecord &blockRecord = m_blockRecord;

  if (blockRecord.data != nullptr && blockRecord.index != recordIndex)
  {
    if (blockRecord.modified)
    {
      size_t offset = 0;

      if (m_recordCount > blockRecord.index)
      {
        WriteOrThrow(m_blockFile, blockRecord.index + 1, blockRecord.data,
                    m_blockRecordSize, offset);
      }
      else
      {
        SmiRecord record;

        do
        {
          AppendOrThrow(m_blockFile, record);
        }
        while (++m_recordCount <= blockRecord.index);

        WriteOrThrow(record, blockRecord.data, m_blockRecordSize, offset);
      }
    }

    delete[] blockRecord.data;

    blockRecord.data = nullptr;
  }

  if (blockRecord.data == nullptr)
  {
    size_t offset = 0;

    blockRecord.index = recordIndex;

    blockRecord.modified = false;

    if (m_recordCount > recordIndex)
    {
      blockRecord.data = ReadOrThrow(m_blockFile, recordIndex + 1,
                                     m_blockRecordSize, offset);
    }
    else
    {
      blockRecord.data = new char[m_blockRecordSize];
    }
  }

  return m_blockRecord;
}

//CRel::BlockCacheEntry---------------------------------------------------------

CRel::BlockCacheEntry::BlockCacheEntry() :
  block(nullptr),
  modified(false)
{
}

//CRel::BlockHeader-------------------------------------------------------------

CRel::BlockHeader::BlockHeader()
{
}

CRel::BlockHeader::BlockHeader(const TBlock &block) :
  rowCount(block.GetRowCount()),
  size(block.GetSize())
{
}

TBlockHeader CRel::BlockHeader::ToHeader(SmiFileId columnFileId,
                                           SmiFileId flobFileId)
{
  return TBlockHeader(rowCount, size, columnFileId, flobFileId);
}

//CRel::BlockRecord-------------------------------------------------------------

CRel::BlockRecord::BlockRecord() :
  index(0),
  data(nullptr),
  modified(false)
{
}