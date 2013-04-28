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

#include <vector>
#include <unordered_set>
#include <map>
#include <algorithm>

#include "Utility.h"
#include "AvlTree.h"
#ifdef RPS_TEST
#include "SpatialAlgebraStubs.h"
#else
#include "SpatialAlgebra.h"
#endif
#include "Rational.h"

namespace RobustPlaneSweep
{
  class SweepStateData;

  class InternalAttribute
  {
  private:
    bool _defined;
    bool _firstBorder;
    bool _firstAbove;
    bool _firstBelow;

    bool _secondBorder;
    bool _secondAbove;
    bool _secondBelow;

  public:
    InternalAttribute() :
      _defined(false),
      _firstBorder(false),
      _firstAbove(false),
      _firstBelow(false),
      _secondBorder(false),
      _secondAbove(false),
      _secondBelow(false)
    {
    }

    InternalAttribute(
      bool firstBorder,
      bool firstAbove,
      bool firstBelow,
      bool secondBorder,
      bool secondAbove,
      bool secondBelow) :
    _defined(true),
      _firstBorder(firstBorder),
      _firstAbove(firstAbove),
      _firstBelow(firstBelow),
      _secondBorder(secondBorder),
      _secondAbove(secondAbove),
      _secondBelow(secondBelow)
    {
      // if ((firstBorder && firstAbove && firstBelow) ||
      //   (!firstBorder && (firstAbove ^ firstBelow))) {
      //     throw new std::invalid_argument("firstBorder");
      // }

      // if ((secondBorder && secondAbove && secondBelow) ||
      //   (!secondBorder && (secondAbove ^ secondBelow))) {
      //     throw new std::invalid_argument("secondBorder");
      // }
    }

    bool IsDefined() const
    {
      return _defined;
    }

    bool IsFirstBorder() const
    {
      return _firstBorder;
    }

    bool IsFirstAbove() const
    {
      return _firstAbove;
    }

    bool IsFirstBelow() const
    {
      return _firstBelow;
    }

    bool IsSecondBorder() const
    {
      return _secondBorder;
    }

    bool IsSecondAbove() const
    {
      return _secondAbove;
    }

    bool IsSecondBelow() const
    {
      return _secondBelow;
    }

    bool IsBorderInBoth() const
    {
      return IsFirstBorder() && IsSecondBorder();
    }

    bool IsInUnionRegion(bool& insideAbove) const
    {
      int coverageAbove = (IsFirstAbove() ? 1 : 0) + (IsSecondAbove() ? 1 : 0);
      int coverageBelow = (IsFirstBelow() ? 1 : 0) + (IsSecondBelow() ? 1 : 0);

      if (coverageAbove == 0 && coverageBelow > 0) {
        insideAbove = false;
        return true;
      } else if (coverageAbove > 0 && coverageBelow == 0) {
        insideAbove = true;
        return true;
      } else {
        return false;
      }
    }

    bool IsInIntersectionRegion(bool& insideAbove) const
    {
      if (IsFirstAbove() && IsSecondAbove()) {
        insideAbove = true;
        return true;
      } else if (IsFirstBelow() && IsSecondBelow()) {
        insideAbove = false;
        return true;
      } else {
        return false;
      }
    }

    bool IsInMinusRegion(bool& insideAbove) const
    {
      int coverageAbove = (IsFirstAbove() ? 1 : 0) + (IsSecondAbove() ? 1 : 0);
      int coverageBelow = (IsFirstBelow() ? 1 : 0) + (IsSecondBelow() ? 1 : 0);
      bool belongsToFirst = (IsFirstAbove() != IsFirstBelow());
      bool belongsToSecond = (IsSecondAbove() != IsSecondBelow());

      if (belongsToFirst && belongsToSecond) {
        if (coverageAbove == 1 && coverageBelow == 1) {
          insideAbove = IsFirstAbove();
          return true;
        }
      } else if (belongsToFirst) {
        if ((coverageAbove + coverageBelow) == 1) {
          insideAbove = IsFirstAbove();
          return true;
        }
      } else if (belongsToSecond) {
        if ((coverageAbove + coverageBelow) == 3) {
          insideAbove = IsSecondBelow();
          return true;
        }
      }

      return false;
    }



