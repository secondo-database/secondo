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

#include <algorithm>
#include "AttrArray.h"
#include "Attribute.h"
#include <cstddef>
#include <cstring>
#include "NestedList.h"
#include "ReadWrite.h"
#include "Shared.h"
#include "SpatialAttrArray.h"
#include <string>

extern NestedList *nl;

namespace CRelAlgebra
{
  template <class T>
  class SimpleFSAttrArrayIterator;

  /*
  Class template that allows easy implementation of ~AttrArray~s with entries
  of fixed size.

  For an example see ~LongInts~.

  */
  template <class T>
  class SimpleFSAttrArray : public AttrArray
  {
  public:
    typedef typename T::AttributeType AttributeType;

    SimpleFSAttrArray() :
      m_count(0),
      m_size(sizeof(SimpleFSAttrArray)),
      m_capacity(0),
      m_values(nullptr)
    {
    }

    SimpleFSAttrArray(Reader &source) :
      SimpleFSAttrArray(source, source.ReadOrThrow<size_t>())
    {
    }

    SimpleFSAttrArray(Reader &source, size_t rowCount) :
      m_count(rowCount),
      m_size(sizeof(SimpleFSAttrArray) + (rowCount * sizeof(T))),
      m_capacity(rowCount),
      m_data(rowCount * sizeof(T)),
      m_values((T*)m_data.GetPointer())
    {
      const size_t size = m_data.GetCapacity();

      if (size > 0)
      {
        source.ReadOrThrow(m_data.GetPointer(), size);
      }
    }

    SimpleFSAttrArray(const SimpleFSAttrArray &array,
                      const SharedArray<const size_t> &filter) :
      AttrArray(filter),
      m_count(array.m_count),
      m_size(array.m_size),
      m_capacity(array.m_capacity),
      m_data(array.m_data),
      m_values(array.m_values)
    {
    }

    virtual ~SimpleFSAttrArray()
    {
    }

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new SimpleFSAttrArray<T>(*this, filter);
    }

    virtual size_t GetCount() const
    {
      return m_count;
    }

    virtual size_t GetSize() const
    {
      return m_size;
    }

    virtual void Save(Writer &target, bool includeHeader = true) const
    {
      if (includeHeader)
      {
        target.WriteOrThrow(m_count);
      }

      if (m_count > 0)
      {
        target.WriteOrThrow(m_data.GetPointer(), m_count * sizeof(T));
      }
    }

    virtual void Append(const AttrArray &array, size_t row)
    {
      Append(((SimpleFSAttrArray<T>&)array).m_values[row]);
    }

    virtual void Append(Attribute &value)
    {
      Append(T((AttributeType&)value));
    }

    void Append(const T &value)
    {
      size_t capacity = m_capacity,
        count = m_count++;

      T *values;

      if (capacity == count)
      {
        if (capacity == 0)
        {
          capacity = 1;

          m_data = SharedArray<char>(sizeof(T));

          values = m_values = (T*)m_data.GetPointer();
        }
        else
        {
          const size_t newCapacity = capacity + capacity,
            newByteCapacity = newCapacity * sizeof(T);

          char *data = new char[newByteCapacity];

          memcpy(data, m_data.GetPointer(), m_data.GetCapacity());

          capacity = newCapacity;

          m_data = SharedArray<char>(data, newByteCapacity);
          m_values = values = (T*)data;
        }

        m_capacity = capacity;
      }
      else
      {
        values = m_values;
      }

      new (values + count) T(value);

      m_size += sizeof(T);
    }

    virtual void Remove()
    {
      --m_count;
      m_size -= sizeof(T);
    }

    virtual void Clear()
    {
      m_count = 0;
      m_size = sizeof(SimpleFSAttrArray);
    }

    const T &GetAt(size_t index) const
    {
      return m_values[index];
    }

    const T &operator[](size_t index) const
    {
      return m_values[index];
    }

