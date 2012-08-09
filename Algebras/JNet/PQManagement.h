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

1. Includes

*/

#ifndef PQMANAGEMENT_H
#define PQMANAGEMENT_H

#include <ostream>
#include "../../Tools/Flob/DbArray.h"
#include "JTreeElement.h"

/*
1 class ~PQManagement~

Manages the priority queue and the visited sections within shortest path search
resp. network distance computation. It consists of two DbArrays to avoid
main memory limitations. The first DbArray contains the visited Junctions and
the second DbArray the priority queue. The two DbArrays are connected by the
pqindex value stored in the first one for each pqentry. If the junction is not
longer in the priority queue this indexentry is set to -1.

*/

class PQManagement
{

/*
1.1 public Methods of class ~PQManagement~

*/
public:

/*
1.1.1 Constructors and Deconstructors

*/

  explicit PQManagement(const int a);
  ~PQManagement();

/*
1.1.1 Insert

Checks first if the end junction of the new entry has already been used.
If it not has been used before it is inserted in visited junction list and
priority queue.
If it has been used before we check if it is still in the priority queue.
If it is still in the priority queue we check if the new distance is smaller
than the old one.
If the new distance is smaller than the old one we update the entry in the
priority queue and in the visited junction list.
Otherwise we ignore the new values.


*/
  void Insert(const JPQEntry& e);

/*
1.1.1 GetAndDeleteMin

Returns the priority queue entry with the smallest priority value and removes
it from the priority queue and updates the corresponding entry in the visited
junctions list.

*/

 JPQEntry* GetAndDeleteMin();

/*
1.1.1 IsEmpty

Returns true if pq is empty false elsewhere.

*/

bool IsEmpty() const;

/*
1.1.1 Destroy

Marks the DbArrays to be removable from Harddisk on delete.

*/

void Destroy();

/*
1.1.1 Print

*/

ostream& Print(ostream& os) const;

/*
1.1 Private definitions of class ~PQManagement~

*/
private:

/*
1.1.1 Attributes

*/

  DbArray<VisitedJunctionTreeElement> visited;
  DbArray<JPQEntryTreeElement> pq;
  int firstFreeVisited, firstFreePQ;

/*
1.1.1 IsEmpty

1.1.1.1 IsEmptyPQ
Returns true if the priority queue is empty, false elsewhere.

*/

bool IsEmptyPQ() const;

/*
1.1.1.1 IsEmptyVisited
Returns true if visited is empty, false elsewhere.

*/

bool IsEmptyVisited() const;

/*
1.1.1 FindVisited

Returns true if the end node of e has already been visited, false elsewhere.
At the end visitedPos contains the index of e in the DbArray if it has been
found or the position of the node which will be the father node of e. In the
latter case visitedLeft indicates if the new node will be the left or right son.
If visited is empty we return visitedPos -1 as special case indikator.

*/

bool FindVisited(const JPQEntry& e, int& visitedPos, bool& visitedLeft) const;

/*
1.1.1 GetVisited

Returns the value at position in visited

*/

VisitedJunctionTreeElement GetVisited(const int pos) const;


/*
1.1.1 GetPQ

Returns the value at position in pq

*/

JPQEntryTreeElement GetPQ(const int pos) const;

/*
1.1.1 RemoveFromPQ

Removes the Entry at position from the Priority Queue

*/

void RemoveFromPQ(JPQEntryTreeElement& elem, int pos);

/*
1.1.1 InsertIntoPQ

Inserts the value into the priority queue and returns the index of the new
position.

*/

int InsertPQ(const JPQEntry& entry);

/*
1.1.1 InsertVisited

Inserts the value into the list of visited junctions.

*/

void InsertVisited(const VisitedJunction& elem, const int visitedPos,
                   const bool visitedLeft);

/*
1.1.1 GetFather

Returns the JPQEntryTreeElement which points to pos. fatherPos returns the
index of this father node element and fatherLeft is true if pos is the left
son false if it is the right son.

*/

JPQEntryTreeElement GetFather(const JPQEntry& elem, const int pos,
                              int& fatherPos, bool& fatherLeft) const;

/*
1.1.1 PQGetMin

Returns the element from pq with the minimal priority value. The position of
the element in qp is given within pos.

*/

void PQGetMin(int& pos, JPQEntryTreeElement& elem) const;

/*
1.1.1 PQGetMax

Returns the element from pq with the maximal priority value. The position of
the element in qp is given within pos.

*/

void PQGetMax(int& pos, JPQEntryTreeElement& elem) const;

/*
1.1.1 Swap

Exchanges the element values from elem and minElem which are allocated at pos
resp. minPos. After that element value from elem is stored at minPos and
the element value from minElem at pos.

*/

void SwapPQ(JPQEntryTreeElement& elem, const int pos,
            JPQEntryTreeElement& minElem, const int minPos);
};

/*
1 Overload output operator

*/

ostream& operator<<(ostream& os, const PQManagement& dir);

#endif // PQMANAGEMENT_H