    static InternalAttribute Create(
      bool belongsToSecondGeometry,
      bool insideAbove,
      bool calculateCoverage)
    {
      if (calculateCoverage) {
        if (!belongsToSecondGeometry) {
          return InternalAttribute(
            true, insideAbove, !insideAbove,
            false, false, false);
        } else {
          return InternalAttribute(
            false, false, false,
            true, insideAbove, !insideAbove);
        }
      } else {
        return InternalAttribute(
          !belongsToSecondGeometry, false, false,
          belongsToSecondGeometry, false, false);
      }
    }

    static void Merge(
      bool above1, bool below1,
      bool above2, bool below2,
      bool& newAbove, bool& newBelow)
    {
      if (!above1 && !below1) {
        if (above2 && below2) {
          throw new std::logic_error("");
        }

        newAbove = above2;
        newBelow = below2;
        return;
      } else if (!above1 && below1) {
        if (!above2 && !below2) {
          newAbove = false;
          newBelow = true;
        } else if (!above2 && below2) {
          throw new std::logic_error("");
        } else if (above2 && !below2) {
          newAbove = false;
          newBelow = false;
        } else /*(above2 && below2)*/ {
          newAbove = false;
          newBelow = true;
        }
      } else if (above1 && !below1) {
        if (!above2 && !below2) {
          newAbove = true;
          newBelow = false;
        } else if (!above2 && below2) {
          newAbove = true;
          newBelow = true;
        } else if (above2 && !below2) {
          throw new std::logic_error("");
        } else /*(above2 && below2)*/ {
          throw new std::logic_error("");
        }
      } else /*(above1 && below1)*/ {
        if (!above2 && !below2) {
          throw new std::logic_error("");
        } else if (!above2 && below2) {
          // overlapping
          newAbove = false;
          newBelow = true;
        } else if (above2 && !below2) {
          newAbove = true;
          newBelow = false;
        } else /*(above2 && below2)*/ {
          newAbove = true;
          newBelow = true;
        }
      }
    }
  };

  class InternalPoint
  {
  private:
    int _x;
    int _y;

  public:
    inline InternalPoint(const int x, const int y) : _x(x), _y(y)
    {
    }

    inline static bool IsEqual(
      const InternalPoint& p0,
      const InternalPoint& p1)
    {
      return (p0.GetX() == p1.GetX() && p0.GetY() == p1.GetY());
    }

    inline static int Compare(
      const InternalPoint& p0,
      const InternalPoint& p1)
    {
      if (p0._x < p1._x) {
        return -1;
      } else if (p0._x > p1._x) {
        return 1;
      } else {
        // p0._x == p1._x
        if (p0._y < p1._y) {
          return -1;
        } else if (p0._y > p1._y) {
          return 1;
        } else {
          return 0;
        }
      }
    }

    inline int GetX() const
    {
      return _x;
    }

    inline int GetY() const
    {
      return _y;
    }
  };

  struct InternalPointComparer {
    size_t operator()(const InternalPoint &x) const{
      return ((size_t)(x.GetX())) ^ ((size_t)(x.GetY()));
    }

    bool operator()(const InternalPoint &x, const InternalPoint &y) const{
      return InternalPoint::IsEqual(x, y);
    }
  };

  class InternalIntersectionPoint
  {
  private:
    Rational _x;
    Rational _y;

  public:
    InternalIntersectionPoint() : _x(0), _y(0)
    {
    }

    inline InternalIntersectionPoint(const Rational& x, const Rational& y) :
    _x(x), _y(y)
    {
    }

    inline InternalIntersectionPoint(const int x, const int y) :
    _x(Rational(x)), _y(Rational(y))
    {
    }

    explicit inline InternalIntersectionPoint(const InternalPoint &p) :
    _x(p.GetX()), _y(p.GetY())
    {
    }

    inline static bool IsEqual(
      const InternalIntersectionPoint &p0,
      const InternalIntersectionPoint &p1)
    {
      return p0.GetX() == p1.GetX() && p0.GetY() == p1.GetY();
    }

