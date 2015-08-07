/*

This class is a FixedMRegion.

*/
#ifndef __FIXEDMREGION_H
#define __FIXEDMREGION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "Secondo_Include.h"
#include "MovingRegionAlgebra.h"
#include "MovingRegion3Algebra.h"
#include "LATransform.h"
#include "FixedMRegiontest.h"
#include "MMove.h"

class TestInterpolate;


class FixedMRegion: public Attribute{
  friend class FMRTest;
  friend class TestInterpolate;
public:     
/*
This is the default constructor. Do not use.

*/
FixedMRegion ();

FixedMRegion (int i);

/*
This is the copy constructor.

*/
FixedMRegion (const FixedMRegion & f);

/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_startp: start point at starttime for the movement
\_startangle: start angle at starttime for the movement
\_startt: start time
\_endp: end point at endtime for the movement
\_endangle: end angle at endtime for the movement
\_endt: end time 
rot\_center: rotational center

*/
FixedMRegion(const Region & _region, const Point _startp, 
const double _startangle, const Instant _startt, const Point _endp, 
const double _endangle, const Instant _endt, const Point & rot_center);
  
/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_startp: start point at starttime for the movement
\_startangle: start angle at starttime for the movement
\_startt: start time
\_endp: end point at endtime for the movement
\_endangle: end angle at endtime for the movement
\_endt: end time
rot\_center: rotational center

*/
FixedMRegion(const Region & _region, const Point _startp, 
const double _startangle, const double _startt, const Point _endp, 
const double _endangle, const double _endt, const Point & rot_center);

/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_move: the move object
rot\_center: the center of the rotation. It will be moved with the object.
\_starttime: the start time of the movement

*/
FixedMRegion (const Region & _region, const MMove & _move,
const Point & rot_center, const double _starttime);

/*
deprecated
This is the constructor that should not be used externally. It gets necessary information. 
$\_t$: never used
$\_xm$: x-value of the rotation center, that will be moved, too
$\_ym$: y-value of the rotation center, that will be moved, too
$\_r$: the region to be moved
$\_x0$: the x-startvalue of the linear movement
$\_y0$: the y-startvalue of the linear movement
$\_alpha0$: the starting angle
$\_vx$: x-value of the linear movement
$\_vy$: y-value of the linear movement
$\_valpha$: the angle of movement.

*/
FixedMRegion (const double _t, const double _xm, const double _ym, 
const Region & _r, const double _x0, const double _y0, const double _alpha0,
const double _vx, const double _vy, const double _valpha);

/*
This is the constructor that should be used externally. It gets necessary information.
\_region: object that shall be moved
\_start: point at which the object ist placed at the beginning $t=0$
alpha\_start: angle of object at the bginning $t=0$
\_speed: speed or velociy (v) of the movement
alpha\_speed: speed or velociy (v) of the angle movement
rot\_center: the rotational center of the object
\_starttime: the start time of the movement

*/
FixedMRegion (const Region & _region, const Point & _start,
const double alpha_start, const Point & _speed, const double alpha_speed, 
const Point & rot_center, const double _starttime = 0.0);

/*
This is the standard destructor.

*/
~FixedMRegion ();

/*
This method will return the region that the FMRegion will have at the given 
time ti.

*/
void atinstant (const double ti, Region & result);

/*
This method will calculate a MPoint which is defined, if the MPoint mp 
is inside the FMRegion at the given time and otherwise not defined.

*/
MBool inside (const MPoint & mp);

/*
This method will calculate a MPoint which is defined, if the MPoint mp 
intersects with the FMRegion at the given time and else not defined.

*/
MPoint intersection(const MPoint & mp);

/*
This method will calculate the Region which contains all points / areas, that
the FMRegion has at least on time (or more often) traversed in the given 
intervall ta to te. 

*/
void traversedNew(Region & result, double ta, double te);

/*
This method returns the region of the FixedMRegion.

*/
const void getRegion (Region & result) const;

/*
This method sets the region of the FixedMRegion.

*/
void setRegion (const Region & _r);

/*
This method returns the time t.
not used

*/ 
const double gett () const;

/*
This method sets the time t.

*/ 
void sett (double _t);

/*
This method extracts a list of halfsegments from the given region.

*/
vector < HalfSegment > getHSFromRegion(const Region reg) const;

/*
Return the name of the Secondo type.

*/
static const string BasicType(){ return "fixedmregion"; }

/*
This method returns the Move.

*/
const MMove & getMove () const;

/*
This method sets the Move.

*/
void setMove (const MMove & _m);

inline Point getRotCenter() const {
  return Point(true, xm, ym);
}

/*
This method implements the clone functionality.

*/
virtual Attribute* Clone() const;

/*
This method implements the checkType function.

*/
static const bool checkType(const ListExpr type){
  return listutils::isSymbol(type, BasicType());
}

/*
This method copies all internal information from another FixedMRegion.

*/
virtual void CopyFrom(const Attribute*);

private:

