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
#include "Array.h"
#include "Interval.h"

namespace ColumnMovingAlgebra {

  class MPointsData {
  public:
    struct Position {
      double x, y;
    };

    struct Unit {
      int id;
      Interval interval;
      double x0, y0, x1, y1;

      Unit() = default;
      Unit(Interval interval, double x0, double y0, double x1, double y1);

      Position at(int64_t t);
      Unit at(int64_t s, int64_t e, bool lc, bool rc);
    };

    class UnitIterator {
    public:
      bool hasNext();
      Unit next();

    private:
      UnitIterator(MPointsData * frames, int currentFrame, int afterLastFrame);

      MPointsData * m_MPointsData;
      int m_CurrentFrame, m_AfterLastFrame;

      friend MPointsData;
    };

    MPointsData() = default;
    MPointsData(CRelAlgebra::Reader& source);

    void load(CRelAlgebra::Reader& source);
    void save(CRelAlgebra::Writer &target);
    int savedSize();

    void clear();
    void addRow();
    void removeRow();
    void addUnit(Interval interval, double x0, double y0, 
                                    double x1, double y1, int & unitId);

    int rowCount();
    int unitCount(int row);
    UnitIterator unitIterator(int row);
    Unit unit(int unitId);
    bool position(int row, int64_t time, Position & result);

    /*
    UnitIterator unitIterator(int row, bool forward = true);
    UnitIterator unitIterator(int row, int startAtUnitId, bool forward = true);

    class Row;
    class UnitSequence;

    class UnitIterator {
    public:
      UnitIterator & operator ++ ();
      Unit operator * ();
      bool operator != (const UnitIterator & b);

    private:
      MPointsData * m_MPointsData;
      bool m_Forward;
      int m_unitId;

      friend UnitSequence;
    };

    class UnitSequence {
    public:
      UnitIterator begin();
      UnitIterator end();

    private:
      MPointsData * m_MPointsData;
      int m_Row;
      int m_firstUnitId;
      int m_lastUnitId;

      friend Row;
    };

    class Row {
    public:
      count();
      unitSequence(bool forward = true);
      unitSequence(int startAtUnitId, bool forward = true);

    private:
      MPointsData * m_MPointsData;
      int m_Row;
    };
    Row row(int index);
    */

  private:
    static const char LEFT_CLOSED = 0, LEFT_OPEN = 1, RIGHT_CLOSED = 2, 
      RIGHT_OPEN = 3, LEFT_AND_RIGHT_CLOSED = 4, CHAINED = 5; 

    struct Frame {
      int64_t time;
      double x, y;
      char type;
    };

    struct Row {
      int unitCount;
      int firstFrame;
    };

    Array<Frame> m_Frames;
    Array<Row> m_Rows;

    void getUnit(int firstFrameOfUnit, Unit &unit, int &firstFrameOfNextUnit);
  };



  inline MPointsData::Unit::Unit(Interval interval, double x0, double y0, 
                                                    double x1, double y1) :
    interval(interval),
    x0(x0),
    y0(y0),
    x1(x1),
    y1(y1)
  {
  }

  inline MPointsData::Position MPointsData::Unit::at(int64_t t)
  {
    Position p;

    double l = static_cast<double>(interval.length());
    if (l != 0.0) {
      double f = (interval.e - t) / l;
      p.x = f * x0 + (1.0 - f) * x1;
      p.y = f * y0 + (1.0 - f) * y1;
    } else {
      p.x = x0;
      p.y = y0;
    }

    return p;
  }

  inline MPointsData::Unit MPointsData::Unit::at(int64_t s, int64_t e, 
                                                 bool lc, bool rc)
  {
    Unit u;
    u.id = -1;

    u.interval.s = s;
    u.interval.e = e;
    u.interval.lc = lc;
    u.interval.rc = rc;

    Position p0 = at(s), p1 = at(e);
    u.x0 = p0.x;
    u.y0 = p0.y;
    u.x1 = p1.x;
    u.y1 = p1.y;

    return u;
  }




  inline bool MPointsData::UnitIterator::hasNext()
  {
    return m_CurrentFrame < m_AfterLastFrame;
  }

