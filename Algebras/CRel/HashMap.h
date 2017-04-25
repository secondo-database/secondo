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

#pragma once

#include <cstddef>
#include <vector>

namespace CRelAlgebra
{
  template<class K, class V, size_t(*hash)(const K&),
           int(*compare)(const K&, const K&)>
  class SortedHashMap
  {
  public:
    class Node
    {
    public:
      K key;

      V value;

      Node *next;

      bool equalsNext;
    };

    class EqualRangeIterator
    {
    public:
      EqualRangeIterator() :
        m_node(nullptr)
      {
      }

      EqualRangeIterator(Node *node) :
        m_node(node)
      {
      }

      bool IsValid()
      {
        return m_node != nullptr;
      }

      V &GetValue()
      {
        return m_node->value;
      }

      bool MoveToNext()
      {
        if (m_node != nullptr)
        {
          m_node = m_node->equalsNext ? m_node->next : nullptr;

          return m_node != nullptr;
        }

        return false;
      }

    private:
      Node *m_node;
    };

    SortedHashMap(size_t bucketCount) :
      m_bucketCount(bucketCount),
      m_count(0),
      m_nodePool(nullptr),
      m_buckets(new Node*[bucketCount])
    {
      for (size_t i = 0; i < bucketCount; ++i)
      {
        m_buckets[i] = nullptr;
      }
    }

    ~SortedHashMap()
    {
      for (size_t i = 0, j = 0; i < m_bucketCount && j < m_count; ++i)
      {
        Node *node = m_buckets[i];

        while (node != nullptr)
        {
          Node *next = node->next;

          delete node;

          node = next;

          ++j;
        }
      }

      delete[] m_buckets;
    }

    EqualRangeIterator Get(const K &key) const
    {
      Node *node = m_buckets[hash(key) % m_bucketCount];

      while (node != nullptr)
      {
        const int c = compare(key, node->key);

        if (c == 0)
        {
          return EqualRangeIterator(node);
        }
        else if (c > 0)
        {
          while (node->equalsNext)
          {
            node = node->next;
          }

          node = node->next;
        }
        else
        {
          break;
        }
      }

      return EqualRangeIterator();
    }

    void Add(const K &key, const V &value)
    {
      ++m_count;

      Node *&bucket = m_buckets[hash(key) % m_bucketCount],
        *newNode = m_nodePool;

      if (newNode == nullptr)
      {
        newNode = new Node();
      }
      else
      {
        m_nodePool = newNode->next;
      }

      newNode->key = key;
      newNode->value = value;

      if (bucket == nullptr)
      {
        bucket = newNode;

        newNode->next = nullptr;
        newNode->equalsNext = false;
      }
      else
      {
        Node *previous = nullptr,
          * node = bucket;

        do
        {
          const int c = compare(key, node->key);

          if (c <= 0)
          {
            if (previous == nullptr)
            {
              bucket = newNode;
            }
            else
            {
              previous->next = newNode;
            }

            newNode->next = node;
            newNode->equalsNext = c == 0;

            return;
          }

          previous = node;
          node = node->next;
        }
        while (node != nullptr);

        previous->next = newNode;

        newNode->next = nullptr;
        newNode->equalsNext = false;
      }
    }

    void Clear()
    {
      for (size_t i = 0, j = 0; i < m_bucketCount && j < m_count; ++i)
      {
        Node *&bucket = m_buckets[i];

        if (bucket != nullptr)
        {
          Node *node = bucket,
            *next = node->next;

          while (next != nullptr)
          {
            node = next;
            next = node->next;

            ++j;
          }

          node->next = m_nodePool;

          m_nodePool = bucket;
          bucket = nullptr;

          ++j;
        }
      }

      m_count = 0;
    }

    size_t GetCount() const
    {
      return m_count;
    }

    size_t GetBucketCount() const
    {
      return m_bucketCount;
    }

  private:
    const size_t m_bucketCount;

    size_t m_count;

    Node* m_nodePool,
      ** m_buckets;
  };

