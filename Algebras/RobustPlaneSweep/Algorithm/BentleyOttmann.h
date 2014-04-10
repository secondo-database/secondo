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

[1] Header File for the class ~BentleyOttmann~

[TOC]

1 Overview

This header file contains all structs and classes required for the
class ~BentleyOttmann~.

This class implements the Bentley-Ottmann-Algorithm. 

To use this class, derive a data class from ~IntersectionAlgorithmData~ and 
overwrite the necessary methods.

1 Defines and includes

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
/*

1 Internal helper classes

1.1 Enum class ~SweepEventType~

*/
enum class SweepEventType
{
  End = 1,
  Intersection = 2,
  Start = 3,
  Point = 4
};

/*

1.1 Class ~SweepEvent~

*/
class SweepEvent
{
/*

1.1.1 Member variables

*/
private:
  InternalLineSegment* _segment;
  std::unordered_set<InternalLineSegment*> *_intersectedSegments;
  InternalIntersectionPoint _point;
  SweepEventType _eventType;
  bool _belongsToSecondGeometry;

/*

1.1.1 Private constructors

*/
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

/*

1.1.1 ~GetEventTypeCompareValue~

*/
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
/*

1.1.1 ~CreateIntersection~

*/
  static SweepEvent* CreateIntersection(const InternalIntersectionPoint &point)
  {
    return new SweepEvent(SweepEventType::Intersection, point);
  }

/*

1.1.1 ~CreateStart~

*/
  static SweepEvent* CreateStart(InternalLineSegment* segment)
  {
    return new SweepEvent(segment,
                          SweepEventType::Start,
                          InternalIntersectionPoint(segment->GetLeft()));
  }

/*

1.1.1 ~CreateEnd~

*/
  static SweepEvent* CreateEnd(InternalLineSegment* segment)
  {
    return new SweepEvent(segment,
                          SweepEventType::End,
                          InternalIntersectionPoint(segment->GetRight()));
  }

/*

1.1.1 ~CreatePoint~

*/
  static SweepEvent* CreatePoint(const InternalIntersectionPoint &point,
                                 bool belongsToSecondGeometry)
  {
    return new SweepEvent(SweepEventType::Point,
                          point,
                          belongsToSecondGeometry);
  }

/*

1.1.1 ~GetSegment~

*/
  InternalLineSegment* GetSegment() const
  {
    return _segment;
  }

/*

1.1.1 ~GetIntersectedSegments~

*/
  std::unordered_set<InternalLineSegment*>* GetIntersectedSegments() const
  {
    return _intersectedSegments;
  }

/*

1.1.1 ~GetEventType~

*/
  SweepEventType GetEventType() const
  {
    return _eventType;
  }

/*

1.1.1 ~SortComparer~

*/
  static bool SortComparer(const SweepEvent* x, const SweepEvent* y)
  {
    return Compare(x, y) < 0;
  }

/*

1.1.1 ~Compare~

*/
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

/*

1.1.1 ~GetPoint~

*/
  const InternalIntersectionPoint GetPoint() const
  {
    return _point;
  }

/*

1.1.1 ~BelongsToSecondGeometry~

*/
  bool BelongsToSecondGeometry() const
  {
    return _belongsToSecondGeometry;
  }

/*

1.1.1 Destructor

*/
  ~SweepEvent()
  {
    if (_intersectedSegments != NULL) {
      delete _intersectedSegments;
      _intersectedSegments = NULL;
    }
  }
};

/*

1.1 Forward declaration of the class ~PossibleIntersectionPair~

*/
class PossibleIntersectionPair;

/*

1.1 Enum class ~PossibleIntersectionPairType~

This enum contains the types of intersection tests. 
For example, ~SegmentNode~ means, the ~PossibleIntersectionPair~ contains 
a node (of type ~SweepStateData~) and a segment (of type ~InternalLineSegment~).

*/
enum class PossibleIntersectionPairType
{
  Undefined,
  SegmentNode,
  NodeNode,
  SegmentSegment,
  SegmentSegmentOverlapping
};

