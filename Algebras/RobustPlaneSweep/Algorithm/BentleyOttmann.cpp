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

#include "../Helper/LineIntersection.h"
#include "BentleyOttmann.h"

using namespace std;

namespace RobustPlaneSweep
{
  void BentleyOttmann::SetNextStartEndEvent()
  {
    if (!_startEndEvents.empty()) {
      _nextHalfSegmentSweepEvent = _startEndEvents.front();
      _startEndEvents.pop();
    } else if (_nextHalfSegmentValid) {
      vector<SweepEvent*> list;

      bool isFirst = true;
      int x = numeric_limits<int>::min();

      bool isInputOrderedByX = GetData()->IsInputOrderedByX();

      while (_nextHalfSegmentValid) {
        const Point& px = _nextHalfSegment.GetDomPoint();

        if (isInputOrderedByX) {
#ifndef USE_RIGHTDOM_HALFSEGMENT
          if (!_nextHalfSegment.IsLeftDomPoint()) {
            _nextHalfSegmentValid =
              GetData()->FetchInputHalfSegment(
              _nextHalfSegment,
              _nextHalfSegmentBelongsToSecondGeometry);
            continue;
          }
#endif
          int internalX = GetTransformation()->TransformToInternalX(px);

          if (isFirst) {
            isFirst = false;
            x = internalX;
          } else if (internalX > x) {
            break;
          } else if (internalX < x) {
            throw new logic_error("invalid input order!");
          }
        }

        SweepEvent* newEvent = CreateEvent(
          _nextHalfSegment,
          _nextHalfSegmentBelongsToSecondGeometry);

        if (newEvent != NULL) {
          list.push_back(newEvent);
        }
        _nextHalfSegmentValid = GetData()->FetchInputHalfSegment(
          _nextHalfSegment,
          _nextHalfSegmentBelongsToSecondGeometry);
      }

      sort(list.begin(), list.end(), SweepEvent::SortComparer);

      for (vector<SweepEvent*>::const_iterator
        i = list.begin();
        i != list.end(); ++i) {
          _startEndEvents.push(*i);
      }

      _nextHalfSegmentSweepEvent = _startEndEvents.front();
      _startEndEvents.pop();
    } else {
      _nextHalfSegmentSweepEvent = NULL;
    }
  }

  SweepEvent* BentleyOttmann::CreateEvent(
    const HalfSegment& halfSegment,
    const bool belongsToSecondGeometry)
  {
#ifdef USE_RIGHTDOM_HALFSEGMENT
    InternalLineSegment *internalSegment;
    HalfSegmentIntersectionId id = GetData()->GetHalfSegmentId(halfSegment);

    unordered_map<int, InternalLineSegment*>::const_iterator findResult;
    findResult = _activeSegments.find(id);

    if (findResult == _activeSegments.end()) {
      internalSegment =
        new InternalLineSegment(*GetTransformation(), halfSegment);

      if (InternalPoint::IsEqual(
        internalSegment->GetLeft(),
        internalSegment->GetRight())) {
          delete internalSegment;
          return NULL;
      }
      _activeSegments[id] = internalSegment;
    } else {
      internalSegment = findResult->second;
    }

    if (halfSegment.IsLeftDomPoint()) {
      return SweepEvent::CreateStart(internalSegment);
    } else {
      return SweepEvent::CreateEnd(internalSegment);
    }
#else
    if (!halfSegment.IsLeftDomPoint()) {
      return NULL;
    }

    InternalLineSegment *internalSegment =
      new InternalLineSegment(
      *GetTransformation(),
      halfSegment,
      belongsToSecondGeometry,
      GetCalulcationType() == CalulationTypeRegion);

    if (InternalPoint::IsEqual(
      internalSegment->GetLeft(),
      internalSegment->GetRight())) {
        delete internalSegment;
        return NULL;
    }

    _intersectionEvents.push(SweepEvent::CreateEnd(internalSegment));

    return SweepEvent::CreateStart(internalSegment);
#endif
  }

  SweepEvent* BentleyOttmann::GetNextEvent()
  {
    int comparisionResult;
    if (_intersectionEvents.empty()) {
      comparisionResult = -1;
    } else if (_nextHalfSegmentSweepEvent == NULL) {
      comparisionResult = 1;
    } else {
      comparisionResult = SweepEvent::Compare(
        _nextHalfSegmentSweepEvent,
        _intersectionEvents.top());
    }

    SweepEvent* result;

    if (comparisionResult < 0) {
      result = _nextHalfSegmentSweepEvent;
      SetNextStartEndEvent();
    } else {
      result = _intersectionEvents.top();
      _intersectionEvents.pop();
    }

    return result;
  }

