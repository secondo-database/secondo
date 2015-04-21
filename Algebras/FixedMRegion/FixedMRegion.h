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
#include "Move.h"
#include "LATransform.h"


//typedef int MBool;
//typedef int MPoint;

class FixedMRegion
{
  public:
/*
This is the default constructor. Do not use.

*/

    FixedMRegion();
/*
This is the copy constructor.

*/

    FixedMRegion(const FixedMRegion &f);
/*
This is the standard destructor.

*/

/*
This is the constructor that gets necessary information. 
region: object that shall be moved
move: th move object
starttime: the start time of the movement

*/  
FixedMRegion(Region * _region, const Move & _move, const Point &rot_center,
  double _starttime);
/*
deprecated
This is the constructor that should not be used externally. It gets necessary information. 

*/  
FixedMRegion(double _t, double _xm, double _ym, 
Region * _r, double _x0, double _y0, double _alpha0, 
double _vx, double _vy, double _valpha);
    

/*
This is the constructor that should be used externally. It gets necessary information.
region: object that shall be moved
start: point at which the object ist placed at the beginning $t=0$
alpha\_start: angle of object at the bginning $t=0$
speed: speed or velociy (v) of the movement
alpha\_speed: speed or velociy (v) of the angle movement
rot\_center: the rotational center of the object
starttime: the start time of the movement

*/  
FixedMRegion(Region * _region, const Point &_start, double alpha_start, 
   const Point &_speed, double alpha_speed, const Point &rot_center, 
   double _starttime=0.0);


    ~FixedMRegion();
    
/*
This is a method that accepts a list of regions. The regions represent 
spots and the movement will be calculated. The constructor expects identical 
regions that can be transformed by a translation or rotation. The region itself
 cannot cahnge its shape.

*/
Region interpolate(Region* spots);
//TODO
/*
This method will return a region that the FMRegion will have at the given 
time ti.

*/
Region * atinstant(double ti);
/*
This method will calculate a MBool with true, if the MPoint mp is inside the
FMRegion at the given time and false, if not. The MBool will be calculated 
for the time intervall ta to te.

*/
MBool inside(MPoint mp, double ta, double te, double precision=0.001);
//TODO
/*
This method will calculate a MPoint with which is defined, if the MPoint mp 
is inside the FMRegion at the given time and else not defined. The MPoint 
will be calculated for the time intervall ta to te.

*/
bool inside(MPoint mp, double t);
//Idee: MPoint inside(MPoint mp, double ta, double te, int steps);
/*
This method will calculate a MBool with true, if the MPoint mp intersects with
the FMRegion at the given time and false, if not. The MBool will be calculated 
for the time intervall ta to te.

*/
MBool intersection(MPoint mp, double ta, double te, double precision=0.001);
//TODO
/*
This method will calculate a MPoint with which is defined, if the MPoint mp 
intersects with the FMRegion at the given time and else not defined. The 
MPoint will be calculated for the time intervall ta to te.

*/
bool intersection(MPoint mp, double t);
//Idee: MPoint intersection(MPoint mp, double ta, double te, int steps);
/*
This method will calculate the Region which contains all points / areas, that
the FMRegion has at least on time (or more often) traversed in the given 
intervall ta to te. 

*/
Region * traversed2(double ta, double te, double precision=0.001);
Region * traversed(double ta, double te, double precision=0.001);
    
    const Region * getRegion() const;
    void setRegion(Region * _r);
    const double gett() const;
    void sett(double _t);
    
private:

   double t;
   Move m;
   Region * r;
   LATransform l;
   double xm;
   double ym;
   
/*
This method calculates the 
intersection point of the given lines p1p2 and p3p4.

*/
Point getIntersectionPoint(const Point & p1, const Point & p2, 
  const Point & p3, const Point & p4);

/*
This methods calculates the case of the intersection type. There are
a lot of different cases.

*/
int getTraversedCase(const Point & p1, const Point & p2, const Point & p3, 
  const Point & p4);
/*
This methods finds ou the correct case and calls other methods to calculate 
the polygon that was traversed in this step. It uses the halfsegments' start 
and end position. It has to deal with al lot of different cases.

*/
   vector<vector<Point> > getTraversedArea(const HalfSegment & hsold, 
   const HalfSegment & hsnew);
   
/*
This method orientates the given polygons clockwise. Therefore, they will be 
faces.

*/
   void traversedCreateCyclesNotHoles(vector< vector<Point> > & v_list);
/*
This methods creates a region that contains the traversed area between the 
step's region at t\_start and t\_end.

*/ 
   Region * getDiffRegion(const Region *resultold, const Region * resultnew);
/*
This methods creates a polygon of the given Points.

*/  
   vector<vector<Point> > traversedCalculateQuadrangle(const Point & p1, 
   const Point & p2, const Point & p3, const Point & p4);
/*
This methods creates a polygon of the given Points.

*/ 
   vector<vector<Point> > traversedCalculateTriangle(const Point & p1, 
     const Point & p2, const Point & p3);
   
/*
This methods creates a polygon of the given Points. This is an Internal 
method that is used from traversedCalculateTriangle.

*/   
   vector<Point> traversedCalcTriangle(const Point & p1, 
   const Point & p2, const Point & p3);
/*
This methods creates two polygons of the given Points, using the intersection
of the lines p1p2 and p3p4.

*/ 
   vector< vector<Point> > traversedCalculateTwoTriangles(Point p1, Point p2,
   Point p3, Point p4);
   
   vector<vector<Point> > traversedGetVectorVector(vector<Point> v);
   void calculateInternalVars();
   const LATransform& getLATransform();
   void setLATransform(const LATransform &_l);
   const Move& getMove();
   void setMove(const Move &_m);

}; 
/*
This methods creates a region that contains the traversed area between the 
step's region at t\_start and t\_end.

*/ 
#endif