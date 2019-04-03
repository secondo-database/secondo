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

1 MReals.h

*/

#pragma once

#include "AlmostEqual.h"
#include "MFsObjects.h"
#include "IReals.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Forward Declaration of ~RealUnit~

~RealUnit~ represents a moving real unit

*/
  class RealUnit;

/*
1.1 Declaration of ~MReals~

~MReals~ represents a moving real

*/
  typedef MFsObjects<RealUnit> MReals;

/*
1.2 Declaration of class ~RealUnit~

*/
  class RealUnit
  {
  public:
    typedef double Value;
    typedef IReals Instants;
    typedef CcReal Attr;
    typedef temporalalgebra::RReal RAttr;
    typedef temporalalgebra::MReal MAttr;

/*
constructors

*/
    RealUnit() = default;
    RealUnit(temporalalgebra::MReal mreal, int unit);
    RealUnit(Interval interval, double a, double b, double c, bool r);

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
    void appendTo(temporalalgebra::MReal & mreal);
/*
~compareValue~ compares this unit

*/
    int compareValue(const RealUnit & realUnit);
/*
~atInstant~ returns an intime for ~instant~

*/
    temporalalgebra::IReal atInstant(Instant instant);
/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
    RealUnit restrictToInterval(Interval interval);
/*
~passes~ returns true, iff this this unit has the same value 

*/
    bool passes(CcReal ccReal);
/*
or iff the value is in the specified range

*/
    bool passes(temporalalgebra::RReal rReal);
/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
    void at(CcReal ccReal, MReals & result);
/*
of iff its value is in the specified range

*/
    void at(temporalalgebra::RReal rReal, MReals & result);
    
/*
~undefinedAttr~ returns an undefined attribute

*/
    static CcReal undefinedAttr();
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
    static RealUnit random(Interval interval);
    
  private:
/*
~mInterval~ represents the definition interval of this unit

*/
    Interval m_Interval;
/*
iff ~mR~ is true, the mapping function f(t) of this unit is
~mA~ t t + ~mB~ t + ~mC~.
Otherwise the mapping function of this unit is
sqrt(~mA~ t t + ~mB~ t + ~mC~).
~mMinimum~ and ~mMaximum~ represent the minimum and maximum
of the mapping function on the definition interval of this unit.
as minimum and maximum value might be on the boundaries of the 
definition interval and the definition interval is not necessarily left and 
right closed, we also need ~mMinIncluded~ and ~mMaxIncluded~ to determine,
whether the mapping function ever has the value ~mMinimum~ and ~mMaximum~
on the definition interval or only converges

*/
    double m_A, m_B, m_C, m_Minimum, m_Maximum;
    bool m_R, m_MinIncluded, m_MaxIncluded;

/*
~calculateExtrema~ 
sets the values ~mMinimum~, ~mMaximum~, ~mMinIncluded~ and ~mMaxIncluded~

*/
    void calculateExtrema();
/* 
~timeToDouble~ converts ~t~ to double

*/
    double timeToDouble(int64_t t);
/*
~doubleToTime~ converts ~x~ to time value

*/
    int64_t doubleToTime(double x);
 /*
 ~solveLinear~ solves a linear mapping function
 
 */
    void solveLinear(double y, int64_t & t);
 /*
 ~solveQuadratic~ solves a quadratic mapping function. returns the
 number of found solutions.
 
 */
    int solveQuadratic(double y, int64_t & t0, int64_t & t1);
  };



/*
1.2 Implementation of class ~RealUnit~

constructors

*/
  inline RealUnit::RealUnit(temporalalgebra::MReal mreal, int unit)
  {
    temporalalgebra::UReal u;
    mreal.Get(unit, u);
    m_Interval = u.timeInterval;
    m_A = u.a;
    m_B = u.b;
    m_C = u.c;
    m_R = u.r;

    calculateExtrema();
  }

  inline RealUnit::RealUnit(Interval interval, double a, 
    double b, double c, bool r) :
    m_Interval(interval),
    m_A(a),
    m_B(b),
    m_C(c),
    m_R(r)
  {
    calculateExtrema();
  }

