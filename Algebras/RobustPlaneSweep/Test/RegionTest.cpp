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

[1] Implementation file for the region tests

[TOC]

1 Overview

This file contains all classes and methods required for the region tests.

This file is not required for SECONDO. It is only used inside the test project.

1 Includes

*/

#include <stdexcept>
#include <vector>
#include <unordered_map>

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

/*

1 Enum class ~SetOpType~

*/
enum class SetOpType
{
  Union = 1,
  Intersection = 2,
  Minus = 3
};

/*

1 Class ~RegionVectorData~

*/
class RegionVectorData : public IntersectionAlgorithmData
{
/*

1.1 Member variables

*/
private:
  vector<HalfSegment>* _input;
  vector<HalfSegment>* _output;
  vector<HalfSegment>::iterator _inputIterator;
  int _outputSegments;
  int _roundToDecimals;
  InternalPointTransformation* _transformation;
  SetOpType _setOpType;
  bool _reverseFirstSecond;
  bool _emitRightDomSegment;
  bool _testIntersectionOnly;
  bool _intersects;

public:
/*

1.1 Constructors

*/
  RegionVectorData(vector<HalfSegment>* input,
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
    _reverseFirstSecond = false;
    _emitRightDomSegment = false;
    _testIntersectionOnly = false;
    _intersects = false;
  }

  RegionVectorData(vector<HalfSegment>* input,
                   SetOpType setOpType,
                   int roundToDecimals,
                   InternalPointTransformation* transformation,
                   bool reverseFirstSecond,
                   bool emitRightDomSegment,
                   bool testIntersectionOnly)
  {
    _input = input;
    _setOpType = setOpType;
    _output = new vector<HalfSegment>();
    _outputSegments = 0;
    _roundToDecimals = roundToDecimals;
    _transformation = transformation;
    _reverseFirstSecond = reverseFirstSecond;
    _emitRightDomSegment = emitRightDomSegment;
    _testIntersectionOnly = testIntersectionOnly;
    _intersects = false;
  }

/*

1.1 Destructor

*/
  ~RegionVectorData()
  {
    if (_output != NULL) {
      delete _output;
      _output = NULL;
    }
  }

/*

1.1 ~FirstGeometryIsRegion~

*/
  bool FirstGeometryIsRegion() const
  {
    return true;
  }

/*

1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return true;
  }

/*

1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
    sort(_input->begin(), _input->end(), HalfSegment::Less);
    _inputIterator = _input->begin();
  }

/*

1.1 ~FetchInput~

*/
  bool FetchInput(HalfSegment &segment,
                  Point& point,
                  bool &belongsToSecondGeometry)
  {
    if (_inputIterator == _input->end()) {
      return false;
    } else {
      segment = *_inputIterator++;
      point.SetDefined(false);
      belongsToSecondGeometry = (segment.attr.faceno == 1);
      if (_reverseFirstSecond) {
        belongsToSecondGeometry = !belongsToSecondGeometry;
      }

      return true;
    }
  }

/*

1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return !_testIntersectionOnly;
  }

/*

1.1 ~OnGeometryIntersectionFound~

*/
  bool OnGeometryIntersectionFound()
  {
    _intersects = true;
    return _testIntersectionOnly;
  }

/*

1.1 ~Intersects~

*/
  bool Intersects()
  {
    return _intersects;
  }

/*

1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
                         const InternalAttribute& a)
  {
    bool include;
    bool insideAbove = false;

    switch (_setOpType) {
      case SetOpType::Union:
        include = a.IsInUnionRegion(insideAbove);
        break;

      case SetOpType::Intersection:
        include = a.IsInIntersectionRegion(insideAbove);
        break;

      case SetOpType::Minus:
        include = a.IsInMinusRegion(insideAbove);
        break;

      default:
        throw new logic_error("setOpType");
    }

    if (!include) {
      return;
    }

    HalfSegment s1 = segment;
    s1.SetLeftDomPoint(true);
    s1.attr.insideAbove = insideAbove;
    s1.attr.edgeno = _outputSegments;
    _output->push_back(s1);

    if (_emitRightDomSegment) {
      HalfSegment s2 = s1;
      s2.SetLeftDomPoint(false);
      _output->push_back(s2);
    }

    _outputSegments++;
  }

/*

1.1 ~GetBoundingBox~

*/
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

/*

1.1 ~GetRoundToDecimals~

*/
  void GetRoundToDecimals(int& decimals, int& stepSize) const
  {
    decimals = _roundToDecimals;
    stepSize = 1;
  }

/*

1.1 ~OutputFinished~

*/
  void OutputFinished()
  {
    sort(_output->begin(), _output->end(), HalfSegment::Less);
  }

/*

1.1 ~GetResult~

*/
  vector<HalfSegment>* GetResult()
  {
    vector<HalfSegment>* result = _output;
    _output = NULL;
    return result;
  }
};

