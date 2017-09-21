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

1 IReals.h

*/

#pragma once

#include "SimpleAttrArray.h"
#include "TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of class IRealEntry 

~IRealEntry~ represents an intime real in an attribut array

*/

  class IRealEntry 
  {
  public:
    typedef temporalalgebra::IReal AttributeType;

    static const bool isPrecise = true;

/*
default constructors

*/
    IRealEntry() = default;
/*
constructor for specific time and value

*/
    IRealEntry(bool defined, int64_t time, double value);
/*
constructor for conversion of the corresponding attribute type

*/
    IRealEntry(const temporalalgebra::IReal &value);

/*
~IsDefined~ returns wether the entry is defined

*/
    bool IsDefined() const;
/*
~GetTime~ returns the time point represented by this entry

*/
    int64_t GetTime() const;
/*
~GetValue~ returns the value at this time point

*/
    double GetValue() const;

/*
~Compare~ compares with another attribut array entry

*/
    int Compare(const IRealEntry &value) const;
/*
or compares with the corresponding attribute type

*/
    int Compare(const temporalalgebra::IReal &value) const;
/*
~Equals~ checks for equality with another attribut array entry

*/
    bool Equals(const IRealEntry &value) const;
/*
or checks for equality with with the corresponding attribute type

*/
    bool Equals(const temporalalgebra::IReal &value) const;

/*
~GetHash~ returns a hash value for the entry

*/
    size_t GetHash() const;

/*
~GetAttribute~ converts the entry to the corresponding attribute type

*/
    temporalalgebra::IReal *GetAttribute(bool clone = true) const;

  private:
    
/*
~mDefined~ determines whether the entry is defined
~mValueDefined~ dertermines whether the value of the entry is defined

*/
    bool m_Defined, m_ValueDefined;
/*
~mTime~ represents the time, if ~mDefined~ is true

*/
    int64_t m_Time;
/*
~mValue~ represents the value, if ~mValueDefined~ is true

*/
    double m_Value;
  };

/*
1.2 Declaration of IReals

*/
  typedef CRelAlgebra::SimpleFSAttrArray<IRealEntry> IReals;
/*
1.2 Declaration of IRealsIterator

*/
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IRealEntry> IRealsIterator;
/*
1.3 Implementation of class IRealEntry 

constructor for specific time and value

*/
  inline IRealEntry::IRealEntry(bool defined, int64_t time, double value) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time),
    m_Value(value)
  {
  }
/*
constructor for conversion of the corresponding attribute type

*/
  inline IRealEntry::IRealEntry(const temporalalgebra::IReal &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull()),
    m_Value(value.value.GetValue())
  {
  }
/*
~IsDefined~ returns wether the entry is defined

*/
  inline bool IRealEntry::IsDefined() const
  {
    return m_Defined;
  }
/*
~GetTime~ returns the time point represented by this entry

*/
  inline int64_t IRealEntry::GetTime() const {
    return m_Time;
  }
/*
~GetValue~ returns the value at this time point

*/
  inline double IRealEntry::GetValue() const {
    return m_Value;
  }
/*
~Compare~ compares with another attribut array entry

*/
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
/*
or compares with the corresponding attribute type

*/
  inline int IRealEntry::Compare(const temporalalgebra::IReal &value) const
  {
    IRealEntry b(value);
    return Compare(b);
  }
/*
~Equals~ checks for equality with another attribut array entry

*/
  inline bool IRealEntry::Equals(const IRealEntry &value) const
  {
    return Compare(value) == 0;
  }
/*
or checks for equality with with the corresponding attribute type

*/
  inline bool IRealEntry::Equals(const temporalalgebra::IReal &value) const
  {
    return Compare(value) == 0;
  }
/*
~GetHash~ returns a hash value for the entry

*/
  inline size_t IRealEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value) ^ static_cast<size_t>(m_Time);
  }
/*
~GetAttribute~ conversion to the corresponding attribute type

*/
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
