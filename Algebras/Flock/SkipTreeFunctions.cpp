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

#ifndef SKIPTREEFUNCTIONS_SOURCE
#define SKIPTREEFUNCTIONS_SOURCE

#include "SkipTreeFunctions.h"
#include <map>
#include <iostream>
#include <assert.h>

//#define SHOW_DEBUG 1


/*
freeing function for a whole skipquadtree represented by a map of
map<int, OctreeCell*>* skipQuadtree
with the lowest index beeing the complete tree.

*/

void
deleteSkipQuadtree(::std::map<int, OctreeCell*>* skipQuadtree){

#ifdef SHOW_DEBUG
  ::std::cout << "deleting skip tree stucture" << '\n';
#endif

  ::std::map<int, OctreeCell*>::iterator mapIterator=skipQuadtree->begin();
  for (; mapIterator!=skipQuadtree->end(); mapIterator++){
#ifdef SHOW_DEBUG
    printf("deleteing tree no. %d\n", mapIterator->first);
#endif
    if(mapIterator->second != 0) delete mapIterator->second;
  }
  delete skipQuadtree;
    
#ifdef SHOW_DEBUG
  printf("all tree data deleted.\n");
#endif
  return;
};


int
boxedRangeQueryCounting(::std::map<int, OctreeCell*>* skipQuadtree,
  double* boxCoords, double boxHalfSideLength, double boxHalfSideLengthError){
  int report=0;  
  assert(skipQuadtree->size() > 0);
  
  report = (*skipQuadtree)[0]->boxedRangeQueryCounting(boxCoords, 
    boxHalfSideLength, boxHalfSideLengthError);
  
  return report;
};

void
unmarkAll(::std::map<int, OctreeCell*>* skipQuadtree){
   (*skipQuadtree)[0]->unmarkReported();
   printf("unmarked all points.\n");
   return;
};


int
exactRangeQuery2Dim(::std::map<int, OctreeCell*>* skipQuadtree, 
  double* coords, double radius, double radiusError){
  int report=0;
  assert(skipQuadtree->size() > 0);

  report = (*skipQuadtree)[0]->exactRangeQuery2DimSteps(coords, radius, 
    radiusError);
      
  return report;
};

void
findAllFlocks(::std::map<int, OctreeCell*>* skipQuadtree, double radius, 
  double radiusError){
    
    
};

#endif // SKIPTREEFUNCTIONS_SOURCE
