
/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of
Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
 by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston,
MA  02111-1307  USA
----

*/

/*
~ComputeMEquality~

This function compoutes the moving equality of two point units defined
over the same time interval. The interval is given by iv. The 
movements of the uints are given by the start and endpoints of both points.
The result is stored in the variable result. If CompValue is set to be 
true, a check of equality is done, otherwise a check for inequality.

*/

#ifndef TEMPORALLIFTED_ALGEBRA
#define TEMPORALLIFTED_ALGEBRA

#include "TemporalAlgebra.h"
#include "SpatialAlgebra.h"
#include "StandardTypes.h"

void computeMEquality(const Interval<Instant>& iv,
                 const Point& p1_start, const Point& p1_end,
                 const Point& p2_start, const Point& p2_end,
                 MBool& result,
                 const bool compValue);

#endif



