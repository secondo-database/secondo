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
#include <cstdint>
#include <cstring>
#include "Algebras/FText/FTextAlgebra.h"
#include "SimpleAttrArray.h"
#include "StandardTypes.h"

namespace CRelAlgebra
{
  template<class A = CcString>
  class StringEntry
  {
  public:
    typedef A AttributeType;

    static const bool isPrecise = true;

    static uint64_t GetSize(const A &value)
    {
      if (value.IsDefined())
      {
        return value.Length() + 1;
      }

      return 0;
    }

    static void Write(SimpleVSAttrArrayEntry target, const CcString &value)
    {
      if (value.IsDefined())
      {
        const uint64_t length = target.size - 1;

        memcpy(target.data, *value.GetStringval(), length);

        target.data[length] = '\0';
      }
    }

    static void Write(SimpleVSAttrArrayEntry target, const FText &value)
    {
      if (value.IsDefined())
      {
        const uint64_t length = target.size - 1;

        memcpy(target.data, value.Get(), length);

        target.data[length] = '\0';
      }

      return;
    }

    StringEntry()
    {
    }

    StringEntry(const SimpleVSAttrArrayEntry &value) :
      data(value.data),
      size(value.size)
    {
    }

    bool IsDefined() const
    {
      return size > 0;
    }

    int Compare(const StringEntry &value) const
    {
      const uint64_t sizeA = size,
        sizeB = value.size;

      if (sizeA > sizeB)
      {
        return sizeB == 0 ? 1 : strcmp(data, value.data);
      }
      else
      {
        return sizeA == 0 ? sizeB == 0 ? 0 : -1 : strcmp(data, value.data);
      }
    }

    int Compare(const CcString &value) const
    {
      if (value.IsDefined())
      {
        if (size == 0)
        {
          return -1;
        }

        return strcmp(data, *value.GetStringval());
      }

      return size > 0 ? 1 : 0;
    }

    int Compare(const FText &value) const
    {
      if (value.IsDefined())
      {
        if (size == 0)
        {
          return -1;
        }

        return strcmp(data, value.Get());
      }

      return size > 0 ? 1 : 0;
    }
    

    template<class Z>
    int CompareAlmost(const Z& z) const{
      return Compare(z);
    }

    bool Equals(const StringEntry &value) const
    {
      return Compare(value) == 0;
    }

    bool Equals(const CcString &value) const
    {
      return Compare(value) == 0;
    }



    bool Equals(const FText &value) const
    {
      return Compare(value) == 0;
    }
    
    template<class Z>
    bool EqualsAlmost(const Z& z) const{
      return Equals(z);
    }

    uint64_t GetHash() const
    {
      uint64_t h = 0;

      for (uint64_t i = 0; i < size; ++i)
      {
        h = 5 * h + data[i];
      }

      return h;
    }

    A *GetAttribute(bool clone = true) const
    {
      return size > 0 ? new A(true, data) : new A(false);
    }

  private:
    const char *data;

    uint64_t size;
  };

  typedef SimpleVSAttrArray<StringEntry<CcString>> Strings;

  typedef StringEntry<FText> TextEntry;
  typedef SimpleVSAttrArray<TextEntry> Texts;
}
