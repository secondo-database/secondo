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

// #define USE_RIGHTDOM_HALFSEGMENT

#include <unordered_map>
#include <queue>
#include <stdexcept>

#include "IntersectionAlgorithm.h"
#include "../Helper/Rational.h"

namespace RobustPlaneSweep
{
enum class SweepEventType
{
  End = 1,
  Intersection = 2,
  Start = 3,
  Point = 4
};

class SweepEvent
{
private:
  InternalLineSegment* _segment;
  std::unordered_set<InternalLineSegment*> *_intersectedSegments;
  InternalIntersectionPoint _point;
  SweepEventType _eventType;
  bool _belongsToSecondGeometry;

  SweepEvent(const SweepEventType eventType,
             const InternalIntersectionPoint& point)
  {
    _segment = NULL;
    _intersectedSegments = new std::unordered_set<InternalLineSegment*>();
    _eventType = eventType;
    _point = point;
  }

  SweepEvent(InternalLineSegment* segment,
             const SweepEventType eventType,
             const InternalIntersectionPoint& point)
  {
    _segment = segment;
    _intersectedSegments = NULL;
    _eventType = eventType;
    _point = point;
  }

  SweepEvent(const SweepEventType eventType,
             const InternalIntersectionPoint& point,
             const bool belongsToSecondGeometry)
  {
    _segment = NULL;
    _intersectedSegments = NULL;
    _eventType = eventType;
    _point = point;
    _belongsToSecondGeometry = belongsToSecondGeometry;
  }

  SweepEvent(SweepEvent&)
  {
  }

  static int GetEventTypeCompareValue(SweepEventType type)
  {
    switch (type) {
      case SweepEventType::Start:
        return 3;
      case SweepEventType::Intersection:
        return 2;
      case SweepEventType::End:
        return 1;
      case SweepEventType::Point:
        return 4;
      default:
        throw new std::invalid_argument("type");
    }
  }

public:
  static SweepEvent* CreateIntersection(const InternalIntersectionPoint &point)
  {
    return new SweepEvent(SweepEventType::Intersection, point);
  }

  static SweepEvent* CreateStart(InternalLineSegment* segment)
  {
    return new SweepEvent(segment,
                          SweepEventType::Start,
                          InternalIntersectionPoint(segment->GetLeft()));
  }

  static SweepEvent* CreateEnd(InternalLineSegment* segment)
  {
    return new SweepEvent(segment,
                          SweepEventType::End,
                          InternalIntersectionPoint(segment->GetRight()));
  }

  static SweepEvent* CreatePoint(const InternalIntersectionPoint &point,
                                 bool belongsToSecondGeometry)
  {
    return new SweepEvent(SweepEventType::Point,
                          point,
                          belongsToSecondGeometry);
  }

  InternalLineSegment* GetSegment() const
  {
    return _segment;
  }

  std::unordered_set<InternalLineSegment*>* GetIntersectedSegments() const
  {
    return _intersectedSegments;
  }

  SweepEventType GetEventType() const
  {
    return _eventType;
  }

  static bool SortComparer(const SweepEvent* x, const SweepEvent* y)
  {
    return Compare(x, y) < 0;
  }

  static int Compare(const SweepEvent* x, const SweepEvent* y)
  {
    int result;

    result = Rational::Compare(x->_point.GetX(), y->_point.GetX());
    if (result == 0) {
      result = Rational::Compare(x->_point.GetY(), y->_point.GetY());
    }

    if (result == 0) {
      result = GetEventTypeCompareValue(x->_eventType) -
               GetEventTypeCompareValue(y->_eventType);

      if (result != 0 && x->_eventType != SweepEventType::Intersection
          && y->_eventType != SweepEventType::Intersection
          && x->_eventType != SweepEventType::Point
          && y->_eventType != SweepEventType::Point
          && x->_segment == y->_segment) {
        result = -result;
      }

      if (result == 0) {
        if (x->_eventType != SweepEventType::Point
            && y->_eventType != SweepEventType::Point) {
          InternalLineSegment* sx = x->GetSegment();
          InternalLineSegment* sy = y->GetSegment();

          result = InternalLineSegment::CompareSlope(sx, sy);
        
          if (result == 0) {
            bool xf, yf;
            xf = x->GetSegment()->GetInitialAttribute().IsBoundaryInFirst();
            yf = y->GetSegment()->GetInitialAttribute().IsBoundaryInFirst();
            result = (xf ? 0 : 1) - (yf ? 0 : 1);
          }
        }
      }
    }

    return result;
  }

