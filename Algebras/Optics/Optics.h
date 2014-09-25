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
#include "MMRTree.h"
#include "DistfunReg.h"

namespace clusteropticsalg
{
/*
1.3 Declarations of the class ~Optics~

*/
 template <unsigned dim>
 class Optics;

 template <unsigned dim>
 class Optics
 {
/*
1.3.1 Public members

*/
  public:
/*
Constructor

*/
   Optics();
/*
Sets the "MTree"[4] for an optimized range query etc.

*/
   void initialize(mtreeAlgebra::MTree* queryTree, TupleBuffer* objsToOrd
    ,gta::DistfunInfo* df, int idxDistData, int idxCDist, int idxRDist
    ,int idxPrc);
/*
Sets the "RTree"[4] for an optimized range query etc.

*/
//   template <unsigned dim>
   void initialize(mmrtree::RtreeT<dim, TupleId>* queryTree
    ,TupleBuffer* objsToOrd, int idxDistData, int idxCDist, int idxRDist
    ,int idxPrc);
/*
Starts the ordering by the optics algorithm and saves the result in "order"[4].

*/
   void order(double pEps, unsigned int pMinPts, list<TupleId>* pOrder);

/*
1.3.2 Private members

*/
  private:
/*
Static constants for the value definition of distance undefined.

*/
   const static double UNDEFINED = -1.0;

/*
Members to access elements of the tuples.

*/
   const static int MODE_MTREE   = 1;
   const static int MODE_MMRTREE = 2;
/*
Members to access elements of the tuples.

*/
   int PNT;
   int COR;
   int REA;
   int PRC;
   int MODE;
   
   unsigned int minPts;
   
   double eps;
/*
Defined "MTree"[4] for the range query etc.

*/
   mtreeAlgebra::MTree* mtree;
/*
Defined "RTree"[4] for the range query etc.

*/
//   template <unsigned dim>
   mmrtree::RtreeT<dim, TupleId>* rtree;
/*
Defined "TupleBuffer"[4] which contains the data to order.

*/
   TupleBuffer* objs;
/*
Defined "TupleBuffer"[4] where the result will be saved.

*/   
   list<TupleId>* result;
/*
Defined "DistFunInfo"[4] to calculate distances.

*/
   gta::DistfunInfo* distFun;
/*
Iterates through the given objects and controlls the next starting object to
find the cluster order.

*/
   void expandClusterOrder(TupleId objId);
/*
Find the neigbors with respect to "obj"[4] and "eps"[4].

*/
   std::list<TupleId>* getNeighbors(TupleId objId);
/*
Sets the core distance to "obj"[4] with respect to "objs"[4] (neighbors), 
"eps"[4] and "minPts"[4].

*/
   void setCoreDistance(std::list<TupleId>* neighbors, TupleId objId);
/*
Returns the core distance to "obj"[4] with respect to "objs"[4] (neighbors), 
"eps"[4] and "minPts"[4].

*/
  	double getCoreDistanceR(std::list<TupleId>* neighbors, TupleId objId);
/*
Updates "orderedSeeds"[4], elements of "neighbors"[4] will be inserted or moved
up within "orderedSeeds"[4] with respect to their reachable distance to 
"center"[4].

*/
   void update(std::list<TupleId>* neighbors, TupleId centerId
    ,std::list<TupleId>& orderedSeeds);
/*
Inserts "obj"[4] into "orderedSeeds"[4].

*/
   void insert(std::list<TupleId>& orderedSeeds, TupleId objId);
/*
Moves "obj"[4] up within "orderedSeeds"[4] with respect to the reachable
distances of the elements of "orderedSeeds"[4].

*/
   void decrease(std::list<TupleId>& orderedSeeds, TupleId objId);
/*
Returns the reachable distance from "neighbor"[4] to "obj"[4].

*/
   double getReachableDist(TupleId objId, TupleId neighborId);
 };
}
