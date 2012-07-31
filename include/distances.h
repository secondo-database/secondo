
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

*/


/*
1 Distance Computations

This file supports the main memomry based m-tree implemented
in file MMMTree.h. If you want to store variables of type ~X~
into an mm-m-tree, there must exist  a distance function.
This function can be defined within this file.


*/


#ifndef DISTANCES_H
#define DISTANCES_H

 double  distance(const int i, const int j){
    int diff = i-j;
    return diff<0?-diff:diff;
 }

 double distance(const double i, const double j){
    double diff = i-j;
    return diff<0?-diff:diff;

 } 

#endif