  const InternalIntersectionPoint GetPoint() const
  {
    return _point;
  }

  bool BelongsToSecondGeometry() const
  {
    return _belongsToSecondGeometry;
  }

  ~SweepEvent()
  {
    if (_intersectedSegments != NULL) {
      delete _intersectedSegments;
      _intersectedSegments = NULL;
    }
  }
};

class PossibleIntersectionPair;

enum class PossibleIntersectionPairType
{
  Undefined,
  SegmentNode,
  NodeNode,
  SegmentSegment,
  SegmentSegmentOverlapping
};

class SweepStateData
{
private:
  InternalLineSegment* _firstSegment;
  std::unordered_set<InternalLineSegment*>* _segments;
  InternalPoint _minLeft;
  InternalPoint _maxRight;

public:
  explicit SweepStateData(InternalLineSegment* segment) :
      _minLeft(segment->GetLeft()),
      _maxRight(segment->GetRight())
  {
    if (segment == NULL) {
      throw new std::invalid_argument("segment");
    }

    _firstSegment = segment;
    _segments = NULL;
  }

  void Add(InternalLineSegment* segment)
  {
    if (segment == _firstSegment) {
      return;
    }

    InternalPoint right = segment->GetRight();

    if (right.GetX() > _maxRight.GetX()
        || (right.GetX() == _maxRight.GetX()
            && right.GetY() > _maxRight.GetY())) {
      _maxRight = right;
    }

    if (_segments == NULL) {
      _segments = new std::unordered_set<InternalLineSegment*>();
    }

    _segments->insert(segment);
  }

  bool Remove(InternalLineSegment* segment)
  {
    if (segment == _firstSegment) {
      if (_segments == NULL) {
        return true;
      } else {
        _firstSegment = *(_segments->begin());
        _segments->erase(_firstSegment);
        if (_segments->size() == 0) {
          delete _segments;
          _segments = NULL;
        }
      }

      return false;
    }

    _segments->erase(segment);

    if (_segments->size() == 0) {
      delete _segments;
      _segments = NULL;
    }

    return false;
  }

  Rational GetYValueAt(const InternalIntersectionPoint& point) const
  {
    return _firstSegment->GetYValueAt(point);
  }

  InternalAttribute GetAttributeAt(const InternalIntersectionPoint& point,
                                   bool includePoint) const
  {
    if (IsSingleSegment()) {
      return _firstSegment->GetAttributeAt(point, includePoint);
    } else if (_segments->size() > 1) {
      throw new std::logic_error("more than two overlapping segments");
    } else {
      InternalAttribute x, y;
      x = _firstSegment->GetAttributeAt(point, includePoint);
      y = (*(_segments->begin()))->GetAttributeAt(point, includePoint);
      if (x.IsBoundaryInFirst()) {
        if (y.IsBoundaryInFirst()) {
          throw new std::logic_error("overlapping segments "
                                     "in first geometry");
        }

        return InternalAttribute(x.GetFirst(), y.GetSecond());
      } else if (x.IsBoundaryInSecond()) {
        if (y.IsBoundaryInSecond()) {
          throw new std::logic_error("overlapping segments "
                                     "in second geometry");
        }
        return InternalAttribute(y.GetFirst(), x.GetSecond());
      } else {
        throw new std::logic_error("could not determine geometry owner");
      }
    }
  }

  InternalLineSegment* GetFirstSegment() const
  {
    return _firstSegment;
  }

  InternalPoint GetMinLeft() const
  {
    return _minLeft;
  }

  InternalPoint GetMaxRight() const
  {
    return _maxRight;
  }

  bool IsSingleSegment() const
  {
    return _segments == NULL;
  }

