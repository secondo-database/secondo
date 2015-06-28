/*

This class is a FixedMRegion.

*/
#ifndef __FMRINTERPOLATOR_H
#define __FMRINTERPOLATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "Secondo_Include.h"
#include "MovingRegionAlgebra.h"
#include "Move.h"
#include "LATransform.h"
#include "FixedMRegiontest.h"
#include "MMove.h"
#include "TestInterpolate.h"
#include "FixedMRegion.h"

class FMRInterpolator
{
friend class TestInterpolate;
public:     
/*
This is the default constructor. Do not use.

*/
  FMRInterpolator();
  Region interpolate(const Region spots[]);
private:

  Region refRegion;//reference region for interpolate
/*
This method calculates the mass point of the given points.

*/
Point calcMasspoint(vector<HalfSegment>);
/*
This method sets the given object as a reference. The (0,0) will be the 
calculated, not the given mass point.

*/
void setReferenceRegion(Region _r, Point _calcMasspoint);
/*
This method returns the reference region.

*/
Region getReferenceRegion();
/*
This method calculates the orientation vector of the given Region and uses the
given, calculated mass point as the central point.

*/
//void calcOrientationVector(Region _r, Point _calcMasspoint);
/*
This method calculates the distance between the two points.

*/
double getDist(Point a, Point b);
/*
This method calculates the point, that has got the maximum distance from
\_calcMasspoint. If this does not exist, it will return the point with 
the minimum distance from \_calcMasspoint.

*/
Point calcMaxMinDistPoint(Region r, Point calcMasspoint);
/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> calcDistVector(Region r, Point masspoint);
/*
This method calculates the distance vector for all points of \_r to
\_calcMasspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/

Point matchVectors(vector<double> distVector, 
  Region r, Point calcMasspoint);
/*
This method calculates the angle between the given region and the x-axis.

*/
double calculateAngleToXAxis(Region r, Point calcMasspoint);
/*
This method calculates the mass center of the given points.

*/
Point calculateMassCenter(const vector<Point> &p);

};
#endif
