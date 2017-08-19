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

#include "stdafx.h"
#include "MInts.h"

namespace ColumnMovingAlgebra
{
  CRelAlgebra::AttrArray* MInts::Filter(
    CRelAlgebra::SharedArray<const size_t> filter) const
  {
    return new MInts(*this, filter);
  }

  size_t MInts::GetCount() const
  {
    return m_Rows->size();
  }

  size_t MInts::GetSize() const
  {
    return m_DefTimes->savedSize() + 
           m_Units   ->savedSize() + 
           m_Rows    ->savedSize();
  }

  void MInts::Save(CRelAlgebra::Writer &target, bool includeHeader) const
  {
    m_DefTimes->save(target);
    m_Units   ->save(target);
    m_Rows    ->save(target, includeHeader);
  }

  void MInts::Append(const CRelAlgebra::AttrArray & array, size_t row)
  {
    MInts mints = static_cast<const MInts &>(array);
    
    addRow();

    int first = mints.firstUnitIndex(row), last = mints.lastUnitIndex(row);

    for (int i = first; i < last; i++) {
      Unit & u = (*mints.m_Units)[i];
      Interval & v = u.interval;
      addUnit(v.s, v.e, v.lc, v.rc, u.value);
    }
  }

  void MInts::Append(Attribute & value)
  {
    temporalalgebra::MInt & p = static_cast<temporalalgebra::MInt&>(value);

    addRow();

    for (int i = 0; i < p.GetNoComponents(); i++) {
      temporalalgebra::UInt uint;
      p.Get(i, uint);
      Interval interval(uint.timeInterval);
      addUnit(interval.s, interval.e, interval.lc, interval.rc,
        uint.constValue.GetIntval());
    }
  }

  void MInts::Remove()
  {
    Row & row = m_Rows->back();
    m_Units->erase(m_Units->begin() + row.firstUnitIndex, m_Units->end());
    m_Rows->pop_back();

    m_DefTimes->removeRow();
  }

  void MInts::Clear()
  {
    m_DefTimes->clear();
    m_Units->clear();
    m_Rows->clear();
  }

  bool MInts::IsDefined(size_t row) const
  {
    return unitCount(row) > 0;
  }

