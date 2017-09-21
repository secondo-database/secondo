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

1 MPoints.h

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
#include <chrono>

namespace ColumnMovingAlgebra
{

/*
1.1 Declaration of the class ~MPoints~

~MPoints~ represents a moving point attribute array.

*/
  class MPoints : public MObjects
  {
  public:
/*
1.1.1 Constructors 

The following constructor signatures are required by the crel algebra for all 
attribute arrays.

*/
    MPoints();
    MPoints(CRelAlgebra::Reader& source);
    MPoints(CRelAlgebra::Reader& source, size_t rowsCount);
    MPoints(const MPoints &array, 
            const CRelAlgebra::SharedArray<const size_t> &filter);
/*
1.1.2 Destructor

*/
    virtual ~MPoints() { }

/*
1.1.3 CRel Algebra Interface

the following functions are required by the crel algebra for all attribute 
arrays.

~Filter~ returns a duplicate of this attribut array with the speficied filter.

 
*/
    virtual AttrArray* Filter(CRelAlgebra::SharedArray<const size_t> 
                              filter) const;

/*
~GetCount~ returns the number of entries in the attribut array.

*/
    virtual size_t GetCount() const;
/*
~GetSize~ returns the amount of space needed to save this attribut array
to persistant storage.

*/
    virtual size_t GetSize() const;
/*
~GetAttribute~ converts the moving point 
in ~row~ to an MPoint as defined in the temporal algebra for row oriented
relations and returns it.

*/
    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

/*
~Save~ saves this attribut array
to persistant storage.

*/
    virtual void Save(CRelAlgebra::Writer &target, 
                      bool includeHeader = true) const;

/*
~Append~ adds the moving point at index ~row~ of the attribut array ~array~

*/
    virtual void Append(const CRelAlgebra::AttrArray & array, size_t row);
/*
or adds the row orientied MPoint ~value~

*/
    virtual void Append(Attribute & value);
/*
~Remove~ removes the last added moving point

*/
    virtual void Remove();
/*
~Clear~ removes all moving points

*/
    virtual void Clear();

/*
~IsDefined~ returns true, iff the moving point with index ~row~ has any units

*/
    virtual bool IsDefined(size_t row) const;

/*
~Compare~ compares the moving point at index ~rowA~ with the moving point
at index ~rowB~ in ~arrayB~

*/
    virtual int Compare(size_t rowA, const AttrArray& arrayB, 
      size_t rowB) const;
/*
~Compare~ compares the moving point at index ~rowA~ with the row oriented
attribute ~value~

*/
    virtual int Compare(size_t row, Attribute &value) const;

/*
~GetHash~ returns a hash value for the moving point at index ~row~

*/
    virtual size_t GetHash(size_t row) const;

/*
1.1.2 Operators

The following functions implement the operators supported by moving points
attribute array. The operators atinstant, atperiods, passes and at are
implemented in two versions for moving point attribute arrays with and 
without a grid index.

~atInstant~ is a timeslice operator and computes an intime for all 
moving points in the attribute array and adds them to ~result~

*/
    void atInstant(Instant instant, IPoints & result);
    void atInstantIndexed(Instant instant, IPoints & result);
    
/*
~atPeriods~ restricts the moving points to a given set of time 
intervals and adds the resulting units to ~result~.

*/
    void atPeriods(temporalalgebra::Periods periods, MPoints & result);
    void atPeriodsIndexed(temporalalgebra::Periods periods, MPoints & result);
    
/*
~passes~ adds the indices of all moving points to ~result~, which
ever assume the specified value or intersect the specified region. 

*/
    void passes(Point & value, CRelAlgebra::LongInts & result);
    void passesIndexed(Point & value, CRelAlgebra::LongInts & result);
    void passes(Region & value, CRelAlgebra::LongInts & result);
    void passesIndexed(Region & value, CRelAlgebra::LongInts & result);
    
/*
~at~ restricts the all moving points to the specified value or 
the specified region. the computed new units are added to ~result~

*/
    void at(Point & value, MPoints & result);
    void atIndexed(Point & value, MPoints & result);
    void at(Region & value, MPoints & result);
    void atIndexed(Region & value, MPoints & result);
    
/*
~addRandomUnits~ adds random units to every moving point 

*/
    void addRandomUnits(CcInt& size, MPoints & result);
    
/*
~index~ creates a new moving points attribute array with a grid index.
the index will cover the rectangular region specified by ~xMin~, ~xMax~,
~yMin~ and ~yMax~ and covers the time interval from ~tMin~ to ~tMax~. The
created grid is an infinite grid, so it will also index units which are
not within these boundaries, but this will be ineffective in most cases.
The number of grid cells in each dimension is determined by ~xSplits~, 
~ySplits~ and ~tSplits~. 

*/
    void index(CcReal& xMin, CcReal& xMax, CcInt& xSplits,
               CcReal& yMin, CcReal& yMax, CcInt& ySplits, 
               Instant& tMin, Instant& tMax, 
               CcInt& tSplits, MPoints & result);
    
/*
1.1.3 Data access

~addRow~ adds a new moving point to the attribute array

*/
    void addRow();
/*
~addUnit~ adds a new unit to the last added moving point

*/
    void addUnit(Interval interval, double x0, double y0, 
                                    double x1, double y1);

/*
~unitIterator~ returns an iterator over the units of the moving
point with index ~row~

*/
    MPointsData::UnitIterator unitIterator(int row);
    
/*
~getDefTimeLimits~ returns the minimum start and maximum end of all
units of all moving points in this attribute array

*/
    void getDefTimeLimits(int64_t &minimum, int64_t &maximum);
    
/*
1.1.4 Intersection of Definition Time

~defTimeIntersection~ is a helper function for the operator inside.
inside has a moving point operand and a moving region operand and
computes a moving boolean which is set to true respectively false
for all time intervals in which
the moving point is located inside respectively outside the moving region.
To compute this result inside first calls intersection on the two operands.
Then it calls ~defTimeIntersection~ on the moving point operand and uses
the result of intersection as parameter for ~defTimeIntersection~. 
This function then performs an intersection on the definition times of 
both moving points to calculate the result for inside.

*/
    void defTimeIntersection(MPoints &subset, MBools &result);
    
