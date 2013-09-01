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

[1] Implementation file for the class ~BentleyOttmann~

[TOC]

1 Overview

This file contains all structs and classes required for the 
class ~BentleyOttmann~.

This class implements the Bentley-Ottmann-Algorithm.

To use this class, derive a data class from ~IntersectionAlgorithmData~ and 
overwrite the necessary methods.

1 Includes

*/
#include <algorithm>

#include "../Helper/LineIntersection.h"
#include "BentleyOttmann.h"

using namespace std;

namespace RobustPlaneSweep
{
/*

1 Class ~BentleyOttmann~

1.1 ~FetchInput~

*/
void BentleyOttmann::FetchInput()
{
  for (;;) {
    _nextDataValid =
        GetData()->FetchInput(_nextDataHalfSegment,
                              _nextDataPoint,
                              _nextDataBelongsToSecondGeometry);

    // if halfsegments with a right dominating point are used,
    // all data is valid, otherwise only points and halfsegments with
    // a left dominating point are valid.
#ifdef USE_RIGHTDOM_HALFSEGMENT
    break;
#else
    if (!_nextDataValid) {
      break;
    }

    if (_nextDataPoint.IsDefined()) {
      break;
    }

    if (_nextDataHalfSegment.IsLeftDomPoint()) {
      break;
    }
#endif
  }
}

/*

1.1 ~SetNextStartEndEvent~

*/
void BentleyOttmann::SetNextStartEndEvent()
{
  if (!_startEndEvents.empty()) {
    _nextHalfSegmentSweepEvent = _startEndEvents.front();
    _startEndEvents.pop();
  } else if (_nextDataValid) {
    vector<SweepEvent*> list;

    bool isFirst = true;
    int maxX = numeric_limits<int>::min();

    bool isInputOrderedByX = GetData()->IsInputOrderedByX();

    int almostEqualSortMargin = GetTransformation()->GetAlmostEqualSortMargin();

    while (_nextDataValid) {
      if (isInputOrderedByX) {
        int internalX;
        if (_nextDataPoint.IsDefined()) {
          internalX = GetTransformation()->TransformToInternalX(_nextDataPoint);
        } else {
          const Point& px = _nextDataHalfSegment.GetDomPoint();
          internalX = GetTransformation()->TransformToInternalX(px);
        }

        if (internalX <= _lastFetchedX) {
          throw new logic_error("invalid input order!");
        }

        if (isFirst) {
          maxX = internalX;
          isFirst = false;
        } else {
          if (internalX > (maxX + almostEqualSortMargin)) {
            _lastFetchedX = maxX;
            break;
          }

          if (internalX > maxX) {
            maxX = internalX;
          }
        }
      }

      SweepEvent* newEvent = CreateEvent(_nextDataHalfSegment,
                                         _nextDataPoint,
                                         _nextDataBelongsToSecondGeometry);

      if (newEvent != NULL) {
        list.push_back(newEvent);
      }

      FetchInput();
    }

    sort(list.begin(), list.end(), SweepEvent::SortComparer);

    for (vector<SweepEvent*>::const_iterator i = list.begin();
        i != list.end(); ++i) {
      _startEndEvents.push(*i);
    }

    _nextHalfSegmentSweepEvent = _startEndEvents.front();
    _startEndEvents.pop();
  } else {
    _nextHalfSegmentSweepEvent = NULL;
  }
}

/*

1.1 ~CreateEvent~

*/
SweepEvent* BentleyOttmann::CreateEvent(const HalfSegment& halfSegment,
                                        const Point& point,
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

    if (InternalPoint::IsEqual(internalSegment->GetLeft(),
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
  if (point.IsDefined()) {
    InternalIntersectionPoint
    transformedPoint(GetTransformation()->TransformToInternalPoint(point));

    return SweepEvent::CreatePoint(transformedPoint, belongsToSecondGeometry);
  }

  if (!halfSegment.IsLeftDomPoint()) {
    return NULL;
  }

  bool isRegion;
  if (belongsToSecondGeometry) {
    isRegion = SecondGeometryIsRegion();
  } else {
    isRegion = FirstGeometryIsRegion();
  }

  InternalLineSegment *internalSegment =
      new InternalLineSegment(*GetTransformation(),
                              halfSegment,
                              belongsToSecondGeometry,
                              isRegion);

  if (InternalPoint::IsEqual(internalSegment->GetLeft(),
                             internalSegment->GetRight())) {
    delete internalSegment;
    return NULL;
  }

  _intersectionEvents.push(SweepEvent::CreateEnd(internalSegment));

  return SweepEvent::CreateStart(internalSegment);
#endif
}

/*

1.1 ~GetNextEvent~

*/
SweepEvent* BentleyOttmann::GetNextEvent()
{
  int comparisionResult;
  if (_intersectionEvents.empty()) {
    comparisionResult = -1;
  } else if (_nextHalfSegmentSweepEvent == NULL) {
    comparisionResult = 1;
  } else {
    comparisionResult = SweepEvent::Compare(_nextHalfSegmentSweepEvent,
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

/*

1.1 ~DeleteProcessedSegments~

*/
void BentleyOttmann::DeleteProcessedSegments()
{
  for (vector<InternalLineSegment*>::const_iterator i =
      _processedInternalSegments.begin();
      i != _processedInternalSegments.end();
      ++i) {
    delete (*i);
  }

  _processedInternalSegments.clear();
}

/*

1.1 ~FlushProcessedSegments~

*/
void BentleyOttmann::FlushProcessedSegments()
{
  vector<InternalResultLineSegment> lineSegmentsToFlush;
  InternalPointTransformation* transformation = GetTransformation();

  for (vector<InternalLineSegment*>::const_iterator
  i = _processedInternalSegments.begin();
      i != _processedInternalSegments.end(); ++i) {
    (*i)->BreakupLines(*transformation, lineSegmentsToFlush);
    delete (*i);
  }

  _processedInternalSegments.clear();

  int breakupUntil = GetBreakupUntil();
#ifdef USE_RIGHTDOM_HALFSEGMENT
  for (unordered_map<int, InternalLineSegment*>::const_iterator
      i = _activeSegments.begin();
      i != _activeSegments.end(); ++i) {
    i->second->BreakupLines(*transformation,
        lineSegmentsToFlush,
        true,
        breakupUntil);
  }
#else
  for (unordered_set<InternalLineSegment*>::const_iterator
  i = _activeSegments.begin();
      i != _activeSegments.end(); ++i) {
    (*i)->BreakupLines(*transformation,
                       lineSegmentsToFlush,
                       true,
                       breakupUntil);
  }
#endif

  if (!lineSegmentsToFlush.empty()) {
    vector<InternalResultLineSegment>* nonOverlappingLineSegments =
        RemoveOverlappingSegments(lineSegmentsToFlush);

    for (vector<InternalResultLineSegment>::const_iterator i =
        nonOverlappingLineSegments->begin();
        i != nonOverlappingLineSegments->end(); ++i) {
      GetData()->OutputHalfSegment(i->GetHalfSegment(GetTransformation()),
                                   i->GetInternalAttribute());
    }

    delete nonOverlappingLineSegments;
  }
}

/*

1.1 ~PointIntersectsWithAvlTree~

*/
bool BentleyOttmann::PointIntersectsWithAvlTree(Rational searchY) const
{
  AvlTreeNode<InternalLineSegment*, SweepStateData*>* currentNode =
      GetState()->GetTreeRoot();

  AvlTreeNode<InternalLineSegment*, SweepStateData*>* lastNode = NULL;
  Rational lastY(0);

  while (currentNode != NULL) {
    lastNode = currentNode;
    lastY = currentNode->Value->GetYValueAt(GetState()->GetCurrentPoint());

    if (searchY < lastY) {
      currentNode = currentNode->Left;
    } else if (searchY > lastY) {
      currentNode = currentNode->Right;
    } else {
      return true;
    }
  }

  if (lastNode != NULL) {
    const InternalAttribute attribute =
        lastNode->Value->GetAttributeAt(GetState()->GetCurrentPoint(), true);

    if (lastY > searchY) {
      return attribute.IsFirstBelow();
    } else {
      return attribute.IsFirstAbove();
    }
  } else {
    return false;
  }
}

/*

1.1 ~DetermineIntersections~

*/
void BentleyOttmann::DetermineIntersections()
{
  if (GetTransformation() == NULL) {
    CreateTransformation();
  }

  GetData()->InitializeFetch();
  _lastFetchedX = numeric_limits<int>::min();
  FetchInput();
  _nextHalfSegmentSweepEvent = NULL;
  SetNextStartEndEvent();

  DetermineIntersectionsInternal();
  GetData()->OutputFinished();
}

/*

1.1 ~DetermineIntersectionsInternal~

*/
void BentleyOttmann::DetermineIntersectionsInternal()
{
  _state = new SweepState();

  vector<PossibleIntersectionPair> possibleIntersections;

  size_t addedIntersections = 0;
  SweepEvent* currentEvent;
  bool reportIntersections = GetData()->ReportIntersections();
  bool outputData = GetData()->OutputData();

  bool calculateRegionCoverage = false;
  if (FirstGeometryIsRegion() || SecondGeometryIsRegion()) {
    calculateRegionCoverage = true;
  }

  bool intersects = false;

  bool hasFirstGeometryEvent = false;
  bool hasSecondGeometryEvent = false;
  bool reportedIntersection = false;

  _state->SetCurrentPoint(InternalIntersectionPoint(numeric_limits<int>::min(),
                                                    0));

  while ((currentEvent = GetNextEvent()) != NULL) {
    if (_state->GetCurrentPoint().GetX() !=
        currentEvent->GetPoint().GetX()) {
      hasFirstGeometryEvent = false;
      hasSecondGeometryEvent = false;
      reportedIntersection = false;

      if (OnXChanged(_state->GetCurrentPoint().GetX(),
                     currentEvent->GetPoint().GetX())) {
        if ((addedIntersections + _processedInternalSegments.size()) > 10000) {
          if (outputData) {
            FlushProcessedSegments();
          } else {
            DeleteProcessedSegments();
          }

          addedIntersections = 0;
        }
      }
    } else if (_state->GetCurrentPoint().GetY() !=
               currentEvent->GetPoint().GetY()) {
      hasFirstGeometryEvent = false;
      hasSecondGeometryEvent = false;
      reportedIntersection = false;
    }

    _state->SetCurrentPoint(currentEvent->GetPoint());

    BeforeProcessEvent(currentEvent);
    possibleIntersections.clear();
    if (currentEvent->GetEventType() == SweepEventType::Start) {
#ifndef USE_RIGHTDOM_HALFSEGMENT
      _activeSegments.insert(currentEvent->GetSegment());
#endif
      _state->Add(currentEvent->GetSegment(),
                  possibleIntersections,
                  calculateRegionCoverage);
    } else if (currentEvent->GetEventType() == SweepEventType::End) {
      _state->Remove(currentEvent->GetSegment(), possibleIntersections);
#ifdef USE_RIGHTDOM_HALFSEGMENT
      _activeSegments.erase(currentEvent->GetSegment()->GetAttr().edgeno);
#else
      _activeSegments.erase(currentEvent->GetSegment());
#endif
      _processedInternalSegments.push_back(currentEvent->GetSegment());
    } else if (currentEvent->GetEventType() == SweepEventType::Intersection) {
      _intersections.erase(currentEvent->GetPoint());
      _state->Reorder(currentEvent,
                      possibleIntersections,
                      calculateRegionCoverage);
      ++_intersectionCount;
    } else if (currentEvent->GetEventType() == SweepEventType::Point) {
      bool pointsIntersects = hasSecondGeometryEvent
                              || hasFirstGeometryEvent
                              || PointIntersectsWithAvlTree(currentEvent
                                  ->GetPoint().GetY());

      if (pointsIntersects && !intersects) {
        intersects = true;
        if (GetData()->OnGeometryIntersectionFound()) {
          delete currentEvent;
          return;
        }
      }

      if (outputData) {
        Point point =
            GetTransformation()->TransformToPoint(currentEvent->GetPoint());

        BoundaryType insideOutside;
        if (pointsIntersects) {
          insideOutside = BoundaryType::InsideInterior;
        } else {
          insideOutside = BoundaryType::OutsideInterior;
        }

        if (currentEvent->BelongsToSecondGeometry()) {
          GetData()->OutputPoint(point,
                                 InternalAttribute(insideOutside,
                                                   BoundaryType::Boundary));
        } else {
          GetData()->OutputPoint(point,
                                 InternalAttribute(BoundaryType::Boundary,
                                                   insideOutside));
        }
      }
    } else {
      throw new logic_error("invalid eventtype!");
    }

    if (currentEvent->GetEventType() == SweepEventType::Start
        || currentEvent->GetEventType() == SweepEventType::End) {
      bool segmentInsideOtherRegion = false;
      InternalAttribute segmentAttribute =
          currentEvent->GetSegment()->GetInitialAttribute();

      if (segmentAttribute.IsBoundaryInSecond()) {
        hasSecondGeometryEvent = true;
        segmentInsideOtherRegion =
            (segmentAttribute.GetFirst() == BoundaryType::InsideInterior);
      } else {
        hasFirstGeometryEvent = true;
        segmentInsideOtherRegion =
            (segmentAttribute.GetSecond() == BoundaryType::InsideInterior);
      }

      if (!intersects) {
        if (segmentInsideOtherRegion
            || (hasFirstGeometryEvent && hasSecondGeometryEvent)) {
          intersects = true;
          if (GetData()->OnGeometryIntersectionFound()) {
            delete currentEvent;
            return;
          }
        }
      }

      if (reportIntersections 
          && hasFirstGeometryEvent 
          && hasSecondGeometryEvent 
          && !reportedIntersection) {
        GetData()->ReportIntersection(GetTransformation()->TransformToPoint(
                                      currentEvent->GetPoint()),
                                      false);
        reportedIntersection = true;
      }

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
        bool s0belongsToSecond;
        bool s1belongsToSecond;

        if (pairType == PossibleIntersectionPairType::SegmentNode) {
          s0s = possibleIntersection->GetSegment1()->GetLeft();
          s0e = possibleIntersection->GetSegment1()->GetRight();
          s1s = possibleIntersection->GetNode2()->GetMinLeft();
          s1e = possibleIntersection->GetNode2()->GetMaxRight();
          s0belongsToSecond = possibleIntersection->GetSegment1()
              ->GetInitialAttribute().IsBoundaryInSecond();

          s1belongsToSecond =
              possibleIntersection->GetNode2()->GetFirstSegment()
                  ->GetInitialAttribute().IsBoundaryInSecond();
        } else if (pairType == PossibleIntersectionPairType::NodeNode) {
          s0s = possibleIntersection->GetNode1()->GetMinLeft();
          s0e = possibleIntersection->GetNode1()->GetMaxRight();
          s1s = possibleIntersection->GetNode2()->GetMinLeft();
          s1e = possibleIntersection->GetNode2()->GetMaxRight();

          s0belongsToSecond =
              possibleIntersection->GetNode1()->GetFirstSegment()
                  ->GetInitialAttribute().IsBoundaryInSecond();

          s1belongsToSecond =
              possibleIntersection->GetNode2()->GetFirstSegment()
                  ->GetInitialAttribute().IsBoundaryInSecond();
        } else if (pairType == PossibleIntersectionPairType::SegmentSegment
                   || pairType ==
                      PossibleIntersectionPairType::SegmentSegmentOverlapping) {
          s0s = possibleIntersection->GetSegment1()->GetLeft();
          s0e = possibleIntersection->GetSegment1()->GetRight();
          s1s = possibleIntersection->GetSegment2()->GetLeft();
          s1e = possibleIntersection->GetSegment2()->GetRight();

          s0belongsToSecond =
              possibleIntersection->GetSegment1()
                  ->GetInitialAttribute().IsBoundaryInSecond();

          s1belongsToSecond =
              possibleIntersection->GetSegment2()
                  ->GetInitialAttribute().IsBoundaryInSecond();
        } else {
          throw new logic_error("invalid pair type!");
        }

        InternalIntersectionPoint i0, i1;
        int c = LineIntersection::GetIntersections(s0s,
                                                   s0e,
                                                   s1s,
                                                   s1e,
                                                   false,
                                                   true,
                                                   i0,
                                                   i1);

        if (c >= 1) {
          if (s0belongsToSecond != s1belongsToSecond && !intersects) {
            intersects = true;
            if (GetData()->OnGeometryIntersectionFound()) {
              delete currentEvent;
              return;
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
                GetData()->ReportIntersection(GetTransformation()
                                                  ->TransformToPoint(ip),
                                              c > 1);
              }
              ++addedIntersections;
              SweepEvent* intersectionEvent = NULL;
              if (pairType == PossibleIntersectionPairType::SegmentSegment
                  || pairType ==
                     PossibleIntersectionPairType::SegmentSegmentOverlapping) {
                bool overlappingSegments =
                    (pairType ==
                     PossibleIntersectionPairType::SegmentSegmentOverlapping);
                AddIntersection(possibleIntersection->GetSegment1(),
                                overlappingSegments,
                                calculateRegionCoverage,
                                ip,
                                isCurrentPoint,
                                false,
                                intersectionEvent);
                AddIntersection(possibleIntersection->GetSegment2(),
                                overlappingSegments,
                                calculateRegionCoverage,
                                ip,
                                isCurrentPoint,
                                false,
                                intersectionEvent);
              } else if (pairType
                         == PossibleIntersectionPairType::SegmentNode) {
                AddIntersection(possibleIntersection->GetSegment1(),
                                false,
                                calculateRegionCoverage,
                                ip,
                                isCurrentPoint,
                                false,
                                intersectionEvent);
                vector<InternalLineSegment*>* allSegments =
                    possibleIntersection->GetNode2()->GetAllSegments();

                for (vector<InternalLineSegment*>::const_iterator s1 =
                    allSegments->begin(); s1 != allSegments->end(); ++s1) {
                  AddIntersection(*s1,
                                  false,
                                  calculateRegionCoverage,
                                  ip,
                                  isCurrentPoint,
                                  true,
                                  intersectionEvent);
                }
                delete allSegments;
              } else if (pairType == PossibleIntersectionPairType::NodeNode) {
                vector<InternalLineSegment*>* allSegments1 =
                    possibleIntersection->GetNode1()->GetAllSegments();

                for (vector<InternalLineSegment*>::const_iterator s1 =
                    allSegments1->begin(); s1 != allSegments1->end(); ++s1) {

                  AddIntersection(*s1,
                                  false,
                                  calculateRegionCoverage,
                                  ip,
                                  isCurrentPoint,
                                  true,
                                  intersectionEvent);
                }
                delete allSegments1;

                vector<InternalLineSegment*>* allSegments2 =
                    possibleIntersection->GetNode2()->GetAllSegments();

                for (vector<InternalLineSegment*>::const_iterator s2 =
                    allSegments2->begin(); s2 != allSegments2->end(); ++s2) {

                  AddIntersection(*s2,
                                  false,
                                  calculateRegionCoverage,
                                  ip,
                                  isCurrentPoint,
                                  true,
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

  if (outputData) {
    FlushProcessedSegments();
  } else {
    DeleteProcessedSegments();
  }
}

/*

1.1 ~AddIntersection~

*/
void BentleyOttmann::AddIntersection(InternalLineSegment* l,
                                     bool overlappingSegments,
                                     bool calculateRegionCoverage,
                                     const InternalIntersectionPoint& ip,
                                     bool isCurrentPoint,
                                     bool checkIntersectionPoint,
                                     SweepEvent*& intersectionEvent)
{
  if (checkIntersectionPoint) {
    if (ip.GetX() < l->GetLeft().GetX() || ip.GetX() > l->GetRight().GetX()) {
      return;
    }
    int ly = l->GetLeft().GetY();
    int ry = l->GetRight().GetY();

    if (ip.GetY() < (ly < ry ? ly : ry) || ip.GetY() > (ly > ry ? ly : ry)) {
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
          intersectionEvent = SweepEvent::CreateIntersection(ip);
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
    } else if (calculateRegionCoverage) {
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* prevNode =
          l->GetTreeNode()->GetPrevious();

      if (prevNode != NULL) {
        l->SetAttribute(prevNode->Value->GetAttributeAt(ip, true), false, ip);
      } else {
        l->SetAttributeNoBelow(false, ip);
      }
    }
  }
}

/*

1 Class ~PossibleIntersectionPair~

1.1 Constructors

*/
PossibleIntersectionPair::PossibleIntersectionPair(SweepStateData* node1,
                                                   SweepStateData* node2)
{
  if (node1->IsSingleSegment()) {
    if (node2->IsSingleSegment()) {
      _type = PossibleIntersectionPairType::SegmentSegment;
      _segment1 = node1->GetFirstSegment();
      _segment2 = node2->GetFirstSegment();
      _node1 = NULL;
      _node2 = NULL;
    } else {
      _type = PossibleIntersectionPairType::SegmentNode;
      _segment1 = node1->GetFirstSegment();
      _segment2 = NULL;
      _node1 = NULL;
      _node2 = node2;
    }
  } else if (node2->IsSingleSegment()) {
    _type = PossibleIntersectionPairType::SegmentNode;
    _segment1 = node2->GetFirstSegment();
    _segment2 = NULL;
    _node1 = NULL;
    _node2 = node1;
  } else {
    _type = PossibleIntersectionPairType::NodeNode;
    _segment1 = NULL;
    _segment2 = NULL;
    _node1 = node1;
    _node2 = node2;
  }
}

PossibleIntersectionPair::
PossibleIntersectionPair(InternalLineSegment* segment, SweepStateData* node)
{
  if (node->IsSingleSegment()) {
    _type = PossibleIntersectionPairType::SegmentSegment;
    _segment1 = segment;
    _segment2 = node->GetFirstSegment();
    _node1 = NULL;
    _node2 = NULL;
  } else {
    _type = PossibleIntersectionPairType::SegmentNode;
    _segment1 = segment;
    _segment2 = NULL;
    _node1 = NULL;
    _node2 = node;
  }
}

PossibleIntersectionPair::
PossibleIntersectionPair(InternalLineSegment* segment1,
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
