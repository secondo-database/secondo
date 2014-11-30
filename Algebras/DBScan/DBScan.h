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

1 Header file "DBScan.h"[4]

March-December 2014, Natalie Jaeckel

1.1 Overview

This file contains the "DBScan"[4] class declaration.

1.2 Includes

*/

#include "Algebra.h"
#include "MMRTree.h"
#include "TupleIdentifier.h"
#include "RelationAlgebra.h"

namespace clusterdbscanalg
{
/*
1.3 Declarations of the class ~DBScan~

*/
 template <unsigned dim>
 class DBScan;

 template <unsigned dim>
 class DBScan
 {
/*
1.3.1 Public members

*/
  public:
/*
Constructor

*/
  DBScan();
   
/*
Function ~DBScan::clusterAlgo~

This function detects cluster in a given amount of tupel. A MMRTree is used as 
an index to provide a better performance.
It starts wirh a random point of the amount and uses the expandCluster 
function to expand the cluster

*/   
  void clusterAlgo(mmrtree::RtreeT<dim, TupleId>* queryTree, TupleBuffer* objs, 
   int eps, int minPts, int idxClusterAttr, int    clusterID, int visited);


/*
1.3.2 Private members

*/
  private:
  
  const static int UNDEFINED = -1;
  const static int NOISE     = -2;

  int PNT; 
  int CID;
  int VIS;

  int nextId(){return ++id;};

  int id;
  mmrtree::RtreeT<dim, TupleId>* rtree;

/*
Function ~DBScan::expandCluster~

This function expands the cluster for a given point of the amount

*/
  bool expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
   int eps, int minPts);

/*
Function ~DBScan::regionQuery~

This function detectes all points in a given eps neighbourhood

*/
  std::list<TupleId>* regionQuery(TupleBuffer* objs, Tuple* obj, int eps);

 };
}