    inline static bool IsEqual(
      const InternalIntersectionPoint &p0,
      const InternalPoint &p1)
    {
      return p0.GetX() == p1.GetX() && p0.GetY() == p1.GetY();
    }

    inline const Rational& GetX() const
    {
      return _x;
    }

    inline const Rational& GetY() const
    {
      return _y;
    }
  };

  struct InternalIntersectionPointComparer {
    size_t operator()(const InternalIntersectionPoint &x) const
    {
      return
        ((size_t)(x.GetX().GetIntegerPart())) ^
        ((size_t)(x.GetY().GetIntegerPart()));
    }

    bool operator()(
      const InternalIntersectionPoint &x,
      const InternalIntersectionPoint &y) const
    {
      return InternalIntersectionPoint::IsEqual(x, y);
    }
  };

  struct InternalIntersectionPointLess {
    bool operator()(
      const InternalIntersectionPoint &x,
      const InternalIntersectionPoint &y) const
    {
      int result = Rational::Compare(x.GetX(), y.GetX());

      if (result == 0) {
        result = Rational::Compare(x.GetY(), y.GetY());
      }

      return result < 0;
    }
  };

  class InternalPointTransformation
  {
  private:
    long long _offsetX;
    long long _offsetY;
    int _scaleFactor;
    int _roundResultToDecimals;
    int _roundingStep;

  public:
    InternalPointTransformation(
      const long long offsetX,
      const long long offsetY,
      const int scaleFactor,
      const int roundResultToDecimals,
      const int roundingDecimalSteps);

    int RoundRational(const Rational& rat) const
    {
      return Rational::Round(rat, _roundingStep);
    }

    const InternalPoint RoundPoint(const InternalPoint& point) const
    {
      int newX = Rational::Round(Rational(point.GetX()), _roundingStep);
      int newY = Rational::Round(Rational(point.GetY()), _roundingStep);

      return InternalPoint(newX, newY);
    }

    const InternalPoint RoundPoint(
      const InternalIntersectionPoint& point) const
    {
      int newX = Rational::Round(point.GetX(), _roundingStep);
      int newY = Rational::Round(point.GetY(), _roundingStep);

      return InternalPoint(newX, newY);
    }

    const Point TransformToPoint(const InternalPoint& point) const
    {
      const InternalPoint& temp = RoundPoint(point);
      long long x = ((long long)temp.GetX()) + _offsetX;
      long long y = ((long long)temp.GetY()) + _offsetY;

      Point result(
        true,
        Utility::Round(x / (double)_scaleFactor, _roundResultToDecimals),
        Utility::Round(y / (double)_scaleFactor, _roundResultToDecimals));

      return result;
    }

    const Point TransformToPoint(const InternalIntersectionPoint& point) const
    {
      const InternalPoint& temp = RoundPoint(point);
      long long x = ((long long)temp.GetX()) + _offsetX;
      long long y = ((long long)temp.GetY()) + _offsetY;

      Point  result(
        true,
        Utility::Round(x / (double)_scaleFactor, _roundResultToDecimals),
        Utility::Round(y / (double)_scaleFactor, _roundResultToDecimals));

      return result;
    }

    int TransformToInternalX(const Point& point) const
    {
      int intX = (int)(((long long)
        Utility::Round(point.GetX() * _scaleFactor, 0)) - _offsetX);

      return intX;
    }

    const InternalPoint TransformToInternalPoint(const Point& point) const
    {
      int intX = (int)(((long long)
        Utility::Round(point.GetX() * _scaleFactor, 0)) - _offsetX);

      int intY = (int)(((long long)
        Utility::Round(point.GetY() * _scaleFactor, 0)) - _offsetY);

      return InternalPoint(intX, intY);
    }

    int GetScaleFactor() const
    {
      return _scaleFactor;
    }

    int GetRoundResultToDecimals() const
    {
      return _roundResultToDecimals;
    }

    int GetMinimalRoundedStep() const
    {
      return _roundingStep;
    }
  };

  class InternalResultLineSegment
  {
  private:
    AttrType _attr;
    InternalPoint _originalStart;
    InternalPoint _start;
    InternalPoint _originalEnd;
    InternalPoint _end;

