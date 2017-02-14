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

class RefCounter
{
public:
  RefCounter() :
    m_count(1)
  {
  }

  size_t GetRefCount() const
  {
    return m_count;
  }

  void AddRef() const
  {
    ++m_count;
  }

  void DecRef() const
  {
    if (m_count <= 1)
    {
      m_count = 0;

      delete this;
    }
    else
    {
      --m_count;
    }
  }

protected:
  virtual ~RefCounter()
  {
  }

private:
  mutable size_t m_count;
};

template <class T>
class Shared
{
public:
  Shared() :
    m_instance(NULL),
    m_refCount(NULL)
  {
  };

  Shared(const T &instance) :
    m_instance(new T(instance)),
    m_refCount(NULL)
  {
  };

  Shared(T *instance) :
    m_instance(instance),
    m_refCount(NULL)
  {
  };

  Shared(const Shared &instance) :
    m_instance(instance.m_instance),
    m_refCount(instance.m_refCount)
  {
    if (m_instance != NULL)
    {
      if (m_refCount == NULL)
      {
        m_refCount = new size_t(2);
        instance.m_refCount = m_refCount;
      }
      else
      {
        ++*m_refCount;
      }
    }
  }

  ~Shared()
  {
    if (m_instance != NULL)
    {
      if (m_refCount == NULL)
      {
        delete m_instance;
      }
      else if (*m_refCount == 1)
      {
        delete m_refCount;
        delete m_instance;
      }
      else
      {
        --*m_refCount;
      }
    }
  };

  inline T * GetPointer() const
  {
    return m_instance;
  }

  inline bool IsNull() const
  {
    return m_instance == NULL;
  }

  inline size_t GetCount()
  {
    return m_refCount != NULL ? *m_refCount : 1;
  }

  inline Shared &operator=(const Shared &instance)
  {
    this->~Shared();
    new (this) Shared(instance);

    return *this;
  }

  inline Shared &operator=(T *instance)
  {
    this->~Shared();
    new (this) Shared(instance);

    return *this;
  }

  inline T *operator->() const
  {
    return m_instance;
  }

  inline T &operator*() const
  {
    return *m_instance;
  }

  inline operator Shared<const T>()
  {
    if (m_instance != NULL)
    {
      if (m_refCount == NULL)
      {
        m_refCount = new size_t(2);
      }
      else
      {
        ++*m_refCount;
      }
    }

    return Shared<const T>(m_instance, m_refCount);
  }

private:
  template <class F>
  friend class Shared;

  T *m_instance;

  mutable size_t *m_refCount;

  Shared(T *instance, size_t *refCount) :
    m_instance(instance),
    m_refCount(refCount)
  {
  }
};

template <class T>
class SharedArray
{
public:
  SharedArray() :
    m_instance(NULL),
    m_capacity(0),
    m_refCount(new size_t(1))
  {
  };

  SharedArray(size_t capacity) :
    m_instance(capacity > 0 ? new T[capacity] : NULL),
    m_capacity(capacity),
    m_refCount(new size_t(1))
  {
  };

  SharedArray(T *instance, size_t capacity) :
    m_instance(instance),
    m_capacity(capacity),
    m_refCount(new size_t(1))
  {
  };

  SharedArray(const SharedArray &instance) :
    m_instance(instance.m_instance),
    m_capacity(instance.m_capacity),
    m_refCount(instance.m_refCount)
  {
    ++*m_refCount;
  }

  ~SharedArray()
  {
    if (*m_refCount == 1)
    {
      if (m_instance != NULL)
      {
        delete[] m_instance;
      }

      delete m_refCount;
    }
    else
    {
      (*m_refCount)--;
    }
  };

  inline T * GetPointer() const
  {
    return m_instance;
  }

  inline bool IsNull() const
  {
    return m_instance == NULL;
  }

  inline size_t GetCapacity() const
  {
    return m_capacity;
  }

  inline SharedArray &operator=(const SharedArray &instance)
  {
    this->~SharedArray();
    new (this) SharedArray(instance);

    return *this;
  }

  inline T& operator[](size_t index) const
  {
    return m_instance[index];
  }

  inline operator SharedArray<const T>()
  {
    return SharedArray<const T>(m_instance, m_refCount, m_capacity);
  }

private:
  template <class F>
  friend class SharedArray;

  T *m_instance;

  size_t m_capacity;

  size_t *m_refCount;

  SharedArray(T *instance, size_t *refCount, size_t capacity) :
    m_instance(instance),
    m_capacity(capacity),
    m_refCount(refCount)
  {
    ++*m_refCount;
  }
};