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

  class InternalPoint
  {
  private:
    int _x;
    int _y;

  public:
    inline InternalPoint(const int x, const int y) : _x(x),_y(y)
    {
    }

    inline static bool IsEqual(const InternalPoint& p0,const InternalPoint& p1)
    {
      return (p0.GetX() == p1.GetX() && p0.GetY() == p1.GetY());
    }

    inline static int Compare(const InternalPoint& p0, const InternalPoint& p1)
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

    bool operator()(const InternalPoint &x,const InternalPoint &y) const{
      return InternalPoint::IsEqual(x,y);
    }
  };

  class InternalIntersectionPoint
  {
  private:
    Rational _x;
    Rational _y;

  public:
    InternalIntersectionPoint() : _x(0),_y(0)
    {
    }

    inline InternalIntersectionPoint(const Rational& x, const Rational& y) : 
    _x(x),_y(y)
    {
    }

    inline InternalIntersectionPoint(const int x, const int y) : 
    _x(Rational(x)),_y(Rational(y))
    {
    }

    inline InternalIntersectionPoint(const InternalPoint &p) : 
    _x(p.GetX()),_y(p.GetY())
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
      return InternalIntersectionPoint::IsEqual(x,y);
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

    InternalPointTransformation(InternalPointTransformation&) 
    {
    }

  public:
    InternalPointTransformation(
      const long long offsetX,
      const long long offsetY,
      const int scaleFactor,
      const int roundResultToDecimals);

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

      Point  result (
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
    InternalPoint _start;
    InternalPoint _end;

  public:
    inline InternalResultLineSegment(
      const AttrType& attr,
      const InternalPoint& start,
      const InternalPoint& end) : _attr(attr),_start(start),_end(end) 
    {
    }

    inline const AttrType& GetAttr() const  
    { 
      return _attr; 
    }

    inline const InternalPoint& GetStart() const 
    { 
      return _start; 
    }

    inline const InternalPoint& GetEnd()  const  
    {
      return _end; 
    }

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
    //int _index;
    InternalPoint _left;
    InternalPoint _right;
    Rational _a;
    bool _isVertical;

    InternalPoint _breakupStart;
    InternalPoint _roundedRight;
    std::unordered_set<
      InternalPoint,
      InternalPointComparer,
      InternalPointComparer> *_intersections;

    Rational _lastX;
    Rational _lastY;
    Rational _lastX2;
    Rational _lastY2;

    AvlTreeNode<InternalLineSegment*, SweepStateData*>* _treeNode;

    InternalLineSegment(InternalLineSegment&) : 
      _left(0,0),
      _right(0,0),
      _a(0),
      _breakupStart(0,0),
      _roundedRight(0,0),
      _lastX(std::numeric_limits<int>::min()),
      _lastY(std::numeric_limits<int>::min()),
      _lastX2(std::numeric_limits<int>::min()),
      _lastY2(std::numeric_limits<int>::min())
    {
    }

    static bool PointSortCompareDown(
      const InternalPoint& p0,
      const InternalPoint& p1)
    {
      if(p0.GetX()<p1.GetX()) {
        return true;
      } else if(p0.GetX()==p1.GetX()){
        return p0.GetY()<p1.GetY();
      } else {
        return false;
      }
    }

    static bool PointSortCompareUp(
      const InternalPoint& p0,
      const InternalPoint& p1)
    {
      if(p0.GetX()<p1.GetX()) {
        return true;
      } else if(p0.GetX()==p1.GetX()){
        return p0.GetY()>p1.GetY();
      } else {
        return false;
      }
    }

  public:
    InternalLineSegment(
      const InternalPointTransformation &transformation,
      const HalfSegment &segment) : 
    _attr(segment.attr),
      _left(0,0),
      _right(0,0),
      _a(0),
      _breakupStart(0,0),
      _roundedRight(0,0),
      _intersections(NULL),
      _lastX(std::numeric_limits<int>::min()),
      _lastY(std::numeric_limits<int>::min()),
      _lastX2(std::numeric_limits<int>::min()),
      _lastY2(std::numeric_limits<int>::min())
    {
      _treeNode=NULL;
      //_index = segment.GetIndex();

      const InternalPoint start = 
        transformation.TransformToInternalPoint(segment.GetLeftPoint());

      const InternalPoint end = 
        transformation.TransformToInternalPoint(segment.GetRightPoint());

      bool isReverse = InternalPoint::Compare(start,end)>0;

      if (!isReverse) {
        _left=start;
        _right=end;
      } else {
        _left=end;
        _right=start;
      }

      _breakupStart = transformation.RoundPoint(_left);
      _roundedRight = transformation.RoundPoint(_right);

      int dx = _right.GetX() - _left.GetX();
      int dy = _right.GetY() - _left.GetY();

      if (dx > 0) {
        _isVertical=false;
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
      if(_intersections!=NULL) {
        delete _intersections;
        _intersections=NULL;
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

    void AddIntersection(
      const InternalPointTransformation *transformation, 
      const InternalIntersectionPoint& intersectionPoint)
    {
      if (!IsStartEndPoint(intersectionPoint)) {

        if(_intersections==NULL) {
          _intersections = new std::unordered_set<
            InternalPoint,
            InternalPointComparer,
            InternalPointComparer>();
        }

        InternalPoint roundedPoint = 
          transformation->RoundPoint(intersectionPoint);

        _intersections->insert(roundedPoint);
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

    std::unordered_set<
      InternalPoint,
      InternalPointComparer,
      InternalPointComparer>* GetIntersections()
    {
      return _intersections;
    }

    void BreakupLines(std::vector<InternalResultLineSegment>& target)
    {
      BreakupLines(target,false,0);
    }

    void BreakupLines(
      std::vector<InternalResultLineSegment>& target, 
      bool observeMaxX, 
      int maxX)
    {
      if(_intersections==NULL || _intersections->size()==0) {
        if(observeMaxX) {
          return;
        }

        if(!InternalPoint::IsEqual(_breakupStart,_roundedRight)) {
          target.push_back(
            InternalResultLineSegment(_attr,_breakupStart,_roundedRight));
        }
        return;
      }

      std::vector<InternalPoint>* newIntersections=NULL;

      std::vector<InternalPoint> ip;
      for(std::unordered_set<
        InternalPoint,
        InternalPointComparer,
        InternalPointComparer>::const_iterator 
        i=_intersections->begin();
        i!=_intersections->end();++i) {

          if (!observeMaxX || i->GetX() <= maxX) {
          ip.push_back(*i);
        } else {
          if(newIntersections==NULL) {
            newIntersections=new std::vector<InternalPoint>();
          }
          newIntersections->push_back(*i);
        }
      }

      if(ip.empty()){
        if(newIntersections!=NULL) {
          delete(newIntersections);
        }
        return;
      }

      ip.push_back(_breakupStart);
      if(!observeMaxX){
        ip.push_back(_roundedRight);
      }

      if (_isVertical || _a > 0) {
        std::sort(ip.begin(),ip.end(),PointSortCompareDown);
      } else {
        std::sort(ip.begin(),ip.end(),PointSortCompareUp);
      }

      InternalPoint currentPosition=ip[0];
      for(std::vector<InternalPoint>::const_iterator 
        i=(ip.begin())++;
        i!=ip.end();++i) {

          if (!InternalPoint::IsEqual(currentPosition, *i)) {
          target.push_back(
            InternalResultLineSegment(_attr, currentPosition, *i));
          currentPosition = *i;
        }
      }

      _breakupStart = ip[ip.size() - 1];
      _intersections->clear();
      if (newIntersections != NULL) {
        _intersections->insert(
          newIntersections->begin(),
          newIntersections->end());
        
        delete newIntersections;
        newIntersections=NULL;
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
      if (_treeNode != NULL && treeNode != NULL && (_treeNode!=treeNode)) {
        throw new std::invalid_argument("treeNode");
      }
      _treeNode = treeNode;
    }
  };
}