/*

1 Class ~RegionHelper~

*/
class RegionHelper
{
private:
/*

1.1 Private Point comparer 

*/
  struct PointComparer
  {
    size_t operator()(const Point &x) const
    {
      return ((size_t)(x.GetX())) ^ ((size_t)(x.GetY()));
    }

    bool operator()(const Point &x, const Point &y) const
    {
      return x.GetX() == y.GetX() && x.GetY() == y.GetY();
    }
  };

/*

1.1 Member variables

*/
  typedef unordered_map<Point, vector<HalfSegment>*, PointComparer,
      PointComparer>
  graphtype;

  graphtype _graph;

/*

1.1 ~BuildGraph~

*/
  void BuildGraph(vector<HalfSegment>* segments)
  {
    for (vector<HalfSegment>::const_iterator segment = segments->begin();
        segment != segments->end();
        segment++) {
      if (segment->IsLeftDomPoint()) {
        for (int ldp = 0; ldp < 2; ldp++) {
          Point p = (ldp == 0 ? segment->GetLeftPoint() :
                                segment->GetRightPoint());

          vector<HalfSegment>* list;
          graphtype::const_iterator findResult = _graph.find(p);
          if (findResult != _graph.end()) {
            list = findResult->second;
          } else {
            list = new vector<HalfSegment>();
            _graph[p] = list;
          }
          list->push_back(*segment);
        }
      }
    }
  }

public:
/*

1.1 Constructor

*/
  RegionHelper()
  {
  }

/*

1.1 ~CanExtractRegions~

*/
  bool CanExtractRegions(vector<HalfSegment>* segments)
  {
    BuildGraph(segments);

    while (!_graph.empty()) {
      HalfSegment currentSegment = *(_graph.begin()->second->begin());

      vector<HalfSegment> ring;

      Point startPoint = currentSegment.GetLeftPoint();

      Point currentPoint = startPoint;
      for (;;) {
        ring.push_back(currentSegment);

        for (int ldp = 0; ldp < 2; ldp++) {
          Point p = (ldp == 0 ? currentSegment.GetLeftPoint() :
                                currentSegment.GetRightPoint());

          graphtype::const_iterator findResult = _graph.find(p);

          vector<HalfSegment>* list = findResult->second;

          bool foundEdge = false;
          for (vector<HalfSegment>::iterator temp = list->begin();
              temp != list->end(); temp++) {
            if (temp->attr.edgeno == currentSegment.attr.edgeno) {
              list->erase(temp);
              if (list->empty()) {
                delete list;
                _graph.erase(p);
              }
              foundEdge = true;
              break;
            }
          }

          if (!foundEdge) {
            return false;
          }
        }

        if (currentPoint == currentSegment.GetLeftPoint()) {
          currentPoint = currentSegment.GetRightPoint();
        } else {
          currentPoint = currentSegment.GetLeftPoint();
        }

        if (startPoint == currentPoint) {
          break;
        }

        graphtype::const_iterator nextSegment = _graph.find(currentPoint);
        if (nextSegment == _graph.end()) {
          return false;
        }

        currentSegment = *(nextSegment->second->begin());
      }
    }

    return true;
  }
};

/*

1 Helper methods

1.1 ~Flip~

*/
Point Flip(const Point& point, bool x, bool y)
{
  return Point(true,
               (x ? -point.GetX() : point.GetX()),
               (y ? -point.GetY() : point.GetY()));
}

/*

1.1 ~Rotate~

*/
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
      throw new invalid_argument("invalid angle");
  }
}

/*

1.1 ~Flip~

*/
HalfSegment Flip(const HalfSegment& segment,
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

  HalfSegment result = HalfSegment(true,
                                   Flip(segment.GetLeftPoint(), x, y),
                                   Flip(segment.GetRightPoint(), x, y));

  result.attr.insideAbove = insideAbove;
  result.attr.edgeno = segment.attr.edgeno;
  result.attr.faceno =
      (flipIndex ? 1 - segment.attr.faceno : segment.attr.faceno);

  return result;
}

/*

1.1 ~Rotate~

*/
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
      throw new invalid_argument("invalid angle");
  }

  HalfSegment result = HalfSegment(true,
                                   Rotate(s, angle),
                                   Rotate(e, angle));

  result.attr.insideAbove = insideAbove;
  result.attr.edgeno = segment.attr.edgeno;
  result.attr.faceno = segment.attr.faceno;

  return result;
}

