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

#ifndef JRLTREE_H
#define JRLTREE_H

#include "../../Tools/Flob/DbArray.h"
#include "JRLTreeElement.h"
#include "RouteLocation.h"

/*
1 class JRLTree

The JRLTree enables the user to sort sets of RouteLocations as
far as possible. It uses an tree embedded in an DbArray to avoid problems
with main memory limitation. For big amounts of data. Therefore the
RouteLocations are embedded in JRLTreeElements which extend the RouteLocation
by two pointers to the indexes of their right and left sons.

*/

class JRLTree {

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and Deconstructors

*/

JRLTree();
explicit JRLTree(const int a);
JRLTree(const DbArray<RouteLocation>* inArray);
~JRLTree();


/*
1.1.1 Destroy

Remove harddisk part of DbArray

*/

void Destroy();

/*
1.1.1 Print

*/

ostream& Print(ostream&os) const;

/*
1.1.1 Insert

Inserts a ~RouteLocation~ in the ~JRLTree~ checking if the ~RouteLocation~
already exists.

*/

void Insert(const RouteLocation ri, int pos = 0);

/*
1.1.1 TreeToDbArray

Stores the ~RouteLocation~s of the ~JRLTree~ sorted by their route ids and
start positions into ~DbArray~ outArray.

*/

void TreeToDbArray(DbArray<RouteLocation>* outArray, int fromPos = 0);


/*
1.1 private declaration part

*/

private:

/*
1.1.1 Private Attributes of JRLTree

*/

  DbArray<JRLTreeElement> tree;  //Tree embedded in DbArray to avoid problems
                                 //with main memory limitations
  int firstFree;

/*
1.1.1 ~Insert~

*/

void Insert (const bool left, const int pos, JRLTreeElement& testRL,
             const RouteLocation& test);

/*
1.1.1 ~IsEmpty~

Returns true if the tree is empty, false otherwise.

*/

bool IsEmpty() const;
};

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRLTree& in);

#endif //JRLTREE_H