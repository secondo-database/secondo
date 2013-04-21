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

// #define OUTPUT_HALFSEGMENTS

#include "RobustPlaneSweepAlgebra.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "DLine.h"

#include "Algorithm/Hobby.h"

namespace RobustPlaneSweep
{
  enum SetOpType
  {
    SetOpUnion = 1,
    SetOpIntersection = 2,
    SetOpMinus = 3
  };

/*

1 data class for line set operations

*/
  class LineSetOp : public IntersectionAlgorithmData
  {
  private:
    const Line& _source1;
    int _currentSource1Index;

    const Line& _source2;
    int _currentSource2Index;

    SetOpType _setOpType;

    Line& _result;
    int _outputSegments;

  public:
    LineSetOp(
      const Line& source1,
      const Line& source2,
      SetOpType setOpType,
      Line& result) :
    _source1(source1),
      _currentSource1Index(0),
      _source2(source2),
      _currentSource2Index(0),
      _setOpType(setOpType),
      _result(result),
      _outputSegments(0)
    {
    }

    ~LineSetOp()
    {
    }

    void InitializeFetch()
    {
      _currentSource1Index = 0;
      _currentSource2Index = 0;
    }

    bool FetchInputHalfSegment(
      HalfSegment &segment,
      bool &belongsToSecondGeometry)
    {
      if (_currentSource1Index < _source1.Size() &&
        _currentSource2Index < _source2.Size()) {
          HalfSegment s1, s2;
          _source1.Get(_currentSource1Index, s1);
          _source2.Get(_currentSource2Index, s2);
          if (s1.Compare(s2) < 0) {
            segment = s1;
            _currentSource1Index++;
            belongsToSecondGeometry = false;
          } else {
            segment = s2;
            _currentSource2Index++;
            belongsToSecondGeometry = true;
          }
          return true;
      } else if (_currentSource1Index < _source1.Size()) {
        _source1.Get(_currentSource1Index++, segment);
        belongsToSecondGeometry = false;
        return true;
      } else if (_currentSource2Index < _source2.Size()) {
        _source2.Get(_currentSource2Index++, segment);
        belongsToSecondGeometry = true;
        return true;
      } else {
        return false;
      }
    }