  double t;//time, never used
  MMove m;
  Region r;//non moving region
  LATransform l;
  double xm;//rotational center
  double ym;//rotational center

/*
This method calculates the 
intersection point of the given lines p1p2 and p3p4.

*/
Point getIntersectionPoint (const Point & p1, const Point & p2,
const Point & p3, const Point & p4) const;

/*
This method extracts a list of halfsegments from the region.

*/
vector < HalfSegment > getHSFromRegion() const;
 
/*
This methods calculates the case of the intersection type. There are
a lot of different cases. This method will find out, which case the 
two lines p1p2 and p3p4 belong to. The cases have got a short explanation 
in getTraversedArea. A two digit value in a number system of base 4 is 
the result.

*/
int getTraversedCase (const Point & p1, const Point & p2,
const Point & p3, const Point & p4) const;

/*
This methods finds out the correct case and calls other methods to calculate 
the polygon that was traversed in this step. It uses the halfsegments' start 
and end position. It has to deal with al lot of different cases.

*/
vector < vector < Point > >getTraversedArea (const HalfSegment & hsold,
const HalfSegment & hsnew) const;

/*
This method orientates the given polygons clockwise. Therefore, they will be 
faces.

*/
void traversedCreateCyclesNotHoles(vector < vector < Point > >&v_list) const;

/*
This methods creates a region that contains the traversed area between 
resultold and resultnew.

*/
Region * getDiffRegion(const vector < HalfSegment > *resultold,
const vector < HalfSegment > *resultnew) const;

/*
This methods creates a polygon of the given Points.
This method adds four edges, the given points, to a vector. This is a 
preparation to create a quadrangle.

*/
vector < vector < Point > >traversedCalculateQuadrangle(const Point & p1, 
const Point & p2, const Point & p3, const Point & p4) const;

/*
This methods creates a polygon of the given Points.
This method adds three edges, the given points, to a vector. This is a 
preparation to create a triangle.

*/
vector < vector < Point > >traversedCalculateTriangle (const Point & p1,
const Point & p2,const Point & p3) const;

/*
This is an internal method that is used from traversedCalculateTriangle.
This methods creates a polygon of the given Points.
This method adds three edges, the given points, to a vector. This is a 
preparation to create a triangle.

*/
vector < Point > traversedCalcTriangle (const Point & p1,
const Point & p2, const Point & p3) const;

/*
This methods creates two polygons of the given Points, using the intersection
of the lines p1p2 and p3p4.
It will prepare two vectors with triangles and gives them back in a 
vectorvector.

*/
vector < vector < Point > >traversedCalculateTwoTriangles (const Point p1,
const Point p2, const Point p3, const Point p4) const;

/*
This method creates a vectorvector and puts the given vector into it.

*/
vector < vector < Point > > traversedGetVectorVector(
const vector < Point > v) const;

/*
This method calculates the internal variables. It is used, when
the time has changed.

*/
void calculateInternalVars ();

/*
This method will return a list of Halfsegments that the FMRegion will have 
at the given 
time ti.

*/
void atinstant (const double ti, const vector < HalfSegment > &v,
vector < HalfSegment > & res);

/*
This method will return a Point3 with the moving values x, y and alpha for the 
given double with time t.

*/
Point3 getMovingForTimeFromMMove (const double t, const bool noLimits=false)
const;

/*
This method will return a Point3 with the moving values x, y and alpha for the given 
DateTime t.

*/
Point3 getMovingForTimeFromMMove (const DateTime t, const bool noLimits=false)
const;

/*
This method returns the start time of the valid interval as an double
as absolute time.

*/
double getMMoveStart(const double dummy) const;

/*
This method calculates the step with depending on alpha.

*/
int calcStepWith(const double ta, const double te) const;

/*
This method calculates the aprroximated movement of the given MPoint
with the given precision unter the condition that the FixedMRegion does
not move. Therefore, it uses an inverse movement of the Region and lets
the MPoint move that way in addition to its own movement.

*/
MPoint approxMovement(const MPoint & p) const;

/*
This method moves the given Point inverse to the movement of the FixedMRegion
for time t and gives it back.

*/
void getInv(const double t, const Point & p, Point & result) const;

/*
This method creates a non moving MRegion.

*/
MRegion *buildMovingRegion();

/*
This method will join adjacent UBools, if they have got the same value.

*/
MBool joinBools(const MBool a) const;

/*
This calculates the euclidian distance between the two points.

*/
double getOneDistance(const Point  &p1, const Point &p2) const;

/*
This method calculates the euclidian distance between the points and the 
one indicated by numberOfPoint.

*/
vector<double> getDistancesForPoint(const int numberOfPoint, 
const vector<Point> &p) const;

/*
This method creates a MMove with the given values.

*/
MMove createMMove(const double _startX, const double _startY, 
const double _startangle, const Instant _startt, const double _endX, 
const double _endY, const double _endangle, const Instant _endt) const;

/*
This method creates a MMove with the given values.

*/
MMove createMMove(const double _startX, const double _startY, 
const double _startangle, const double _endX, const double _endY, 
const double _endangle) const;

/*
This method returns the size of the object.

*/
virtual size_t Sizeof() const;

/*
This method implements the compare functionality.

*/
virtual int Compare(const Attribute*) const;

/*
This method implements the adjacent functionality.

*/
virtual bool Adjacent(const Attribute*) const;

/*
This method implements the HashValue functionality.

*/
virtual size_t HashValue() const;

/*
This method implements the NumOfFLOBs functionality.

*/
virtual int NumOfFLOBs() const;

/*
This method implements the GetFLOB functionality.

*/
Flob* GetFLOB(const int i);

/*
This method implements the Print functionality.

*/
ostream& Print(ostream &os) const;
};
#endif