  std::vector<InternalLineSegment*>* GetAllSegments()
  {
    std::vector<InternalLineSegment*>* result =
        new std::vector<InternalLineSegment*>();

    if (_segments == NULL) {
      result->push_back(_firstSegment);
    } else {
      result->push_back(_firstSegment);
      for (std::unordered_set<InternalLineSegment*>::const_iterator
      i = _segments->begin();
          i != _segments->end(); ++i) {
        result->push_back(*i);
      }
    }

    return result;
  }
};

class PossibleIntersectionPair
{
private:
  PossibleIntersectionPairType _type;
  SweepStateData* _node1;
  SweepStateData* _node2;
  InternalLineSegment* _segment1;
  InternalLineSegment* _segment2;

public:
  PossibleIntersectionPair(SweepStateData* node1,
                           SweepStateData* node2);

  PossibleIntersectionPair(InternalLineSegment* segment,
                           SweepStateData* node);

  PossibleIntersectionPair(InternalLineSegment* segment1,
                           InternalLineSegment* segment2,
                           PossibleIntersectionPairType type);

  PossibleIntersectionPairType GetType() const
  {
    return _type;
  }

  SweepStateData* GetNode1() const
  {
    return _node1;
  }

  SweepStateData* GetNode2() const
  {
    return _node2;
  }

  InternalLineSegment* GetSegment1() const
  {
    return _segment1;
  }

  InternalLineSegment* GetSegment2() const
  {
    return _segment2;
  }
};

class SweepState
{
private:
  InternalIntersectionPoint _currentPoint;
  AvlTree<InternalLineSegment*, SweepStateData*, SweepState>* _tree;
  bool _assumeYEqual;

  void
  DeleteTreeContent(AvlTreeNode<InternalLineSegment*, SweepStateData*> *node)
  {
    if (node != NULL) {
      if (node->Value != NULL) {
        delete node->Value;
      }

      DeleteTreeContent(node->Left);
      DeleteTreeContent(node->Right);
    }
  }

public:
  SweepState()
  {
    _currentPoint =
        InternalIntersectionPoint(Rational(std::numeric_limits<int>::min()),
                                  Rational(std::numeric_limits<int>::min()));

    _assumeYEqual = false;

    _tree = new AvlTree<
        InternalLineSegment*,
        SweepStateData*,
        SweepState>(this);
  }

  ~SweepState()
  {
    if (_tree != NULL) {
      DeleteTreeContent(_tree->GetRoot());
      delete _tree;
      _tree = NULL;
    }
  }

  const InternalIntersectionPoint& GetCurrentPoint() const
  {
    return _currentPoint;
  }

  void SetCurrentPoint(const InternalIntersectionPoint& newCurrentPoint)
  {
    _currentPoint = newCurrentPoint;
  }

  bool GetAssumeYEqual() const
  {
    return _assumeYEqual;
  }

  void SetAssumeYEqual(bool assumeYEqual)
  {
    _assumeYEqual = assumeYEqual;
  }

  void Add(InternalLineSegment* segment,
           std::vector<PossibleIntersectionPair>& possibleIntersectionPairs,
           bool calculateRegionCoverage)
  {
    SetAssumeYEqual(false);
    bool isOverlapping;

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* node = NULL;
    if (_tree->TryAddNode(segment, node)) {
      node->Value = new SweepStateData(segment);
      segment->SetTreeNode(node);
      isOverlapping = false;
    } else {
      SweepStateData* data = node->Value;
      if (data->IsSingleSegment()) {
        possibleIntersectionPairs.
            push_back(PossibleIntersectionPair(data->GetFirstSegment(),
                                               segment,
                                               PossibleIntersectionPairType::
                                               SegmentSegmentOverlapping));
      } else {
        std::vector<InternalLineSegment*>* allSegments =
            data->GetAllSegments();

        for (std::vector<InternalLineSegment*>::const_iterator i =
            allSegments->begin(); i != allSegments->end(); ++i) {
          possibleIntersectionPairs.
              push_back(PossibleIntersectionPair(*i,
                                                 segment,
                                                 PossibleIntersectionPairType::
                                                 SegmentSegmentOverlapping));
        }

        delete allSegments;
      }

      node->Value->Add(segment);
      segment->SetTreeNode(node);
      isOverlapping = true;
    }

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* prevNode =
        node->GetPrevious();

    if (prevNode != NULL) {
      possibleIntersectionPairs.
          push_back(PossibleIntersectionPair(segment, prevNode->Value));
    }

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* nextNode =
        node->GetNext();

    if (nextNode != NULL) {
      possibleIntersectionPairs.
          push_back(PossibleIntersectionPair(segment, nextNode->Value));
    }

    if (calculateRegionCoverage) {
      if (prevNode != NULL && !isOverlapping) {
        segment->SetAttribute(prevNode->Value->GetAttributeAt(GetCurrentPoint(),
                                                              true),
                              true,
                              InternalIntersectionPoint());
      } else {
        segment->SetAttributeNoAbove(true,
                                     InternalIntersectionPoint());
      }
    }
  }

