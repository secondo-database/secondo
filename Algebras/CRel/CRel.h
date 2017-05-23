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
#include "SmiUtils.h"
#include <string>
#include "TBlock.h"

namespace CRelAlgebra
{
  class CRelIterator;
  class CRelBlockIterator;

  /*
  Implements a column-oriented relation.

  The main functionality provided is appending tuples, retrieving tuple-blocks
  and persisting / restoring the relation.

  The whole relation (sequence of tuples) is split up into tuple-blocks.
  This hapens on insertion.
  Once a tuple-block excceeds a specified size (in bytes) a new tuple-block is
  appended.

  This class uses ~TBlock~s as tuple-block representation which implements the
  column-oriented aspects can be further parameterized by a ~TBlockInfo~.

  To potentialy improve random access of tuple-blocks a specified number of
  blocks is cached.

  */
  class CRel
  {
  public:
    /*
    Creates a ~CRel~ using ~TBlock~s specified by ~blockInfo~, the
    ~desiredBlockSize~ in bytes and ~cacheSize~ representing the number of
    blocks to cache.

    */
    CRel(const PTBlockInfo &blockInfo, size_t desiredBlockSize,
        size_t cacheSize);

    /*
    Creates a ~CRel~ using ~TBlock~s specified by ~blockInfo~ and ~cacheSize~
    representing the number of blocks to cache.

    The ~CRel~ is restored from the data provided by ~source~.

    Precondition: the data held by ~source~ was created by a ~CRel~ with similar
                  ~blockInfo~

    */
    CRel(const PTBlockInfo &blockInfo, size_t cacheSize, Reader &source);

    /*
    Destroys this ~CRel~ instance.

    */
    ~CRel();

    /*
    Returns the ~TBlockInfo~ used by the ~TBlock~s in this relation.

    */
    const PTBlockInfo &GetBlockInfo() const;

    /*
    Persists this relation into the provided ~target~.

    This actually only saves metadata.
    The rest is stored in three files created by this relation.

    */
    void Save(Writer &target);

    /*
    Deletes the files created by this relation.

    */
    void DeleteFiles();

    /*
    Returns the number of columns of this ~CRel~.

    */
    size_t GetColumnCount() const;

    /*
    Returns the number of tuples in this ~CRel~.

    */
    size_t GetRowCount() const;

    /*
    Returns the number of tuple-blocks in this ~CRel~.

    */
    size_t GetBlockCount() const;

    /*
    Returns the desired block size of this ~CRel~.

    */
    size_t GetDesiredBlockSize() const;

    /*
    Returns the maximum number of tuple blocks cached by this ~CRel~.

    */
    size_t GetCacheSize() const;

    /*
    Appends a tuple to this relation.

    Preconditions:
      *~tuple~ must point to a array of ~AttrArrayEntry~
      *~tuple~ must contain ~GetColumnCount()~ entries
      *the attribute-array types of the entries must match the attribute-array
       types of this relation's columns
    */
    void Append(const AttrArrayEntry* tuple);

    /*
    Appends a tuple to this relation.

    This function does not touch the ~Attribute~'s reference counters.

    Preconditions:
      *~tuple~ must point to a array of ~Attribute~ pointers
      *~tuple~ must contain ~GetColumnCount()~ entries
      *the attribute types of the entries must match the attribute types of this
       relation's columns
    */
    void Append(Attribute** tuple);

    /*
    Appends a tuple to this relation.

    Preconditions:
      *~tuple~ must represent a valid ~TBlockEntry~
      *~tuple~'s ~TBlock~ must have a similar ~TBlockInfo~ to this relation
    */
    void Append(const TBlockEntry &tuple);

    /*
    Appends a tuple to this relation.

    This function does not touch the ~Tuple~'s reference counter.

    Preconditions:
      *~tuple~ must contain ~GetColumnCount()~ attributes
      *the attribute types of the tuple's attributes must match the attribute
       types of this relation's columns
    */
    void Append(const Tuple &tuple);

    /*
    Accesses a tuple block by it's ~index~.

    */
    TBlock &GetBlock(size_t index) const;

    /*
    Returns a ~CRelIterator~ over this relation's tuples.

    */
    CRelIterator GetIterator() const;

    /*
    Returns a ~CRelBlockIterator~ over this relation's tuple blocks.

    */
    CRelBlockIterator GetBlockIterator() const;

    /*
    ~CRelIterator~s used (only!) for range-loop support.

    */
    CRelIterator begin() const;
    CRelIterator end() const;

  private:
    /*
    Class representing a entry of the tuple-block cache.
    If a entry leaves the cache and was marked ~modified~ the ~block~ gets
    saved.

    */
    class BlockCacheEntry
    {
    public:
      TBlock *block;

      bool modified;

      BlockCacheEntry();
    };

    /*
    Used to save and restore part of a ~TBlockHeader~'s data in one read / write
    operation.

    */
    class BlockHeader
    {
    public:
      size_t rowCount,
        size;

      BlockHeader();

      BlockHeader(const TBlock &block);

      TBlockHeader ToHeader(SmiFileId columnFileId, SmiFileId flobFileId);
    };

