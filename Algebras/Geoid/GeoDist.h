/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, 
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

class Geoid;

namespace geodist{
  
  /*
   Computes the distance between two points on a geoid.
  */
  double HaversineInM(const double lat1, const double long1, 
                      const double lat2, const double long2, 
                      const Geoid* geoid);

  /*
    Computes the distance between an segment and a point
    on a geoid.
  */  
  double getDist( const double sx1, const double sy1,
                  const double sx2, const double sy2,
                  const double x, const double y,
                  const Geoid* geoid);

}

