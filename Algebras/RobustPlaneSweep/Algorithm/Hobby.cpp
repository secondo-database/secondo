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


#include <algorithm>

#include "Hobby.h"

using namespace std;

namespace RobustPlaneSweep
{
  void Hobby::DetermineIntersectionsInternal()
  {
    _spacing = GetTransformation()->GetMinimalRoundedStep() / 2;

    BentleyOttmann::DetermineIntersectionsInternal();
  }

  AvlTreeNode<InternalLineSegment*, SweepStateData*>* 
    Hobby::GetStartNode(Rational searchY)
  {
    AvlTreeNode<InternalLineSegment*, SweepStateData*>* currentNode = 
      GetState()->GetTreeRoot();

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* lastNode = NULL;

    while (currentNode != NULL) {
      lastNode = currentNode;
      if (searchY < 
        currentNode->Value->GetYValueAt(GetState()->GetCurrentPoint())) {
          currentNode = currentNode->Left;
      } else {
        currentNode = currentNode->Right;
      }
    }

    return lastNode;
  }

  void Hobby::AddEvents()
  {
    for(unordered_set<InternalLineSegment*>::const_iterator 
      segment =_openSegments.begin();
      segment!=_openSegments.end();++segment) {
        _events.push_back (
          HobbyEvent(
          SegmentExit, 
          (*segment)->GetYValueAt(
          InternalIntersectionPoint(_currentHammockEnd, 0)), 
          *segment));
    }

    InternalIntersectionPoint hammockStart(_currentHammockStart, 0);
    InternalIntersectionPoint hammockEnd (_currentHammockEnd, 0);

    vector<int> squares;
    for(unordered_set<int>::const_iterator 
      i =_addedSquares.begin();
      i!=_addedSquares.end();++i) {
        squares.push_back(*i);
    }
    sort(squares.begin(),squares.end());

    SweepState* state=GetState();
    state->SetCurrentPoint( hammockEnd);
    state->SetAssumeYEqual(false);

    int spacing4 = _spacing * 4;

    for (unsigned int i = 0; i < squares.size(); ) {
      int topSquare = squares[i];
      int bottomSquare = topSquare;

      while (++i < squares.size() && squares[i] <= (bottomSquare + spacing4)) {
        bottomSquare = squares[i];
      }

      int topSquareEdge = topSquare - _spacing;
      int bottomSquareEdge = bottomSquare + _spacing;

      AvlTreeNode<InternalLineSegment*, SweepStateData*>* startNode=
        GetStartNode(Rational(topSquareEdge));

      for (int direction = 0; direction < 2; ++direction) {
        AvlTreeNode<InternalLineSegment*, SweepStateData*>* node = startNode;
        bool loop = true;
        while (node != NULL && loop) {
          vector<InternalLineSegment*>* segments=
            node->Value->GetAllSegments();

          for(vector<InternalLineSegment*>::const_iterator segment=
            segments->begin();
            segment!=segments->end();++segment) {
              if (_addedSegments.insert (*segment).second) {
                Rational yEnter = (*segment)->GetYValueAt(hammockStart);
                Rational yExit = (*segment)->GetYValueAt(hammockEnd);
                _events.push_back(HobbyEvent(SegmentEnter, yEnter, *segment));
                _events.push_back(HobbyEvent(SegmentExit, yExit, *segment));

                if(direction==0) {
                  if (yEnter < topSquareEdge && yExit < topSquareEdge) {
                    loop =false;
                  }
                } else {
                  if (yEnter > bottomSquareEdge && yExit > bottomSquareEdge) {
                    loop = false;
                  }
                }
              }
          }

          delete segments;

          node = (direction == 0 ? node->GetPrevious() : node->GetNext());
        }
      }
    }
  }


  void Hobby::ProcessEvents()
  {
    if (_events.empty()) {
      return;
    }

    AddEvents();

    sort(_events.begin(),_events.end());

    unordered_set<InternalLineSegment*>  activeLineSegments;

    bool inToleranceSquare = false;
    int currentSquareY = 0;
    for(vector<HobbyEvent>::const_iterator 
      e=_events.begin();
      e!=_events.end();++e) {
        if (e->GetEventType() == BeginTolaranceSquare) {
          if (inToleranceSquare) {
            throw new logic_error (
              "wrong event order! (still in tolerance square)");
          }
          inToleranceSquare = true;

          currentSquareY = GetTransformation()->
            RoundRational(e->GetY () + _spacing);

          for(unordered_set<InternalLineSegment*>::const_iterator 
            s=activeLineSegments.begin();
            s!=activeLineSegments.end();++s) {
              (*s)->AddIntersection(
                GetTransformation(), 
                InternalIntersectionPoint(_currentHammock,  currentSquareY));
          }
        } else if (e->GetEventType() == EndTolaranceSquare) {
          if (!inToleranceSquare) {
            throw new logic_error(
              "wrong event oder! (not in tolerance square)");
          }

          for(unordered_set<InternalLineSegment*>::const_iterator 
            s=activeLineSegments.begin();
            s!=activeLineSegments.end();++s) {
              (*s)->AddIntersection(
                GetTransformation(), 
                InternalIntersectionPoint(_currentHammock, currentSquareY));
          }

          inToleranceSquare = false;
        } else if (e->GetEventType() == SegmentBegin || 
          e->GetEventType() == SegmentEnd || 
          e->GetEventType() == SegmentEnter || 
          e->GetEventType() == SegmentExit) {

            if (activeLineSegments.find(e->GetSegment())!=
              activeLineSegments.end()) {
                activeLineSegments.erase(e->GetSegment());
            } else {
              activeLineSegments.insert(e->GetSegment());
            }

            if (inToleranceSquare) {
              e->GetSegment()->AddIntersection(
                GetTransformation(), 
                InternalIntersectionPoint(_currentHammock, currentSquareY));
            }

        } else {
          throw new logic_error("not supported event type!");
        }

    }

    _events.clear();
    _addedSegments.clear();
    _openSegments.clear();
    _addedSquares.clear();
  }
}
