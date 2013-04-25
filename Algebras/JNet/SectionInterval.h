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

April 2013, Simone Jandt

1 Defines and Includes

*/


#ifndef SECTIONINTERVAL_H
#define SECTIONINTERVAL_H

#include <ostream>
#include "JRouteInterval.h"

/*
1 class SectionInterval

The class Section Interval is used for the estimation of Bounding JPoints of
JLine values. It describes the single sections (partially) covered by an
JRouteInterval. By the section Identifier, the part of the section covered as
~jrint~ and two boolean values telling if the start respectively endpoint of
the section is covered or not.

*/

namespace jnetwork{
class SectionInterval
{

public:

/*
1.1 Constructors and Deconstructor

*/

    SectionInterval();
    SectionInterval(const int sid, const JRouteInterval& inRint,
                    const bool start, bool end);
    SectionInterval(const SectionInterval& other);
    ~SectionInterval();

/*
1.1 Getter and Setter

*/

  int GetSid() const;
  JRouteInterval GetRouteInterval() const;
  bool GetStartIncluded() const;
  bool GetEndIncluded() const;

/*
1.1. Standard operators

*/

  SectionInterval& operator=(const SectionInterval& other);
  ostream& Print(ostream& os) const;

private:
  int sectId;
  JRouteInterval jrint;
  bool includeStart, includeEnd;

};

} //end namespace

/*
1 Overwrite output operator

*/

using namespace jnetwork;
ostream& operator<<(ostream& os, const SectionInterval& si);


#endif // SECTIONINTERVAL_H