    InternalAttribute _internalAttribute;

  public:
    inline InternalResultLineSegment(
      const AttrType& attr,
      const InternalPoint& originalStart,
      const InternalPoint& start,
      const InternalPoint& originalEnd,
      const InternalPoint& end,
      const InternalAttribute& internalAttribute,
      bool correctAttribute)
      : _attr(attr),
      _originalStart(originalStart),
      _start(start),
      _originalEnd(originalEnd),
      _end(end),
      _internalAttribute(internalAttribute)
    {
      if (correctAttribute &&
          start.GetX() == end.GetX() &&
          internalAttribute.IsDefined()) {
        int dx, dy;
        dx = _originalEnd.GetX() - _originalStart.GetX();
        dy = _originalEnd.GetY() - _originalStart.GetY();

        // segment is vertical after rounding, but was not so before ->
        // adjust attributes if necessary
        if ((dx > 0 && dy < 0) || (dx < 0 && dy > 0)) {
          _internalAttribute = InternalAttribute(
            _internalAttribute.IsFirstBorder(),
            _internalAttribute.IsFirstBelow(),
            _internalAttribute.IsFirstAbove(),
            _internalAttribute.IsSecondBorder(),
            _internalAttribute.IsSecondBelow(),
            _internalAttribute.IsSecondAbove());
        }
      }
    }

    inline const AttrType& GetAttr() const
    {
      return _attr;
    }

    inline const InternalPoint& GetStart() const
    {
      return _start;
    }

    inline const InternalPoint& GetOriginalStart() const
    {
      return _originalStart;
    }

    inline const InternalPoint& GetEnd()  const
    {
      return _end;
    }

    inline const InternalPoint& GetOriginalEnd()  const
    {
      return _originalEnd;
    }

    inline const InternalAttribute GetInternalAttribute() const
    {
      return _internalAttribute;
    }

    static InternalResultLineSegment MergeSegments(
      const InternalResultLineSegment& s0,
      const InternalResultLineSegment& s1);

    HalfSegment GetRealLineSegment(
      const InternalPointTransformation* transformation) const
    {
      HalfSegment h(
        true,
        transformation->TransformToPoint(_start),
        transformation->TransformToPoint(_end));
      return h;
    }
  };

  class InternalLineSegment
  {
  private:
    AttrType _attr;
    InternalPoint _left;
    InternalPoint _right;
    Rational _a;
    bool _isVertical;

    InternalIntersectionPoint _breakupStart;

    std::map<
      InternalIntersectionPoint,
      InternalAttribute,
      InternalIntersectionPointLess> *_intersections;

    InternalAttribute _initialAttribute;

    Rational _lastX;
    Rational _lastY;
    Rational _lastX2;
    Rational _lastY2;

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* _treeNode;

    InternalLineSegment(InternalLineSegment&) :
      _left(0, 0),
      _right(0, 0),
      _a(0),
      _breakupStart(0, 0),
      _intersections(NULL),
      _initialAttribute(),
      _lastX(std::numeric_limits<int>::min()),
      _lastY(std::numeric_limits<int>::min()),
      _lastX2(std::numeric_limits<int>::min()),
      _lastY2(std::numeric_limits<int>::min())
    {
    }

    static bool GetInitialInsideAbove(
      const Point& start,
      const Point& end,
      const InternalPoint& internalStart,
      const InternalPoint& internalEnd,
      bool insideAbove)
    {
      bool inputVertical = AlmostEqual(start.GetX(), end.GetX());
      bool internalVertical = (internalStart.GetX() == internalEnd.GetX());

      if (inputVertical == internalVertical) {
        return insideAbove;
      } else if (inputVertical) {
        // !internalVertical

        // not tested
        int dx = internalEnd.GetX() - internalStart.GetX();
        int dy = internalEnd.GetY() - internalStart.GetY();

        if ((dx > 0 && dy < 0) || (dx < 0 && dy > 0)) {
          return !insideAbove;
        } else {
          return insideAbove;
        }
      } else if (internalVertical) {
        double dx = end.GetX() - start.GetX();
        double dy = end.GetY() - start.GetY();

        if ((dx > 0 && dy < 0) || (dx < 0 && dy > 0)) {
          return !insideAbove;
        } else {
          return insideAbove;
        }
      } else {
        throw new std::logic_error("not possible");
      }
    }

