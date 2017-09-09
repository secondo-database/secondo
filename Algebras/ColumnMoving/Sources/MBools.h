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
#include "IBools.h"

namespace ColumnMovingAlgebra
{
  class BoolUnit;

  typedef MFsObjects<BoolUnit> MBools;

  class BoolUnit
  {
  public:
    typedef bool Value;
    typedef IBools Instants;
    typedef CcBool Attr;
    typedef temporalalgebra::RBool RAttr;
    typedef temporalalgebra::MBool MAttr;
    
    BoolUnit() = default;
    BoolUnit(temporalalgebra::MBool mint, int unit);
    BoolUnit(Interval interval, bool m_Value);

    Interval interval() const;
    Value minimum() const;
    Value maximum() const;
    void appendTo(temporalalgebra::MBool & mbool);
    int compareValue(const BoolUnit & boolUnit);
    temporalalgebra::IBool atInstant(Instant instant);
    BoolUnit restrictToInterval(Interval unitInterval);
    bool passes(CcBool ccBool);
    bool passes(temporalalgebra::RBool rBool);
    void at(CcBool ccBool, MBools & result);
    void at(temporalalgebra::RBool rBool, MBools & result);
    
    static CcBool undefinedAttr();
    static int compare(Value value, Attr attr);
    
  private:
    Interval m_Interval;
    bool m_Value;
  };



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

  inline Interval BoolUnit::interval() const
  {
    return m_Interval;
  }
  
  inline BoolUnit::Value BoolUnit::minimum() const
  {
    return m_Value;
  }
  
  inline BoolUnit::Value BoolUnit::maximum() const
  {
    return m_Value;
  }
  
  inline void BoolUnit::appendTo(
    temporalalgebra::MBool & mbool)
  {
    mbool.Add(temporalalgebra::UBool(m_Interval.convert(), 
                                     CcBool(true, m_Value)));
  }
  
  inline int BoolUnit::compareValue(const BoolUnit & boolUnit)
  {
    if (m_Value)
      return boolUnit.m_Value ? 0 : 1;
    else
      return boolUnit.m_Value ? -1 : 0;
  }

  inline temporalalgebra::IBool BoolUnit::atInstant(Instant instant)
  {
    return temporalalgebra::IBool(instant, CcBool(true, m_Value));
  }

  inline BoolUnit BoolUnit::restrictToInterval(Interval unitInterval)
  {
    BoolUnit u;
    u.m_Interval = unitInterval;
    u.m_Value = m_Value;
    return u;
  }

  inline bool BoolUnit::passes(CcBool ccBool)
  {
    return m_Value == ccBool.GetValue();
  }

  inline bool BoolUnit::passes(temporalalgebra::RBool rBool)
  {
    return rBool.Contains(CcBool(true, m_Value));
  }

  inline void BoolUnit::at(CcBool ccBool, MBools & result)
  {
    if (passes(ccBool)) 
      result.addUnit(*this);
  }

  inline void BoolUnit::at(temporalalgebra::RBool rBool, MBools & result)
  {
    if (passes(rBool))
      result.addUnit(*this);
  }

  inline CcBool BoolUnit::undefinedAttr()
  {
    CcBool r(false);
    r.SetDefined(false);
    return r;
  }
  
  inline int BoolUnit::compare(Value value, Attr attr)
  {
    bool b = attr.GetValue();
    return value < b ? -1 : (value == b ? 0 : 1);
  }
}
