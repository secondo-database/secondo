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

namespace CRelAlgebra
{
  /*
  The class ~Shared<T>~ implements a templated smart pointer.

  */
  template <class T>
  class Shared
  {
  public:
    Shared() :
      m_instance(nullptr),
      m_refCount(nullptr)
    {
    };

    Shared(const T &instance) :
      m_instance(new T(instance)),
      m_refCount(nullptr)
    {
    };

    Shared(T *instance) :
      m_instance(instance),
      m_refCount(nullptr)
    {
    };

    Shared(const Shared &instance) :
      m_instance(instance.m_instance),
      m_refCount(instance.m_refCount)
    {
      if (m_instance != nullptr)
      {
        if (m_refCount == nullptr)
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

    template<class F>
    Shared(const Shared<F> &instance) :
      m_instance(instance.m_instance),
      m_refCount(instance.m_refCount)
    {
      if (m_instance != nullptr)
      {
        if (m_refCount == nullptr)
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
      if (m_instance != nullptr)
      {
        if (m_refCount == nullptr)
        {
          delete m_instance;
        }
        else if (*m_refCount == 1)
        {
          delete m_instance;
          delete m_refCount;
        }
        else
        {
          --*m_refCount;
        }
      }
    };

    T * GetPointer() const
    {
      return m_instance;
    }

    bool IsNull() const
    {
      return m_instance == nullptr;
    }

    size_t GetCount()
    {
      return m_refCount != nullptr ? *m_refCount : 1;
    }

    Shared &operator=(const Shared &instance)
    {
      this->~Shared();
      new (this) Shared(instance);

      return *this;
    }

    Shared &operator=(T *instance)
    {
      this->~Shared();
      new (this) Shared(instance);

      return *this;
    }

    T *operator->() const
    {
      return m_instance;
    }

    T &operator*() const
    {
      return *m_instance;
    }

    template<class F>
    operator Shared<F>()
    {
      return Shared<F>(*this);
    }

  private:
    template <class F>
    friend class Shared;

    T *m_instance;

    mutable size_t *m_refCount;
  };

  /*
  The class ~SharedArray<T>~ implements a templated smart array pointer.
  For convenience it also stores the size of the array.

  */
  template <class T>
  class SharedArray
  {
  public:
    /*


    */
    SharedArray() :
      m_instance(nullptr),
      m_capacity(0),
      m_refCount(nullptr)
    {
    };

    SharedArray(size_t capacity) :
      m_instance(capacity > 0 ? new T[capacity] : nullptr),
      m_capacity(capacity),
      m_refCount(nullptr)
    {
    };

    SharedArray(T *instance, size_t capacity) :
      m_instance(instance),
      m_capacity(capacity),
      m_refCount(nullptr)
    {
    };

    SharedArray(const SharedArray &instance) :
      m_instance(instance.m_instance),
      m_capacity(instance.m_capacity),
      m_refCount(instance.m_refCount)
    {
      if (m_refCount != nullptr)
      {
        ++*m_refCount;
      }
      else
      {
        m_refCount = instance.m_refCount = new size_t(2);
      }
    }

    template<class F>
    SharedArray(const SharedArray<F> &instance) :
      m_instance(instance.m_instance),
      m_capacity(instance.m_capacity),
      m_refCount(instance.m_refCount)
    {
      if (m_refCount != nullptr)
      {
        ++*m_refCount;
      }
      else
      {
        m_refCount = instance.m_refCount = new size_t(2);
      }
    }

    ~SharedArray()
    {
      if (m_refCount == nullptr)
      {
        if (m_instance != nullptr)
        {
          delete[] m_instance;
        }
      }
      else if (*m_refCount == 1)
      {
        if (m_instance != nullptr)
        {
          delete[] m_instance;
        }

        delete m_refCount;
      }
      else
      {
        --*m_refCount;
      }
    };

    T * GetPointer() const
    {
      return m_instance;
    }

    bool IsNull() const
    {
      return m_instance == nullptr;
    }

    size_t GetCapacity() const
    {
      return m_capacity;
    }

    SharedArray &operator=(const SharedArray &instance)
    {
      this->~SharedArray();
      new (this) SharedArray(instance);

      return *this;
    }

    T& operator[](size_t index) const
    {
      return m_instance[index];
    }

    operator SharedArray<const T>()
    {
      return SharedArray<const T>(*this);
    }

  private:
    template <class F>
    friend class SharedArray;

    T *m_instance;

    size_t m_capacity;

    mutable size_t *m_refCount;
  };
}