/*
~minimum~ returns the minimum value of the mapping function during the
definition interval

*/
  inline RealUnit::Value RealUnit::minimum() const
  {
    return m_Minimum;
  }
  
/*
~maximum~ returns the maximum value of the mapping function during the
definition interval

*/
  inline RealUnit::Value RealUnit::maximum() const
  {
    return m_Maximum;
  }
  
/*
~interval~ returns the definition interval of this unit

*/
  inline Interval RealUnit::interval() const
  {
    return m_Interval;
  }
  
/*
~appendTo~ adds this unit to a moving object in the temporal algebra

*/
  inline void RealUnit::appendTo(
    temporalalgebra::MReal & mreal)
  {
    mreal.Add(temporalalgebra::UReal(m_Interval.convert(), m_A, m_B, m_C, m_R));
  }
  
/*
~compareValue~ compares this unit

*/
  inline int RealUnit::compareValue(const RealUnit & realUnit)
  {
    double dDiff;

    dDiff = m_A - realUnit.m_A;
    if (dDiff != 0.0)
      return dDiff < 0.0 ? -1 : 1;

    dDiff = m_B - realUnit.m_B;
    if (dDiff != 0.0)
      return dDiff < 0.0 ? -1 : 1;

    dDiff = m_C - realUnit.m_C;
    if (dDiff != 0.0)
      return dDiff < 0.0 ? -1 : 1;

    if (m_R != realUnit.m_R)
      return m_R ? -1 : 1;

    return 0;
  }

/*
~atInstant~ returns an intime for ~instant~

*/
  inline temporalalgebra::IReal RealUnit::atInstant(Instant instant)
  {
    double x = timeToDouble(instant.millisecondsToNull());
    double result = m_A * x * x + m_B * x + m_C;

    if (m_R) {
      if (result >= 0.0) {
        result = sqrt(result);
      } else {
        check(AlmostEqual(result, 0.0), "result is imaginary number");
        result = 0.0;
      }
    }

    return temporalalgebra::IReal(instant, CcReal(true, result));
  }

/*
~restrictToInterval~ restricts the unit to ~unitInterval~

*/
  inline RealUnit RealUnit::restrictToInterval(Interval newInterval)
  {
    double x = timeToDouble(newInterval.s);
    double uc = m_A * x * x + m_B * x + m_C;
  
    if (newInterval.s == newInterval.e) {
      return RealUnit(newInterval, 0.0, 0.0, uc, m_R);
    } else {
      double ub = 2 * m_A * x + m_B;
      return RealUnit(newInterval, m_A, ub, uc, m_R);
    }
  }

/*
~passes~ returns true, iff this this unit has the same value 

*/
  inline bool RealUnit::passes(CcReal ccReal)
  {
    double y = ccReal.GetValue();

    if (y < m_Minimum || (y == m_Minimum && !m_MinIncluded))
      return false;

    if (y > m_Maximum || (y == m_Maximum && !m_MaxIncluded))
      return false;

    return true;
  }

/*
or iff the value is in the specified range

*/
  inline bool RealUnit::passes(temporalalgebra::RReal rReal)
  {
    for (int index = 0; index < rReal.GetNoComponents(); index++) {
      temporalalgebra::Interval<CcReal> ri;
      rReal.Get(index, ri);

      double ys = ri.end.GetValue(), ye = ri.start.GetValue();
      bool fis = ri.rc, fie = ri.lc;
     
      if (ys < m_Minimum || (ys == m_Minimum && (!m_MinIncluded || !fis)))
        continue;

      if (ye > m_Maximum || (ye == m_Maximum && (!m_MaxIncluded || !fie)))
        continue;

      return true;
    }
    
    return false;
  }

