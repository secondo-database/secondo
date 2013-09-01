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


//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Header File for the class ~Hobby~

[TOC]

1 Overview

This header file contains all structs and classes required for the
class ~Hobby~.

This class implements the (slightly modified) Hobby algorithm.

To use this class, derive a data class from ~IntersectionAlgorithmData~ and 
overwrite the necessary methods.

1 Includes

*/
#pragma once

#include <limits>
#include <unordered_set>

#include "BentleyOttmann.h"

namespace RobustPlaneSweep
{
/*

1 Class ~Hobby~

*/
class Hobby : public BentleyOttmann
{
private:
/*

1.1 Private enum class ~HobbyEventType~

*/
  enum class HobbyEventType
  {
    Undefined,
    EndTolaranceSquare,
    BeginTolaranceSquare,
    SegmentEnter,
    SegmentStart,
    SegmentEnd,
    SegmentExit,
  };

/*

1.1 Private class ~HobbyEvent~

*/
  class HobbyEvent
  {
/*

1.1.1 Member variables

*/
  private:
    Rational _y;
    InternalLineSegment* _segment;
    HobbyEventType _eventType;

  public:
/*

1.1.1 Constructor

*/
    HobbyEvent(HobbyEventType eventType,
               Rational y,
               InternalLineSegment* segment) :
        _y(y),
        _segment(segment),
        _eventType(eventType)
    {
    }

/*

1.1.1 ~GetEventType~

*/
    HobbyEventType GetEventType() const
    {
      return _eventType;
    }

/*

1.1.1 ~GetY~

*/ 
    Rational GetY() const
    {
      return _y;
    }

/*

1.1.1 ~GetSegment~

*/  
    InternalLineSegment* GetSegment() const
    {
      return _segment;
    }

/*

1.1.1 ~Compare~

*/
    static int Compare(const HobbyEvent &x, const HobbyEvent &y)
    {
      int result = Rational::Compare(x._y, y._y);

      if (result == 0) {
        HobbyEventType xe = x._eventType;
        HobbyEventType ye = y._eventType;

        if (x._eventType == HobbyEventType::SegmentExit
            && y._eventType == HobbyEventType::SegmentExit) {
          if (x._segment->GetSlope() < y._segment->GetSlope()) {
            result = 1;
            return result;
          } else if (x._segment->GetSlope() > y._segment->GetSlope()) {
            result = -1;
            return result;
          }
        } else if (x._eventType == HobbyEventType::SegmentExit) {
          if (x._segment->GetSlope() < 0) {
            result = 1;
            return result;
          } else if (x._segment->GetSlope() > 0) {
            result = -1;
            return result;
          }
        } else if (y._eventType == HobbyEventType::SegmentExit) {
          if (y._segment->GetSlope() < 0) {
            result = -1;
            return result;
          } else if (y._segment->GetSlope() > 0) {
            result = 1;
            return result;
          }
        }

        if (xe == HobbyEventType::BeginTolaranceSquare
            && ye == HobbyEventType::EndTolaranceSquare) {
          return 1;
        } else if (xe == HobbyEventType::EndTolaranceSquare
                   && ye == HobbyEventType::BeginTolaranceSquare) {
          return -1;
        }

        if (xe == HobbyEventType::EndTolaranceSquare) {
          return -1;
        } else if (ye == HobbyEventType::EndTolaranceSquare) {
          return 1;
        }

        if (xe == HobbyEventType::BeginTolaranceSquare) {
          return -1;
        } else if (ye == HobbyEventType::BeginTolaranceSquare) {
          return 1;
        }
      }

      return result;
    }

/*

1.1 ~operator\textless~

*/
    bool operator<(const HobbyEvent &y) const
    {
      return Compare(*this, y) < 0;
    }
  };

/*

1.1 Private struct ~IgnoredIntersectionsComparer~

*/
  struct IgnoredIntersectionsComparer
  {
    size_t operator()(const std::pair<int, InternalLineSegment*> &x) const
    {
      return ((size_t)x.first) ^ ((size_t)x.second);
    }

