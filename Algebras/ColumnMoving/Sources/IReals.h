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
  class IRealEntry 
  {
  public:
    typedef temporalalgebra::IReal AttributeType;

    static const bool isPrecise = true;

    IRealEntry() = default;
    IRealEntry(bool defined, int64_t time, double value);
    IRealEntry(const temporalalgebra::IReal &value);

    bool IsDefined() const;
    int64_t GetTime() const;
    double GetValue() const;

    int Compare(const IRealEntry &value) const;
    int Compare(const temporalalgebra::IReal &value) const;
    bool Equals(const IRealEntry &value) const;
    bool Equals(const temporalalgebra::IReal &value) const;

    size_t GetHash() const;

    temporalalgebra::IReal *GetAttribute(bool clone = true) const;

  private:
    
    bool m_Defined, m_ValueDefined;
    int64_t m_Time;
    double m_Value;
  };

  typedef CRelAlgebra::SimpleFSAttrArray<IRealEntry> IReals;
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IRealEntry> IRealsIterator;


  inline IRealEntry::IRealEntry(bool defined, int64_t time, double value) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time),
    m_Value(value)
  {
  }

  inline IRealEntry::IRealEntry(const temporalalgebra::IReal &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull()),
    m_Value(value.value.GetValue())
  {
  }

  inline bool IRealEntry::IsDefined() const
  {
    return m_Defined;
  }

  inline int64_t IRealEntry::GetTime() const {
    return m_Time;
  }

  inline double IRealEntry::GetValue() const {
    return m_Value;
  }

  inline int IRealEntry::Compare(const IRealEntry &value) const
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

    double dDiff = m_Value - value.m_Value;
    if (dDiff != 0)
      return dDiff < 0 ? -1 : 1;

    return 0;
  }

  inline int IRealEntry::Compare(const temporalalgebra::IReal &value) const
  {
    IRealEntry b(value);
    return Compare(b);
  }

  inline bool IRealEntry::Equals(const IRealEntry &value) const
  {
    return Compare(value) == 0;
  }

  inline bool IRealEntry::Equals(const temporalalgebra::IReal &value) const
  {
    return Compare(value) == 0;
  }

  inline size_t IRealEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value) ^ static_cast<size_t>(m_Time);
  }

  inline temporalalgebra::IReal *IRealEntry::GetAttribute(bool clone) const
  {
    if (!m_Defined)
      return new temporalalgebra::IReal(false);
      
    if (!m_ValueDefined) {
      CcReal r(2.0);
      r.SetDefined(false);
      return new temporalalgebra::IReal(Instant(m_Time), r);
    }
        
    return new temporalalgebra::IReal(Instant(m_Time), CcReal(m_Value));
  }

}
