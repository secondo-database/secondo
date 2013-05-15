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

2012, July Simone Jandt

1 Defines and includes

*/

#ifndef JPQENTRY_H
#define JPQENTRY_H

#include <ostream>
#include "JRouteInterval.h"

using namespace std;

namespace jnetwork {
/*
1 class JPQEntry

Stores priority queue informations for shortest path resp. network distance
computation within JNetAlgebra.

It consist of three junction identifiers, one for the
junction at the start of the shortest path, one for the junction currently
reached and one for the junction before current junction, two double values
one for the priority value and one for the network distance of the current
junction from the start of the shortest path, the section identifier of the
section between current junction and before junction, and the route interval
describing this section.

*/

class JPQEntry{

/*
1.1 Public Declaration Part

*/
  public:

/*
1.1.1 Constructors and Deconstructor

*/

    JPQEntry();
    JPQEntry(const JPQEntry& other);
    JPQEntry(const Direction dir, const int sect,
             const int startPathJunc, const int startPathNextJID,
             const int startPathSID, const int startPartJunc,
             const int endPartJunc, const double dist, const double prio);
    ~JPQEntry();

/*
1.1.1 Getter and Setter

*/

int GetStartPathJID() const;
int GetStartNextJID() const;
int GetStartNextSID() const;
int GetStartPartJID() const;
int GetEndPartJID() const;
double GetPriority() const;
double GetDistFromStart() const;
int GetSectionId() const;
Direction GetDirection() const;

void SetStartPathJID(const int id);
void SetStartNextJID(const int id);
void SetStartNextSID(const int id);
void SetStartPartJID(const int id);
void SetEndPartJID(const int id);
void SetPriority(const double prio);
void SetDistFromStart(const double dist);
void SetSectionId(const int id);
void SetDirection(const Direction& dir);

/*
1.1.1 Some standard functions

*/

  int Compare(const JPQEntry& other) const;
  JPQEntry& operator=(const JPQEntry& other);
  ostream& Print(ostream& os) const;

/*
1.1 Private Declaration Part

*/

  private:

    Direction movDir;
    int sid, startPathJID, startNextJID, startNextSID, startPartJID, endPartJID;
    double distFromStart, prioval;

};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;

ostream& operator<< (ostream& os, const jnetwork::JPQEntry elem);



#endif //JPQUENTRY_H