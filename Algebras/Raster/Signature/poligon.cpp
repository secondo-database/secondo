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

A poligon formed by a sequence of N points, where the last point is equal to
the first. The points go from 0 to N - 1

Developed in 16/11/96 by Geraldo Zimbrao da Silva

May, 2007 Leonardo Azevedo, Rafael Brand

*/

#ifdef __BORLANDC__
#include <mem.h>
#else
#include <memory.h>
#endif

#include <math.h>
#include <iostream.h>
#include "assinat4CRS.h"
#include <poligon.h>
#include <5c.h>

#define sign(a) ((a<0) ? -1 : 1)

Poligon::Poligon( const Poligon& p ) :
numberOfPoints( p.numberOfPoints ),
points( new Coordinate[ p.numberOfPoints ] ),
buffer( (Coordinate *) points )
{
  memcpy( buffer, p.points, sizeof( Coordinate ) * numberOfPoints );
}

Poligon::Poligon( unsigned numberOfPoints, Coordinate points[] ) :
numberOfPoints( numberOfPoints ),
points( points ),
buffer( points )
{
}

Poligon::Poligon( Coordinate points[], unsigned numberOfPoints ) :
numberOfPoints( numberOfPoints ),
points( points ),
buffer( NULL )
{
}

Poligon::~Poligon()
{
  if( buffer != NULL )
    delete buffer;
}

void Poligon::move( long x, long y )
{
  unsigned i;

  for( i = 0; i < numberOfPoints; i++ )
  {
    buffer[ i ].x += x;
    buffer[ i ].y += y;
  }
}

MBR Poligon::mbr() const
{
  Coordinate min( points[ 0 ].x, points[ 0 ].y ), max = min;
  unsigned i;

  for( i = 1; i < numberOfPoints; i++ )
  {
    if( points[ i ].x > max.x )
      max.x = points[ i ].x;
    else
      if( points[ i ].x < min.x )
        min.x = points[ i ].x;

    if( points[ i ].y > max.y )
      max.y = points[ i ].y;
    else
      if( points[ i ].y < min.y )
        min.y = points[ i ].y;
  }

  return ::MBR( min, max );
}

Plane Poligon::minimumPlane( long* sizeOfCell,
                             unsigned* numberOfCellsX,
                             unsigned* numberOfCellsY ) const
{
  Coordinate min( points[ 0 ].x, points[ 0 ].y ), max = min;
  long &factor = *sizeOfCell;
  unsigned i;

  for( i = 1; i < numberOfPoints; i++ )
  {
    if( points[ i ].x > max.x )
      max.x = points[ i ].x;
    else
      if( points[ i ].x < min.x )
        min.x = points[ i ].x;

    if( points[ i ].y > max.y )
      max.y = points[ i ].y;
    else
      if( points[ i ].y < min.y )
        min.y = points[ i ].y;
  }

  for( i = 0; i < 8 * sizeof( long ); i++ )
  {
    factor = 1l << i;
    Coordinate lmax( max.x < 0 ? max.x - factor : max.x,
                     max.y < 0 ? max.y - factor : max.y ),
               lmin( min.x < 0 ? min.x - factor : min.x,
                     min.y < 0 ? min.y - factor : min.y );
    double dx = lmax.x / factor - lmin.x / factor + 1,
           dy = lmax.y / factor - lmin.y / factor + 1;

    long larger = (long) (dx > dy ? dx : dy);

    if( dx * dy <= (double) PRODUCT )
    {
      *numberOfCellsX = (int) dx;
      *numberOfCellsY = (int) dy;

      return Plane( Coordinate( (lmin.x / factor) * factor, 
                (lmin.y / factor) * factor),
                Coordinate( ((lmin.x / factor) + larger) * factor,
                        ((lmin.y / factor) + larger) * factor), factor );


    }
  }

  return Plane( Coordinate( 0, 0 ), Coordinate( 0, 0 ), 0 );
}

void Poligon::move( Coordinate c )
{
  move( c.x, c.y );
}

void Poligon::turn( int a )
{
  double rad = (a * 3.14159) / 180.0,
         senA = sin( rad ),
         cosA = cos( rad );
  unsigned i;

  for( i = 0; i < numberOfPoints; i++ )
  {
    const Coordinate &o = buffer[ i ];
    Coordinate n( (long) (o.y * senA + o.x * cosA),
                  (long) (o.y * cosA - o.x * senA) );

    buffer[ i ] = n;
  }
}

void Poligon::scale( int factor )
// If factor == 0, mantains the original size.
// If factor > 0, multiplies the factor.
// If factor < 0, divides by abs( factor ).
{
  if( factor != 0 )
  {
    double multiplier = (factor > 0 ? factor : -1.0 / factor);
    unsigned i;

    for( i = 0; i < numberOfPoints; i++ )
    {
      buffer[ i ].x = (long) (buffer[ i ].x * multiplier);
      buffer[ i ].y = (long) (buffer[ i ].y * multiplier);
    }
  }
}

void Poligon::scaleF( double factor )
{
  unsigned i;

  for( i = 0; i < numberOfPoints; i++ )
  {
    buffer[ i ].x = (long) (buffer[ i ].x * factor);
    buffer[ i ].y = (long) (buffer[ i ].y * factor);
  }
}

long double Poligon::area() const
{
  unsigned i;
  double sum = 0.0;
  if (numberOfPoints <= 0) return 0;
  for( i = 1; i < numberOfPoints - 2; i++ )
    sum += triangleArea( points[ 0 ], points[ i ], points[ i + 1 ] );

  return abs((long)sum);
}


static unsigned m[ PRODUCT ];

