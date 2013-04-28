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

#include <stdexcept>
#include <vector>

#include "../Algorithm/Hobby.h"
#ifdef RPS_TEST
#include "../Helper/SpatialAlgebraStubs.h"
#else
#include "SpatialAlgebra.h"
#endif
#include "../Helper/TestDataGenerator.h"
#include "../Helper/LineSegmentComparer.h"

using namespace std;
using namespace RobustPlaneSweep;

enum SetOpType
{
  SetOpUnion = 1,
  SetOpIntersection = 2,
  SetOpMinus = 3
};

class RegionVectorData : public IntersectionAlgorithmData
{
private:
  vector<HalfSegment>* _input;
  vector<HalfSegment>* _output;
  vector<HalfSegment>::iterator _inputIterator;
  int _outputSegments;
  int _roundToDecimals;
  InternalPointTransformation* _transformation;
  SetOpType _setOpType;

public:
  RegionVectorData(
    vector<HalfSegment>* input,
    SetOpType setOpType,
    int roundToDecimals,
    InternalPointTransformation* transformation)
  {
    _input = input;
    _setOpType = setOpType;
    _output = new vector<HalfSegment>();
    _outputSegments = 0;
    _roundToDecimals = roundToDecimals;
    _transformation = transformation;
  }

  ~RegionVectorData()
  {
    if (_output != NULL) {
      delete _output;
      _output = NULL;
    }
  }

  IntersectionAlgorithmCalculationType GetCalculationType()
  {
    return IntersectionAlgorithmCalculationType::CalulationTypeRegion;
  }

  void InitializeFetch()
  {
    _inputIterator = _input->begin();
  }

  bool FetchInputHalfSegment(
    HalfSegment &segment,
    bool &belongsToSecondGeometry)
  {
    if (_inputIterator == _input->end()) {
      return false;
    } else {
      segment = *_inputIterator++;
      belongsToSecondGeometry = (segment.attr.faceno == 1);
      return true;
    }
  }

  void OutputHalfSegment(
    const HalfSegment& segment,
    const InternalAttribute& a)
  {
    bool include;
    bool insideAbove = false;

    switch (_setOpType) {
    case SetOpType::SetOpUnion:
      include = a.IsInUnionRegion(insideAbove);
      break;

    case SetOpType::SetOpIntersection:
      include = a.IsInIntersectionRegion(insideAbove);
      break;

    case SetOpType::SetOpMinus:
      include = a.IsInMinusRegion(insideAbove);
      break;

    default:
      throw new std::logic_error("setOpType");
    }

    if (!include) {
      return;
    }

    HalfSegment s1 = segment;
    s1.SetLeftDomPoint(true);
    s1.attr.insideAbove = insideAbove;
    s1.attr.edgeno = _outputSegments;
    _output->push_back(s1);
    _outputSegments++;
  }

  const Rectangle<2> GetBoundingBox()
  {
    double minX = 1e300;
    double maxX = -1e300;
    double minY = 1e300;
    double maxY = -1e300;

    bool foundSegments = false;

    for (vector<HalfSegment>::iterator
      i = _input->begin(); i != _input->end(); ++i) {
        foundSegments = true;
        const HalfSegment& segment = *i;
        if (segment.GetDomPoint().GetX() < minX) {
          minX = segment.GetDomPoint().GetX();
        }
        if (segment.GetDomPoint().GetY() < minY) {
          minY = segment.GetDomPoint().GetY();
        }

        if (segment.GetDomPoint().GetX() > maxX) {
          maxX = segment.GetDomPoint().GetX();
        }
        if (segment.GetDomPoint().GetY() > maxY) {
          maxY = segment.GetDomPoint().GetY();
        }
    }

    if (foundSegments) {
      return Rectangle<2>(true, minX, maxX, minY, maxY);
    } else {
      return Rectangle<2>(false, 0, 0, 0, 0);
    }
  }

  bool IsInputOrderedByX()
  {
    return false;
  }

  void GetRoundToDecimals(int& decimals, int& stepSize)
  {
    decimals = _roundToDecimals;
    stepSize = 1;
  }

  void OutputFinished()
  {
    sort(_output->begin(), _output->end(), HalfSegment::Less);
  }

  vector<HalfSegment>* GetResult()
  {
    std::vector<HalfSegment>* result = _output;
    _output = NULL;
    return result;
  }
};

