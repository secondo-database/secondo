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

#include <memory>
#include "MObjects.h"
#include "Ints.h"
#include "DefTimes.h"
#include "IPoints.h"
#include "MPointsData.h"
#include "Grid.h"
#include "MBools.h"

namespace ColumnMovingAlgebra
{
  class MPoints : public MObjects
  {
  public:
    MPoints();
    MPoints(CRelAlgebra::Reader& source);
    MPoints(CRelAlgebra::Reader& source, size_t rowsCount);
    MPoints(const MPoints &array, 
            const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MPoints() { }

    virtual AttrArray* Filter(CRelAlgebra::SharedArray<const size_t> 
                              filter) const;

    virtual size_t GetCount() const;
    virtual size_t GetSize() const;
    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

    virtual void Save(CRelAlgebra::Writer &target, 
                      bool includeHeader = true) const;

    virtual void Append(const CRelAlgebra::AttrArray & array, size_t row);
    virtual void Append(Attribute & value);
    virtual void Remove();
    virtual void Clear();

    virtual bool IsDefined(size_t row) const;

    virtual int Compare(size_t rowA, const AttrArray& arrayB, 
      size_t rowB) const;
    virtual int Compare(size_t row, Attribute &value) const;

    virtual size_t GetHash(size_t row) const;

    void atInstant(Instant instant, IPoints & result);
    void atInstantIndexed(Instant instant, IPoints & result);
    
    void atPeriods(temporalalgebra::Periods periods, MPoints & result);
    void atPeriodsIndexed(temporalalgebra::Periods periods, MPoints & result);
    
    void passes(Point & value, CRelAlgebra::LongInts & result);
    void passesIndexed(Point & value, CRelAlgebra::LongInts & result);
    void passes(Region & value, CRelAlgebra::LongInts & result);
    void passesIndexed(Region & value, CRelAlgebra::LongInts & result);
    
    void at(Point & value, MPoints & result);
    void atIndexed(Point & value, MPoints & result);
    void at(Region & value, MPoints & result);
    void atIndexed(Region & value, MPoints & result);
    
    void addRandomRows(CcInt& size, MPoints & result);
    
    void index(CcInt& temporalSplits, CcInt& spatialSplits, MPoints & result);
    
    void addRow();
    void addUnit(Interval interval, double x0, double y0, 
                                    double x1, double y1);

    MPointsData::UnitIterator unitIterator(int row);
    
    void getDefTimeLimits(int64_t &minimum, int64_t &maximum);
    
    void defTimeIntersection(MPoints &subset, MBools &result);
    
  private:
    struct GridEntry {
      int row, unitId;
      bool operator==(const GridEntry & b) const;
      bool operator< (const GridEntry & b) const;
    };

    typedef Grid<3, 31, GridEntry> GridIndex;

    std::shared_ptr<MPointsData> m_MPointsData;
    GridIndex::Mbr m_Mbr;
    std::shared_ptr<GridIndex> m_GridIndex;
  };




  inline MPoints::MPoints() :
    m_MPointsData(std::make_shared<MPointsData>())
  {
  }

  inline MPoints::MPoints(CRelAlgebra::Reader& source)
  {
    m_MPointsData = std::make_shared<MPointsData>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Mbr), sizeof(m_Mbr));

