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

#include "JPQEntry.h"

using namespace std;
using namespace jnetwork;

/*
1 Implementation of class JPQEntry

1.1 Constructors and Deconstructor

*/

JPQEntry::JPQEntry()
{}

JPQEntry::JPQEntry(const JPQEntry& other) :
  movDir(other.GetDirection()), sid(other.GetSectionId()),
  startPathJID(other.GetStartPathJID()), startNextJID(other.GetStartNextJID()),
  startNextSID(other.GetStartNextSID()), startPartJID(other.GetStartPartJID()),
  endPartJID(other.GetEndPartJID()), distFromStart(other.GetDistFromStart()),
  prioval(other.GetPriority())
{}

JPQEntry::JPQEntry(const Direction dir, const int sect,
                   const int startPathJunc, const int startNextJunc,
                   const int startNextSect, const int startPartJunc,
                   const int endPartJunc, const double dist, const double prio)
: movDir(dir), sid(sect), startPathJID(startPathJunc),
  startNextJID(startNextJunc), startNextSID(startNextSect),
  startPartJID(startPartJunc), endPartJID(endPartJunc),  distFromStart(dist),
  prioval(prio)
{}

JPQEntry::~JPQEntry()
{}

/*
1.1 Getter and Setter

*/

int JPQEntry::GetStartPathJID() const
{
  return startPathJID;
}

int JPQEntry::GetStartNextJID() const
{
  return startNextJID;
}

int JPQEntry::GetStartNextSID() const
{
  return startNextSID;
}


int JPQEntry::GetStartPartJID() const
{
  return startPartJID;
}

int JPQEntry::GetEndPartJID() const
{
  return endPartJID;
}

double JPQEntry::GetPriority() const
{
  return prioval;
}

double JPQEntry::GetDistFromStart() const
{
  return distFromStart;
}

int JPQEntry::GetSectionId() const
{
  return sid;
}

Direction JPQEntry::GetDirection() const
{
  return movDir;
}


void JPQEntry::SetStartPathJID(const int id)
{
  startPartJID = id;
}

void JPQEntry::SetStartNextJID(const int id)
{
  startNextJID = id;
}

void JPQEntry::SetStartNextSID(const int id)
{
  startNextSID = id;
}

void JPQEntry::SetStartPartJID(const int id)
{
  startPartJID = id;
}

void JPQEntry::SetEndPartJID(const int id)
{
  endPartJID = id;
}

void JPQEntry::SetPriority(const double prio)
{
  assert(prio >= 0.0);
  prioval = prio;
}

void JPQEntry::SetDistFromStart(const double dist)
{
  distFromStart = dist;
}

void JPQEntry::SetSectionId(const int id)
{
  sid = id;
}

void JPQEntry::SetDirection(const Direction& dir)
{
  movDir = dir;
}

/*
1.1 Some standard functions

*/

int JPQEntry::Compare(const JPQEntry& other) const
{
  if (prioval < other.GetPriority()) return -1;
  if (prioval > other.GetPriority()) return 1;
  if (distFromStart < other.GetDistFromStart()) return -1;
  if (distFromStart > other.GetDistFromStart()) return 1;
  return movDir.Compare(other.GetDirection());
}

ostream& JPQEntry::Print(ostream& os) const
{
  os << "Path started at " << startPathJID
     << ", first junction on way: " << startNextJID
     << ", reached via section: " << startNextSID
     << ", reached junction: " << endPartJID
     << ", from junction: " << startPartJID
     << ", using section: " << sid
     << ", in direction: " << movDir
     << ", distFromStart: " << distFromStart
     << ", priority: " << prioval << endl;
  return os;
}

JPQEntry& JPQEntry::operator=(const JPQEntry& other)
{
  movDir = other.GetDirection();
  sid = other.GetSectionId();
  startPathJID = other.GetStartPathJID();
  startNextJID = other.GetStartNextJID();
  startNextSID = other.GetStartNextSID();
  startPartJID = other.GetStartPartJID();
  endPartJID = other.GetEndPartJID();
  distFromStart = other.GetDistFromStart();
  prioval = other.GetPriority();
  return *this;
}


/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JPQEntry elem)
{
  elem.Print(os);
  return os;
}
