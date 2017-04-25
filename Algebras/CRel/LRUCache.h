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

#include <cstddef>
#include "TBlock.h"

namespace CRelAlgebra
{
  template<class T>
  class LRUCache
  {
  private:
    class Entry
    {
    public:
      T value;

      size_t key;

      Entry *next,
        *previous,
        *lru,
        *mru;

      Entry() :
        value(),
        key(~0),
        next(nullptr),
        previous(nullptr),
        lru(nullptr),
        mru(nullptr)
      {
      }
    };

  public:
    class Iterator
    {
    public:
      Iterator() :
        m_entry(nullptr),
        m_end(nullptr)
      {
      }

      Iterator(LRUCache &instance) :
        m_entry(instance.m_entries),
        m_end(m_entry + instance.m_count)
      {
      }

      T &GetValue(size_t &key)
      {
        key = m_entry->key;

        return m_entry->value;
      }

      bool IsValid()
      {
        return m_entry < m_end;
      }

      bool MoveToNext()
      {
        if (IsValid())
        {
          ++m_entry;

          return IsValid();
        }
        else
        {
          return false;
        }
      }

    private:
      Entry *m_entry,
        *m_end;
    };

    LRUCache(size_t maxCount) :
      m_count(0)
    {
      size_t size = 2,
        hash = ~0;

      while (size <= maxCount && size != 0)
      {
        size = size << 1;
        hash = hash << 1;
      }

      if (size == 0)
      {
        size = ~0;
        hash = ~0;
      }
      else
      {
        size = size >> 1;
        hash = ~hash;
      }

      m_size = size;
      m_hash = hash;

      m_entries = new Entry[size];

      m_lru = m_entries;
      m_mru = m_entries;

      m_map = new Entry*[size];

      for (size_t i = 0; i < size; i++)
      {
        m_map[i] = nullptr;
      }
    }

    ~LRUCache()
    {
      delete[] m_map;
      delete[] m_entries;
    }

    T &Get(size_t key, size_t &recycledKey)
    {
      if (m_mru->key == key)
      {
        recycledKey = key;
        return m_mru->value;
      }

      const size_t hash = key & m_hash;

      Entry *entry = m_map[hash],
        *previous,
        *next;

      if (entry == nullptr)
      {
        previous = nullptr;
        next = nullptr;
      }
      else if (entry->key == key)
      {
        SetMRU(entry);

        recycledKey = key;

        return entry->value;
      }
      else if (entry->key > key)
      {
        //search for key or insertion position
        previous = entry,
        next = entry->next;

        while (next != nullptr)
        {
          if (next->key == key)
          {
            //make next the mru and return a value reference
            if (m_mru != next)
            {
              if (m_lru == next)
              {
                m_lru = next->mru;
              }
              else
              {
                Entry *lru = next->lru,
                  *mru = next->mru;

                lru->mru = mru;
                mru->lru = lru;
              }

              SetMRU(next);
            }

            recycledKey = key;

            return next->value;
          }

          if (next->key < key)
          {
            //insert between previous and next
            break;
          }

          previous = next;
          next = previous->next;
        }

        //insert after last element
      }
      else
      {
        //insert before first element
        previous = nullptr;
        next = entry;
      }

      if (m_count < m_size)
      {
        //get a unused entry and make it the mru
        entry = &m_entries[m_count];
        ++m_count;
      }
      else
      {
        //reuse the lru
        entry = m_lru;
        m_lru = entry->mru;
      }

      if (entry != previous && entry != next)
      {
        //unlink the entry
        if (entry->previous != nullptr)
        {
          entry->previous->next = entry->next;
        }
        else
        {
          m_map[entry->key & m_hash] = entry->next;
        }

        if (entry->next != nullptr)
        {
          entry->next->previous = entry->previous;
        }

        //link the entry
        entry->previous = previous;
        entry->next = next;

        if (previous != nullptr)
        {
          previous->next = entry;
        }
        else
        {
          m_map[hash] = entry;
        }

        if (next != nullptr)
        {
          next->previous = entry;
        }
      }

      SetMRU(entry);

      recycledKey = entry->key != ~0 ? entry->key : key;

      entry->key = key;

      return entry->value;
    }

    bool TryGet(size_t key, const T &value) const
    {
      Entry *entry(m_map[m_hash & key]);

      if (entry != nullptr)
      {
        do
        {
          if (entry->key == key)
          {
            value = entry->value;

            return true;
          }

          entry = entry->next;
        }
        while (entry != nullptr && entry->key > key);
      }

      return false;
    }

    size_t GetCount() const
    {
      return m_count;
    }

    size_t GetSize() const
    {
      return m_size;
    }

  private:
    Entry *m_lru,
      *m_mru;

    Entry* m_entries;

    Entry** m_map;

    size_t m_size,
      m_hash,
      m_count;

    void SetMRU(Entry *entry)
    {
      entry->lru = m_mru;
      m_mru->mru = entry;
      m_mru = entry;
    }
  };
}