    /*
    Represents a block of ~TBlock~ metadata.
    Saving ~TBlock~ metadata in blocks saves read / write operations.
    We allways keep one ~BlockRecord~ in memory so ~sizeLimit~ should be chosen
    keeping that in mind.

    */
    class BlockRecord
    {
    public:
      static const size_t sizeLimit = 1024 * 1024;

      size_t index;

      char *data;

      bool modified;

      BlockRecord();
    };

    PTBlockInfo m_blockInfo;

    const size_t m_columnCount,
      m_desiredBlockSize;

    size_t m_rowCount,
      m_blockCount,
      m_nextBlockIndex;

    mutable size_t m_recordCount;

    const size_t m_blockRecordEntrySize,
      m_blockRecordEntryCount,
      m_blockRecordSize;

    mutable BlockRecord m_blockRecord;

    SmiFileId m_blockFileId,
      m_columnFileId,
      m_flobFileId;

    mutable SmiRecordFile m_blockFile;

    Shared<SmiRecordFile> m_columnFile;

    mutable LRUCache<BlockCacheEntry> m_blockCache;

    /*
    Deleted copy-constructor to prevent pointer copies

    */
    CRel(const CRel&) = delete;

    /*
    This internal append function accepts all ~T~ which are valid in
    ~TBlock.Append(T)~. This saves us some redundant code.

    */
    template<class T>
    void TAppend(T tuple);

    /*
    Saves a ~TBlock~ with the provided ~index~.

    */
    void SaveBlock(size_t index, TBlock &block) const;

    /*
    Restores a ~TBlock~ with the provided ~index~.

    */
    TBlock *LoadBlock(size_t index) const;

    /*
    Gets the ~BlockRecord~ containing the metadata for the requested
    ~blockIndex~.

    */
    BlockRecord &GetBlockRecord(size_t blockIndex) const;
  };

  /*
  A iterator over a ~CRel~’s tuple-blocks.

  Changes of the ~CRel~ invalidate the iterator which is not reflected by
  ~CRelBlockIterator.IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  class CRelBlockIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    CRelBlockIterator() :
      m_relation(nullptr),
      m_block(nullptr),
      m_index(0)
    {
    }

    /*
    Creates a iterator pointing at the first block in the passed ~relation~.
    If the ~relation~ is empty the iterator is invalid.

    */
    CRelBlockIterator(const CRel *relation) :
      m_relation(relation),
      m_block(relation->GetBlockCount() > 0 ? &relation->GetBlock(0) : nullptr),
      m_index(0)
    {
      if (m_block != nullptr)
      {
        m_block->IncRef();
      }
    }

    ~CRelBlockIterator()
    {
      if (m_block != nullptr)
      {
        m_block->DecRef();
      }
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid()
    {
      return m_block != nullptr;
    }

    /*
    Returns the ~TBlock~ at the iterator's current position.

    Precondition: ~IsValid()~

    */
    TBlock &Get()
    {
      return *m_block;
    }

    /*
    Moves the iterator to the next position.
    Returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
    bool MoveToNext()
    {
      if (m_block != nullptr)
      {
        m_block->DecRef();

        if (++m_index < m_relation->GetBlockCount())
        {
          m_block = &m_relation->GetBlock(m_index);
          m_block->IncRef();

          return true;
        }

        m_block = nullptr;
      }

      return false;
    }

  private:
    const CRel *m_relation;

    TBlock *m_block;

    size_t m_index;
  };

  /*
  A iterator over a ~CRel~’s tuples.

  Changes of the ~CRel~ invalidate the iterator which is not reflected by
  ~CRelIterator.IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  class CRelIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    CRelIterator()
    {
    }

    /*
    Creates a iterator pointing at the first tuple in the passed ~relation~.
    If the ~relation~ is empty the iterator is invalid.

    */
    CRelIterator(const CRel *relation) :
      m_blockIterator(relation),
      m_tupleIterator(m_blockIterator.IsValid() ?
        TBlockIterator(&m_blockIterator.Get()) :
        TBlockIterator())
    {
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid()
    {
      return m_tupleIterator.IsValid();
    }

    /*
    Returns a ~TBlockEntry~ representing the tuple at the iterator's current
    position.

    Precondition: ~IsValid()~

    */
    const TBlockEntry &Get()
    {
      return m_tupleIterator.Get();
    }

    const TBlockEntry &operator * () const
    {
      return m_tupleIterator.Get();
    }

    /*
    Moves the iterator to the next position.
    ~MoveToNext~ returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
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
              m_tupleIterator = m_blockIterator.Get().GetIterator();
            }
          }
          while (!m_tupleIterator.IsValid());
        }

        return true;
      }

      return false;
    }

    CRelIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }


    /*
    Compares this iterator and the ~other~ iterator for equality.

    */
    bool operator == (const CRelIterator &other) const
    {
      return m_tupleIterator == other.m_tupleIterator;
    }

    /*
    Compares this iterator and the ~other~ iterator for inequality.

    */
    bool operator != (const CRelIterator &other) const
    {
      return m_tupleIterator != other.m_tupleIterator;
    }

  private:
    CRelBlockIterator m_blockIterator;

    TBlockIterator m_tupleIterator;
  };
}