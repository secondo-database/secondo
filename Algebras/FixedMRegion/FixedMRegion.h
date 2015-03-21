/*

This class is a FixedMRegion.

*/
#ifndef __FIXEDMREGION_H
#define __FIXEDMREGION_H

#include "Move.h";
#include "LATransform.h";

class FixedMRegion
{
  public:
/*
This is the constructor. It gets necessary information. 

*/  
    FixedMRegion(const Region &_r, const LATransform &_l, 
const Move &_m, double t);
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
    ~FixedMRegion();
    
/*
This is a constructor that accepts a list of regions. The regions represent 
spots and the movement will be calculated. The constructor expects identical 
regions that can be transformed by a translation or rotation. The region itself
 cannot cahnge its shape.

*/
    FixedMRegion(Region* spots);
/*
This method will return a region that the FMRegion will have at the given 
time ti.

*/
Region atinstant(double ti);
/*
This method will calculate a MBool with true, if the MPoint mp is inside the
FMRegion at the given time and false, if not. The MBool will be calculated 
for the time intervall ta to te.

*/
MBool inside(MPoint mp, double ta, double te), int steps;
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
MBool intersection(MPoint mp, double ta, double te, int steps);
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
Region traversed(double ta, double te, int steps);
    
    
    const Region& getRegion();
    void setRegion(const Region &_r);
    const LATransform& getLATransform();
    void setLATransform(const LATransform &_l);
    const Move& getMove();
    void setMove(const Move &_m);
    double gett();
    void sett(double _t);
    
private:
   Region r;
   LATransform l;
   Move m;
   double t;
   //void calculateInternalVars();
}; 
#endif