Point Flip(const Point& point, bool x, bool y)
{
  return Point(
    true,
    (x ? -point.GetX() : point.GetX()),
    (y ? -point.GetY() : point.GetY()));
}

Point Rotate(const Point& point, int angle)
{
  switch (angle) {
  case 0:
    return point;

  case 1:
    return Point(true, -point.GetY(), point.GetX());

  case 2:
    return Point(true, -point.GetX(), -point.GetY());

  case 3:
    return Point(true, point.GetY(), -point.GetX());

  default:
    throw new std::invalid_argument("invalid angle");
  }
}

HalfSegment Flip(
  const HalfSegment& segment,
  bool x,
  bool y,
  bool flipIndex)
{
  bool insideAbove = segment.attr.insideAbove;
  if (!x && !y) {
  } else {
    if (y && segment.GetLeftPoint().GetX() != segment.GetRightPoint().GetX()) {
      insideAbove = !insideAbove;
    }

    if (x && segment.GetLeftPoint().GetX() == segment.GetRightPoint().GetX()) {
      insideAbove = !insideAbove;
    }
  }

  HalfSegment result = HalfSegment(
    true,
    Flip(segment.GetLeftPoint(), x, y),
    Flip(segment.GetRightPoint(), x, y));

  result.attr.insideAbove = insideAbove;
  result.attr.edgeno = segment.attr.edgeno;
  result.attr.faceno = (flipIndex?1-segment.attr.faceno:segment.attr.faceno);

  return result;
}

HalfSegment Rotate(const HalfSegment& segment, int angle)
{
  bool insideAbove = segment.attr.insideAbove;

  Point s = segment.GetLeftPoint();
  Point e = segment.GetRightPoint();

  switch (angle) {
  case 0:
    break;

  case 1:
    if (s.GetY() < e.GetY() || s.GetX() == e.GetX()) {
      insideAbove = !insideAbove;
    }
    break;

  case 2:
    insideAbove = !insideAbove;
    break;

  case 3:
    if (s.GetY() >= e.GetY() && s.GetX() != e.GetX()) {
      insideAbove = !insideAbove;
    }
    break;

  default:
    throw new std::invalid_argument("invalid angle");
  }

  HalfSegment result = HalfSegment(
    true,
    Rotate(s, angle),
    Rotate(e, angle));

  result.attr.insideAbove = insideAbove;
  result.attr.edgeno = segment.attr.edgeno;
  result.attr.faceno = segment.attr.faceno;

  return result;
}

vector<HalfSegment> FlipRotate(
  const vector<HalfSegment>& source,
  bool x,
  bool y,
  bool flipIndex,
  int angle)
{
  vector<HalfSegment> result;

  for (vector<HalfSegment>::const_iterator segment = source.begin();
    segment != source.end(); segment++) {
      result.push_back(Rotate(Flip(*segment, x, y, flipIndex), angle));
  }

  return result;
}

vector<vector<HalfSegment>> FlipRotate(
  const vector<vector<HalfSegment>>& source,
  bool x,
  bool y,
  bool flipIndex,
  int angle)
{
  vector<vector<HalfSegment>> result;

  for (vector<vector<HalfSegment>>::const_iterator sourceList = source.begin();
    sourceList != source.end(); sourceList++) {
      result.push_back(FlipRotate(*sourceList, x, y, flipIndex, angle));
  }

  return result;
}

void TestInternal(
  vector<HalfSegment> source,
  SetOpType type,
  int decimals,
  InternalPointTransformation* transformation,
  const vector<vector<HalfSegment>>& expectedResults)
{
  RegionVectorData v(&source, type, decimals, transformation);
  Hobby ho(&v);
  if (transformation != NULL) {
    ho.SetTransformation(transformation);
  }

  ho.DetermineIntersections();
  vector<HalfSegment>* actualResult = v.GetResult();

  bool isEqual = false;

  for (vector<vector<HalfSegment>>::const_iterator
    expectedResult = expectedResults.begin();
    expectedResult != expectedResults.end(); expectedResult++) {
      LineSegmentComparer comparer(
        actualResult->begin(),
        actualResult->end(),
        expectedResult->begin(),
        expectedResult->end());
      if (comparer.IsEqual()) {
        isEqual = true;
        break;
      }
  }

  if (!isEqual) {
    throw std::logic_error("region test failed!");
  }

  delete actualResult;
}

