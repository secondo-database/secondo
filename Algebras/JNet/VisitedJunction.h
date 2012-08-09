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

#ifndef VISITEDJUNCTION_H
#define VISITEDJUNCTION_H

#include <ostream>
#include "JPQEntry.h"

using namespace std;

/*
1 class VisitedJunction

Extends JPQEntry by an integer indexing the position of the Entry in the
priority Queue.

*/

class VisitedJunction : public JPQEntry{

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and deconstructor

*/

VisitedJunction();
VisitedJunction(const VisitedJunction& other);
VisitedJunction(const JPQEntry& entry, const int index);
~VisitedJunction();

/*
1.1.1 Getter and Setter

*/


int GetIndexPQ() const;
void SetIndexPQ(const int id);

/*
1.1.1 Some standard operations

*/

VisitedJunction& operator= (const VisitedJunction& other);
ostream& Print(ostream& os) const;

int Compare(const VisitedJunction& other) const;
int CompareEndJID(const int id) const;

/*
1.1 private declaration part

*/

private:

  int pqIndex; //index of endJID in priority queue

};

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const VisitedJunction elem);

#endif //VISITEDJUNCTION_H