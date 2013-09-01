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

[1] Implementation file for the class ~Hobby~

[TOC]

1 Overview

This file contains all methods required for the class ~Hobby~.

This class implements the (slightly modified) Hobby algorithm.

To use this class, derive a data class from ~IntersectionAlgorithmData~ and 
overwrite the necessary methods.

1 Includes

*/
#include <algorithm>

#include "Hobby.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~Hobby~

1.1 ~DetermineIntersectionsInternal~

*/
void Hobby::DetermineIntersectionsInternal()
{
  _spacing = GetTransformation()->GetMinimalRoundedStep() / 2;

  BentleyOttmann::DetermineIntersectionsInternal();
}

/*

1.1 ~GetStartNode~

*/
AvlTreeNode<InternalLineSegment*, SweepStateData*>*
Hobby::GetStartNode(const Rational& searchY) const
{
  AvlTreeNode<InternalLineSegment*, SweepStateData*>*
  currentNode = GetState()->GetTreeRoot();

  AvlTreeNode<InternalLineSegment*, SweepStateData*>* lastNode = NULL;

  while (currentNode != NULL) {
    lastNode = currentNode;
    if (searchY
        < currentNode->Value->GetYValueAt(GetState()->GetCurrentPoint())) {
      currentNode = currentNode->Left;
    } else {
      currentNode = currentNode->Right;
    }
  }

  return lastNode;
}

/*

1.1 ~AddSegmentEvent~

*/
bool Hobby::AddSegmentEvent(int topSquareEdge,
                            int bottomSquareEdge,
                            int direction,
                            InternalLineSegment* segment)
{
  if (_addedSegments.insert(segment).second) {
    Rational yEnter = (segment)->GetYValueAt(_currentHammockStart);
    Rational yExit = (segment)->GetYValueAt(_currentHammockEnd);
    _events.push_back(HobbyEvent(HobbyEventType::SegmentEnter,
                                 yEnter,
                                 segment));

    _events.push_back(HobbyEvent(HobbyEventType::SegmentExit, yExit, segment));

    if (direction == 0) {
      if (yEnter < topSquareEdge && yExit < topSquareEdge) {
        return true;
      }
    } else {
      if (yEnter > bottomSquareEdge && yExit > bottomSquareEdge) {
        return true;
      }
    }
  }

  return false;
}

/*

1.1 ~AddEvents~

*/
void Hobby::AddEvents()
{
  for (unordered_set<InternalLineSegment*>::const_iterator segment =
      _openSegments.begin(); segment != _openSegments.end(); ++segment) {

    _events.push_back(HobbyEvent(HobbyEventType::SegmentExit,
                                 (*segment)->GetYValueAt(_currentHammockEnd),
                                 *segment));
  }

  InternalIntersectionPoint hammockStart(_currentHammockStart, 0);
  InternalIntersectionPoint hammockEnd(_currentHammockEnd, 0);

  vector<int> squares;
  for (unordered_set<int>::const_iterator i = _addedSquares.begin();
      i != _addedSquares.end(); ++i) {
    squares.push_back(*i);
  }

  sort(squares.begin(), squares.end());

  SweepState* state = GetState();
  state->SetCurrentPoint(hammockEnd);
  state->SetAssumeYEqual(false);

  int spacing4 = _spacing * 4;

  for (unsigned int i = 0; i < squares.size();) {
    int topSquare = squares[i];
    int bottomSquare = topSquare;

    while (++i < squares.size() && squares[i] <= (bottomSquare + spacing4)) {
      bottomSquare = squares[i];
    }

    int topSquareEdge = topSquare - _spacing;
    int bottomSquareEdge = bottomSquare + _spacing;

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* startNode =
        GetStartNode(Rational(topSquareEdge));

    for (int direction = 0; direction < 2; ++direction) {
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* node = startNode;
      bool loop = true;
      while (node != NULL && loop) {
        if (node->Value->IsSingleSegment()) {
          loop = !AddSegmentEvent(topSquareEdge,
                                  bottomSquareEdge,
                                  direction,
                                  node->Value->GetFirstSegment());
        } else {
          vector<InternalLineSegment*>* segments =
              node->Value->GetAllSegments();

          for (vector<InternalLineSegment*>::const_iterator segment =
              segments->begin();
              segment != segments->end(); ++segment) {
            loop = !AddSegmentEvent(topSquareEdge,
                                    bottomSquareEdge,
                                    direction,
                                    *segment);
          }

          delete segments;
        }

        node = (direction == 0 ? node->GetPrevious() : node->GetNext());
      }
    }
  }
}

/*

1.1 ~ProcessEvents~

*/
void Hobby::ProcessEvents()
{
  if (_events.empty()) {
    return;
  }

  AddEvents();

  sort(_events.begin(), _events.end());

  unordered_set<InternalLineSegment*> activeLineSegments;

  bool inToleranceSquare = false;
  int currentSquareY = 0;
  for (vector<HobbyEvent>::const_iterator
  e = _events.begin();
      e != _events.end(); ++e) {
    if (e->GetEventType() == HobbyEventType::BeginTolaranceSquare) {
      if (inToleranceSquare) {
        throw new logic_error("wrong event order! (still in tolerance square)");
      }
      inToleranceSquare = true;

      currentSquareY = GetTransformation()->RoundRational(e->GetY() + _spacing);

      for (unordered_set<InternalLineSegment*>::const_iterator s =
          activeLineSegments.begin(); s != activeLineSegments.end(); ++s) {
        if (_ignoredIntersections.find(std::make_pair(currentSquareY, *s))
            == _ignoredIntersections.end()) {
          (*s)->AddHobbyIntersection(_currentHammock,
                                     currentSquareY,
                                     _spacing);
        }
      }
    } else if (e->GetEventType() == HobbyEventType::EndTolaranceSquare) {
      if (!inToleranceSquare) {
        throw new logic_error("wrong event oder! (not in tolerance square)");
      }
      /*
      for (unordered_set<InternalLineSegment*>::const_iterator s =
          activeLineSegments.begin(); s != activeLineSegments.end(); ++s) {
        if (_ignoredIntersections.find(std::make_pair(currentSquareY, *s))
            == _ignoredIntersections.end()) {
          (*s)->AddHobbyIntersection(_currentHammock,
                                     currentSquareY,
                                     _spacing);
        }
      }
      */
      inToleranceSquare = false;
    } else if (e->GetEventType() == HobbyEventType::SegmentStart
               || e->GetEventType() == HobbyEventType::SegmentEnd
               || e->GetEventType() == HobbyEventType::SegmentEnter
               || e->GetEventType() == HobbyEventType::SegmentExit) {
      if (activeLineSegments.find(e->GetSegment()) !=
          activeLineSegments.end()) {
        activeLineSegments.erase(e->GetSegment());
      } else {
        activeLineSegments.insert(e->GetSegment());
      }

      if (inToleranceSquare) {
        if (_ignoredIntersections.find(std::make_pair(currentSquareY,
                                                      e->GetSegment()))
            == _ignoredIntersections.end()) {
          e->GetSegment()->AddHobbyIntersection(_currentHammock,
                                                currentSquareY,
                                                _spacing);
        }
      }
    } else {
      throw new logic_error("not supported event type!");
    }
  }

  _events.clear();
  _addedSegments.clear();
  _openSegments.clear();
  _addedSquares.clear();
  _ignoredIntersections.clear();
}
}