  inline MPointsData::Unit MPointsData::UnitIterator::next()
  {
    Unit unit;
    int firstFrameOfNextUnit;
    m_MPointsData->getUnit(m_CurrentFrame, unit, firstFrameOfNextUnit);
    m_CurrentFrame = firstFrameOfNextUnit;
    return unit;
  }

  inline MPointsData::UnitIterator::UnitIterator(MPointsData * frames, 
    int currentFrame, int afterLastFrame) :
    m_MPointsData(frames), 
    m_CurrentFrame(currentFrame),
    m_AfterLastFrame(afterLastFrame)
  {
  }



  inline MPointsData::MPointsData(CRelAlgebra::Reader& source)
  {
    load(source);
  }

  inline void MPointsData::load(CRelAlgebra::Reader& source)
  {
    m_Frames.load(source);
    m_Rows.load(source);
  }

  inline void MPointsData::save(CRelAlgebra::Writer &target)
  {
    m_Frames.save(target);
    m_Rows.save(target);
  }

  inline int MPointsData::savedSize() {
    return m_Frames.savedSize() + 
           m_Rows.savedSize();
  }

  inline void MPointsData::clear()
  {
    m_Frames.clear();
    m_Rows.clear();
  }

  inline void MPointsData::addRow()
  {
    Row r;
    r.firstFrame = m_Frames.size();
    r.unitCount = 0;
    m_Rows.push_back(r);
  }

  inline void MPointsData::removeRow()
  {
    m_Frames.resize(m_Rows.back().firstFrame);
    m_Rows.pop_back();
  }

  inline void MPointsData::addUnit(Interval interval, double x0, double y0, 
    double x1, double y1, int & unitId)
  {
    assert(interval.s <= interval.e);
    assert(interval.s != interval.e || (interval.lc && interval.rc));
    assert(m_Rows.size() > 0);

    m_Frames.emplace_back();
    Frame & f0 = m_Rows.back().unitCount > 0 ? 
      m_Frames[m_Frames.size() - 2] : 
      m_Frames.back();
    Frame & f1 = m_Frames.back();

    assert(m_Rows.back().unitCount == 0 || f0.type == RIGHT_CLOSED || 
           f0.type == RIGHT_OPEN || f0.type == LEFT_AND_RIGHT_CLOSED);
    assert(m_Rows.back().unitCount == 0 || f0.time <= interval.s);

    if ( m_Rows.back().unitCount == 0 || 
       f0.time != interval.s || f0.x != x0 || f0.y != y0 || 
      (f0.type == RIGHT_OPEN && !interval.lc) ) 
    {
      assert(m_Rows.back().unitCount == 0 || f0.time != interval.s || 
             f0.type == RIGHT_OPEN || !interval.lc);
      unitId = m_Frames.size() - 1;

      if (interval.s == interval.e) {
        assert(AlmostEqual(x0, x1) && AlmostEqual(y0, y1));
        f1.time = interval.s;
        f1.x = x0;
        f1.y = y0;
        f1.type = LEFT_AND_RIGHT_CLOSED;
      } else {
        m_Frames.emplace_back();
        Frame & f1 = m_Frames[m_Frames.size() - 2];
        Frame & f2 = m_Frames.back();

        f1.time = interval.s;
        f1.x = x0;
        f1.y = y0;
        f1.type = interval.lc ? LEFT_CLOSED : LEFT_OPEN;
        f2.time = interval.e;
        f2.x = x1;
        f2.y = y1;
        f2.type = interval.rc ? RIGHT_CLOSED : RIGHT_OPEN;
      }
    } else {
      assert(f0.type == RIGHT_OPEN || !interval.lc);
      unitId = m_Frames.size() - 2;

      f0.type = CHAINED;

      f1.time = interval.e;
      f1.x = x1;
      f1.y = y1;
      f1.type = interval.rc ? RIGHT_CLOSED : RIGHT_OPEN;
    }

    m_Rows.back().unitCount++;
  }

  inline int MPointsData::rowCount()
  {
    return m_Rows.size();
  }

  inline int MPointsData::unitCount(int row)
  {
    return m_Rows[row].unitCount;
  }

  inline MPointsData::Unit MPointsData::unit(int unitId)
  {
    Unit unit;
    int firstFrameOfNextUnit;
    getUnit(unitId, unit, firstFrameOfNextUnit);
    return unit;
  }

