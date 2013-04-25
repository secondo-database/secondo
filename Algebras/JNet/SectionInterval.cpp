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

2013, April Simone Jandt

1 Defines and Includes

*/

#include "SectionInterval.h"

using namespace jnetwork;

/*
1 Implementation of class SectionInterval

1.1. Constructor and Deconstructor

*/

SectionInterval::SectionInterval()
{}

SectionInterval::SectionInterval(const int sid,
                                 const JRouteInterval& inRint,
                                 const bool start,
                                 bool end) :
  sectId(sid), jrint(inRint), includeStart(start), includeEnd(end)
{}

SectionInterval::SectionInterval(const SectionInterval& other) :
  sectId(other.GetSid()), jrint(other.GetRouteInterval()),
  includeStart(other.GetStartIncluded()), includeEnd(other.GetEndIncluded())
{}

SectionInterval::~SectionInterval()
{}

/*
1.1 Getter and Setter

*/

int SectionInterval::GetSid() const
{
  return sectId;
}

JRouteInterval SectionInterval::GetRouteInterval() const
{
  return jrint;
}

bool SectionInterval::GetStartIncluded() const
{
  return includeStart;
}

bool SectionInterval::GetEndIncluded() const
{
  return includeEnd;
}

SectionInterval& SectionInterval::operator=(const SectionInterval& other)
{
  sectId = other.GetSid();
  jrint= other.GetRouteInterval();
  includeStart = other.GetStartIncluded();
  includeEnd = other.GetEndIncluded();
  return *this;
}

/*
1.1 Output

*/
ostream& SectionInterval::Print(ostream& os) const
{
  os << boolalpha
     << "SectId: " << sectId
     << ", RouteInterval: " << jrint
     << ", startincluded: " << includeStart
     << ", endincluded: " << includeEnd
     << endl;
  return os;
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const SectionInterval& si)
{
  si.Print(os);
  return os;
}


