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
#include "TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
  class IntEntry
  {
  public:
    typedef temporalalgebra::IInt AttributeType;

    static const bool isPrecise = true;

  IntEntry() = default;
  IntEntry(bool defined, int64_t time, int value);
  IntEntry(const temporalalgebra::IInt &value);

  bool IsDefined() const;
  int64_t GetTime() const;
  int GetValue() const;

  int Compare(const IntEntry &value) const;
  int Compare(const temporalalgebra::IInt &value) const;
  bool Equals(const IntEntry &value) const;
  bool Equals(const temporalalgebra::IInt &value) const;

  size_t GetHash() const;

  temporalalgebra::IInt *GetAttribute(bool clone = true) const;

  private:
    
    bool m_Defined;
    int64_t m_Time;
    int m_Value;
  };

  typedef CRelAlgebra::SimpleFSAttrArray<IntEntry> IInts;
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IntEntry> IIntsIterator;


  inline IntEntry::IntEntry(bool defined, int64_t time, int value) :
    m_Defined(defined),
    m_Time(time),
    m_Value(value)
  {
  }

  inline IntEntry::IntEntry(const temporalalgebra::IInt &value) :
    m_Defined(value.IsDefined()),
    m_Time(value.instant.millisecondsToNull()),
    m_Value(value.value.GetIntval())
  {
  }

  inline bool IntEntry::IsDefined() const
  {
    return m_Defined;
  }

  inline int64_t IntEntry::GetTime() const {
    return m_Time;
  }

  inline int IntEntry::GetValue() const {
    return m_Value;
  }

  inline int IntEntry::Compare(const IntEntry &value) const
  {
    int iDiff = (m_Defined ? 1 : 0) - (value.m_Defined ? 1 : 0);
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    int64_t tDiff = m_Time - value.m_Time;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    iDiff = m_Value - value.m_Value;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    return 0;
  }

  inline int IntEntry::Compare(const temporalalgebra::IInt &value) const
  {
    IntEntry b(value);
    return Compare(b);
  }

  inline bool IntEntry::Equals(const IntEntry &value) const
  {
    return Compare(value) == 0;
  }

  inline bool IntEntry::Equals(const temporalalgebra::IInt &value) const
  {
    return Compare(value) == 0;
  }

  inline size_t IntEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value) ^ static_cast<size_t>(m_Time);
  }

  inline temporalalgebra::IInt *IntEntry::GetAttribute(bool clone) const
  {
    if (m_Defined)
      return new temporalalgebra::IInt(Instant(m_Time), CcInt(m_Value));
    else
      return new temporalalgebra::IInt(0);
  }

}