  void BentleyOttmann::FlushProcessedSegments()
  {
    vector<InternalResultLineSegment> lineSegmentsToFlush;
    InternalPointTransformation* transformation = GetTransformation();

    for (std::vector<InternalLineSegment*>::const_iterator
      i = _processedInternalSegments.begin();
      i != _processedInternalSegments.end(); ++i) {
        (*i)->BreakupLines(*transformation, lineSegmentsToFlush);
        delete (*i);
    }
    _processedInternalSegments.clear();

    int breakupUntil = GetBreakupUntil();
#ifdef USE_RIGHTDOM_HALFSEGMENT
    for (std::unordered_map<int, InternalLineSegment*>::const_iterator
      i = _activeSegments.begin();
      i != _activeSegments.end(); ++i) {
        i->second->BreakupLines(
          *transformation,
          lineSegmentsToFlush,
          true,
          breakupUntil);
    }
#else
    for (std::unordered_set<InternalLineSegment*>::const_iterator
      i = _activeSegments.begin();
      i != _activeSegments.end(); ++i) {
        (*i)->BreakupLines(
          *transformation,
          lineSegmentsToFlush,
          true,
          breakupUntil);
    }
#endif

    if (!lineSegmentsToFlush.empty()) {
      vector<InternalResultLineSegment>* nonOverlappingLineSegments =
        RemoveOverlappingSegments(lineSegmentsToFlush);

      for (vector<InternalResultLineSegment>::const_iterator
        i = nonOverlappingLineSegments->begin();
        i != nonOverlappingLineSegments->end(); ++i) {
          GetData()->OutputHalfSegment(
            i->GetRealLineSegment(
            GetTransformation()),
            i->GetInternalAttribute());
      }
      delete nonOverlappingLineSegments;
    }
  }

  void BentleyOttmann::DetermineIntersections()
  {
    if (GetTransformation() == NULL) {
      CreateTransformation();
    }

    GetData()->InitializeFetch();
    _nextHalfSegmentValid = GetData()->FetchInputHalfSegment(
      _nextHalfSegment,
      _nextHalfSegmentBelongsToSecondGeometry);
    _nextHalfSegmentSweepEvent = NULL;
    SetNextStartEndEvent();

    DetermineIntersectionsInternal();
    GetData()->OutputFinished();
  }