/*

1.1 Class ~SweepStateData~

~SweepStateData~ objects are used inside the sweep status structure. 
They contain a set of (overlapping) ~InternalLineSegment~ pointers. 
Because there is usually only one such pointer, there is a member variable
~\_firstSegment~. The creation of the set is delayed until there are at least
two segments.


*/
class SweepStateData
{
/*

1.1.1 Member variables

*/
private:
  InternalLineSegment* _firstSegment;
  std::unordered_set<InternalLineSegment*>* _segments;
  InternalPoint _minLeft;
  InternalPoint _maxRight;

public:
/*

1.1.1 Constructor

*/
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

/*

1.1.1 ~Add~

*/
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

/*

1.1.1 ~Remove~

*/
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

/*

1.1.1 ~GetYValueAt~

*/
  Rational GetYValueAt(const InternalIntersectionPoint& point) const
  {
    return _firstSegment->GetYValueAt(point);
  }

/*

1.1.1 ~GetAttributeAt~

*/
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

/*

1.1.1 ~GetFirstSegment~

*/
  InternalLineSegment* GetFirstSegment() const
  {
    return _firstSegment;
  }

/*

1.1.1 ~GetMinLeft~

*/
  InternalPoint GetMinLeft() const
  {
    return _minLeft;
  }

/*

1.1.1 ~GetMaxRight~

*/
  InternalPoint GetMaxRight() const
  {
    return _maxRight;
  }

/*

1.1.1 ~IsSingleSegment~

*/
  bool IsSingleSegment() const
  {
    return _segments == NULL;
  }

/*

1.1.1 ~GetAllSegments~

*/
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

/* 

1.1 Class ~PossibleIntersectionPair~

The ~PossibleIntersectionPair~ class is used inside the sweep status structure
to collect pairs of segments, which needs checking for intersections.

*/
class PossibleIntersectionPair
{
/*

1.1.1 Member variables

*/
private:
  PossibleIntersectionPairType _type;
  SweepStateData* _node1;
  SweepStateData* _node2;
  InternalLineSegment* _segment1;
  InternalLineSegment* _segment2;

public:
/*

1.1.1 Constructors

*/
  PossibleIntersectionPair(SweepStateData* node1,
                           SweepStateData* node2);

  PossibleIntersectionPair(InternalLineSegment* segment,
                           SweepStateData* node);

  PossibleIntersectionPair(InternalLineSegment* segment1,
                           InternalLineSegment* segment2,
                           PossibleIntersectionPairType type);

/*

1.1.1 ~GetType~

*/
  PossibleIntersectionPairType GetType() const
  {
    return _type;
  }

/*

1.1.1 ~GetNode1~

*/
  SweepStateData* GetNode1() const
  {
    return _node1;
  }

/*

1.1.1 ~GetNode2~

*/
  SweepStateData* GetNode2() const
  {
    return _node2;
  }

/*

1.1.1 ~GetSegment1~

*/
  InternalLineSegment* GetSegment1() const
  {
    return _segment1;
  }

/*

1.1.1 ~GetSegment2~

*/
  InternalLineSegment* GetSegment2() const
  {
    return _segment2;
  }
};

/* 

1.1 Class ~SweepState~

This class contains the sweep status structure of the Bentley-Ottmann algorithm.
It wraps an AVL-tree and contains an ~InternalIntersectionPoint~ which 
represents the current position of the sweep line.

*/
class SweepState
{
/*

1.1.1 Member variables

*/
private:
  InternalIntersectionPoint _currentPoint;
  AvlTree<InternalLineSegment*, SweepStateData*, SweepState>* _tree;
  bool _assumeYEqual;


/*

1.1.1 ~DeleteTreeContent~

*/
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
/*

1.1.1 Constructor

*/
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

/*

1.1.1 Destructor

*/
  ~SweepState()
  {
    if (_tree != NULL) {
      DeleteTreeContent(_tree->GetRoot());
      delete _tree;
      _tree = NULL;
    }
  }

/*

1.1.1 ~GetCurrentPoint~

*/
  const InternalIntersectionPoint& GetCurrentPoint() const
  {
    return _currentPoint;
  }

/*

1.1.1 ~SetCurrentPoint~

*/
  void SetCurrentPoint(const InternalIntersectionPoint& newCurrentPoint)
  {
    _currentPoint = newCurrentPoint;
  }

/*

1.1.1 ~GetAssumeYEqual~

*/
  bool GetAssumeYEqual() const
  {
    return _assumeYEqual;
  }

/*

1.1.1 ~SetAssumeYEqual~

*/
  void SetAssumeYEqual(bool assumeYEqual)
  {
    _assumeYEqual = assumeYEqual;
  }

/*

1.1.1 ~Add~

*/
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
        segment->SetAttributeNoBelow(true,
                                     InternalIntersectionPoint());
      }
    }
  }