  public:
    InternalLineSegment(
      const InternalPointTransformation &transformation,
      const HalfSegment &segment,
      const bool &belongsToSecondGeometry,
      const bool calculateCoverage) :
    _attr(segment.attr),
      _left(0, 0),
      _right(0, 0),
      _a(0),
      _breakupStart(0, 0),
      _intersections(NULL),
      _initialAttribute(),
      _lastX(std::numeric_limits<int>::min()),
      _lastY(std::numeric_limits<int>::min()),
      _lastX2(std::numeric_limits<int>::min()),
      _lastY2(std::numeric_limits<int>::min())
    {
      _treeNode = NULL;

      const InternalPoint start =
        transformation.TransformToInternalPoint(segment.GetLeftPoint());

      const InternalPoint end =
        transformation.TransformToInternalPoint(segment.GetRightPoint());

      bool isReverse = InternalPoint::Compare(start, end) > 0;

      if (!isReverse) {
        _left = start;
        _right = end;
      } else {
        _left = end;
        _right = start;
      }

      bool insideAbove = segment.attr.insideAbove;

      if (calculateCoverage) {
        insideAbove = GetInitialInsideAbove(
          segment.GetLeftPoint(), segment.GetRightPoint(),
          start, end,
          insideAbove);
      }

      _breakupStart = InternalIntersectionPoint(_left);
      _initialAttribute = InternalAttribute::Create(
        belongsToSecondGeometry,
        insideAbove,
        calculateCoverage);

      int dx = _right.GetX() - _left.GetX();
      int dy = _right.GetY() - _left.GetY();

      if (dx > 0) {
        _isVertical = false;
        _a = Rational(dy, dx);
      } else if (dx == 0) {
        _isVertical = true;
        _a = Rational(0, 1);
      } else {
        throw new std::invalid_argument("segment");
      }
    }

    ~InternalLineSegment()
    {
      if (_intersections != NULL) {
        delete _intersections;
        _intersections = NULL;
      }
    }

    inline bool IsStartEndPoint(
      const InternalIntersectionPoint &intersectionPoint) const
    {
      if (
        InternalIntersectionPoint::IsEqual(intersectionPoint, _left) ||
        InternalIntersectionPoint::IsEqual(intersectionPoint, _right)) {
          return true;
      } else {
        return false;
      }
    }

    void AddHobbyIntersection(
      int x,
      int y,
      int hobbySpacing);

    void AddIntersection(
      const InternalIntersectionPoint& intersectionPoint)
    {
      if (!IsStartEndPoint(intersectionPoint)) {
        if (_intersections == NULL) {
          _intersections = new std::map<
            InternalIntersectionPoint,
            InternalAttribute,
            InternalIntersectionPointLess>();
        }

        _intersections->insert(
          std::make_pair(intersectionPoint, InternalAttribute()));
      }
    }

    inline const InternalPoint& GetLeft() const
    {
      return _left;
    }

    inline const InternalPoint& GetRight() const
    {
      return _right;
    }

    const Rational& GetYValueAt(const InternalIntersectionPoint& p)
    {
      if (_isVertical) {
        return p.GetY();
      }

      if (_lastX == p.GetX()) {
        return _lastY;
      }

      if (_lastX2 == p.GetX()) {
        return _lastY2;
      }

      _lastX2 = _lastX;
      _lastY2 = _lastY;

      _lastX = p.GetX();
      _lastY = (p.GetX() - _left.GetX()) * _a + _left.GetY();
      return _lastY;
    }

    std::map<
      InternalIntersectionPoint,
      InternalAttribute,
      InternalIntersectionPointLess>*
      GetIntersections()
    {
      return _intersections;
    }

    void BreakupLines(
      const InternalPointTransformation &transformation,
      std::vector<InternalResultLineSegment>& target)
    {
      BreakupLines(transformation, target, false, 0);
    }