void TestInternal(
  vector<HalfSegment> source,
  int decimals,
  InternalPointTransformation* transformation,
  const vector<vector<HalfSegment>>& unionResults,
  const vector<vector<HalfSegment>>& intersectionResults,
  const vector<vector<HalfSegment>>& differenceResults,
  const vector<vector<HalfSegment>>& differenceRevResults)
{
  for (int angle = 0; angle < 4; angle++) {
    for (int g = 0; g < 2; g++) {
      for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 2; y++) {
          TestInternal(
            FlipRotate(source, x == 1, y == 1, g == 1, angle),
            SetOpType::SetOpUnion,
            decimals,
            transformation,
            FlipRotate(unionResults, x == 1, y == 1, g == 1, angle));

          TestInternal(
            FlipRotate(source, x == 1, y == 1, g == 1, angle),
            SetOpType::SetOpIntersection,
            decimals,
            transformation,
            FlipRotate(intersectionResults, x == 1, y == 1, g == 1, angle));

          if (g == 0) {
            TestInternal(
              FlipRotate(source, x == 1, y == 1, g == 1, angle),
              SetOpType::SetOpMinus,
              decimals,
              transformation,
              FlipRotate(differenceResults, x == 1, y == 1, false, angle));
          } else {
            TestInternal(
              FlipRotate(source, x == 1, y == 1, g == 1, angle),
              SetOpType::SetOpMinus,
              decimals,
              transformation,
              FlipRotate(differenceRevResults, x == 1, y == 1, false, angle));
          }
        }
      }
    }
  }
}

void TestInternal(
  vector<HalfSegment> source,
  int decimals,
  InternalPointTransformation* transformation,
  const vector<HalfSegment>& unionResult,
  const vector<HalfSegment>& intersectionResult,
  const vector<HalfSegment>& differenceResult,
  const vector<HalfSegment>& differenceRevResult)
{
  vector<vector<HalfSegment>> unionResults;
  vector<vector<HalfSegment>> intersectionResults;
  vector<vector<HalfSegment>> differenceResults;
  vector<vector<HalfSegment>> differenceRevResults;

  unionResults.push_back(unionResult);
  intersectionResults.push_back(intersectionResult);
  differenceResults.push_back(differenceResult);
  differenceRevResults.push_back(differenceRevResult);

  TestInternal(
    source,
    decimals,
    transformation,
    unionResults,
    intersectionResults,
    differenceResults,
    differenceRevResults);
}

HalfSegment CreateHalfSegment(
  int geometryIndex,
  int edgoNo,
  double x0,
  double y0,
  double x1,
  double y1,
  bool insideAbove)
{
  HalfSegment result = HalfSegment(
    true,
    Point(true, x0, y0),
    Point(true, x1, y1));

  result.attr.faceno = geometryIndex;
  result.attr.edgeno = edgoNo;
  result.attr.insideAbove = insideAbove;

  return result;
}

/*

  Test 1

*/
void RegionTest1()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 7, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 14, 11, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 12, 10, false));
  s->push_back(CreateHalfSegment(1, 12, 12, 10, 13, 8, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12.9, 8.3, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 3, 12.9, 8.3, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 13.2, 8.1, true));
  s->push_back(CreateHalfSegment(0, 5, 13.2, 8.1, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 6, 14, 11, 14.4, 9.4, false));
  s->push_back(CreateHalfSegment(0, 7, 14.4, 9.4, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 8, 14.6, 8.5, 15, 7, false));
  s->push_back(CreateHalfSegment(0, 9, 14.6, 8.5, 16, 9, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 12, 10, 12.9, 8.3, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14.4, 9.4, false));
  s->push_back(CreateHalfSegment(0, 3, 12.9, 8.3, 13.2, 8.1, true));
  s->push_back(CreateHalfSegment(0, 4, 13.2, 8.1, 14.6, 8.5, true));
  s->push_back(CreateHalfSegment(0, 5, 14.4, 9.4, 14.6, 8.5, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12.9, 8.3, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 12.9, 8.3, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14.4, 9.4, true));
  s->push_back(CreateHalfSegment(0, 5, 13.2, 8.1, 14.6, 8.5, false));
  s->push_back(CreateHalfSegment(0, 6, 13.2, 8.1, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 7, 14, 11, 14.4, 9.4, false));
  s->push_back(CreateHalfSegment(0, 8, 14.6, 8.5, 15, 7, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 12.9, 8.3, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 12.9, 8.3, 13.2, 8.1, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 13.2, 8.1, true));
  s->push_back(CreateHalfSegment(0, 4, 14.4, 9.4, 14.6, 8.5, true));
  s->push_back(CreateHalfSegment(0, 5, 14.4, 9.4, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 14.6, 8.5, 16, 9, true));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 2

