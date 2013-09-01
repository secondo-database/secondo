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

[1] Header File for the class ~HobbyNaiveIntersectionAlgorithm~

[TOC]

1 Overview

This header file contains all structs and classes required for the
class ~HobbyNaiveIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

This class is a naive Hobby implementation. 
It uses the ~SimpleSweepIntersectionAlgorithm~ to find all intersections.
Then tolerance square edges are constructed and a second sweep is performed.

1 Includes

*/

#pragma once

#include <unordered_set>
#include <vector>
#include "SimpleSweepIntersectionAlgorithm.h"

namespace RobustPlaneSweep
{
/*

1 Class ~HobbyNaiveIntersectionAlgorithm~

*/
class HobbyNaiveIntersectionAlgorithm :
    public SimpleSweepIntersectionAlgorithm
{
private:
  std::unordered_set<
      InternalPoint,
      InternalPointComparer,
      InternalPointComparer> _roundedPoints;

  int _spacing;

/*

1.1 Private enum class ~EventType~

*/
  enum class EventType
  {
    Undefined,
    BeginSegment,
    EndSegment,
    TolaranceSquare
  };

/*

1.1 Private class ~SweepEvent~

*/
  class SweepEvent
  {
  private:
    int _spacing;
    InternalLineSegment* _segment;
    EventType _eventType;
    int _squareX;
    int _squareY;

  public:
/*

1.1.1 Constructors

*/
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

/*

1.1.1 ~GetEventType~

*/
    EventType GetEventType() const
    {
      return _eventType;
    }

/*

1.1.1 ~GetSegment~

*/
    InternalLineSegment* GetSegment() const
    {
      return _segment;
    }

/*

1.1.1 ~GetSquareX~

*/
    int GetSquareX() const
    {
      return _squareX;
    }

/*

1.1.1 ~GetSquareY~

*/
    int GetSquareY() const
    {
      return _squareY;
    }

  private:
/*

1.1.1 ~GetCompareValue~

*/
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
/*

1.1.1 ~operator\textless~

*/
    bool operator<(const SweepEvent &y) const
    {
      return GetCompareValue() < y.GetCompareValue();
    }
  };

/*

1.1 Member variables

*/
  std::vector<SweepEvent> _events;

/*

1.1 Methods

1.1.1 ~FindRoundedPoints~

*/
  void FindRoundedPoints();
/*

1.1.1 ~CreateEvents~

*/
  void CreateEvents();
/*

1.1.1 ~Hobby~

*/
  void Hobby();

protected:
/*

1.1.1 ~GetInitialScaleFactor~

*/
  int GetInitialScaleFactor() const
  {
    return 2;
  }

/*

1.1.1 ~DetermineIntersectionsInternal~

*/
  void DetermineIntersectionsInternal();

public:
/*

1.1.1 Constructor

*/
  explicit HobbyNaiveIntersectionAlgorithm(IntersectionAlgorithmData* data) :
      SimpleSweepIntersectionAlgorithm(data),
      _spacing(0)
  {
  }
};
}
