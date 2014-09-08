/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
----

//[_] [\_]
//characters   [1]  verbatim:  [$]  [$]
//characters   [2]  formula:  [$]  [$]
//characters   [3]  capital:  [\textsc{] [}]
//characters   [4]  teletype:  [\texttt{] [}]

1 Header file "Optics.h"[4]

March-October 2014, Marius Haug

1.1 Overview

This file contains the "Optics"[4] class declaration.

1.2 Includes

*/
#include "Algebra.h"
#include "MTree.h"

namespace clusteropticsalg
{
/*
1.3 Declarations of the class ~Optics~

*/
 class Optics;

 class Optics
 {
/*
1.3.1 Public members

*/
  public:
/*
Static constants to access elements of the tuples and value definition of
distance undefined.

*/
   const static int PNT = 0;
   const static int COR = 1;
   const static int REA = 2;
   const static int PRC = 3;
   const static int ORD = 4;
   const static double UNDEFINED = -1.0;
/*
Constructor

*/
   Optics();
/*
Sets the "MTree"[4] for an optimized range query etc.

*/
   void setQueryTree(mtreeAlgebra::MTree* queryTree) { mtree = queryTree; };
/*
Starts the ordering by the optics algorithm.

*/
   void order(TupleBuffer* objs, int eps, unsigned int minPts
    ,TupleBuffer* order);

/*
1.3.2 Private members

*/
  private:
/*
Defined "MTree"[4] for the range query etc.

*/
   mtreeAlgebra::MTree* mtree;
/*
Iterates through the given objects and controlls the next starting object to
find the cluster order.

*/
   void expandClusterOrder(TupleBuffer* objs, Tuple* obj, int eps
    ,unsigned int minPts, TupleBuffer* order);
/*
Find the neigbors with respect to "obj"[4] and "eps"[4].

*/
   std::list<Tuple*> getNeighbors(TupleBuffer* objs, Tuple* obj, int eps);
/*
Sets the core distance to "obj"[4] with respect to "objs"[4] (neighbors), 
"eps"[4] and "minPts"[4].

*/
   void setCoreDistance(std::list<Tuple*>& objs, Tuple* obj
    ,int eps, unsigned int minPts);
/*
Updates "orderedSeeds"[4], elements of "neighbors"[4] will be inserted or moved
up within "orderedSeeds"[4] with respect to their reachable distance to 
"center"[4].

*/
   void update(std::list<Tuple*>& neighbors, Tuple* center
    ,std::list<Tuple*>& orderedSeeds);
/*
Inserts "obj"[4] into "orderedSeeds"[4].

*/
   void insert(std::list<Tuple*>& orderedSeeds, Tuple* obj);
/*
Moves "obj"[4] up within "orderedSeeds"[4] with respect to the reachable
distances of the elements of "orderedSeeds"[4].

*/
   void decrease(std::list<Tuple*>& orderedSeeds, Tuple* obj);
/*
Returns the reachable distance from "neighbor"[4] to "obj"[4].

*/
   double getReachableDist(Tuple* obj, Tuple* neighbor);
 };
}
