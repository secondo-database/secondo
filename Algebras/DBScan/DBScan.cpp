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

2 Source file "Optics.cpp"[4]

March-October 2014, Marius Haug

2.1 Overview

This file contains the "Optics"[4] class definition.

2.2 Includes

*/
#include "DBScan.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"


namespace clusterdbscanalg
{


 template <unsigned dim>
 DBScan<dim>::DBScan()
 {
  id = 0;
  return;
 }



 template <unsigned dim>
 void DBScan<dim>::clusterAlgo(mtreeAlgebra::MTree* queryTree
 , TupleBuffer* objs, 
 int eps, int minPts, int idxClusterAttr, int idxCID, int idxVisited)
 {
  mtree = queryTree;
       
  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
  MOD = MTREE;
 
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* obj;

  int clusterId = 0;
  
  while((obj = relIter->GetNextTuple()))
  {
   if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
   {
    CcBool visited(true, true);
    obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

    std::list<TupleId>* N = regionQuery(objs, obj, eps);
    int nSize = N->size();    
    
    if(nSize < minPts) 
    {
     CcInt* distI = new CcInt;
     distI->Set(NOISE);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() ); 
    }   
    else
    {
     clusterId = nextId();
     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      //TupleId curObjId = *it;
      Tuple* point = objs->GetTuple(*it, true);
      
      if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED 
       || ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
      {
       expandCluster(objs, point, clusterId, eps, minPts);
      }
     }
    }
   }
  }
 }
 
 template <unsigned dim>
 void DBScan<dim>::clusterAlgo(mmrtree::RtreeT<dim, TupleId>* queryTree, 
   TupleBuffer* objs, int eps, int minPts, int idxClusterAttr, int idxCID, 
   int idxVisited)
 {
  rtree = queryTree;
       
  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
  MOD = MMRTREE;
 
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* obj;

  int clusterId = 0;
  
  while((obj = relIter->GetNextTuple()))
  {
   if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
   {
    CcBool visited(true, true);
    obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

    std::list<TupleId>* N = regionQuery(objs, obj, eps);
    int nSize = N->size();    
    
    if(nSize < minPts) 
    {
     CcInt* distI = new CcInt;
     distI->Set(NOISE);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() ); 
    }   
    else
    {
     clusterId = nextId();
     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      //TupleId curObjId = *it;
      Tuple* point = objs->GetTuple(*it, true);
      
      if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED 
       || ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
      {
       expandCluster(objs, point, clusterId, eps, minPts);
      }
     }
    }
   }
  }
 }

 template <unsigned dim>
 bool DBScan<dim>::expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
          int eps, int minPts)
 {

  CcInt* distI = new CcInt;
  distI->Set(clusterId);
  obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );

  if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
  {
   CcBool visited(true, true);
   obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

   std::list<TupleId>* N = regionQuery(objs, obj, eps);

   int nSize = N->size();
   
   if(nSize >= minPts) 
   { 
    std::list<TupleId>::iterator it;

    for (it = N->begin(); it != N->end(); it++)
    {
     //TupleId curObjId = *it;
     Tuple* point = objs->GetTuple(*it, true);
      
     if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED ||
      ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
     {
      expandCluster(objs, point, clusterId, eps, minPts);
     }
    }  
   }
  }
  return true;
 }
 
 template <unsigned dim>
 std::list<TupleId>* DBScan<dim>::regionQuery(TupleBuffer* objs, Tuple* obj
 , int eps)
 {
  std::list<TupleId>* near(new list<TupleId>);
  
  if(MOD == MTREE)
  {
   mtree->rangeSearch((Attribute*)obj->GetAttribute(PNT), eps, near);
  }
  else if (MOD == MMRTREE)
  {
   double x = ((Point*) obj->GetAttribute(PNT))->GetX();
   double y = ((Point*) obj->GetAttribute(PNT))->GetY();
   double recP1[2];
   double recP2[2];
    
   recP1[0] = x - eps;
   recP1[1] = y - eps;
   recP2[0] = x + eps;
   recP2[1] = y + eps;
                
   Rectangle<dim> rEps(true, recP1, recP2);
   set<TupleId> b;
   rtree->findAll(rEps, b);
   
   set<TupleId>::iterator it;
   for(it = b.begin(); it != b.end(); it++)
   {
    Tuple* curObj;
  
    curObj = objs->GetTuple(*it, false);
    
    double distance = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
     (*(Rectangle<dim>*)curObj->GetAttribute(PNT)));
    
    if(distance <= eps)
    { 
     near->push_back(*it);
    }
   }
  }
  return near;
 }
 
 template class DBScan<0>;
 template class DBScan<2>;
 template class DBScan<3>;
 template class DBScan<4>;
 template class DBScan<8>;
 
}