    void BreakupLines(
      const InternalPointTransformation &transformation,
      std::vector<InternalResultLineSegment>& target,
      bool observeMaxX,
      int maxX)
    {
      if (_intersections == NULL || _intersections->size() == 0) {
        if (observeMaxX) {
          return;
        }

        InternalPoint start = transformation.RoundPoint(_breakupStart);
        InternalPoint end = transformation.RoundPoint(_right);

        if (!InternalPoint::IsEqual(start, end)) {
          target.push_back(
            InternalResultLineSegment(
            _attr,
            _left,
            start,
            _right,
            end,
            _initialAttribute,
            true));
        }
        return;
      }

      bool newBreakupStartSet = false;

      InternalIntersectionPoint currentPosition = _breakupStart;
      InternalAttribute currentAttribute = _initialAttribute;
      InternalPoint currentPositionRounded =
        transformation.RoundPoint(currentPosition);

      std::map<
        InternalIntersectionPoint,
        InternalAttribute,
        InternalIntersectionPointLess> *newIntersections = NULL;

      for (std::map<
        InternalIntersectionPoint,
        InternalAttribute,
        InternalIntersectionPointLess>::const_iterator
        i = _intersections->begin();
      i != _intersections->end(); ++i) {
        InternalIntersectionPoint p = i->first;
        if (!observeMaxX || transformation.RoundRational(p.GetX()) <= maxX) {
          InternalPoint roundedP = transformation.RoundPoint(p);
          if (!InternalPoint::IsEqual(currentPositionRounded, roundedP)) {
            target.push_back(
              InternalResultLineSegment(
              _attr,
              _left,
              currentPositionRounded,
              _right,
              roundedP,
              currentAttribute,
              true));
            currentPosition = p;
            currentPositionRounded = roundedP;
          }
          if (i->second.IsDefined()) {
            currentAttribute = i->second;
          }
        } else {
          if (!newBreakupStartSet) {
            newBreakupStartSet = true;
            _breakupStart = currentPosition;
            _initialAttribute = currentAttribute;
          }

          if (newIntersections == NULL) {
            newIntersections = new std::map<
              InternalIntersectionPoint,
              InternalAttribute,
              InternalIntersectionPointLess>();
          }
          newIntersections->insert(*i);
        }
      }

      if (!observeMaxX) {
        InternalPoint roundedRight = transformation.RoundPoint(_right);
        if (!InternalPoint::IsEqual(currentPositionRounded, roundedRight)) {
          target.push_back(
            InternalResultLineSegment(
            _attr,
            _left,
            currentPositionRounded,
            _right,
            roundedRight,
            currentAttribute,
            true));
        }
      } else {
        if (!newBreakupStartSet) {
          _breakupStart = currentPosition;
          _initialAttribute = currentAttribute;
        }
        if (_intersections != NULL) {
          delete _intersections;
          _intersections = NULL;
        }
        _intersections = newIntersections;
      }
    }

    inline const Rational& GetA() const
    {
      if (_isVertical) {
        throw new std::logic_error("Vertical");
      }

      return _a;
    }

    inline bool GetIsVertical() const
    {
      return _isVertical;
    }

    inline const AttrType& GetAttr() const
    {
      return _attr;
    }

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* GetTreeNode() const
    {
      return _treeNode;
    }

    void SetTreeNode(
      AvlTreeNode<InternalLineSegment*, SweepStateData*>* treeNode)
    {
      if (_treeNode != NULL && treeNode != NULL && (_treeNode != treeNode)) {
        throw new std::invalid_argument("treeNode");
      }
      _treeNode = treeNode;
    }

    const InternalAttribute GetAttributeAt(
      const InternalIntersectionPoint& p,
      bool includeP) const
    {
      InternalAttribute result = _initialAttribute;

      if (_intersections != NULL) {
        for (std::map<
          InternalIntersectionPoint,
          InternalAttribute,
          InternalIntersectionPointLess>::const_iterator
          i = _intersections->begin();
        i != _intersections->end(); ++i) {
          if (i->second.IsDefined()) {
            if (!_isVertical) {
              if (i->first.GetX() < p.GetX() ||
                (includeP && i->first.GetX() == p.GetX())) {
                  result = i->second;
              } else {
                break;
              }
            } else {
              if (i->first.GetY() < p.GetY() ||
                (includeP && i->first.GetY() == p.GetY())) {
                  result = i->second;
              } else {
                break;
              }
            }
          }
        }
      }

      return result;
    }

