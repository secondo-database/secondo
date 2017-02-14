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

#include "GenericAttrArray.h"
#include "AttrArray.h"
#include "Attribute.h"
#include <cstddef>
#include "NestedList.h"
#include "ReadWrite.h"
#include "RelationAlgebra.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include <string>
#include <vector>

class TBlock;

class BlockTuple
{
public:
  BlockTuple();

  BlockTuple(const TBlock *block, size_t row);

  ArrayAttribute operator[](size_t index) const;

  bool operator == (const BlockTuple &other) const;

  bool operator != (const BlockTuple &other) const;

private:
  const TBlock *m_block;

  size_t m_row;

  friend class TBlock;
};

class TBlock : public RefCounter
{
public:
  class TupleIterator
  {
  public:
    TupleIterator();

    TupleIterator(const TBlock *block);

    bool IsValid() const;

    const BlockTuple &GetCurrent() const;

    bool MoveToNext();

    const BlockTuple &operator * () const;

    TupleIterator &operator ++ ();

    bool operator == (const TupleIterator &other) const;

    bool operator != (const TupleIterator &other) const;

  private:
    BlockTuple m_tuple;

    size_t m_rowCount;
  };

  class Iterator
  {
  public:
    Iterator() :
      m_block(NULL),
      m_row(0),
      m_rowCount(0)
    {
    }

    Iterator(const TBlock *block) :
      m_block(block),
      m_row(0),
      m_rowCount(block->GetRowCount())
    {
    }

    bool IsValid()
    {
      return m_row < m_rowCount;
    }

    ArrayAttribute GetAttribute(size_t column)
    {
      return m_block->GetAttrArray(column)[m_row];
    }

    bool MoveToNext()
    {
      if (m_row < m_rowCount)
      {
        return ++m_row < m_rowCount;
      }

      return false;
    }

  private:
    const TBlock *m_block;

    size_t m_row,
      m_rowCount;
  };

  class Info
  {
  public:
    size_t columnCount;

    ListExpr *columnTypes;

    AttrArrayFactory **columnFactories;

    Info();

    Info(ListExpr columnTypes);

    ~Info();
  };

  typedef Shared<const Info> PInfo;

  typedef SharedArray<const GenericAttrArray::PInfo> ColumnInfos;

  static void* Cast(void *pointer);

  static size_t GetSaveSize(size_t columnCount);

  TBlock(const PInfo &info, SmiFileId columnFileId, SmiFileId flobFileId);

  TBlock(const PInfo &info, SmiFileId columnFileId, SmiFileId flobFileId,
         Shared<SmiRecordFile> columnFile, Shared<SmiRecordFile> flobFile);

  TBlock(const PInfo &info, Reader &source);

  TBlock(const PInfo &info, Reader &source, Shared<SmiRecordFile> columnFile,
         Shared<SmiRecordFile> flobFile);

  TBlock(const TBlock &instance, size_t *columnIndices, size_t columnCount);

  virtual ~TBlock();

  const PInfo GetInfo() const;

  const ColumnInfos GetColumnInfos() const;

  void Save(Writer &target);

  void DeleteRecords();

  void CloseFiles();

  size_t GetColumnCount() const;

  size_t GetRowCount() const;

  size_t GetSize() const;

  const TBlock *Project(size_t *columnIndices, size_t columnCount) const;

  void Append(ArrayAttribute* tuple);

  void Append(Attribute** tuple);

  void Append(const BlockTuple &tuple);

  void Append(const Tuple &tuple);

  AttrArray &GetAt(size_t index) const;

  AttrArray &operator[](size_t index) const
  {
    return GetAt(index);
  }

  Iterator GetIterator() const;

  TupleIterator GetTupleIterator() const
  {
    return TupleIterator(this);
  }

  TupleIterator begin() const
  {
    return TupleIterator(this);
  }

  TupleIterator end() const
  {
    return TupleIterator();
  }

private:
  class PersistentMembers
  {
  public:
    size_t rowCount;

    SmiFileId columnFileId,
      flobFileId;

    PersistentMembers()
    {
    }

    PersistentMembers(size_t rowCount, SmiFileId columnFileId,
                      SmiFileId flobFileId) :
      rowCount(rowCount),
      columnFileId(columnFileId),
      flobFileId(flobFileId)
    {
    }

    PersistentMembers(Reader &source)
    {
      source.ReadOrThrow((char*)this, sizeof(PersistentMembers));
    }
  };

  PersistentMembers m_persistent;

  PInfo m_info;

  //The count of tuples in this block.
  //Corresponds the count of attributes in each block.
  size_t m_columnCount;

  SmiRecordId *m_recordIds;

  //The attribute-blocks.
  AttrArray **m_columns;

  mutable Shared<SmiRecordFile> m_columnFile,
    m_flobFile;

  TBlock();
  TBlock(const TBlock &instance);

  AttrArray &GetAttrArray(size_t index) const;
};