/*

1.1 ~FlipRotate~

*/
vector<HalfSegment> FlipRotate(const vector<HalfSegment>& source,
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

vector<vector<HalfSegment>>
FlipRotate(const vector<vector<HalfSegment>>& source,
           bool x,
           bool y,
           bool flipIndex,
           int angle)
{
  vector < vector < HalfSegment >> result;

  for (vector<vector<HalfSegment>>::const_iterator sourceList = source.begin();
      sourceList != source.end(); sourceList++) {
    result.push_back(FlipRotate(*sourceList, x, y, flipIndex, angle));
  }

  return result;
}

/*

1.1 ~TestInternal~

*/
void TestInternal(vector<HalfSegment> source,
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

  for (vector<vector<HalfSegment>>::const_iterator expectedResult =
      expectedResults.begin(); expectedResult != expectedResults.end();
      expectedResult++) {
    LineSegmentComparer comparer(actualResult->begin(),
                                 actualResult->end(),
                                 expectedResult->begin(),
                                 expectedResult->end());
    if (comparer.IsEqual()) {
      isEqual = true;
      break;
    }
  }

  if (!isEqual) {
    throw logic_error("region test failed!");
  }

  delete actualResult;
}

void TestInternal(vector<HalfSegment> source,
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
          TestInternal(FlipRotate(source, x == 1, y == 1, g == 1, angle),
                       SetOpType::Union,
                       decimals,
                       transformation,
                       FlipRotate(unionResults, x == 1, y == 1, g == 1, angle));

          TestInternal(FlipRotate(source, x == 1, y == 1, g == 1, angle),
                       SetOpType::Intersection,
                       decimals,
                       transformation,
                       FlipRotate(intersectionResults,
                                  x == 1,
                                  y == 1,
                                  g == 1,
                                  angle));

          if (g == 0) {
            TestInternal(FlipRotate(source, x == 1, y == 1, g == 1, angle),
                         SetOpType::Minus,
                         decimals,
                         transformation,
                         FlipRotate(differenceResults,
                                    x == 1,
                                    y == 1,
                                    false,
                                    angle));
          } else {
            TestInternal(FlipRotate(source, x == 1, y == 1, g == 1, angle),
                         SetOpType::Minus,
                         decimals,
                         transformation,
                         FlipRotate(differenceRevResults,
                                    x == 1,
                                    y == 1,
                                    false,
                                    angle));
          }
        }
      }
    }
  }
}

void TestInternal(vector<HalfSegment> source,
                  int decimals,
                  InternalPointTransformation* transformation,
                  const vector<HalfSegment>& unionResult,
                  const vector<HalfSegment>& intersectionResult,
                  const vector<HalfSegment>& differenceResult,
                  const vector<HalfSegment>& differenceRevResult)
{
  vector < vector < HalfSegment >> unionResults;
  vector < vector < HalfSegment >> intersectionResults;
  vector < vector < HalfSegment >> differenceResults;
  vector < vector < HalfSegment >> differenceRevResults;

  unionResults.push_back(unionResult);
  intersectionResults.push_back(intersectionResult);
  differenceResults.push_back(differenceResult);
  differenceRevResults.push_back(differenceRevResult);

  TestInternal(source,
               decimals,
               transformation,
               unionResults,
               intersectionResults,
               differenceResults,
               differenceRevResults);
}

/*

1.1 ~CreateHalfSegment~

*/
HalfSegment CreateHalfSegment(int geometryIndex,
                              int edgoNo,
                              double x0,
                              double y0,
                              double x1,
                              double y1,
                              bool insideAbove)
{
  HalfSegment result = HalfSegment(true,
                                   Point(true, x0, y0),
                                   Point(true, x1, y1));

  result.attr.faceno = geometryIndex;
  result.attr.edgeno = edgoNo;
  result.attr.insideAbove = insideAbove;

  return result;
}

