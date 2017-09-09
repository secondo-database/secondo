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

#include "AlmostEqual.h"
#include "MFsObjects.h"
#include "IStrings.h"

namespace ColumnMovingAlgebra
{
  class StringUnit;

  typedef MFsObjects<StringUnit> MStrings;

  class StringUnit
  {
  public:
    static const int STRING_SIZE = 49;

    struct Value {
      char s[STRING_SIZE];
      bool operator < (const Value & b) const;
    };
    typedef IStrings Instants;
    typedef CcString Attr;
    typedef temporalalgebra::RString RAttr;
    typedef temporalalgebra::MString MAttr;
    
    Value minimum() const;
    Value maximum() const;
    StringUnit() = default;
    StringUnit(temporalalgebra::MString mstring, int unit);
    StringUnit(Interval interval, std::string str);

    Interval interval() const;
    void appendTo(temporalalgebra::MString & mstring);
    int compareValue(const StringUnit & stringUnit);
    temporalalgebra::IString atInstant(Instant instant);
    StringUnit restrictToInterval(Interval unitInterval);
    bool passes(CcString ccString);
    bool passes(temporalalgebra::RString rString);
    void at(CcString ccString, MStrings & result);
    void at(temporalalgebra::RString rString, MStrings & result);
    
    static CcString undefinedAttr();
    static int compare(Value value, Attr attr);
    
  private:
    Interval m_Interval;
    char m_Value[STRING_SIZE];
  };



  inline bool StringUnit::Value::operator < (const Value & b) const
  {
    return strcmp(s, b.s) < 0;
  }

  inline StringUnit::StringUnit(temporalalgebra::MString mstring, int unit)
  {
    temporalalgebra::UString u;
    mstring.Get(unit, u);
    m_Interval = u.timeInterval;
    snprintf(m_Value, STRING_SIZE, "%s", u.constValue.GetValue().c_str());
  }

  inline StringUnit::StringUnit(Interval interval, std::string str) :
    m_Interval(interval)
  {
    snprintf(m_Value, STRING_SIZE, "%s", str.c_str());
  }

  inline StringUnit::Value StringUnit::minimum() const
  {
    Value v;
    strcpy(v.s, m_Value);
    return v;
  }
  
  inline StringUnit::Value StringUnit::maximum() const
  {
    Value v;
    strcpy(v.s, m_Value);
    return v;
  }
  
  inline Interval StringUnit::interval() const
  {
    return m_Interval;
  }
  
  inline void StringUnit::appendTo(
    temporalalgebra::MString & mstring)
  {
    mstring.Add(temporalalgebra::UString(m_Interval.convert(), 
      CcString(true, std::string(m_Value))));
  }
  
  inline int StringUnit::compareValue(const StringUnit & stringUnit)
  {
    return strcmp(m_Value, stringUnit.m_Value);
  }

  inline temporalalgebra::IString StringUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IString(instant, 
      CcString(true, std::string(m_Value)));
  }

  inline StringUnit StringUnit::restrictToInterval(Interval unitInterval)
  {
    StringUnit u;
    u.m_Interval = unitInterval;
    strcpy(u.m_Value, m_Value);
    return u;
  }

  inline bool StringUnit::passes(CcString ccString)
  {
    return strcmp(m_Value, ccString.GetValue().c_str()) == 0;
  }

  inline bool StringUnit::passes(temporalalgebra::RString rString)
  {
    return rString.Contains(CcString(true, std::string(m_Value)));
  }

  inline void StringUnit::at(CcString ccString, MStrings & result)
  {
    if (passes(ccString)) 
      result.addUnit(*this);
  }

  inline void StringUnit::at(temporalalgebra::RString rString, 
    MStrings & result)
  {
    if (passes(rString))
      result.addUnit(*this);
  }

  inline CcString StringUnit::undefinedAttr()
  {
    CcString r(false, std::string());
    r.SetDefined(false);
    return r;
  }
  
  inline int StringUnit::compare(Value value, Attr attr)
  {
    const char *b = attr.GetValue().c_str();
    int r = strcmp(value.s, b);;
    return r < 0 ? -1 : (r == 0 ? 0 : 1);
  }
}
