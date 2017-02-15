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

#include <stdexcept>
#include "StringUtils.h"
#include "Utility.h"

using std::invalid_argument;
using std::runtime_error;
using std::string;
using std::vector;
using stringutils::any2str;

void* CRel::Cast(void *pointer)
{
  return new (pointer) CRel;
}

CRel::CRel(const TBlock::PInfo &blockInfo, size_t desiredBlockSize,
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
  m_blockIterator(NULL),
  m_blockFile(new SmiRecordFile(false)),
  m_columnFile(new SmiRecordFile(false)),
  m_flobFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  CreateOrThrow(*m_blockFile);
  CreateOrThrow(*m_columnFile);
  CreateOrThrow(*m_flobFile);

  m_blockFileId = m_blockFile->GetFileId();
  m_columnFileId = m_columnFile->GetFileId();
  m_flobFileId = m_flobFile->GetFileId();
}

CRel::CRel(const TBlock::PInfo &blockInfo, size_t cacheSize, Reader &source) :
  m_blockInfo(blockInfo),
  m_columnCount(blockInfo->columnCount),
  m_desiredBlockSize(source.ReadOrThrow<size_t>()),
  m_blockSaveSize(TBlock::GetSaveSize(m_columnCount)),
  m_rowCount(source.ReadOrThrow<size_t>()),
  m_blockCount(source.ReadOrThrow<size_t>()),
  m_nextBlockIndex(source.ReadOrThrow<size_t>()),
  m_recordCount(m_blockCount),
  m_iteratorPosition(0),
  m_blockIterator(NULL),
  m_blockFileId(source.ReadOrThrow<SmiFileId>()),
  m_columnFileId(source.ReadOrThrow<SmiFileId>()),
  m_flobFileId(source.ReadOrThrow<SmiFileId>()),
  m_blockFile(new SmiRecordFile(false)),
  m_columnFile(new SmiRecordFile(false)),
  m_flobFile(new SmiRecordFile(false)),
  m_blockCache(cacheSize)
{
  OpenOrThrow(*m_blockFile, m_blockFileId);
  OpenOrThrow(*m_columnFile, m_columnFileId);
  OpenOrThrow(*m_flobFile, m_blockFileId);
}

CRel::~CRel()
{
  if (m_blockIterator != NULL)
  {
    delete m_blockIterator;
    m_blockIterator = NULL;
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

  if (m_blockFile->IsOpen())
  {
    CloseOrThrow(*m_blockFile);
  }

  if (m_flobFile->IsOpen())
  {
    CloseOrThrow(*m_flobFile);
  }
}

const TBlock::PInfo CRel::GetBlockInfo() const
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
  if (m_blockIterator != NULL)
  {
    delete m_blockIterator;
    m_blockIterator = NULL;
  }

  LRUCache<BlockCacheEntry>::Iterator iterator(m_blockCache);
  if (iterator.IsValid())
  {
    do
    {
      size_t index;
      BlockCacheEntry &entry = iterator.GetValue(index);

      entry.block->CloseFiles();
    }
    while (iterator.MoveToNext());
  }

  if (m_blockFile->GetFileId() != m_blockFileId)
  {
    CloseOrThrow(*m_blockFile);
  }

  if (!m_blockFile->IsOpen())
  {
    OpenOrThrow(*m_blockFile, m_blockFileId);
  }

  DropOrThrow(*m_blockFile);


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

CRel::Column CRel::GetAt(size_t index) const
{
  return Column(*this, index);
}

CRel::Iterator CRel::GetIterator() const
{
  return Iterator(this);
}

void CRel::Append(ArrayAttribute* tuple)
{
  TAppend(tuple);
}

void CRel::Append(Attribute** tuple)
{
  TAppend(tuple);
}

void CRel::Append(const BlockTuple &tuple)
{
  TAppend<const BlockTuple&>(tuple);
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
    entry.block = NULL;
  }

  if (entry.block == NULL)
  {
    entry.block = LoadBlock(index);
  }

  return *entry.block;
}

CRel::CRel() :
  m_columnCount(0),
  m_desiredBlockSize(0),
  m_blockSaveSize(0),
  m_blockCache(0)
{
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
    block = new TBlock(m_blockInfo, m_columnFileId, m_flobFileId, m_columnFile,
                       m_flobFile);

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
  if (m_blockFile->GetFileId() != m_blockFileId)
  {
    CloseOrThrow(*m_blockFile);
  }

  if (!m_blockFile->IsOpen())
  {
    OpenOrThrow(*m_blockFile, m_blockFileId);
  }

  if (m_iteratorPosition > index && m_blockIterator != NULL)
  {
    delete m_blockIterator;
    m_blockIterator = NULL;
  }

  if (m_blockIterator == NULL)
  {
    m_blockIterator = m_blockFile->SelectAllPrefetched();
    m_blockIterator->Next();
    m_iteratorPosition = 0;
  }

  while (m_iteratorPosition < index)
  {
    m_blockIterator->Next();
    ++m_iteratorPosition;
  }

  //SmiRecord record;
  //SelectOrThrow(*m_blockFile, (SmiRecordId)index + 1, SmiFile::ReadOnly,
  //              record);

  class PrefetchingIteratorReader : public Reader
  {
  public:
    PrefetchingIteratorReader(PrefetchingIterator &iterator, size_t position) :
      m_iterator(iterator),
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
      size_t read = m_iterator.ReadCurrentData(target, count, m_position);

      m_position += read;

      return read == count;
    }

  private:
    PrefetchingIterator &m_iterator;

    size_t m_position;
  };

  PrefetchingIteratorReader source(*m_blockIterator, 0);

  return new TBlock(m_blockInfo, source, m_columnFile, m_flobFile);
}

void CRel::SaveBlock(size_t index, TBlock &block) const
{
  if (m_blockIterator != NULL)
  {
    delete m_blockIterator;
    m_blockIterator = NULL;
  }

  if (m_blockFile->GetFileId() != m_blockFileId)
  {
    CloseOrThrow(*m_blockFile);
  }

  if (!m_blockFile->IsOpen())
  {
    OpenOrThrow(*m_blockFile, m_blockFileId);
  }

  SmiRecord record;
  if (m_recordCount <= index)
  {
    do
    {
      AppendOrThrow(*m_blockFile, record);
    }
    while (++m_recordCount <= index);
  }
  else
  {
    SelectOrThrow(*m_blockFile, (SmiRecordId)index + 1, SmiFile::Update,
                  record);
  }

  SmiWriter target(record, 0);
  block.Save(target);
}

ArrayAttribute CRel::GetAttribute(size_t block, size_t column, size_t row) const
{
  return GetBlock(block).GetAt(column)[row];
}