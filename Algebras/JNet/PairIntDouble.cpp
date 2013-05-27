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

#include "PairIntDouble.h"

using namespace std;
using namespace jnetwork;

/*
1 Implementation of Class PairIntDouble

1.1 Constructors and Deconstructor

*/

PairIntDouble::PairIntDouble()
{}

PairIntDouble::PairIntDouble(const PairIntDouble& other) :
  jid(other.GetJunctionId()), dist(other.GetDistance())
{}

PairIntDouble::PairIntDouble(const int juncid, const double distFromSource) :
  jid(juncid), dist(distFromSource)
{}

PairIntDouble::~PairIntDouble()
{}

/*
1.1 Getter and Setter

*/

int PairIntDouble::GetJunctionId() const
{
  return jid;
}

double PairIntDouble::GetDistance() const
{
  return dist;
}

void PairIntDouble::SetJunctionId(const int id)
{
  jid = id;
}
void PairIntDouble::SetDistance(const double distance)
{
  dist = distance;
}

/*
1.1 Some standard functions

*/

int PairIntDouble::Compare(const PairIntDouble& other) const
{
  if (jid < other.GetJunctionId()) return -1;
  if (jid > other.GetJunctionId()) return 1;
  if (dist < other.GetDistance()) return -1;
  if (dist > other.GetDistance()) return 1;
  return 0;
}

PairIntDouble& PairIntDouble::operator=(const PairIntDouble& other)
{
  jid = other.GetJunctionId();
  dist = other.GetDistance();
  return *this;
}

ostream& PairIntDouble::Print(ostream& os) const
{
  os << "junction id: " << jid
     << ", distance: " << dist
     << endl;
}


/*
1 Overwrite output operator

*/


ostream& operator<< (ostream& os, const jnetwork::PairIntDouble elem)
{
  elem.Print(os);
  return os;
}
