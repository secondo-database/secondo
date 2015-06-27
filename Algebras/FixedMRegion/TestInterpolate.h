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

class TestInterpolate {
public:
  void testcalcMassPoint();
  void testsetReferenceRegion();
  void testcalcMaxMinDistPoint();
  void testcalcDistVectorsIdentSmallestRotFirstPoint();
  void testcalculateAngleToXAxis();
};

void runTestInterpolateMethod();


#endif
