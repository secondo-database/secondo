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

2013, May Simone Jandt

1 Includes and Defines

*/

#ifndef POSJNETSPATIAL_H
#define POSJNETSPATIAL_H

#include "RouteLocation.h"
#include "SpatialAlgebra.h"

namespace jnetwork{
/*
1 Class ~PosJNetSpatial~

Helper class for shortest path computation. Connects route locations with
their spatial postions, section ids, the junction ids limiting the section and
the distance to the junction.

*/

class PosJNetSpatial
{

public:

/*
1.1 Constructors and Deconstructors

*/

    PosJNetSpatial();
    PosJNetSpatial(const RouteLocation rloc, const Point* p, const int sectId,
                   const int jid1, const double dist1, const int jid2,
                   const double distjid2);
    PosJNetSpatial(const PosJNetSpatial& other);

    virtual ~PosJNetSpatial();

/*
1.1. Getter

*/

  RouteLocation GetNetworkPos() const;
  Point GetSpatialPos()const;
  int GetSectionId() const;
  int GetStartJID() const;
  double GetDistFromStartJunction() const;
  int GetEndJID() const;
  double GetDistFromEndJunction() const;

/*
1.1 Standardoperators

*/
    PosJNetSpatial& operator=(const PosJNetSpatial& other);
    ostream& Print(ostream& os) const;

private:
  RouteLocation netpos;
  Point spatialpos;
  int sid;
  int startjid;
  double diststartjid;
  int endjid;
  double distendjid;

};

}//end namespace jnetwork

using namespace jnetwork;
ostream& operator<<(ostream& os, const PosJNetSpatial& elem);

#endif // POSJNETSPATIAL_H
