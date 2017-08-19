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
#include "MPoints.h"

namespace ColumnMovingAlgebra
{
  CRelAlgebra::AttrArray* MPoints::Filter(
    CRelAlgebra::SharedArray<const size_t> filter) const
  {
    return new MPoints(*this, filter);
  }

  size_t MPoints::GetCount() const
  {
    return m_MPointsData->rowCount();
  }

  size_t MPoints::GetSize() const
  {
    return m_MPointsData->savedSize() +
           m_DefTimes->savedSize() +
           sizeof(m_Mbr) +
           sizeof(bool) +
           (m_GridIndex.get() != 0 ? m_GridIndex->savedSize() : 0);
  }

  void MPoints::Save(CRelAlgebra::Writer &target, bool includeHeader) const
  {
    m_MPointsData->save(target);
    m_DefTimes->save(target);
    
    GridIndex::Mbr m = m_Mbr;
    target.WriteOrThrow(reinterpret_cast<char*>(&m), sizeof(m));

    bool hasGrid = m_GridIndex.get() != 0;
    target.WriteOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex->save(target);
  }

  void MPoints::Append(const CRelAlgebra::AttrArray & array, size_t row)
  {
    MPoints mpoints = static_cast<const MPoints &>(array);

    addRow();

    MPointsData::UnitIterator i = mpoints.m_MPointsData->unitIterator(row);
    while (i.hasNext()) {
      MPointsData::Unit u = i.next();
      addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
    }
  }

  void MPoints::Append(Attribute & value)
  {
    temporalalgebra::MPoint & a = static_cast<temporalalgebra::MPoint&>(value);

    addRow();

    if (a.IsDefined()) {
      for (int i = 0; i < a.GetNoComponents(); i++) {
        temporalalgebra::UPoint c;
        a.Get(i, c);
        Interval interval(c.timeInterval);
        addUnit(interval, c.p0.GetX(), c.p0.GetY(), c.p1.GetX(), c.p1.GetY());
      }
    }
  }

  void MPoints::Remove()
  {
    m_MPointsData->removeRow();
    m_DefTimes->removeRow();

    if (m_GridIndex.get() != 0)
      m_GridIndex->removeRow();
  }

  void MPoints::Clear()
  {
    m_MPointsData->clear();
    m_DefTimes->clear();

    if (m_GridIndex.get() != 0)
      m_GridIndex->clear();
  }

  bool MPoints::IsDefined(size_t row) const
  {
    return m_MPointsData->unitCount(row) > 0;
  }

