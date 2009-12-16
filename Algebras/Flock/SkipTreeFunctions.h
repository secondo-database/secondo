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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] The file is originally provided by Thomas Wolle, and integrated into SECONDO
by Mahmoud Sakr

June, 2009 Mahmoud Sakr

[TOC]

*/

#ifndef SKIPTREEFUNCTIONS_HEADER
#define SKIPTREEFUNCTIONS_HEADER


#include "OctreeElements.h"
#include <map>

const int STABBING_SQUARE = -1;

void 
deleteSkipQuadtree(::std::map<int, OctreeCell*>* skipQuadtree);

int
boxedRangeQueryCounting(::std::map<int, OctreeCell*>* skipQuadtree, 
  double* boxCoords, double boxHalfSideLength, double boxHalfSideLengthError);
  

int
exactRangeQuery2Dim(::std::map<int, OctreeCell*>* skipQuadtree, double* coords, 
  double radius, double radiusError);


void
unmarkAll(::std::map<int, OctreeCell*>* skipQuadtree);

void
findAllFlocks(::std::map<int, OctreeCell*>* skipQuadtree, double radius, 
  double radiusError);
  
#endif // SKIPTREEFUNCTIONS_HEADER
