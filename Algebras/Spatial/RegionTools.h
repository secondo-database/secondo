

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


1 RegionTools


This file contains functions handling with regions.


1.1 Function ~buildRegion~

This function creates a region from a vector of cycles.

The cycles must be closed. This means, the first and the
last point must be the same.

The outer cycle of a face must be given in clockwise order.
Inner cycles (holes) must be given in counter-clockwise order.

You can ensure this using code like:

---
  if(  isFace ){
     if(!getDir(cycle)){
         reverseCycle(cycle);
     }
  } else {
       if(getDir(cycle) ){
          reverseCycle(cycle);
       }
  }
---



*/


Region* buildRegion(vector< vector<Point> >& cycles);
Region* buildRegion2(vector< vector<Point> >& cycles);


/*
1.2 Function ~getDir~

This function returns the direction of a cycle as a boolean value.
It returns true, if the given cycle is in clockwise order. If the
cycle is directed counter clockwise, the result will be false.

*/

bool getDir(const vector<Point>& vp);



/*
1.3 Function reverse cycle

This function changes the direction of a cycle.

*/
void reverseCycle(vector<Point>& cycle);





