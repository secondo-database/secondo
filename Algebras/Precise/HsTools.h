/*
----
This file is part of SECONDO.

Copyright (C) 2014,
Faculty of Mathematics and Computer Science,
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

This file provides auxiliary function dealing with precise halfsegments.

*/


#ifndef HSTOOLS_H
#define HSTOOLS_H

#include <vector>
#include "PreciseHalfSegment.h"

namespace hstools{

/*
~isSorted~

Checks whether ~v~ is sorted according to the halfsegment order.

*/
  bool isSorted(const vector<MPrecHalfSegment>& v);

/*
~sort~

Sorts ~v~ using the halfsegment order.

*/
  void sort(vector<MPrecHalfSegment>& v);


/*
~isLogicalSorted~

Checks whether ~v~ is sorted using the logical order (faceno, cycleno, edgeno) 

*/
   bool isLogicalSorted(const vector<MPrecHalfSegment>& v);


/*
~sortLogical~

Sorts ~v~ according to the logical order of halfsegments 
(faceno, cycleno, edgeno)

*/

  void sortLogical(vector<MPrecHalfSegment>& v);


/*
~setCoverage~

Sets the coverage number of a set of halfsegments. This number is used 
in the accerelated computation of the point in region algorithm. The argument 
v must be sorted according to the halfsegment order.

*/
   void setCoverage(vector<MPrecHalfSegment>& v);


/*
~setPartnerNumbers~

This function sets the partner number of the contained halfsegments. 
Partners must have the same edge number. Non-partners must have different
edge numbers. The edges must be numbered from 0 to n-1 where n is the 
number of contained segments ( v.size()/2).

*/

   bool setPartnerNumbers(vector<MPrecHalfSegment>& v);


/*
~checkRealm~

This function checks whether the segments contained in v are realminized. 
This means, two different segments in ~v~ have no common point except end 
points. The halfsegments in ~v~ have to be sorted in halfsegment order.

*/
   bool checkRealm(const vector<MPrecHalfSegment>& v);


/*
~realminize~

The function computes a realminized version of ~v~ and stores it in ~res~.
~v~ has to be sorted according to the halfsegment order.

*/

   void realminize(const vector<MPrecHalfSegment>& v, 
                   vector<MPrecHalfSegment>& res);


/*
~checkCycles~

This funtion checks whether ~v~ consists of cycles only, i.e. whether each 
dominating point is reached by an even number of halfsegments. ~v~ has to 
be sorted according to the halfsegment order.

*/
   bool checkCycles(const vector<MPrecHalfSegment>& v);


/*
~removeConnections~

This function will copy all halfsegments of ~v~ belonging to cycles into ~res~.
v has to be sorted in halfsegment order.

*/
   void removeConnections(const vector<MPrecHalfSegment>& v, 
                          vector<MPrecHalfSegment>& res);


/*
~setInsideAbove~

This function will compute and set the insideAbove flag for each 
halfsegment in ~v~. 
~v~ has to be sorted in halfsegment order.

*/

  void setInsideAbove(vector<MPrecHalfSegment>& v);


/*
~computeCycles~

This function computes the faceno, cycleno, edgeno for each halfsegment in ~v~.
The halfsegments in v has to be sorted in halfsegment order , has to be 
realminized, and has to build only cycles.

*/

  void computeCycles(vector<MPrecHalfSegment>& v);



} // end of namespace hstools

#endif


