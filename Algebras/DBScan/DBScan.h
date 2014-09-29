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

March-December 2014, Natalie Jaeckel

1.1 Overview

This file contains the "DBScan"[4] class declaration.

1.2 Includes

*/
#include "Algebra.h"
#include "MTree.h"
#include "MMRTree.h"
#include "DistfunReg.h"

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
   
  void clusterAlgo(mtreeAlgebra::MTree* queryTree, TupleBuffer* objs, int eps, 
  int minPts, int idxClusterAttr, int    clusterID, int visited);
  
  void clusterAlgo(mmrtree::RtreeT<dim, TupleId>* queryTree, TupleBuffer* objs, 
  int eps, int minPts, int idxClusterAttr, int    clusterID, int visited);


/*
1.3.2 Private members

*/
  private:
  
  const static int UNDEFINED = -1;
  const static int NOISE     = -2;
  const static int MTREE   = 1;
  const static int MMRTREE = 2;

  int PNT; 
  int CID;
  int VIS;
  int MOD;


  int nextId(){return ++id;};

  int id;
  mtreeAlgebra::MTree* mtree;
  mmrtree::RtreeT<dim, TupleId>* rtree;
  bool expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
          int eps, int minPts);
  std::list<TupleId>* regionQuery(TupleBuffer* objs, Tuple* obj, int eps);
  void changeClustId(std::list<Tuple*>& seeds, Tuple* obj, 
          int clusterId);


 };
}