*/
void RegionTest2()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 1, 14, 7, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 11, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 12, 10, false));
  s->push_back(CreateHalfSegment(1, 12, 12, 10, 13, 8, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 3, 14, 7, 14, 8.3, true));
  s->push_back(CreateHalfSegment(0, 4, 14, 8.3, 16, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 9.5, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 6, 14, 9.5, 16, 9, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 12, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14, 9.5, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 14, 8.3, true));
  s->push_back(CreateHalfSegment(0, 4, 14, 8.3, 14, 9.5, true));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 8, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14, 9.5, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 8, 14, 8.3, false));
  s->push_back(CreateHalfSegment(0, 6, 14, 7, 14, 8.3, true));
  s->push_back(CreateHalfSegment(0, 7, 14, 9.5, 14, 11, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 8.3, 14, 9.5, false));
  s->push_back(CreateHalfSegment(0, 2, 14, 8.3, 16, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9.5, 16, 9, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 3

*/
void RegionTest3()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 13, 8, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 14.2, 8.4, 15, 6, false));
  s->push_back(CreateHalfSegment(0, 6, 14.2, 8.4, 16, 9, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 8, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 8, 14.2, 8.4, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14.2, 8.4, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 14.2, 8.4, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 14.2, 8.4, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 14.2, 8.4, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 14.2, 8.4, 16, 9, true));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 4

*/
void RegionTest4()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 14.5, 7.5, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 14.5, 7.5, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 14.5, 7.5, 15, 6, false));
  s->push_back(CreateHalfSegment(0, 6, 14.5, 7.5, 16, 9, true));

  s = &intersectionResult;

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 14.5, 7.5, false));
  s->push_back(CreateHalfSegment(0, 5, 14.5, 7.5, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 14.5, 7.5, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 14.5, 7.5, 16, 9, true));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 5

*/
void RegionTest5()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 14.5, 7.5, 13, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 14.5, 7.5, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 14.5, 7.5, false));
  s->push_back(CreateHalfSegment(0, 5, 14.5, 7.5, 15, 6, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 9, 14.5, 7.5, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14.5, 7.5, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 9, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 9, 14.5, 7.5, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 14.5, 7.5, 15, 6, false));

  s = &differenceRevResult;

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test Circle 4

