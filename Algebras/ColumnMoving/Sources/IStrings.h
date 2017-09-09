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
  class IStringEntry
  {
  public:
    typedef temporalalgebra::IString AttributeType;

    static const bool isPrecise = true;

    IStringEntry() = default;
    IStringEntry(bool defined, int64_t time, std::string value);
    IStringEntry(const temporalalgebra::IString &value);

    bool IsDefined() const;
    int64_t GetTime() const;
    std::string GetValue() const;

    int Compare(const IStringEntry &value) const;
    int Compare(const temporalalgebra::IString &value) const;
    bool Equals(const IStringEntry &value) const;
    bool Equals(const temporalalgebra::IString &value) const;

    size_t GetHash() const;

    temporalalgebra::IString *GetAttribute(bool clone = true) const;

  private:
    static const int STRING_SIZE = 49;
    
    bool m_Defined, m_ValueDefined;
    int64_t m_Time;
    char m_Value[STRING_SIZE];
  };

  typedef CRelAlgebra::SimpleFSAttrArray<IStringEntry> IStrings;
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IStringEntry> IStringsIterator;


  inline IStringEntry::IStringEntry(bool defined, int64_t time, 
    std::string value) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time)
  {
    snprintf(m_Value, STRING_SIZE, "%s", value.c_str());
  }

  inline IStringEntry::IStringEntry(const temporalalgebra::IString &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull())
  {
    snprintf(m_Value, STRING_SIZE, "%s", value.value.GetValue().c_str());
  }

  inline bool IStringEntry::IsDefined() const
  {
    return m_Defined;
  }

  inline int64_t IStringEntry::GetTime() const {
    return m_Time;
  }

  inline std::string IStringEntry::GetValue() const {
    return m_Value;
  }

  inline int IStringEntry::Compare(const IStringEntry &value) const
  {
    if (!m_Defined)
      return !value.m_Defined ? 0 : -1;
    else if (!value.m_Defined)
      return 1;

    int64_t tDiff = m_Time - value.m_Time;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    if (!m_ValueDefined)
      return !value.m_ValueDefined ? 0 : -1;
    else if (!value.m_ValueDefined)
      return 1;

    return strcmp(m_Value, value.m_Value);
  }

  inline int IStringEntry::Compare(const temporalalgebra::IString &value) const
  {
    IStringEntry b(value);
    return Compare(b);
  }

  inline bool IStringEntry::Equals(const IStringEntry &value) const
  {
    return Compare(value) == 0;
  }

  inline bool IStringEntry::Equals(const temporalalgebra::IString &value) const
  {
    return Compare(value) == 0;
  }

  inline size_t IStringEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value[0]) ^ static_cast<size_t>(m_Time);
  }

  inline temporalalgebra::IString *IStringEntry::GetAttribute(bool clone) const
  {
    if (!m_Defined)
      return new temporalalgebra::IString(false);
      
    if (!m_ValueDefined) {
      CcString r(false, std::string());
      r.SetDefined(false);
      return new temporalalgebra::IString(Instant(m_Time), r);
    }
        
    return new temporalalgebra::IString(Instant(m_Time), 
      CcString(true, std::string(m_Value)));
  }

}