  inline bool MPointsData::position(int row, int64_t time, Position & result)
  {
    int iFirst = m_Rows[row].firstFrame;
    int iAfterLast = row < static_cast<int>(m_Rows.size()) - 1 ?
      m_Rows[row + 1].firstFrame :
      m_Frames.size();

    if (iFirst >= iAfterLast)
      return false;

    Frame * first = m_Frames.data() + iFirst;
    Frame * afterLast = m_Frames.data() + iAfterLast;

    Frame * f1 = lower_bound(first, afterLast, time,
      [](const Frame &a, const int64_t &b) -> bool { return a.time < b; } );

    if (f1 == afterLast)
      return false;

    if (f1->time == time) {
      if (f1->type == LEFT_OPEN)
        return false;

      if (f1->type == RIGHT_OPEN) {
        f1++;

        if ( f1 == afterLast || f1->time > time || 
            (f1->type != LEFT_CLOSED && f1->type != LEFT_AND_RIGHT_CLOSED))
          return false;
      }

      result.x = f1->x;
      result.y = f1->y;
      return true;
    }

    if (f1 == first)
      return false;

    Frame * f0 = f1;
    f0--;

    if (f0->type != LEFT_OPEN && f0->type != LEFT_CLOSED && 
        f0->type != CHAINED)
      return false;

    if (f0->time == f1->time) {
      result.x = f1->x;
      result.y = f1->y;
      return true;
    }

    double d = static_cast<double>(time - f0->time) / 
               static_cast<double>(f1->time - f0->time);
    result.x = f0->x * (1.0 - d) + f1->x * d;
    result.y = f0->y * (1.0 - d) + f1->y * d;
    return true;
  }

  inline MPointsData::UnitIterator MPointsData::unitIterator(int row)
  {
    int afterLastUnit = row < static_cast<int>(m_Rows.size()) - 1 ?
      m_Rows[row + 1].firstFrame :
      m_Frames.size();

    return UnitIterator(this, m_Rows[row].firstFrame, afterLastUnit);
  }

  inline void MPointsData::getUnit(int firstFrameOfUnit, Unit & unit, 
    int & firstFrameOfNextUnit)
  {
    Frame & f0 = m_Frames[firstFrameOfUnit];
    assert(f0.type != RIGHT_CLOSED && f0.type != RIGHT_OPEN);

    if (f0.type == LEFT_AND_RIGHT_CLOSED) {
      firstFrameOfNextUnit = firstFrameOfUnit + 1;

      unit.id = firstFrameOfUnit;
      unit.interval.s = f0.time;
      unit.interval.e = f0.time;
      unit.interval.lc = true;
      unit.interval.rc = true;
      unit.x0 = f0.x;
      unit.y0 = f0.y;
      unit.x1 = f0.x;
      unit.y1 = f0.y;
    } else {
      assert(firstFrameOfUnit < static_cast<int>(m_Frames.size()) - 1);
      Frame & f1 = m_Frames[firstFrameOfUnit + 1];
      assert(f1.type != LEFT_CLOSED && f1.type != LEFT_OPEN && 
             f1.type != LEFT_AND_RIGHT_CLOSED);

      bool lc, rc;
      if (f0.type == LEFT_CLOSED) {
        lc = true;
      } else if (f0.type == CHAINED) {
        lc = true;
      } else if (f0.type == LEFT_OPEN) {
        lc = false;
      }

      if (f1.type == RIGHT_CLOSED) {
        rc = true;
        firstFrameOfNextUnit = firstFrameOfUnit + 2;
      } else if (f1.type == CHAINED) {
        rc = false;
        firstFrameOfNextUnit = firstFrameOfUnit + 1;
      } else if (f1.type == RIGHT_OPEN) {
        rc = false;
        firstFrameOfNextUnit = firstFrameOfUnit + 2;
      }

      unit.id = firstFrameOfUnit;
      unit.interval.s = f0.time;
      unit.interval.e = f1.time;
      unit.interval.lc = lc;
      unit.interval.rc = rc;
      unit.x0 = f0.x;
      unit.y0 = f0.y;
      unit.x1 = f1.x;
      unit.y1 = f1.y;
    }
  }

}