/*

1.1 ~TriangleSetOpTest~

*/
bool TriangleSetOpTest(unsigned int seed, 
                       unsigned int triangleCount, 
                       int decimals)
{
  vector<vector<HalfSegment>*>* sourceGeometries;

  sourceGeometries = TestDataGenerator::
      GenerateTriangles(seed, 10, 10, 20, 20, triangleCount, 0);

  while (sourceGeometries->size() > 1) {
    vector<vector<HalfSegment>*>* newSourceGeometries =
        new vector<vector<HalfSegment>*>();

    for (unsigned int i = 0; i < sourceGeometries->size() - 1; i += 2) {
      vector<HalfSegment> source;
      for (int geometryIndex = 0; geometryIndex < 2; geometryIndex++) {
        for (vector<HalfSegment>::const_iterator hs =
            (*sourceGeometries)[i + geometryIndex]->begin();
            hs != (*sourceGeometries)[i + geometryIndex]->end(); hs++) {
          HalfSegment h = *hs;
          h.attr.faceno = geometryIndex;
          source.push_back(h);
        }

        delete (*sourceGeometries)[i + geometryIndex];
      }

      if (source.size() == 0) {
        continue;
      }

      {
        RegionVectorData
        v(&source, SetOpType::Union, decimals, NULL, false, true, false);

        Hobby ho(&v);
        ho.DetermineIntersections();
        vector<HalfSegment>* unionResult = v.GetResult();

        RegionHelper r;
        if (!r.CanExtractRegions(unionResult)) {
          return false;
        }

        newSourceGeometries->push_back(unionResult);
      }

      bool intersects = false;
      {
        RegionVectorData
        v(&source, SetOpType::Intersection, decimals, NULL, false, true, true);

        BentleyOttmann ho(&v);
        ho.DetermineIntersections();
        intersects = v.Intersects();
      }

      {
        RegionVectorData
        v(&source, SetOpType::Intersection, decimals, NULL, false, true, false);

        Hobby ho(&v);
        ho.DetermineIntersections();
        vector<HalfSegment>* intersectionResult = v.GetResult();

        if (intersectionResult->size() > 0 && !intersects) {
          return false;
        }

        RegionHelper r;
        if (!r.CanExtractRegions(intersectionResult)) {
          return false;
        }

        delete intersectionResult;
      }

      {
        RegionVectorData
        v(&source, SetOpType::Minus, decimals, NULL, false, true, false);

        Hobby ho(&v);
        ho.DetermineIntersections();
        vector<HalfSegment>* minusResult = v.GetResult();
        RegionHelper r;
        if (!r.CanExtractRegions(minusResult)) {
          return false;
        }

        delete minusResult;
      }

      {
        RegionVectorData
        v(&source, SetOpType::Minus, decimals, NULL, true, true, false);

        Hobby ho(&v);
        ho.DetermineIntersections();
        vector<HalfSegment>* minusRevResult = v.GetResult();
        RegionHelper r;
        if (!r.CanExtractRegions(minusRevResult)) {
          return false;
        }

        delete minusRevResult;
      }
    }

    if (sourceGeometries->size() % 2 == 1) {
      newSourceGeometries->
          push_back((*sourceGeometries)[sourceGeometries->size() - 1]);
    }

    delete sourceGeometries;
    sourceGeometries = newSourceGeometries;
  }

  delete (*sourceGeometries)[0];
  delete sourceGeometries;

  return true;
}

/*

1 Region $\times$ Region test cases

1.1 Test case 1

*/
void RegionRegionTest1()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 2

*/
void RegionRegionTest2()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 3

*/
void RegionRegionTest3()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 4

*/
void RegionRegionTest4()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 5

*/
void RegionRegionTest5()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 6

*/
void RegionRegionTest6()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 7

*/
void RegionRegionTest7()
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

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 13, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 11, 15, 6, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 15, 6, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 15, 6, 16, 3, true));
  s->push_back(CreateHalfSegment(0, 4, 15, 9, 16, 3, false));

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 8

*/
void RegionRegionTest8()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 9

*/
void RegionRegionTest9()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 10

*/
void RegionRegionTest10()
{
  vector<HalfSegment> source;
  vector < vector < HalfSegment >> unionResult;
  vector < vector < HalfSegment >> intersectionResult;
  vector < vector < HalfSegment >> differenceResult;
  vector < vector < HalfSegment >> differenceRevResult;

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

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 11

*/
void RegionRegionTest11()
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

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 7, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 8, 18, 13, 18, 18, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 13, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 13, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 15, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, true));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 13, 13, 15, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 13, 15, 13, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 15, 15, 15, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 5, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 6, 18, 13, 18, 18, true));

  InternalPointTransformation transformation(-10000, -10000, 2, 0, 1);

  TestInternal(source,
               1,
               &transformation,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 12

*/
void RegionRegionTest12()
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

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 5, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 7, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 8, 18, 13, 18, 18, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 13, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 13, 15, 13, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 15, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, true));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 10, 15, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 10, true));
  s->push_back(CreateHalfSegment(0, 3, 10, 15, 13, 15, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 13, 13, 15, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 13, 15, 13, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 10, 15, 13, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 13, 15, 13, 18, false));
  s->push_back(CreateHalfSegment(0, 2, 13, 15, 15, 15, true));
  s->push_back(CreateHalfSegment(0, 3, 13, 18, 18, 18, false));
  s->push_back(CreateHalfSegment(0, 4, 15, 13, 15, 15, false));
  s->push_back(CreateHalfSegment(0, 5, 15, 13, 18, 13, true));
  s->push_back(CreateHalfSegment(0, 6, 18, 13, 18, 18, true));

  InternalPointTransformation transformation(-10000, -10000, 2, 0, 1);

  TestInternal(source,
               1,
               &transformation,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 13

*/
void RegionRegionTest13()
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

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 20, 9, 12, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 20, 9, 20, false));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 9, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 9, 11, 10, 11, true));
  s->push_back(CreateHalfSegment(0, 5, 9, 20, 10, 12, false));
  s->push_back(CreateHalfSegment(0, 6, 10, 11, 10, 12, true));
  s = &intersectionResult;

  s->push_back(CreateHalfSegment(0, 1, 9, 12, 9, 11, false));
  s->push_back(CreateHalfSegment(0, 2, 9, 11, 10, 11, true));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 10, 12, false));
  s->push_back(CreateHalfSegment(0, 4, 10, 11, 10, 12, true));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 8, 20, 9, 12, true));
  s->push_back(CreateHalfSegment(0, 2, 8, 20, 9, 20, false));
  s->push_back(CreateHalfSegment(0, 3, 9, 12, 10, 12, true));
  s->push_back(CreateHalfSegment(0, 4, 9, 20, 10, 12, false));

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 14

