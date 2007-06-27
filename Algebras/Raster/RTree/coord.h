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

Coordenates (x,y).

Developed in 16/11/96 by Geraldo Zimbrao da Silva.
Adapted in 17/09/97, same.
May, 2007 Leonardo Azevedo, Rafael Brand

*/


#ifndef COORD_H
#define COORD_H

struct Coordinate
{
  long x;
  long y;

  Coordinate()
    {}

  Coordinate( const long x, const long y ) :
    x( x ), y( y ) {}

  int operator != ( Coordinate c );
  int operator < (Coordinate c);
};

struct RealCoordinate
{
  long double x, y;
  inline RealCoordinate()
    {};

  inline RealCoordinate( const Coordinate& c ) :
    x( c.x ), y( c.y ) {};

  inline RealCoordinate( const long double x, const long double y ):
    x( x ), y( y )
    {};

  int operator != ( RealCoordinate c );
  int operator == ( RealCoordinate c );
  RealCoordinate operator - ( RealCoordinate c );
  RealCoordinate operator + ( RealCoordinate c );
  int operator < ( RealCoordinate c );
  int operator <= ( RealCoordinate c );
  int operator > ( RealCoordinate c );

  long double module();
  long double angle( RealCoordinate c );
};

struct rRectangle
{
  Coordinate min, max;

  rRectangle() :
    min(), max()
    {}

  rRectangle( Coordinate min, Coordinate max ) :
    min( min ), max( max ) {}

  inline long contais( Coordinate point ) const;
    // Checks if the Rectangle contais the point.
};

typedef rRectangle MER;

inline Coordinate operator - ( Coordinate );
inline Coordinate operator + ( Coordinate, Coordinate );
inline Coordinate operator * ( double, Coordinate );
//inline int operator == ( Coordinate, Coordinate );
inline int operator == ( rRectangle, rRectangle );
inline double rectangleArea( Coordinate, Coordinate );
  // Area of the Rectangle.
inline long contaisPoint( rRectangle, Coordinate point );
inline long contaisPoint( Coordinate min, Coordinate max, Coordinate point );
  // Checks if the rectangle defined by (min, max) contains the point
  // - Returns 0 if doesn´t.
  // - Returns 1 if does.
inline long existsIntersection( const rRectangle& a, const rRectangle& b );
inline long existsIntersection( const Coordinate& minA, const Coordinate& maxA,
                             const Coordinate& minB, const Coordinate& maxB );
  // Checks the intersection:
  // - Returns 0 if the rectangles defined by (minA,maxA) and (minB,maxB)
  //   doesn´t have intersection. (Rejected)
  // - Returns 1 if the rectangles defined by (minA,maxA) and (minB,maxB)
  //   have intersection. (Candidate)
  // - Returns 2 if the rectangles defined by (minA,maxA) and (minB,maxB)
  //   cross each other. (Accepted)
long linesCross( Coordinate a1, Coordinate a2,
                   Coordinate b1, Coordinate b2, Coordinate* intersection = 0 );
  // Checks the intersection of two segments of line
  // - Returns 0 if there is no intersection (whether they are colinears or not)
  // - Returns 1 if there is a point of intersection 
  //    (whether they are colinears or not)
  // - Returns 2 if there is more than 1 point of intersection 
  //    (colinears segments)
  // OBS: even if the segments are colinears, if the beginning of the first
  // coincides with the end of the second, it will be considered only one 
  //     intersection
  // In cases 0 and 2, nothing will be returned in 'intersection'
double triangleArea( Coordinate a, Coordinate b, Coordinate c );
  // Returns the area (with signal) of the triangle abc.

//--- INLINES ---------------------------------------------------------

inline Coordinate operator - ( Coordinate c )
{
  return Coordinate( - c.x , - c.y );
}

inline Coordinate operator + ( Coordinate a, Coordinate b )
{
  return Coordinate( a.x + b.x, a.y + b.y );
}

inline Coordinate operator * ( double n, Coordinate a )
{
  return Coordinate( (long) (n * a.x), (long) (n * a.y) );
}

inline int operator == ( Coordinate a, Coordinate b )
{
  return (a.x == b.x) && (a.y == b.y);
}

inline int operator == ( rRectangle a, rRectangle b )
{
  return (a.min == b.min) && (a.max == b.max);
}

inline double rectangleArea( Coordinate a, Coordinate b )
{
  return ((double) (b.x - a.x + 1)) * ((double) (b.y - a.y + 1));
}

inline long contaisPoint( rRectangle r, Coordinate ponto )
{
  return contaisPoint( r.min, r.max, ponto );
}

inline long existsIntersection( const Coordinate& minA, const Coordinate& maxA,
                             const Coordinate& minB, const Coordinate& maxB )
{
  if( maxA.x < minB.x ) return 0;
  if( maxB.x < minA.x ) return 0;
  if( maxA.y < minB.y ) return 0;
  if( maxB.y < minA.y ) return 0;

  if( minA.x < minB.x && maxA.x > maxB.x &&
      minB.y < minA.y && maxB.y > maxA.y ) return 2;

  if( minB.x < minA.x && maxB.x > maxA.x &&
      minA.y < minB.y && maxA.y > maxB.y ) return 2;

  return 1;
}

inline long existsIntersection( const rRectangle& a, const rRectangle& b )
{
  return existsIntersection( a.min, a.max, b.min, b.max );
}

inline long rRectangle::contais( Coordinate point ) const
{
  return contaisPoint( min, max, point );
}


inline long contaisPoint( Coordinate min, Coordinate max, Coordinate point )
{
  if( point.x < min.x || point.x > max.x )
    return 0;
  else if( point.y < min.y || point.y > max.y )
    return 0;
  else
    return 1;
}

#endif // PLANO_H
