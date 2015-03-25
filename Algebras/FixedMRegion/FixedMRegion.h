/*

This class is a FixedMRegion.

*/
#ifndef __FIXEDMREGION_H
#define __FIXEDMREGION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

*/  
    FixedMRegion(double _t, 
const Move & _m, Region * _r, const LATransform & _l);  
/*
This is the constructor that should not be used externally. It gets necessary information. 

*/  
    FixedMRegion(double _t, double _xm, double _ym, 
Region * _r, double _x0, double _y0, double _alpha0, 
double _vx, double _vy, double _valpha);
    


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
MBool inside(MPoint mp, double ta, double te, double precision);
//TODO
/*
This method will calculate a MPoint with which is defined, if the MPoint mp 
is inside the FMRegion at the given time and else not defined. The MPoint 
will be calculated for the time intervall ta to te.

*/
//Idee: MPoint inside(MPoint mp, double ta, double te, int steps);
/*
This method will calculate a MBool with true, if the MPoint mp intersects with
the FMRegion at the given time and false, if not. The MBool will be calculated 
for the time intervall ta to te.

*/
MBool intersection(MPoint mp, double ta, double te, double precision);
//TODO
/*
This method will calculate a MPoint with which is defined, if the MPoint mp 
intersects with the FMRegion at the given time and else not defined. The 
MPoint will be calculated for the time intervall ta to te.

*/
//Idee: MPoint intersection(MPoint mp, double ta, double te, int steps);
/*
This method will calculate the Region which contains all points / areas, that
the FMRegion has at least on time (or more often) traversed in the given 
intervall ta to te. 

*/
Region traversed(double ta, double te, double precision);
//TODO    
    
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

   
   void calculateInternalVars();
   const LATransform& getLATransform();
   void setLATransform(const LATransform &_l);
   const Move& getMove();
   void setMove(const Move &_m);

}; 
#endif