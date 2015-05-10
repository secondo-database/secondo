/*
This my test methd.

*/
#ifndef __FIXEDMREGIONTEST_H
#define __FIXEDMREGIONTEST_H

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
#include "MovingRegionAlgebra.h"

class FMRTest {
public:
void testgenerateListOfRegionPoints();
void testgetOneDistance();
void testgetDistancesForPoint();
void testgenerateDistancesMatrix();
void testidentifyPoint();
void testidentifyPoints();
  

};

void runTestMethod ();

#endif
