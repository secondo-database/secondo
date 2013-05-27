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

#include "InterSP.h"

using namespace std;

using namespace jnetwork;

/*
1 Implementation of class InterSP

*/

  InterSP::InterSP()
  {}

  InterSP::InterSP(const InterSP& other) :
    origjid(other.GetOrigStartPathJID()), curjid(other.GetCurStartPathJID()),
    nextjid(other.GetNextJID()), nextsid(other.GetNextSID()),
    distJID(other.GetDistFromOrigStartPath()),
    distStartOrigJID(other.GetDistFromStartPointToOrigJID())
  {}

  InterSP::InterSP(const int origStartPathJID, const int curStartPathJID,
            const double distOfOriginStartPath, const int nextJIDOnPath,
            const int nextSIDOnPath, const double distStartPointOrigJID) :
    origjid(origStartPathJID), curjid(curStartPathJID), nextjid(nextJIDOnPath),
    nextsid(nextSIDOnPath), distJID(distOfOriginStartPath),
    distStartOrigJID(distStartPointOrigJID)
  {}

  InterSP::~InterSP()
  {}

/*
1.1.1 Getter and Setter

*/

int InterSP::GetOrigStartPathJID() const
{
  return origjid;
}

int InterSP::GetCurStartPathJID() const
{
  return curjid;
}

int InterSP::GetNextJID() const
{
  return nextjid;
}

int InterSP::GetNextSID() const
{
  return nextsid;
}

double InterSP::GetDistFromOrigStartPath() const
{
  return distJID;
}

double InterSP::GetDistFromStartPointToOrigJID() const
{
  return distStartOrigJID;
}


void InterSP::SetOrigStartPathJID(const int id)
{
  origjid = id;
}

void InterSP::SetCurStartPathJID(const int id)
{
  curjid = id;
}

void InterSP::SetNextJID(const int id)
{
  nextjid = id;
}

void InterSP::SetNextSID(const int id)
{
  nextsid = id;
}

void InterSP::SetDistFromOrigStartPath(const double distance)
{
  distJID = distance;
}

void InterSP::SetDistFromStartPointToOrigJID(const double distance)
{
  distStartOrigJID = distance;
}

/*
1.1.1 Some standard functions

*/

  int InterSP::Compare(const InterSP& other) const
  {
    if (origjid < other.GetOrigStartPathJID()) return -1;
    if (origjid > other.GetOrigStartPathJID()) return 1;
    if (distJID < other.GetDistFromOrigStartPath()) return -1;
    if (distJID > other.GetDistFromOrigStartPath()) return 1;
    return 0;
  }

  InterSP& InterSP::operator=(const InterSP& other)
  {
    origjid = other.GetOrigStartPathJID();
    curjid = other.GetCurStartPathJID();
    nextjid = other.GetNextJID();
    nextsid = other.GetNextSID();
    distJID = other.GetDistFromOrigStartPath();
    distStartOrigJID = other.GetDistFromStartPointToOrigJID();
    return *this;
  }

  ostream& InterSP::Print(ostream& os) const
  {
    os << "origjid: " << origjid
       << ", curstart: " << curjid
       << ", nextjid: " << nextjid
       << ", viasid: " << nextsid
       << ", distFromStart: " << distJID
       << ", distStartPoint from origJunc: " << distStartOrigJID
       << endl;
    return os;
  }

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const InterSP& elem)
{
  elem.Print(os);
  return os;
}