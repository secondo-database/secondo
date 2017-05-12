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

namespace CRelAlgebra
{
  template<bool text = false>
  class StringEntry
  {
  public:
    StringEntry();

    StringEntry(const char *data, size_t size);

    bool IsDefined() const;

    int Compare(const StringEntry<text> &value) const;

    int Compare(Attribute &value) const;

    bool Equals(const StringEntry<text> &value) const;

    bool Equals(Attribute &value) const;

    size_t GetHash() const;

    size_t GetSize() const;

    const char *GetData() const;

    operator const char *() const;

    const char *data;

    size_t size;
  };

  template<bool text = false>
  class Strings : public SimpleVSAttrArray<StringEntry<text>>
  {
  public:
    Strings();

    Strings(Reader &source);

    Strings(Reader &source, size_t rowCount);

    virtual AttrArray *Filter(SharedArray<const size_t> filter) const
    {
      return new Strings(*this, filter);
    }

    //using SimpleFSAttrArray<StringEntry>::Append;
    void Append(const StringEntry<text> &value)
    {
      SimpleVSAttrArray<StringEntry<text>>::Append(value);
    }

    virtual void Append(Attribute &value);

    virtual Attribute *GetAttribute(size_t row, bool clone) const;

  private:
    Strings(const Strings &array, const SharedArray<const size_t> &filter) :
      SimpleVSAttrArray<StringEntry<text>>(array, filter)
    {
    }
  };

  typedef StringEntry<true> TextEntry;
  typedef Strings<true> Texts;
}