/*
This my test methd.

*/
#ifndef __TESTINTERPOLATE_H
#define __TESTINTERPOLATE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Attribute.h"
#include "Symbols.h"
#include "math.h"
#include "Secondo_Include.h"
#include "LATransform.h"
#include "FixedMRegion.h"
#include "FMRInterpolator.h"

namespace temporalalgebra{

class TestInterpolate {
public:
  void testcalcMasspoint();
  void testsetReferenceRegion();
  void testsetMasspoint();
  void testcalcMaxDistPoint1();
  void testcalcMinDistPoint2();
  void testcalcMaxDistPoint3();
  void testcalcMinDistPoint3();
  void testInList();
  void testCreatePointList();
  void testCalcDistVector();
  void testmatchVectors();
  void test_calcAngle(double x, double y);
  void test_getTurnDir(double a1, double a2);
  void testSortList();
  void testGetSortedList();
  void testCalcAngle();
  void testGetTurnDir();
  void testcalculateAngleToXAxis();
  void testinterpolatetest();
  void testsetDistVector();
  void testsetAngleInit();
  void testsetRotcenter();
  void testdetermineAngleAlgorithm();
  void testcalcThisAngle();
  void testcalcAngles();
  void testcalcTranslations();
  void testcalcFinalAngles();
  void testcreateMMove();
};

void runTestInterpolateMethod();

}

#endif
