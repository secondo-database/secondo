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

2012, May Simone Jandt

1 Defines and includes

*/

#ifndef JRITREE_H
#define JRITREE_H

#include "../../Tools/Flob/DbArray.h"
#include "JRITreeElement.h"
#include "JRouteInterval.h"
#include "JLine.h"

/*
1 class JRITree

The JRITree enables the user to sort and compress sets of JRouteIntervals as
far as possible. It uses an tree embedded in an DbArray to avoid problems
with main memory limitation. For big amounts of data. Therefore the
JRouteIntervals are embedded in JRITReeElements which extend the JRouteInterval
by two pointers to the indexes of their right and left sons.

*/

class JRITree {

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and Deconstructors

*/

JRITree();
explicit JRITree(const int a);
JRITree(const DbArray<JRouteInterval>* inArray);
~JRITree();


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

Inserts a ~JRouteInterval~ in the ~JRITree~ checking if there are already
~JRouteIntervals~ which overlap or are adjacent to the current ~JRouteInterval~.

*/

void Insert(const JRouteInterval ri, int pos = 0);

/*
1.1.1 TreeToDbArray

Stores the ~JRouteInterval~s of the ~JRITree~ sorted by their route ids and
start positions into ~DbArray~ outArray.

*/

void TreeToDbArray(DbArray<JRouteInterval>* outArray, int fromPos = 0);

/*
1.1 private declaration part

*/

private:

/*
1.1.1 Private Attributes of JRITree

*/

  DbArray<JRITreeElement> tree;  //Tree embedded in DbArray to avoid problems
                                 //with main memory limitations
  int firstFree;

/*
1.1.1 ~Insert~

*/

void Insert (const bool left, const int pos, JRITreeElement& testRI,
             const JRouteInterval& test);

/*
1.1.1 ~CheckTree~

Checks if in the ~JRITree~ exist son intervals which are covered by the
previos changed interval of the father.

*/

double CheckTree(JRITreeElement& father, const int posfather,
                 JRITreeElement& testRI, const int postest,
                 const bool bleft);

/*
1.1.1 ~IsEmpty~

Returns true if the tree is empty, false otherwise.

*/

bool IsEmpty() const;
};

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRITree& in);

#endif //JRITREE_H