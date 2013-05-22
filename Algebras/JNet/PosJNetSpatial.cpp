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

2013 May, Simone Jandt

1 Defines and Includes

*/

#include "PosJNetSpatial.h"

using namespace jnetwork;

/*
1 Implementation of class PosJNetSpatial

1.1. Constructors and Deconstructors

*/

PosJNetSpatial::PosJNetSpatial()
{}

PosJNetSpatial::PosJNetSpatial(const RouteLocation rloc,
                               const Point* p, const int sectId,
                               const int jid1, const double dist1,
                               const int jid2, const double distjid2) :
  netpos(rloc), spatialpos(*p), sid(sectId), startjid(jid1),
  diststartjid(dist1), endjid(jid2), distendjid(distjid2)
{}


PosJNetSpatial::PosJNetSpatial(const PosJNetSpatial& other) :
  netpos(other.GetNetworkPos()), spatialpos(other.GetSpatialPos()),
  sid(other.GetSectionId()), startjid(other.GetStartJID()),
  diststartjid(other.GetDistFromStartJunction()),
  endjid(other.GetEndJID()), distendjid(other.GetDistFromEndJunction())
{}

PosJNetSpatial::~PosJNetSpatial()
{}

/*
1.1. Getter

*/

  RouteLocation PosJNetSpatial::GetNetworkPos() const
  {
    return netpos;
  }

  Point PosJNetSpatial::GetSpatialPos()const
  {
    return spatialpos;
  }

  int PosJNetSpatial::GetSectionId() const
  {
    return sid;
  }

  int PosJNetSpatial::GetStartJID() const
  {
    return startjid;
  }

  double PosJNetSpatial::GetDistFromStartJunction() const
  {
    return diststartjid;
  }

  int PosJNetSpatial::GetEndJID() const
  {
    return endjid;
  }

  double PosJNetSpatial::GetDistFromEndJunction() const
  {
    return distendjid;
  }

/*
1.1 Standardoperators

*/

PosJNetSpatial& PosJNetSpatial::operator=(const PosJNetSpatial& other)
{
  netpos = other.GetNetworkPos();
  spatialpos = other.GetSpatialPos();
  sid = other.GetSectionId();
  startjid = other.GetStartJID();
  diststartjid = other.GetDistFromStartJunction();
  endjid = other.GetEndJID();
  distendjid = other.GetDistFromEndJunction();
  return *this;
}

ostream& PosJNetSpatial::Print(ostream& os) const
{
  os << "netpos: " << netpos
     << ", spatialpos: " << spatialpos
     << ", sid: " << sid
     << ", startjid: " << startjid
     << ", diststartjid: " << diststartjid
     << ", endjid: " << endjid
     << ", distendjid: " << distendjid
     << endl;
  return os;
}


ostream& operator<<(ostream& os, const jnetwork::PosJNetSpatial& elem)
{
  return elem.Print(os);
}