  void Remove(InternalLineSegment* segment,
              std::vector<PossibleIntersectionPair>& possibleIntersectionPairs)
  {
    SetAssumeYEqual(false);

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* treeNode =
        segment->GetTreeNode();

    if (treeNode->Value->Remove(segment)) {
      segment->SetTreeNode(NULL);
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* prevNode =
          treeNode->GetPrevious();

      AvlTreeNode<InternalLineSegment*, SweepStateData*>* nextNode =
          treeNode->GetNext();

      delete treeNode->Value;
      _tree->Remove(treeNode);

      if (prevNode != NULL && nextNode != NULL) {
        possibleIntersectionPairs.
            push_back(PossibleIntersectionPair(prevNode->Value,
                                               nextNode->Value));
      }
    } else if (treeNode->GetKey() == segment) {
      treeNode->SetKey(treeNode->Value->GetFirstSegment());
    }
  }

  void Reorder(const SweepEvent* sweepEvent,
               std::vector<PossibleIntersectionPair>& possibleIntersectionPairs,
               bool calculateRegionCoverage)
  {
    if (sweepEvent->GetIntersectedSegments()->size() > 1) {
      SetAssumeYEqual(true);

      std::vector<AvlTreeNode<InternalLineSegment*, SweepStateData*>*> nodes;
      size_t firstNodeIndex;
      size_t lastNodeIndex;

      if (sweepEvent->GetIntersectedSegments()->size() == 2) {
        std::unordered_set<InternalLineSegment*>::const_iterator i =
            sweepEvent->GetIntersectedSegments()->begin();

        AvlTreeNode<InternalLineSegment*, SweepStateData*>* n0 =
            (*i)->GetTreeNode();

        ++i;

        AvlTreeNode<InternalLineSegment*, SweepStateData*>* n1 =
            (*i)->GetTreeNode();

        if (n0 == n1) {
          return;
        } else if (n0->GetNext() == n1) {
          nodes.push_back(n0);
          nodes.push_back(n1);
          firstNodeIndex = 0;
          lastNodeIndex = 1;
        } else if (n1->GetNext() == n0) {
          nodes.push_back(n1);
          nodes.push_back(n0);
          firstNodeIndex = 0;
          lastNodeIndex = 1;
        } else {
          throw new std::logic_error("n0 and n1 must be adjecent!");
        }
      } else {
        GetNodesToSwap(sweepEvent, nodes, firstNodeIndex, lastNodeIndex);
      }

      for (size_t i = firstNodeIndex, j = lastNodeIndex; i < j; ++i, --j) {
        _tree->SwapNodes(nodes[i], nodes[j]);
      }

      // last node was first node
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* prevFirstNode =
          nodes[lastNodeIndex]->GetPrevious();

      if (prevFirstNode != NULL) {
        possibleIntersectionPairs.
            push_back(PossibleIntersectionPair(prevFirstNode->Value,
                                               nodes[lastNodeIndex]->Value));
      }

      AvlTreeNode<InternalLineSegment*, SweepStateData*>* nextLastNode =
          nodes[firstNodeIndex]->GetNext();

      if (nextLastNode != NULL) {
        possibleIntersectionPairs.
            push_back(PossibleIntersectionPair(nextLastNode->Value,
                                               nodes[firstNodeIndex]->Value));
      }
    }

    if (calculateRegionCoverage) {
      for (std::unordered_set<InternalLineSegment*>::const_iterator i =
          sweepEvent->GetIntersectedSegments()->begin();
          i != sweepEvent->GetIntersectedSegments()->end(); ++i) {
        InternalLineSegment* segment = *i;
        AvlTreeNode<InternalLineSegment*, SweepStateData*>* prevNode =
            segment->GetTreeNode()->GetPrevious();

        if (prevNode != NULL) {
          InternalIntersectionPoint p = sweepEvent->GetPoint();
          segment->SetAttribute(prevNode->Value->GetAttributeAt(p, true),
                                false,
                                sweepEvent->GetPoint());
        } else {
          segment->SetAttributeNoAbove(false, sweepEvent->GetPoint());
        }
      }
    }
  }

