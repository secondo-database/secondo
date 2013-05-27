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

1 Defines and includes

*/

#ifndef InterSP_H
#define InterSP_H

#include <ostream>

using namespace std;

namespace jnetwork {

/*
1 class InterSP

Stores netdistance informations for shortest path resp. network distance
computation within JNetAlgebra.

It consist of four integers and one double value. The four integers are
identifiers of the junction the original path starts, the current junction
idenitfier of the start of the path part the distance of between this both
junctions. The identfiers of the next junction and the next section on the path
followed.

*/

class InterSP{

/*
1.1 Public Declaration Part

*/
  public:

/*
1.1.1 Constructors and Deconstructor

*/

    InterSP();
    InterSP(const InterSP& other);
    InterSP(const int origStartPathJID, const int curStartPathJID,
            const double distOfOriginStartPath, const int nextJIDOnPath,
            const int nextSIDOnPath, const double distStartPointOrigJID);
    ~InterSP();

/*
1.1.1 Getter and Setter

*/

int GetOrigStartPathJID() const;
int GetCurStartPathJID() const;
int GetNextJID() const;
int GetNextSID() const;
double GetDistFromOrigStartPath() const;
double GetDistFromStartPointToOrigJID() const;

void SetOrigStartPathJID(const int id);
void SetCurStartPathJID(const int id);
void SetNextJID(const int id);
void SetNextSID(const int id);
void SetDistFromOrigStartPath(const double distance);
void SetDistFromStartPointToOrigJID(const double distance);

/*
1.1.1 Some standard functions

*/

  int Compare(const InterSP& other) const;
  InterSP& operator=(const InterSP& other);
  ostream& Print(ostream& os) const;

/*
1.1 Private Declaration Part

*/

  private:

    int origjid, curjid, nextjid, nextsid;
    double distJID, distStartOrigJID;

};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;

ostream& operator<< (ostream& os, const jnetwork::InterSP elem);



#endif //InterSP_H