  int MPoints::Compare(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    const MPoints & mpointsB = static_cast<const MPoints&>(arrayB);
    int iDiff;
    double dDiff;

    int unitCountA = m_MPointsData->unitCount(rowA);
    int unitCountB = mpointsB.m_MPointsData->unitCount(rowB);

    iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    MPointsData::UnitIterator iA = m_MPointsData->unitIterator(rowA);
    MPointsData::UnitIterator iB = mpointsB.m_MPointsData->unitIterator(rowB);

    while (iA.hasNext()) {
      assert(iB.hasNext());

      MPointsData::Unit a = iA.next();
      MPointsData::Unit b = iB.next();

      Interval & intervalA = a.interval;
      Interval & intervalB = b.interval;
      
      iDiff = intervalA.compare(intervalB);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      dDiff = a.x0 - b.x0;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.x1 - b.x1;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y0 - b.y0;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y1 - b.y1;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  int MPoints::Compare(size_t row, Attribute &value) const
  {
    auto mpointB = static_cast<temporalalgebra::MPoint&>(value);

    if (!mpointB.IsDefined())
      return IsDefined(row) ? 1 : 0;

    int iDiff;
    double dDiff;

    int unitCountA = m_MPointsData->unitCount(row);
    int unitCountB = mpointB.GetNoComponents();

    iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    MPointsData::UnitIterator iA = m_MPointsData->unitIterator(row);
    int iB = 0;

    while (iA.hasNext()) {
      assert(iB < unitCountB);
      temporalalgebra::UPoint b;

      MPointsData::Unit a = iA.next();
      mpointB.Get(iB, b);

      Interval & intervalA = a.interval;
      Interval intervalB(b.timeInterval);

      iDiff = intervalA.compare(intervalB);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      dDiff = a.x0 - b.p0.GetX();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.x1 - b.p1.GetX();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y0 - b.p0.GetY();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y1 - b.p1.GetY();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  int MPoints::CompareAlmost(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB);
  }

  int MPoints::CompareAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value);
  }

  bool MPoints::Equals(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MPoints::Equals(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  bool MPoints::EqualsAlmost(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MPoints::EqualsAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  size_t MPoints::GetHash(size_t row) const
  {
    if (m_MPointsData->unitCount(row) == 0)
      return 0;

    MPointsData::UnitIterator i = m_MPointsData->unitIterator(row);
    assert(i.hasNext());
    MPointsData::Unit u = i.next();
    return (size_t)(u.interval.s ^ u.interval.e);
  }

  Attribute * MPoints::GetAttribute(size_t row, bool clone) const
  {
    temporalalgebra::MPoint * attribute = 
      new temporalalgebra::MPoint(m_MPointsData->unitCount(row));
      
    attribute->StartBulkLoad();

    MPointsData::UnitIterator i = m_MPointsData->unitIterator(row);
    while (i.hasNext()) {
      MPointsData::Unit u = i.next();
      Interval & interval = u.interval;
      attribute->Add(temporalalgebra::UPoint(
        temporalalgebra::Interval<Instant>(interval.s, interval.e, 
                                           interval.lc, interval.rc),
        u.x0, u.y0, u.x1, u.y1));
    }

    attribute->EndBulkLoad();
    return attribute;
  }

  void MPoints::present(Instant instant, CRelAlgebra::LongInts & result)
  {
    result.Clear();
    
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, instant))
        result.Append(i);
    }
  }

  void MPoints::present(temporalalgebra::Periods periods, 
    CRelAlgebra::LongInts & result)
  {
    result.Clear();

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, periods))
        result.Append(i);
    }
  }

  void MPoints::atInstant(Instant instant, IPoints & result)
  {
    if (!instant.IsDefined()) {

      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.Append(temporalalgebra::IPoint(0));
      
    } else if (m_GridIndex.get() != 0) {

      typedef map<int, MPointsData::Position> RowMap;
      typedef pair<int, MPointsData::Position> RowMapPair;

      RowMap rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();
      mbr.l[2] = mbr.h[2] = static_cast<double>(instant.millisecondsToNull());

      for (auto & e : m_GridIndex->selection(mbr)) {
        MPointsData::Unit u = m_MPointsData->unit(e.unitId);
        if (u.interval.contains(instant.millisecondsToNull()))
          rows.insert(RowMapPair(e.row, u.at(instant.millisecondsToNull())));
      }

      map<int, MPointsData::Position>::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && si->first < i)
          si++;

        if (si != rows.end() && si->first == i)
          result.Append(temporalalgebra::IPoint(instant, 
            Point(true, si->second.x, si->second.y)));
        else
          result.Append(temporalalgebra::IPoint(0));
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        MPointsData::Position p;

        if (m_MPointsData->position(i, instant.millisecondsToNull(), p))
          result.Append(temporalalgebra::IPoint(instant, 
            Point(true, p.x, p.y)));
        else
          result.Append(temporalalgebra::IPoint(0));
      }

    }
  }

  void MPoints::atPeriods(temporalalgebra::Periods periods, MPoints & result)
  {
    if (!periods.IsDefined() || periods.GetNoComponents() == 0) {

      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.addRow();

    } else if (m_GridIndex.get() != 0) {

      typedef map<int64_t, MPointsData::Unit> UnitMap;
      typedef pair<int64_t, MPointsData::Unit> UnitMapPair;
      typedef map<int, UnitMap> RowMap;
      typedef pair<int, UnitMap> RowMapPair;

      RowMap rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();

      Instant instant;
      periods.Minimum(instant);
      mbr.l[2] = static_cast<double>(instant.millisecondsToNull());
      periods.Maximum(instant);
      mbr.h[2] = static_cast<double>(instant.millisecondsToNull());

      for (auto & e : m_GridIndex->selection(mbr)) {
        RowMap::iterator r = rows.find(e.row);
        if (r == rows.end())
          r = rows.insert(RowMapPair(e.row, UnitMap())).first;

        MPointsData::Unit u = m_MPointsData->unit(e.unitId);
        temporalalgebra::UPoint upoint(u.interval.convert(), 
                                       u.x0, u.y0, u.x1, u.y1);

        for (int i = 0; i < periods.GetNoComponents(); i++) {
          temporalalgebra::Interval<Instant> interval;
          periods.Get(i, interval);
          if (upoint.getTimeInterval().Intersects(interval)) {
            temporalalgebra::UPoint result;
            upoint.AtInterval(interval, result);
            MPointsData::Unit unit(result.timeInterval, 
              result.p0.GetX(), result.p0.GetY(), 
              result.p1.GetX(), result.p1.GetY());
              
            r->second.insert(UnitMapPair(
              result.timeInterval.start.millisecondsToNull(), unit));
          }
        }
      }

      RowMap::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && si->first < i)
          si++;

        result.addRow();

        if (si != rows.end() && si->first == i)
          for (auto & ui : si->second) {
            MPointsData::Unit u = ui.second;
            result.addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
          }
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();

        result.addRow();

        temporalalgebra::Interval<Instant> period;
        int iPeriod = 0; 
        bool periodValid = iPeriod < periods.GetNoComponents();
        if (periodValid)
          periods.Get(iPeriod, period);

        temporalalgebra::UPoint upoint;
        MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
        bool upointValid = iUnit.hasNext();
        if (upointValid) {
          MPointsData::Unit u = iUnit.next();
          upoint = temporalalgebra::UPoint(u.interval.convert(), 
                                           u.x0, u.y0, u.x1, u.y1);
        }

        while (periodValid && upointValid) {
          if (upoint.getTimeInterval().Intersects(period)) {
            temporalalgebra::UPoint r;
            upoint.AtInterval(period, r);
            result.addUnit(r.timeInterval, r.p0.GetX(), r.p0.GetY(), 
                                           r.p1.GetX(), r.p1.GetY());
          }

          int64_t upointEnd = upoint.getTimeInterval().end.millisecondsToNull();
          int64_t periodEnd = period.end.millisecondsToNull();

          if ( periodEnd < upointEnd || 
              (periodEnd == upointEnd && upoint.getTimeInterval().rc)) 
          {
            iPeriod++;
            periodValid = iPeriod < periods.GetNoComponents();
            if (periodValid)
              periods.Get(iPeriod, period);
          } else {
            upointValid = iUnit.hasNext();
            if (upointValid) {
              MPointsData::Unit u = iUnit.next();
              upoint = temporalalgebra::UPoint(u.interval.convert(), 
                                               u.x0, u.y0, u.x1, u.y1);
            }
          }
        }
      }

    }
  }

  void MPoints::passes(Point & value, CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined()) {
      return;

    } else if (m_GridIndex.get() != 0) {

      typedef set<int> Rows;
      Rows rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();
      mbr.l[0] = mbr.h[0] = value.GetX();
      mbr.l[1] = mbr.h[1] = value.GetY();

      for (auto & e : m_GridIndex->selection(mbr)) {
        MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
        temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                       unit.x0, unit.y0, unit.x1, unit.y1);

        if (upoint.Passes(value))
          rows.insert(e.row);
      }

      Rows::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && *si < i)
          si++;

        if (si != rows.end() && *si == i)
          result.Append(i);
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();

        bool found = false;

        MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
        while (iUnit.hasNext()) {
          MPointsData::Unit unit = iUnit.next();

          temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                         unit.x0, unit.y0, unit.x1, unit.y1);

          if (upoint.Passes(value)) {
            found = true;
            break;
          }
        }

        if (found)
          result.Append(i);
      }

    }
  }

  void MPoints::passes(Region & value, CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined()) {

      return;

    } else if (m_GridIndex.get() != 0) {

      typedef set<int> Rows;
      Rows rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();
      Rectangle<2> rect = value.BoundingBox();
      for (int i = 0; i < 2; i++) {
        mbr.l[i] = rect.MinD(i);
        mbr.h[i] = rect.MaxD(i);
      }

      for (auto & e : m_GridIndex->selection(mbr)) {
        MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
        temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                       unit.x0, unit.y0, unit.x1, unit.y1);

        if (upoint.Passes(value))
          rows.insert(e.row);
      }

      Rows::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && *si < i)
          si++;

        if (si != rows.end() && *si == i)
          result.Append(i);
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();

        bool found = false;

        MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
        while (iUnit.hasNext()) {
          MPointsData::Unit unit = iUnit.next();

          temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                         unit.x0, unit.y0, unit.x1, unit.y1);

          if (upoint.Passes(value)) {
            found = true;
            break;
          }
        }

        if (found)
          result.Append(i);
      }

    }
  }

  void MPoints::at(Point & value, MPoints & result)
  {
    if (!value.IsDefined()) {

      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.addRow();

    } else if (m_GridIndex.get() != 0) {

      typedef map<int64_t, MPointsData::Unit> UnitMap;
      typedef pair<int64_t, MPointsData::Unit> UnitMapPair;
      typedef map<int, UnitMap> RowMap;
      typedef pair<int, UnitMap> RowMapPair;

      RowMap rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();
      mbr.l[0] = mbr.h[0] = value.GetX();
      mbr.l[1] = mbr.h[1] = value.GetY();

      for (auto & e : m_GridIndex->selection(mbr)) {
        MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
        temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                       unit.x0, unit.y0, unit.x1, unit.y1);
        temporalalgebra::UPoint result;

        if (upoint.At(value, result)) {
          RowMap::iterator r = rows.find(e.row);
          if (r == rows.end())
            r = rows.insert(RowMapPair(e.row, UnitMap())).first;

          MPointsData::Unit unit(result.timeInterval, 
                                 result.p0.GetX(), result.p0.GetY(), 
                                 result.p1.GetX(), result.p1.GetY());
                                 
          r->second.insert(UnitMapPair(unit.interval.s, unit));
        }
      }

      RowMap::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && si->first < i)
          si++;

        result.addRow();

        if (si != rows.end() && si->first == i) {
          for (auto & ui : si->second) {
            MPointsData::Unit u = ui.second;
            result.addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
          }
        }
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();

        result.addRow();

        MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
        while (iUnit.hasNext()) {
          MPointsData::Unit unit = iUnit.next();

          temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                         unit.x0, unit.y0, unit.x1, unit.y1);
          temporalalgebra::UPoint r;

          if (upoint.At(value, r))
            result.addUnit(r.timeInterval, r.p0.GetX(), r.p0.GetY(), 
                                           r.p1.GetX(), r.p1.GetY());
        }
      }

    }
  }

  void MPoints::at(Region & value, MPoints & result)
  {
    if (!value.IsDefined()) {

      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addRow();

    } else if (m_GridIndex.get() != 0) {

      typedef map<int64_t, MPointsData::Unit> UnitMap;
      typedef pair<int64_t, MPointsData::Unit> UnitMapPair;
      typedef map<int, UnitMap> RowMap;
      typedef pair<int, UnitMap> RowMapPair;

      RowMap rows;

      GridIndex::Mbr mbr = m_GridIndex->mbr();
      Rectangle<2> rect = value.BoundingBox();
      for (int i = 0; i < 2; i++) {
        mbr.l[i] = rect.MinD(i);
        mbr.h[i] = rect.MaxD(i);
      }

      for (auto & e : m_GridIndex->selection(mbr)) {
        MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
        temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                       unit.x0, unit.y0, unit.x1, unit.y1);

        vector<temporalalgebra::UPoint> intersection;
        upoint.AtRegion(&value, intersection);

        if (intersection.size() > 0) {
          RowMap::iterator r = rows.find(e.row);
          if (r == rows.end())
            r = rows.insert(RowMapPair(e.row, UnitMap())).first;

          for (auto & upoint : intersection) {
            MPointsData::Unit unit(upoint.timeInterval, 
                                   upoint.p0.GetX(), upoint.p0.GetY(), 
                                   upoint.p1.GetX(), upoint.p1.GetY());
                                   
            r->second.insert(UnitMapPair(unit.interval.s, unit));
          }
        }
      }

      RowMap::iterator si = rows.begin();
      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();
        while (si != rows.end() && si->first < i)
          si++;

        result.addRow();

        if (si != rows.end() && si->first == i) {
          for (auto & ui : si->second) {
            MPointsData::Unit u = ui.second;
            result.addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
          }
        }
      }

    } else {

      for (auto & iterator : GetFilter()) {
        int i = iterator.GetRow();

        result.addRow();

        MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
        while (iUnit.hasNext()) {
          MPointsData::Unit unit = iUnit.next();

          temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                         unit.x0, unit.y0, unit.x1, unit.y1);
          vector<temporalalgebra::UPoint> r;
          upoint.AtRegion(&value, r);

          for (auto & ir : r) 
            result.addUnit(ir.timeInterval, 
                           ir.p0.GetX(), ir.p0.GetY(), 
                           ir.p1.GetX(), ir.p1.GetY());
        }
      }

    }
  }

  void MPoints::index(CcInt & temporalSplits, CcInt & spatialSplits, 
    MPoints & result)
  {
    GridIndex::IVector splits;
    splits.p[0] = splits.p[1] = spatialSplits.GetIntval();
    splits.p[2] = temporalSplits.GetIntval();

    GridIndex::DVector cellSize;
    for (int i = 0; i < 3; i++)
      cellSize.s[i] = (m_Mbr.h[i] - m_Mbr.l[i]) / splits.p[i];

    result.m_GridIndex = make_shared<GridIndex>(splits, cellSize);

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(*GetAttribute(i));
    }
  }

  void MPoints::addRandomRows(CcInt& size, MPoints & result)
  {
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(*GetAttribute(i));
    }
 
    if (!size.IsDefined())
      return;
    
    int rows = size.GetIntval(), units = size.GetIntval();
      
    srand(1);

    for (int i = 0; i < rows; i++) {
      result.addRow();

      double p[2], v[2];
      int64_t t = 1;

      for (int d = 0; d < 2; d++) {
        p[d] = static_cast<double>(rand()) / RAND_MAX * 100.0;
        v[d] = static_cast<double>(rand()) / RAND_MAX * 5.0;
      }

      for (int u = 0; u < units; u++) {
        int64_t dt = 5 + (rand() % 5);
        result.addUnit(Interval(t, t + dt, true, false), 
                       p[0], p[1], p[0] + v[0], p[1] + v[1]);

        t += dt;

        for (int d = 0; d < 2; d++) {
          p[d] += v[d];

          if (p[d] > 100.0)
            v[d] = -abs(v[d]);
          if (p[d] < 0.0)
            v[d] = abs(v[d]);
        }
      }
    }
  }
}
