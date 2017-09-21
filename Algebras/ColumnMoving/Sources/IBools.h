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

1 IBools.h

*/

#pragma once

#include "SimpleAttrArray.h"
#include "TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of class IBoolEntry 

~IBoolEntry~ represents an intime bool in an attribut array

*/

  class IBoolEntry
  {
  public:
    typedef temporalalgebra::IBool AttributeType;

    static const bool isPrecise = true;

/*
default constructors

*/
    IBoolEntry() = default;
/*
constructor for specific time and value

*/
    IBoolEntry(bool defined, int64_t time, bool value);
/*
constructor for conversion of the corresponding attribute type

*/
    IBoolEntry(const temporalalgebra::IBool &value);

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
    bool GetValue() const;

/*
~Compare~ compares with another attribut array entry

*/
    int Compare(const IBoolEntry &value) const;
/*
or compares with the corresponding attribute type

*/
    int Compare(const temporalalgebra::IBool &value) const;
/*
~Equals~ checks for equality with another attribut array entry

*/
    bool Equals(const IBoolEntry &value) const;
/*
or checks for equality with with the corresponding attribute type

*/
    bool Equals(const temporalalgebra::IBool &value) const;

/*
~GetHash~ returns a hash value for the entry

*/
    size_t GetHash() const;

/*
~GetAttribute~ conversion to the corresponding attribute type

*/
    temporalalgebra::IBool *GetAttribute(bool clone = true) const;

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
    bool m_Value;
  };

/*
1.2 Declaration of IBools

*/
  typedef CRelAlgebra::SimpleFSAttrArray<IBoolEntry> IBools;
/*
1.2 Declaration of IBoolsIterator

*/
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IBoolEntry> IBoolsIterator;
/*
1.1 Implementation of class IBoolEntry 

constructor for specific time and value

*/
  inline IBoolEntry::IBoolEntry(bool defined, int64_t time, bool value) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time),
    m_Value(value)
  {
  }
/*
constructor for conversion of the corresponding attribute type

*/
  inline IBoolEntry::IBoolEntry(const temporalalgebra::IBool &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull()),
    m_Value(value.value.GetValue())
  {
  }
/*
~IsDefined~ returns wether the entry is defined

*/
  inline bool IBoolEntry::IsDefined() const
  {
    return m_Defined;
  }
/*
~GetTime~ returns the time point represented by this entry

*/
  inline int64_t IBoolEntry::GetTime() const {
    return m_Time;
  }
/*
~GetValue~ returns the value at this time point

*/
  inline bool IBoolEntry::GetValue() const {
    return m_Value;
  }
/*
~Compare~ compares with another attribut array entry

*/
  inline int IBoolEntry::Compare(const IBoolEntry &value) const
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

    int iDiff = (m_Value ? 1 : 0) - (value.m_Value ? 1 : 0);
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    return 0;
  }
/*
or compares with the corresponding attribute type

*/
  inline int IBoolEntry::Compare(const temporalalgebra::IBool &value) const
  {
    IBoolEntry b(value);
    return Compare(b);
  }
/*
~Equals~ checks for equality with another attribut array entry

*/
  inline bool IBoolEntry::Equals(const IBoolEntry &value) const
  {
    return Compare(value) == 0;
  }
/*
or checks for equality with with the corresponding attribute type

*/
  inline bool IBoolEntry::Equals(const temporalalgebra::IBool &value) const
  {
    return Compare(value) == 0;
  }
/*
~GetHash~ returns a hash value for the entry

*/
  inline size_t IBoolEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value) ^ static_cast<size_t>(m_Time);
  }
/*
~GetAttribute~ conversion to the corresponding attribute type

*/
  inline temporalalgebra::IBool *IBoolEntry::GetAttribute(bool clone) const
  {
    if (!m_Defined)
      return new temporalalgebra::IBool(false);
      
    if (!m_ValueDefined) {
      CcBool r(false);
      r.SetDefined(false);
      return new temporalalgebra::IBool(Instant(m_Time), r);
    }
        
    return new temporalalgebra::IBool(Instant(m_Time), CcBool(true, m_Value));
  }

}
