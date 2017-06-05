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

#include <limits>
#include "LongInt.h"
#include "SimpleAttrArray.h"
#include "StandardTypes.h"
#include <stdint.h>

namespace CRelAlgebra
{
  template<class V = int32_t, class A = CcInt>
  class IntEntry
  {
  public:
    typedef A AttributeType;

    static const bool isPrecise = true;

    IntEntry()
    {
    }

    IntEntry(V value) :
      value(value)
    {
    }

    IntEntry(const A &value) :
      value(value.IsDefined() ? (V)value.GetValue() : undefined)
    {
    }

    bool IsDefined() const
    {
      return value != undefined;
    }

    int Compare(const IntEntry &value) const
    {
      return this->value < value.value ? -1 : this->value > value.value ? 1 : 0;
    }

    int Compare(const A &value) const
    {
      if (value.IsDefined())
      {
        if (this->value == undefined)
        {
          return -1;
        }

        const V intValue = value.GetValue();

        return this->value < intValue ? -1 : this->value > intValue ? 1 : 0;
      }

      return this->value == undefined ? 0 : 1;
    }

    bool Equals(const IntEntry &value) const
    {
      return this->value == value.value;
    }

    bool Equals(const A &value) const
    {
      if (value.IsDefined())
      {
        return this->value != undefined && this->value == value.GetValue();
      }

      return this->value == undefined;
    }

    uint64_t GetHash() const
    {
      return value;
    }

    A *GetAttribute(bool clone = true) const
    {
      return new A(value != undefined, value);
    }

    operator V() const
    {
      return value;
    }

    V value;

  private:
    static const V undefined = std::numeric_limits<V>::min();
  };

  typedef SimpleFSAttrArray<IntEntry<int32_t, CcInt>> Ints;
  typedef SimpleFSAttrArrayIterator<IntEntry<int32_t, CcInt>> IntsIterator;

  typedef IntEntry<int64_t, LongInt> LongIntEntry;
  typedef SimpleFSAttrArray<LongIntEntry> LongInts;
  typedef SimpleFSAttrArrayIterator<LongIntEntry> LongIntsIterator;
}