    void AddAttributeEntry(
      const InternalIntersectionPoint& p,
      const InternalAttribute& attribute)
    {
      (*_intersections)[p] = attribute;
    }

    void SetAttributeNoAbove(
      const bool setInitialAttribute,
      const InternalIntersectionPoint& point)
    {
      InternalAttribute newAttribute;

      if (_initialAttribute.IsFirstBorder()) {
        newAttribute = InternalAttribute(
          _initialAttribute.IsFirstBorder(),
          _initialAttribute.IsFirstAbove(),
          _initialAttribute.IsFirstBelow(),
          false,
          false,
          false);
      } else if (_initialAttribute.IsSecondBorder()) {
        newAttribute = InternalAttribute(
          false,
          false,
          false,
          _initialAttribute.IsSecondBorder(),
          _initialAttribute.IsSecondAbove(),
          _initialAttribute.IsSecondBelow());
      } else {
        throw new std::logic_error(
          "unknown attribute owner (noAbove)");
      }

      if (newAttribute.IsBorderInBoth()) {
        throw new std::logic_error(
          "attribute must belong to exactly one geometry (noAbove)!");
      }

      if (setInitialAttribute) {
        _initialAttribute = newAttribute;
      } else {
        AddAttributeEntry(point, newAttribute);
      }
    }

    void SetAttribute(
      const InternalAttribute& attribute,
      const bool setInitialAttribute,
      const InternalIntersectionPoint& point)
    {
      InternalAttribute newAttribute;

      InternalAttribute currentAttribute = (setInitialAttribute
        ? _initialAttribute : GetAttributeAt(point, false));

      if (currentAttribute.IsFirstBorder()) {
        if (attribute.IsFirstBorder() && !attribute.IsSecondBorder()) {
          newAttribute = InternalAttribute(
            currentAttribute.IsFirstBorder(),
            currentAttribute.IsFirstAbove(),
            currentAttribute.IsFirstBelow(),
            attribute.IsSecondBorder(),
            attribute.IsSecondAbove(),
            attribute.IsSecondBelow());
        } else {
          newAttribute = InternalAttribute(
            currentAttribute.IsFirstBorder(),
            currentAttribute.IsFirstAbove(),
            currentAttribute.IsFirstBelow(),
            currentAttribute.IsSecondBorder(),
            attribute.IsSecondAbove(),
            attribute.IsSecondAbove());
        }
      } else if (currentAttribute.IsSecondBorder()) {
        if (attribute.IsSecondBorder() && !attribute.IsFirstBorder()) {
          newAttribute = InternalAttribute(
            attribute.IsFirstBorder(),
            attribute.IsFirstAbove(),
            attribute.IsFirstBelow(),
            currentAttribute.IsSecondBorder(),
            currentAttribute.IsSecondAbove(),
            currentAttribute.IsSecondBelow());
        } else {
          newAttribute = InternalAttribute(
            currentAttribute.IsFirstBorder(),
            attribute.IsFirstAbove(),
            attribute.IsFirstAbove(),
            currentAttribute.IsSecondBorder(),
            currentAttribute.IsSecondAbove(),
            currentAttribute.IsSecondBelow());
        }
      } else {
        throw new std::logic_error(
          "unknown attribute owner!");
      }

      if (newAttribute.IsBorderInBoth()) {
        throw new std::logic_error(
          "attribute must belong to exactly one geometry!");
      }

      if (setInitialAttribute) {
        _initialAttribute = newAttribute;
      } else {
        AddAttributeEntry(point, newAttribute);
      }
    }

    int static CompareSlope(InternalLineSegment* x, InternalLineSegment* y) {
      int result;

      if (x->GetIsVertical() && y->GetIsVertical()) {
        result = 0;
      } else if (x->GetIsVertical()) {
        result = 1;
      } else if (y->GetIsVertical()) {
        result = -1;
      } else {
        result = Rational::Compare(x->GetA(), y->GetA());
      }

      return result;
    }
  };
}
