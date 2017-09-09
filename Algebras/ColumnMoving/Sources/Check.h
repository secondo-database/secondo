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

#include <cassert>
#include "Array.h"
#include "Interval.h"

namespace ColumnMovingAlgebra
{
  class DefTimes {
  public:
    DefTimes() = default;
    DefTimes(CRelAlgebra::Reader& source);

    void save(CRelAlgebra::Writer &target);
    int savedSize();

    void addRow();
    void removeRow();
    void clear();

    void addInterval(Interval interval);

    bool present(int collection, Instant instant);
    bool present(int collection, temporalalgebra::Periods periods);

  private:
    Array<int> m_CollectionsFirstIndizes;
    Array<Interval> m_Intervals;

    int collectionsLastIndizes(int collection);
  };




  inline DefTimes::DefTimes(CRelAlgebra::Reader & source)
  {
    m_CollectionsFirstIndizes.load(source);
    m_Intervals.load(source);
  }

  inline void DefTimes::save(CRelAlgebra::Writer & target)
  {
    m_CollectionsFirstIndizes.save(target);
    m_Intervals.save(target);
  }

  inline int DefTimes::savedSize()
  {
    return m_CollectionsFirstIndizes.savedSize() + m_Intervals.savedSize();
  }

  inline void DefTimes::addRow()
  {
    m_CollectionsFirstIndizes.push_back(m_Intervals.size());
  }

  inline void DefTimes::removeRow()
  {
    while ((int)m_Intervals.size() > m_CollectionsFirstIndizes.back())
      m_Intervals.pop_back();

    m_CollectionsFirstIndizes.pop_back();
  }

  inline void DefTimes::clear()
  {
    m_CollectionsFirstIndizes.clear();
    m_Intervals.clear();
  }

  inline void DefTimes::addInterval(Interval interval)
  {
    if (m_CollectionsFirstIndizes.back() < (int)m_Intervals.size() - 1) {
      Interval & last = m_Intervals.back();
      assert(last.e <= interval.s);

      if (last.e == interval.s && (last.rc || interval.lc)) {
        assert(!last.rc || !interval.lc);
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

  inline bool DefTimes::present(int collection, Instant instant)
  {
    if (!instant.IsDefined())
      return false;

    int64_t t = instant.millisecondsToNull();

    int l = m_CollectionsFirstIndizes[collection];
    int h = collectionsLastIndizes(collection);

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

  inline bool DefTimes::present(int collection, 
    temporalalgebra::Periods periods)
  {
    if (!periods.IsDefined())
      return false;

    int iA = m_CollectionsFirstIndizes[collection];
    int lastA = collectionsLastIndizes(collection);
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

  inline int DefTimes::collectionsLastIndizes(int collection)
  {
    if (collection == static_cast<int>(m_CollectionsFirstIndizes.size()) - 1)
      return m_Intervals.size() - 1;
    else
      return m_CollectionsFirstIndizes[collection + 1] - 1;
  }
}
