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

2 Source file "DBScanDAC.cpp"[4]

March-December 2014, Natalie Jaeckel

2.1 Overview

This file contains the "DBScanDAC"[4] class definition.

2.2 Includes

*/
#include "DBScanDAC.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"
#include <map>

/*
2.3 Implementation of the class ~DBScanDAC~

*/
namespace clusterdbscanalg
{
/*
Default constructor ~DBScanDAC::DBScanDAC~

*/
 DBScanDAC::DBScanDAC()
 {
  id = 0;
  return;
 }
 
/*
Function ~DBScanDAC::merge~

*/ 
 void DBScanDAC::merge(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
  int eps, int minPts, int idxClusterAttr, int    idxCID, int idxVisited)
 {  

  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
  
  Tuple* obj;
  int clusterId = 0;
     
  std::list<TupleId>::iterator itTIDs;
  for (itTIDs = tupleIds->begin(); itTIDs != tupleIds->end(); itTIDs++)
  {

   obj = objs->GetTuple(*itTIDs, true);
   
   if( ((CcBool*)obj->GetAttribute(VIS))->GetValue() && 
    ((CcInt*)obj->GetAttribute(CID))->GetValue() != NOISE)
   {
    std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
    int nSize = N->size(); 

    if(nSize >= minPts) 
    {
     clusterId = ((CcInt*)obj->GetAttribute(CID))->GetValue();

     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      Tuple* point = objs->GetTuple(*it, true);
      expandClusterMerge(objs, tupleIds, point, clusterId, eps, minPts);
     }    
    }
   }
   else if( ((CcBool*)obj->GetAttribute(VIS))->GetValue() && 
    ((CcInt*)obj->GetAttribute(CID))->GetValue() == NOISE)
   {
    std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
    int nSize = N->size(); 
     
    if(nSize >= minPts) 
    {
     clusterId = nextId();
     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;
     for (it = N->begin(); it != N->end(); it++)
     {
      Tuple* point = objs->GetTuple(*it, true);
      expandClusterMerge(objs, tupleIds, point, clusterId, eps, minPts);
     } 
    }
   } 
  }  
 } 
 
 
/*
Function ~DBScanDAC::cluster~

*/
 void DBScanDAC::cluster(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
  int eps, int minPts, int idxClusterAttr, int    idxCID, int idxVisited)
 {
  PNT = idxClusterAttr;
  CID = idxCID;
  VIS = idxVisited;
  double border = -1;
  std::map<int, int> m;
  std::map<int,int>::iterator itMap;
  int c = 0;
  int clusID;
    
  int tIdsSize = tupleIds->size();
 
  //In case the amount of tuple is less than MinPts, there is no need to divide
  //the amount
  if(tIdsSize <=   minPts)  
  { 
   clusterAlgo(objs, tupleIds, eps, minPts);
  }
  else
  {
   div(objs, tupleIds, minPts, eps, border);
  }
  
  std::list<TupleId>::iterator it;
  for (it = tupleIds->begin(); it != tupleIds->end(); it++)
  {
   Tuple* tmp = objs->GetTuple(*it, true);
   clusID = ((CcInt*)tmp->GetAttribute(CID))->GetValue();
   if(clusID != NOISE)
   {
    itMap = m.find(clusID);
    if(itMap == m.end())
    {
     m.insert ( std::pair<int,int>(clusID,++c) );
    }
    CcInt* distI = new CcInt;
    distI->Set(m.find(clusID)->second);
    tmp->PutAttribute( CID, ((Attribute*)distI)->Clone() ); 
   }
  }
  
 }
 
/*
Method ~DBScanDAC::div~

*/
void DBScanDAC::div(TupleBuffer* objs,  
 std::list<TupleId>* tupleIds, int minPts, int eps, double border)
 {
   std::list<TupleId>* v1(new list<TupleId>);
   std::list<TupleId>* v2(new list<TupleId>);
         
   std::list<TupleId>::iterator it;
   int tIdsSize = tupleIds->size()/2;

   //Abort condition
   if(tupleIds->size() <= minPts*2)  
   {
    clusterAlgo(objs, tupleIds, eps, minPts);
  
    return;
   }
   else
   {
    Tuple* tmpLeft;
    Tuple* tmpRight;
    double v1Border = -1;    
    double v2Border = -1;
    bool first = true;
    int count = 0;
   
    for (it = tupleIds->begin(); it != tupleIds->end(); it++)
    {
     if(count < tIdsSize)
     {
      v1->push_back(*it);
      tmpLeft = objs->GetTuple(*(tupleIds->begin()), true);
      v1Border = ((Point*)tmpLeft->GetAttribute(PNT))->GetX();
     }
     else
     {
      if(first)
      {
       tmpRight = objs->GetTuple(*it, true);
       v2Border = ((Point*)tmpRight->GetAttribute(PNT))->GetX();
      }
      v2->push_back(*it);
      first = false;
     }
     count++;
    }
    
    if(v1->size() == 0)
    {
     border = v2Border;
    }
    else if (v2->size() == 0)
    {
     border = v1Border;
    }
    else
    {
     double dist = ((v2Border - v1Border) / 2);
     double abs = dist < 0 ? dist * -1 : dist;
     border = abs + v1Border;
    }
    
    div(objs, v1, minPts, eps, border);
    div(objs, v2, minPts, eps, border);
    
    v1->splice(v1->end(), *v2);
    clusterAlgoMerge(objs, v1, eps, minPts, border);
    
    return;
   }  
 }
 