*/
void RegionRegionTest14()
{
  vector<HalfSegment> source;
  vector < vector < HalfSegment >> unionResult;
  vector < vector < HalfSegment >> intersectionResult;
  vector < vector < HalfSegment >> differenceResult;
  vector < vector < HalfSegment >> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 1, 14, 11, 15, 7, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 8, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 12, 10, 16, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 12, 10, 13, 8, true));

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 8, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 11, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 6, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 15, 9, 15, 7, true));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 12, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 8, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 15, 9, false));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 13, 8, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 8, true));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 8, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 6, 13, 8, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 7, 13, 8, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 8, 14, 11, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 9, 15, 9, 15, 7, true));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 15, 7, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 2, 13, 8, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 5, 13, 8, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 7, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 8, 12, 10, 13, 8, false));
  s->push_back(CreateHalfSegment(0, 9, 10, 10, 13, 8, true));
  s->push_back(CreateHalfSegment(0, 10, 10, 10, 12, 10, false));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  differenceRevResult.push_back(*s);
  delete s;

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 15

*/
void RegionRegionTest15()
{
  vector<HalfSegment> source;
  vector < vector < HalfSegment >> unionResult;
  vector < vector < HalfSegment >> intersectionResult;
  vector < vector < HalfSegment >> differenceResult;
  vector < vector < HalfSegment >> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 1, 14, 11, 15, 7, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 9, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 12, 10, 16, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 12, 10, 13, 9, true));

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 13, 9, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 11, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 6, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 15, 9, 15, 7, true));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 9, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 5, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 15, 7, 15, 9, true));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 15, 7, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 5, 10, 10, 12, 10, false));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 12, 10, 13, 9, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 9, 14, 9, true));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 13, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 6, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 13, 9, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 8, 14, 11, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 9, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 10, 15, 9, 15, 7, true));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 5, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 6, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 14, 9, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 8, 14, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 9, 15, 7, 15, 9, true));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 14, 9, 15, 7, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 14, 9, 14, 11, true));
  s->push_back(CreateHalfSegment(0, 4, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 5, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 6, 12, 10, 14, 11, false));
  s->push_back(CreateHalfSegment(0, 7, 12, 10, 13, 9, false));
  s->push_back(CreateHalfSegment(0, 8, 10, 10, 12, 10, false));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  differenceRevResult.push_back(*s);
  delete s;

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 16

*/
void RegionRegionTest16()
{
  vector<HalfSegment> source;
  vector < vector < HalfSegment >> unionResult;
  vector < vector < HalfSegment >> intersectionResult;
  vector < vector < HalfSegment >> differenceResult;
  vector < vector < HalfSegment >> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 1, 14, 7, 15, 11, true));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 15, 11, false));
  s->push_back(CreateHalfSegment(1, 10, 13, 9, 16, 9, true));
  s->push_back(CreateHalfSegment(1, 11, 12, 10, 16, 9, false));
  s->push_back(CreateHalfSegment(1, 12, 12, 10, 13, 9, true));

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 15, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 7, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 15, 9, 15, 11, true));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 15, 9, 15, 11, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 15, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 7, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 6, 10, 10, 12, 10, false));
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  unionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 12, 10, 13, 9, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 13, 9, 15, 9, true));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 13, 9, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 2, 12, 10, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 9, true));
  intersectionResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 10, 10, 12, 10, false));
  s->push_back(CreateHalfSegment(0, 2, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 13, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 12, 10, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 12, 10, 15, 11, false));
  s->push_back(CreateHalfSegment(0, 6, 13, 9, 15, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 14, 7, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 8, 15, 9, 15, 11, true));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  s->push_back(CreateHalfSegment(0, 1, 15, 9, 15, 11, true));
  s->push_back(CreateHalfSegment(0, 2, 14, 9, 15, 9, true));
  s->push_back(CreateHalfSegment(0, 3, 12, 10, 15, 11, false));
  s->push_back(CreateHalfSegment(0, 4, 14, 7, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 5, 10, 10, 14, 7, true));
  s->push_back(CreateHalfSegment(0, 6, 13, 9, 14, 9, false));
  s->push_back(CreateHalfSegment(0, 7, 12, 10, 14, 9, true));
  s->push_back(CreateHalfSegment(0, 8, 12, 10, 13, 9, false));
  s->push_back(CreateHalfSegment(0, 9, 10, 10, 12, 10, false));
  differenceResult.push_back(*s);
  delete s;

  s = new vector<HalfSegment>();
  differenceRevResult.push_back(*s);
  delete s;

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 17