    virtual bool IsDefined(size_t row) const
    {
      return m_values[row].IsDefined();
    }

    virtual int Compare(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((SimpleFSAttrArray<T>&)arrayB).m_values[rowB];

      return m_values[rowA].Compare(value);
    }

    virtual int Compare(size_t row, Attribute &value) const
    {
      return m_values[row].Compare((AttributeType&)value);
    }

    virtual int CompareAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      const T &value = ((const SimpleFSAttrArray<T>&)arrayB).GetAt(rowB);

      return TCompareAlmost(GetAt(rowA), value);
    }

    virtual int CompareAlmost(size_t row, Attribute &value) const
    {
      return TCompareAlmost(GetAt(row), (const AttributeType&)value);
    }

    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((SimpleFSAttrArray<T>&)arrayB).m_values[rowB];

      return m_values[rowA].Equals(value);
    }

    virtual bool Equals(size_t row, Attribute &value) const
    {
      return m_values[row].Equals((AttributeType&)value);
    }

    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      const T &value = ((const SimpleFSAttrArray<T>&)arrayB).GetAt(rowB);

      return TEqualsAlmost(GetAt(rowA), value);
    }

    virtual bool EqualsAlmost(size_t row, Attribute &value) const
    {
      return TEqualsAlmost(GetAt(row), (const AttributeType&)value);
    }

    virtual size_t GetHash(size_t row) const
    {
      return m_values[row].GetHash();
    }

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const
    {
      return GetAt(row).GetAttribute(clone);
    }

    SimpleFSAttrArrayIterator<T> GetIterator() const
    {
      return SimpleFSAttrArrayIterator<T>(*this);
    }

    SimpleFSAttrArrayIterator<T> begin() const
    {
      return GetIterator();
    }

    SimpleFSAttrArrayIterator<T> end() const
    {
      return SimpleFSAttrArrayIterator<T>();
    }

  private:
    friend class SimpleFSAttrArrayIterator<T>;

    size_t m_count,
      m_size,
      m_capacity;

    SharedArray<char> m_data;

    T *m_values;

    SimpleFSAttrArray(const SimpleFSAttrArray &instance) = delete;

    template <class V>
    static int TCompareAlmost(const T &entry, V &value,
                              char(*)[!T::isPrecise] = 0)
    {
      return entry.CompareAlmost(value);
    }

    template <class V>
    static int TCompareAlmost(const T &entry, V &value,
                              char(*)[T::isPrecise] = 0)
    {
      return entry.Compare(value);
    }

    template <class V>
    static bool TEqualsAlmost(const T &entry, V &value,
                              char(*)[!T::isPrecise] = 0)
    {
      return entry.EqualsAlmost(value);
    }

    template <class V>
    static bool TEqualsAlmost(const T &entry, V &value,
                              char(*)[T::isPrecise] = 0)
    {
      return entry.Equals(value);
    }
  };

  /*
  Iterator over the entries of a SimpleFSAttrArray<T>.

  */
  template <class T>
  class SimpleFSAttrArrayIterator
  {
  public:
    SimpleFSAttrArrayIterator() :
      m_instance(nullptr),
      m_current(nullptr),
      m_end(nullptr)
    {
    }

    SimpleFSAttrArrayIterator(const SimpleFSAttrArray<T> &instance) :
      m_instance(&instance),
      m_current(instance.m_values),
      m_end(m_current + instance.m_count)
    {
    }

    bool IsValid() const
    {
      return m_current < m_end;
    }

    const T &Get() const
    {
      return *m_current;
    }

    bool MoveToNext()
    {
      if (IsValid())
      {
        ++m_current;

        return IsValid();
      }

      return false;
    }

    const T &operator * ()
    {
      return Get();
    }

    SimpleFSAttrArrayIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    bool operator == (const SimpleFSAttrArrayIterator &other) const
    {
      return !(*this != other);
    }

    bool operator != (const SimpleFSAttrArrayIterator &other) const
    {
      if (m_current < m_end)
      {
        if (other.m_current < other.m_end)
        {
          return m_current != other.m_current || m_instance != other.m_instance;
        }

        return true;
      }

      return other.m_current < other.m_end;
    }

  private:
    const SimpleFSAttrArray<T> *m_instance;

    T *m_current,
      *m_end;
  };

  class SimpleVSAttrArrayEntry
  {
  public:
    char *data;

    size_t size;
  };

  template <class T>
  class SimpleVSAttrArray : public AttrArray
  {
  public:
    typedef typename T::AttributeType AttributeType;

    SimpleVSAttrArray() :
      m_header({0, 0}),
      m_dataEnd(nullptr),
      m_size(sizeof(SimpleVSAttrArray))
    {
    }

    SimpleVSAttrArray(Reader &source) :
      SimpleVSAttrArray(source, source.ReadOrThrow<Header>())
    {
    }

    SimpleVSAttrArray(Reader &source, size_t rowCount) :
      SimpleVSAttrArray(source,
                        Header({rowCount, source.ReadOrThrow<size_t>()}))
    {
    }

    SimpleVSAttrArray(const SimpleVSAttrArray &array,
                      const SharedArray<const size_t> &filter) :
      AttrArray(filter),
      m_header(array.m_header),
      m_data(array.m_data),
      m_dataEnd(array.m_dataEnd),
      m_positions(array.m_positions),
      m_size(array.m_size)
    {
    }

    virtual ~SimpleVSAttrArray()
    {
    }

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new SimpleVSAttrArray<T>(*this, filter);
    }

    const T GetAt(size_t index) const
    {
      return T(GetEntry(index));
    }

    const T operator [](size_t index) const
    {
      return T(GetEntry(index));
    }

    virtual size_t GetCount() const
    {
      return m_header.count;
    }

    virtual size_t GetSize() const
    {
      return m_size;
    }

    virtual void Save(Writer &target, bool includeHeader = true) const
    {
      if (includeHeader)
      {
        target.WriteOrThrow(m_header);
      }
      else
      {
        target.WriteOrThrow(m_header.dataSize);
      }

      if (m_header.count > 0)
      {
        target.WriteOrThrow(m_data.GetPointer(), m_header.dataSize);
        target.WriteOrThrow((char*)m_positions.GetPointer(),
                            m_header.count * sizeof(size_t));
      }
    }

    virtual void Append(const AttrArray &array, size_t row)
    {
      const SimpleVSAttrArrayEntry source =
        ((const SimpleVSAttrArray<T>&)array).GetEntry(row),
        target = AppendEntry(source.size);

      memcpy(target.data, source.data, target.size);
    }

    virtual void Append(Attribute &value)
    {
      AttributeType &attribute = (AttributeType&)value;

      T::Write(AppendEntry(T::GetSize(attribute)), attribute);
    }

    virtual void Remove()
    {
      const size_t oldDataSize = m_header.dataSize,
        newDataSize = m_positions[--m_header.count];

      m_header.dataSize = newDataSize;

      m_size -= (oldDataSize - newDataSize) + sizeof(size_t);

      m_dataEnd = m_data.GetPointer() + newDataSize;
    }

    virtual void Clear()
    {
      m_header.count = 0;
      m_header.dataSize = 0;
      m_size = sizeof(SimpleVSAttrArray);
      m_dataEnd = m_data.GetPointer();
    }

    virtual bool IsDefined(size_t row) const
    {
      return GetAt(row).IsDefined();
    }

    virtual int Compare(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((const SimpleVSAttrArray<T>&)arrayB).GetAt(rowB);

      return GetAt(rowA).Compare(value);
    }

    virtual int Compare(size_t row, Attribute &value) const
    {
      return GetAt(row).Compare((AttributeType&)value);
    }

    virtual int CompareAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      const T &value = ((const SimpleVSAttrArray<T>&)arrayB).GetAt(rowB);

      return TCompareAlmost(GetAt(rowA), value);
    }

    virtual int CompareAlmost(size_t row, Attribute &value) const
    {
      return TCompareAlmost(GetAt(row), (const AttributeType&)value);
    }

    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((const SimpleVSAttrArray<T>&)arrayB).GetAt(rowB);

      return GetAt(rowA).Equals(value);
    }

    virtual bool Equals(size_t row, Attribute &value) const
    {
      return GetAt(row).Equals((AttributeType&)value);
    }

    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      const T &value = ((const SimpleVSAttrArray<T>&)arrayB).GetAt(rowB);

      return TEqualsAlmost(GetAt(rowA), value);
    }

    virtual bool EqualsAlmost(size_t row, Attribute &value) const
    {
      return TEqualsAlmost(GetAt(row), (const AttributeType&)value);
    }

    virtual size_t GetHash(size_t row) const
    {
      return GetAt(row).GetHash();
    }

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const
    {
      return GetAt(row).GetAttribute(clone);
    }

  private:
    class Header
    {
    public:
      size_t count,
        dataSize;
    };

    Header m_header;

    SharedArray<char> m_data;

    char* m_dataEnd;

    SharedArray<size_t> m_positions;

    size_t m_size;

    SimpleVSAttrArray(Reader &source, const Header &header) :
      m_header(header),
      m_data(header.dataSize),
      m_dataEnd(m_data.GetPointer() + header.dataSize),
      m_positions(header.count),
      m_size(sizeof(SimpleVSAttrArray) + header.dataSize +
             (header.count * sizeof(size_t)))
    {
      char * data = m_data.GetPointer();
      size_t * positions = m_positions.GetPointer();

      source.ReadOrThrow(data, m_data.GetCapacity());
      source.ReadOrThrow((char*)positions,
                         m_positions.GetCapacity() * sizeof(size_t));
    }

    SimpleVSAttrArray(const SimpleVSAttrArray &instance) = delete;

    template <class V>
    static int TCompareAlmost(const T &entry, V &value,
                              char(*)[!T::isPrecise] = 0)
    {
      return entry.CompareAlmost(value);
    }

    template <class V>
    static int TCompareAlmost(const T &entry, V &value,
                              char(*)[T::isPrecise] = 0)
    {
      return entry.Compare(value);
    }

    template <class V>
    static bool TEqualsAlmost(const T &entry, V &value,
                              char(*)[!T::isPrecise] = 0)
    {
      return entry.EqualsAlmost(value);
    }

    template <class V>
    static bool TEqualsAlmost(const T &entry, V &value,
                              char(*)[T::isPrecise] = 0)
    {
      return entry.Equals(value);
    }

    SimpleVSAttrArrayEntry GetEntry(size_t row) const
    {
      const size_t position = m_positions[row],
        nextRow = row + 1,
        size = nextRow < m_header.count ? m_positions[nextRow] - position :
                                          m_header.dataSize - position;

      return {m_data.GetPointer() + position, size};
    }

    SimpleVSAttrArrayEntry AppendEntry(const size_t valueSize)
    {
      const size_t dataSize = m_header.dataSize,
        count = m_header.count++,
        capacity = m_data.GetCapacity(),
        freeCapacity = capacity - dataSize;

      //Add new positions entry
      //Provide space if neccessery
      if (m_positions.GetCapacity() == count)
      {
        if (count == 0)
        {
          m_positions = SharedArray<size_t>(1);
          m_positions[0] = 0;
        }
        else
        {
          const size_t newCapacity = count + count;

          size_t *positions = new size_t[newCapacity];

          positions[count] = dataSize;

          memcpy(positions, m_positions.GetPointer(), count * sizeof(size_t));

          m_positions = SharedArray<size_t>(positions, newCapacity);
        }
      }
      else
      {
        m_positions[count] = dataSize;
      }

      char *dataEnd;

      //Provide space for the new value
      if (freeCapacity < valueSize)
      {
        const size_t diffCapacity = valueSize - freeCapacity,
          newCapacity = capacity == 0 ?
            diffCapacity : capacity + std::max(capacity, diffCapacity);

        char *data = new char[newCapacity];

        if (dataSize > 0)
        {
          memcpy(data, m_data.GetPointer(), dataSize);
        }

        m_data = SharedArray<char>(data, newCapacity);

        dataEnd = data + dataSize;
      }
      else
      {
        dataEnd = m_dataEnd;
      }

      m_header.dataSize = dataSize + valueSize;
      m_dataEnd = dataEnd + valueSize;
      m_size += valueSize + sizeof(size_t);

      return {dataEnd, valueSize};
    }
  };

  template <class T>
  class SimpleSpatialVSAttrArray : public SpatialAttrArray<T::dimension>
  {
  public:
    SimpleSpatialVSAttrArray()
    {
    }

    SimpleSpatialVSAttrArray(Reader &source) :
      m_array(source)
    {
    }

    SimpleSpatialVSAttrArray(Reader &source, size_t rowCount) :
      m_array(source, rowCount)
    {
    }

    SimpleSpatialVSAttrArray(const SimpleSpatialVSAttrArray &array,
                             const SharedArray<const size_t> &filter) :
      m_array(array.m_array, filter)
    {
    }

    virtual ~SimpleSpatialVSAttrArray()
    {
    }

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new SimpleSpatialVSAttrArray<T>(*this, filter);
    }

    const T GetAt(size_t index) const
    {
      return m_array.GetAt(index);
    }

    const T operator [](size_t index) const
    {
      return m_array.GetAt(index);
    }

    virtual size_t GetCount() const
    {
      return m_array.GetCount();
    }

    virtual size_t GetSize() const
    {
      return m_array.GetSize();
    }

    virtual void Save(Writer &target, bool includeHeader = true) const
    {
      m_array.Save(target, includeHeader);
    }

    virtual void Append(const AttrArray &array, size_t row)
    {
      m_array.Append(((const SimpleSpatialVSAttrArray<T>&)array).m_array, row);
    }

    virtual void Append(Attribute &value)
    {
      m_array.Append(value);
    }

    virtual void Remove()
    {
      m_array.Remove();
    }

    virtual void Clear()
    {
      m_array.Clear();
    }

    virtual bool IsDefined(size_t row) const
    {
      return m_array.IsDefined(row);
    }

    virtual int Compare(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      return m_array.Compare(rowA,
                             ((const SimpleSpatialVSAttrArray&)arrayB).m_array,
                             rowB);
    }

    virtual int Compare(size_t row, Attribute &value) const
    {
      return m_array.Compare(row, value);
    }

    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      return m_array.Equals(rowA,
                            ((const SimpleSpatialVSAttrArray&)arrayB).m_array,
                            rowB);
    }

    virtual bool Equals(size_t row, Attribute &value) const
    {
      return m_array.Equals(row, value);
    }

    virtual size_t GetHash(size_t row) const
    {
      return m_array.GetAt(row).GetHash();
    }

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const
    {
      return m_array.GetAt(row).GetAttribute(clone);
    }

    virtual Rectangle<T::dimension> GetBoundingBox(
      size_t row, const Geoid *geoid = nullptr) const
    {
      return m_array.GetAt(row).GetBoundingBox();
    }

    virtual double GetDistance(size_t row, const Rectangle<T::dimension>& rect,
                               const Geoid *geoid = nullptr) const
    {
      return m_array.GetAt(row).GetDistance(rect, geoid);
    }

    virtual bool Intersects(size_t row, const Rectangle<T::dimension>& rect,
                            const Geoid *geoid = nullptr) const
    {
      return m_array.GetAt(row).Intersects(rect, geoid);
    }

    virtual bool IsEmpty(size_t row) const
    {
      return m_array.GetAt(row).IsEmpty();
    }

  private:
    SimpleVSAttrArray<T> m_array;
  };
}