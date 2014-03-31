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


*/

#include <PrecTools.h>
#include <vector>
#include <PrecisePoint.h>


namespace precisetools{


  bool isClockwise(vector<MPrecPoint>& cycle){
    assert(cycle.size()>3);
    int smallestIndex = 0;
    for(size_t i=1;i<cycle.size();i++){
      if(cycle[i] < cycle[smallestIndex]){
         smallestIndex = 0;
      }
    }
    int previous = (smallestIndex + (cycle.size()-1)) % cycle.size();
    int next = (smallestIndex + 1) % cycle.size();
    MPrecPoint p1 = cycle[previous];
    MPrecPoint p2 = cycle[smallestIndex];
    MPrecPoint p3 = cycle[next];

    if(p1.getX()==p2.getX()){ // found vertical line
      return p2.getY() > p1.getY(); // up
    }
    if(p2.getX() == p3.getX()){
       return p2.getY() < p3.getY();
    }
    // no vertical line, compare slopes
    MPrecCoordinate s12  = (p1.getY() - p2.getY()) / (p1.getX() - p2.getX());
    MPrecCoordinate s32  = (p3.getY() - p2.getY()) / (p3.getX() - p2.getX());
    return s12 < s32;
  }



} // end of namespace precisetools