*/
void RegionRegionTest17()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 1, 1, 3, 2, true));
  s->push_back(CreateHalfSegment(0, 1, 3, 2, 8, 6, true));
  s->push_back(CreateHalfSegment(0, 2, 1, 1, 8, 6, false));
  s->push_back(CreateHalfSegment(1, 10, 1, 1, 9, 1, true));
  s->push_back(CreateHalfSegment(1, 11, 7, 4, 9, 1, false));
  s->push_back(CreateHalfSegment(1, 12, 1, 1, 7, 4, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 1, 3, 2, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 1, 9, 1, true));
  s->push_back(CreateHalfSegment(0, 3, 3, 2, 7, 4, false));
  s->push_back(CreateHalfSegment(0, 4, 7, 4, 9, 1, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 1, 3, 2, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 1, 9, 1, true));
  s->push_back(CreateHalfSegment(0, 3, 3, 2, 7, 4, false));
  s->push_back(CreateHalfSegment(0, 4, 7, 4, 9, 1, false));

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 18

*/
void RegionRegionTest18()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 3, 4, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 1, 4, 5, 6, 1, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(1, 10, 3, 4, 6, 7, false));
  s->push_back(CreateHalfSegment(1, 11, 3, 4, 10, 6, true));
  s->push_back(CreateHalfSegment(1, 12, 6, 7, 10, 6, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 4, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 3, 4, 5, 6, 7, false));
  s->push_back(CreateHalfSegment(0, 4, 4.3, 4.4, 6, 1, false));
  s->push_back(CreateHalfSegment(0, 5, 4.3, 4.4, 10, 6, true));
  s->push_back(CreateHalfSegment(0, 6, 6, 7, 10, 6, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 4, 4.3, 4.4, true));
  s->push_back(CreateHalfSegment(0, 3, 4, 5, 4.3, 4.4, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 3, 4, 4.3, 4.4, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 4, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 3, 4.3, 4.4, 6, 1, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 4, 5, 4.3, 4.4, true));
  s->push_back(CreateHalfSegment(0, 2, 4, 5, 6, 7, false));
  s->push_back(CreateHalfSegment(0, 3, 4.3, 4.4, 10, 6, true));
  s->push_back(CreateHalfSegment(0, 4, 6, 7, 10, 6, false));

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 19

*/
void RegionRegionTest19()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 2, 1, 8, 10, true));
  s->push_back(CreateHalfSegment(0, 1, 2, 1, 5, 6, false));
  s->push_back(CreateHalfSegment(0, 2, 5, 6, 8, 10, false));
  s->push_back(CreateHalfSegment(1, 10, 0, 4, 6, 7, false));
  s->push_back(CreateHalfSegment(1, 11, 0, 4, 4, 4, true));
  s->push_back(CreateHalfSegment(1, 12, 4, 4, 6, 7, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 0, 4, 4, 4, true));
  s->push_back(CreateHalfSegment(0, 2, 0, 4, 5, 6, false));
  s->push_back(CreateHalfSegment(0, 3, 4, 4, 5, 6, true));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 0, 4, 4, 4, true));
  s->push_back(CreateHalfSegment(0, 2, 0, 4, 5, 6, false));
  s->push_back(CreateHalfSegment(0, 3, 4, 4, 5, 6, true));

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 20