  void BentleyOttmann::DetermineIntersectionsInternal()
  {
    _state = new SweepState();

    vector<PossibleIntersectionPair> possibleIntersections;

    size_t addedIntersections = 0;
    SweepEvent* currentEvent;
    bool reportIntersections = GetData()->ReportIntersections();

    bool calculateRegionCoverage = false;
    if (GetCalulcationType() == CalulationTypeRegion) {
      calculateRegionCoverage = true;
    }

    _state->SetCurrentPoint(
      InternalIntersectionPoint(std::numeric_limits<int>::min(), 0));

    while ((currentEvent = GetNextEvent()) != NULL) {
      if (_state->GetCurrentPoint().GetX() !=
        currentEvent->GetPoint().GetX()) {
          if (OnXChanged(
            _state->GetCurrentPoint().GetX(),
            currentEvent->GetPoint().GetX())) {
              if ((addedIntersections + _processedInternalSegments.size()) >
                1000) {
                  FlushProcessedSegments();
                  addedIntersections = 0;
              }
          }
      }
      _state->SetCurrentPoint(currentEvent->GetPoint());

      BeforeProcessEvent(currentEvent);
      possibleIntersections.clear();
      if (currentEvent->GetEventType() == Start) {
#ifndef USE_RIGHTDOM_HALFSEGMENT
        _activeSegments.insert(currentEvent->GetSegment());
#endif
        _state->Add(
          currentEvent->GetSegment(),
          possibleIntersections,
          calculateRegionCoverage);
      } else if (currentEvent->GetEventType() == End) {
        _state->Remove(currentEvent->GetSegment(), possibleIntersections);
#ifdef USE_RIGHTDOM_HALFSEGMENT
        _activeSegments.erase(currentEvent->GetSegment()->GetAttr().edgeno);
#else
        _activeSegments.erase(currentEvent->GetSegment());
#endif
        _processedInternalSegments.push_back(currentEvent->GetSegment());
      } else if (currentEvent->GetEventType() == Intersection ||
        currentEvent->GetEventType() == TouchIntersection) {
          _intersections.erase(currentEvent->GetPoint());
          _state->Reorder(
            currentEvent,
            possibleIntersections,
            calculateRegionCoverage);
          ++_intersectionCount;
      } else {
        throw new logic_error("invalid eventtype!");
      }

      if (!possibleIntersections.empty()) {
        for (vector<PossibleIntersectionPair>::const_iterator
          possibleIntersection = possibleIntersections.begin();
          possibleIntersection != possibleIntersections.end();
        ++possibleIntersection) {
          InternalPoint s0s(0, 0), s0e(0, 0);
          InternalPoint s1s(0, 0), s1e(0, 0);
          PossibleIntersectionPairType pairType =
            possibleIntersection->GetType();

          if (pairType == SegmentNode) {
            s0s = possibleIntersection->GetSegment1()->GetLeft();
            s0e = possibleIntersection->GetSegment1()->GetRight();
            s1s = possibleIntersection->GetNode2()->GetMinLeft();
            s1e = possibleIntersection->GetNode2()->GetMaxRight();
          } else if (pairType == NodeNode) {
            s0s = possibleIntersection->GetNode1()->GetMinLeft();
            s0e = possibleIntersection->GetNode1()->GetMaxRight();
            s1s = possibleIntersection->GetNode2()->GetMinLeft();
            s1e = possibleIntersection->GetNode2()->GetMaxRight();
          } else if (pairType == SegmentSegment ||
            pairType == SegmentSegmentOverlapping) {
              s0s = possibleIntersection->GetSegment1()->GetLeft();
              s0e = possibleIntersection->GetSegment1()->GetRight();
              s1s = possibleIntersection->GetSegment2()->GetLeft();
              s1e = possibleIntersection->GetSegment2()->GetRight();
          } else {
            throw new logic_error("invalid pair type!");
          }

          InternalIntersectionPoint i0, i1;
          int c = LineIntersection::GetIntersections(
            s0s,
            s0e,
            s1s,
            s1e,
            false,
            i0,
            i1);

          if (c == 0) {
          } else if (c >= 1) {
            bool touchIntersection = false;

            if (calculateRegionCoverage && c == 1) {
              if (InternalIntersectionPoint::IsEqual(i0, s0s) ||
                InternalIntersectionPoint::IsEqual(i0, s0e) ||
                InternalIntersectionPoint::IsEqual(i0, s1s) ||
                InternalIntersectionPoint::IsEqual(i0, s1e)) {
                  touchIntersection = true;
              }
            }

            for (int ii = 0; ii < c; ++ii) {
              InternalIntersectionPoint ip = (ii == 0 ? i0 : i1);

              bool isCurrentPoint = false;
              bool isInFuture = false;

              if (ip.GetX() > currentEvent->GetPoint().GetX()) {
                isInFuture = true;
              } else if (ip.GetX() == currentEvent->GetPoint().GetX()) {
                if (ip.GetY() > currentEvent->GetPoint().GetY()) {
                  isInFuture = true;
                } else if (ip.GetY() == currentEvent->GetPoint().GetY()) {
                  isCurrentPoint = true;
                }
              }

              if (isInFuture || isCurrentPoint) {
                if (reportIntersections) {
                  GetData()->ReportIntersection(
                    GetTransformation()->TransformToPoint(ip),
                    c > 1);
                }
                ++addedIntersections;
                SweepEvent* intersectionEvent = NULL;
                if (pairType == SegmentSegment ||
                  pairType == SegmentSegmentOverlapping) {
                    bool overlappingSegments =
                      (pairType == SegmentSegmentOverlapping);
                    AddIntersection(
                      possibleIntersection->GetSegment1(),
                      overlappingSegments,
                      ip,
                      isCurrentPoint,
                      false,
                      touchIntersection,
                      intersectionEvent);
                    AddIntersection(
                      possibleIntersection->GetSegment2(),
                      overlappingSegments,
                      ip,
                      isCurrentPoint,
                      false,
                      touchIntersection,
                      intersectionEvent);
                } else if (pairType == SegmentNode) {
                  AddIntersection(
                    possibleIntersection->GetSegment1(),
                    false,
                    ip,
                    isCurrentPoint,
                    false,
                    touchIntersection,
                    intersectionEvent);
                  vector<InternalLineSegment*>* allSegments =
                    possibleIntersection->GetNode2()->GetAllSegments();
                  for (vector<InternalLineSegment*>::const_iterator
                    s1 = allSegments->begin();
                    s1 != allSegments->end(); ++s1) {
                      AddIntersection(
                        *s1,
                        false,
                        ip,
                        isCurrentPoint,
                        true,
                        touchIntersection,
                        intersectionEvent);
                  }
                  delete allSegments;
                } else if (pairType == NodeNode) {
                  vector<InternalLineSegment*>* allSegments1 =
                    possibleIntersection->GetNode1()->GetAllSegments();
                  for (vector<InternalLineSegment*>::const_iterator
                    s1 = allSegments1->begin();
                    s1 != allSegments1->end(); ++s1) {
                      AddIntersection(
                        *s1,
                        false,
                        ip,
                        isCurrentPoint,
                        true,
                        touchIntersection,
                        intersectionEvent);
                  }
                  delete allSegments1;

                  vector<InternalLineSegment*>* allSegments2 =
                    possibleIntersection->GetNode2()->GetAllSegments();
                  for (vector<InternalLineSegment*>::const_iterator
                    s2 = allSegments2->begin();
                    s2 != allSegments2->end(); ++s2) {
                      AddIntersection(
                        *s2,
                        false,
                        ip,
                        isCurrentPoint,
                        true,
                        touchIntersection,
                        intersectionEvent);
                  }
                  delete allSegments2;
                } else {
                  throw new logic_error("invalid pair type!");
                }
              }
            }
          }
        }
      }

      delete currentEvent;
    }

    Finished();

    if (!_activeSegments.empty()) {
      throw new logic_error("not all active segment processed!");
    }
    FlushProcessedSegments();
  }