*/
void RegionTestCircle4()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 5, 5, 10, true));
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 15, 15, 10, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 10, 10, 5, true));
  s->push_back(CreateHalfSegment(1, 10, 13, 5, 8, 10, true));
  s->push_back(CreateHalfSegment(1, 11, 8, 10, 13, 15, false));
  s->push_back(CreateHalfSegment(1, 12, 13, 15, 18, 10, false));
  s->push_back(CreateHalfSegment(1, 13, 18, 10, 13, 5, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 10, 5, true));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 3, 10, 5, 11.5, 6.5, true));
  s->push_back(CreateHalfSegment(0, 4, 10, 15, 11.5, 13.5, false));
  s->push_back(CreateHalfSegment(0, 5, 11.5, 6.5, 13, 5, true));
  s->push_back(CreateHalfSegment(0, 6, 11.5, 13.5, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 7, 13, 5, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 8, 13, 15, 18, 10, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 10, 11.5, 6.5, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 10, 11.5, 13.5, false));
  s->push_back(CreateHalfSegment(0, 3, 11.5, 6.5, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 4, 11.5, 13.5, 15, 10, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 10, 5, true));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 3, 8, 10, 11.5, 6.5, false));
  s->push_back(CreateHalfSegment(0, 4, 8, 10, 11.5, 13.5, true));
  s->push_back(CreateHalfSegment(0, 5, 10, 5, 11.5, 6.5, true));
  s->push_back(CreateHalfSegment(0, 6, 10, 15, 11.5, 13.5, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 11.5, 6.5, 13, 5, true));
  s->push_back(CreateHalfSegment(0, 2, 11.5, 6.5, 15, 10, false));
  s->push_back(CreateHalfSegment(0, 3, 11.5, 13.5, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 11.5, 13.5, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 5, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 6, 13, 15, 18, 10, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test Circle 6

*/
void RegionTestCircle6()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 5, 10, 7.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 2, 7.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 12.5, 14.3, 15, 10, false));
  s->push_back(CreateHalfSegment(0, 4, 12.5, 5.6, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 5, 7.5, 5.6, 12.5, 5.6, true));
  s->push_back(CreateHalfSegment(1, 10, 8, 10, 10.5, 5.6, true));
  s->push_back(CreateHalfSegment(1, 11, 8, 10, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(1, 12, 10.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(1, 13, 15.5, 14.3, 18, 10, false));
  s->push_back(CreateHalfSegment(1, 14, 15.5, 5.6, 18, 10, true));
  s->push_back(CreateHalfSegment(1, 15, 10.5, 5.6, 15.5, 5.6, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 7.5, 5.6, 10.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 4, 7.5, 14.3, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 5, 10.5, 5.6, 12.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 6, 10.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 7, 12.5, 5.6, 15.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 8, 12.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 9, 15.5, 5.6, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 10, 15.5, 14.3, 18, 10, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 10, 10.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 10, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 10.5, 5.6, 12.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 4, 10.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 5, 12.5, 5.6, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 6, 12.5, 14.3, 15, 10, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 7.5, 5.6, 10.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 4, 7.5, 14.3, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 5, 8, 10, 10.5, 5.6, false));
  s->push_back(CreateHalfSegment(0, 6, 8, 10, 10.5, 14.3, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 12.5, 5.6, 15, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 12.5, 5.6, 15.5, 5.6, true));
  s->push_back(CreateHalfSegment(0, 3, 12.5, 14.3, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 4, 12.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 5, 15.5, 5.6, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 6, 15.5, 14.3, 18, 10, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 6

*/
void RegionTest6()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 16, 3, 15, 9, false));
  s->push_back(CreateHalfSegment(1, 11, 15, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 16, 3, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 15, 6, 16, 3, true));
  s->push_back(CreateHalfSegment(0, 6, 15, 9, 16, 3, false));

  s = &intersectionResult;

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 6, 16, 3, true));
  s->push_back(CreateHalfSegment(0, 4, 15, 9, 16, 3, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 7

*/
void RegionTest7()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 11, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 16, 3, 15, 9, false));
  s->push_back(CreateHalfSegment(1, 11, 15, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 16, 3, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 11, 15, 6, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 6, 16, 3, true));
  s->push_back(CreateHalfSegment(0, 7, 15, 9, 16, 3, false));

  s = &intersectionResult;

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 11, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 6, 16, 3, true));
  s->push_back(CreateHalfSegment(0, 4, 15, 9, 16, 3, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 8

*/
void RegionTest8()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 6, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 13, 12, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 13, 8, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 14.2, 8.4, 15, 6, false));
  s->push_back(CreateHalfSegment(0, 6, 14.2, 8.4, 16, 9, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 8, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 8, 14.2, 8.4, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14.2, 8.4, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 14.2, 8.4, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 14.2, 8.4, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 14.2, 8.4, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 16, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 14.2, 8.4, 16, 9, true));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 9