  private:
/*
1.1.5 Attributes

~GridEntry~ represents an entry in the grid index. 

*/
    struct GridEntry {
      int row, unitId;
      bool operator==(const GridEntry & b) const;
      bool operator< (const GridEntry & b) const;
    };

    typedef Grid<3, 31, GridEntry> GridIndex;

/*
~mMPointsData~ represents the data of all moving points in this attribute
array.

*/
    std::shared_ptr<MPointsData> m_MPointsData;
/*
~mMbr~ is the minimum bounding volume for all moving points in this attribute
array.

*/
    GridIndex::Mbr m_Mbr;
/*
~mGridIndex~ represents the grid index of all units of all moving points.
It is set to null if the moving points attribut array has no index.

*/
    std::shared_ptr<GridIndex> m_GridIndex;
  };

/*
1.2 Implementation of the inline functions for class ~MPoints~

1.2.1 Constructors 

The following constructor signatures are required by the crel algebra for all 
attribute arrays.

*/
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

/*
1.2.2 Data access

~addRow~ adds a new moving point to the attribute array

*/
  inline void MPoints::addRow()
  {
    m_MPointsData->addRow();
    m_DefTimes->addRow();

    if (m_GridIndex.get() != 0)
      m_GridIndex->addRow();
  }

/*
~addUnit~ adds a new unit to the last added moving point. The minimum bounding
volume of the attribute array is updated and the new unit is indexed if
the attribute array has a grid index.

*/
  inline void MPoints::addUnit(Interval interval, double x0, double y0, 
                                                  double x1, double y1)
  {
    GridIndex::Mbr mbr;
    mbr.l[0] = std::min(x0, x1);
    mbr.h[0] = std::max(x0, x1);
    mbr.l[1] = std::min(y0, y1);
    mbr.h[1] = std::max(y0, y1);
    mbr.l[2] = static_cast<double>(interval.s);
    mbr.h[2] = static_cast<double>(interval.e);

    if (m_MPointsData->empty())
      m_Mbr = mbr;
    else
      m_Mbr = m_Mbr.unite(mbr);
    
    int unitId;
    m_MPointsData->addUnit(interval, x0, y0, x1, y1, unitId);

    m_DefTimes->addInterval(interval);

    GridEntry e;
    e.row = m_MPointsData->rowCount() - 1;
    e.unitId = unitId;

    if (m_GridIndex.get() != 0)
      m_GridIndex->add(mbr, e);
  }

/*
~unitIterator~ returns an iterator over the units of the moving
point with index ~row~

*/
  inline MPointsData::UnitIterator MPoints::unitIterator(int row)
  {
    return m_MPointsData->unitIterator(row);
  }
  
/*
~getDefTimeLimits~ returns the minimum start and maximum end of all
units of all moving points in this attribute array

*/
  inline void MPoints::getDefTimeLimits(int64_t &minimum, int64_t &maximum)
  {
    m_DefTimes->getLimits(minimum, maximum);
  }
  
/*
1.2.3 Intersection of Definition Time

~defTimeIntersection~ is a helper function for the operator inside.
inside has a moving point operand and a moving region operand and
computes a moving boolean which is set to true respectively false
for all time intervals in which
the moving point is located inside respectively outside the moving region.
To compute this result inside first calls intersection on the two operands.
Then it calls ~defTimeIntersection~ on the moving point operand and uses
the result of intersection as parameter for ~defTimeIntersection~. 
This function then performs an intersection on the definition times of 
both moving points to calculate the result for inside.

*/
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

/*
1.1.4 Operators

The following functions implement the operators supported by moving points
attribute array. The operators atinstant, atperiods, passes and at are
implemented in two versions for moving point attribute arrays with and 
without a grid index.

~atInstant~ is a timeslice operator and computes an intime for all 
moving points in the attribute array and adds them to ~result~. The 
actual algorithm is implemented in the class MPointsData

*/
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

/*
~atInstantIndexed~ is the version of atinstant that uses the grid index.

*/
  inline void MPoints::atInstantIndexed(Instant instant, IPoints & result)
  {
    typedef std::map<int, MPointsData::Position> RowMap;
    typedef std::pair<int, MPointsData::Position> RowMapPair;

    RowMap rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;
    mbr.l[2] = mbr.h[2] = static_cast<double>(instant.millisecondsToNull());

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the instant
    //the results are added to a map because we need them ordered
    //by moving point index
    
    for (auto & e : m_GridIndex->selection(mbr)) {
      MPointsData::Unit u = m_MPointsData->unit(e.unitId);
      if (u.interval.contains(instant.millisecondsToNull()))
        rows.insert(RowMapPair(e.row, u.at(instant.millisecondsToNull())));
    }
    
    //now we can construct the result

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
  
/*
~atPeriods~ restricts the moving points to a given set of time 
intervals and adds the resulting units to ~result~.

*/
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
      
      //we perform a parallel scan on the given time intervals and the
      //units of each moving point

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

/*
~atPeriodsIndexed~ is the version of atperiods that uses the grid index.

*/
  inline void MPoints::atPeriodsIndexed(temporalalgebra::Periods periods, 
    MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

    RowMap rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;

    Instant instant;
    periods.Minimum(instant);
    mbr.l[2] = static_cast<double>(instant.millisecondsToNull());
    periods.Maximum(instant);
    mbr.h[2] = static_cast<double>(instant.millisecondsToNull());

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the periods
    //the results are added to a map because we need them ordered
    //by moving point index and time
    
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

    //now we can construct the result

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
  
/*
~passes~ adds the indices of all moving points to ~result~, which
ever assume the specified value. 

*/
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
  
/*
~passesIndexed~ is the version of passes that uses the grid index.

*/
  inline void MPoints::passesIndexed(Point & value, 
    CRelAlgebra::LongInts & result)
  {
    typedef std::set<int> Rows;
    Rows rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;
    mbr.l[0] = mbr.h[0] = value.GetX();
    mbr.l[1] = mbr.h[1] = value.GetY();

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the value
    //the results are added to a map because we need them ordered
    //by moving point index
    
    for (auto & e : m_GridIndex->selection(mbr)) {
      MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
      temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                     unit.x0, unit.y0, unit.x1, unit.y1);

      if (upoint.Passes(value))
        rows.insert(e.row);
    }

    //now we can construct the result

    Rows::iterator si = rows.begin();
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      while (si != rows.end() && *si < i)
        si++;

      if (si != rows.end() && *si == i)
        result.Append(i);
    }
  }
  
/*
~passes~ adds the indices of all moving points to ~result~, which
ever intersect the specified region. 

*/
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
  
/*
~passesIndexed~ is the version of passes that uses the grid index.

*/
  inline void MPoints::passesIndexed(Region & value, 
    CRelAlgebra::LongInts & result)
  {
    typedef std::set<int> Rows;
    Rows rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;
    Rectangle<2> rect = value.BoundingBox();
    for (int i = 0; i < 2; i++) {
      mbr.l[i] = rect.MinD(i);
      mbr.h[i] = rect.MaxD(i);
    }

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the region
    //the results are added to a map because we need them ordered
    //by moving point index
    
    for (auto & e : m_GridIndex->selection(mbr)) {
      MPointsData::Unit unit = m_MPointsData->unit(e.unitId);
      temporalalgebra::UPoint upoint(unit.interval.convert(), 
                                     unit.x0, unit.y0, unit.x1, unit.y1);

      if (upoint.Passes(value))
        rows.insert(e.row);
    }

    //now we can construct the result

    Rows::iterator si = rows.begin();
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      while (si != rows.end() && *si < i)
        si++;

      if (si != rows.end() && *si == i)
        result.Append(i);
    }
  }

