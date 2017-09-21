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

1 MBools.h

*/

#pragma once

#include "AlmostEqual.h"
#include "MFsObjects.h"
#include "IBools.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Forward Declaration of ~BoolUnit~

~BoolUnit~ represents a moving boolean unit

*/
  class BoolUnit;

/*
1.1 Declaration of ~MBools~

~MBools~ represents a moving boolean

*/
  typedef MFsObjects<BoolUnit> MBools;

/*
1.2 Declaration of class ~BoolUnit~

*/
  class BoolUnit
  {
  public:
    typedef bool Value;
    typedef IBools Instants;
    typedef CcBool Attr;
    typedef temporalalgebra::RBool RAttr;
    typedef temporalalgebra::MBool MAttr;
    
/*
constructors

*/
    BoolUnit() = default;
    BoolUnit(temporalalgebra::MBool mint, int unit);
    BoolUnit(Interval interval, bool m_Value);

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
    void appendTo(temporalalgebra::MBool & mbool);
/*
~compareValue~ compares this unit

*/
    int compareValue(const BoolUnit & boolUnit);
/*
~atInstant~ returns an intime for ~instant~

*/
    temporalalgebra::IBool atInstant(Instant instant);
/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
    BoolUnit restrictToInterval(Interval unitInterval);
/*
~passes~ returns true, iff this this unit has the same value 

*/
    bool passes(CcBool ccBool);
/*
or iff the value is in the specified range

*/
    bool passes(temporalalgebra::RBool rBool);
/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
    void at(CcBool ccBool, MBools & result);
/*
of iff its value is in the specified range

*/
    void at(temporalalgebra::RBool rBool, MBools & result);
    
/*
~undefinedAttr~ returns an undefined attribute

*/
    static CcBool undefinedAttr();
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
    static BoolUnit random(Interval interval);
    
  private:
/*
~mInterval~ represents the definition interval of this unit

*/
    Interval m_Interval;
/*
~mValue~ represents the value of this unit

*/
    bool m_Value;
  };



/*
1.2 Implementation of class ~BoolUnit~

constructors

*/
  inline BoolUnit::BoolUnit(temporalalgebra::MBool mbool, int unit)
  {
    temporalalgebra::UBool u;
    mbool.Get(unit, u);
    m_Interval = u.timeInterval;
    m_Value = u.constValue.GetValue();
  }

  inline BoolUnit::BoolUnit(Interval interval, bool m_Value) :
    m_Interval(interval),
    m_Value(m_Value)
  {
  }

/*
~interval~ returns the definition interval of this unit

*/
  inline Interval BoolUnit::interval() const
  {
    return m_Interval;
  }
  
/*
~minimum~ returns the minimum value of the mapping function during the
definition interval

*/
  inline BoolUnit::Value BoolUnit::minimum() const
  {
    return m_Value;
  }
  
/*
~maximum~ returns the maximum value of the mapping function during the
definition interval

*/
  inline BoolUnit::Value BoolUnit::maximum() const
  {
    return m_Value;
  }
  
/*
~appendTo~ adds this unit to a moving object in the temporal algebra

*/
  inline void BoolUnit::appendTo(
    temporalalgebra::MBool & mbool)
  {
    mbool.Add(temporalalgebra::UBool(m_Interval.convert(), 
                                     CcBool(true, m_Value)));
  }
  
/*
~compareValue~ compares this unit

*/
  inline int BoolUnit::compareValue(const BoolUnit & boolUnit)
  {
    if (m_Value)
      return boolUnit.m_Value ? 0 : 1;
    else
      return boolUnit.m_Value ? -1 : 0;
  }

/*
~atInstant~ returns an intime for ~instant~

*/
  inline temporalalgebra::IBool BoolUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IBool(instant, CcBool(true, m_Value));
  }

/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
  inline BoolUnit BoolUnit::restrictToInterval(Interval unitInterval)
  {
    BoolUnit u;
    u.m_Interval = unitInterval;
    u.m_Value = m_Value;
    return u;
  }

/*
~passes~ returns true, iff this this unit has the same value 

*/
  inline bool BoolUnit::passes(CcBool ccBool)
  {
    return m_Value == ccBool.GetValue();
  }

/*
or iff the value is in the specified range

*/
  inline bool BoolUnit::passes(temporalalgebra::RBool rBool)
  {
    return rBool.Contains(CcBool(true, m_Value));
  }

/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
  inline void BoolUnit::at(CcBool ccBool, MBools & result)
  {
    if (passes(ccBool)) 
      result.addUnit(*this);
  }

/*
of iff its value is in the specified range

*/
  inline void BoolUnit::at(temporalalgebra::RBool rBool, MBools & result)
  {
    if (passes(rBool))
      result.addUnit(*this);
  }

/*
~undefinedAttr~ returns an undefined attribute

*/
  inline CcBool BoolUnit::undefinedAttr()
  {
    CcBool r(false);
    r.SetDefined(false);
    return r;
  }
  
/*
~compare~ 
returns -1 if ~value~ smaller ~attr~,
returns  0 if ~value~ equal ~attr~, 
returns  1 if ~value~ greater ~attr~ 

*/
  inline int BoolUnit::compare(Value value, Attr attr)
  {
    bool b = attr.GetValue();
    return value < b ? -1 : (value == b ? 0 : 1);
  }
  
/*
~random~ returns a unit with the specified interval and a random value

*/
  inline BoolUnit BoolUnit::random(Interval interval)
  {
    return BoolUnit(interval, (rand() % 2) == 1);
  }
}
