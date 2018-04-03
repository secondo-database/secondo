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
#include "../../Tools/Flob/DBATree.h"
#include "JTreeElement.h"

namespace jnetwork{

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

  PQManagement();
  ~PQManagement();

/*
1.1.1 Insert

Inserts the JPQEntry in the priority queue if the end junction of the new entry
has not been used before, respectively if it is still in the priority queue and
the new distance is smaller than the old one.
Otherwise we ignore the new values.

*/
  void Insert(const JPQEntry& e);

/*
1.1.1 GetAndDeleteMin

Returns the priority queue entry with the smallest priority value and removes
it from the priority queue.

*/

 JPQEntry* GetAndDeleteMin();

/*
1.1.1 IsEmpty

Returns true if pq is empty false elsewhere.

*/

bool IsEmpty() const;

inline int Size() const{
  return sizePQ;
}


/*
1.1.1 InsertJunctionVisited

Inserts the given Junction as visited junction without inserting it into pqueue.
Used to mark junctions visited which are used in initialization phase of the
priority queue, and therefore do not become members of priority queue
themselves.

*/

void InsertJunctionAsVisited(const JPQEntry juncE);

/*
1.1.1 Destroy

Marks the DbArrays to be removable from harddisk on delete.

*/

void Destroy();

/*
1.1.1 Print

*/

std::ostream& Print(std::ostream& os) const;

/*
1.1 Private definitions of class ~PQManagement~

*/
private:

/*
1.1.1 Attributes

*/
  dbatree::DBATree<VisitedJunction, VisitedJunctionComparator> visited;
  DbArray<JPQEntry> pq;
  int sizePQ;

  void blowUp(int index);
  void sink(int index);


};

} // end of namespace jnetwork

/*
1 Overload output operator

*/

std::ostream& operator<<(std::ostream& os, const jnetwork::PQManagement& dir);

#endif // PQMANAGEMENT_H
