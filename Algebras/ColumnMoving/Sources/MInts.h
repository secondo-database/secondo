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

1 MInts.h

*/

#pragma once

#include "AlmostEqual.h"
#include "MFsObjects.h"
#include "IInts.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Forward Declaration of ~IntUnit~

~IntUnit~ represents a moving integer unit

*/
  class IntUnit;

/*
1.1 Declaration of ~MInts~

~MInts~ represents a moving integer

*/
  typedef MFsObjects<IntUnit> MInts;

/*
1.2 Declaration of class ~IntUnit~

*/
  class IntUnit
  {
  public:
    typedef int Value;
    typedef IInts Instants;
    typedef CcInt Attr;
    typedef temporalalgebra::RInt RAttr;
    typedef temporalalgebra::MInt MAttr;
    
/*
constructors

*/
    IntUnit() = default;
    IntUnit(temporalalgebra::MInt mint, int unit);
    IntUnit(Interval interval, int m_Value);

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
    void appendTo(temporalalgebra::MInt & mint);
/*
~compareValue~ compares this unit

*/
    int compareValue(const IntUnit & intUnit);
/*
~atInstant~ returns an intime for ~instant~

*/
    temporalalgebra::IInt atInstant(Instant instant);
/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
    IntUnit restrictToInterval(Interval unitInterval);
/*
~passes~ returns true, iff this this unit has the same value 

*/
    bool passes(CcInt ccInt);
/*
or iff the value is in the specified range

*/
    bool passes(temporalalgebra::RInt rInt);
/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
    void at(CcInt ccInt, MInts & result);
/*
of iff its value is in the specified range

*/
    void at(temporalalgebra::RInt rInt, MInts & result);
    
/*
~undefinedAttr~ returns an undefined attribute

*/
    static CcInt undefinedAttr();
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
    static IntUnit random(Interval interval);

  private:
/*
~mInterval~ represents the definition interval of this unit

*/
    Interval m_Interval;
/*
~mValue~ represents the value of this unit

*/
    int m_Value;
  };



/*
1.2 Implementation of class ~IntUnit~

constructors

*/
  inline IntUnit::IntUnit(temporalalgebra::MInt mint, int unit)
  {
    temporalalgebra::UInt u;
    mint.Get(unit, u);
    m_Interval = u.timeInterval;
    m_Value = u.constValue.GetValue();
  }

  inline IntUnit::IntUnit(Interval interval, int value) :
    m_Interval(interval),
    m_Value(value)
  {
  }

/*
~minimum~ returns the minimum value of the mapping function during the
definition interval

*/
  inline IntUnit::Value IntUnit::minimum() const
  {
    return m_Value;
  }
  
/*
~maximum~ returns the maximum value of the mapping function during the
definition interval

*/
  inline IntUnit::Value IntUnit::maximum() const
  {
    return m_Value;
  }
  
/*
~interval~ returns the definition interval of this unit

*/
  inline Interval IntUnit::interval() const
  {
    return m_Interval;
  }
  
/*
~appendTo~ adds this unit to a moving object in the temporal algebra

*/
  inline void IntUnit::appendTo(
    temporalalgebra::MInt & mint)
  {
    mint.Add(temporalalgebra::UInt(m_Interval.convert(), CcInt(m_Value)));
  }
  
/*
~compareValue~ compares this unit

*/
  inline int IntUnit::compareValue(const IntUnit & intUnit)
  {
    int iDiff;

    iDiff = m_Value - intUnit.m_Value;
    if (iDiff != 0.0)
      return iDiff < 0.0 ? -1 : 1;

    return 0;
  }

/*
~atInstant~ returns an intime for ~instant~

*/
  inline temporalalgebra::IInt IntUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IInt(instant, CcInt(true, m_Value));
  }

/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
  inline IntUnit IntUnit::restrictToInterval(Interval unitInterval)
  {
    IntUnit u;
    u.m_Interval = unitInterval;
    u.m_Value = m_Value;
    return u;
  }

/*
~passes~ returns true, iff this this unit has the same value 

*/
  inline bool IntUnit::passes(CcInt ccInt)
  {
    return m_Value == ccInt.GetValue();
  }

/*
or iff the value is in the specified range

*/
  inline bool IntUnit::passes(temporalalgebra::RInt rInt)
  {
    for (int index = 0; index < rInt.GetNoComponents(); index++) {
      temporalalgebra::Interval<CcInt> ri;
      rInt.Get(index, ri);

      int s = ri.start.GetValue(), e = ri.end.GetValue();
      bool is = ri.lc, ie = ri.rc;
      
      if (m_Value < s || (m_Value == s && !is))
        continue;
      
      if (m_Value > e || (m_Value == e && !ie))
        continue;
        
      return true;
    }
    
    return false;
  }

/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
  inline void IntUnit::at(CcInt ccInt, MInts & result)
  {
    if (passes(ccInt)) 
      result.addUnit(*this);
  }

/*
of iff its value is in the specified range

*/
  inline void IntUnit::at(temporalalgebra::RInt rInt, MInts & result)
  {
    if (passes(rInt))
      result.addUnit(*this);
  }

/*
~undefinedAttr~ returns an undefined attribute

*/
  inline CcInt IntUnit::undefinedAttr()
  {
    CcInt r(1);
    r.SetDefined(false);
    return r;
  }

/*
~compare~ 
returns -1 if ~value~ smaller ~attr~,
returns  0 if ~value~ equal ~attr~, 
returns  1 if ~value~ greater ~attr~ 

*/
  inline int IntUnit::compare(Value value, Attr attr)
  {
    int b = attr.GetValue();
    return value < b ? -1 : (value == b ? 0 : 1);
  }
  
/*
~random~ returns a unit with the specified interval and a random value

*/
  inline IntUnit IntUnit::random(Interval interval)
  {
    return IntUnit(interval, rand());
  }
}