/*
~at~ restricts the all moving points to the specified value. 
the computed new units are added to ~result~

*/
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

/*
~atIndexed~ is the version of at that uses the grid index.

*/
  inline void MPoints::atIndexed(Point & value, MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

    RowMap rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;
    mbr.l[0] = mbr.h[0] = value.GetX();
    mbr.l[1] = mbr.h[1] = value.GetY();

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the value
    //the results are added to a map because we need them ordered
    //by moving point index and time
    
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

    //now we can construct the result

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

/*
~at~ restricts the all moving points to the specified 
the specified region. the computed new units are added to ~result~

*/
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

/*
~atIndexed~ is the version of at that uses the grid index.

*/
  inline void MPoints::atIndexed(Region & value, MPoints & result)
  {
    typedef std::map<int64_t, MPointsData::Unit> UnitMap;
    typedef std::pair<int64_t, MPointsData::Unit> UnitMapPair;
    typedef std::map<int, UnitMap> RowMap;
    typedef std::pair<int, UnitMap> RowMapPair;

    RowMap rows;

    //first we construct a search volume
    
    GridIndex::Mbr mbr = m_Mbr;
    Rectangle<2> rect = value.BoundingBox();
    for (int i = 0; i < 2; i++) {
      mbr.l[i] = rect.MinD(i);
      mbr.h[i] = rect.MaxD(i);
    }

    //then we find all units within that volume with the help of the grid index
    //for each unit we check for intersection with the region
    //the results are added to a map because we need them ordered
    //by moving point index and time
    
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

    //now we can construct the result

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

/*
~index~ creates a new moving points attribute array with a grid index.
the index will cover the rectangular region specified by ~xMin~, ~xMax~,
~yMin~ and ~yMax~ and covers the time interval from ~tMin~ to ~tMax~. The
created grid is an infinite grid, so it will also index units which are
not within these boundaries, but this will be ineffective in most cases.
The number of grid cells in each dimension is determined by ~xSplits~, 
~ySplits~ and ~tSplits~. 

*/
  inline void MPoints::index(CcReal& xMin, CcReal& xMax, CcInt& xSplits,
    CcReal& yMin, CcReal& yMax, CcInt& ySplits, 
    Instant& tMin, Instant& tMax, 
    CcInt& tSplits, MPoints & result)
  {
    GridIndex::IVector splits;
    splits.p[0] = xSplits.GetValue();
    splits.p[1] = ySplits.GetValue();
    splits.p[2] = tSplits.GetValue();

    GridIndex::DVector min;
    min.s[0] = xMin.GetValue();
    min.s[1] = yMin.GetValue();
    min.s[2] = static_cast<double>(tMin.millisecondsToNull());

    GridIndex::DVector max;
    max.s[0] = xMax.GetValue();
    max.s[1] = yMax.GetValue();
    max.s[2] = static_cast<double>(tMax.millisecondsToNull());

    GridIndex::DVector delta = max - min, cellSize;
    
    for (int i = 0; i < 3; i++) {
      check(splits.p[i] >= 1, "the number of splits must be greater or equal "
                              "one.");
      cellSize.s[i] = delta.s[i] / std::max(1, splits.p[i]);
    }

    result.m_GridIndex = std::make_shared<GridIndex>(min, cellSize, splits);

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(*this, i);
    }
  }

/*
~addRandomUnits~ adds random units to every moving point 

*/
  inline void MPoints::addRandomUnits(CcInt& size, MPoints & result)
  {
    int64_t t0 = 0;
    
    if (!m_MPointsData->empty()) {
      t0 = (static_cast<int64_t>(m_Mbr.h[2] * 1.1) / MILLISECONDS + 1)
           * MILLISECONDS;
    }

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      result.Append(*this, i);
 
      if (!size.IsDefined())
        continue;
      
      int units = size.GetIntval();
        
      double p[2], v[2];
      int64_t t = t0;

      for (int d = 0; d < 2; d++) {
        p[d] = static_cast<double>(rand()) / RAND_MAX * 100.0;
        v[d] = static_cast<double>(rand()) / RAND_MAX * 1.0;
      }

      for (int u = 0; u < units; u++) {
        int64_t dt = (20 + (rand() % 5)) * MILLISECONDS;
        result.addUnit(Interval(t, t + dt, true, false), 
                       p[0], p[1], p[0] + v[0], p[1] + v[1]);

        t += dt;

        for (int d = 0; d < 2; d++) {
          p[d] += v[d];

          if (p[d] > 100.0)
            v[d] = -static_cast<double>(rand()) / RAND_MAX * 1.0;
          if (p[d] < 0.0)
            v[d] = static_cast<double>(rand()) / RAND_MAX * 1.0;
        }
      }
    }
  }

/*
1.1.5 Implementation of Nested Class GridEntry 

*/
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
  

}


