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

1 Includes

*/

#include "VisitedJunction.h"

/*
1 Implementation of class VisitedJunction

1.1 Constructors and Deconstructors

*/

VisitedJunction::VisitedJunction(): JPQEntry() {};


VisitedJunction::VisitedJunction(const VisitedJunction& other) :
  JPQEntry(other), pqIndex(other.GetIndexPQ())
{}

VisitedJunction::VisitedJunction(const JPQEntry& entry, const int index):
  JPQEntry(entry), pqIndex(index)
{}

VisitedJunction::~VisitedJunction()
{}

/*
1.1.1 Getter and Setter

*/

int VisitedJunction::GetIndexPQ() const
{
  return pqIndex;
}


void VisitedJunction::SetIndexPQ(const int id)
{
  pqIndex = id;
}

/*
1.1.1 Some standard operations

*/

VisitedJunction& VisitedJunction::operator= (const VisitedJunction& other)
{
  JPQEntry::operator=(other);
  pqIndex = other.GetIndexPQ();
  return * this;
}

ostream& VisitedJunction::Print(ostream& os)const
{
  JPQEntry::Print(os);
  os << ", pqindex: " << pqIndex << endl;
  return os;
}

int VisitedJunction::Compare(const VisitedJunction& other) const
{
  return JPQEntry::Compare(other);
}

int VisitedJunction::CompareEndJID(const int id) const
{
  if (GetEndPartJID() < id) return -1;
  if (GetEndPartJID() > id) return 1;
  return 0;
}


/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const VisitedJunction elem)
{
  elem.Print(os);
  return os;
}
