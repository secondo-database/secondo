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
#include <cstddef>
#include "NestedList.h"
#include "ReadWrite.h"
#include "RelationAlgebra.h"
#include "SecondoSMI.h"
#include "Shared.h"

namespace CRelAlgebra
{
  class TBlock;
  class TBlockInfo;
  class TBlockEntry;
  class TBlockIterator;
  class FilteredTBlockIterator;

  typedef Shared<const TBlockInfo> PTBlockInfo;

  /*
  Represents data of a ~TBlock~ which can be stored seperately.
  It also is used as a ~AttrArrayHeader~ on column restoring to avoid redundant
  data.

  */
  class TBlockHeader
  {
  public:
    size_t rowCount,
      size;

    SmiFileId columnFileId,
      flobFileId;

    TBlockHeader();

    TBlockHeader(size_t rowCount, size_t size, SmiFileId columnFileId,
            SmiFileId flobFileId);

    operator AttrArrayHeader() const;
  };

  /*
  This class adds support to filter a ~TBlock~'s tuples without copying them to
  a new ~TBlock~.

  It does so by providing its own row numbers which are mapped to the
  ~TBlock~'s row numbers.

  */
  class TBlockFilter
  {
  public:
    /*
    Creates a ~TBlockFilter~ for the passed ~block~ containing all
    tuples.

    Note: The ~TBlock~'s refcounter is not touched

    */
    TBlockFilter(const TBlock &block) :
      m_block(&block)
    {
    }

    /*
    Creates a ~TBlockFilter~ for the passed ~bloch~ containing all only the
    tuples specified by the passed row numbers in ~filter~.

    Note: The ~TBlock~'s refcounter is not touched

    */
    TBlockFilter(const TBlock &block,
                 const SharedArray<const size_t> &filter) :
      m_block(&block),
      m_filter(filter)
    {
    }

    /*
    Returns the ~TBlocks~'s row number for this ~TBlockFilter~'s row number
    ~row~.

    */
    size_t GetAt(size_t row) const
    {
      return m_filter.IsNull() ? row : m_filter[row];
    }
    size_t operator [] (size_t row) const
    {
      return m_filter.IsNull() ? row : m_filter[row];
    }

    /*
    Returns number of row numbers contained in this ~TBlockFilter~.

    */
    size_t GetRowCount() const;

    /*
    Returns a iterator over the ~TBlock~'s tuples taking this
    ~TBlockFilter~ into account

    */
    FilteredTBlockIterator GetIterator() const;

    /*
    Only for range-loop support!

    */
    FilteredTBlockIterator begin() const;

    FilteredTBlockIterator end() const;

  protected:
    friend class TBlock;

    const TBlock * const m_block;

    const SharedArray<const size_t> m_filter;
  };

  /*
  Implements a column-oriented tuple-block.

  The main functionality provided is appending tuples, retrieving tuples
  and persisting / restoring the block.

  The tuple-block is split into columns which themselves are ~AttrArray~s.
  Each of them is restored only when neccessary.

  This class is reference counted because it's heavily used in stream operators.

   Note: When accessing a ~TBlock~'s tuples you probably want to use the
  ~TBlockFilter~ provided by ~GetFilter~!

  */
  class TBlock
  {
  public:
    /*
    Returns the size the metadata of a ~TBlock~ with the specified number of
    columns would occupy.

    ~include~ header determines if ~TBlockHeader~ data should be included.

    */
    static size_t GetSaveSize(size_t columnCount, bool includeHeader = true);

    /*
    Creates a ~TBlock~ parameterized by a ~TBlockInfo~ and the provided column-
    and flob-file id.

    ~columnFile~ provides a (potentialy shared) ~SmiRecordFile~ instance.

    */
    TBlock(const PTBlockInfo &info, SmiFileId columnFileId,
           SmiFileId flobFileId);
    TBlock(const PTBlockInfo &info, SmiFileId columnFileId,
           SmiFileId flobFileId, Shared<SmiRecordFile> columnFile);

    /*
    Restores a ~TBlock~ parameterized by a ~TBlockInfo~ from the provided
    ~source~.

    ~columnFile~ provides a (potentialy shared) ~SmiRecordFile~ instance.

    Precondition: ~source~ must hold data saved by a ~TBlock~ with similar
    ~info~ and ~includeHeader~ == true.

    */
    TBlock(const PTBlockInfo &info, Reader &source);
    TBlock(const PTBlockInfo &info, Reader &source,
           Shared<SmiRecordFile> columnFile);

    /*
    Restores a ~TBlock~ parameterized by a ~TBlockInfo~ from the provided
    ~source~ and ~header~.

    ~columnFile~ provides a (potentialy shared) ~SmiRecordFile~ instance.

    Precondition: ~source~ must hold data saved by a ~TBlock~ with similar
    ~info~ and ~includeHeader~ == false.

    */
    TBlock(const PTBlockInfo &info, const TBlockHeader &header, Reader &source);

