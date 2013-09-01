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

[1] Implementation file for the class ~HobbyNaiveIntersectionAlgorithm~

[TOC]

1 Overview

This file contains all methods required for the 
class ~HobbyNaiveIntersectionAlgorithm~.

This file is not required for SECONDO. It is only used inside the test project.

This class is a naive Hobby implementation. 
It uses the ~SimpleSweepIntersectionAlgorithm~ to find all intersections.
Then tolerance square edges are constructed and a second sweep is performed.

1 Includes

*/
#include <algorithm>

#include "../Helper/LineIntersection.h"
#include "HobbyNaiveIntersectionAlgorithm.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~HobbyNaiveIntersectionAlgorithm~

1.1 ~DetermineIntersectionsInternal~

*/
void HobbyNaiveIntersectionAlgorithm::DetermineIntersectionsInternal()
{
  _spacing = GetTransformation()->GetMinimalRoundedStep() / 2;

  SimpleSweepIntersectionAlgorithm::DetermineIntersectionsInternal();

  Hobby();
}

/*

1.1 ~FindRoundedPoints~

*/
void HobbyNaiveIntersectionAlgorithm::FindRoundedPoints()
{
  const InternalPointTransformation* transformation = GetTransformation();

  std::vector<InternalLineSegment*>::const_iterator begin = GetInputBegin();
  std::vector<InternalLineSegment*>::const_iterator end = GetInputEnd();

  for (std::vector<InternalLineSegment*>::const_iterator
  i = begin; i != end; ++i) {
    InternalLineSegment* segment = *i;

    InternalPoint roundedLeft =
        transformation->RoundPoint(segment->GetLeft());

    InternalPoint roundedRight =
        transformation->RoundPoint(segment->GetRight());

    _roundedPoints.insert(roundedLeft);
    _roundedPoints.insert(roundedRight);

    std::map<
        InternalIntersectionPoint,
        InternalAttribute,
        InternalIntersectionPointLess>*
    intersections = segment->GetIntersections();

    if (intersections != NULL) {
      for (std::map<
          InternalIntersectionPoint,
          InternalAttribute,
          InternalIntersectionPointLess>::const_iterator intersection =
          intersections->begin(); intersection != intersections->end();
          ++intersection) {

        InternalPoint roundedIntersection =
            transformation->RoundPoint(intersection->first);

        _roundedPoints.insert(roundedIntersection);
      }
    }
  }
}

/*

1.1 ~CreateEvents~

*/
void HobbyNaiveIntersectionAlgorithm::CreateEvents()
{
  _events.reserve(_roundedPoints.size() + GetInputSize() * 2);

  for (unordered_set<
      InternalPoint,
      InternalPointComparer,
      InternalPointComparer>::const_iterator i = _roundedPoints.begin();
      i != _roundedPoints.end(); ++i) {
    _events.push_back(SweepEvent(i->GetX(), i->GetY()));
  }

  int segmentSpacing = _spacing + 1;

  std::vector<InternalLineSegment*>::const_iterator begin = GetInputBegin();
  std::vector<InternalLineSegment*>::const_iterator end = GetInputEnd();

  for (std::vector<InternalLineSegment*>::const_iterator i = begin;
      i != end; ++i) {
    InternalLineSegment* segment = *i;
    _events.push_back(SweepEvent(segment, segmentSpacing, true));
    _events.push_back(SweepEvent(segment, segmentSpacing, false));
  }

  sort(_events.begin(), _events.end());
}

/*

1.1 ~Hobby~

*/
void HobbyNaiveIntersectionAlgorithm::Hobby()
{
  FindRoundedPoints();
  CreateEvents();

  unordered_set<InternalLineSegment*> possibleSegments;
  for (vector<SweepEvent>::const_iterator e = _events.begin();
      e != _events.end(); ++e) {
    if (e->GetEventType() == EventType::TolaranceSquare) {
      int ppx0 = e->GetSquareX() - _spacing;
      int ppx1 = e->GetSquareX() + _spacing;
      int ppy0 = e->GetSquareY() - _spacing;
      int ppy1 = e->GetSquareY() + _spacing;

      InternalPoint tolPoints[4] = {
          InternalPoint(ppx0, ppy0),
          InternalPoint(ppx0, ppy1),
          InternalPoint(ppx1, ppy0),
          InternalPoint(ppx1, ppy1)
      };

      for (unordered_set<InternalLineSegment*>::const_iterator segment =
          possibleSegments.begin();
          segment != possibleSegments.end();
          ++segment) {
        int count = 0;
        bool first = true;
        Rational minX(0);
        Rational minY(0);

        for (int i = 0; i < 4; ++i) {
          InternalIntersectionPoint i0, i1;
          int c =
              LineIntersection::GetIntersections((*segment)->GetLeft(),
                                                 (*segment)->GetRight(),
                                                 tolPoints[i > 0 ? i - 1 : 0],
                                                 tolPoints[i < 3 ? i + 1 : 3],
                                                 true,
                                                 true,
                                                 i0,
                                                 i1);
          if (c >= 1) {
            minX = (first ? i0.GetX() : Rational::Min(minX, i0.GetX()));
            minY = (first ? i0.GetY() : Rational::Min(minY, i0.GetY()));
            first = false;
            if (c == 2) {
              minX = Rational::Min(minX, i1.GetX());
              minY = Rational::Min(minY, i1.GetY());
            }
          }
          count += c;
        }

        if (count >= 1) {
          if (minX == ppx1 || minY == ppy1) {
            continue;
          }

          if (!InternalPoint::
              IsEqual(GetTransformation()->RoundPoint((*segment)->GetLeft()),
                      InternalPoint(e->GetSquareX(),
                                    e->GetSquareY()))
              && !InternalPoint::
              IsEqual(GetTransformation()->RoundPoint((*segment)->GetRight()),
                      InternalPoint(e->GetSquareX(), e->GetSquareY()))) {
            (*segment)->AddHobbyIntersection(e->GetSquareX(),
                                             e->GetSquareY(),
                                             _spacing);
          }
        }
      }
    } else if (e->GetEventType() == EventType::BeginSegment) {
      possibleSegments.insert(e->GetSegment());
    } else if (e->GetEventType() == EventType::EndSegment) {
      possibleSegments.erase(e->GetSegment());
    } else {
      throw new std::logic_error("not supported event type!");
    }
  }
}
}
