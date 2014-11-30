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

1 Header file "DBScanDAC.h"[4]

March-December 2014, Natalie Jaeckel

1.1 Overview

This file contains the "DBScanDAC"[4] class declaration.

1.2 Includes

*/
#include "Algebra.h"
#include "TupleIdentifier.h"
#include "RelationAlgebra.h"

namespace clusterdbscanalg
{

/*
1.3 Declarations of the class ~DBScanDAC~

*/

 class DBScanDAC;


 class DBScanDAC
 {
 
/*
1.3.1 Public members

*/
  public:
  
/*
Constructor

*/
   DBScanDAC();
   

/*
Function ~DBScanDAC::cluster~

Start function for the DBScan Divide and Conquer implementation

*/   
  void cluster(TupleBuffer* objs,std::list<TupleId>* tupleIds,  int eps, 
  int minPts, int idxClusterAttr, int    clusterID, int visited);
  
/*
Function ~DBScanDAC::merge~

Start function for the DBScan Merge operator

*/  
  void merge(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
  int eps, int minPts, int idxClusterAttr, int    idxCID, int idxVisited);
 
 
 

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
  
/*
Method ~DBScanDAC::clusterAlgo~

Detects the cluster when no merge is needed

*/  
  void clusterAlgo(TupleBuffer* objs, std::list<TupleId>* tupleIds, int eps, 
    int minPts);
   
/*
Method ~DBScanDAC::clusterAlgoMerge~

Merges the found cluster

*/    
  void clusterAlgoMerge(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
   int eps, int minPts, double border);
  
/*
Method ~DBScanDAC::div~

Divides the amount till size(amount) <= MinPts

*/
  void div(TupleBuffer* objs, std::list<TupleId>* tupleIds, int minPts, 
   int eps, double border);


/*
Method ~DBScan::expandCluster~

This function expands the cluster for a given point of the amount

*/    
  bool expandCluster(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
   Tuple* obj, int clusterId, int eps, int minPts);

/*
Method ~DBScanDAC::expandClusterMerge~

This function expands the cluster for a given point of the amount in the merge
step

*/   
  bool expandClusterMerge(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
   Tuple* obj, int clusterId, int eps, int minPts);    
         
/*
Method ~DBScan::regionQuery~

This function detectes all points in a given eps neighbourhood

*/
  std::list<TupleId>* regionQuery(TupleBuffer* objs, std::list<TupleId>* 
     tupleIds, Tuple* obj, int eps);


 };
}
