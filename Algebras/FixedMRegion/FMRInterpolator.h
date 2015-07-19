/*

This class is a FixedMRegion.

*/
#ifndef __FMRINTERPOLATOR_H
#define __FMRINTERPOLATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "FixedMRegion.h"
#include "Secondo_Include.h"
#include "MovingRegionAlgebra.h"
#include "Move.h"
#include "LATransform.h"
#include "FixedMRegiontest.h"
#include "MMove.h"
#include "TestInterpolate.h"
#include "PointStore.h"

class FMRObservation
{
public:
  FMRObservation(const Point& _translation, double _angle, 
                 const Instant& _time, bool _valid=true):
                 translation(_translation), angle(_angle), 
                 time(_time), valid(_valid) {}

  FMRObservation(): translation(Point(0)), angle(0),
                    time(Instant(0.0)), valid(false) {}

  FMRObservation(const FMRObservation& o):
                 translation(o.translation), angle(o.angle), time(o.time) {}

  inline Point getTranslation() const { return translation; }

  inline void setTranslation(Point & _translation) {
    translation=_translation;
  }

  inline double getAngle() const { return angle; }

  inline void setAngle(double _angle) {
    angle=_angle;
  }

  inline Instant getTime() const { return time; }
  
  bool isValid() const {return valid;}
  void setValid(bool _valid=true) {
    valid=_valid;
  }

  inline bool operator<(const FMRObservation& o) const {
    return time<o.time;
  }

  inline bool operator>(const FMRObservation &o) const {
    return time>o.time;
  }

  inline bool operator==(const FMRObservation& o) const {
    return time==o.time;
  }

private:
  Point translation;
  double angle;
  Instant time;
  bool valid;
};

class FMRInterpolator
{
friend class TestInterpolate;
public:
/*
This is the default constructor.

*/
  FMRInterpolator(const Region* _refRegion=NULL, 
                  const Point* _rotCenter=NULL);
  void start();
  void end();

  void addObservation(const IRegion& observation);
  
  FixedMRegion getResult();

private:
  vector<FMRObservation> observations;
  FixedMRegion result;
  
  double angle_init;
  int angle_method;
  Region refRegion;//reference region for interpolate
  vector<double> distVector;
/*
This method calculates the mass point of the given points.

*/
Point calcMasspoint(const vector<HalfSegment> &v) const;
/*
This method calculates the mass point of the given points.

*/
Point calcMasspoint(const vector<Point> &a) const;
/*
This method sets the given object as a reference. The (0,0) will be the 
calculated, not the given mass point.

*/
void setReferenceRegion(const Region &r, const Point & masspoint);

/*
This method calculates the distance between the two points.

*/
double getDist(const Point &a, const Point &b) const;
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
Point calcMaxDistPoint(const Region &r, const Point &masspoint) const;
/*
This method calculates the point, that has got the minimum distance from
\_calcMasspoint. If this does not exist, it will return the point with false.

*/
Point calcMinDistPoint(const Region &r, const Point &masspoint) const;
/*
This method checks, if the given point is already in the list.

*/
bool inList(const vector<Point> &list, const Point &p) const;
/*
This method creates a list of all region points.

*/
void createPointList(const vector<HalfSegment>& a, vector<Point>& result) const;

/*
This method sorts the points of the given list clockwise and according to the 
points distance to the given point ref.

*/

void sortList(vector<Point>& list, const Point& ref) const;
/*
This method creates a clockwise sorted list of region points.

*/
vector<Point> getSortedList(const Region &re) const;
/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> calcDistVector(const Region &r, const Point &masspoint) const;
/*
This method calculates the distance vector for all points of \_r to
\_calcMasspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/

Point matchVectors(const Region &r, const Point &masspoint) const;


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
double calcThisAngle(const Region &r) const;

/*
This method calculates the final angles. the result will always be the angle
with the shortest path from the angle before to this one.

*/
void calcFinalAngles();
/*
This method creates all UMoves and puts them into a MMove that will be returned.

*/
MMove createMMove() const;

};
#endif
