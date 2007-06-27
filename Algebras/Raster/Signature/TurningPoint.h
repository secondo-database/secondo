/*
---- 
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science, 
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

May, 2007 Leonardo Azevedo, Rafael Brand

*/
//---------------------------------------------------------------------------
#ifndef TurningPointH
#define TurningPointH
//#include "RealCoordinate.h"
//#include "Secondo.h"
#include "Segment.h"
#include "../../Spatial/SpatialAlgebra.h"

//---------------------------------------------------------------------------
class TurningPoint
{

  public:
    double x,y;
    Segment s;
    unsigned defined;
    TurningPoint(){defined=0;}

    TurningPoint(const RealCoordinate p,const bool dir,const bool reject,
                 Segment segment)
    {
       set(p.x,p.y,dir,reject,segment);
    }

    void set(const long double& X, const long double& Y,const bool dir,
             const bool reject,Segment segment)
    {
      this->x = X;
      this->y = Y;
      this->direction = dir;
      this->rejected = reject;
      this->s = segment;
      defined=1;
    }

    void set(const RealCoordinate p,const bool dir,const bool reject,
             Segment segment)
    {
      set(p.x,p.y,dir,reject,s);
    }

    TurningPoint& operator=(const TurningPoint& p)
    {
      this->x = p.x;
      this->y = p.y;
      this->direction = p.direction;
      this->rejected = p.rejected;
      this->s = p.s;
      this->defined = p.defined;
      return *this;
    }
    
    bool AlmostEqual( const double d1, const double d2 )
    {
      double factor=FACTOR;
      if ( abs(d1 + d2) < 1 )
        factor *= abs( d1 + d2 );
    
      if( abs(d1 - d2) <= factor )
        return true;
      return false;
    }


  static bool getDirectionTurningPoint(const RealCoordinate &p,
                                            const RealCoordinate &p2,
                                            bool insideAbove,
                                            const RealCoordinate &v);

    bool operator==(const TurningPoint& p)
    {
      if( AlmostEqual( this->x, p.x ) &&
          AlmostEqual( this->y, p.y ) &&
          (this->direction == p.direction)  &&
          (this->rejected == p.rejected) )
          return true;
      return false;
    }

    bool operator!=(const TurningPoint &p)
    {
      return !(*this==p);
    }


    inline bool operator<( const TurningPoint& p ) const
    {
      if( this->x < p.x )
        return 1;
      else
        if( this->x == p.x && this->y < p.y )
          return 1;
        else
          if( this->x == p.x && this->y == p.y )
          { //If both points has the same coordinates, if they have diferent
            //directions the less one will be that has the attribute direction
            //true (left and down), otherwise, both have direction true, and
            //the less one will be that was rejected.
            //The rejected points come before accepted points
            if (this->direction == p.direction)
            {
              if (this->rejected)
                return 1;
            }
            //If the directions are different then the turning point that
            //comes first according to the PointList must be considered as first
            else
              if (this->s.id < p.s.id)
                return 1;
              //if (this->direction)
              //return 1;
          }
      return 0;
    }



    /*
    Assignement operator redefinition.

    *Precondition:* ~p.IsDefined()~

    */
    /*
      This function sets the value of the "attr" argument of a half segment.
    */

    bool direction;

/*
The direction attributes represents where is the area of the region related to
the point and the edge of the window in which it lies:
--> For horizontal edges (top and bottom edges), its value is true if the
    area of the region is on the left (<==), and false if it 
    lies on the right (==>).
--> For vertical edges (left and right edges), its value is true if the 
    area of the region is down the point (DOWN), and false otherwise (UP).

*/

    bool rejected;
    //Indicates if the point is of a segment that was rejected
};
//---------------------------------------------------------------------------
#endif
