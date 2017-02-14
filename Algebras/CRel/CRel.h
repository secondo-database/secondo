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

#include "AttrArray.h"
#include "Attribute.h"
#include "LRUCache.h"
#include "ReadWrite.h"
#include "RelationAlgebra.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include <string>
#include "TBlock.h"

class CRel
{
public:
  class BlockIterator
  {
  public:
    BlockIterator() :
      m_relation(NULL),
      m_block(NULL),
      m_index(0)
    {
    }

    BlockIterator(const CRel *instance) :
      m_relation(instance),
      m_block(m_relation->GetBlockCount() > 0 ? &m_relation->GetBlock(0) :
                                                NULL),
      m_index(0)
    {
      if (m_block != NULL)
      {
        m_block->AddRef();
      }
    }

    ~BlockIterator()
    {
      if (m_block != NULL)
      {
        m_block->DecRef();
      }
    }

    bool IsValid()
    {
      return m_block != NULL;
    }

    TBlock &GetBlock()
    {
      return *m_block;
    }

    bool MoveToNext()
    {
      if (m_block != NULL)
      {
        m_block->DecRef();

        if (++m_index < m_relation->GetBlockCount())
        {
          m_block = &m_relation->GetBlock(m_index);
          m_block->AddRef();

          return true;
        }

        m_block = NULL;
      }

      return false;
    }

  private:
    const CRel *m_relation;

    TBlock *m_block;

    size_t m_index;
  };

  class Column
  {
  public:
    class Iterator
    {
    public:
      Iterator()
      {
      }

      Iterator(const Column &instance) :
        m_blockIterator(instance.m_relation),
        m_tupleIterator(m_blockIterator.IsValid() ?
          TBlock::Iterator(&m_blockIterator.GetBlock()) : TBlock::Iterator()),
        m_column(instance.m_column)
      {
      }

      bool IsValid()
      {
        return m_tupleIterator.IsValid();
      }

      ArrayAttribute GetAttribute()
      {
        return m_tupleIterator.GetAttribute(m_column);
      }

      bool MoveToNext()
      {
        if (m_tupleIterator.IsValid())
        {
          if (!m_tupleIterator.MoveToNext())
          {
            do
            {
              if (!m_blockIterator.MoveToNext())
              {
                return false;
              }
              else
              {
                m_tupleIterator = m_blockIterator.GetBlock().GetIterator();
              }
            }
            while (!m_tupleIterator.IsValid());
          }

          return true;
        }

        return false;
      }

    private:
      BlockIterator m_blockIterator;

      TBlock::Iterator m_tupleIterator;

      size_t m_column;
    };

    Column() :
      m_column(0),
      m_relation(NULL)
    {
    }

    Column(const CRel &instance, size_t column) :
      m_column(column),
      m_relation(&instance)
    {
    }

    ArrayAttribute operator[](size_t index)
    {
      return m_relation->GetAttribute(m_column, index);
    }

    Iterator GetIterator() const
    {
      return Iterator(*this);
    }

  private:
    size_t m_column;

    const CRel *m_relation;
  };

  class Iterator
  {
  public:
    Iterator()
    {
    }

    Iterator(const CRel *instance) :
      m_blockIterator(instance),
      m_tupleIterator(m_blockIterator.IsValid() ?
        TBlock::TupleIterator(&m_blockIterator.GetBlock()) :
        TBlock::TupleIterator())
    {
    }

    bool IsValid()
    {
      return m_tupleIterator.IsValid();
    }

    const BlockTuple &GetCurrent()
    {
      return m_tupleIterator.GetCurrent();
    }

    bool MoveToNext()
    {
      if (m_tupleIterator.IsValid())
      {
        if (!m_tupleIterator.MoveToNext())
        {
          do
          {
            if (!m_blockIterator.MoveToNext())
            {
              return false;
            }
            else
            {
              m_tupleIterator = m_blockIterator.GetBlock().GetTupleIterator();
            }
          }
          while (!m_tupleIterator.IsValid());
        }

        return true;
      }

      return false;
    }

    const BlockTuple &operator * () const
    {
      return m_tupleIterator.GetCurrent();
    }

    Iterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    bool operator == (const Iterator &other) const
    {
      return m_tupleIterator == other.m_tupleIterator;
    }

    bool operator != (const Iterator &other) const
    {
      return m_tupleIterator != other.m_tupleIterator;
    }

  private:
    BlockIterator m_blockIterator;

    TBlock::TupleIterator m_tupleIterator;
  };

  static void* Cast(void *pointer);

  CRel(const TBlock::PInfo &blockInfo, size_t desiredBlockSize,
       size_t cacheSize);

  CRel(const TBlock::PInfo &blockInfo, size_t cacheSize, Reader &source);

  ~CRel();

  const TBlock::PInfo GetBlockInfo() const;

  void Save(Writer &target);

  void DeleteFiles();

  size_t GetColumnCount() const;

  size_t GetRowCount() const;

  size_t GetBlockCount() const;

  size_t GetDesiredBlockSize() const;

  size_t GetCacheSize() const;

  void Append(ArrayAttribute* tuple);

  void Append(Attribute** tuple);

  void Append(const BlockTuple &tuple);

  void Append(const Tuple &tuple);

  void Append(Iterator attributes);

  void Append(TBlock::Iterator attributes);

  TBlock &GetBlock(size_t index) const;

  Column GetAt(size_t index) const;

  Column operator[](size_t index) const
  {
    return GetAt(index);
  }

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
  class BlockDataCacheEntry
  {
  public:
    char *data;

    bool modified;

    BlockDataCacheEntry() :
      data(NULL),
      modified(false)
    {
    }
  };

  class BlockCacheEntry
  {
  public:
    TBlock *block;

    bool modified;

    BlockCacheEntry() :
      block(NULL),
      modified(false)
    {
    }
  };

  TBlock::PInfo m_blockInfo;

  const size_t m_columnCount,
    m_desiredBlockSize,
    m_blockSaveSize;

  size_t m_rowCount,
    m_blockCount,
    m_nextBlockIndex;

  mutable size_t m_recordCount,
    m_iteratorPosition;

  mutable PrefetchingIterator *m_blockIterator;//SelectAllPrefetched();
  //mutable SmiRecordFileIterator *m_blockIterator;
  //mutable SmiRecord m_blockRecord;

  SmiFileId m_blockFileId,
    m_columnFileId,
    m_flobFileId;

  Shared<SmiRecordFile> m_blockFile,
    m_columnFile,
    m_flobFile;

  //mutable LRUCache<BlockDataCacheEntry> m_blockDataCache;
  mutable LRUCache<BlockCacheEntry> m_blockCache;

  CRel();

  template<class T>
  void TAppend(T tuple);

  void SaveBlock(size_t index, TBlock &block) const;

  TBlock *LoadBlock(size_t index) const;

  ArrayAttribute GetAttribute(size_t collumn, size_t row) const;

  ArrayAttribute GetAttribute(size_t block, size_t collumn, size_t row) const;
};