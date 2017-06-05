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

CRel::CRel(const PTBlockInfo &blockInfo, uint64_t desiredBlockSize,
           uint64_t cacheSize) :
  m_blockInfo(blockInfo),
  m_desiredBlockSize(desiredBlockSize),
  m_rowCount(0),
  m_blockCount(0),
  m_nextBlockIndex(0),
  m_recordCount(0),
  m_blockRecordEntrySize(TBlock::GetSaveSize(blockInfo->columnCount, false) +
                         sizeof(BlockHeader)),
  m_blockRecordEntryCount((BlockRecord::sizeLimit + m_blockRecordEntrySize - 1)
                          / m_blockRecordEntrySize),
  m_blockRecordSize(m_blockRecordEntryCount * m_blockRecordEntrySize),
  m_blockFile(false),
  m_columnFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  CreateOrThrow(m_blockFile);
  CreateOrThrow(*m_columnFile);

  m_blockFileId = m_blockFile.GetFileId();
  m_columnFileId = m_columnFile->GetFileId();
}

CRel::CRel(const PTBlockInfo &blockInfo, uint64_t cacheSize, Reader &source) :
  m_blockInfo(blockInfo),
  m_desiredBlockSize(source.ReadOrThrow<uint64_t>()),
  m_rowCount(source.ReadOrThrow<uint64_t>()),
  m_blockCount(source.ReadOrThrow<uint64_t>()),
  m_nextBlockIndex(source.ReadOrThrow<uint64_t>()),
  m_recordCount(source.ReadOrThrow<uint64_t>()),
  m_blockRecordEntrySize(TBlock::GetSaveSize(blockInfo->columnCount, false) +
                         sizeof(BlockHeader)),
  m_blockRecordEntryCount((BlockRecord::sizeLimit + m_blockRecordEntrySize - 1)
                          / m_blockRecordEntrySize),
  m_blockRecordSize(m_blockRecordEntryCount * m_blockRecordEntrySize),
  m_blockFileId(source.ReadOrThrow<SmiFileId>()),
  m_columnFileId(source.ReadOrThrow<SmiFileId>()),
  m_blockFile(false),
  m_columnFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  OpenOrThrow(m_blockFile, m_blockFileId);
  OpenOrThrow(*m_columnFile, m_columnFileId);
}

CRel::~CRel()
{
  if (m_blockRecord.data != nullptr)
  {
    if (m_blockRecord.modified)
    {
      uint64_t offset = 0;

      WriteOrThrow(m_blockFile, m_blockRecord.index + 1, m_blockRecord.data,
                    m_blockRecordSize, offset);
    }
  }

  if (m_blockFileId != 0)
  {
    LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

    if (iterator.IsValid())
    {
      do
      {
        uint64_t index;
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
        uint64_t index;
        iterator.GetValue(index).block->DecRef();
      }
      while (iterator.MoveToNext());
    }
  }

  if (m_blockFile.IsOpen())
  {
    CloseOrThrow(m_blockFile);
  }
}

const PTBlockInfo &CRel::GetBlockInfo() const
{
  return m_blockInfo;
}

void CRel::Save(Writer &target)
{
  LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

  if (iterator.IsValid())
  {
    do
    {
      uint64_t index;
      BlockCacheEntry &entry = iterator.GetValue(index);

      if (entry.modified)
      {
        SaveBlock(index, *entry.block);
        entry.modified = false;
      }
    }
    while (iterator.MoveToNext());
  }

  target.WriteOrThrow(m_desiredBlockSize);
  target.WriteOrThrow(m_rowCount);
  target.WriteOrThrow(m_blockCount);
  target.WriteOrThrow(m_nextBlockIndex);
  //It is important to write the m_recordCount AFTER saving the tupleblocks
  target.WriteOrThrow(m_recordCount);
  target.WriteOrThrow(m_blockFileId);
  target.WriteOrThrow(m_columnFileId);
}