    TBlock(const PTBlockInfo &info, const TBlockHeader &header, Reader &source,
           Shared<SmiRecordFile> columnFile);

    /*
    Creates the projection of a existing ~block~.
    The indices of the columns to project on are provided by the array pointer
    ~columnIndices~. The number of columns in ~columnIndices~ is passed by
    ~columnCount~.

    The returned ~TBlock~ is supposed to be read only.
    God knows what might happen if one trys to actually modify or save it.

    */
    TBlock(const TBlock &block, const size_t *columnIndices,
           size_t columnCount);

    /*
    Creates the projection of a existing ~block~ and applies a filter to it.
    The indices of the columns to project on are provided by the array pointer
    ~columnIndices~. The number of columns in ~columnIndices~ is passed by
    ~columnCount~. The rows to include are specified by ~filter~.

    The returned ~TBlock~ is supposed to be read only.
    God knows what might happen if one trys to actually modify or save it.

    */
    TBlock(const TBlock &block, const size_t *columnIndices, size_t columnCount,
           const SharedArray<const size_t> &filter);

    /*
    Creates filtered ~TBlock~ from the passed ~block~.
    The rows to include are specified by ~filter~.

    */
    TBlock(const TBlock &block, const SharedArray<const size_t> &filter);

    virtual ~TBlock();

    /*
    Returns the ~TBlockInfo~ used by this ~TBlock~.

    */
    const PTBlockInfo &GetInfo() const;

    /*
    Returns the applied ~TBlockFilter~.

    */
    const TBlockFilter &GetFilter() const;

    /*
    Writes this ~TBlock~ into the specified ~target~ either with or without
    ~TBlockHeader~ data.

    */
    void Save(Writer &target, bool includeHeader = true);

    /*
    Deletes persistent data created by this tuple-block.

    */
    void DeleteRecords();

    /*
    Returns the number of columns of this ~TBlock~.

    */
    size_t GetColumnCount() const;

    /*
    Returns the number of tuples in this ~TBlock~.

    */
    size_t GetRowCount() const;

    /*
    Returns the size of this ~TBlock~ in bytes.

    */
    size_t GetSize() const;

    /*
    Appends a tuple to this tuple-block.

    Preconditions:
      *~tuple~ must point to a array of ~AttrArrayEntry~
      *~tuple~ must contain ~GetColumnCount()~ entries
      *the attribute-array types of the entries must match the attribute-array
       types of this tuple-block's columns
    */
    void Append(const AttrArrayEntry* tuple);

    /*
    Appends a tuple to this tuple-block.

    This function does not touch the ~Attribute~'s reference counters.

    Preconditions:
      *~tuple~ must point to a array of ~Attribute~ pointers
      *~tuple~ must contain ~GetColumnCount()~ entries
      *the attribute types of the entries must match the attribute types of this
       tuple-block's columns
    */
    void Append(Attribute** tuple);

    /*
    Appends a tuple to this tuple-block.

    Preconditions:
      *~tuple~ must represent a valid ~TBlockEntry~
      *~tuple~'s ~TBlock~ must have a similar ~TBlockInfo~ to this tuple-block
    */
    void Append(const TBlockEntry &tuple);

    /*
    Appends a tuple to this tuple-block.

    This function does not touch the ~Tuple~'s reference counter.

    Preconditions:
      *~tuple~ must contain ~GetColumnCount()~ attributes
      *the attribute types of the tuple's attributes must match the attribute
       types of this tuple-block's columns
    */
    void Append(const Tuple &tuple);

    /*
    Accesses the column with the specified ~index~

    */
    AttrArray &GetAt(size_t index) const;
    AttrArray &operator[](size_t index) const;

    /*
    Returns a ~TBlockIterator~ over this tuple-block's tuples.

    */
    TBlockIterator GetIterator() const;

    FilteredTBlockIterator GetFilteredIterator() const;

    /*
    ~TBlockIterator~s used (only!) for range-loop support.

    */
    TBlockIterator begin() const;
    TBlockIterator end() const;

    /*
    Increases the reference counter by one.

    */
    void IncRef() const;

    /*
    Decreases the reference counter by one.
    If the reference counter reaches zero this object is deleted.

    */
    void DecRef() const;

    /*
    Returns the reference count.

    */
    size_t GetRefCount() const;

  private:
    class ColumnInfo
    {
    public:
      size_t size;

      SmiRecordId recordId;
    };

    TBlockHeader m_header;

    PTBlockInfo m_info;

    //The count of tuples in this block.
    //Corresponds the count of attributes in each block.
    const size_t m_columnCount;

    ColumnInfo *m_columnInfos;

    //The attribute-blocks.
    AttrArray **m_columns;

    mutable Shared<SmiRecordFile> m_columnFile;

    TBlockFilter m_filter;

    mutable size_t m_refCount;

    TBlock(const TBlock&) = delete;
  };

  inline size_t TBlockFilter::GetRowCount() const
  {
    return m_filter.IsNull() ? m_block->GetRowCount() : m_filter.GetCapacity();
  }

