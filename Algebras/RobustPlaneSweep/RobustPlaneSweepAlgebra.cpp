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

[1] Implementation file for the class ~RobustPlaneSweepAlgebra~

[TOC]

1 Overview

This file contains all structs and classes required for the 
RobustPlaneSweepAlgebra.

1 Includes and defines

*/

// #define OUTPUT_HALFSEGMENTS
#include "RobustPlaneSweepAlgebra.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "DLine.h"

#include "Algorithm/Hobby.h"

namespace RobustPlaneSweep
{
/*

1 Data classes

1.1 Data class for operations with two HalfSegment-Collections (Line or Region)

*/
template<class TINPUT1, class TINPUT2>
class HalfSegmentHalfSegmentData :
    public IntersectionAlgorithmData
{
/*

1.1.1 Member variables

*/
private:
  const TINPUT1& _source1;
  int _source1Index;
  int _source1Size;
  HalfSegment _source1segment;

  const TINPUT2& _source2;
  int _source2Index;
  int _source2Size;
  HalfSegment _source2segment;

public:
/*

1.1.1 Constructor

*/
  HalfSegmentHalfSegmentData(const TINPUT1& source1,
                             const TINPUT2& source2) :
      _source1(source1),
      _source1Index(0),
      _source1Size(-1),
      _source2(source2),
      _source2Index(0),
      _source2Size(-1)
  {
#ifdef OUTPUT_HALFSEGMENTS
    cout.precision(15);
#endif
  }

/*

1.1.1 Destructor

*/
  ~HalfSegmentHalfSegmentData()
  {
  }

/*

1.1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
#ifdef OUTPUT_HALFSEGMENTS
    cout << "\n";
#endif
    _source1Index = 0;
    _source1Size = _source1.Size();
    if (_source1Index < _source1Size) {
      _source1.Get(_source1Index, _source1segment);
    }

    _source2Index = 0;
    _source2Size = _source2.Size();
    if (_source2Index < _source2Size) {
      _source2.Get(_source2Index, _source2segment);
    }
  }

/*

1.1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point &point,
                  bool &belongsToSecondGeometry)
  {
    point.SetDefined(false);
    if (_source1Index < _source1Size && _source2Index < _source2Size) {
      if (_source1segment.Compare(_source2segment) < 0) {
        belongsToSecondGeometry = false;
      } else {
        belongsToSecondGeometry = true;
      }
    } else if (_source1Index < _source1Size) {
      belongsToSecondGeometry = false;
    } else if (_source2Index < _source2Size) {
      belongsToSecondGeometry = true;
    } else {
      return false;
    }

    if (!belongsToSecondGeometry) {
      segment = _source1segment;
      if ((++_source1Index) < _source1Size) {
        _source1.Get(_source1Index, _source1segment);
      }
    } else {
      segment = _source2segment;
      if ((++_source2Index) < _source2Size) {
        _source2.Get(_source2Index, _source2segment);
      }
    }

#ifdef OUTPUT_HALFSEGMENTS
    if (!segment.IsLeftDomPoint()) {
      cout << "// ";
    }

    cout << "s.Add(new LineSegment(" <<
    (belongsToSecondGeometry?"1":"0") << ", " <<
    (segment.attr.edgeno) << ", " <<
    " new Base.Point(" <<
    segment.GetLeftPoint().GetX() << "," <<
    segment.GetLeftPoint().GetY() << "), " <<
    " new Base.Point(" <<
    segment.GetRightPoint().GetX() << "," <<
    segment.GetRightPoint().GetY() << "), " <<
    (segment.attr.insideAbove?"true":"false") <<
    "));\n";
#endif

    return true;
  }

/*

1.1.1 ~GetBoundingBox~

*/
  const Rectangle<2> GetBoundingBox()
  {
    return _source1.BoundingBox().Union(_source2.BoundingBox());
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return std::is_same<TINPUT1, Region>::value;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return std::is_same<TINPUT2, Region>::value;
  }
};

/*

1.1 Data class for operations with HalfSegment-Collection and Point(s)

*/
template<class THALFSEGMENT, class TPOINTS>
class HalfSegmentPointsData :
    public IntersectionAlgorithmData
{
};

/*

1.1 Template specialization class for operations with 
HalfSegment-Collection x Points

*/
template<class THALFSEGMENT>
class HalfSegmentPointsData<THALFSEGMENT, Points> :
    public IntersectionAlgorithmData
{
/*

1.1.1 Member variables

*/
private:
  const THALFSEGMENT& _source1;
  int _source1Index;
  int _source1Size;
  HalfSegment _source1segment;

  const Points& _source2;
  int _source2Index;
  int _source2Size;
  Point _source2Point;

public:
/*

1.1.1 Constructor

*/
  HalfSegmentPointsData(const THALFSEGMENT& source1,
                        const Points& source2) :
      _source1(source1),
      _source1Index(0),
      _source1Size(-1),
      _source2(source2),
      _source2Index(0),
      _source2Size(-1)
  {
  }

/*

1.1.1 Destructor

*/
  ~HalfSegmentPointsData()
  {
  }

/*

1.1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
    _source1Index = 0;
    _source1Size = _source1.Size();
    if (_source1Index < _source1Size) {
      _source1.Get(_source1Index, _source1segment);
    }

    _source2Index = 0;
    _source2Size = _source2.Size();
    if (_source2Index < _source2Size) {
      _source2.Get(_source2Index, _source2Point);
    }
  }

/*

1.1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point &point,
                  bool &belongsToSecondGeometry)
  {
    if (_source1Index < _source1Size && _source2Index < _source2Size) {
      if (_source1segment.GetDomPoint() < _source2Point) {
        belongsToSecondGeometry = false;
      } else {
        belongsToSecondGeometry = true;
      }
    } else if (_source1Index < _source1Size) {
      belongsToSecondGeometry = false;
    } else if (_source2Index < _source2Size) {
      belongsToSecondGeometry = true;
    } else {
      return false;
    }

    if (!belongsToSecondGeometry) {
      point.SetDefined(false);
      segment = _source1segment;
      if ((++_source1Index) < _source1Size) {
        _source1.Get(_source1Index, _source1segment);
      }
#ifdef OUTPUT_HALFSEGMENTS
      if (!segment.IsLeftDomPoint()) {
        cout << "// ";
      }

      cout << "s.Add(new LineSegment(" <<
      (belongsToSecondGeometry?"1":"0") << ", " <<
      (segment.attr.edgeno) << ", " <<
      " new Base.Point(" <<
      segment.GetLeftPoint().GetX() << "," <<
      segment.GetLeftPoint().GetY() << "), " <<
      " new Base.Point(" <<
      segment.GetRightPoint().GetX() << "," <<
      segment.GetRightPoint().GetY() << "), " <<
      (segment.attr.insideAbove?"true":"false") <<
      "));\n";
#endif
    } else {
      point = _source2Point;
      if ((++_source2Index) < _source2Size) {
        _source2.Get(_source2Index, _source2Point);
      }
#ifdef OUTPUT_HALFSEGMENTS
      cout << "s.Add(new LineSegment(" <<
      (belongsToSecondGeometry?"1":"0") << ", " <<
      "-1, " <<
      " new Base.Point(" <<
      point.GetX() << "," <<
      point.GetY() << "), " <<
      " new Base.Point(" <<
      point.GetX() << "," <<
      point.GetY() << "), " <<
      "false" <<
      "));\n";
#endif
    }

    return true;
  }

/*

1.1.1 ~GetBoundingBox~

*/
  const Rectangle<2> GetBoundingBox()
  {
    return _source1.BoundingBox().Union(_source2.BoundingBox());
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return std::is_same<THALFSEGMENT, Region>::value;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }
};

/*

1.1 Template specialization class for operations with 
 HalfSegment-Collection x Point

*/
template<class THALFSEGMENT>
class HalfSegmentPointsData<THALFSEGMENT, Point> :
    public IntersectionAlgorithmData
{
/*

1.1.1 Member variables

*/
private:
  const THALFSEGMENT& _source1;
  int _source1Index;
  int _source1Size;
  HalfSegment _source1segment;

  const Point& _source2Point;
  bool _source2PointRead;

public:
/*

1.1.1 Constructor

*/
  HalfSegmentPointsData(const THALFSEGMENT& source1,
                        const Point& source2) :
      _source1(source1),
      _source1Index(0),
      _source1Size(-1),
      _source2Point(source2),
      _source2PointRead(false)
  {
  }

/*

1.1.1 Destructor

*/
  ~HalfSegmentPointsData()
  {
  }

/*

1.1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
    _source1Index = 0;
    _source1Size = _source1.Size();
    if (_source1Index < _source1Size) {
      _source1.Get(_source1Index, _source1segment);
    }

    _source2PointRead = false;
  }

/*

1.1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point &point,
                  bool &belongsToSecondGeometry)
  {
    if (_source1Index < _source1Size && !_source2PointRead) {
      if (_source1segment.GetDomPoint() < _source2Point) {
        belongsToSecondGeometry = false;
      } else {
        belongsToSecondGeometry = true;
      }
    } else if (_source1Index < _source1Size) {
      belongsToSecondGeometry = false;
    } else if (!_source2PointRead) {
      belongsToSecondGeometry = true;
    } else {
      return false;
    }

    if (!belongsToSecondGeometry) {
      point.SetDefined(false);
      segment = _source1segment;
      if ((++_source1Index) < _source1Size) {
        _source1.Get(_source1Index, _source1segment);
      }

#ifdef OUTPUT_HALFSEGMENTS
      if (!segment.IsLeftDomPoint()) {
        cout << "// ";
      }

      cout << "s.Add(new LineSegment(" <<
      (belongsToSecondGeometry?"1":"0") << ", " <<
      (segment.attr.edgeno) << ", " <<
      " new Base.Point(" <<
      segment.GetLeftPoint().GetX() << "," <<
      segment.GetLeftPoint().GetY() << "), " <<
      " new Base.Point(" <<
      segment.GetRightPoint().GetX() << "," <<
      segment.GetRightPoint().GetY() << "), " <<
      (segment.attr.insideAbove?"true":"false") <<
      "));\n";
#endif

    } else {
      point = _source2Point;
      _source2PointRead = true;

#ifdef OUTPUT_HALFSEGMENTS
      cout << "s.Add(new LineSegment(" <<
      (belongsToSecondGeometry?"1":"0") << ", " <<
      "-1, " <<
      " new Base.Point(" <<
      point.GetX() << "," <<
      point.GetY() << "), " <<
      " new Base.Point(" <<
      point.GetX() << "," <<
      point.GetY() << "), " <<
      "false" <<
      "));\n";
#endif
    }

    return true;
  }

/*

1.1.1 ~GetBoundingBox~

*/
  const Rectangle<2> GetBoundingBox()
  {
    return _source1.BoundingBox().Union(_source2Point.BoundingBox());
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return std::is_same<THALFSEGMENT, Region>::value;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }
};

/*

1.1 Enum class for set operation types

*/
enum class SetOpType
{
  Union = 1,
  Intersection = 2,
  Minus = 3
};

/*

1.1 Data class for line set operations

*/
class LineLineSetOp :
    public HalfSegmentHalfSegmentData<Line, Line>
{
/*

1.1.1 Member variables

*/
private:
  SetOpType _setOpType;

  Line& _result;
  int _outputSegments;

public:
/*

1.1.1 Constructor

*/
  LineLineSetOp(const Line& source1,
                const Line& source2,
                SetOpType setOpType,
                Line& result) :
      HalfSegmentHalfSegmentData<Line, Line>(source1, source2),
      _setOpType(setOpType),
      _result(result),
      _outputSegments(0)
  {
  }

/*

1.1.1 Destructor

*/
  ~LineLineSetOp()
  {
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
    switch (_setOpType) {
      case SetOpType::Union:
        break;

      case SetOpType::Intersection:
        if (!attribute.IsBoundaryInBoth()) {
          return;
        }
        break;

      case SetOpType::Minus:
        if (attribute.IsBoundaryInSecond()) {
          return;
        }
        break;

      default:
        throw new std::logic_error("setOpType");
    }

    // cout << "O:" << _outputSegments << "\n";
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s2.attr.edgeno = _outputSegments;
    _result += s1;
    _result += s2;
    _outputSegments++;
  }

/*

1.1.1 ~SetOp~

*/
  static void SetOp(const Line& line1,
                    const Line& line2,
                    SetOpType setOpType,
                    Line& result)
  {
    result.StartBulkLoad();
    LineLineSetOp data(line1, line2, setOpType, result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();
    result.EndBulkLoad(true, false);
  }
};

/*

1.1 Data class for region set operations

*/
class RegionRegionSetOp :
    public HalfSegmentHalfSegmentData<Region, Region>
{
/*

1.1.1 Member variables

*/
private:
  SetOpType _setOpType;

  Region& _result;
  int _outputSegments;

public:
/*

1.1.1 Constructor

*/
  RegionRegionSetOp(const Region& source1,
                    const Region& source2,
                    SetOpType setOpType,
                    Region& result) :
      HalfSegmentHalfSegmentData<Region, Region>(source1, source2),
      _setOpType(setOpType),
      _result(result),
      _outputSegments(0)
  {
  }

/*

1.1.1 Destructor

*/
  ~RegionRegionSetOp()
  {
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
    bool include;
    bool insideAbove;

    switch (_setOpType) {
      case SetOpType::Union:
        include = attribute.IsInUnionRegion(insideAbove);
        break;

      case SetOpType::Intersection:
        include = attribute.IsInIntersectionRegion(insideAbove);
        break;

      case SetOpType::Minus:
        include = attribute.IsInMinusRegion(insideAbove);
        break;

      default:
        throw new std::logic_error("setOpType");
    }

#ifdef OUTPUT_HALFSEGMENTS
    cout << "O: " <<
    _outputSegments << " (" <<
    segment.GetLeftPoint().GetX() << "," <<
    segment.GetLeftPoint().GetY() << ") - (" <<
    segment.GetRightPoint().GetX() << "," <<
    segment.GetRightPoint().GetY() << ") (" <<
    (attribute.IsFirstAbove()?"A":"_") <<
    (attribute.IsFirstBelow()?"B":"_") << "/" <<
    (attribute.IsSecondAbove()?"A":"_") <<
    (attribute.IsSecondBelow()?"B":"_") << ")" <<
    (include?" Inc ":"  -  ") <<
    (insideAbove?" Above ":" Below ") <<
    "\n";
#endif

    if (!include) {
      return;
    }

    // cout << "O:" << _outputSegments << "\n";
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s1.attr.insideAbove = insideAbove;
    s2.attr.edgeno = _outputSegments;
    s2.attr.insideAbove = insideAbove;
    _result += s1;
    _result += s2;
    _outputSegments++;
  }

/*

1.1.1 ~SetOp~

*/
  static void SetOp(const Region& region1,
                    const Region& region2,
                    SetOpType setOpType,
                    Region& result)
  {
    result.StartBulkLoad();
    RegionRegionSetOp data(region1, region2, setOpType, result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();
    result.EndBulkLoad();
  }
};

/*

1.1 Data class for region/line set operations

*/
class RegionLineSetOp :
    public HalfSegmentHalfSegmentData<Region, Line>
{
/*

1.1.1 Member variables

*/
private:
  SetOpType _setOpType;

  Line& _result;
  int _outputSegments;

public:
/*

1.1.1 Constructor

*/
  RegionLineSetOp(const Region& source1,
                  const Line& source2,
                  SetOpType setOpType,
                  Line& result) :
      HalfSegmentHalfSegmentData<Region, Line>(source1, source2),
      _setOpType(setOpType),
      _result(result),
      _outputSegments(0)
  {
  }

/*

1.1.1 Destructor

*/
  ~RegionLineSetOp()
  {
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
    bool include;

    switch (_setOpType) {
      case SetOpType::Intersection:
        include = (attribute.GetFirst() != BoundaryType::OutsideInterior &&
                   attribute.IsBoundaryInSecond());
        break;

      case SetOpType::Minus:
        include = (attribute.GetFirst() == BoundaryType::OutsideInterior &&
                   attribute.IsBoundaryInSecond());
        break;

      default:
        throw new std::logic_error("setOpType");
    }

    if (!include) {
      return;
    }

    // cout << "O:" << _outputSegments << "\n";
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s2.attr.edgeno = _outputSegments;
    _result += s1;
    _result += s2;
    _outputSegments++;
  }

/*

1.1.1 ~SetOp~

*/
  static void SetOp(const Region& region,
                    const Line& line,
                    SetOpType setOpType,
                    Line& result)
  {
    // set result to a highly estimated value to avoid
    // frequent enlargements of the underlying DbArray
    result.Resize(line.Size());
    result.StartBulkLoad();
    RegionLineSetOp data(region, line, setOpType, result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();
    result.EndBulkLoad();
  }
};

/*

1.1 Data class for {region, line} x {region, line} intersects operator

*/
template<class TINPUT1, class TINPUT2>
class RegionLineIntersects :
    public HalfSegmentHalfSegmentData<TINPUT1, TINPUT2>
{
/*

1.1.1 Member variables

*/
private:
  bool _intersects;

public:
/*

1.1.1 Constructor

*/
  RegionLineIntersects(const TINPUT1& source1,
                       const TINPUT2& source2) :
      HalfSegmentHalfSegmentData<TINPUT1, TINPUT2>(source1, source2),
      _intersects(false)
  {
  }

/*

1.1.1 Destructor

*/
  ~RegionLineIntersects()
  {
  }

/*

1.1.1 ~OnGeometryIntersectionFound~

*/
  bool OnGeometryIntersectionFound()
  {
    _intersects = true;
    return true;
  }

/*

1.1.1 ~Intersects~

*/
  static bool Intersects(const TINPUT1& source1,
                         const TINPUT2& source2)
  {
    RegionLineIntersects<TINPUT1, TINPUT2> data(source1, source2);
    BentleyOttmann bentleyOttmann(&data);
    bentleyOttmann.DetermineIntersections();

    return data._intersects;
  }
};

/*

1.1 Data class for {region, line} x points set operations

*/
template<class THALFSEGMENT, class TPOINTS>
class RegionLinePointsSetOp :
    public HalfSegmentPointsData<THALFSEGMENT, TPOINTS>
{
/*

1.1.1 Member Variables

*/
private:
  SetOpType _setOpType;
  Points& _result;

public:
/*

1.1.1 Constructor

*/
  RegionLinePointsSetOp(const THALFSEGMENT& source1,
                        const TPOINTS& source2,
                        SetOpType setOpType,
                        Points& result) :
      HalfSegmentPointsData<THALFSEGMENT, TPOINTS>(source1, source2),
      _setOpType(setOpType),
      _result(result)
  {
  }

/*

1.1.1 Destructor

*/
  ~RegionLinePointsSetOp()
  {
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
  }

/*

1.1.1 ~OutputPoint~

*/
  void OutputPoint(const Point& point,
                   const InternalAttribute& attribute)
  {
    bool include;

    switch (_setOpType) {
      case SetOpType::Intersection:
        include = (attribute.GetFirst() != BoundaryType::OutsideInterior);
        break;

      case SetOpType::Minus:
        include = (attribute.GetFirst() == BoundaryType::OutsideInterior);
        break;

      default:
        throw new std::logic_error("setOpType");
    }

    if (!include) {
      return;
    }

    _result += point;
  }

/*

1.1.1 ~SetOp~

*/
  static void SetOp(const THALFSEGMENT& source,
                    const TPOINTS& points,
                    SetOpType setOpType,
                    Points& result)
  {
    result.StartBulkLoad();
    RegionLinePointsSetOp data(source, points, setOpType, result);
    BentleyOttmann bentleyOttmann(&data);
    bentleyOttmann.DetermineIntersections();
    result.EndBulkLoad();
  }
};

/*

1.1 Data class for {region, line} x {point, points} intersects operator

*/
template<class THALFSEGMENT, class TPOINTS>
class RegionLinePointsIntersects :
    public HalfSegmentPointsData<THALFSEGMENT, TPOINTS>
{
/*

1.1.1 Member variables

*/
private:
  bool _intersects;

public:
/*

1.1.1 Constructor

*/
  RegionLinePointsIntersects(const THALFSEGMENT& source1,
                             const Points& source2) :
      HalfSegmentPointsData<THALFSEGMENT, TPOINTS>(source1, source2),
      _intersects(false)
  {
  }

/*

1.1.1 Destructor

*/
  ~RegionLinePointsIntersects()
  {
  }

/*

1.1.1 ~OnGeometryIntersectionFound~

*/
  bool OnGeometryIntersectionFound()
  {
    _intersects = true;
    return true;
  }

/*

1.1.1 ~Intersects~

*/
  static bool Intersects(const THALFSEGMENT& source,
                         const Points& points)
  {
    RegionLinePointsIntersects data(source, points);
    BentleyOttmann bentleyOttmann(&data);
    bentleyOttmann.DetermineIntersections();

    return data._intersects;
  }
};

/*

1.1 Data class for crossing operator

*/
class CrossingsData :
    public HalfSegmentHalfSegmentData<Line, Line>
{
private:
/*

1.1.1 Internal Point comparer struct.

The coordinates were integer values and the transformation should be 
deterministic, so there is no need for AlmostEqual.

*/
  struct PointComparer
  {
    size_t operator()(const Point &x) const
    {
      return ((size_t)((int)x.GetX())) ^ ((size_t)((int)x.GetY()));
    }

    bool operator()(const Point &x, const Point &y) const
    {
      return (x.GetX() == y.GetX()) && (x.GetY() == y.GetY());
    }
  };

/*

1.1.1 Member Variables

*/
  std::unordered_set<Point, PointComparer> _overlappingPoints;
  std::vector<Point> _possibleCrossings;

  Points& _result;

public:
/*

1.1.1 Constructor

*/
  CrossingsData(const Line& source1,
                const Line& source2,
                Points& result) :
      HalfSegmentHalfSegmentData<Line, Line>(source1, source2),
      _result(result)
  {
  }

/*

1.1.1 Destructor

*/
  ~CrossingsData()
  {
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~ReportIntersections~

*/
  bool ReportIntersections() const
  {
    return true;
  }

/*

1.1.1 ~ReportIntersection~

*/
  void ReportIntersection(const Point& intersectionPoint,
                          const bool overlappingIntersection)
  {
    if (overlappingIntersection) {
      _overlappingPoints.insert(intersectionPoint);
    } else {
      _possibleCrossings.push_back(intersectionPoint);
    }
  }

/*

1.1.1 ~OutputFinished~

*/
  virtual void OutputFinished()
  {
    for (std::vector<Point>::const_iterator i = _possibleCrossings.begin();
        i != _possibleCrossings.end(); ++i) {
      if (_overlappingPoints.find (*i) == _overlappingPoints.end()) {
        _result += *i;
      }
    }
  }

/*

1.1.1 ~Crossings~

*/
  static void Crossings(const Line& line1,
                        const Line& line2,
                        Points& result)
  {
    result.Clear();
    if (!line1.IsDefined() || !line2.IsDefined()) {
      result.SetDefined(false);
      return;
    }

    result.SetDefined(true);
    if (line1.IsEmpty() || line2.IsEmpty()) {
      return;
    }

    // assert(line1.IsOrdered());
    // assert(line2.IsOrdered());

    result.StartBulkLoad();
    if (line1.BoundingBox().IntersectsUD(line2.BoundingBox())) {
      CrossingsData data(line1, line2, result);
      BentleyOttmann bo(&data);
      bo.DetermineIntersections();
    }

    result.EndBulkLoad(true, true);    // sort and remove duplicates
  }
};

/*

1.1 Data class for line minize

*/
class LineMinize : public IntersectionAlgorithmData
{
/*

1.1.1 Member Variables

*/
private:
  const Line& _source;
  int _currentSourceIndex;
  Line& _result;
  int _outputSegments;

public:
/*

1.1.1 Constructor

*/
  LineMinize(const Line& src, Line& result) :
      _source(src),
      _currentSourceIndex(0),
      _result(result),
      _outputSegments(0)
  {
  }

/*

1.1.1 Destructor

*/
  ~LineMinize()
  {
  }

/*

1.1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
    _currentSourceIndex = 0;
  }

/*

1.1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point& point,
                  bool &belongsToSecondGeometry)
  {
    point.SetDefined(false);
    if (_currentSourceIndex < _source.Size()) {
      _source.Get(_currentSourceIndex++, segment);
      belongsToSecondGeometry = false;
      return true;
    } else {
      return false;
    }
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s2.attr.edgeno = _outputSegments;
    _result += s1;
    _result += s2;
    _outputSegments++;
  }

/*

1.1.1 ~GetBoundingBox~

*/
  const Rectangle<2> GetBoundingBox()
  {
    return _source.BoundingBox();
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~IsInputOrderedByX~

*/
  bool IsInputOrderedByX() const
  {
    return false;
  }

/*

1.1.1 ~Realminize~

*/
  static void Realminize(const Line& src, Line& result)
  {
    result.Clear();
    if (!src.IsDefined()) {
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);
    if (src.Size() == 0) {    // empty line, nothing to realminize
      return;
    }

    result.StartBulkLoad();
    LineMinize data(src, result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();
    //  BentleyOttmann bo(&data);
    //  bo.DetermineIntersections();

    result.StartBulkLoad();    // ordered = true
    result.EndBulkLoad(true, false);
  }
};

/*

1.1 Data class for conversion from DLine to Line

*/
class DLineToLine : public IntersectionAlgorithmData
{
/*

1.1.1 Member Variables

*/
private:
  const DLine& _source;
  size_t _currentSourceIndex;
  Line& _result;
  int _outputSegments;

public:
/*

1.1.1 Constructor

*/
  DLineToLine(const DLine& src, Line& result) :
      _source(src),
      _currentSourceIndex(0),
      _result(result),
      _outputSegments(0)
  {
  }

/*

1.1.1 Destructor

*/
  ~DLineToLine()
  {
  }

/*

1.1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
    _currentSourceIndex = 0;
  }

/*

1.1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point &point,
                  bool &belongsToSecondGeometry)
  {
    point.SetDefined(false);
    // there is no Size()-method?
    while (_currentSourceIndex < _source.HashValue()) {
      SimpleSegment s;
      _source.get(_currentSourceIndex++, s);
      Point p1 = Point(true, s.x1, s.y1);
      Point p2 = Point(true, s.x2, s.y2);
      if (!AlmostEqual(p1, p2)) {
        segment = HalfSegment(true, p1, p2);
        belongsToSecondGeometry = false;
        return true;
      }
    }

    return false;
  }

/*

1.1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1.1 ~IsInputOrderedByX~

*/
  bool IsInputOrderedByX() const
  {
    return false;
  }

/*

1.1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& attribute)
  {
    // cout << "O:" << _outputSegments << "\n";
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s2.attr.edgeno = _outputSegments;
    _result += s1;
    _result += s2;
    _outputSegments++;
  }

/*

1.1.1 ~OutputPoint~

*/
  void OutputPoint(const Point& poiint,
                   const InternalAttribute& attribute)
  {
    throw new std::logic_error("there shouldn't be any points to output!");
  }

/*

1.1.1 ~GetBoundingBox~

*/
  const Rectangle<2> GetBoundingBox()
  {
    if (_source.HashValue() == 0) {
      return Rectangle<2>(false, 0, 0, 0, 0);
    }

    double minX = 1e300;
    double maxX = -1e300;
    double minY = 1e300;
    double maxY = -1e300;

    SimpleSegment segment;
    for (size_t i = 0; i < _source.HashValue(); i++) {
      _source.get(i, segment);

      if (segment.x1 < minX) {
        minX = segment.x1;
      }
      if (segment.y1 < minY) {
        minY = segment.y1;
      }
      if (segment.x2 < minX) {
        minX = segment.x2;
      }
      if (segment.y2 < minY) {
        minY = segment.y2;
      }

      if (segment.x1 > maxX) {
        maxX = segment.x1;
      }
      if (segment.y1 > maxY) {
        maxY = segment.y1;
      }
      if (segment.x2 > maxX) {
        maxX = segment.x2;
      }
      if (segment.y2 > maxY) {
        maxY = segment.y2;
      }
    }

    return Rectangle<2>(true, minX, maxX, minY, maxY);
  }

/*

1.1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }

/*

1.1.1 ~ToLine~

*/
  static void ToLine(const DLine& src, Line& result)
  {
    result.Clear();
    if (!src.IsDefined()) {
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);
    if (src.HashValue() == 0) {    // empty line, nothing to realminize
      return;
    }

    result.StartBulkLoad();
    DLineToLine data(src, result);
    Hobby hobby(&data);
    hobby.DetermineIntersections();

    result.StartBulkLoad();    // ordered = true
    result.EndBulkLoad(true, false);
  }
};

/*

1 Operators with strongly typed parameters

1.1 Trajectory2 operator

*/
void MPointTrajectory(MPoint* mpoint, Line& line)
{
  line.Clear();
  if (!mpoint->IsDefined()) {
    line.SetDefined(false);
    return;
  }
  line.SetDefined(true);
  line.StartBulkLoad();

  HalfSegment hs;
  UPoint unit;
  int edgeno = 0;

  int size = mpoint->GetNoComponents();
  if (size > 0)
    line.Resize(size);

  Point p0(false);    // starting point
  Point p1(false);    // end point of the first unit
  Point p_last(false);    // last point of the connected segment

  for (int i = 0; i < size; i++) {
    mpoint->Get(i, unit);

    if (!AlmostEqual(unit.p0, unit.p1)) {
      if (!p0.IsDefined()) {    // first unit
        p0 = unit.p0;
        p1 = unit.p1;
        p_last = unit.p1;
      } else {    // segment already exists
        if (p_last != unit.p0) {    // spatial jump
          hs.Set(true, p0, p_last);
          hs.attr.edgeno = ++edgeno;
          line += hs;
          hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
          line += hs;
          p0 = unit.p0;
          p1 = unit.p1;
          p_last = unit.p1;
        } else {    // an extension, check direction
          if (!AlmostEqual(p0, unit.p1)) {
            HalfSegment tmp(true, p0, unit.p1);
            double dist = tmp.Distance(p1);
            double dist2 = tmp.Distance(p_last);
            if (AlmostEqual(dist, 0.0) && AlmostEqual(dist2, 0.0)) {
              p_last = unit.p1;
            } else {
              hs.Set(true, p0, p_last);
              hs.attr.edgeno = ++edgeno;
              line += hs;
              hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
              line += hs;
              p0 = unit.p0;
              p1 = unit.p1;
              p_last = unit.p1;
            }
          }
        }
      }
    }
  }

  if (p0.IsDefined() && p_last.IsDefined() && !AlmostEqual(p0, p_last)) {
    hs.Set(true, p0, p_last);
    hs.attr.edgeno = ++edgeno;
    line += hs;
    hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
    line += hs;
  }

  Line line2(0);
  LineMinize::Realminize(line, line2);
  line.CopyFrom(&line2);
  line2.Destroy();
}

/*

1.1 Intersection2 operator

1.1.1 Points $\times$ Line $\rightarrow$ Points

*/
void IntersectionOp(const Points& points,
                    const Line& line,
                    Points& result)
{
  RegionLinePointsSetOp<Line, Points>::SetOp(line,
                                             points,
                                             SetOpType::Intersection,
                                             result);
}

/*

1.1.1 Points $\times$ Region $\rightarrow$ Points

*/
void IntersectionOp(const Points& points,
                    const Region& region,
                    Points& result)
{
  RegionLinePointsSetOp<Region, Points>::SetOp(region,
                                               points,
                                               SetOpType::Intersection,
                                               result);
}

/*

1.1.1 Line $\times$ Point $\rightarrow$ Points

*/
void IntersectionOp(const Line& line,
                    const Point& point,
                    Points& result)
{
  RegionLinePointsSetOp<Line, Point>::SetOp(line,
                                            point,
                                            SetOpType::Intersection,
                                            result);
}

/*

1.1.1 Line $\times$ Points $\rightarrow$ Points

*/
void IntersectionOp(const Line& line,
                    const Points& points,
                    Points& result)
{
  RegionLinePointsSetOp<Line, Points>::SetOp(line,
                                             points,
                                             SetOpType::Intersection,
                                             result);
}

/*

1.1.1 Line $\times$ Line $\rightarrow$ Line

*/
void IntersectionOp(const Line& line1,
                    const Line& line2,
                    Line& result)
{
  LineLineSetOp::SetOp(line1,
                       line2,
                       SetOpType::Intersection,
                       result);
}

/*

1.1.1 Region $\times$ Point $\rightarrow$ Points

*/
void IntersectionOp(const Region& region,
                    const Point& point,
                    Points& result)
{
  RegionLinePointsSetOp<Region, Point>::SetOp(region,
                                              point,
                                              SetOpType::Intersection,
                                              result);
}

/*

1.1.1 Region $\times$ Points $\rightarrow$ Points

*/
void IntersectionOp(const Region& region,
                    const Points& points,
                    Points& result)
{
  RegionLinePointsSetOp<Region, Points>::SetOp(region,
                                               points,
                                               SetOpType::Intersection,
                                               result);
}

/*

1.1.1 Region $\times$ Line $\rightarrow$ Line

*/
void IntersectionOp(const Region& region,
                    const Line& line,
                    Line& result)
{
  RegionLineSetOp::SetOp(region,
                         line,
                         SetOpType::Intersection,
                         result);
}

/*

1.1.1 Region $\times$ Region $\rightarrow$ Region

*/
void IntersectionOp(const Region& region1,
                    const Region& region2,
                    Region& result)
{
  RegionRegionSetOp::SetOp(region1,
                           region2,
                           SetOpType::Intersection,
                           result);
}

/*

1.1 Union2 operator

1.1.1 Line $\times$ Line $\rightarrow$ Line

*/
void UnionOp(const Line& line1,
             const Line& line2,
             Line& result)
{
  LineLineSetOp::SetOp(line1,
                       line2,
                       SetOpType::Union,
                       result);
}

/*

1.1.1 Region $\times$ Region $\rightarrow$ Region

*/
void UnionOp(const Region& region1,
             const Region& region2,
             Region& result)
{
  RegionRegionSetOp::SetOp(region1,
                           region2,
                           SetOpType::Union,
                           result);
}

/*

1.1 Minus2 operator

1.1.1 Point $\times$ Line $\rightarrow$ Points

*/
void MinusOp(const Point& point,
             const Line& line,
             Points& result)
{
  RegionLinePointsSetOp<Line, Point>::SetOp(line,
                                            point,
                                            SetOpType::Minus,
                                            result);
}

/*

1.1.1 Point $\times$ Region $\rightarrow$ Points

*/
void MinusOp(const Point& point,
             const Region& region,
             Points& result)
{
  RegionLinePointsSetOp<Region, Point>::SetOp(region,
                                              point,
                                              SetOpType::Minus,
                                              result);
}

/*

1.1.1 Points $\times$ Line $\rightarrow$ Points

*/
void MinusOp(const Points& points,
             const Line& line,
             Points& result)
{
  RegionLinePointsSetOp<Line, Points>::SetOp(line,
                                             points,
                                             SetOpType::Minus,
                                             result);
}

/*

1.1.1 Points $\times$ Region $\rightarrow$ Points

*/
void MinusOp(const Points& points,
             const Region& region,
             Points& result)
{
  RegionLinePointsSetOp<Region, Points>::SetOp(region,
                                               points,
                                               SetOpType::Minus,
                                               result);
}

/*

1.1.1 Line $\times$ Line $\rightarrow$ Line

*/
void MinusOp(const Line& line1,
             const Line& line2,
             Line& result)
{
  LineLineSetOp::SetOp(line1,
                       line2,
                       SetOpType::Minus,
                       result);
}

/*

1.1.1 Line $\times$ Region $\rightarrow$ Line

*/
void MinusOp(const Line& line,
             const Region& region,
             Line& result)
{
  RegionLineSetOp::SetOp(region,
                         line,
                         SetOpType::Minus,
                         result);
}

/*

1.1.1 Region $\times$ Region $\rightarrow$ Region

*/
void MinusOp(const Region& region1,
             const Region& region2,
             Region& result)
{
  RegionRegionSetOp::SetOp(region1,
                           region2,
                           SetOpType::Minus,
                           result);
}

/*

1.1 Intersects2 operator

1.1.1 Points $\times$ Line

*/
bool IntersectsOp(const Points& points,
                  const Line& line)
{
  return
  RegionLinePointsIntersects<Line, Points>::Intersects(line, points);
}

/*

1.1.1 Points $\times$ Region

*/
bool IntersectsOp(const Points& points,
                  const Region& region)
{
  return
  RegionLinePointsIntersects<Region, Points>::Intersects(region, points);
}

/*

1.1.1 Line $\times$ Line

*/
bool IntersectsOp(const Line& line1,
                  const Line& line2)
{
  return RegionLineIntersects<Line, Line>::Intersects(line1, line2);
}

/*

1.1.1 Line $\times$ Region

*/
bool IntersectsOp(const Line& line,
                  const Region& region)
{
  return RegionLineIntersects<Line, Region>::Intersects(line, region);
}

/*

1.1.1 Region $\times$ Region

*/
bool IntersectsOp(const Region& region1,
                  const Region& region2)
{
  return RegionLineIntersects<Region, Region>::Intersects(region1, region2);
}

/*

1.1 Crossings2 operator

*/
void CrossingsLine(const Line& line1,
                   const Line& line2,
                   Points& result)
{
  CrossingsData::Crossings(line1,
                           line2,
                           result);
}

/*

1.1 ToLine operator

*/
void ToLine(const DLine& dline,
            Line& result)
{
  DLineToLine::ToLine(dline, result);
}

/*

1 Helpers 

1.1 ~SpatialReturnFirstParameter~

*/
template<class T1, class T2, class TResult>
int SpatialReturnFirstParameter(Word* args,
                                Word& result,
                                int message,
                                Word& local,
                                Supplier s)
{
  result = qp->ResultStorage(s);

  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  TResult* res = static_cast<TResult*>(result.addr);

  res->Clear();
  if (!arg1->IsDefined() || !arg2->IsDefined()) {
    res->SetDefined(false);
  } else {
    res->SetDefined(true);
    res->CopyFrom(arg1);
  }

  return 0;
}

/*

1.1 ~SpatialReturnSecondParameter~

*/
template<class T1, class T2, class TResult>
int SpatialReturnSecondParameter(Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier s)
{
  result = qp->ResultStorage(s);

  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  TResult* res = static_cast<TResult*>(result.addr);

  res->Clear();
  if (!arg1->IsDefined() || !arg2->IsDefined()) {
    res->SetDefined(false);
  } else {
    res->SetDefined(true);
    res->CopyFrom(arg2);
  }

  return 0;
}


/*

1.1 Helper template to determine the size of an object

*/
template<class T>
struct SizeHelper
{
  static int Size(const T* obj)
  {
    return obj->Size();
  }
};

/*

1.1 Template specialization to determine the size of an point (always 1)

*/
template<>
struct SizeHelper<Point>
{
  static int Size(const Point* obj)
  {
    return 1;
  }
};


/*

1 Operator with Secondo parameters

1.1 Trajectory2

*/

int MPointTrajectory(Word* args,
                     Word& result,
                     int message,
                     Word& local,
                     Supplier s)
{
  result = qp->ResultStorage(s);

  Line *line = ((Line*)result.addr);
  MPoint *mpoint = ((MPoint*)args[0].addr);

  MPointTrajectory(mpoint, *line);

  return 0;
}

/*

1.1 Intersection2

*/
template<class T1, class T2, class TResult, bool reverseT1T2>
int SpatialIntersectionGeneric(Word* args,
                               Word& result,
                               int message,
                               Word& local,
                               Supplier s)
{
  result = qp->ResultStorage(s);

  T1* arg1;
  T2* arg2;

  if (!reverseT1T2) {
    arg1 = static_cast<T1*>(args[0].addr);
    arg2 = static_cast<T2*>(args[1].addr);
  } else {
    arg1 = static_cast<T1*>(args[1].addr);
    arg2 = static_cast<T2*>(args[0].addr);
  }
  TResult* res = static_cast<TResult*>(result.addr);

  res->Clear();
  if (!arg1->IsDefined() || !arg2->IsDefined()) {
    res->SetDefined(false);
  } else {
    res->SetDefined(true);
    if (SizeHelper<T1>::Size(arg1) == 0 || SizeHelper<T2>::Size(arg2) == 0
        || !(arg1->BoundingBox().IntersectsUD(arg2->BoundingBox()))) {
      // empty result
      // Clear doesn't clear everything?
      res->StartBulkLoad();
      res->EndBulkLoad();
    } else {
      IntersectionOp(*arg1, *arg2, *res);
    }
  }

  return 0;
}

/*

1.1 Minus2

*/
template<class T1, class T2, class TResult>
int SpatialMinusGeneric(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s)
{
  result = qp->ResultStorage(s);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  TResult* res = static_cast<TResult*>(result.addr);

  res->Clear();
  if (!arg1->IsDefined() || !arg2->IsDefined()) {
    res->SetDefined(false);
  } else {
    res->SetDefined(true);
    if (SizeHelper<T1>::Size(arg1) == 0) {
      // empty result;
      // Clear doesn't clear everything?
      res->StartBulkLoad();
      res->EndBulkLoad();
    } else if (std::is_same<T1, TResult>::value
               && (SizeHelper<T2>::Size(arg2) == 0 ||
                   !(arg1->BoundingBox().IntersectsUD(arg2->BoundingBox())))) {
      res->CopyFrom(arg1);
    } else {
      MinusOp(*arg1, *arg2, *res);
    }
  }

  return 0;
}


/*

1.1 Union2

*/
template<class T1, class T2, class TResult>
int SpatialUnionGeneric(
                        Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s)
{
  result = qp->ResultStorage(s);
  T1* arg1 = static_cast<T1*>(args[0].addr);
  T2* arg2 = static_cast<T2*>(args[1].addr);
  TResult* res = static_cast<TResult*>(result.addr);

  res->Clear();
  if (!arg1->IsDefined() || !arg2->IsDefined()) {
    res->SetDefined(false);
  } else {
    res->SetDefined(true);
    if (std::is_same<T2, TResult>::value && SizeHelper<T1>::Size(arg1) == 0) {
      res->CopyFrom(arg2);
    } else if (std::is_same<T1, TResult>::value
               && SizeHelper<T2>::Size(arg2) == 0) {
      res->CopyFrom(arg1);
    } else if (!(arg1->BoundingBox().IntersectsUD(arg2->BoundingBox()))) {
      res->StartBulkLoad();
      int edgeno = 0;
      int s = arg1->Size();
      HalfSegment hs;
      for (int i = 0; i < s; i++) {
        arg1->Get(i, hs);
        if (hs.IsLeftDomPoint()) {
          HalfSegment HS(hs);
          HS.attr.edgeno = edgeno;
          (*res) += HS;
          HS.SetLeftDomPoint(false);
          (*res) += HS;
          edgeno++;
        }
      }
      s = arg2->Size();
      for (int i = 0; i < s; i++) {
        arg2->Get(i, hs);
        if (hs.IsLeftDomPoint()) {
          HalfSegment HS(hs);
          HS.attr.edgeno = edgeno;
          (*res) += HS;
          HS.SetLeftDomPoint(false);
          (*res) += HS;
          edgeno++;
        }
      }

      if (std::is_same<TResult, Region>::value) {
        res->EndBulkLoad();
      } else {
        res->EndBulkLoad(true, false);
      }
    } else {
      UnionOp(*arg1, *arg2, *res);
    }
  }

  return 0;
}

/*

1.1 Crossings2

*/
int CrossingsLine(Word* args,
                  Word& result,
                  int message,
                  Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  Line* arg1 = static_cast<Line*>(args[0].addr);
  Line* arg2 = static_cast<Line*>(args[1].addr);
  Points* res = static_cast<Points*>(result.addr);
  CrossingsLine(*arg1, *arg2, *res);
  return 0;
}

/*

1.1 ToLine

*/
int ToLine(Word* args,
           Word& result,
           int message,
           Word& local,
           Supplier s)
{
  result = qp->ResultStorage(s);
  DLine* arg1 = static_cast<DLine*>(args[0].addr);
  Line* res = static_cast<Line*>(result.addr);
  ToLine(*arg1, *res);

  return 0;
}

/*

1.1 Intersects2

*/
template<class A, class B, bool symm>
int SpatialIntersectsVM(Word* args, Word& result, int message,
                        Word& local,
                        Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*>(result.addr);
  A* a;
  B* b;
  if (symm) {
    a = static_cast<A*>(args[1].addr);
    b = static_cast<B*>(args[0].addr);
  } else {
    a = static_cast<A*>(args[0].addr);
    b = static_cast<B*>(args[1].addr);
  }

  if (!a->IsDefined() || !b->IsDefined()) {
    res->Set(false, false);
  } else if (!(a->BoundingBox().IntersectsUD(b->BoundingBox()))) {
    res->Set(true, false);
  } else {
    bool result = IntersectsOp(*a, *b);
    res->Set(true, result);
  }

  return 0;
}

/*

1 Operator definitions 

1.1 Signatures

1.1.1 Trajectory2

*/
const string TemporalSpecTrajectory =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>mpoint -> line</text--->"
        "<text> trajectory2( _ )</text--->"
        "<text>Get the trajectory of the corresponding"
        " moving point object.</text--->"
        "<text>trajectory2( mp1 )</text--->"
        ") )";

/*

1.1.1 ToLine

*/
const string ToLineSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>dline -> line</text--->"
        "<text>toline( _ )</text--->"
        "<text>Converts a dline into a line</text--->"
        "<text>toline( dline )</text--->"
        ") )";

/*

1.1.1 Intersection2

*/
const string SpatialIntersectionSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>{point, points, line, region } x"
        "   {point, points, line, region} -> T, "
        " where T = points if any point or point type is one of the "
        " arguments or the argument having the smaller dimension </text--->"
        "<text>intersection2(arg1, arg2)</text--->"
        "<text>intersection of two spatial objects</text--->"
        "<text>query intersection2(tiergarten, thecenter) </text--->"
        ") )";

/*

1.1.1 Minus2

*/
const string SpatialMinusSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>{point, points, line, region } x"
        "   {point, points, line, sline region} -> T "
        " </text--->"
        "<text>arg1 minus2 arg2</text--->"
        "<text>difference of two spatial objects</text--->"
        "<text>query tiergarten minus2 thecenter </text--->"
        ") )";

/*

1.1.1 Union2

*/
const string SpatialUnionSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>{point , points, line, region } x"
        "   {point, points, line, region} -> T "
        " </text--->"
        "<text>arg1 union2 arg2</text--->"
        "<text>union of two spatial objects</text--->"
        "<text>query tiergarten union2 thecenter </text--->"
        ") )";

/*

1.1.1 Crossings2

*/
const string SpatialSpecCrossings =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
        "( <text>(line x line) -> points</text--->"
        "<text>crossings2( _, _ )</text--->"
        "<text>crossing points of two lines.</text--->"
        "<text>query crossings2(line1, line2)</text--->"
        ") )";

/*

1.1.1 Intersects2

*/
const string SpatialSpecIntersects =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text>(points||line||region x points||line||region) -> bool"
        " </text--->"
        "<text>_ intersects2 _</text--->"
        "<text>TRUE, iff both arguments intersect.</text--->"
        "<text>query region1 intersects2 region2</text--->"
        ") )";

/*

1.1 Value Mapping

1.1.1 Intersection2

*/
ValueMapping spatialintersectionVM[] = {
    SpatialIntersectionGeneric<Line, Point, Points, true>,
    SpatialIntersectionGeneric<Region, Point, Points, true>,

    SpatialIntersectionGeneric<Points, Line, Points, false>,
    SpatialIntersectionGeneric<Points, Region, Points, false>,

    SpatialIntersectionGeneric<Line, Point, Points, false>,
    SpatialIntersectionGeneric<Line, Points, Points, false>,
    SpatialIntersectionGeneric<Line, Line, Line, false>,
    SpatialIntersectionGeneric<Region, Line, Line, true>,

    SpatialIntersectionGeneric<Region, Point, Points, false>,
    SpatialIntersectionGeneric<Region, Points, Points, false>,
    SpatialIntersectionGeneric<Region, Line, Line, false>,
    SpatialIntersectionGeneric<Region, Region, Region, false>
};

/*

1.1.1 Minus2

*/
ValueMapping spatialminusVM[] = {
    SpatialMinusGeneric<Point, Line, Points>,
    SpatialMinusGeneric<Point, Region, Points>,

    SpatialMinusGeneric<Points, Line, Points>,
    SpatialMinusGeneric<Points, Region, Points>,

    SpatialReturnFirstParameter<Line, Point, Line>,
    SpatialReturnFirstParameter<Line, Points, Line>,
    SpatialMinusGeneric<Line, Line, Line>,
    SpatialMinusGeneric<Line, Region, Line>,

    SpatialReturnFirstParameter<Region, Point, Region>,
    SpatialReturnFirstParameter<Region, Points, Region>,
    SpatialReturnFirstParameter<Region, Line, Region>,
    SpatialMinusGeneric<Region, Region, Region>
};

/*

1.1.1 Union2

*/
ValueMapping spatialunionVM[] = {
    SpatialReturnSecondParameter<Point, Line, Line>,
    SpatialReturnSecondParameter<Point, Region, Region>,

    SpatialReturnSecondParameter<Points, Line, Line>,
    SpatialReturnSecondParameter<Points, Region, Region>,

    SpatialReturnFirstParameter<Line, Point, Line>,
    SpatialReturnFirstParameter<Line, Points, Line>,
    SpatialUnionGeneric<Line, Line, Line>,
    SpatialReturnSecondParameter<Line, Region, Region>,

    SpatialReturnFirstParameter<Region, Point, Region>,
    SpatialReturnFirstParameter<Region, Points, Region>,
    SpatialReturnFirstParameter<Region, Line, Region>,
    SpatialUnionGeneric<Region, Region, Region>
};

/*

1.1.1 Intersects

*/
ValueMapping spatialintersectsmap[] = {
    SpatialIntersectsVM<Points, Line, false>,
    SpatialIntersectsVM<Points, Region, false>,
    SpatialIntersectsVM<Points, Line, true>,
    SpatialIntersectsVM<Line, Line, false>,
    SpatialIntersectsVM<Line, Region, false>,
    SpatialIntersectsVM<Points, Region, true>,
    SpatialIntersectsVM<Line, Region, true>,
    SpatialIntersectsVM<Region, Region, false>
};

/*

1.1 Selection methods

1.1.1 SpatialSetOpSelect

used for set operations

*/
int SpatialSetOpSelect(ListExpr args)
{
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if (a1 == Point::BasicType()) {
    if (a2 == Line::BasicType()) {
      return 0;
    }

    if (a2 == Region::BasicType()) {
      return 1;
    }

    return -1;
  }

  if (a1 == Points::BasicType()) {
    if (a2 == Line::BasicType()) {
      return 2;
    }

    if (a2 == Region::BasicType()) {
      return 3;
    }

    return -1;
  }

  if (a1 == Line::BasicType()) {
    if (a2 == Point::BasicType()) {
      return 4;
    }

    if (a2 == Points::BasicType()) {
      return 5;
    }

    if (a2 == Line::BasicType()) {
      return 6;
    }

    if (a2 == Region::BasicType()) {
      return 7;
    }

    return -1;
  }

  if (a1 == Region::BasicType()) {
    if (a2 == Point::BasicType()) {
      return 8;
    }

    if (a2 == Points::BasicType()) {
      return 9;
    }

    if (a2 == Line::BasicType()) {
      return 10;
    }

    if (a2 == Region::BasicType()) {
      return 11;
    }

    return -1;
  }

  return -1;
}

enum SpatialType
{
  stpoint,
  stpoints,
  stline,
  stregion,
  stbox,
  sterror,
  stsline
};

SpatialType SpatialTypeOfSymbol(ListExpr symbol)
{
  if (nl->AtomType(symbol) == SymbolType) {
    string s = nl->SymbolValue(symbol);
    if (s == Point::BasicType()) {
      return (stpoint);
    }

    if (s == Points::BasicType()) {
      return (stpoints);
    }

    if (s == Line::BasicType()) {
      return (stline);
    }

    if (s == Region::BasicType()) {
      return (stregion);
    }

    if (s == Rectangle<2>::BasicType()) {
      return (stbox);
    }

    if (s == SimpleLine::BasicType()) {
      return (stsline);
    }
  }
  return (sterror);
}

/*

1.1.1 SpatialSelectIntersects

used for intersects2 operator

*/
int SpatialSelectIntersects(ListExpr args)
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (SpatialTypeOfSymbol(arg1) == stpoints
      && SpatialTypeOfSymbol(arg2) == stline) {
    return 0;
  }

  if (SpatialTypeOfSymbol(arg1) == stpoints
      && SpatialTypeOfSymbol(arg2) == stregion) {
    return 1;
  }

  if (SpatialTypeOfSymbol(arg1) == stline
      && SpatialTypeOfSymbol(arg2) == stpoints) {
    return 2;
  }

  if (SpatialTypeOfSymbol(arg1) == stline
      && SpatialTypeOfSymbol(arg2) == stline) {
    return 3;
  }

  if (SpatialTypeOfSymbol(arg1) == stline
      && SpatialTypeOfSymbol(arg2) == stregion) {
    return 4;
  }

  if (SpatialTypeOfSymbol(arg1) == stregion
      && SpatialTypeOfSymbol(arg2) == stpoints) {
    return 5;
  }

  if (SpatialTypeOfSymbol(arg1) == stregion
      && SpatialTypeOfSymbol(arg2) == stline) {
    return 6;
  }

  if (SpatialTypeOfSymbol(arg1) == stregion
      && SpatialTypeOfSymbol(arg2) == stregion) {
    return 7;
  }

  return -1;    // This point should never be reached
}

/*

1.1 Type mapping

1.1.1 Trajectory2

*/
ListExpr MovingTypeMapSpatial(ListExpr args)
{
  if (nl->ListLength(args) == 1) {
    ListExpr arg1 = nl->First(args);

    if (nl->IsEqual(arg1, MPoint::BasicType())) {
      return nl->SymbolAtom(Line::BasicType());
    }
  }

  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

1.1.1 ToLine

*/
ListExpr ToLineTypeMap(ListExpr args)
{
  if (nl->ListLength(args) == 1) {
    ListExpr arg1 = nl->First(args);

    if (nl->IsEqual(arg1, DLine::BasicType())) {
      return nl->SymbolAtom(Line::BasicType());
    }
  }

  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*

1.1.1 Union2

*/
ListExpr SpatialUnionTypeMap(ListExpr args)
{
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("wrong number of arguments");
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (!listutils::isSymbol(arg1)) {
    return listutils::typeError("first arg is not a supported spatial type");
  }

  if (!listutils::isSymbol(arg2)) {
    return listutils::typeError("second arg is not a supported spatial type");
  }

  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if (a1 == Point::BasicType()) {
    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Points::BasicType()) {
    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Line::BasicType()) {
    if (a2 == Point::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Points::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Region::BasicType()) {
    if (a2 == Point::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Points::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  return listutils::typeError("first arg is not a supported spatial type");
}

/*

1.1.1 Crossings2

*/
ListExpr SpatialCrossingsTM(ListExpr args)
{
  ListExpr arg1, arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);

    string a1 = nl->SymbolValue(arg1);
    string a2 = nl->SymbolValue(arg2);

    if (a1 == Line::BasicType() && a2 == Line::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }
  }

  return listutils::typeError("line x line expected");
}

/*

1.1.1 Minus2

*/
ListExpr SpatialMinusTypeMap(ListExpr args)
{
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("wrong number of arguments");
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if (!listutils::isSymbol(arg1)) {
    return listutils::typeError("first arg is not a supported spatial type");
  }

  if (!listutils::isSymbol(arg2)) {
    return listutils::typeError("second arg is not a supported spatial type");
  }

  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if (a1 == Point::BasicType()) {
    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Points::BasicType()) {
    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }
    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Line::BasicType()) {
    if (a2 == Point::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Points::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Region::BasicType()) {
    if (a2 == Point::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Points::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  return listutils::typeError("first arg is not a supported spatial type");
}

/*

1.1.1 Intersection2

*/
ListExpr SpatialIntersectionTypeMap(ListExpr args)
{
  if (nl->ListLength(args) != 2) {
    return listutils::typeError("wrong number of arguments");
  }

  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if (!listutils::isSymbol(arg1)) {
    return listutils::typeError("first arg is not a supported spatial type");
  }

  if (!listutils::isSymbol(arg2)) {
    return listutils::typeError("second arg is not a supported spatial type");
  }

  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if (a1 == Point::BasicType()) {
    if (a2 == Line::BasicType() || a2 == Region::BasicType())
      return nl->SymbolAtom(Points::BasicType());

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Points::BasicType()) {
    if (a2 == Line::BasicType() || a2 == Region::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Line::BasicType()) {
    if (a2 == Point::BasicType() || a2 == Points::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    if (a2 == Line::BasicType() || a2 == Region::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  if (a1 == Region::BasicType()) {
    if (a2 == Point::BasicType() || a2 == Points::BasicType()) {
      return nl->SymbolAtom(Points::BasicType());
    }

    if (a2 == Line::BasicType()) {
      return nl->SymbolAtom(Line::BasicType());
    }

    if (a2 == Region::BasicType()) {
      return nl->SymbolAtom(Region::BasicType());
    }

    return listutils::typeError("second arg is not a supported spatial type");
  }

  return listutils::typeError("first arg not a supported spatial type");
}

/*

1.1.1 Intersects2

*/
ListExpr IntersectsTM(ListExpr args)
{
  ListExpr arg1, arg2;
  if (nl->ListLength(args) == 2) {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    SpatialType st1 = SpatialTypeOfSymbol(arg1);
    SpatialType st2 = SpatialTypeOfSymbol(arg2);

    if (st1 != stpoints || st2 != stpoints) {
      if (((st1 == stpoints) || (st1 == stline) || (st1 == stregion)) &&
          ((st2 == stpoints) || (st2 == stline) || (st2 == stregion))) {
        return nl->SymbolAtom(CcBool::BasicType());
      }
    }
  }

  return listutils::typeError(" t_1 x t_2 expected,"
                              " with t_1, t_2 in {points,line,region}");
}

/*

1.1 Definitions

*/

Operator temporaltrajectory2("trajectory2",
                             TemporalSpecTrajectory,
                             MPointTrajectory,
                             Operator::SimpleSelect,
                             MovingTypeMapSpatial);

Operator toline("toline",
                ToLineSpec,
                ToLine,
                Operator::SimpleSelect,
                ToLineTypeMap);

Operator spatialcrossings2("crossings",
                           SpatialSpecCrossings,
                           CrossingsLine,
                           Operator::SimpleSelect,
                           SpatialCrossingsTM);

Operator spatialintersection2("intersection",
                              SpatialIntersectionSpec,
                              12,
                              spatialintersectionVM,
                              SpatialSetOpSelect,
                              SpatialIntersectionTypeMap);

Operator spatialminus2("minus",
                       SpatialMinusSpec,
                       12,
                       spatialminusVM,
                       SpatialSetOpSelect,
                       SpatialMinusTypeMap);

Operator spatialunion2("union",
                       SpatialUnionSpec,
                       12,
                       spatialunionVM,
                       SpatialSetOpSelect,
                       SpatialUnionTypeMap);

Operator spatialintersects2("intersects",
                            SpatialSpecIntersects,
                            8,
                            spatialintersectsmap,
                            SpatialSelectIntersects,
                            IntersectsTM);

/*

1 Algebra definition

*/

RobustPlaneSweepAlgebra::RobustPlaneSweepAlgebra()
{
  AddOperator(&temporaltrajectory2);
  AddOperator(&toline);
  AddOperator(&spatialintersection2);
  AddOperator(&spatialminus2);
  AddOperator(&spatialunion2);
  AddOperator(&spatialcrossings2);
  AddOperator(&spatialintersects2);
}
}

extern "C"
Algebra* InitializeRobustPlaneSweepAlgebra(
                                           NestedList* nlRef,
                                           QueryProcessor* qpRef)
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new RobustPlaneSweep::RobustPlaneSweepAlgebra();
}