  int MInts::Compare(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    const MInts & mints = static_cast<const MInts&>(arrayB);

    int unitCountA = unitCount(rowA), unitCountB = mints.unitCount(rowB);
    int iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    for (int i = 0; i < unitCountA; i++) {
      int64_t tDiff;

      Unit & unitA = (*m_Units)[firstUnitIndex(rowA) + i];
      Unit & unitB = (*mints.m_Units)[mints.firstUnitIndex(rowB) + i];

      Interval & intervalA = unitA.interval;
      Interval & intervalB = unitB.interval;

      tDiff = intervalA.s - intervalB.s;
      if (tDiff != 0)
        return tDiff < 0 ? -1 : 1;

      tDiff = intervalA.e - intervalB.e;
      if (tDiff != 0)
        return tDiff < 0 ? -1 : 1;

      if (intervalA.lc != intervalB.lc)
        return intervalA.lc ? -1 : 1;

      if (intervalA.rc != intervalB.rc)
        return intervalA.rc ? -1 : 1;

      iDiff = unitA.value - unitB.value;
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  int MInts::Compare(size_t row, Attribute &value) const
  {
    temporalalgebra::MInt & b = static_cast<temporalalgebra::MInt&>(value);

    int unitCountA = unitCount(row), unitCountB = b.GetNoComponents();
    int iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    for (int i = 0; i < unitCountA; i++) {
      temporalalgebra::UInt unitB;
      int64_t tDiff;

      Unit & unitA = (*m_Units)[firstUnitIndex(row) + i];
      b.Get(i, unitB);

      Interval & intervalA = unitA.interval;
      Interval intervalB(unitB.timeInterval);

      tDiff = intervalA.s - intervalB.s;
      if (tDiff != 0)
        return tDiff < 0 ? -1 : 1;

      tDiff = intervalA.e - intervalB.e;
      if (tDiff != 0)
        return tDiff < 0 ? -1 : 1;

      if (intervalA.lc != intervalB.lc)
        return intervalA.lc ? -1 : 1;

      if (intervalA.rc != intervalB.rc)
        return intervalA.rc ? -1 : 1;

      iDiff = unitA.value - unitB.constValue.GetIntval();
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  int MInts::CompareAlmost(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB);
  }

  int MInts::CompareAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value);
  }

  bool MInts::Equals(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MInts::Equals(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  bool MInts::EqualsAlmost(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MInts::EqualsAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  size_t MInts::GetHash(size_t row) const
  {
    if (unitCount(row) == 0)
      return 0;

    int firstIndex = firstUnitIndex(row);
    Interval & i = (*m_Units)[firstIndex].interval;
    return (size_t) (i.s ^ i.e);
  }

  Attribute * MInts::GetAttribute(size_t row, bool clone) const
  {
    temporalalgebra::MInt * attribute = 
      new temporalalgebra::MInt(unitCount(row));
    attribute->StartBulkLoad();

    for (int i = firstUnitIndex(row); i <= lastUnitIndex(row); i++) {
      Unit & unit = (*m_Units)[i];
      Interval & interval = unit.interval;
      attribute->Add(temporalalgebra::UInt(
        temporalalgebra::Interval<Instant>(interval.s, interval.e,
          interval.lc, interval.rc),
        CcInt(true, unit.value)
      ));
    }

    attribute->EndBulkLoad();
    return attribute;
  }

  void MInts::present(::Instant instant, Bools & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(CcBool(true, m_DefTimes->present(i, instant)));
    }
  }

  void MInts::present(temporalalgebra::Periods periods, Bools & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(CcBool(true, m_DefTimes->present(i, periods)));
    }
  }

  void MInts::atInstant(Instant instant, IInts & result)
  {
    int64_t t = instant.millisecondsToNull();

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      bool found = false;
      int l = firstUnitIndex(i), h = lastUnitIndex(i);

      while (l <= h && !found) {
        int m = (l + h) / 2;
        Unit & unit = (*m_Units)[m];
        Interval & mi = unit.interval;

        if (mi.before(t)) {
          l = m + 1;
        } else if (mi.after(t)) {
          h = m - 1;
        } else {
          found = true;
          result.Append(temporalalgebra::IInt(t, CcInt(true, unit.value)));
        }
      }

      if (!found) 
        result.Append(temporalalgebra::IInt(0));
    }
  }

  void MInts::atPeriods(temporalalgebra::Periods periods, MInts & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.addRow();

      int iA = firstUnitIndex(i);
      int lastA = lastUnitIndex(i);
      int iB = 0;
      int lastB = periods.GetNoComponents() - 1;

      while (iA <= lastA && iB <= lastB) {
        Unit & unitA = (*m_Units)[iA];
        Interval & a = unitA.interval;
        temporalalgebra::Interval<Instant> bInterval;
        periods.Get(iB, bInterval);
        Interval b(bInterval);

        if (a.before(b)) {
          iA++;
        } else if (b.before(a)) {
          iB++;
        } else {
          Interval c = a.intersection(b);
          result.addUnit(c.s, c.e, c.lc, c.rc, unitA.value);

          if (a.endsFirstComparedTo(b))
            iA++;
          else
            iB++;
        }
      }
    }
  }

  void MInts::passes(CcInt & value, Bools & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      bool found = false;
      for (int ui = firstUnitIndex(i); ui <= lastUnitIndex(i); ui++) {
        Unit & u = (*m_Units)[ui];
        if (u.value == value.GetIntval()) {
          found = true;
          break;
        }
      }

      result.Append(CcBool(true, found));
    }
  }

  void MInts::at(CcInt & value, MInts & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.addRow();

      for (int ui = firstUnitIndex(i); ui <= lastUnitIndex(i); ui++) {
        Unit & u = (*m_Units)[ui];
        if (u.value == value.GetIntval()) {
          Interval & c = u.interval;
          result.addUnit(c.s, c.e, c.lc, c.rc, u.value);
        }
      }
    }
  }

  void MInts::at(temporalalgebra::RInt & value, MInts & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.addRow();

      for (int ui = firstUnitIndex(i); ui <= lastUnitIndex(i); ui++) {
        Unit & u = (*m_Units)[ui];
        if (value.Contains(CcInt(true, u.value))) {
          Interval & c = u.interval;
          result.addUnit(c.s, c.e, c.lc, c.rc, u.value);
        }
      }
    }
  }
}
