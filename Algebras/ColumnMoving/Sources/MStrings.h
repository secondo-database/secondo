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

1 MStrings.h

*/

#pragma once

#include "AlmostEqual.h"
#include "MFsObjects.h"
#include "IStrings.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Forward Declaration of ~StringUnit~

~StringUnit~ represents a moving string unit

*/
  class StringUnit;

/*
1.1 Declaration of ~MStrings~

~MStrings~ represents a string boolean

*/
  typedef MFsObjects<StringUnit> MStrings;

/*
1.2 Declaration of class ~StringUnit~

*/
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
    
/*
constructors

*/
    StringUnit() = default;
    StringUnit(temporalalgebra::MString mstring, int unit);
    StringUnit(Interval interval, std::string str);

/*
~interval~ returns the definition interval of this unit

*/
    Interval interval() const;
/*
~minimum~ returns the minimum value of the mapping function during the
definition interval

*/
    Value minimum() const;
/*
~maximum~ returns the maximum value of the mapping function during the
definition interval

*/
    Value maximum() const;
/*
~appendTo~ adds this unit to a moving object in the temporal algebra

*/
    void appendTo(temporalalgebra::MString & mstring);
/*
~compareValue~ compares this unit

*/
    int compareValue(const StringUnit & stringUnit);
/*
~atInstant~ returns an intime for ~instant~

*/
    temporalalgebra::IString atInstant(Instant instant);
/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
    StringUnit restrictToInterval(Interval unitInterval);
/*
~passes~ returns true, iff this this unit has the same value 

*/
    bool passes(CcString ccString);
/*
or iff the value is in the specified range

*/
    bool passes(temporalalgebra::RString rString);
/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
    void at(CcString ccString, MStrings & result);
/*
of iff its value is in the specified range

*/
    void at(temporalalgebra::RString rString, MStrings & result);
    
/*
~undefinedAttr~ returns an undefined attribute

*/
    static CcString undefinedAttr();
/*
~compare~ 
returns -1 if ~value~ smaller ~attr~,
returns  0 if ~value~ equal ~attr~, 
returns  1 if ~value~ greater ~attr~ 

*/
    static int compare(Value value, Attr attr);
/*
~random~ returns a unit with the specified interval and a random value

*/
    static StringUnit random(Interval interval);
    
  private:
/*
~mInterval~ represents the definition interval of this unit

*/
    Interval m_Interval;
/*
~mValue~ represents the value of this unit

*/
    char m_Value[STRING_SIZE];
  };



/*
1.2 Implementation of class ~StringUnit~

comparision operator for string values

*/
  inline bool StringUnit::Value::operator < (const Value & b) const
  {
    return strcmp(s, b.s) < 0;
  }

/*
constructors

*/
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

/*
~minimum~ returns the minimum value of the mapping function during the
definition interval

*/
  inline StringUnit::Value StringUnit::minimum() const
  {
    Value v;
    strcpy(v.s, m_Value);
    return v;
  }
  
/*
~maximum~ returns the maximum value of the mapping function during the
definition interval

*/
  inline StringUnit::Value StringUnit::maximum() const
  {
    Value v;
    strcpy(v.s, m_Value);
    return v;
  }
  
/*
~interval~ returns the definition interval of this unit

*/
  inline Interval StringUnit::interval() const
  {
    return m_Interval;
  }
  
/*
~appendTo~ adds this unit to a moving object in the temporal algebra

*/
  inline void StringUnit::appendTo(
    temporalalgebra::MString & mstring)
  {
    mstring.Add(temporalalgebra::UString(m_Interval.convert(), 
      CcString(true, std::string(m_Value))));
  }
  
/*
~compareValue~ compares this unit

*/
  inline int StringUnit::compareValue(const StringUnit & stringUnit)
  {
    return strcmp(m_Value, stringUnit.m_Value);
  }

/*
~atInstant~ returns an intime for ~instant~

*/
  inline temporalalgebra::IString StringUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IString(instant, 
      CcString(true, std::string(m_Value)));
  }

/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
  inline StringUnit StringUnit::restrictToInterval(Interval unitInterval)
  {
    StringUnit u;
    u.m_Interval = unitInterval;
    strcpy(u.m_Value, m_Value);
    return u;
  }

/*
~passes~ returns true, iff this this unit has the same value 

*/
  inline bool StringUnit::passes(CcString ccString)
  {
    return strcmp(m_Value, ccString.GetValue().c_str()) == 0;
  }

/*
or iff the value is in the specified range

*/
  inline bool StringUnit::passes(temporalalgebra::RString rString)
  {
    return rString.Contains(CcString(true, std::string(m_Value)));
  }

/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
  inline void StringUnit::at(CcString ccString, MStrings & result)
  {
    if (passes(ccString)) 
      result.addUnit(*this);
  }

/*
of iff its value is in the specified range

*/
  inline void StringUnit::at(temporalalgebra::RString rString, 
    MStrings & result)
  {
    if (passes(rString))
      result.addUnit(*this);
  }

/*
~undefinedAttr~ returns an undefined attribute

*/
  inline CcString StringUnit::undefinedAttr()
  {
    CcString r(false, std::string());
    r.SetDefined(false);
    return r;
  }
  
/*
~compare~ 
returns -1 if ~value~ smaller ~attr~,
returns  0 if ~value~ equal ~attr~, 
returns  1 if ~value~ greater ~attr~ 

*/
  inline int StringUnit::compare(Value value, Attr attr)
  {
    const char *b = attr.GetValue().c_str();
    int r = strcmp(value.s, b);;
    return r < 0 ? -1 : (r == 0 ? 0 : 1);
  }
  
/*
~random~ returns a unit with the specified interval and a random value

*/
  inline StringUnit StringUnit::random(Interval interval)
  {
    return StringUnit(interval, std::to_string(rand()));
  }
}