*/
void RegionRegionTest20()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 3, 4, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 1, 4, 5, 6, 1, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(0, 4, 1, 4, 4, 1, true));
  s->push_back(CreateHalfSegment(0, 5, 2, 5, 4, 1, false));
  s->push_back(CreateHalfSegment(0, 6, 1, 4, 2, 5, false));
  s->push_back(CreateHalfSegment(1, 10, 2, 3, 6, 7, false));
  s->push_back(CreateHalfSegment(1, 11, 2, 3, 10, 6, true));
  s->push_back(CreateHalfSegment(1, 12, 6, 7, 10, 6, false));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 4, 2, 3, true));
  s->push_back(CreateHalfSegment(0, 2, 1, 4, 2, 5, false));
  s->push_back(CreateHalfSegment(0, 3, 2, 3, 4, 1, true));
  s->push_back(CreateHalfSegment(0, 4, 2, 5, 2.7, 3.7, false));
  s->push_back(CreateHalfSegment(0, 5, 2.7, 3.7, 3, 4, false));
  s->push_back(CreateHalfSegment(0, 6, 2.8, 3.3, 3.5, 3.5, true));
  s->push_back(CreateHalfSegment(0, 7, 2.8, 3.3, 4, 1, false));
  s->push_back(CreateHalfSegment(0, 8, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(0, 9, 3.5, 3.5, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 10, 4, 5, 6, 7, false));
  s->push_back(CreateHalfSegment(0, 11, 4.5, 3.9, 6, 1, false));
  s->push_back(CreateHalfSegment(0, 12, 4.5, 3.9, 10, 6, true));
  s->push_back(CreateHalfSegment(0, 13, 6, 7, 10, 6, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 2, 3, 2.7, 3.7, false));
  s->push_back(CreateHalfSegment(0, 2, 2, 3, 2.8, 3.3, true));
  s->push_back(CreateHalfSegment(0, 3, 2.7, 3.7, 2.8, 3.3, false));
  s->push_back(CreateHalfSegment(0, 4, 3, 4, 3.5, 3.5, true));
  s->push_back(CreateHalfSegment(0, 5, 3, 4, 4, 5, false));
  s->push_back(CreateHalfSegment(0, 6, 3.5, 3.5, 4.5, 3.9, true));
  s->push_back(CreateHalfSegment(0, 7, 4, 5, 4.5, 3.9, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 4, 2, 3, true));
  s->push_back(CreateHalfSegment(0, 2, 1, 4, 2, 5, false));
  s->push_back(CreateHalfSegment(0, 3, 2, 3, 2.7, 3.7, true));
  s->push_back(CreateHalfSegment(0, 4, 2, 3, 2.8, 3.3, false));
  s->push_back(CreateHalfSegment(0, 5, 2, 3, 4, 1, true));
  s->push_back(CreateHalfSegment(0, 6, 2, 5, 2.7, 3.7, false));
  s->push_back(CreateHalfSegment(0, 7, 2.8, 3.3, 4, 1, false));
  s->push_back(CreateHalfSegment(0, 8, 3.5, 3.5, 4.5, 3.9, false));
  s->push_back(CreateHalfSegment(0, 9, 3.5, 3.5, 6, 1, true));
  s->push_back(CreateHalfSegment(0, 10, 4.5, 3.9, 6, 1, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 2.7, 3.7, 2.8, 3.3, true));
  s->push_back(CreateHalfSegment(0, 2, 2.7, 3.7, 3, 4, false));
  s->push_back(CreateHalfSegment(0, 3, 2.8, 3.3, 3.5, 3.5, true));
  s->push_back(CreateHalfSegment(0, 4, 3, 4, 3.5, 3.5, false));
  s->push_back(CreateHalfSegment(0, 5, 4, 5, 4.5, 3.9, true));
  s->push_back(CreateHalfSegment(0, 6, 4, 5, 6, 7, false));
  s->push_back(CreateHalfSegment(0, 7, 4.5, 3.9, 10, 6, true));
  s->push_back(CreateHalfSegment(0, 8, 6, 7, 10, 6, false));

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 21

*/
void RegionRegionTest21()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 1, 9, 4, 4, true));
  s->push_back(CreateHalfSegment(0, 1, 1, 9, 8, 9, false));
  s->push_back(CreateHalfSegment(0, 2, 4, 4, 5, 5, true));
  s->push_back(CreateHalfSegment(0, 3, 5, 5, 7, 0, true));
  s->push_back(CreateHalfSegment(0, 4, 6, 6, 7, 0, false));
  s->push_back(CreateHalfSegment(0, 5, 6, 6, 8, 9, true));
  s->push_back(CreateHalfSegment(1, 6, 1, 0, 1, 1, false));
  s->push_back(CreateHalfSegment(1, 7, 1, 1, 9, 9, false));
  s->push_back(CreateHalfSegment(1, 8, 1, 0, 9, 9, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 0, 1, 1, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 0, 5.1, 4.7, true));
  s->push_back(CreateHalfSegment(0, 3, 1, 1, 4, 4, false));
  s->push_back(CreateHalfSegment(0, 4, 1, 9, 4, 4, true));
  s->push_back(CreateHalfSegment(0, 5, 1, 9, 8, 9, false));
  s->push_back(CreateHalfSegment(0, 6, 5.1, 4.7, 7, 0, true));
  s->push_back(CreateHalfSegment(0, 7, 6, 6, 8, 9, true));
  s->push_back(CreateHalfSegment(0, 8, 6, 6, 9, 9, false));
  s->push_back(CreateHalfSegment(0, 9, 6.1, 5.7, 7, 0, false));
  s->push_back(CreateHalfSegment(0, 10, 6.1, 5.7, 9, 9, true));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 5, 5, 5.1, 4.7, true));
  s->push_back(CreateHalfSegment(0, 2, 5, 5, 6, 6, false));
  s->push_back(CreateHalfSegment(0, 3, 5.1, 4.7, 6.1, 5.7, true));
  s->push_back(CreateHalfSegment(0, 4, 6, 6, 6.1, 5.7, false));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 9, 4, 4, true));
  s->push_back(CreateHalfSegment(0, 2, 1, 9, 8, 9, false));
  s->push_back(CreateHalfSegment(0, 3, 4, 4, 5, 5, true));
  s->push_back(CreateHalfSegment(0, 4, 5, 5, 6, 6, true));
  s->push_back(CreateHalfSegment(0, 5, 5.1, 4.7, 6.1, 5.7, false));
  s->push_back(CreateHalfSegment(0, 6, 5.1, 4.7, 7, 0, true));
  s->push_back(CreateHalfSegment(0, 7, 6, 6, 8, 9, true));
  s->push_back(CreateHalfSegment(0, 8, 6.1, 5.7, 7, 0, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 0, 1, 1, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 0, 5.1, 4.7, true));
  s->push_back(CreateHalfSegment(0, 3, 1, 1, 4, 4, false));
  s->push_back(CreateHalfSegment(0, 4, 4, 4, 5, 5, false));
  s->push_back(CreateHalfSegment(0, 5, 5, 5, 5.1, 4.7, false));
  s->push_back(CreateHalfSegment(0, 6, 6, 6, 6.1, 5.7, true));
  s->push_back(CreateHalfSegment(0, 7, 6, 6, 9, 9, false));
  s->push_back(CreateHalfSegment(0, 8, 6.1, 5.7, 9, 9, true));

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 22