/*
~at~ adds this unit to ~result~ iff it has the same value as specified

*/
  inline void RealUnit::at(CcReal ccReal, MReals & result)
  {
    if (passes(ccReal)) {
      std::list<Interval> intersections;
      double y = ccReal.GetValue();
      
      if (AlmostEqual(m_A, 0.0)) {
        if (AlmostEqual(m_B, 0.0)) {
          result.addUnit(*this);
          return;
        }
        
        int64_t t;
        solveLinear(y, t);
        intersections.push_back(Interval(t, t, true, true));
      } else {
        int64_t t0, t1;
        int count = solveQuadratic(y, t0, t1);
        
        if (count >= 1)
          intersections.push_back(Interval(t0, t0, true, true));
        
        if (count >= 2)
          intersections.push_back(Interval(t1, t1, true, true));
      }
      
      for (auto & i : intersections)
        if (i.intersects(m_Interval)) {
          RealUnit u(i, 0.0, 0.0, y, false);
          result.addUnit(u);
        }
    }
  }

/*
of iff its value is in the specified range

*/
  inline void RealUnit::at(temporalalgebra::RReal rReal, MReals & result)
  {
    temporalalgebra::Interval<CcReal> ri;
    
    struct IntervalCompare {
      bool operator() (const Interval &a, const Interval &b) const {
          return a.s < b.s; 
      }
    };
    std::set<Interval, IntervalCompare> intersections;

    if (AlmostEqual(m_A, 0.0)) {
      if (AlmostEqual(m_B, 0.0)) {
        for (int index = 0; index < rReal.GetNoComponents(); index++) {
          rReal.Get(index, ri);
          double ys = ri.start.GetValue(), ye = ri.end.GetValue();
          
          if ( ye >= m_Minimum && 
              (ye != m_Minimum || (m_MinIncluded && ri.rc)) &&
               ys <= m_Maximum && 
              (ys != m_Maximum || (m_MaxIncluded && ri.lc))) 
          {
            result.addUnit(*this);
            return;
          }
        }
        
        return;
      }

      for (int index = 0; index < rReal.GetNoComponents(); index++) {
        rReal.Get(index, ri);
        double ys = ri.start.GetValue(), ye = ri.end.GetValue();

        int64_t t0, t1;
        solveLinear(ys, t0);
        solveLinear(ye, t1);
        
        if (t0 < t1)
          intersections.insert(Interval(t0, t1, ri.lc, ri.rc));
        else
          intersections.insert(Interval(t1, t0, ri.rc, ri.lc));
      }
    } else {
      for (int index = 0; index < rReal.GetNoComponents(); index++) {
        rReal.Get(index, ri);
        double ys = ri.start.GetValue(), ye = ri.end.GetValue();

        int64_t t00=0, t01=0, t10=0, t11=0;
        int count0 = solveQuadratic(ys, t00, t01);
        int count1 = solveQuadratic(ye, t10, t11);
        bool i0 = ri.lc, i1 = ri.rc;
        
        if (count1 > count0) {
          std::swap(count0, count1);
          std::swap(t00, t10);
          std::swap(t01, t11);
          std::swap(i0, i1);
        }
        
        if (count0 == 2) {
          if (count1 == 2) {
            if (t00 > t10) {
              std::swap(t00, t10);
              std::swap(t01, t11);
              std::swap(i0, i1);
            }
            
            intersections.insert(Interval(t00, t10, i0, i1));
            intersections.insert(Interval(t11, t01, i1, i0));
          } else if (count1 == 1) {
            if (i1) {
              intersections.insert(Interval(t00, t01, i0, i0));
            } else {
              intersections.insert(Interval(t00, t10, i0, false));
              intersections.insert(Interval(t10, t01, false, i0));
            }
          } else {
            intersections.insert(Interval(t00, t01, i0 || i1, i0 || i1));
          }
        } else if (count0 == 1) {
          intersections.insert(Interval(t00, t00, i0 || i1, i0 || i1));
        }
      }
    }

    for (auto i : intersections)
      if (i.intersects(m_Interval)) {
        RealUnit u = restrictToInterval(i.intersection(m_Interval));
        result.addUnit(u);
      }
  }

