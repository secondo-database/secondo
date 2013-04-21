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

#ifdef WIN32
#include <tchar.h>
#include <crtdbg.h>
#endif

#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>
#include <iostream>

#include "../Algorithm/NaiveIntersectionAlgorithm.h"
#include "../Algorithm/SimpleSweepIntersectionAlgorithm.h"
#include "../Algorithm/BentleyOttmann.h"
#include "../Algorithm/HobbyNaiveIntersectionAlgorithm.h"
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

class VectorData : public IntersectionAlgorithmData
{
private:
  vector<HalfSegment>* _input;
  vector<HalfSegment>* _output;
  vector<HalfSegment>::iterator _inputIterator;
  int _outputSegments;
  int _roundToDecimals;
  int _stepSize;

public:
  VectorData(vector<HalfSegment>* input, int roundToDecimals, int stepSize)
  {
    _input = input;
    _output = new vector<HalfSegment>();
    _outputSegments = 0;
    _roundToDecimals = roundToDecimals;
    _stepSize = stepSize;
  }

  ~VectorData()
  {
    if (_output != NULL) {
      delete _output;
      _output = NULL;
    }
  }

  IntersectionAlgorithmCalculationType GetCalculationType()
  {
    return IntersectionAlgorithmCalculationType::CalulationTypeLine;
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
      belongsToSecondGeometry = false;
      segment = *_inputIterator++;
      return true;
    }
  }

  void OutputHalfSegment(
    const HalfSegment& segment,
    const InternalAttribute& /*attribute*/)
  {
    HalfSegment s1 = segment;
    HalfSegment s2 = segment;
    s1.SetLeftDomPoint(true);
    s2.SetLeftDomPoint(false);
    s1.attr.edgeno = _outputSegments;
    s2.attr.edgeno = _outputSegments;
    _output->push_back(s1);
    _output->push_back(s2);
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
    stepSize = _stepSize;
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

int main()
{
#ifdef WIN32
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

#ifdef _DEBUG
  unsigned int count = 100;
#else
  unsigned int count = 10000;
#endif
  int errorCount = 0;

  for (unsigned int i = 1; errorCount == 0; ++i)
  {
    for (unsigned int subTest = 0; subTest < 7; ++subTest) {
      vector<HalfSegment>* input;
      int roundToDecimals;

      switch (subTest) {
      case 0:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 8);
        roundToDecimals = 7;
        break;

      case 1:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 8);
        roundToDecimals = 1;
        break;

      case 2:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 1);
        roundToDecimals = 4;
        break;

      case 3:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 1);
        roundToDecimals = 1;
        break;

      case 4:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 33500000, 5800000, 20, 20, count, 4);
        roundToDecimals = 2;
        break;

      case 5:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 10, 50, 0.01, 0.01, count, 8);
        roundToDecimals = 7;
        break;

      case 6:
        input= TestDataGenerator::
          GenerateRandomWalk(i, 0, 0, 0.05, 0.05, count, 8);
        roundToDecimals = 7;
        break;

      default:
        throw new std::logic_error("invalid subtest number!");
      }

      int roundStepSize = (roundToDecimals == 7?2:1);

      clock_t time_ni = 0;
      vector<HalfSegment>* niResult = NULL;
      if (count <= 100)
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        NaiveIntersectionAlgorithm ni(&v);
        ni.DetermineIntersections();
        niResult = v.GetResult();
        time_ni = std::clock()-start;
      }

      clock_t time_ssi = 0;
      vector<HalfSegment>* ssiResult;
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        SimpleSweepIntersectionAlgorithm ssi(&v);
        ssi.DetermineIntersections();
        ssiResult = v.GetResult();
        time_ssi = std::clock()-start;
      }

      clock_t time_bo = 0;
      vector<HalfSegment>*  boResult;
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        BentleyOttmann bo(&v);
        bo.DetermineIntersections();
        boResult = v.GetResult();
        time_bo = std::clock()-start;
      }

      clock_t time_hos = 0;
      vector<HalfSegment>* hosResult;
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        HobbyNaiveIntersectionAlgorithm hos(&v);
        hos.DetermineIntersections();
        hosResult = v.GetResult();
        time_hos = std::clock()-start;
      }

      clock_t time_ho = 0;
      vector<HalfSegment>* hoResult;
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        Hobby ho(&v);
        ho.DetermineIntersections();
        hoResult = v.GetResult();
        time_ho = std::clock()-start;
      }

      bool isEqualNaiveSimple;
      bool isEqualSimpleBentleyOttmann;
      bool isEqualHobby;
      size_t nonHobbyIntersections;
      size_t hobbyIntersections;

      if (niResult != NULL) {
        LineSegmentComparer comparer(
          niResult->begin(),
          niResult->end(),
          ssiResult->begin(),
          ssiResult->end());
        isEqualNaiveSimple = comparer.IsEqual();
      } else {
        isEqualNaiveSimple = true;
      }

      {
        LineSegmentComparer comparer(
          ssiResult->begin(),
          ssiResult->end(),
          boResult->begin(),
          boResult->end());
        isEqualSimpleBentleyOttmann = comparer.IsEqual();
      }

      {
        LineSegmentComparer comparer(
          hosResult->begin(),
          hosResult->end(),
          hoResult->begin(),
          hoResult->end());
        isEqualHobby = comparer.IsEqual();
      }

      {
        VectorData v(boResult, roundToDecimals, roundStepSize);
        BentleyOttmann bo(&v);
        bo.DetermineIntersections();
        nonHobbyIntersections = bo.GetIntersectionCount();
      }

      {
        VectorData v(hoResult, roundToDecimals, roundStepSize);
        BentleyOttmann bo(&v);
        bo.DetermineIntersections();
        hobbyIntersections = bo.GetIntersectionCount();
      }

      if (i%25 == 1 && subTest == 0) {
        printf(" Seed.Sub | Naive SimSw BenOt HobSi Hobby |"
          "  InCnt OutCnt HobCnt |     | BoCnt HoCnt\n");
      }

      printf("%5d.%03d | %5d %5d %5d %5d %5d |"
        " %6d %6d %6d | %d%d%d | %5d %5d\n",
        i,
        subTest,
        (int)(time_ni/(CLOCKS_PER_SEC/1000)),
        (int)(time_ssi/(CLOCKS_PER_SEC/1000)),
        (int)(time_bo/(CLOCKS_PER_SEC/1000)),
        (int)(time_hos/(CLOCKS_PER_SEC/1000)),
        (int)(time_ho/(CLOCKS_PER_SEC/1000)),
        (int)(input->size()),
        (int)(ssiResult->size()),
        (int)(hosResult->size()),
        isEqualNaiveSimple,
        isEqualSimpleBentleyOttmann,
        isEqualHobby,
        (int)nonHobbyIntersections,
        (int)hobbyIntersections);

      if (!isEqualNaiveSimple ||
        !isEqualSimpleBentleyOttmann ||
        !isEqualHobby ||
        hobbyIntersections > 0) {
          printf("Fehler!\n");
          errorCount++;
          break;
      }

      delete input;
      if (niResult != NULL) {
        delete niResult;
      }
      delete ssiResult;
      delete boResult;
      delete hosResult;
      delete hoResult;

#ifdef WIN32
#ifdef _DEBUG
      _CrtDumpMemoryLeaks();
#endif
#endif
    }
  }
}
