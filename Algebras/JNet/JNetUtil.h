/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, November Simone Jandt

1 Defines and Includes

*/

#ifndef JNETUTIL_H
#define JNETUTIL_H

#include "../../Tools/Flob/DbArray.h"

/*
1 Forward Declarations

*/

class JRouteInterval;
class RouteLocation;

/*
1 class ~JNetUtil~

Provides helpful functions for code optimization.
Secondo does not allow datatypes which contain flobs to be an attribute of
another flob datatype. Therefore datatypes which logically would contain
corresponding flob datatypes contain the needed flob directly. Now several
operations would have to be implemented for each datatype although they only
work on the dbarray and do the same for the same other function parameters.
To avoid inconsistant implementation between the different datatypes for these
functions we provide this functions in an external JNetUtil class. Sucht that
the functions can be provided once and called by each of the classs needing
the provided functionality.

*/

class JNetUtil
{

/*
1.1 Public Declarations

*/
public:
  static int GetIndexOfJRouteIntervalForRLoc(
                                      const DbArray<JRouteInterval>& list,
                                      const RouteLocation& rloc,
                                      const int startindex,
                                      const int endindex);

  static int GetIndexOfJRouteIntervalForJRInt(
                                       const DbArray<JRouteInterval>& list,
                                       const JRouteInterval rint,
                                       const int startindex,
                                       const int endindex);

  static bool ArrayContainIntersections (const DbArray<JRouteInterval>& lhs,
                                         const DbArray<JRouteInterval>& rhs);

private:

};

#endif // JNETUTIL_H
