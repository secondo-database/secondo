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

#include <unordered_set>
#include <vector>
#include "SimpleSweepIntersectionAlgorithm.h"

namespace RobustPlaneSweep
{
class HobbyNaiveIntersectionAlgorithm :
    public SimpleSweepIntersectionAlgorithm
{
private:
  std::unordered_set<
      InternalPoint,
      InternalPointComparer,
      InternalPointComparer> _roundedPoints;

  int _spacing;

  enum class EventType
  {
    Undefined,
    BeginSegment,
    EndSegment,
    TolaranceSquare
  };

  class SweepEvent
  {
  private:
    int _spacing;
    InternalLineSegment* _segment;
    EventType _eventType;
    int _squareX;
    int _squareY;

  public:
    SweepEvent(int squareX, int squareY) :
        _spacing(0),
        _segment(NULL),
        _eventType(EventType::TolaranceSquare),
        _squareX(squareX),
        _squareY(squareY)
    {
    }

    SweepEvent(InternalLineSegment* segment,
               int spacing,
               bool isStart) :
        _spacing(spacing),
        _segment(segment),
        _eventType(isStart ? EventType::BeginSegment : EventType::EndSegment),
        _squareX(0),
        _squareY(0)
    {
    }

    EventType GetEventType() const
    {
      return _eventType;
    }

    InternalLineSegment* GetSegment() const
    {
      return _segment;
    }

    int GetSquareX() const
    {
      return _squareX;
    }

    int GetSquareY() const
    {
      return _squareY;
    }

  private:
    long long GetCompareValue() const
    {
      switch (_eventType) {
        case EventType::BeginSegment:
          return (_segment->GetLeft().GetX()) - _spacing;
        case EventType::EndSegment:
          return (_segment->GetRight().GetX()) + _spacing;
        case EventType::TolaranceSquare:
          return _squareX;
        default:
          throw new std::logic_error("not supported event type!");
      }
    }

  public:
    bool operator<(const SweepEvent &y) const
                   {
      return GetCompareValue() < y.GetCompareValue();
    }
  };

  std::vector<SweepEvent> _events;

  void FindRoundedPoints();
  void CreateEvents();
  void Hobby();

protected:
  int GetInitialScaleFactor() const
  {
    return 2;
  }

  void DetermineIntersectionsInternal();

public:
  explicit HobbyNaiveIntersectionAlgorithm(IntersectionAlgorithmData* data) :
      SimpleSweepIntersectionAlgorithm(data),
      _spacing(0)
  {
  }
};
}