  /*
  Class used to configure a ~TBlock~'s columns.
  It can be shared among multiple ~TBlock~s.

  */
  class TBlockInfo
  {
  public:
    size_t columnCount;

    ListExpr *columnTypes,
      *columnAttributeTypes;

    AttrArrayManager **columnFactories;

    TBlockInfo();

    /*
    Creates a ~TBlockInfo~ from a list of attribute-array types.

    */
    TBlockInfo(ListExpr columnTypes);

    ~TBlockInfo();
  };

  /*
  This class represents the tuple of a ~TBlock~ by a pointer to the block and
  a row number.

  If either the pointer doesn't point to a ~TBlock~ instance or the row
  number is out of the block's range of rows, the ~TBlockEntry~ is considered
  invalid.

  This class doesn't change a ~TBlockEntry~'s reference count.
  If the pointed to block is deleted this ~TBlockEntry~ becomes invalid.

  Using a invalid ~TBlockEntry~ is considered undefined behaviour.

  */
  class TBlockEntry
  {
  public:
    TBlockEntry()
    {
    }

    TBlockEntry(const TBlock *block, size_t row) :
      m_block(block),
      m_row(row)
    {
    }

    size_t GetRow() const
    {
      return m_row;
    }

    const TBlock *GetBlock() const
    {
      return m_block;
    }

    const AttrArrayEntry operator[](size_t index) const
    {
      return AttrArrayEntry(&m_block->GetAt(index), m_row);
    }

    bool operator == (const TBlockEntry &other) const
    {
      return m_row == other.m_row && m_block == other.m_block;
    }

    bool operator != (const TBlockEntry &other) const
    {
      return !(*this == other);
    }

  private:
    const TBlock *m_block;

    size_t m_row;

    friend class TBlockIterator;
    friend class FilteredTBlockIterator;
  };

  /*
  A iterator over a ~TBlock~’s tuples.

  Changes of the ~TBlock~ invalidate the iterator which is not reflected by
  ~TBlockIterator.IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  class TBlockIterator
  {
  public:
    TBlockIterator() :
      m_tuple(nullptr, 0),
      m_rowCount(0)
    {
    }

    TBlockIterator(const TBlock *block) :
      m_tuple(block, 0),
      m_rowCount(block->GetRowCount())
    {
    }

    bool IsValid() const
    {
      return m_tuple.m_row < m_rowCount;
    }

    const TBlockEntry &Get() const
    {
      return m_tuple;
    }

    bool MoveToNext()
    {
      if (m_tuple.m_row < m_rowCount)
      {
        return ++m_tuple.m_row < m_rowCount;
      }

      return false;
    }

    const TBlockEntry &operator * () const
    {
      return Get();
    }

    TBlockIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    bool operator == (const TBlockIterator &other) const
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

    bool operator != (const TBlockIterator &other) const
    {
      return !(*this == other);
    }

  private:
    TBlockEntry m_tuple;

    size_t m_rowCount;
  };

  class FilteredTBlockIterator
  {
  public:
    FilteredTBlockIterator() :
      m_filter(nullptr),
      m_rowCount(0),
      m_row(0),
      m_tuple(nullptr, 0)
    {
    }

    FilteredTBlockIterator(const TBlock *block) :
      m_filter(block != nullptr ? &block->GetFilter() : nullptr),
      m_rowCount(m_filter != nullptr ? m_filter->GetRowCount() : 0),
      m_row(0),
      m_tuple(block, m_rowCount > 0 ? m_filter->GetAt(0) : 0)
    {
    }

    bool IsValid() const
    {
      return m_row < m_rowCount;
    }

    const TBlockEntry &Get() const
    {
      return m_tuple;
    }

    bool MoveToNext()
    {
      if (m_row < m_rowCount)
      {
        if (++m_row < m_rowCount)
        {
          m_tuple.m_row = m_filter->GetAt(m_row);

          return true;
        }
      }

      return false;
    }

    const TBlockEntry &operator * () const
    {
      return Get();
    }

    FilteredTBlockIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    bool operator == (const FilteredTBlockIterator &other) const
    {
      return !(*this != other);
    }

    bool operator != (const FilteredTBlockIterator &other) const
    {
      if (IsValid())
      {
        if (other.IsValid())
        {
          return m_tuple == other.m_tuple;
        }

        return true;
      }

      return other.IsValid();
    }

  private:
    const TBlockFilter * m_filter;

    size_t m_rowCount,
      m_row;

    TBlockEntry m_tuple;
  };

  inline FilteredTBlockIterator TBlockFilter::GetIterator() const
  {
    return FilteredTBlockIterator(m_block);
  }

  inline FilteredTBlockIterator TBlockFilter::begin() const
  {
    return FilteredTBlockIterator(m_block);
  }

  inline FilteredTBlockIterator TBlockFilter::end() const
  {
    return FilteredTBlockIterator();
  }

  inline FilteredTBlockIterator TBlock::GetFilteredIterator() const
  {
    return FilteredTBlockIterator(this);
  }
}