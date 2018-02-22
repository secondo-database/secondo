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

1 IStrings.h

*/

#pragma once

#include "Algebras/CRel/SimpleAttrArray.h"
#include "Algebras/Temporal/TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of class IStringEntry 

~IStringEntry~ represents an intime string in an attribut array

*/

  class IStringEntry
  {
  public:
    typedef temporalalgebra::IString AttributeType;

    static const bool isPrecise = true;

/*
default constructors

*/
    IStringEntry() = default;
/*
constructor for specific time and value

*/
    IStringEntry(bool defined, int64_t time, std::string value);
/*
constructor for conversion of the corresponding attribute type

*/
    IStringEntry(const temporalalgebra::IString &value);

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
    std::string GetValue() const;

/*
~Compare~ compares with another attribut array entry

*/
    int Compare(const IStringEntry &value) const;
/*
or compares with the corresponding attribute type

*/
    int Compare(const temporalalgebra::IString &value) const;
/*
~Equals~ checks for equality with another attribut array entry

*/
    bool Equals(const IStringEntry &value) const;
/*
or checks for equality with with the corresponding attribute type

*/
    bool Equals(const temporalalgebra::IString &value) const;

/*
~GetHash~ returns a hash value for the entry

*/
    size_t GetHash() const;

/*
~GetAttribute~ converts the entry to the corresponding attribute type

*/
    temporalalgebra::IString *GetAttribute(bool clone = true) const;

  private:
    static const int STRING_SIZE = 49;
    
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
    char m_Value[STRING_SIZE];
  };

/*
1.2 Declaration of IStrings

*/
  typedef CRelAlgebra::SimpleFSAttrArray<IStringEntry> IStrings;
/*
1.2 Declaration of IStringsIterator

*/
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IStringEntry> IStringsIterator;
/*
1.3 Implementation of class IStringEntry 

constructor for specific time and value

*/
  inline IStringEntry::IStringEntry(bool defined, int64_t time, 
    std::string value) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time)
  {
    snprintf(m_Value, STRING_SIZE, "%s", value.c_str());
  }
/*
constructor for conversion of the corresponding attribute type

*/
  inline IStringEntry::IStringEntry(const temporalalgebra::IString &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull())
  {
    snprintf(m_Value, STRING_SIZE, "%s", value.value.GetValue().c_str());
  }
/*
~IsDefined~ returns wether the entry is defined

*/
  inline bool IStringEntry::IsDefined() const
  {
    return m_Defined;
  }
/*
~GetTime~ returns the time point represented by this entry

*/
  inline int64_t IStringEntry::GetTime() const {
    return m_Time;
  }
/*
~GetValue~ returns the value at this time point

*/
  inline std::string IStringEntry::GetValue() const {
    return m_Value;
  }
/*
~Compare~ compares with another attribut array entry

*/
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
/*
or compares with the corresponding attribute type

*/
  inline int IStringEntry::Compare(const temporalalgebra::IString &value) const
  {
    IStringEntry b(value);
    return Compare(b);
  }
/*
~Equals~ checks for equality with another attribut array entry

*/
  inline bool IStringEntry::Equals(const IStringEntry &value) const
  {
    return Compare(value) == 0;
  }
/*
or checks for equality with with the corresponding attribute type

*/
  inline bool IStringEntry::Equals(const temporalalgebra::IString &value) const
  {
    return Compare(value) == 0;
  }
/*
~GetHash~ returns a hash value for the entry

*/
  inline size_t IStringEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value[0]) ^ static_cast<size_t>(m_Time);
  }
/*
~GetAttribute~ conversion to the corresponding attribute type

*/
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