void CRel::Clear()
{
  LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);

  if (iterator.IsValid())
  {
    do
    {
      uint64_t index;
      iterator.GetValue(index).block->DecRef();
    }
    while (iterator.MoveToNext());
  }

  m_blockCache.~LRUCache<BlockCacheEntry>();
  new (&m_blockCache) LRUCache<BlockCacheEntry>(m_blockCache.GetSize());

  m_rowCount = 0;
  m_blockCount = 0;
  m_nextBlockIndex = 0;
  m_recordCount = 0;

  m_blockRecord = BlockRecord();

  if (!m_blockFile.Truncate())
  {
    DropOrThrow(m_blockFile);

    CreateOrThrow(m_blockFile);

    m_blockFileId = m_blockFile.GetFileId();
  }

  if (m_columnFile->GetFileId() != m_columnFileId)
  {
    CloseOrThrow(*m_columnFile);
  }

  if (!m_columnFile->IsOpen())
  {
    OpenOrThrow(*m_columnFile, m_columnFileId);
  }

  if (!m_columnFile->Truncate())
  {
    DropOrThrow(*m_columnFile);

    CreateOrThrow(*m_columnFile);

    m_columnFileId = m_columnFile->GetFileId();
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

  m_blockFileId = 0;
  m_columnFileId = 0;
}

uint64_t CRel::GetColumnCount() const
{
  return m_blockInfo->columnCount;
}

uint64_t CRel::GetRowCount() const
{
  return m_rowCount;
}

uint64_t CRel::GetBlockCount() const
{
  return m_blockCount;
}

uint64_t CRel::GetDesiredBlockSize() const
{
  return m_desiredBlockSize;
}

uint64_t CRel::GetCacheSize() const
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

TBlock &CRel::GetBlock(uint64_t index) const
{
  uint64_t replacedIndex;
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

  uint64_t index = m_nextBlockIndex,
    replacedIndex;

  if (index < m_blockCount)
  {
    block = &GetBlock(index);
  }
  else
  {
    block = new TBlock(m_blockInfo, m_columnFileId, m_columnFileId,
                       m_columnFile);

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

TBlock *CRel::LoadBlock(uint64_t index) const
{
  BlockRecord &blockRecord = GetBlockRecord(index);

  const uint64_t offset =
    (index % m_blockRecordEntryCount) * m_blockRecordEntrySize;

  BufferReader source(blockRecord.data, offset);

  return new TBlock(m_blockInfo,
                    source.ReadOrThrow<BlockHeader>().ToHeader(m_columnFileId,
                                                               m_columnFileId),
                    source, m_columnFile);
}

void CRel::SaveBlock(uint64_t index, TBlock &block) const
{
  BlockRecord &blockRecord = GetBlockRecord(index);

  blockRecord.modified = true;

  const uint64_t offset =
    (index % m_blockRecordEntryCount) * m_blockRecordEntrySize;

  BufferWriter target(blockRecord.data, offset);

  target.WriteOrThrow(BlockHeader(block));

  block.Save(target, false);
}


CRel::BlockRecord &CRel::GetBlockRecord(uint64_t blockIndex) const
{
  const uint64_t recordIndex = blockIndex / m_blockRecordEntryCount;

  BlockRecord &blockRecord = m_blockRecord;

  if (blockRecord.data != nullptr && blockRecord.index != recordIndex)
  {
    if (blockRecord.modified)
    {
      uint64_t offset = 0;

      WriteOrThrow(m_blockFile, blockRecord.index + 1, blockRecord.data,
                   m_blockRecordSize, offset);
    }

    delete[] blockRecord.data;

    blockRecord.data = nullptr;
  }

  if (blockRecord.data == nullptr)
  {
    uint64_t offset = 0;

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

      SmiRecord record;

      do
      {
        AppendOrThrow(m_blockFile, record);
      }
      while (++m_recordCount <= recordIndex);
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

CRel::BlockRecord::~BlockRecord()
{
  if (data != nullptr)
  {
    delete[] data;
  }
}