  template<class K, class V, size_t(*hash)(const K&),
          bool(*equals)(const K&, const K&)>
  class HashMap
  {
  public:
    class Entry
    {
    public:
      const K key;

      V value;

      Entry(const K &key, const V &value) :
        key(key),
        value(value)
      {
      }
    };

    typedef std::vector<Entry> Bucket;

    class EqualRangeIterator
    {
    public:
      EqualRangeIterator() :
        m_bucket(nullptr)
      {
      }

      EqualRangeIterator(std::vector<Entry> *bucket, const K &key) :
        m_key(key)
      {
        if (bucket != nullptr)
        {
          const size_t bucketSize = bucket->size();

          for (size_t i = 0; i < bucketSize; ++i)
          {
            if (equals(key, bucket->at(i).key))
            {
              m_bucket = bucket;
              m_index = i;
              m_bucketSize = bucketSize;

              return;
            }
          }
        }

        m_bucket = nullptr;
      }

      bool IsValid()
      {
        return m_bucket != nullptr;
      }

      V &GetValue()
      {
        return m_bucket->at(m_index).value;
      }

      bool MoveToNext()
      {
        if (m_bucket != nullptr)
        {
          for (size_t i = m_index + 1; i < m_bucketSize; ++i)
          {
            if (equals(m_key, m_bucket->at(i).key))
            {
              m_index = i;

              return true;
            }
          }

          m_bucket = nullptr;
        }

        return false;
      }

    private:
      std::vector<Entry> *m_bucket;

      K m_key;

      size_t m_index,
        m_bucketSize;
    };

    HashMap(size_t bucketCount) :
      m_bucketCount(bucketCount),
      m_count(0),
      m_size(sizeof(HashMap<K, V, hash, equals>)),
      m_buckets(new Bucket*[bucketCount])
    {
      for (size_t i = 0; i < bucketCount; ++i)
      {
        m_buckets[i] = nullptr;
      }

      m_size += sizeof(Bucket*) * bucketCount;
    }

    ~HashMap()
    {
      for (size_t i = 0, j = 0; i < m_bucketCount && j < m_count; ++i)
      {
        Bucket *bucket = m_buckets[i];

        if (bucket != nullptr)
        {
          delete bucket;
        }
      }

      const size_t bucketPoolSize = m_bucketPool.size();
      for (size_t i = 0; i < bucketPoolSize; ++i)
      {
        delete m_bucketPool[i];
      }

      delete[] m_buckets;
    }

    EqualRangeIterator Get(const K &key) const
    {
      return EqualRangeIterator(m_buckets[hash(key) % m_bucketCount], key);
    }

    void Add(const K &key, const V &value)
    {
      ++m_count;

      Bucket *&bucket = m_buckets[hash(key) % m_bucketCount];

      if (bucket == nullptr)
      {
        if (!m_bucketPool.empty())
        {
          bucket = m_bucketPool.back();
          m_bucketPool.pop_back();

          m_size += sizeof(Entry);
        }
        else
        {
          bucket = new Bucket();

          m_size += sizeof(Bucket) + sizeof(Entry);
        }
      }
      else
      {
        m_size += sizeof(Entry);
      }

      bucket->push_back(Entry(key, value));
    }

    void Clear()
    {
      for (size_t i = 0, j = 0; i < m_bucketCount && j < m_count; ++i)
      {
        Bucket *&bucket = m_buckets[i];

        if (bucket != nullptr)
        {
          j += bucket->size();

          bucket->clear();

          m_bucketPool.push_back(bucket);

          bucket = nullptr;
        }
      }

      m_count = 0;

      m_size = sizeof(HashMap<K, V, hash, equals>) +
               (sizeof(Bucket*) * m_bucketCount) +
               (sizeof(Bucket) * m_bucketPool.size());
    }

    size_t GetCount() const
    {
      return m_count;
    }

    size_t GetBucketCount() const
    {
      return m_bucketCount;
    }

    size_t GetSize() const
    {
      return m_size;
    }

  private:
    const size_t m_bucketCount;

    size_t m_count,
      m_size;

    std::vector<Bucket*> m_bucketPool;

    Bucket** m_buckets;
  };
}