    bool operator()(const std::pair<int, InternalLineSegment*> &x,
                    const std::pair<int, InternalLineSegment*> &y) const
    {
      return (x.first == y.first && x.second == y.second);
    }
  };

/*

1.1 Member variables

*/
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
  void AddEvents();
  bool AddSegmentEvent(int topSquareEdge,
                       int bottomSquareEdge,
                       int direction,
                       InternalLineSegment* segment);

  AvlTreeNode<InternalLineSegment*, SweepStateData*>*
  GetStartNode(const Rational& searchY) const;

protected:
/*

1.1 Methods

1.1.1 ~GetInitialScaleFactor~

*/
  int GetInitialScaleFactor() const
  {
    return 2;
  }

/*

1.1.1 ~GetBreakupUntil~

*/
  int GetBreakupUntil() const
  {
    return _currentHammock - _spacing - 1;
  }

/*

1.1.1 ~DetermineIntersectionsInternal~

*/
  void DetermineIntersectionsInternal();

/*

1.1.1 ~OnXChanged~

*/
  bool OnXChanged(const Rational& /* oldx */, const Rational& newX)
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

/*

1.1.1 ~BeforeProcessEvent~

*/
  void BeforeProcessEvent(SweepEvent* sweepEvent)
  {
    int square = GetTransformation()->
        RoundRational(sweepEvent->GetPoint().GetY());

    if (_addedSquares.insert(square).second) {
      _events.push_back(HobbyEvent(HobbyEventType::BeginTolaranceSquare,
                                   Rational(square - _spacing, 0, 1),
                                   NULL));

      _events.push_back(HobbyEvent(HobbyEventType::EndTolaranceSquare,
                                   Rational(square + _spacing, 0, 1),
                                   NULL));
    }

    if (sweepEvent->GetEventType() == SweepEventType::Start) {
      if (!_addedSegments.insert(sweepEvent->GetSegment()).second) {
        throw new std::logic_error("segment could not be "
                                   "added to _addedSegments!");
      }
      if (!_openSegments.insert(sweepEvent->GetSegment()).second) {
        throw new std::logic_error("segment could not be "
                                   "added to _openSegments!");
      }
      _ignoredIntersections.insert(std::make_pair(square,
                                                  sweepEvent->GetSegment()));
      _events.push_back(HobbyEvent(HobbyEventType::SegmentStart,
                                   sweepEvent->GetPoint().GetY(),
                                   sweepEvent->GetSegment()));
    } else if (sweepEvent->GetEventType() == SweepEventType::End) {
      if (_openSegments.erase(sweepEvent->GetSegment()) == 0) {
        _events.push_back(HobbyEvent(HobbyEventType::SegmentEnter,
                                     sweepEvent->GetSegment()->
                                         GetYValueAt(_currentHammockStart),
                                     sweepEvent->GetSegment()));
        if (!_addedSegments.insert(sweepEvent->GetSegment()).second) {
          throw new std::logic_error("segment could not be "
                                     "added to _addedSegments!");
        }
      }
      _ignoredIntersections.insert(std::make_pair(square,
                                                  sweepEvent->GetSegment()));
      _events.push_back(HobbyEvent(HobbyEventType::SegmentEnd,
                                   sweepEvent->GetPoint().GetY(),
                                   sweepEvent->GetSegment()));
    } else if (sweepEvent->GetEventType() == SweepEventType::Intersection) {
      std::unordered_set<InternalLineSegment*>* intersectedSegments =
          sweepEvent->GetIntersectedSegments();
      for (std::unordered_set<InternalLineSegment*>::const_iterator
      s = intersectedSegments->begin();
          s != intersectedSegments->end(); ++s) {
        _ignoredIntersections.insert(std::make_pair(square, *s));
        if (_openSegments.insert(*s).second) {
          _events.push_back(HobbyEvent(HobbyEventType::SegmentEnter,
                                       (*s)->GetYValueAt(_currentHammockStart),
                                       *s));
          if (!_addedSegments.insert(*s).second) {
            throw new std::logic_error("segment could not be "
                                       "added to _addedSegments!");
          }
        }
      }
    } else if (sweepEvent->GetEventType() == SweepEventType::Point) {
    } else {
      throw new std::invalid_argument("sweepEvent");
    }
  }

/*

1.1.1 ~Finished~

*/
  void Finished()
  {
    ProcessEvents();
  }

public:
/*

1.1.1 Constructor

*/
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
