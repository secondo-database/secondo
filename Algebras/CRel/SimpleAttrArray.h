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
#include <string>

extern NestedList *nl;

namespace CRelAlgebra
{
  template <class T, void*(*cast)(void*) = nullptr>
  class SimpleFSAttrArrayIterator;

  /*
  Class template that allows easy implementation of ~AttrArray~s with entries
  of fixed size.

  For an example see ~LongInts~.

  */
  template <class T, void*(*cast)(void*) = nullptr>
  class SimpleFSAttrArray : public AttrArray
  {
  public:
    virtual ~SimpleFSAttrArray()
    {
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
      Append(((SimpleFSAttrArray<T, cast>&)array).m_values[row]);
    }

    virtual void Append(Attribute &value) = 0;

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
      const T &value = ((SimpleFSAttrArray<T, cast>&)arrayB).m_values[rowB];

      return m_values[rowA].Compare(value);
    }

    virtual int Compare(size_t row, Attribute &value) const
    {
      return m_values[row].Compare(value);
    }

    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((SimpleFSAttrArray<T, cast>&)arrayB).m_values[rowB];

      return m_values[rowA].Equals(value);
    }

    virtual bool Equals(size_t row, Attribute &value) const
    {
      return m_values[row].Equals(value);
    }

    virtual size_t GetHash(size_t row) const
    {
      return m_values[row].GetHash();
    }

    SimpleFSAttrArrayIterator<T, cast> GetIterator() const
    {
      return SimpleFSAttrArrayIterator<T, cast>(*this);
    }

    SimpleFSAttrArrayIterator<T, cast> begin() const
    {
      return GetIterator();
    }

    SimpleFSAttrArrayIterator<T, cast> end() const
    {
      return SimpleFSAttrArrayIterator<T, cast>();
    }

  protected:
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

        if (cast != nullptr)
        {
          T *value = m_values,
            *end = value + rowCount;

          while (value < end)
          {
            cast(value);

            ++value;
          }
        }
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

  private:
    friend class SimpleFSAttrArrayIterator<T, cast>;

    size_t m_count,
      m_size,
      m_capacity;

    SharedArray<char> m_data;

    T *m_values;

    SimpleFSAttrArray(const SimpleFSAttrArray &instance) = delete;
  };

  /*
  Iterator over the entries of a SimpleFSAttrArray<T, cast>.

  */
  template <class T, void*(*cast)(void*)>
  class SimpleFSAttrArrayIterator
  {
  public:
    SimpleFSAttrArrayIterator() :
      m_instance(nullptr),
      m_current(nullptr),
      m_end(nullptr)
    {
    }

    SimpleFSAttrArrayIterator(const SimpleFSAttrArray<T, cast> &instance) :
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
    const SimpleFSAttrArray<T, cast> *m_instance;

    T *m_current,
      *m_end;
  };


  template <class T>
  class SimpleVSAttrArray : public AttrArray
  {
  public:
    virtual ~SimpleVSAttrArray()
    {
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
      Append(((const SimpleVSAttrArray<T>&)array).GetAt(row));
    }

    virtual void Append(Attribute &value) = 0;

    void Append(const T &value)
    {
      const size_t valueSize = value.GetSize(),
        dataSize = m_header.dataSize,
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

      memcpy(dataEnd, value.GetData(), valueSize);

      m_header.dataSize = dataSize + valueSize;
      m_dataEnd = dataEnd + valueSize;
      m_size += valueSize + sizeof(size_t);
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

    const T GetAt(size_t index) const
    {
      const size_t position = m_positions[index],
        nextIndex = index + 1,
        size = nextIndex < m_header.count ? m_positions[nextIndex] - position :
                                            m_header.dataSize - position;

      return T(m_data.GetPointer() + position, size);
    }

    const T operator[](size_t index) const
    {
      const size_t position = m_positions[index],
        nextIndex = index + 1,
        size = nextIndex < m_header.count ? m_positions[nextIndex] - position :
                                            m_header.dataSize - position;

      return T(m_data.GetPointer() + position, size);
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
      return GetAt(row).Compare(value);
    }

    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      const T &value = ((const SimpleVSAttrArray<T>&)arrayB).GetAt(rowB);

      return GetAt(rowA).Equals(value);
    }

    virtual bool Equals(size_t row, Attribute &value) const
    {
      return GetAt(row).Equals(value);
    }

    virtual size_t GetHash(size_t row) const
    {
      return GetAt(row).GetHash();
    }

  protected:
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
  };
}