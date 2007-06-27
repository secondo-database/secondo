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

Coordinates (x,y).

Developed in 16/11/96 by Geraldo Zimbrao da Silva.
Adapted in 17/09/97, same.
May, 2007 Leonardo Azevedo, Rafael Brand

*/

#include <math.h>
#include <stdlib.h>
#include "coord.h"
#define SAME_SIGNAL( a, b ) \
  (((a) > 0 && (b) > 0) || ((a) < 0 && (b) < 0))

//---------------------------------------------------------------------------
int Coordinate::operator < ( Coordinate c )
{
  if (this->x < c.x)
    return true;
  else
    if ( ( this->x==c.x ) && (this->y < c.y) )
      return true;
  return false;
}

long double RealCoordinate::module()
{
  return sqrtl( x*x + y*y );
}

long double RealCoordinate::angle( RealCoordinate c )
{
  if ( ( ( ( x * c.x + y *c.y) / ( this->module() * c.module() ) ) > 1) ||
       ( ( ( x * c.x + y *c.y) / ( this->module() * c.module() ) ) < -1) )
  {
    return 0;
  }
  return acosl( ( x * c.x + y *c.y) / ( this->module() * c.module() ) );
}

RealCoordinate RealCoordinate::operator - ( RealCoordinate c )
{
  return RealCoordinate( x - c.x , y - c.y );
}

RealCoordinate RealCoordinate::operator + ( RealCoordinate c )
{
  return RealCoordinate( x + c.x, y + c.y );
}

int RealCoordinate::operator < ( RealCoordinate c )
{
  if (this->x < c.x)
    return true;
  else
    if ( ( this->x==c.x ) && (this->y < c.y) )
      return true;
  return false;
}

int RealCoordinate::operator <= ( RealCoordinate c )
{
  if (*this==c)
    return true;
  if (*this < c)
    return true;
  return false;
}

int RealCoordinate::operator > ( RealCoordinate c )
{
  return !(*this<=c);
}

int RealCoordinate::operator !=( RealCoordinate c )
{
  return (x != c.x) || (y != c.y);
}

int RealCoordinate::operator ==( RealCoordinate c )
{
  return !(*this!=c);
}

int Coordinate::operator != ( Coordinate c )
{
  return (x != c.x) || (y != c.y);
}

static inline void orderByX( Coordinate& a, Coordinate& b )
{
  if( a.x > b.x )
  {
    Coordinate aux = a;
    a = b;
    b = aux;
  }
}

static inline void orderByY( Coordinate& a, Coordinate& b )
{
  if( a.y > b.y )
  {
    Coordinate aux = a;
    a = b;
    b = aux;
  }
}

double triangleArea( Coordinate a, Coordinate b, Coordinate c )
{
  double area = ((double) (b.x - a.x)) * (c.y - a.y) -
                ((double) (c.x - a.x)) * (b.y - a.y);

  return area / 2.0;
}

long linesCross( Coordinate ai, Coordinate af,
                   Coordinate bi, Coordinate bf, Coordinate* intersection )
{
  double a1, a2, b1, b2, c1, c2; // coefficient of the equation: ax + by + c = 0
  double denominator;

  a1 = (double) af.y - (double) ai.y;
  b1 = (double) ai.x - (double) af.x;
  c1 = (double) af.x * (double) ai.y - (double) ai.x * (double) af.y;

  a2 = (double) bf.y - (double) bi.y;
  b2 = (double) bi.x - (double) bf.x;
  c2 = (double) bf.x * (double) bi.y - (double) bi.x * (double) bf.y;

  {
    // Side of the segment where the point is located.
    double r1 = a1 * bi.x + b1 * bi.y + c1, 
           r2 = a1 * bf.x + b1 * bf.y + c1;

    if( r1 != 0 && r2 != 0 && SAME_SIGNAL( r1, r2 ) )
      // If they have the same signal, then they are 
      //on the same side of the segment
      return 0; 
  }

  {
    // Side of the segment where the point is located.
    double r1 = a2 * ai.x + b2 * ai.y + c2, 
           r2 = a2 * af.x + b2 * af.y + c2;

    if( r1 != 0 && r2 != 0 && SAME_SIGNAL( r1, r2 ) )
      // If they have the same signal, then they are
      // on the same side of the segment
      return 0; 
  }

  // There is intersection or they are colinears.

  denominator = a1 * b2 - a2 * b1;

  if( fabs( denominator ) < 1E-12 )
  { // Colinears. Checks if there is intersection of 0, 1, or infinite points.
    orderByX( ai, af );
    orderByX( bi, bf );

    if( ai == bf || af == bi )
    { // One point of intersection.
      if( intersection != NULL )
        *intersection = ai == bf ? ai : af;

      return 1;
    }

    if( af.x < bi.x || ai.x > bf.x )
      return 0;
    else
    {
      orderByY( ai, af );
      orderByY( bi, bf );
      if( af.y < bi.y || ai.y > bf.y )
        return 0;
      return 2;
    }
  }

  if( intersection != NULL )
  {  // Calculates the intersection.
    double displacement = denominator < 0 ? - denominator / 2 : denominator / 2,
           numerator = b1 * c2 - b2 * c1;

    intersection->x = (long) ((numerator < 0 ? numerator - displacement  :
                                      numerator + displacement ) / denominator);
    numerator = a2 * c1 - a1 * c2;
    intersection->y = (long) ((numerator < 0 ? numerator - displacement  :
                                      numerator + displacement ) / denominator);
  }

  return 1;
}

