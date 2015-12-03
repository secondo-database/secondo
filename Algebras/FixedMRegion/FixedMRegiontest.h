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
#include "LATransform.h"
#include "FixedMRegion.h"
#include "MovingRegionAlgebra.h"
#include "ListUtils.h"

namespace temporalalgebra{

class FixedMRegion;

class FMRTest {
public:
void testgenerateListOfRegionPoints();
void testgetOneDistance();
void testgetDistancesForPoint();
void testgenerateDistancesMatrix();
void testidentifyPoint();
void testidentifyPoints();
void printLATransform(const FixedMRegion & r);  
void testgetIntersectionPoint();
void testgetTraversedCase();
void testtraversedGetVectorVector();
void testtraversedCalculateQuadrangle();
void testtraversedCalcTriangle();
void testtraversedCalculateTriangle();
void testtraversedCalculateTwoTriangles();
void testgetTraversedArea();
void testtraversedCreateCyclesNotHoles();
void testgetHSFromRegion();
void testatinstant();
void testgetDiffRegion();
void testtraversed();
void testRobustUnionGemeinsamerPunkt();
void testRobustUnionGemeinsameKante();
void testRobustUnionIntersection1();
void testRobustUnionIntersection2();
void testRobustUnionDisjunkt();
void testApproxMovement();
void testInsideVersusApproxMovement();
};

void runTestMethod ();

}

#endif
