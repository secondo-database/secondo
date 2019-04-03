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

1 MPointsData.h

*/

#pragma once

#include "AlmostEqual.h"
#include "Array.h"
#include "Interval.h"
#include "Check.h"

namespace ColumnMovingAlgebra {

/*
1.1 Declaration of ~MPointsData~

~MPointsData~ represents the data of a moving points attribute array.
It uses a compression algorithm to save the units. As an layer of abstraction
to make the algorithms in MPoints independent of the actual compression 
algorithm used we implement the compression 
in the additional class MPointsData and 
not directly in the class MPoints.

*/
  class MPointsData {
  public:
/*
1.1.1 Nested Classes

~Position~ represents a simple 2D-vector

*/
    struct Position {
      double x, y;
    };

/*
~Unit~ represents a moving point unit. When the unit is retrieved from 
MPointsData the member ~id~ is set to an integer
that uniquely identifies the unit and so allows direct access to this unit.
This is important when units shall be indexed in an grid index.

*/
    struct Unit {
      int id;
      Interval interval;
      double x0, y0, x1, y1;

      Unit() = default;
      Unit(Interval interval, double x0, double y0, double x1, double y1);

/*
~at~ performs a linear interpolation to compute the position of the unit
at a certain instant ~t~ or restrict the unit to a time interval

*/
      Position at(int64_t t);
      Unit at(int64_t s, int64_t e, bool lc, bool rc);
    };

/*
~UnitIterator~ represents an iterator over all units of a moving point and
is needed by operators in MPoints, that scan all units.

*/
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

/*
1.1.2 Constructors

*/
    MPointsData() = default;
    MPointsData(CRelAlgebra::Reader& source);

/*
1.1.3 Loading and Saving

The following funktions are needed in the context of persistant storage.

*/
    void load(CRelAlgebra::Reader& source);
    void save(CRelAlgebra::Writer &target);
    int savedSize();

/*
1.1.4 Data Access Functions

~clear~ removes all moving points

*/
    void clear();
/*
~addRow~ adds a new moving point

*/
    void addRow();
/*
~removeRow~ removes the last added moving point

*/
    void removeRow();
/*
~addUnit~ adds a new unit to the the last added moving point.
~unitId~ is set to an integer that uniquely identifies the unit 
and so allows direct access to this unit.
This is important when units shall be indexed in an grid index.

*/
    void addUnit(Interval interval, double x0, double y0, 
                                    double x1, double y1, int & unitId);

/*

~rowCount~ returns the number of moving points

*/
    int rowCount();
/*
~empty~ return true if there
are no moving points or if there are no units in any moving point

*/
    bool empty();
/*
~unitCount~ returns the number of units of the moving point with index ~row~

*/
    int unitCount(int row);
/*
~unitIterator~ returns a unit iterator for the moving point with index ~row~

*/
    UnitIterator unitIterator(int row);
/*
~unit~ returns the unit identified by ~unitId~

*/
    Unit unit(int unitId);
/*
~position~ implements an algorithm for the atinstant operator. if
the moving point with index ~row~ is defined at ~time~ it returns true
and sets ~result~ to the position of the moving point. Otherwise it returns
false.

*/
    bool position(int row, int64_t time, Position & result);

