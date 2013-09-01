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

[1] Main file for the test project.

[TOC]

1 Overview

This file contains the methods for the test project.

This file is not required for SECONDO. It is only used inside the test project.

1 Includes

*/

//#define PERFORMANCETEST

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS

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

/*

1 Class ~VectorData~

*/
class VectorData : public IntersectionAlgorithmData
{
/*

1.1 Member Variables

*/
private:
  vector<HalfSegment>* _input;
  vector<HalfSegment>* _output;
  vector<HalfSegment>::iterator _inputIterator;
  int _outputSegments;
  int _roundToDecimals;
  int _stepSize;

public:
/*

1.1 Constructor

*/
  VectorData(vector<HalfSegment>* input, int roundToDecimals, int stepSize)
  {
    _input = input;
    _output = new vector<HalfSegment>();
    _outputSegments = 0;
    _roundToDecimals = roundToDecimals;
    _stepSize = stepSize;
  }

/*

1.1 Destructor

*/
  ~VectorData()
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
    return false;
  }

/*

1.1 ~SecondGeometryIsRegion~

*/
  bool SecondGeometryIsRegion() const
  {
    return false;
  }

/*

1.1 ~InitializeFetch~

*/
  void InitializeFetch()
  {
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
      belongsToSecondGeometry = false;
      segment = *_inputIterator++;
      point.SetDefined(false);
      return true;
    }
  }

/*

1.1 ~OutputData~

*/
  bool OutputData() const
  {
    return true;
  }

/*

1.1 ~OutputHalfSegment~

*/
  void OutputHalfSegment(const HalfSegment& segment,
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
    stepSize = _stepSize;
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
    std::vector<HalfSegment>* result = _output;
    _output = NULL;
    return result;
  }
};

/*

1 External methods

1.1 ~RegionRegionTest~

*/
extern void RegionRegionTest();

/*

1.1 ~TriangleSetOpTest~

*/
extern bool TriangleSetOpTest(unsigned int seed,
                              unsigned int triangleCount,
                              int decimals);

/*

1 Main method

*/
int main(int argc, char* argv[])
{
#ifdef WIN32
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

  unsigned int processCount = 1;
  unsigned int processNumber = 0;

#ifndef PERFORMANCETEST
  if (argc == 3) {
    sscanf(argv[1], "%d", &processNumber);
    sscanf(argv[2], "%d", &processCount);
  }

  {
    cout << "region x region tests...";
    RegionRegionTest();
    cout << " done\n";
  }

#ifdef _DEBUG
  unsigned int triangleCount = 20;
#else
  unsigned int triangleCount = 2048;
#endif

#endif

#ifdef WIN32
#ifdef _DEBUG
  _CrtDumpMemoryLeaks();
  //_CrtSetBreakAlloc(2150);
#endif
#endif

#ifdef _DEBUG
  unsigned int count = 100;
#else
  unsigned int count = 10000;
#endif

  int errorCount = 0;
  int loopsBentleyOttmann = 1;
  int loopsHobby = 1;

  unsigned int maximumIterations = numeric_limits<unsigned int>::max();

#ifdef PERFORMANCETEST
  if (argc == 5) {
    sscanf(argv[1], "%d", &count);
    sscanf(argv[2], "%d", &loopsBentleyOttmann);
    sscanf(argv[3], "%d", &loopsHobby);
    sscanf(argv[4], "%d", &maximumIterations);
    printf("%6d %6d %6d -----------------------------------------\n",
           count, loopsBentleyOttmann, loopsHobby);
  }
#endif

  clock_t totalTimeBentleyOttmann = 0;
  clock_t totalTimeHobby = 0;
 

  for (unsigned int i = 1, j = 0; 
       (i-1) < maximumIterations && errorCount == 0; ++i) {

    if ((i % processCount) != processNumber) {
      continue;
    }

    if (j % 5 == 0) {
      printf(" Seed.Sub | Naive SimSw BenOt HobSi Hobby |"
             "  InCnt OutCnt HobCnt |     | BoCnt HoCnt\n");
    }

#ifndef PERFORMANCETEST
    for (unsigned int subTest = 0; subTest < 5 && errorCount == 0; ++subTest) {
      {
        int decimals = (subTest == 4 ? 6 : (int)subTest);
        bool result = TriangleSetOpTest(i, triangleCount, decimals);

        printf("%5d.%03d | triangle region test\n", i, subTest);
        if (!result) {
          printf("Fehler!\n");
          errorCount++;
          break;
        }
      }

#ifdef WIN32
#ifdef _DEBUG
      _CrtDumpMemoryLeaks();
#endif
#endif
    }
#endif

    for (unsigned int subTest = 0; subTest < 7 && errorCount == 0; ++subTest) {
      vector<HalfSegment>* input;
      int roundToDecimals;

      switch (subTest) {
        case 0:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 9);
          roundToDecimals = 8;
          break;

        case 1:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 8);
          roundToDecimals = 1;
          break;

        case 2:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 1);
          roundToDecimals = 4;
          break;

        case 3:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 1000000, 50000, 10, 10, count, 1);
          roundToDecimals = 1;
          break;

        case 4:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 33500000, 5800000, 20, 20, count, 4);
          roundToDecimals = 2;
          break;

        case 5:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 10, 50, 0.01, 0.01, count, 9);
          roundToDecimals = 8;
          break;

        case 6:
          input = TestDataGenerator::
              GenerateRandomWalk(i, 0, 0, 0.05, 0.05, count, 9);
          roundToDecimals = 8;
          break;

        default:
          throw new std::logic_error("invalid subtest number!");
      }

      int roundStepSize = (roundToDecimals == 8 ? 2 : 1);

      clock_t time_ni = 0;
      vector<HalfSegment>* niResult = NULL;