    void OutputHalfSegment(
      const HalfSegment& segment,
      const InternalAttribute& attribute)
    {
      switch (_setOpType) {
      case SetOpType::SetOpUnion:
        break;
      case SetOpType::SetOpIntersection:
        if (!attribute.IsBorderInBoth()) {
          return;
        }
        break;

      case SetOpType::SetOpMinus:
        if (attribute.IsSecondBorder()) {
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

    const Rectangle<2> GetBoundingBox()
    {
      return _source1.BoundingBox().Union(_source2.BoundingBox());
    }

    IntersectionAlgorithmCalculationType GetCalculationType()
    {
      return IntersectionAlgorithmCalculationType::CalulationTypeLine;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    void GetRoundToDecimals(int& decimals, int& stepSize)
    {
      // because of AlmostEqual
      decimals = 7;
      stepSize = 2;
    }

    void OutputFinished()
    {
    }

    static void SetOp(const Line& line1,
      const Line& line2,
      SetOpType setOpType,
      Line& result)
    {
      result.Clear();

      if (!line1.IsDefined() || !line2.IsDefined()) {
        result.SetDefined(false);
        return;
      }

      result.SetDefined(true);
      if (line1.Size() == 0) {
        switch (setOpType) {
        case SetOpType::SetOpUnion:
          result = line2;
          return;
        case SetOpType::SetOpIntersection:
          return;  // empty line
        case SetOpType::SetOpMinus:
          return;  // empty line
        default:
          throw new std::invalid_argument("setOpType");
        }
      }
      if (line2.Size() == 0) {
        switch (setOpType) {
        case SetOpType::SetOpUnion:
          result = line1;
          return;
        case SetOpType::SetOpIntersection:
          return;  // empty line
        case SetOpType::SetOpMinus:
          result = line1;
          return;
        default:
          throw new std::invalid_argument("setOpType");
        }
      }

      result.StartBulkLoad();
      LineSetOp data(line1, line2, setOpType, result);
      Hobby hobby(&data);
      hobby.DetermineIntersections();

      result.EndBulkLoad(true, false);
    }
  };


/*

1 data class for region set operations

*/
  class RegionSetOp : public IntersectionAlgorithmData
  {
  private:
    const Region& _source1;
    int _currentSource1Index;

    const Region& _source2;
    int _currentSource2Index;

    SetOpType _setOpType;

    Region& _result;
    int _outputSegments;

  public:
    RegionSetOp(
      const Region& source1,
      const Region& source2,
      SetOpType setOpType,
      Region& result) :
    _source1(source1),
      _currentSource1Index(0),
      _source2(source2),
      _currentSource2Index(0),
      _setOpType(setOpType),
      _result(result),
      _outputSegments(0)
    {
    }

    ~RegionSetOp()
    {
    }

    void InitializeFetch()
    {
      _currentSource1Index = 0;
      _currentSource2Index = 0;
    }

    bool FetchInputHalfSegment(
      HalfSegment &segment,
      bool &belongsToSecondGeometry)
    {
      if (_currentSource1Index < _source1.Size() &&
        _currentSource2Index < _source2.Size()) {
          HalfSegment s1, s2;
          _source1.Get(_currentSource1Index, s1);
          _source2.Get(_currentSource2Index, s2);
          if (s1.Compare(s2) < 0) {
            segment = s1;
            _currentSource1Index++;
            belongsToSecondGeometry = false;
          } else {
            segment = s2;
            _currentSource2Index++;
            belongsToSecondGeometry = true;
          }
      } else if (_currentSource1Index < _source1.Size()) {
        _source1.Get(_currentSource1Index++, segment);
        belongsToSecondGeometry = false;
      } else if (_currentSource2Index < _source2.Size()) {
        _source2.Get(_currentSource2Index++, segment);
        belongsToSecondGeometry = true;
      } else {
        return false;
      }

#if OUTPUT_HALFSEGMENTS
      if (segment.IsLeftDomPoint()) {
        cout << "I: " <<
          (belongsToSecondGeometry?"1":"0") << " " <<
          (segment.attr.edgeno) << " (" <<
          segment.GetLeftPoint().GetX() << "," <<
          segment.GetLeftPoint().GetY() << ") - (" <<
          segment.GetRightPoint().GetX() << "," <<
          segment.GetRightPoint().GetY() << ") " <<
          (segment.attr.insideAbove?"Above":"Below") <<
          "\n";
      }
#endif

      return true;
    }

    void OutputHalfSegment(
      const HalfSegment& segment,
      const InternalAttribute& a)
    {
      bool include;
      bool insideAbove = false;

      switch (_setOpType) {
      case SetOpType::SetOpUnion:
        {
          int coverageAbove = (a.IsFirstAbove()?1:0)+(a.IsSecondAbove()?1:0);
          int coverageBelow = (a.IsFirstBelow()?1:0)+(a.IsSecondBelow()?1:0);

          if (coverageAbove == 0 && coverageBelow > 0) {
            insideAbove = false;
            include= true;
          } else if (coverageAbove > 0 && coverageBelow == 0) {
            insideAbove = true;
            include = true;
          } else {
            include = false;
          }
        }
        break;
      case SetOpType::SetOpIntersection:
        {
          if (a.IsFirstAbove() && a.IsSecondAbove()) {
            insideAbove = true;
            include = true;
          } else if (a.IsFirstBelow() && a.IsSecondBelow()) {
            insideAbove = false;
            include = true;
          } else {
            include = false;
          }
        }
        break;
      case SetOpType::SetOpMinus:
        {
          int coverageAbove = (a.IsFirstAbove()?1:0)+(a.IsSecondAbove()?1:0);
          int coverageBelow = (a.IsFirstBelow()?1:0)+(a.IsSecondBelow()?1:0);
          include = false;

          if (a.IsBorderInBoth()) {
            if (coverageAbove == 1 && coverageBelow == 1) {
              insideAbove = a.IsFirstAbove();
              include = true;
            }
          } else if (a.IsFirstBorder()) {
            if ((coverageAbove + coverageBelow) == 1) {
              insideAbove = a.IsFirstAbove();
              include = true;
            }
          } else if (a.IsSecondBorder()) {
            if ((coverageAbove + coverageBelow) == 3) {
              insideAbove = a.IsSecondBelow();
              include = true;
            }
          }
        }
        break;
      default:
        throw new std::logic_error("setOpType");
      }

#if OUTPUT_HALFSEGMENTS
      cout << "O: " <<
        _outputSegments << " (" <<
        segment.GetLeftPoint().GetX() << "," <<
        segment.GetLeftPoint().GetY() << ") - (" <<
        segment.GetRightPoint().GetX() << "," <<
        segment.GetRightPoint().GetY() << ") (" <<
        (a.IsFirstAbove()?"A":"_") <<
        (a.IsFirstBelow()?"B":"_") << "/" <<
        (a.IsSecondAbove()?"A":"_") <<
        (a.IsSecondBelow()?"B":"_") << ")" <<
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
      s1.attr.insideAbove = insideAbove;
      s1.attr.edgeno = _outputSegments;
      s2.attr.insideAbove = insideAbove;
      s2.attr.edgeno = _outputSegments;
      _result += s1;
      _result += s2;
      _outputSegments++;
    }

    const Rectangle<2> GetBoundingBox()
    {
      return _source1.BoundingBox().Union(_source2.BoundingBox());
    }

    IntersectionAlgorithmCalculationType GetCalculationType()
    {
      return IntersectionAlgorithmCalculationType::CalulationTypeRegion;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    void GetRoundToDecimals(int& decimals, int& stepSize)
    {
      // because of AlmostEqual
      decimals = 7;
      stepSize = 2;
    }

    void OutputFinished()
    {
    }

    static void SetOp(const Region& region1,
      const Region& region2,
      SetOpType setOpType,
      Region& result)
    {
      result.Clear();
      if (!region1.IsDefined() || !region2.IsDefined()) {
        result.SetDefined(false);
        return;
      }
      result.SetDefined(true);
      if (region1.Size() == 0) {
        switch (setOpType) {
        case SetOpType::SetOpUnion:
          result = region2;
          return;
        case SetOpType::SetOpIntersection:
          return;  // empty region
        case SetOpType::SetOpMinus:
          return;  // empty region
        default:
          throw new std::invalid_argument("setOpType");
        }
      }

      if (region2.Size() == 0) {
        switch (setOpType) {
        case SetOpType::SetOpUnion:
          result = region1;
          return;
        case SetOpType::SetOpIntersection:
          return;  // empty region
        case SetOpType::SetOpMinus:
          result = region1;
          return;
        default:
          throw new std::invalid_argument("setOpType");
        }
      }

      if (!region1.BoundingBox().Intersects(region2.BoundingBox())) {
        switch (setOpType) {
        case SetOpType::SetOpUnion:
          {
            result.StartBulkLoad();
            int edgeno = 0;
            int s = region1.Size();
            HalfSegment hs;
            for (int i = 0; i < s; i++) {
              region1.Get(i, hs);
              if (hs.IsLeftDomPoint()) {
                HalfSegment HS(hs);
                HS.attr.edgeno = edgeno;
                result += HS;
                HS.SetLeftDomPoint(false);
                result += HS;
                edgeno++;
              }
            }
            s = region2.Size();
            for (int i = 0; i < s; i++) {
              region2.Get(i, hs);
              if (hs.IsLeftDomPoint()) {
                HalfSegment HS(hs);
                HS.attr.edgeno = edgeno;
                result += HS;
                HS.SetLeftDomPoint(false);
                result += HS;
                edgeno++;
              }
            }
            result.EndBulkLoad();
            return;
          }
        case SetOpType::SetOpIntersection:
          return;
        case SetOpType::SetOpMinus:
          result = region1;
          return;
        default:
          throw new std::invalid_argument("setOpType");
        }
      }

      result.Clear();

      result.StartBulkLoad();
      RegionSetOp data(region1, region2, setOpType, result);
      Hobby hobby(&data);
      hobby.DetermineIntersections();

      result.EndBulkLoad();
    }
  };

/*

1 data class for crossing operator 

*/
  class CrossingsData : public IntersectionAlgorithmData
  {
  private:
    const Line& _source1;
    int _currentSource1Index;

    const Line& _source2;
    int _currentSource2Index;

    Points& _result;

  public:
    CrossingsData(
      const Line& source1,
      const Line& source2,
      Points& result) :
    _source1(source1),
      _currentSource1Index(0),
      _source2(source2),
      _currentSource2Index(0),
      _result(result)
    {
    }

    ~CrossingsData()
    {
    }

    void InitializeFetch()
    {
      _currentSource1Index = 0;
      _currentSource2Index = 0;
    }

    bool FetchInputHalfSegment(
      HalfSegment &segment,
      bool &belongsToSecondGeometry)
    {
      if (_currentSource1Index < _source1.Size() &&
        _currentSource2Index < _source2.Size()) {
          HalfSegment s1, s2;
          _source1.Get(_currentSource1Index, s1);
          _source2.Get(_currentSource2Index, s2);
          if (s1.Compare(s2) < 0) {
            segment = s1;
            _currentSource1Index++;
            belongsToSecondGeometry = false;
          } else {
            segment = s2;
            _currentSource2Index++;
            belongsToSecondGeometry = true;
          }
          return true;
      } else if (_currentSource1Index < _source1.Size()) {
        _source1.Get(_currentSource1Index++, segment);
        belongsToSecondGeometry = false;
        return true;
      } else if (_currentSource2Index < _source2.Size()) {
        _source2.Get(_currentSource2Index++, segment);
        belongsToSecondGeometry = true;
        return true;
      } else {
        return false;
      }
    }

    void OutputHalfSegment(
      const HalfSegment& segment,
      const InternalAttribute& attribute)
    {
    }

    const Rectangle<2> GetBoundingBox()
    {
      return _source1.BoundingBox().Union(_source2.BoundingBox());
    }

    IntersectionAlgorithmCalculationType GetCalculationType()
    {
      return IntersectionAlgorithmCalculationType::CalulationTypeNone;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    void GetRoundToDecimals(int& decimals, int& stepSize)
    {
      // because of AlmostEqual
      decimals = 7;
      stepSize = 2;
    }

    void OutputFinished()
    {
    }

    bool ReportIntersections()
    {
      return true;
    }

    void ReportIntersection(
      const Point& intersectionPoint,
      const bool overlappingIntersection)
    {
      if (!overlappingIntersection) {
        _result += intersectionPoint;
      }
    }

    static void Crossings(
      const Line& line1,
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
      CrossingsData data(line1, line2, result);
      BentleyOttmann bo(&data);
      bo.DetermineIntersections();

      result.EndBulkLoad(true, true);  // sort and remove duplicates
    }
  };

/*

1 data class for line minize

*/
  class LineMinize : public IntersectionAlgorithmData
  {
  private:
    const Line& _source;
    int _currentSourceIndex;
    Line& _result;
    int _outputSegments;

  public:
    LineMinize(const Line& src, Line& result) :
      _source(src),
      _currentSourceIndex(0),
      _result(result),
      _outputSegments(0)
    {
    }

    ~LineMinize()
    {
    }

    void InitializeFetch()
    {
      _currentSourceIndex = 0;
    }

    bool FetchInputHalfSegment(
      HalfSegment &segment,
      bool &belongsToSecondGeometry)
    {
      if (_currentSourceIndex < _source.Size()) {
        _source.Get(_currentSourceIndex++, segment);
        belongsToSecondGeometry = false;
        return true;
      } else {
        return false;
      }
    }

    void OutputHalfSegment(
      const HalfSegment& segment,
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

    const Rectangle<2> GetBoundingBox()
    {
      return _source.BoundingBox();
    }

    IntersectionAlgorithmCalculationType GetCalculationType()
    {
      return IntersectionAlgorithmCalculationType::CalulationTypeLine;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    void GetRoundToDecimals(int& decimals, int& stepSize)
    {
      // because of AlmostEqual
      decimals = 7;
      stepSize = 2;
    }

    void OutputFinished()
    {
    }

    static void Realminize(const Line& src, Line& result) {
      result.Clear();
      if (!src.IsDefined()) {
        result.SetDefined(false);
        return;
      }
      result.SetDefined(true);
      if (src.Size() == 0) {  // empty line, nothing to realminize
        return;
      }

      result.StartBulkLoad();
      LineMinize data(src, result);
      Hobby hobby(&data);
      hobby.DetermineIntersections();
      //  BentleyOttmann bo(&data);
      //  bo.DetermineIntersections();

      result.StartBulkLoad();  // ordered = true
      result.EndBulkLoad(true, false);
    }
  };

/*

1 data class for conversion from DLine to Line

*/
  class DLineToLine : public IntersectionAlgorithmData
  {
  private:
    const DLine& _source;
    size_t _currentSourceIndex;
    Line& _result;
    int _outputSegments;

  public:
    DLineToLine(const DLine& src, Line& result) :
      _source(src),
      _currentSourceIndex(0),
      _result(result),
      _outputSegments(0)
    {
    }

    ~DLineToLine()
    {
    }

    void InitializeFetch()
    {
      _currentSourceIndex = 0;
    }

    bool FetchInputHalfSegment(
      HalfSegment &segment,
      bool &belongsToSecondGeometry)
    {
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

    void OutputHalfSegment(
      const HalfSegment& segment,
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

        if (segment.x1 < minX) { minX = segment.x1; }
        if (segment.y1 < minY) { minY = segment.y1; }
        if (segment.x2 < minX) { minX = segment.x2; }
        if (segment.y2 < minY) { minY = segment.y2; }

        if (segment.x1 > maxX) { maxX = segment.x1; }
        if (segment.y1 > maxY) { maxY = segment.y1; }
        if (segment.x2 > maxX) { maxX = segment.x2; }
        if (segment.y2 > maxY) { maxY = segment.y2; }
      }

      return Rectangle<2>(true, minX, maxX, minY, maxY);
    }

    IntersectionAlgorithmCalculationType GetCalculationType()
    {
      return IntersectionAlgorithmCalculationType::CalulationTypeNone;
    }

    bool IsInputOrderedByX()
    {
      return false;
    }

    void GetRoundToDecimals(int& decimals, int& stepSize)
    {
      // because of AlmostEqual
      decimals = 7;
      stepSize = 2;
    }

    void OutputFinished()
    {
    }

    static void ToLine(const DLine& src, Line& result) {
      result.Clear();
      if (!src.IsDefined()) {
        result.SetDefined(false);
        return;
      }
      result.SetDefined(true);
      if (src.HashValue() == 0) {  // empty line, nothing to realminize
        return;
      }

      result.StartBulkLoad();
      DLineToLine data(src, result);
      Hobby hobby(&data);
      hobby.DetermineIntersections();

      result.StartBulkLoad();  // ordered = true
      result.EndBulkLoad(true, false);
    }
  };

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

    Point p0(false);      // starting point
    Point p1(false);      // end point of the first unit
    Point p_last(false);  // last point of the connected segment

    for (int i = 0; i < size; i++) {
      mpoint->Get(i, unit);

      if (!AlmostEqual(unit.p0, unit.p1)) {
        if (!p0.IsDefined()) {  // first unit
          p0 = unit.p0;
          p1 = unit.p1;
          p_last = unit.p1;
        } else {  // segment already exists
          if (p_last != unit.p0) {  // spatial jump
            hs.Set(true, p0, p_last);
            hs.attr.edgeno = ++edgeno;
            line += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            line += hs;
            p0 = unit.p0;
            p1 = unit.p1;
            p_last = unit.p1;
          } else {  // an extension, check direction
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

  void IntersectionLine(
    const Line& line1,
    const Line& line2,
    Line& result)
  {
    LineSetOp::SetOp(
      line1,
      line2,
      SetOpType::SetOpIntersection,
      result);
  }

  void IntersectionRegion(
    const Region& region1,
    const Region& region2,
    Region& result)
  {
    RegionSetOp::SetOp(
      region1, region2,
      SetOpType::SetOpIntersection,
      result);
  }

  void UnionLine(
    const Line& line1,
    const Line& line2,
    Line& result)
  {
    LineSetOp::SetOp(
      line1,
      line2,
      SetOpType::SetOpUnion,
      result);
  }

  void UnionRegion(
    const Region& region1,
    const Region& region2,
    Region& result)
  {
    RegionSetOp::SetOp(
      region1,
      region2,
      SetOpType::SetOpUnion,
      result);
  }

  void MinusLine(
    const Line& line1,
    const Line& line2,
    Line& result)
  {
    LineSetOp::SetOp(
      line1,
      line2,
      SetOpType::SetOpMinus,
      result);
  }

  void MinusRegion(
    const Region& region1,
    const Region& region2,
    Region& result)
  {
    RegionSetOp::SetOp(
      region1,
      region2,
      SetOpType::SetOpMinus,
      result);
  }

  void CrossingsLine(
    const Line& line1,
    const Line& line2,
    Points& result)
  {
    CrossingsData::Crossings(
      line1,
      line2,
      result);
  }

  void ToLine(
    const DLine& dline,
    Line& result)
  {
    DLineToLine::ToLine(dline, result);
  }

  int MPointTrajectory(
    Word* args,
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

  int SpatialIntersectionLine(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Line* arg1 = static_cast<Line*>(args[0].addr);
      Line* arg2 = static_cast<Line*>(args[1].addr);
      Line* res = static_cast<Line*>(result.addr);
      IntersectionLine(*arg1, *arg2, *res);
      return 0;
  }

  int SpatialIntersectionRegion(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Region* arg1 = static_cast<Region*>(args[0].addr);
      Region* arg2 = static_cast<Region*>(args[1].addr);
      Region* res = static_cast<Region*>(result.addr);
      IntersectionRegion(*arg1, *arg2, *res);
      return 0;
  }

  int SpatialUnionLine(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Line* arg1 = static_cast<Line*>(args[0].addr);
      Line* arg2 = static_cast<Line*>(args[1].addr);
      Line* res = static_cast<Line*>(result.addr);
      UnionLine(*arg1, *arg2, *res);
      return 0;
  }

  int SpatialUnionRegion(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Region* arg1 = static_cast<Region*>(args[0].addr);
      Region* arg2 = static_cast<Region*>(args[1].addr);
      Region* res = static_cast<Region*>(result.addr);
      UnionRegion(*arg1, *arg2, *res);
      return 0;
  }

  int SpatialMinusLine(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Line* arg1 = static_cast<Line*>(args[0].addr);
      Line* arg2 = static_cast<Line*>(args[1].addr);
      Line* res = static_cast<Line*>(result.addr);
      MinusLine(*arg1, *arg2, *res);
      return 0;
  }

  int SpatialMinusRegion(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Region* arg1 = static_cast<Region*>(args[0].addr);
      Region* arg2 = static_cast<Region*>(args[1].addr);
      Region* res = static_cast<Region*>(result.addr);
      MinusRegion(*arg1, *arg2, *res);
      return 0;
  }

  int CrossingsLine(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      Line* arg1 = static_cast<Line*>(args[0].addr);
      Line* arg2 = static_cast<Line*>(args[1].addr);
      Points* res = static_cast<Points*>(result.addr);
      CrossingsLine(*arg1, *arg2, *res);
      return 0;
  }

  int ToLine(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s) {
      result = qp->ResultStorage(s);
      DLine* arg1 = static_cast<DLine*>(args[0].addr);
      Line* res = static_cast<Line*>(result.addr);
      ToLine(*arg1, *res);

      return 0;
  }

  const string TemporalSpecTrajectory =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>mpoint -> line</text--->"
    "<text> trajectory2( _ )</text--->"
    "<text>Get the trajectory of the corresponding"
    " moving point object.</text--->"
    "<text>trajectory2( mp1 )</text--->"
    ") )";

  const string ToLineSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>dline -> line</text--->"
    "<text>toline( _ )</text--->"
    "<text>Converts a dline into a line</text--->"
    "<text>toline( dline )</text--->"
    ") )";

  const string SpatialIntersectionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line x line -> line || "
    "region x region -> region</text--->"
    "<text>intersection2(arg1, arg2)</text--->"
    "<text>intersection of two spatial objects</text--->"
    "<text>query intersection(tiergarten, thecenter) </text--->"
    ") )";

  const string SpatialMinusSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line x line -> line || "
    "region x region -> region</text--->"
    "<text>arg1 minus2 arg2</text--->"
    "<text>difference of two spatial objects</text--->"
    "<text>query tiergarten minus thecenter </text--->"
    ") )";

  const string SpatialUnionSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>line x line -> line || "
    "region x region -> region</text--->"
    "<text>arg1 union2 arg2</text--->"
    "<text>union of two spatial objects</text--->"
    "<text>query tiergarten union thecenter </text--->"
    ") )";

  const string SpatialSpecCrossings  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "( <text>(line x line) -> points</text--->"
    "<text>crossings( _, _ )</text--->"
    "<text>crossing points of two lines.</text--->"
    "<text>query crossings(line1, line2)</text--->"
    ") )";

  ValueMapping spatialintersectionVM[] = {
    SpatialIntersectionLine,
    SpatialIntersectionRegion,
  };

  ValueMapping spatialminusVM[] = {
    SpatialMinusLine,
    SpatialMinusRegion,
  };

  ValueMapping spatialunionVM[] = {
    SpatialUnionLine,
    SpatialUnionRegion,
  };

  int SpatialSetOpSelect(ListExpr args) {
    string a1 = nl->SymbolValue(nl->First(args));
    string a2 = nl->SymbolValue(nl->Second(args));

    if (a1 == Line::BasicType()) {
      if (a2 == Line::BasicType()) {
        return 0;
      }
      return -1;
    }

    if (a1 == Region::BasicType()) {
      if (a2 == Region::BasicType()) {
        return 1; }
      return -1;
    }

    return -1;
  }

  ListExpr MovingTypeMapSpatial(ListExpr args)
  {
    if (nl->ListLength(args)  == 1) {
      ListExpr arg1 = nl->First(args);

      if (nl->IsEqual(arg1, MPoint::BasicType())) {
        return nl->SymbolAtom(Line::BasicType());
      }
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr ToLineTypeMap(ListExpr args)
  {
    if (nl->ListLength(args) == 1)
    {
      ListExpr arg1 = nl->First(args);

      if (nl->IsEqual(arg1, DLine::BasicType())) {
        return nl->SymbolAtom(Line::BasicType());
      }
    }

    return nl->SymbolAtom(Symbol::TYPEERROR());
  }

  ListExpr SpatialUnionTypeMap(ListExpr args) {
    string err = "t1 x t2 expected, t_i in {line, region";
    if (nl->ListLength(args) != 2) {
      return listutils::typeError(err + ": wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (!listutils::isSymbol(arg1)) {
      return listutils::typeError(err+ ": first arg not a spatial type");
    }
    if (!listutils::isSymbol(arg2)) {
      return listutils::typeError(err+ ": second arg not a spatial type");
    }
    string a1 = nl->SymbolValue(arg1);
    string a2 = nl->SymbolValue(arg2);

    if (a1 == Line::BasicType()) {
      if (a2 == Line::BasicType()) {
        return nl->SymbolAtom(Line::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }
    if (a1 == Region::BasicType()) {
      if (a2 == Region::BasicType()) {
        return nl->SymbolAtom(Region::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }

    return listutils::typeError(err+ ": first arg not a spatial type");
  }

  ListExpr SpatialCrossingsTM(ListExpr args)
  {
    ListExpr arg1, arg2;
    if (nl->ListLength(args) == 2)
    {
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

  ListExpr SpatialMinusTypeMap(ListExpr args) {
    string err = "t1 x t2 expected, t_i in {line, region";
    if (nl->ListLength(args) != 2) {
      return listutils::typeError(err + ": wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (!listutils::isSymbol(arg1)) {
      return listutils::typeError(err+ ": first arg not a spatial type");
    }
    if (!listutils::isSymbol(arg2)) {
      return listutils::typeError(err+ ": second arg not a spatial type");
    }
    string a1 = nl->SymbolValue(arg1);
    string a2 = nl->SymbolValue(arg2);

    if (a1 == Line::BasicType()) {
      if (a2 == Line::BasicType()) {
        return nl->SymbolAtom(Line::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }

    if (a1 == Region::BasicType()) {
      if (a2 == Region::BasicType()) {
        return nl->SymbolAtom(Region::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }

    return listutils::typeError(err+ ": first arg not a spatial type");
  }

  ListExpr SpatialIntersectionTypeMap(ListExpr args) {
    string err = "t1 x t2 expected, t_i in {line, region";
    if (nl->ListLength(args) != 2) {
      return listutils::typeError(err + ": wrong number of arguments");
    }
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if (!listutils::isSymbol(arg1)) {
      return listutils::typeError(err+ ": first arg not a spatial type");
    }
    if (!listutils::isSymbol(arg2)) {
      return listutils::typeError(err+ ": second arg not a spatial type");
    }
    string a1 = nl->SymbolValue(arg1);
    string a2 = nl->SymbolValue(arg2);

    if (a1 == Line::BasicType()) {
      if (a2 == Line::BasicType()) {
        return nl->SymbolAtom(Line::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }

    if (a1 == Region::BasicType()) {
      if (a2 == Region::BasicType()) {
        return nl->SymbolAtom(Region::BasicType());
      }
      return listutils::typeError(err+ ": second arg not a spatial type");
    }
    return listutils::typeError(err+ ": first arg not a spatial type");
  }

  Operator temporaltrajectory2(
    "trajectory2",
    TemporalSpecTrajectory,
    MPointTrajectory,
    Operator::SimpleSelect,
    MovingTypeMapSpatial);

  Operator toline(
    "toline",
    ToLineSpec,
    ToLine,
    Operator::SimpleSelect,
    ToLineTypeMap);

  Operator spatialcrossings2(
    "crossings2",
    SpatialSpecCrossings,
    CrossingsLine,
    Operator::SimpleSelect,
    SpatialCrossingsTM);

  Operator spatialintersection2(
    "intersection2",
    SpatialIntersectionSpec,
    2,
    spatialintersectionVM,
    SpatialSetOpSelect,
    SpatialIntersectionTypeMap);

  Operator spatialminus2(
    "minus2",
    SpatialMinusSpec,
    2,
    spatialminusVM,
    SpatialSetOpSelect,
    SpatialMinusTypeMap);

  Operator spatialunion2(
    "union2",
    SpatialUnionSpec,
    2,
    spatialunionVM,
    SpatialSetOpSelect,
    SpatialUnionTypeMap);

  RobustPlaneSweepAlgebra::RobustPlaneSweepAlgebra()
  {
    AddOperator(&temporaltrajectory2);
    AddOperator(&toline);
    AddOperator(&spatialintersection2);
    AddOperator(&spatialminus2);
    AddOperator(&spatialunion2);
    AddOperator(&spatialcrossings2);
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
