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
//[_] [\_]

[1] Header File of the MapMatching Algebra

January-April, 2012. Matthias Roth

[TOC]

1 Overview

This header file contains the convenience-class ~NetworkRoute~

2 Defines and includes

*/
#ifndef __NETWORK_ROUTE_H__
#define __NETWORK_ROUTE_H__

class Tuple;
class SimpleLine;

#include <stdio.h>


namespace mapmatch {

/*
3 class NetworkRoute
  accessing the attributes of a Network-Route

*/
class NetworkRoute
{
public:
    NetworkRoute(Tuple* pTupleRoute, bool bIncReference = true);
    NetworkRoute(const NetworkRoute& rNetworkRoute);
    ~NetworkRoute();

    const NetworkRoute& operator=(const NetworkRoute& rNetworkRoute);

    bool IsDefined(void) const {return m_pTupleRoute != NULL;}

    int GetRouteID(void) const;

    double GetRouteLength(void) const;

    const SimpleLine* GetCurve(void) const;

    bool GetDual(void) const;

    bool GetStartsSmaller(void) const;

private:

    Tuple* m_pTupleRoute;
};


} // end of namespace mapmatch

#endif /* __NETWORK_ROUTE_H__ */
