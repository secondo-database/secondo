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

1 DefTimes.h

*/

#pragma once

#include "Check.h"
#include "Array.h"
#include "Interval.h"

namespace ColumnMovingAlgebra
{

/*
1.1 Declaration of ~DefTimes~

This class manages the time intervals on which moving objects in an
attribut array are defined.

*/

  class DefTimes {
  public:
/*
default constructor

*/
    DefTimes() = default;
/*
~DefTimes(CRelAlgebra::Reader)~ loads from persistant storage

*/
    DefTimes(CRelAlgebra::Reader& source);

/*
saves to persistant storage

*/
    void save(CRelAlgebra::Writer &target);
/*
~savedSize~ returns the size needed for saving on persistant storage

*/
    int savedSize();

/*
~addRow~ adds a moving object for which the definition time is to manage

*/
    void addRow();
/*
~addInterval~ adds a new definition time interval for the last added row

*/
    void addInterval(Interval interval);
/*
~removeRow~ removes the last added row

*/
    void removeRow();
/*
~clear~ removes all rows

*/
    void clear();

/*
~present~ checks wether the object corresponding to ~row~ is present at 
~instant~

*/
    bool present(int row, Instant instant);
/*
~present~ checks wether the object corresponding to ~row~ is present at 
~periods~

*/
    bool present(int row, temporalalgebra::Periods periods);
/*
~getLimits~ returns the minimum and maximum definition time for all 
managed definition intervals

*/
    void getLimits(int64_t &minimum, int64_t &maximum);

/*
~rowCount~ returns the number of managed moving objects

*/
    int rowCount();
/*
~intervalFirst~ returns the index of the first interval belonging to this row

*/
    int intervalFirst(int row);
/*
~intervalAfterLast~ returns one more than the index of the last interval 
belonging to this row

*/
    int intervalAfterLast(int row);
/*
~intervalCount~ returns the number of definition intervals belonging to this
row

*/
    int intervalCount(int row);
/*
~interval~ returns the interval with the given index

*/
    Interval interval(int index);

  private:
/* 
~mRows~ saves the indices of the first element in ~mIntervals~ that belongs 
to a moving object of the attribut array which owns the instance of ~DefTimes~

*/
    Array<int> m_Rows;
/*
~mIntervals~ contains the intervals on which moving objects in an
attribut array are defined.

*/
    Array<Interval> m_Intervals;
  };


/*
1.2 Implementation of the class  ~DefTimes~

*/

  inline DefTimes::DefTimes(CRelAlgebra::Reader & source)
  {
    m_Rows.load(source);
    m_Intervals.load(source);
  }

  inline void DefTimes::save(CRelAlgebra::Writer & target)
  {
    m_Rows.save(target);
    m_Intervals.save(target);
  }

  inline int DefTimes::savedSize()
  {
    return m_Rows.savedSize() + m_Intervals.savedSize();
  }

  inline void DefTimes::addRow()
  {
    m_Rows.push_back(m_Intervals.size());
  }

  inline void DefTimes::addInterval(Interval interval)
  {
    if (m_Rows.back() < (int)m_Intervals.size() - 1) {
      Interval & last = m_Intervals.back();
      check(last.e <= interval.s, "intervals not disjoint and ordered");

      if (last.e == interval.s && (last.rc || interval.lc)) {
        check(!last.rc || !interval.lc, "intervals not disjoint");
        last.e = interval.e;
        last.rc = interval.rc;
        return;
      }
      else {
        m_Intervals.push_back(interval);
      }
    }
    else {
      m_Intervals.push_back(interval);
    }
  }

  inline void DefTimes::removeRow()
  {
    while ((int)m_Intervals.size() > m_Rows.back())
      m_Intervals.pop_back();

    m_Rows.pop_back();
  }

  inline void DefTimes::clear()
  {
    m_Rows.clear();
    m_Intervals.clear();
  }

  inline bool DefTimes::present(int row, Instant instant)
  {
    if (!instant.IsDefined())
      return false;

    int64_t t = instant.millisecondsToNull();

    int l = intervalFirst(row);
    int h = intervalAfterLast(row) - 1;

    while (l <= h) {
      int m = (l + h) / 2;
      Interval & mi = m_Intervals[m];

      if (mi.before(t))
        l = m + 1;
      else if (mi.after(t))
        h = m - 1;
      else
        return true;
    }

    return false;
  }

  inline bool DefTimes::present(int row, 
    temporalalgebra::Periods periods)
  {
    if (!periods.IsDefined())
      return false;

    int iA = intervalFirst(row);
    int lastA = intervalAfterLast(row) - 1;
    int iB = 0;
    int lastB = periods.GetNoComponents() - 1;

    while (iA <= lastA && iB <= lastB) {
      Interval & a = m_Intervals[iA];
      temporalalgebra::Interval<Instant> bInterval;
      periods.Get(iB, bInterval);
      Interval b(bInterval);

      if (a.before(b))
        iA++;
      else if (b.before(a))
        iB++;
      else
        return true;
    }

    return false;
  }
  
  inline void DefTimes::getLimits(int64_t &minimum, int64_t &maximum)
  {
    if (m_Intervals.size() == 0) {
      minimum = 0;
      maximum = 0;
      return;
    }
    
    minimum = std::numeric_limits<int64_t>::max();
    maximum = std::numeric_limits<int64_t>::min();

    for (auto & i : m_Intervals) {
      minimum = std::min(minimum, i.s);
      maximum = std::max(maximum, i.e);
    }
  }

  inline int DefTimes::rowCount()
  {
    return m_Rows.size();
  }

  inline int DefTimes::intervalFirst(int row)
  {
    return m_Rows[row];
  }

  inline int DefTimes::intervalAfterLast(int row)
  {
    if (row == rowCount() - 1)
      return m_Intervals.size();
    else
      return m_Rows[row + 1];
  }

  inline int DefTimes::intervalCount(int row)
  {
    return intervalAfterLast(row) - intervalFirst(row);
  }
  
  inline Interval DefTimes::interval(int index) {
    return m_Intervals[index];
  }
}