    bool hasGrid;
    source.ReadOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex = std::make_shared<GridIndex>(source);
  }

  inline MPoints::MPoints(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_MPointsData = std::make_shared<MPointsData>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Mbr), sizeof(m_Mbr));

    bool hasGrid;
    source.ReadOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex = std::make_shared<GridIndex>(source);
  }

  inline MPoints::MPoints(const MPoints &array, 
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    MObjects(array, filter),
    m_MPointsData(array.m_MPointsData),
    m_Mbr(array.m_Mbr),
    m_GridIndex(array.m_GridIndex)
  {
  }

  inline void MPoints::addRow()
  {
    m_MPointsData->addRow();
    m_DefTimes->addRow();

    if (m_GridIndex.get() != 0)
      m_GridIndex->addRow();
  }

  inline void MPoints::addUnit(Interval interval, double x0, double y0, 
                                                  double x1, double y1)
  {
    int unitId;
    m_MPointsData->addUnit(interval, x0, y0, x1, y1, unitId);

    m_DefTimes->addInterval(interval);

    GridIndex::Mbr mbr;
    mbr.l[0] = std::min(x0, x1);
    mbr.h[0] = std::max(x0, x1);
    mbr.l[1] = std::min(y0, y1);
    mbr.h[1] = std::max(y0, y1);
    mbr.l[2] = static_cast<double>(interval.s);
    mbr.h[2] = static_cast<double>(interval.e);
    
    GridEntry e;
    e.row = m_MPointsData->rowCount() - 1;
    e.unitId = unitId;

    if (m_GridIndex.get() != 0)
      m_GridIndex->add(mbr, e);

    if (m_MPointsData->rowCount() == 1 && m_MPointsData->unitCount(0) == 0)
      m_Mbr = mbr;
    else
      m_Mbr = m_Mbr.unite(mbr);
  }

  inline MPointsData::UnitIterator MPoints::unitIterator(int row)
  {
    return m_MPointsData->unitIterator(row);
  }
  
  inline void MPoints::getDefTimeLimits(int64_t &minimum, int64_t &maximum)
  {
    m_DefTimes->getLimits(minimum, maximum);
  }
  
  inline void MPoints::defTimeIntersection(MPoints &subset, MBools &result)
  {
    checkr(m_DefTimes->rowCount() == subset.m_DefTimes->rowCount(),
           "different number of mpoints");
           
    for (int r = 0; r < m_DefTimes->rowCount(); r++) {
      result.addMObject();
      
      Interval ai, bi;
    
      int aii = m_DefTimes->intervalFirst(r);
      if (aii < m_DefTimes->intervalAfterLast(r))
        ai = m_DefTimes->interval(aii);

      int bii = subset.m_DefTimes->intervalFirst(r);
      if (bii < subset.m_DefTimes->intervalAfterLast(r))
        bi = subset.m_DefTimes->interval(bii);

      while (aii < m_DefTimes->intervalAfterLast(r)) 
      {
        if (bii >= subset.m_DefTimes->intervalAfterLast(r) ||
            ai.e < bi.s || (ai.e == bi.s && !ai.rc && bi.lc)) 
        {
          if (ai.s < ai.e || (ai.s == ai.e && ai.lc && ai.rc)) {
            BoolUnit u(ai, false);
            result.addUnit(u);
          }

          if (++aii < m_DefTimes->intervalAfterLast(r))
            ai = m_DefTimes->interval(aii);
        } else {
          if (ai.s < bi.s || (ai.s == bi.s && ai.lc && !bi.lc)) {
            BoolUnit u(Interval(ai.s, bi.s, ai.lc, !bi.lc), false);
            result.addUnit(u);
          }
          
          BoolUnit u(bi, true);
          result.addUnit(u);
          
          ai.s = bi.e;
          ai.lc = !bi.rc;

          if (++bii < subset.m_DefTimes->intervalAfterLast(r))
            bi = subset.m_DefTimes->interval(bii);
        }
      }
    }
  }

  inline bool MPoints::GridEntry::operator==(const GridEntry & b) const
  {
    return row == b.row && unitId == b.unitId;
  }

  inline bool MPoints::GridEntry::operator< (const GridEntry & b) const
  {
    if (row < b.row)
      return true;
    if (row > b.row)
      return false;
    
    return unitId < b.unitId;
  }
  
  
  
  inline void MPoints::atInstant(Instant instant, IPoints & result)
  {
    if (!instant.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.Append(temporalalgebra::IPoint(0));
      
      return;
    }
    
    if (m_GridIndex.get() != 0) {
      atInstantIndexed(instant, result);
      return;
    }

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

  inline void MPoints::atInstantIndexed(Instant instant, IPoints & result)
  {
    typedef std::map<int, MPointsData::Position> RowMap;
    typedef std::pair<int, MPointsData::Position> RowMapPair;

    RowMap rows;

    GridIndex::Mbr mbr = m_GridIndex->mbr();
    mbr.l[2] = mbr.h[2] = static_cast<double>(instant.millisecondsToNull());

    for (auto & e : m_GridIndex->selection(mbr)) {
      MPointsData::Unit u = m_MPointsData->unit(e.unitId);
      if (u.interval.contains(instant.millisecondsToNull()))
        rows.insert(RowMapPair(e.row, u.at(instant.millisecondsToNull())));
    }

    std::map<int, MPointsData::Position>::iterator si = rows.begin();
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
  }
  
  inline void MPoints::atPeriods(temporalalgebra::Periods periods, 
    MPoints & result)
  {
    if (!periods.IsDefined() || periods.GetNoComponents() == 0) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.addRow();
      
      return;
    }
    
    if (m_GridIndex.get() != 0) {
      atPeriodsIndexed(periods, result);
      return;
    }

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

  inline void MPoints::atPeriodsIndexed(temporalalgebra::Periods periods, 
    MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

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
  }
  
  inline void MPoints::passes(Point & value, CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined()) 
      return;

    if (m_GridIndex.get() != 0) {
      passesIndexed(value, result);
      return;
    }

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
  
  inline void MPoints::passesIndexed(Point & value, 
    CRelAlgebra::LongInts & result)
  {
    typedef std::set<int> Rows;
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
  }
  
  inline void MPoints::passes(Region & value, CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined()) 
      return;

    if (m_GridIndex.get() != 0) {
      passesIndexed(value, result);
      return;
    }

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
  
  inline void MPoints::passesIndexed(Region & value, 
    CRelAlgebra::LongInts & result)
  {
    typedef std::set<int> Rows;
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
  }

  inline void MPoints::at(Point & value, MPoints & result)
  {
    if (!value.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.addRow();

      return;
    }
    
    if (m_GridIndex.get() != 0) {
      atIndexed(value, result);
      return;
    }

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

  inline void MPoints::atIndexed(Point & value, MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

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
  }

  inline void MPoints::at(Region & value, MPoints & result)
  {
    if (!value.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addRow();
  
      return;
    }
    
    if (m_GridIndex.get() != 0) {
      atIndexed(value, result);
      return;
    }

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();

      result.addRow();

      MPointsData::UnitIterator iUnit = m_MPointsData->unitIterator(i);
      while (iUnit.hasNext()) {
        MPointsData::Unit unit = iUnit.next();

        temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                       unit.x0, unit.y0, unit.x1, unit.y1);
        std::vector<temporalalgebra::UPoint> r;
        upoint.AtRegion(&value, r);

        for (auto & ir : r) 
          result.addUnit(ir.timeInterval, 
                         ir.p0.GetX(), ir.p0.GetY(), 
                         ir.p1.GetX(), ir.p1.GetY());
      }
    }
  }

  inline void MPoints::atIndexed(Region & value, MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

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

      std::vector<temporalalgebra::UPoint> intersection;
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
  }

  inline void MPoints::index(CcInt & temporalSplits, CcInt & spatialSplits, 
    MPoints & result)
  {
    GridIndex::IVector splits;
    splits.p[0] = splits.p[1] = spatialSplits.GetIntval();
    splits.p[2] = temporalSplits.GetIntval();

    GridIndex::DVector cellSize;
    for (int i = 0; i < 3; i++)
      cellSize.s[i] = (m_Mbr.h[i] - m_Mbr.l[i]) / splits.p[i];

    result.m_GridIndex = std::make_shared<GridIndex>(splits, cellSize);

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(*GetAttribute(i));
    }
  }

  inline void MPoints::addRandomRows(CcInt& size, MPoints & result)
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
            v[d] = -std::abs(v[d]);
          if (p[d] < 0.0)
            v[d] = std::abs(v[d]);
        }
      }
    }
  }

}