  void BentleyOttmann::AddIntersection(
    InternalLineSegment* l,
    bool overlappingSegments,
    InternalIntersectionPoint ip,
    bool isCurrentPoint,
    bool checkIntersectionPoint,
    bool touchIntersection,
    SweepEvent*& intersectionEvent)
  {
    if (checkIntersectionPoint) {
      if (ip.GetX() < l->GetLeft().GetX() ||
        ip.GetX() > l->GetRight().GetX()) {
          return;
      }
      int ly = l->GetLeft().GetY();
      int ry = l->GetRight().GetY();

      if (ip.GetY() < (ly < ry ? ly : ry) ||
        ip.GetY() > (ly > ry ? ly : ry)) {
          return;
      }
    }

    if (!l->IsStartEndPoint(ip)) {
      l->AddIntersection(ip);

      if (!overlappingSegments && !isCurrentPoint) {
        if (intersectionEvent == NULL) {
          unordered_map<
            InternalIntersectionPoint,
            SweepEvent*,
            InternalIntersectionPointComparer,
            InternalIntersectionPointComparer>
            ::const_iterator findResult = _intersections.find(ip);

          if (findResult == _intersections.end()) {
            if (touchIntersection) {
              intersectionEvent = SweepEvent::CreateTouchIntersection(ip);
            } else {
              intersectionEvent = SweepEvent::CreateIntersection(ip);
            }
            _intersections[ip] = intersectionEvent;
            intersectionEvent->GetIntersectedSegments()->insert(l);
            _intersectionEvents.push(intersectionEvent);
          } else {
            intersectionEvent = findResult->second;
            intersectionEvent->GetIntersectedSegments()->insert(l);
          }
        } else {
          intersectionEvent->GetIntersectedSegments()->insert(l);
        }
      }
    }
  }

  PossibleIntersectionPair::PossibleIntersectionPair(
    SweepStateData* node1,
    SweepStateData* node2)
  {
    if (node1->IsSingleSegment()) {
      if (node2->IsSingleSegment()) {
        _type = SegmentSegment;
        _segment1 = node1->GetFirstSegment();
        _segment2 = node2->GetFirstSegment();
        _node1 = NULL;
        _node2 = NULL;
      } else {
        _type = SegmentNode;
        _segment1 = node1->GetFirstSegment();
        _segment2 = NULL;
        _node1 = NULL;
        _node2 = node2;
      }
    } else if (node2->IsSingleSegment()) {
      _type = SegmentNode;
      _segment1 = node2->GetFirstSegment();
      _segment2 = NULL;
      _node1 = NULL;
      _node2 = node1;
    } else {
      _type = NodeNode;
      _segment1 = NULL;
      _segment2 = NULL;
      _node1 = node1;
      _node2 = node2;
    }
  }

  PossibleIntersectionPair::PossibleIntersectionPair(
    InternalLineSegment* segment,
    SweepStateData* node)
  {
    if (node->IsSingleSegment()) {
      _type = SegmentSegment;
      _segment1 = segment;
      _segment2 = node->GetFirstSegment();
      _node1 = NULL;
      _node2 = NULL;
    } else {
      _type = SegmentNode;
      _segment1 = segment;
      _segment2 = NULL;
      _node1 = NULL;
      _node2 = node;
    }
  }

  PossibleIntersectionPair::PossibleIntersectionPair(
    InternalLineSegment* segment1,
    InternalLineSegment* segment2,
    PossibleIntersectionPairType type)
  {
    _type = type;
    _segment1 = segment1;
    _segment2 = segment2;
    _node1 = NULL;
    _node2 = NULL;
  }
}