#ifndef PERFORMANCETEST
      if (count <= 100) {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        NaiveIntersectionAlgorithm ni(&v);
        ni.DetermineIntersections();
        niResult = v.GetResult();
        time_ni = std::clock() - start;
      }
#endif

      clock_t time_ssi = 0;
      vector<HalfSegment>* ssiResult = NULL;
#ifndef PERFORMANCETEST
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        SimpleSweepIntersectionAlgorithm ssi(&v);
        ssi.DetermineIntersections();
        ssiResult = v.GetResult();
        time_ssi = std::clock() - start;
      }
#endif

      clock_t time_bo = 0;
      vector<HalfSegment>* boResult = NULL;
      {
        clock_t start = std::clock();
        for(int loop=0 ; loop < loopsBentleyOttmann; loop++) {
          if(boResult != NULL) {
            delete boResult;
          }

          VectorData v(input, roundToDecimals, roundStepSize);
          BentleyOttmann bo(&v);
          bo.DetermineIntersections();
          boResult = v.GetResult();
        }
        time_bo = std::clock() - start;
        totalTimeBentleyOttmann += time_bo;
      }

      clock_t time_hos = 0;
      vector<HalfSegment>* hosResult=NULL;
#ifndef PERFORMANCETEST
      {
        clock_t start = std::clock();

        VectorData v(input, roundToDecimals, roundStepSize);
        HobbyNaiveIntersectionAlgorithm hos(&v);
        hos.DetermineIntersections();
        hosResult = v.GetResult();
        time_hos = std::clock() - start;
      }
#endif

      clock_t time_ho = 0;
      vector<HalfSegment>* hoResult = NULL;
      {
        clock_t start = std::clock();

        for(int loop=0; loop < loopsHobby; loop++) {
          if(hoResult != NULL) {
            delete hoResult;
          }

          VectorData v(input, roundToDecimals, roundStepSize);
          Hobby ho(&v);
          ho.DetermineIntersections();
          hoResult = v.GetResult();
        }
        time_ho = std::clock() - start;
        totalTimeHobby += time_ho;
      }

      bool isEqualNaiveSimple;
      bool isEqualSimpleBentleyOttmann;
      bool isEqualHobby;
      size_t nonHobbyIntersections;
      size_t hobbyIntersections;

#ifndef PERFORMANCETEST
      if (niResult != NULL) {
        LineSegmentComparer comparer(niResult->begin(),
                                     niResult->end(),
                                     ssiResult->begin(),
                                     ssiResult->end());
        isEqualNaiveSimple = comparer.IsEqual();
      } else {
        isEqualNaiveSimple = true;
      }

      {
        LineSegmentComparer comparer(ssiResult->begin(),
                                     ssiResult->end(),
                                     boResult->begin(),
                                     boResult->end());
        isEqualSimpleBentleyOttmann = comparer.IsEqual();
      }

      {
        LineSegmentComparer comparer(hosResult->begin(),
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
#else
      isEqualNaiveSimple = true;
      isEqualSimpleBentleyOttmann = true;
      isEqualHobby = true;
      nonHobbyIntersections = 0;
      hobbyIntersections = 0;
#endif

      printf("%5d.%03d | %5d %5d %5d %5d %5d |"
             " %6d %6d %6d | %d%d%d | %5d %5d\n",
             i,
             subTest,
             (int)(time_ni / (CLOCKS_PER_SEC / 1000)),
             (int)(time_ssi / (CLOCKS_PER_SEC / 1000)),
             (int)(time_bo / (CLOCKS_PER_SEC / 1000)),
             (int)(time_hos / (CLOCKS_PER_SEC / 1000)),
             (int)(time_ho / (CLOCKS_PER_SEC / 1000)),
             (int)(input->size()),
             (int)(boResult == NULL ? 0 : boResult->size()),
             (int)(hoResult == NULL ? 0 : hoResult->size()),
             isEqualNaiveSimple,
             isEqualSimpleBentleyOttmann,
             isEqualHobby,
             (int)nonHobbyIntersections,
             (int)hobbyIntersections);

      if (!isEqualNaiveSimple
          || !isEqualSimpleBentleyOttmann
          || !isEqualHobby
          || hobbyIntersections > 0) {
        printf("Fehler!\n");
        errorCount++;
        break;
      }

      delete input;
      if (niResult != NULL) {
        delete niResult;
      }

      if (ssiResult != NULL) {
        delete ssiResult;
      }

      if (boResult != NULL) {
        delete boResult;
      }

      if (hosResult != NULL) {
        delete hosResult;
      }

      if (hoResult != NULL) {
        delete hoResult;
      }

#ifdef WIN32
#ifdef _DEBUG
      _CrtDumpMemoryLeaks();
#endif
#endif
    }
  }

#ifdef PERFORMANCETEST
    printf("%6d %4d | %6d : %6d | %6d : %6d\n",
           count,
           maximumIterations,
           loopsBentleyOttmann, 
           (int)(totalTimeBentleyOttmann / (CLOCKS_PER_SEC / 1000)),
           loopsHobby,
           (int)(totalTimeHobby / (CLOCKS_PER_SEC / 1000)));
#endif

}