*/
void RegionTest9()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 2, 7.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 12.5, 14.3, 15, 10, false));
  s->push_back(CreateHalfSegment(0, 5, 5, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(1, 11, 8, 10, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(1, 12, 10.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(1, 13, 15.5, 14.3, 18, 10, false));
  s->push_back(CreateHalfSegment(1, 15, 8, 10, 18, 10, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 8, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 7.5, 14.3, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 4, 8, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 5, 10.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 6, 12.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 7, 15, 10, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 8, 15.5, 14.3, 18, 10, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 10, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 2, 8, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10.5, 14.3, 12.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 4, 12.5, 14.3, 15, 10, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 10, 7.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 2, 5, 10, 8, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 7.5, 14.3, 10.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 4, 8, 10, 10.5, 14.3, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 12.5, 14.3, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 2, 12.5, 14.3, 15.5, 14.3, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 10, 18, 10, true));
  s->push_back(CreateHalfSegment(0, 4, 15.5, 14.3, 18, 10, false));

  TestInternal(
    source,
    1,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 10

*/
void RegionTest10()
{
  vector<HalfSegment> source;
  vector<vector<HalfSegment>> unionResult;
  vector<vector<HalfSegment>> intersectionResult;
  vector<vector<HalfSegment>> differenceResult;
  vector<vector<HalfSegment>> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 12, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 16, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 14, 9, 13, 8, false));

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 14, 8, 15, 6, false));
  s->push_back(CreateHalfSegment(0, 6, 14, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(0, 7, 14, 9, 16, 9, false));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 15, 6, 14, 8, false));
  s->push_back(CreateHalfSegment(0, 2, 15, 6, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 8, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 8, 10, 10, true));
  s->push_back(CreateHalfSegment(0, 6, 13, 12, 10, 10, false));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 13, 8, 14, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 13, 8, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14, 8, true));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 14, 8, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 8, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 6, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 14, 8, 15, 6, false));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 12, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 12, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 9, 14, 8, true));
  s->push_back(CreateHalfSegment(0, 6, 14, 8, 15, 6, false));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 14, 8, false));
  s->push_back(CreateHalfSegment(0, 2, 14, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 16, 9, false));
  differenceRevResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  differenceRevResult.push_back(*s);
  delete s;

  TestInternal(
    source,
    0,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 11

*/
void RegionTest11()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15.2, 10, true));
  s->push_back(CreateHalfSegment(0, 1, 15.2, 10, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 15, 15, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(1, 11, 18, 13, 18, 18, true));
  s->push_back(CreateHalfSegment(1, 12, 18, 18, 13, 18, false));
  s->push_back(CreateHalfSegment(1, 13, 13, 18, 13, 13, false));

  s=&unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 7, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 8, 18, 13, 18, 18, true));

  s=&intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 13, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 13, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 15, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, true));

  s=&differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 13, 13, 15, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 13, 15, 13, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));

  s=&differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 15, 15, 15, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 5, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 6, 18, 13, 18, 18, true));

  InternalPointTransformation transformation(-10000, -10000, 2, 0, 1);

  TestInternal(
    source,
    1,
    &transformation,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 12

*/
void RegionTest12()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 14.8, 10, true));
  s->push_back(CreateHalfSegment(0, 1, 14.8, 10, 15, 15, true));
  s->push_back(CreateHalfSegment(0, 2, 15, 15, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 10, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(1, 11, 18, 13, 18, 18, true));
  s->push_back(CreateHalfSegment(1, 12, 18, 18, 13, 18, false));
  s->push_back(CreateHalfSegment(1, 13, 13, 18, 13, 13, false));

  s=&unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 7, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 8, 18, 13, 18, 18, true));

  s=&intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 13, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 13, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 15, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, true));

  s=&differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 13, 13, 15, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 13, 15, 13, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));

  s=&differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 15, 15, 15, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 5, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 6, 18, 13, 18, 18, true));

  InternalPointTransformation transformation(-10000, -10000, 2, 0, 1);

  TestInternal(
    source,
    1,
    &transformation,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  Test 12

*/
void RegionTest13()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 8, 20, 9.7, 10, true));
  s->push_back(CreateHalfSegment(0, 1, 8, 20, 9, 20, false));
  s->push_back(CreateHalfSegment(0, 2, 9, 20, 9.7, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 10, 11, 10, 12, true));
  s->push_back(CreateHalfSegment(1, 11, 9, 12, 10, 12, false));
  s->push_back(CreateHalfSegment(1, 12, 9, 11, 9, 12, false));
  s->push_back(CreateHalfSegment(1, 13, 9, 11, 10, 11, true));

  s=&unionResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 20, 9, 12, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 20, 9, 20, false));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 9, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 9, 11, 10, 11, true));
  s->push_back(CreateHalfSegment(0, 5, 9, 20, 10, 12, false));
  s->push_back(CreateHalfSegment(0, 6, 10, 11, 10, 12, true));

  s=&intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 9, 12, 9, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 9, 11, 10, 11, true));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 10, 12, false));
  s->push_back(CreateHalfSegment(0, 4, 10, 11, 10, 12, true));

  s=&differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 20, 9, 12, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 20, 9, 20, false));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 10, 12, true));
  s->push_back(CreateHalfSegment(0, 4, 9, 20, 10, 12, false));

  s=&differenceRevResult;

  TestInternal(
    source,
    0,
    NULL,
    unionResult,
    intersectionResult,
    differenceResult ,
    differenceRevResult);
}

/*

  region test method

*/
void RegionTest()
{
  RegionTest1();
  RegionTest2();
  RegionTest3();
  RegionTest4();
  RegionTestCircle4();
  RegionTestCircle6();
  RegionTest5();
  RegionTest6();
  RegionTest7();
  RegionTest8();
  RegionTest9();
  RegionTest10();
  RegionTest11();
  RegionTest12();
  RegionTest13();
}

