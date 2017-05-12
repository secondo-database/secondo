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

#include "Attribute.h"
#include <cstddef>
#include "NestedList.h"
#include "ReadWrite.h"
#include "SimpleAttrArray.h"
#include <stdint.h>
#include <string>

#include "StandardTypes.h"

namespace CRelAlgebra
{
  class IntEntry
  {
  public:
    static const IntEntry undefined;

    IntEntry();

    IntEntry(int32_t value);

    bool IsDefined() const;

    int Compare(const IntEntry &value) const;

    int Compare(Attribute &value) const;

    bool Equals(const IntEntry &value) const;

    bool Equals(Attribute &value) const;

    size_t GetHash() const;

    operator int32_t() const;

    int32_t value;
  };

  typedef SimpleFSAttrArrayIterator<IntEntry> IntsIterator;

  class Ints : public SimpleFSAttrArray<IntEntry>
  {
  public:
    Ints();

    Ints(Reader &source);

    Ints(Reader &source, size_t rowCount);

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new Ints(*this, filter);
    }

    //using SimpleFSAttrArray<IntEntry>::Append;
    void Append(const IntEntry &value)
    {
      SimpleFSAttrArray<IntEntry>::Append(value);
    }

    virtual void Append(Attribute &value);

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

  private:
    Ints(const Ints &array, const SharedArray<const size_t> &filter) :
      SimpleFSAttrArray<IntEntry>(array, filter)
    {
    }
  };

  class IntEntry2
  {
  public:
    static void *Cast(void *target);

    IntEntry2();

    IntEntry2(int32_t value);

    IntEntry2(const CcInt &value);

    bool IsDefined() const;

    int Compare(const IntEntry2 &value) const;

    int Compare(const Attribute &value) const;

    int Equals(const IntEntry2 &value) const;

    bool Equals(const Attribute &value) const;

    CcInt &GetValue() const;

    operator CcInt &() const;

    size_t GetHash() const;

  private:
    mutable CcInt m_value;
  };

  typedef SimpleFSAttrArrayIterator<IntEntry2, IntEntry2::Cast> Ints2Iterator;

  class Ints2 : public SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>
  {
  public:
    Ints2();

    Ints2(Reader &source);

    Ints2(Reader &source, size_t rowCount);

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new Ints2(*this, filter);
    }

    //using SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>::Append;
    void Append(const IntEntry2 &value)
    {
      SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>::Append(value);
    }

    virtual void Append(Attribute &value);

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

  private:
    Ints2(const Ints2 &array, const SharedArray<const size_t> &filter) :
      SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>(array, filter)
    {
    }
  };
}