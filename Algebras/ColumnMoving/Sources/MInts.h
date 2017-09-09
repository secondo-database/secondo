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
#include "IInts.h"

namespace ColumnMovingAlgebra
{
  class IntUnit;

  typedef MFsObjects<IntUnit> MInts;

  class IntUnit
  {
  public:
    typedef int Value;
    typedef IInts Instants;
    typedef CcInt Attr;
    typedef temporalalgebra::RInt RAttr;
    typedef temporalalgebra::MInt MAttr;
    
    IntUnit() = default;
    IntUnit(temporalalgebra::MInt mint, int unit);
    IntUnit(Interval interval, int m_Value);

    Value minimum() const;
    Value maximum() const;
    Interval interval() const;
    void appendTo(temporalalgebra::MInt & mint);
    int compareValue(const IntUnit & intUnit);
    temporalalgebra::IInt atInstant(Instant instant);
    IntUnit restrictToInterval(Interval unitInterval);
    bool passes(CcInt ccInt);
    bool passes(temporalalgebra::RInt rInt);
    void at(CcInt ccInt, MInts & result);
    void at(temporalalgebra::RInt rInt, MInts & result);
    
    static CcInt undefinedAttr();
    static int compare(Value value, Attr attr);

  private:
    Interval m_Interval;
    int m_Value;
  };



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

  inline IntUnit::Value IntUnit::minimum() const
  {
    return m_Value;
  }
  
  inline IntUnit::Value IntUnit::maximum() const
  {
    return m_Value;
  }
  
  inline Interval IntUnit::interval() const
  {
    return m_Interval;
  }
  
  inline void IntUnit::appendTo(
    temporalalgebra::MInt & mint)
  {
    mint.Add(temporalalgebra::UInt(m_Interval.convert(), CcInt(m_Value)));
  }
  
  inline int IntUnit::compareValue(const IntUnit & intUnit)
  {
    int iDiff;

    iDiff = m_Value - intUnit.m_Value;
    if (iDiff != 0.0)
      return iDiff < 0.0 ? -1 : 1;

    return 0;
  }

  inline temporalalgebra::IInt IntUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IInt(instant, CcInt(true, m_Value));
  }

  inline IntUnit IntUnit::restrictToInterval(Interval unitInterval)
  {
    IntUnit u;
    u.m_Interval = unitInterval;
    u.m_Value = m_Value;
    return u;
  }

  inline bool IntUnit::passes(CcInt ccInt)
  {
    return m_Value == ccInt.GetValue();
  }

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

  inline void IntUnit::at(CcInt ccInt, MInts & result)
  {
    if (passes(ccInt)) 
      result.addUnit(*this);
  }

  inline void IntUnit::at(temporalalgebra::RInt rInt, MInts & result)
  {
    if (passes(rInt))
      result.addUnit(*this);
  }

  inline CcInt IntUnit::undefinedAttr()
  {
    CcInt r(1);
    r.SetDefined(false);
    return r;
  }

  inline int IntUnit::compare(Value value, Attr attr)
  {
    int b = attr.GetValue();
    return value < b ? -1 : (value == b ? 0 : 1);
  }
}
