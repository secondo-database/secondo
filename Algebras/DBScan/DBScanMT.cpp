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

2 Source file "DBScanMT.cpp"[4]

March-December 2014, Natalie Jaeckel

2.1 Overview

This file contains the "DBScanMT"[4] class definition.

2.2 Includes

*/
#include "DBScanMT.h"
#include "StandardTypes.h"
#include "DistFunction.h"
#include "PictureAlgebra.h"
#include <vector>

/*
2.3 Implementation of the class ~DBScanMT~

*/
namespace clusterdbscanalg
{

/*
Default constructor ~DBScanMT::DBScanMT~

*/
 template<class T, class DistComp>
 DBScanMT<T, DistComp>::DBScanMT()
 {
  id = 0;
  return;
 }

 
/*
Function ~DBScanMT::clusterAlgo~

*/
 template<class T, class DistComp>
 void DBScanMT<T, DistComp>::clusterAlgo(MMMTree<pair<T, TupleId>, DistComp >* 
  queryTree, TupleBuffer* objs, int eps, int minPts, int idxClusterAttr, 
  int idxCID, int idxVisited)
 {
  mtree = queryTree;
       
  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
 
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* obj;

  int clusterId = 0;
  
  while((obj = relIter->GetNextTuple()))
  {
   if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
   {
    CcBool visited(true, true);
    obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

    std::list<TupleId>* N = regionQuery(objs, obj->GetTupleId(), eps);
    
    int nSize = N->size();    
    
    if(nSize < minPts) 
    {
     CcInt distI(true,NOISE);
     obj->PutAttribute( CID, distI.Clone() ); 
    }   
    else
    {
     clusterId = nextId();
     CcInt distI(true,clusterId);
     obj->PutAttribute( CID, distI.Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      Tuple* point = objs->GetTuple(*it, true);
      
      if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED 
       || ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
      {
       expandCluster(objs, *it, clusterId, eps, minPts);
      }
      point->DeleteIfAllowed();
     }
    }
    delete N;
   }
   obj->DeleteIfAllowed();
  }
  delete relIter;
 }
 
/*
Function ~DBScanMT::expandCluster~

*/
 template<class T, class DistComp>
 bool DBScanMT<T, DistComp>::expandCluster(TupleBuffer* objs, TupleId objId, 
  int clusterId, int eps, int minPts)
 {
  Tuple* obj = objs->GetTuple(objId, false);
  CcInt distI(true,clusterId);
  obj->PutAttribute( CID, distI.Clone() );

  if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
  {
   CcBool visited(true, true);
   obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

   //std::list<TupleId>* N = regionQuery(objs, obj, eps);
   std::list<TupleId>* N = regionQuery(objs, objId, eps);

   int nSize = N->size();
   
   if(nSize >= minPts) 
   { 
    std::list<TupleId>::iterator it;

    for (it = N->begin(); it != N->end(); it++)
    {
     Tuple* point = objs->GetTuple(*it, true);
      
     if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED ||
      ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
     {
      expandCluster(objs, *it, clusterId, eps, minPts);
     }
     point->DeleteIfAllowed();
    }  
   }
   delete N; 
  }
  obj->DeleteIfAllowed();;
  return true;
 }
 
/*
Function ~DBScanMT::regionQuery~

*/ 
 template<class T, class DistComp>
 std::list<TupleId>* DBScanMT<T, DistComp>::regionQuery(TupleBuffer* objs, 
  TupleId objId, int eps)
 {
  std::list<TupleId>* near(new list<TupleId>);
  Tuple* obj;
  
  obj = objs->GetTuple(objId, false);
  T key = (T) obj->GetAttribute(PNT);
  pair<T,TupleId> p(key, objId);
  RangeIterator<pair<T,TupleId>, DistComp >*  it;
  it = mtree->rangeSearch(p, eps);

  while(it->hasNext())
  {
   pair<T,TupleId> p = *(it->next());
   if(p.second != objId)
   {
    near->push_back(p.second);
   }
  }
  obj->DeleteIfAllowed();
  delete it;
  return near;
 }
 
/*
Definition of the available template values

*/ 
 template class DBScanMT<CcInt*, IntDist>;
 template class DBScanMT<CcReal*, RealDist>;
 template class DBScanMT<Point*, PointDist>;
 template class DBScanMT<CcString*, StringDist>;
 template class DBScanMT<Picture*, PictureDist>;
 template class DBScanMT<CcInt*, CustomDist<CcInt*, CcInt> >;
 template class DBScanMT<CcReal*, CustomDist<CcReal*, CcReal> >;
 template class DBScanMT<Point*, CustomDist<Point*, CcReal> >;
 template class DBScanMT<CcString*, CustomDist<CcString*, CcInt> >;
 template class DBScanMT<Picture*, CustomDist<Picture*, CcReal> >;
 
}
