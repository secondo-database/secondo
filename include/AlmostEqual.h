/*
----
This file is part of SECONDO.

Copyright (C) 2004-2007, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

1 Header File: AlmostEqual

This file defines several different methods to test float values on whether they
are semantically equal and selects the standard method for more complex ones
(as for Points, Units, etc.).

The implementation and the standard tolerance factors are located in
file ``../Tools/Utilities/AlmostEqual.cpp''.

*/

#ifndef ALMOSTEQUAL_H
#define ALMOSTEQUAL_H

#include <stdint.h>

double getAlmostEqualFactor();
int64_t getAlmostEqualDeltaNumbers();


const double FACTOR = getAlmostEqualFactor();// Precision factor. Maximun
                                             //   tolerated absolute difference
                                             //   between "equal" float numbers.
                                             //   used by AlmostEqualAbsolute()

const int64_t DELTA_NUMBERS =  getAlmostEqualDeltaNumbers();


inline double getAlmostEqualFACTOR(){
  return FACTOR;
}

inline int64_t getAlmostEqualDELTA_NUMBERS(){
  return DELTA_NUMBERS;
}


// standard methode
bool AlmostEqual( const double &d1, const double &d2 );

// Using absolute difference
bool AlmostEqualAbsolute( const double &d1,
                                 const double &d2, const double &epsilon);

// Using distance in number of separating prepresentable in-between values
bool AlmostEqual2sComplement( const double &A,
                                     const double &B, const int64_t &maxUlps  );

bool AlmostEqual_CheckTypeSizes();

#endif