/*

1.1.1 ~Remove~

*/
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

/*

1.1.1 ~Reorder~

*/
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
          segment->SetAttributeNoBelow(false, sweepEvent->GetPoint());
        }
      }
    }
  }

/*

1.1.1 ~GetNodesToSwap~

*/
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

/*

1.1.1 ~Compare~

*/
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

/*

1 Class ~BentleyOttmann~

This class contains the declaration for the 
implementation of the Bentley-Ottmann algorithm.

*/
class BentleyOttmann : public IntersectionAlgorithm
{
private:
/*

1.1 Internal struct ~SweepEventCompare~

*/
  struct SweepEventCompare
  {
    bool operator()(SweepEvent*& x, SweepEvent* &y) const
    {
      return SweepEvent::Compare(x, y) > 0;
    }
  };

/*

1.1 Member variables

*/
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

/*

1.1 Methods

1.1.1 ~FetchInput~

*/
  void FetchInput();

/*

1.1.1 ~SetNextStartEndEvent~

*/
  void SetNextStartEndEvent();

/*

1.1.1 ~GetNextEvent~

*/
  SweepEvent* GetNextEvent();

/*

1.1.1 ~CreateEvent~

*/
  SweepEvent* CreateEvent(const HalfSegment& halfSegment,
                          const Point& point,
                          const bool belongsToSecondGeometry);

/*

1.1.1 ~FlushProcessedSegments~

*/
  void FlushProcessedSegments();
/*

1.1.1 ~DeleteProcessedSegments~

*/
  void DeleteProcessedSegments();

/*

1.1.1 ~PointIntersectsWithAvlTree~

*/
  bool PointIntersectsWithAvlTree(Rational searchY) const;

/*

1.1.1 ~AddIntersection~

*/
  void AddIntersection(InternalLineSegment* l,
                       bool overlappingSegments,
                       bool calculateRegionCoverage,
                       const InternalIntersectionPoint& ip,
                       bool isCurrentPoint,
                       bool checkIntersectionPoint,
                       SweepEvent*& intersectionEvent);

protected:
/*

1.1.1 ~GetInitialScaleFactor~

overwritten by ~Hobby~ class.

*/
  virtual int GetInitialScaleFactor() const
  {
    return 1;
  }

/*

1.1.1 ~GetBreakupUntil~

overwritten by ~Hobby~ class.

*/
  virtual int GetBreakupUntil() const
  {
    return GetTransformation()->RoundRational(_state->GetCurrentPoint().GetX());
  }

/*

1.1.1 ~DetermineIntersectionsInternal~

overwritten by ~Hobby~ class.

*/
  virtual void DetermineIntersectionsInternal();

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
/*

1.1.1 ~BeforeProcessEvent~

overwritten by ~Hobby~ class.

*/
  virtual void BeforeProcessEvent(SweepEvent* sweepEvent)
  {
  }

/*

1.1.1 ~Finished~

overwritten by ~Hobby~ class.

*/
  virtual void Finished()
  {
  }

/*

1.1.1 ~OnXChanged~

overwritten by ~Hobby~ class.

*/
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

/*

1.1.1 ~GetState~

*/
  SweepState* GetState() const
  {
    return _state;
  }

public:
/*

1.1.1 Constructor

*/
  explicit BentleyOttmann(IntersectionAlgorithmData* data) :
      IntersectionAlgorithm(data)
  {
    _state = NULL;
    _intersectionCount = 0;
    _nextHalfSegmentSweepEvent = NULL;
  }

/*

1.1.1 ~DetermineIntersections~

*/
  void DetermineIntersections();

/*

1.1.1 Destructor

*/
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

/*

1.1.1 ~GetIntersectionCount~

*/
  size_t GetIntersectionCount() const
  {
    return _intersectionCount;
  }
};
}
