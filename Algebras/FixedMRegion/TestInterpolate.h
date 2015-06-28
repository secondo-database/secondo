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
#include "Move.h"
#include "LATransform.h"
#include "FixedMRegion.h"
#include "FMRInterpolator.h"

class TestInterpolate {
public:
  void testcalcMasspoint();
  void testsetReferenceRegion();
  void testcalcMaxMinDistPoint1();
  void testcalcMaxMinDistPoint2();
  void testcalcMaxMinDistPoint3();
  void testCalcDistVector();
  void testmatchVectors();
  void testcalculateAngleToXAxis();
};

void runTestInterpolateMethod();


#endif
