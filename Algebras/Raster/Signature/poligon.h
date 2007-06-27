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

A pligon formed by a sequence of N points, where the last point is equal to
the first. The points go from 0 to N - 1
Developed in 16/11/96 by Geraldo Zimbrao da Silva

May, 2007 Leonardo Azevedo, Rafael Brand

*/

#ifndef POLIGONO_H
#define POLIGONO_H

#ifdef DEBUG
#include <myheap.h>
#endif
#include "Plane.h"
//#include "plano4CRS.h"
//#include <stdio.h>
#include "vhline.h"

class istream;
class ostream;
class Signature4CRS;


class Poligon
{
  public:
    // --- Attributes ----------------------------------------------------
    const unsigned numberOfPoints;
      // Number of points of the poligon. One more than the number of vertices
    const Coordinate * const points;
      // List of points.

    // --- Constructors and Destructor -------------------------------------
    Poligon( const Poligon& );
    Poligon( unsigned numberOfPoints, Coordinate points[] );
      //Poligon manages the list of points, desalocating then in his desctructor
    ~Poligon();

    // --- Modifiers ------------------------------------------------
    void move( Coordinate );
    void move( long x, long y );
      // Moves the poligon accorgin to the vector (x, y).
    void turn( int angle );
      // Turns the poligon in 'angle' degrees around the point (0,0).
    void scale( int factor );
      // If factor == 0, mantains the original size.
      // If factor > 0, multiplies the factor.
      // If factor < 0, divides by abs( factor ).
    void scaleF( double factor );
      // Just multiplies by factor.

    // --- Selectors ----------------------------------------------------
    MBR mbr() const;
      // Returns MBR.
    Plane minimumPlane( long* sizeOfCell,
                       unsigned* numberOfCellsX,
                       unsigned* numberOfCellsY ) const;

    long double area() const;
      // Returns the area of the poligon.

  protected:
    // --- Constructors and Destructor -------------------------------------
    Poligon( Coordinate points[], unsigned numberOfPoints );
      // Poligon does not assumes the management of the list
      // of points, not desalocating then in his desctructor

  private:
    // --- Attributes ----------------------------------------------------
    Coordinate *buffer;
};

#endif
