/*

This class is a FixedMRegion.

*/
#ifndef __FMRINTERPOLATOR_H
#define __FMRINTERPOLATOR_H

class FixedMRegion;

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
#include "PointStore.h"


class FMRInterpolator
{
friend class TestInterpolate;
public:     
/*
This is the default constructor. Do not use.

*/
  FMRInterpolator();
  FixedMRegion interpolate(const std::vector<IRegion> &spots);
/*
This method creates the FixedMRegion from the sights.

*/

FixedMRegion interpolatetest(const vector<Region>& regions, 
  const vector<DateTime>& times, const Region* ref_Region=NULL, 
  const Point* start_Point=NULL);

private:
  Point rotcenter;
  double angle_init;
  Region refRegion;//reference region for interpolate
  vector<double> distVector;
/*  
This method sets the angle of the refRegion for internal use.

*/
void setDistVector(vector<double> a);
/*  
This method returns the angle of the refRegion for internal use.

*/
vector<double> getDistVector();
/*  
This method sets the angle of the refRegion for internal use.

*/
void setAngleInit(double a);
/*  
This method returns the angle of the refRegion for internal use.

*/
double getAngleInit();
/*
This method calculates the mass point of the given points.

*/
Point calcMasspoint(const vector<HalfSegment> &v);
/*
This method calculates the mass point of the given points.

*/
Point calcMasspoint(vector<Point> a);
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
This method sets the given object.

*/
void setRotcenter(Point _rotpoint);
/*
This method returns the masspoint.

*/
Point getRotcenter();


/*
This method calculates the distance between the two points.

*/
double getDist(Point a, Point b);
/*
This method calculates the point, that has got the maximum distance from
\_calcMasspoint. If this does not exist, it will return the point with 
the minimum distance from \_calcMasspoint.

*/
//Point calcMaxMinDistPoint(Region r, Point calcMasspoint);
/*
This method calculates the point, that has got the maximum distance from
\_calcMasspoint. If this does not exist, it will return the point with false.

*/
Point calcMaxDistPoint(Region r, Point calcMasspoint);
/*
This method calculates the point, that has got the minimum distance from
\_calcMasspoint. If this does not exist, it will return the point with false.

*/
Point calcMinDistPoint(Region r, Point calcMasspoint);
/*
This method checks, if the given point is already in the list.

*/
bool inList(vector<Point> list, Point p);
/*
This method creates a list of all region points.

*/
void createPointList(vector<HalfSegment> a, vector<Point>& result);

/*
This method sorts the points of the given list clockwise and according to the 
points distance to the given point ref.

*/

void sortList(vector<Point>& list, const Point& ref);
/*
This method creates a clockwise sorted list of region points.

*/
vector<Point> getSortedList(Region re);
/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> calcDistVector(Region r, Point calcMasspoint);
/*
This method calculates the distance vector for all points of \_r to
\_calcMasspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/

Point matchVectors(vector<double> distVector, 
  Region r, Point calcMasspoint);


/*
This method calculates the orientation between two angles. It will give back
true, if the shortest movement is positiv and else false.

*/
bool getTurnDir(double a1, double a2) const;
/*
This method finds out which method will identify a special point of th region.
It returns the following values:
1 maximum
2 minimum
3 vector

*/
int determineAngleAlgorithm();

/*
This method will calculate the angle of the given region with the chosen 
method.

*/
double calcThisAngle(Region r, int method);
/*
This method calculates the angles for all given objects and returns them in 
the angle vector.

*/
void calcAngles(vector<Region> rs, int method, 
  vector<double> &angles);
/*
This method calculates the translation for all given objects and returns them in 
the translation vector.

*/
void calcTranslations(vector<Region> rs, vector<Point> &translations);
/*
This method calculates the translation from the rotational center for all 
given objects and returns them in the final translation vector.

*/
void calcTranslationFromRotcenter(vector<Point>& translations,
  Point rotcenter, vector<Point>& final_translations);
/*
This method calculates the final angles. the result will always be the angle
with the shortest path from the angle before to this one.

*/
void calcFinalAngles(vector<double> &angles);
/*
This method creates all UMoves and puts them into a MMove that will be returned.

*/
MMove createMMove(const vector<DateTime>&times, 
vector<Point> final_translations, vector<double> angles);

};
#endif