  static void GetNodesToSwap(const SweepEvent* sweepEvent,
                             std::vector<
                                 AvlTreeNode<InternalLineSegment*,
                                     SweepStateData*>*> &nodes,
                             size_t& firstNodeIndex,
                             size_t& lastNodeIndex)
  {
    size_t offset = sweepEvent->GetIntersectedSegments()->size();
    for (unsigned int i = 0; i < offset * 2 + 1; ++i) {
      nodes.
          push_back((AvlTreeNode<InternalLineSegment*, SweepStateData*>*)NULL);
    }

    bool isFirst = true;
    firstNodeIndex = offset;
    lastNodeIndex = offset;

    for (std::unordered_set<InternalLineSegment*>::const_iterator i = sweepEvent
        ->GetIntersectedSegments()->begin();
        i != sweepEvent->GetIntersectedSegments()->end(); ++i) {
      InternalLineSegment* segment = *i;
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* currentNode =
          segment->GetTreeNode();

      if (isFirst) {
        nodes[offset] = currentNode;
        isFirst = false;
      } else {
        bool assigned = false;
        AvlTreeNode<InternalLineSegment*, SweepStateData*>* previousNode =
            nodes[offset];

        AvlTreeNode<InternalLineSegment*, SweepStateData*>* nextNode =
            previousNode;

        for (size_t previosIndex = offset, nextIndex = offset; previosIndex > 0;
            --previosIndex, ++nextIndex) {
          if (previousNode == currentNode) {
            if (nodes[previosIndex] == NULL
                || nodes[previosIndex] == currentNode) {
              nodes[previosIndex] = currentNode;
              if (previosIndex < firstNodeIndex) {
                firstNodeIndex = previosIndex;
              }
              assigned = true;
              break;
            } else {
              throw new std::logic_error("wrong node order!");
            }
          } else if (nextNode == currentNode) {
            if (nodes[nextIndex] == NULL || nodes[nextIndex] == currentNode) {
              nodes[nextIndex] = currentNode;
              if (nextIndex > lastNodeIndex) {
                lastNodeIndex = nextIndex;
              }
              assigned = true;
              break;
            } else {
              throw new std::logic_error("wrong node order!");
            }
          }

          if (previousNode != NULL) {
            previousNode = previousNode->GetPrevious();
          }

          if (nextNode != NULL) {
            nextNode = nextNode->GetNext();
          }
        }

        if (!assigned) {
          throw new std::logic_error("node could not be assigned!");
        }
      }
    }
  }

  int Compare(InternalLineSegment* x, InternalLineSegment* y) const
  {
    if (x == y) {
      return 0;
    }

    int result;
    if (!_assumeYEqual) {
      Rational yx = x->GetYValueAt(_currentPoint);
      Rational yy = y->GetYValueAt(_currentPoint);

      result = Rational::Compare(yx, yy);
    } else {
      result = 0;
    }

    if (result == 0) {
      result = InternalLineSegment::CompareSlope(x, y);
    }

    return result;
  }

