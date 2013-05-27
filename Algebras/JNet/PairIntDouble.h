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

#ifndef PairIntDouble_H
#define PairIntDouble_H

#include <ostream>

using namespace std;

namespace jnetwork {

/*
1 class PairIntDouble

Used in shortestpath tree computation. Stores the pairs of junction identifier
and distance from source jpoint.

*/

class PairIntDouble{

/*
1.1 Public Declaration Part

*/
  public:

/*
1.1.1 Constructors and Deconstructor

*/

    PairIntDouble();
    PairIntDouble(const PairIntDouble& other);
    PairIntDouble(const int juncid, const double distFromSource);
    ~PairIntDouble();

/*
1.1.1 Getter and Setter

*/

int GetJunctionId() const;
double GetDistance() const;

void SetJunctionId(const int id);
void SetDistance(const double distance);

/*
1.1.1 Some standard functions

*/

  int Compare(const PairIntDouble& other) const;
  PairIntDouble& operator=(const PairIntDouble& other);
  ostream& Print(ostream& os) const;

/*
1.1 Private Declaration Part

*/

  private:

    int jid;
    double dist;

};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;

ostream& operator<< (ostream& os, const jnetwork::PairIntDouble elem);



#endif //PairIntDouble_H