/*
Method ~DBScanDAC::clusterAlgo~

*/
 void DBScanDAC::clusterAlgo(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
  int eps, int minPts)
 {  
  Tuple* obj;
  int clusterId = 0;
  
  std::list<TupleId>::iterator itTIDs;
  for (itTIDs = tupleIds->begin(); itTIDs != tupleIds->end(); itTIDs++)
  {
   obj = objs->GetTuple(*itTIDs, true);
   if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
   {
    CcBool visited(true, true);
    obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

    std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
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
      Tuple* point = objs->GetTuple(*it, true);

      if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED 
      || ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE)
      {
       expandCluster(objs, tupleIds, point, clusterId, eps, minPts);
      }
     }
    }
   }
  }
 }
 

/*
Method ~DBScanDAC::clusterAlgoMerge~

*/
 void DBScanDAC::clusterAlgoMerge(TupleBuffer* objs, std::list<TupleId>* 
 tupleIds, int eps, int minPts, double border)
 {    
  Tuple* obj;
  int clusterId = 0;
  
  std::list<TupleId>::iterator itTIDs;
  for (itTIDs = tupleIds->begin(); itTIDs != tupleIds->end(); itTIDs++)
  {
   obj = objs->GetTuple(*itTIDs, true);
   
   if( ((CcBool*)obj->GetAttribute(VIS))->GetValue() && 
    ((CcInt*)obj->GetAttribute(CID))->GetValue() != NOISE)
   {
    std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
    int nSize = N->size(); 
   
    if(nSize >= minPts) 
    {
     clusterId = ((CcInt*)obj->GetAttribute(CID))->GetValue();

     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      Tuple* point = objs->GetTuple(*it, true);
      expandClusterMerge(objs, tupleIds, point, clusterId, eps, minPts);
     }
         
    }
   }
   else if( ((CcBool*)obj->GetAttribute(VIS))->GetValue() && 
    ((CcInt*)obj->GetAttribute(CID))->GetValue() == NOISE)
   {
    std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
    int nSize = N->size(); 
   
    if(nSize >= minPts) 
    {
     clusterId = nextId();
     CcInt* distI = new CcInt;
     distI->Set(clusterId);
     obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
     
     std::list<TupleId>::iterator it;

     for (it = N->begin(); it != N->end(); it++)
     {
      Tuple* point = objs->GetTuple(*it, true);
      expandClusterMerge(objs, tupleIds, point, clusterId, eps, minPts);
     } 
    }
   } 
  }  
 }
 
/*
Method ~DBScanDAC::expandClusterMerge~

*/
 bool DBScanDAC::expandClusterMerge(TupleBuffer* objs, 
  std::list<TupleId>* tupleIds, Tuple* obj, int clusterId, int eps, int minPts)
 {
  CcInt* distI = new CcInt;
  distI->Set(clusterId);
  obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );
 
  if( ((CcBool*)obj->GetAttribute(VIS))->GetValue())
  {
   std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
   int nSize = N->size();

   if(nSize >= minPts) 
   { 
    std::list<TupleId>::iterator it;

    for (it = N->begin(); it != N->end(); it++)
    {
     Tuple* point = objs->GetTuple(*it, true);
      
     if( ((CcInt*)point->GetAttribute(CID))->GetValue() != clusterId )
     {
      expandClusterMerge(objs, tupleIds, point, clusterId, eps, minPts);
     }
    }  
   }
  }
  return true;
 }
 
/*
Method ~DBScan::expandCluster~

*/
 bool DBScanDAC::expandCluster(TupleBuffer* objs, std::list<TupleId>* tupleIds, 
  Tuple* obj, int clusterId, int eps, int minPts)
 {
  CcInt* distI = new CcInt;
  distI->Set(clusterId);
  obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );

  if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
  {
   CcBool visited(true, true);
   obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

   std::list<TupleId>* N = regionQuery(objs, tupleIds, obj, eps);
   int nSize = N->size();

   if(nSize >= minPts) 
   { 
    std::list<TupleId>::iterator it;

    for (it = N->begin(); it != N->end(); it++)
    {
     Tuple* point = objs->GetTuple(*it, true);
      
     if(((CcInt*)point->GetAttribute(CID))->GetValue() == UNDEFINED ||
      ((CcInt*)point->GetAttribute(CID))->GetValue() == NOISE )
     {
      expandCluster(objs, tupleIds, point, clusterId, eps, minPts);
     }
    }  
   }
  }
  return true;
 }
 
 
/*
Method ~DBScan::regionQuery~

*/
 std::list<TupleId>* DBScanDAC::regionQuery(TupleBuffer* objs, 
  std::list<TupleId>* tupleIds, Tuple* obj, int eps)
 {
  Tuple* point;
  std::list<TupleId>* near(new list<TupleId>);

  std::list<TupleId>::iterator it;
  for (it = tupleIds->begin(); it != tupleIds->end(); it++)
  {   
   point = objs->GetTuple(*it, true);
   if(obj != point)
   { 
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
  }
  return near;
 }
  
}
