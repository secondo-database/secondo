/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

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

#include <limits>
#include <unordered_set>

#include "BentleyOttmann.h"

namespace RobustPlaneSweep
{
  class Hobby : public BentleyOttmann
  {
  private:
    enum HobbyEventType
    {
      Undefined,
      EndTolaranceSquare,
      BeginTolaranceSquare,
      SegmentEnter,
      SegmentBegin,
      SegmentEnd,
      SegmentExit,
    };

    class HobbyEvent
    {
    private:
      Rational _y;
      InternalLineSegment* _segment;
      HobbyEventType _eventType;

    public:
      HobbyEvent(
        HobbyEventType eventType,
        Rational y,
        InternalLineSegment* segment) :
      _y(y), _segment(segment), _eventType(eventType)
      {
      }

      HobbyEventType GetEventType() const
      {
        return _eventType;
      }

      Rational GetY() const
      {
        return _y;
      }

      InternalLineSegment* GetSegment() const
      {
        return _segment;
      }

      static int Compare(const HobbyEvent &x, const HobbyEvent &y)
      {
        int result = Rational::Compare(x._y, y._y);

        if (result == 0) {
          HobbyEventType xe = x._eventType;
          HobbyEventType ye = y._eventType;

          if (x._eventType == SegmentExit && y._eventType == SegmentExit) {
            if (x._segment->GetA() < y._segment->GetA()) {
              result = 1;
              return result;
            } else if (x._segment->GetA() > y._segment->GetA()) {
              result = -1;
              return result;
            }
          } else if (x._eventType == SegmentExit) {
            if (x._segment->GetA() < 0) {
              result = 1;
              return result;
            } else if (x._segment->GetA() > 0) {
              result = -1;
              return result;
            }
          } else if (y._eventType == SegmentExit) {
            if (y._segment->GetA() < 0) {
              result = -1;
              return result;
            } else if (y._segment->GetA() > 0) {
              result = 1;
              return result;
            }
          }

          if (xe == BeginTolaranceSquare && ye == EndTolaranceSquare) {
            return 1;
          } else if (xe == EndTolaranceSquare && ye == BeginTolaranceSquare) {
            return -1;
          }

          if (xe == EndTolaranceSquare) {
            return -1;
          } else if (ye == EndTolaranceSquare) {
            return 1;
          }

          if (xe == BeginTolaranceSquare) {
            return -1;
          } else if (ye == BeginTolaranceSquare) {
            return 1;
          }
        }

        return result;
      }

    public:
      bool operator<(const HobbyEvent &y) const
      {
        return Compare(*this, y) < 0;
      }
    };

    struct IgnoredIntersectionsComparer {
      size_t operator()(const std::pair<int, InternalLineSegment*> &x) const
      {
        return ((size_t)x.first)^((size_t)x.second);
      }

      bool operator()(
        const std::pair<int, InternalLineSegment*> &x,
        const std::pair<int, InternalLineSegment*> &y) const
      {
        return (x.first == y.first && x.second == y.second);
      }
    };

    int _currentHammock;
    int _currentHammockStart;
    int _currentHammockEnd;

    std::unordered_set<InternalLineSegment*> _addedSegments;
    std::unordered_set<InternalLineSegment*> _openSegments;
    std::unordered_set<
      std::pair<int, InternalLineSegment*>,
      IgnoredIntersectionsComparer,
      IgnoredIntersectionsComparer> _ignoredIntersections;

    std::vector<HobbyEvent> _events;
    std::unordered_set<int> _addedSquares;
    int _spacing;

    void ProcessEvents();
    void AddEvents(Rational currentX, HobbyEventType eventType);
    void AddEvents();

    AvlTreeNode<InternalLineSegment*, SweepStateData*>*
      GetStartNode(Rational searchY);

  protected:
    int GetInitialScaleFactor()
    {
      return 2;
    }

    int GetBreakupUntil()
    {
      return _currentHammock - _spacing - 1;
    }

    void DetermineIntersectionsInternal();

    bool OnXChanged(Rational /* oldx */, Rational newX)
    {
      if (newX >= _currentHammockEnd) {
        ProcessEvents();
        _currentHammock = GetTransformation()->RoundRational(newX);

        _currentHammockStart = _currentHammock - _spacing;
        _currentHammockEnd = _currentHammock + _spacing;
        return true;
      } else {
        return false;
      }
    }

    void BeforeProcessEvent(SweepEvent* sweepEvent)
    {
      int square = GetTransformation()->
        RoundRational(sweepEvent->GetPoint().GetY());

      if (_addedSquares.insert(square).second) {
        _events.push_back(
          HobbyEvent(
          BeginTolaranceSquare,
          Rational(square - _spacing, 0, 1),
          NULL));

        _events.push_back(
          HobbyEvent(
          EndTolaranceSquare,
          Rational(square + _spacing, 0, 1),
          NULL));
      }

      if (sweepEvent->GetEventType() == Start) {
        if (!_addedSegments.insert(sweepEvent->GetSegment()).second) {
          throw new std::logic_error(
            "segment could not be added to _addedSegments!");
        }
        if (!_openSegments.insert(sweepEvent->GetSegment()).second) {
          throw new std::logic_error(
            "segment could not be added to _openSegments!");
        }
        _ignoredIntersections.insert(
          std::make_pair(square, sweepEvent->GetSegment()));
        _events.push_back(
          HobbyEvent(
          SegmentBegin,
          sweepEvent->GetPoint().GetY(),
          sweepEvent->GetSegment()));
      } else if (sweepEvent->GetEventType() == End) {
        if (_openSegments.erase(sweepEvent->GetSegment()) == 0) {
          _events.push_back(
            HobbyEvent(
            SegmentEnter,
            sweepEvent->GetSegment()->GetYValueAt(
            InternalIntersectionPoint(_currentHammockStart, 0)),
            sweepEvent->GetSegment()));
          if (!_addedSegments.insert(sweepEvent->GetSegment()).second) {
            throw new std::logic_error(
              "segment could not be added to _addedSegments!");
          }
        }
        _ignoredIntersections.insert(
          std::make_pair(square, sweepEvent->GetSegment()));
        _events.push_back(
          HobbyEvent(
          SegmentEnd,
          sweepEvent->GetPoint().GetY(),
          sweepEvent->GetSegment()));
      } else if (sweepEvent->GetEventType() == Intersection ||
        sweepEvent->GetEventType() == TouchIntersection) {
          std::unordered_set<InternalLineSegment*>* intersectedSegments =
            sweepEvent->GetIntersectedSegments();
          for (std::unordered_set<InternalLineSegment*>::const_iterator
            s = intersectedSegments->begin();
            s != intersectedSegments->end(); ++s) {
              _ignoredIntersections.insert(std::make_pair(square, *s));
              if (_openSegments.insert(*s).second) {
                _events.push_back(
                  HobbyEvent(
                  SegmentEnter,
                  (*s)->GetYValueAt(
                  InternalIntersectionPoint(_currentHammockStart, 0)),
                  *s));
                if (!_addedSegments.insert(*s).second) {
                  throw new std::logic_error(
                    "segment could not be added to _addedSegments!");
                }
              }
          }
      } else {
        throw new std::invalid_argument("sweepEvent");
      }
    }

    void Finished()
    {
      ProcessEvents();
    }

  public:
    explicit Hobby(IntersectionAlgorithmData* data) :
    BentleyOttmann(data),
      _currentHammock(std::numeric_limits<int>::min()),
      _currentHammockStart(std::numeric_limits<int>::min()),
      _currentHammockEnd(std::numeric_limits<int>::min()),
      _spacing(0)
    {
    }
  };
}