*/
void RegionRegionTest22()
{
  vector<HalfSegment> source;
  vector<HalfSegment> unionResult;
  vector<HalfSegment> intersectionResult;
  vector<HalfSegment> differenceResult;
  vector<HalfSegment> differenceRevResult;

  vector<HalfSegment>* s = &source;
  s->push_back(CreateHalfSegment(0, 0, 1, 3, 9, 5, true));
  s->push_back(CreateHalfSegment(0, 1, 1, 3, 3, 6, false));
  s->push_back(CreateHalfSegment(0, 3, 3, 6, 9, 7, false));
  s->push_back(CreateHalfSegment(0, 4, 7, 6, 9, 5, false));
  s->push_back(CreateHalfSegment(0, 5, 7, 6, 9, 7, true));
  s->push_back(CreateHalfSegment(1, 6, 1, 3, 5, 9, false));
  s->push_back(CreateHalfSegment(1, 7, 5, 9, 9, 6, false));
  s->push_back(CreateHalfSegment(1, 8, 1, 3, 9, 6, true));

  s = &unionResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 3, 3, 6, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 3, 9, 5, true));
  s->push_back(CreateHalfSegment(0, 3, 3, 6, 5, 9, false));
  s->push_back(CreateHalfSegment(0, 4, 5, 9, 8, 7, false));
  s->push_back(CreateHalfSegment(0, 5, 8, 7, 8, 6, true));
  s->push_back(CreateHalfSegment(0, 6, 8, 6, 9, 5, false));

  s = &intersectionResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 3, 3, 6, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 3, 8, 6, true));
  s->push_back(CreateHalfSegment(0, 3, 3, 6, 8, 7, false));
  s->push_back(CreateHalfSegment(0, 4, 8, 7, 8, 6, true));

  s = &differenceResult;
  s->push_back(CreateHalfSegment(0, 1, 1, 3, 8, 6, false));
  s->push_back(CreateHalfSegment(0, 2, 1, 3, 9, 5, true));
  s->push_back(CreateHalfSegment(0, 3, 8, 6, 9, 5, false));

  s = &differenceRevResult;
  s->push_back(CreateHalfSegment(0, 1, 3, 6, 5, 9, false));
  s->push_back(CreateHalfSegment(0, 2, 3, 6, 8, 7, true));
  s->push_back(CreateHalfSegment(0, 3, 5, 9, 8, 7, false));

  TestInternal(source,
               0,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 23

*/
void RegionRegionTest23()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Test case 24

*/
void RegionRegionTest24()
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

  TestInternal(source,
               1,
               NULL,
               unionResult,
               intersectionResult,
               differenceResult,
               differenceRevResult);
}

/*

1.1 Main test method

*/
void RegionRegionTest()
{
  RegionRegionTest1();
  RegionRegionTest2();
  RegionRegionTest3();
  RegionRegionTest4();
  RegionRegionTest5();
  RegionRegionTest6();
  RegionRegionTest7();
  RegionRegionTest8();
  RegionRegionTest9();
  RegionRegionTest10();
  RegionRegionTest11();
  RegionRegionTest12();
  RegionRegionTest13();
  RegionRegionTest14();
  RegionRegionTest15();
  RegionRegionTest16();
  RegionRegionTest17();
  RegionRegionTest18();
  RegionRegionTest19();
  RegionRegionTest20();
  RegionRegionTest21();
  RegionRegionTest22();
  RegionRegionTest23();
  RegionRegionTest24();
}