  private:
/*
1.1.1 Attributes and Constants

The moving point units are saved as frames. For each moving point unit a frame
is saved for the start and end of the unit. The frame saves a x- and
y-coordinate, a time and a type.
The following constants can be used as type.
They determine, if a frame belongs to the start (left side)
or end (right side) of a unit and whether the corresponding time interval
boundary of the unit is open or closed.
If the time interval of a unit starts at the same time as the time interval
of the unit before ends, then the corresponding frames are merged to one frame 
and with the type CHAINED.
If the start and end of a units time interval are equal, the frames can
also be merged and the type LEFT AND RIGHT CLOSED is used.

*/
    static const char LEFT_CLOSED = 0, LEFT_OPEN = 1, RIGHT_CLOSED = 2, 
      RIGHT_OPEN = 3, LEFT_AND_RIGHT_CLOSED = 4, CHAINED = 5; 

/*
~Frame~ represents a frame 

*/
    struct Frame {
      int64_t time;
      double x, y;
      char type;
    };

/*
~Row~ represents a moving point. ~firstFrame~ is the index of the first
frame that belongs to the moving point 

*/
    struct Row {
      int unitCount;
      int firstFrame;
    };

/*
~mFrames~ represents all frames of all moving points 

*/
    Array<Frame> m_Frames;
/*
~mRows~ represents all moving points

*/
    Array<Row> m_Rows;

/*
1.1.1 getUnit

~getUnit~ sets ~unit~ to the unit that starts with the frame with
index ~firstFrameOfUnit~. ~firstFrameOfNextUnit~ is set to the
index of the next unit of the same moving point.

*/
    void getUnit(int firstFrameOfUnit, Unit &unit, int &firstFrameOfNextUnit);
  };



/*
1.1 Implementation of ~MPointsData~

1.1.1 Implementation of Nested Class ~Unit~

*/
  inline MPointsData::Unit::Unit(Interval interval, double x0, double y0, 
                                                    double x1, double y1) :
    interval(interval),
    x0(x0),
    y0(y0),
    x1(x1),
    y1(y1)
  {
  }

/*
~at~ performs a linear interpolation to compute the position of the unit
at a certain instant ~t~ 

*/
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

/*
~at~ performs a linear interpolation to restrict the unit to a time interval

*/
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




/*
1.1.1 Implementation of Nested Class ~UnitIterator~

*/
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



/*
1.1.1 Constructors

*/
  inline MPointsData::MPointsData(CRelAlgebra::Reader& source)
  {
    load(source);
  }

/*
1.1.3 Loading and Saving

The following funktions are needed in the context of persistant storage.

*/
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

/*
1.1.4 Data Access Functions

~clear~ removes all moving points

*/
  inline void MPointsData::clear()
  {
    m_Frames.clear();
    m_Rows.clear();
  }

/*
~addRow~ adds a new moving point

*/
  inline void MPointsData::addRow()
  {
    Row r;
    r.firstFrame = m_Frames.size();
    r.unitCount = 0;
    m_Rows.push_back(r);
  }

/*
~removeRow~ removes the last added moving point

*/
  inline void MPointsData::removeRow()
  {
    m_Frames.resize(m_Rows.back().firstFrame);
    m_Rows.pop_back();
  }

/*
~addUnit~ adds a new unit to the the last added moving point.
~unitId~ is set to an integer that uniquely identifies the unit.
When adding the unit a transformation to the compressed frame representation is
necessary. This is performed by a case differentiation. The important
question is, whether the unit is connected to the unit before, which
means the end point of the last unit is identical to the start of the new unit.

*/
  inline void MPointsData::addUnit(Interval interval, double x0, double y0, 
    double x1, double y1, int & unitId)
  {
    check(interval.s <= interval.e, "interval must start before its end");
    check(interval.s != interval.e || (interval.lc && interval.rc), 
          "point interval must be left and right closed");
    check(m_Rows.size() > 0, "addRow must be called before addUnit");

    m_Frames.emplace_back();
    Frame & f0 = m_Rows.back().unitCount > 0 ? 
      m_Frames[m_Frames.size() - 2] : 
      m_Frames.back();
    Frame & f1 = m_Frames.back();

    check(m_Rows.back().unitCount == 0 || f0.type == RIGHT_CLOSED || 
          f0.type == RIGHT_OPEN || f0.type == LEFT_AND_RIGHT_CLOSED,
          "last interval not finished");
    check(m_Rows.back().unitCount == 0 || f0.time <= interval.s,
          "intervals not disjoint and ordered");

    if ( m_Rows.back().unitCount == 0 || 
         f0.time != interval.s || f0.x != x0 || f0.y != y0 || 
         (f0.type == RIGHT_OPEN && !interval.lc) ) 
    {
    
      //Not connected to the last unit
      
      check(m_Rows.back().unitCount == 0 || f0.time != interval.s || 
            f0.type == RIGHT_OPEN || !interval.lc, "intervals overlap");
      unitId = m_Frames.size() - 1;

      if (interval.s == interval.e) {
        check(AlmostEqual(x0, x1) && AlmostEqual(y0, y1), 
              "point interval must start and end at the same point");
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
    
      //connected to the last unit
      
      check(f0.type == RIGHT_OPEN || !interval.lc, "intervals overlap");
      unitId = m_Frames.size() - 2;

      f0.type = CHAINED;

      f1.time = interval.e;
      f1.x = x1;
      f1.y = y1;
      f1.type = interval.rc ? RIGHT_CLOSED : RIGHT_OPEN;
    }

    m_Rows.back().unitCount++;
  }

/*

~rowCount~ returns the number of moving points

*/
  inline int MPointsData::rowCount()
  {
    return m_Rows.size();
  }

/*
~empty~ return true if there
are no moving points or if there are no units in any moving point

*/
  inline bool MPointsData::empty()
  {
    return m_Frames.size() == 0;
  }

/*
~unitCount~ returns the number of units of the moving point with index ~row~

*/
  inline int MPointsData::unitCount(int row)
  {
    return m_Rows[row].unitCount;
  }
  
/*
~unit~ returns the unit identified by ~unitId~

*/
  inline MPointsData::Unit MPointsData::unit(int unitId)
  {
    Unit unit;
    int firstFrameOfNextUnit;
    getUnit(unitId, unit, firstFrameOfNextUnit);
    return unit;
  }

/*
~position~ implements an algorithm for the atinstant operator. if
the moving point with index ~row~ is defined at ~time~ it returns true
and sets ~result~ to the position of the moving point. Otherwise it returns
false. The search for the unit overlapping ~time~ is implemented as a binary
search on the frames.

*/
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

    Frame * f1 = std::lower_bound(first, afterLast, time,
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

/*
~unitIterator~ returns a unit iterator for the moving point with index ~row~

*/
  inline MPointsData::UnitIterator MPointsData::unitIterator(int row)
  {
    int afterLastUnit = row < static_cast<int>(m_Rows.size()) - 1 ?
      m_Rows[row + 1].firstFrame :
      m_Frames.size();

    return UnitIterator(this, m_Rows[row].firstFrame, afterLastUnit);
  }

/*
1.1.1 getUnit

~getUnit~ sets ~unit~ to the unit that starts with the frame with
index ~firstFrameOfUnit~. ~firstFrameOfNextUnit~ is set to the
index of the next unit of the same moving point.

*/
  inline void MPointsData::getUnit(int firstFrameOfUnit, Unit & unit, 
    int & firstFrameOfNextUnit)
  {
    Frame & f0 = m_Frames[firstFrameOfUnit];
    check(f0.type != RIGHT_CLOSED && f0.type != RIGHT_OPEN, 
          "frame is not start of a unit");

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
      checkr(firstFrameOfUnit < static_cast<int>(m_Frames.size()) - 1,
            "frame is not start of a unit");
      Frame & f1 = m_Frames[firstFrameOfUnit + 1];
      check(f1.type != LEFT_CLOSED && f1.type != LEFT_OPEN && 
            f1.type != LEFT_AND_RIGHT_CLOSED, "frames inconsistant");

      bool lc = false, rc=true; // will be overwritten later
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
