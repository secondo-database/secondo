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

March-October 2014, Natalie Jaeckel

2.1 Overview

This file contains the "Optics"[4] class definition.

2.2 Includes

*/
#include "DBScanDC.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"


namespace clusterdbscanalg
{


 DBScanDC::DBScanDC()
 {
  id = 0;
  return;
 }



 void DBScanDC::clusterAlgo(TupleBuffer* objs, int eps, 
  int minPts, int idxClusterAttr, int    idxCID, int idxVisited
  , std::list<TupleId>* tupleIDtoCheck)
 {
         
  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
  checkList = tupleIDtoCheck;
 
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
      
      //if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED 
       //|| ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
      {
       expandCluster(objs, point, clusterId, eps, minPts);
      }
     }
    }
   }
  }
 }
 
 

 bool DBScanDC::expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
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
      ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE 
      ||
      ((CcInt*)point->GetAttribute(CID))->GetValue() != 
      ((CcInt*)obj->GetAttribute(CID))->GetValue())
     {
      expandCluster(objs, point, clusterId, eps, minPts);
     }
    }  
   }
  }
  return true;
 }
 

 std::list<TupleId>* DBScanDC::regionQuery(TupleBuffer* objs, Tuple* obj
 , int eps)
 {
  std::list<TupleId>* near(new list<TupleId>);
  
  std::list<TupleId>::iterator it;

   for(it = checkList->begin(); it != checkList->end(); it++)
   {
    Tuple* point;
  
    point = objs->GetTuple(*it, false);
       
    if( ( ((Point*)obj->GetAttribute(PNT))->GetX() - 
       ((Point*)point->GetAttribute(PNT))->GetX()) * 
       (((Point*)obj->GetAttribute(PNT))->GetX() - 
       ((Point*)point->GetAttribute(PNT))->GetX()) + 
       (((Point*)obj->GetAttribute(PNT))->GetY() - 
       ((Point*)point->GetAttribute(PNT))->GetY()) * 
       (((Point*)obj->GetAttribute(PNT))->GetY() - 
       ((Point*)point->GetAttribute(PNT))->GetY()) < eps*eps ) 
     {
     near->push_back(*it);
    }
   }
  return near;
  }
  
}