  AvlTreeNode<InternalLineSegment*, SweepStateData*>* GetTreeRoot()
  {
    return _tree->GetRoot();
  }
};

class BentleyOttmann : public IntersectionAlgorithm
{
private:
  struct SweepEventCompare
  {
    bool operator()(SweepEvent*& x, SweepEvent* &y) const
                    {
      return SweepEvent::Compare(x, y) > 0;
    }
  };

#ifdef USE_RIGHTDOM_HALFSEGMENT
  std::unordered_map<
  HalfSegmentIntersectionId,
  InternalLineSegment*> _activeSegments;
#else
  std::unordered_set<InternalLineSegment*> _activeSegments;
  #endif

  std::unordered_map<
      InternalIntersectionPoint,
      SweepEvent*,
      InternalIntersectionPointComparer,
      InternalIntersectionPointComparer> _intersections;

  std::priority_queue<
      SweepEvent*,
      std::vector<SweepEvent*>,
      SweepEventCompare> _intersectionEvents;

  HalfSegment _nextDataHalfSegment;
  Point _nextDataPoint;
  bool _nextDataValid;
  bool _nextDataBelongsToSecondGeometry;

  int _lastFetchedX;

  std::queue<SweepEvent*> _startEndEvents;

  std::vector<InternalLineSegment*> _processedInternalSegments;

  SweepEvent* _nextHalfSegmentSweepEvent;
  size_t _intersectionCount;

  SweepState *_state;

  void FetchInput();

  void SetNextStartEndEvent();

  SweepEvent* GetNextEvent();

  SweepEvent* CreateEvent(const HalfSegment& halfSegment,
                          const Point& point,
                          const bool belongsToSecondGeometry);

  void FlushProcessedSegments();
  void DeleteProcessedSegments();

  bool PointIntersectsWithAvlTree(Rational searchY) const;

  void AddIntersection(InternalLineSegment* l,
                       bool overlappingSegments,
                       bool calculateRegionCoverage,
                       const InternalIntersectionPoint& ip,
                       bool isCurrentPoint,
                       bool checkIntersectionPoint,
                       SweepEvent*& intersectionEvent);

protected:
  int GetInitialScaleFactor() const
  {
    return 1;
  }

  virtual int GetBreakupUntil() const
  {
    return GetTransformation()->RoundRational(_state->GetCurrentPoint().GetX());
  }

  virtual void DetermineIntersectionsInternal();

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
  virtual void BeforeProcessEvent(SweepEvent* sweepEvent)
  {
  }

  virtual void Finished()
  {
  }

  virtual bool OnXChanged(const Rational& oldX, const Rational& newX)
  {
    if (GetTransformation()->RoundRational(oldX) <
        GetTransformation()->RoundRational(newX)) {
      return true;
    } else {
      return false;
    }
  }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  SweepState* GetState() const
  {
    return _state;
  }

public:
  explicit BentleyOttmann(IntersectionAlgorithmData* data) :
      IntersectionAlgorithm(data)
  {
    _state = NULL;
    _intersectionCount = 0;
    _nextHalfSegmentSweepEvent = NULL;
  }

  void DetermineIntersections();

  virtual ~BentleyOttmann()
  {
    if (_state != NULL) {
      delete _state;
      _state = NULL;
    }

    while (!_intersectionEvents.empty()) {
      SweepEvent* e = _intersectionEvents.top();
      if (e->GetEventType() == SweepEventType::Start) {
        delete e->GetSegment();
      }

      delete e;
      _intersectionEvents.pop();
    }

    if (_nextHalfSegmentSweepEvent != NULL) {
      if (_nextHalfSegmentSweepEvent->GetEventType() == SweepEventType::Start) {
        delete _nextHalfSegmentSweepEvent->GetSegment();
      }

      delete _nextHalfSegmentSweepEvent;
      _nextHalfSegmentSweepEvent = NULL;
    }

    while (!_startEndEvents.empty()) {
      SweepEvent* e = _startEndEvents.front();
      if (e->GetEventType() == SweepEventType::Start) {
        delete e->GetSegment();
      }

      delete e;
      _startEndEvents.pop();
    }

    for (auto s = _activeSegments.begin(); s != _activeSegments.end(); s++) {
      delete (*s);
    }

    DeleteProcessedSegments();
  }

  size_t GetIntersectionCount() const
  {
    return _intersectionCount;
  }
};
}
