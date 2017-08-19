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

#include "SimpleAttrArray.h"
#include "StandardTypes.h"

namespace ColumnMovingAlgebra
{
  class BoolEntry
  {
  public:
    enum Values { UNDEFINED, DEFINED_FALSE, DEFINED_TRUE };
    
    typedef CcBool AttributeType;

    static const bool isPrecise = true;

    BoolEntry()
    {
    }

    BoolEntry(Values value) :
      m_Value(value)
    {
    }

    BoolEntry(const CcBool &value) :
      m_Value(value.IsDefined() ? 
        (value.GetValue() ? DEFINED_TRUE : DEFINED_FALSE) : 
        UNDEFINED)
    {
    }

    bool IsDefined() const
    {
      return m_Value != UNDEFINED;
    }

    int Compare(const BoolEntry &value) const
    {
      int d = this->m_Value - value.m_Value;
      if (d != 0)
        return d < 0 ? -1 : 1;
        
      return 0;
    }

    int Compare(const CcBool &value) const
    {
      BoolEntry b(value);
      return Compare(b);
    }

    bool Equals(const BoolEntry &value) const
    {
      return Compare(value) == 0;
    }

    bool Equals(const CcBool &value) const
    {
      return Compare(value) == 0;
    }

    size_t GetHash() const
    {
      return m_Value;
    }

    CcBool *GetAttribute(bool clone = true) const
    {
      return new CcBool(m_Value != UNDEFINED, m_Value == DEFINED_TRUE);
    }

    operator bool() const
    {
      return m_Value == DEFINED_TRUE;
    }

    Values m_Value;
  };

  typedef CRelAlgebra::SimpleFSAttrArray<BoolEntry> Bools;
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<BoolEntry> BoolsIterator;
}