/*
~calculateExtrema~ 
sets the values ~mMinimum~, ~mMaximum~, ~mMinIncluded~ and ~mMaxIncluded~

*/
  inline void RealUnit::calculateExtrema()
  {
    m_Minimum = m_Maximum = m_C;
    m_MinIncluded = m_MaxIncluded = m_Interval.lc;

    double xEnd = timeToDouble(m_Interval.e);
    double y = m_A * xEnd * xEnd + m_B * xEnd + m_C;

    if (y < m_Minimum || (y == m_Minimum && !m_MinIncluded)) {
      m_Minimum = y;
      m_MinIncluded = m_Interval.rc;
    }
    if (y > m_Maximum || (y == m_Maximum && !m_MaxIncluded)) {
      m_Maximum = y;
      m_MaxIncluded = m_Interval.rc;
    }

    if (!AlmostEqual(m_A, 0.0)) {
      double x = -m_B / (2.0 * m_A);

      if (0.0 < x && x < xEnd) {
        double y = m_A * x * x + m_B * x + m_C;

        if (y < m_Minimum || (y == m_Minimum && !m_MinIncluded)) {
          m_Minimum = y;
          m_MinIncluded = true;
        } 
        if (y > m_Maximum || (y == m_Maximum && !m_MaxIncluded)) {
          m_Maximum = y;
          m_MaxIncluded = true;
        }
      }
    }

    if (m_R) {
      if (m_Minimum >= 0.0) {
        m_Minimum = sqrt(m_Minimum);
      } else {
        check(AlmostEqual(m_Minimum, 0.0), "minimum is imaginary number");
        m_Minimum = 0.0;
      }
      if (m_Maximum >= 0.0) {
        m_Maximum = sqrt(m_Maximum);
      } else {
        check(AlmostEqual(m_Maximum, 0.0), "maximum is imaginary number");
        m_Maximum = 0.0;
      }
    }
  }

/* 
~timeToDouble~ converts ~t~ to double

*/
  inline double RealUnit::timeToDouble(int64_t t)
  {
    return static_cast<double>(t - m_Interval.s) / 
           static_cast<double>(MILLISECONDS);
  }

/*
~doubleToTime~ converts ~x~ to time value

*/
  inline int64_t RealUnit::doubleToTime(double x)
  {
    return static_cast<int64_t>(x * static_cast<double>(MILLISECONDS)) +
           m_Interval.s;
  }

/*
~solveLinear~ solves a linear mapping function

*/
  inline void RealUnit::solveLinear(double y, int64_t & t)
  {
    if (m_R)
      y *= y;

    t = doubleToTime((y - m_C) / m_B);
  }

/*
~solveQuadratic~ solves a quadratic mapping function. returns the
number of found solutions.

*/
  inline int RealUnit::solveQuadratic(double y, int64_t & t0, int64_t & t1)
  {
    if (m_R)
      y *= y;

    double root = m_B * m_B - 4 * m_A * (m_C - y);
    
    if (root < 0.0)
      return 0;

    root = sqrt(root);
    t0 = doubleToTime((-m_B - root) / (2 * m_A));

    if (root == 0.0)
      return 1;

    t1 = doubleToTime((-m_B + root) / (2 * m_A));
    return t0 != t1 ? 2 : 1;
  }
  
/*
~undefinedAttr~ returns an undefined attribute

*/
  inline CcReal RealUnit::undefinedAttr()
  {
    CcReal r(1.0);
    r.SetDefined(false);
    return r;
  }
  
/*
~compare~ 
returns -1 if ~value~ smaller ~attr~,
returns  0 if ~value~ equal ~attr~, 
returns  1 if ~value~ greater ~attr~ 

*/
  inline int RealUnit::compare(Value value, Attr attr)
  {
    double b = attr.GetValue();
    return value < b ? -1 : (value == b ? 0 : 1);
  }
  
/*
~random~ returns a unit with the specified interval and a random value

*/
 inline RealUnit RealUnit::random(Interval interval)
  {
    return RealUnit(interval, static_cast<double>(rand()) / RAND_MAX,
                              static_cast<double>(rand()) / RAND_MAX,
                              static_cast<double>(rand()) / RAND_MAX,
                              (rand() % 2) == 1);
  }
}
