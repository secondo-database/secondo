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

2 Source file "OpticsR.cpp"[4]

March-October 2014, Marius Haug

2.1 Overview

This file contains the "OpticsR"[4] class definition.

2.2 Includes

*/
#include "OpticsR.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RectangleAlgebra.h"

namespace clusteropticsalg
{
/*
2.3 Implementation of the class ~OpticsR~

*/
/*
Default constructor ~OpticsR::OpticsR~

*/
 template <unsigned dim>
 OpticsR<dim>::OpticsR()
 {
  return;
 }
/*
Method ~OpticsR::initialize~

*/
 template <unsigned dim>
 void OpticsR<dim>::initialize(mmrtree::RtreeT<dim, TupleId>* queryTree
  ,TupleBuffer* objsToOrd, int idxDistData, int idxCDist, int idxRDist
  ,int idxPrc)
 {
  rtree = queryTree;
  objs = objsToOrd;
  PNT = idxDistData;
  COR = idxCDist;
  REA = idxRDist;
  PRC = idxPrc;
 }
/*
Method ~OpticsR::order~

*/
 template <unsigned dim>
 void OpticsR<dim>::order(double pEps, unsigned int pMinPts
  ,list<TupleId>* pOrder)
 {
  pOrder->clear();
  
  minPts = pMinPts;
  eps = pEps;
  result = pOrder;

  GenericRelationIterator* relIter = objs->MakeScan();
  TupleId objId;
  Tuple* obj;

  while((obj = relIter->GetNextTuple()))
  {
   objId = relIter->GetTupleId();
   
   if(!((CcBool*)obj->GetAttribute(PRC))->GetValue())
   {
    expandClusterOrder(objId);
   }
  }
 }
/*
Method ~OpticsR::expandClusterOrder~

*/
 template <unsigned dim>
 void OpticsR<dim>::expandClusterOrder(TupleId objId)
 {
  Tuple* obj;
  std::list<TupleId> orderedSeeds;
  
  obj = objs->GetTuple(objId, false);
  
  std::list<TupleId>* neighbors = getNeighbors(objId);

  CcBool processed(true, true);
  obj->PutAttribute(PRC, ((Attribute*) &processed)->Clone());
  
  CcReal rDist(UNDEFINED);
  obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());

  setCoreDistance(neighbors, objId);
  //orderedOut add the Point obj
  result->push_back(objId);

  //(obj->distCore != UNDEFINED)
  if(((CcReal*)obj->GetAttribute(COR))->GetValue() != UNDEFINED) 
  {
   update(neighbors, objId, orderedSeeds);

   std::list<TupleId>::iterator it;
   for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
   {
    TupleId curObjId = *it;
    Tuple* curObj = objs->GetTuple(*it, true);

    neighbors = getNeighbors(curObjId);
    
    CcBool processed(true, true);
    curObj->PutAttribute(PRC, ((Attribute*) &processed)->Clone());

    setCoreDistance(neighbors, curObjId);
    
    //orderedOut add the Point curObj
    result->push_back(curObjId);
    
    //(currentObject->distCore != UNDEFINED)
    if(((CcReal*)curObj->GetAttribute(COR))->GetValue() != UNDEFINED) 
    {
     update(neighbors, curObjId, orderedSeeds);
    }
   }
  }
 }
/*
Method ~OpticsR::getNeighbors~

*/
 template <unsigned dim>
 std::list<TupleId>* OpticsR<dim>::getNeighbors(TupleId objId)
 {
  std::list<TupleId>* near(new list<TupleId>);
  Tuple* obj;
  
  obj = objs->GetTuple(objId, false);
  
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
  
  return near;
 }
/*
Method ~OpticsR::setCoreDistance~

*/
 template <unsigned dim>
 void OpticsR<dim>::setCoreDistance(std::list<TupleId>* neighbors
  ,TupleId objId)
 {
  double coreDist = UNDEFINED;
  Tuple* obj;
  std::list<TupleId>* near;
  near = new list<TupleId>;
  
  obj = objs->GetTuple(objId, false);

  if(neighbors->size() >= minPts)
  {
   double curDist  = UNDEFINED;
   double lastDist = UNDEFINED;
   unsigned int count   = 0;
   unsigned int biggest = 0;
   TupleId nearest[minPts];

   Tuple* obj = objs->GetTuple(objId, true);
   
   std::list<TupleId>::iterator it;
   for (it = neighbors->begin(); it != neighbors->end(); it++)
   {
    TupleId curObjId = *it;
    Tuple* neighbor = objs->GetTuple(curObjId, true);

    if(count < minPts)
    {
     nearest[count++] = curObjId;
     
     curDist = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
      (*(Rectangle<dim>*)neighbor->GetAttribute(PNT)));

     if(curDist > lastDist)
     {
      biggest = count-1;
      coreDist = curDist;
     }

     lastDist = curDist;
    }
    else
    {
     curDist = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
      (*(Rectangle<dim>*)neighbor->GetAttribute(PNT)));
    
     for (unsigned int i = 0; i < count; i++)
     {
      Tuple* curNear = objs->GetTuple(nearest[i], true);
      
      double cDst = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
       (*(Rectangle<dim>*)curNear->GetAttribute(PNT)));
      
      if(cDst > lastDist)
      {
       biggest = i;
       coreDist = cDst;
      }

      lastDist = cDst;
     }

     Tuple* curNear = objs->GetTuple(nearest[biggest], true);

     double dstnc = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
      (*(Rectangle<dim>*)curNear->GetAttribute(PNT)));

     if(dstnc > curDist)
     {
      nearest[biggest] = curObjId;
      coreDist = curDist;
     }
    }
   }
  }
  
  CcReal cDist(coreDist);
  obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
 }
/*
Method ~OpticsR::update~

*/
 template <unsigned dim>
 void OpticsR<dim>::update(std::list<TupleId>* neighbors, TupleId centerId
    ,std::list<TupleId>& orderedSeeds)
 {
  //FORALL Object FROM neighbors DO
  std::list<TupleId>::iterator it;
  for (it = neighbors->begin(); it != neighbors->end(); it++)
  {
   TupleId objId = *it;
   Tuple* obj = objs->GetTuple(objId, false);

   //IF NOT Object.Processed THEN   
   if(!(((CcBool*)obj->GetAttribute(PRC))->GetValue())) //(!obj->processed)
   {
    //new_r_dist:=max(c_dist,CenterObject.dist(Object));
    double newReachDist = getReachableDist(centerId, objId);

    //IF Object.reachability_distance=UNDEFINED THEN
    if(((CcReal*)obj->GetAttribute(REA))->GetValue() == UNDEFINED) 
    {
     //Object.reachability_distance := new_r_dist;
     CcReal rDist(newReachDist);
     obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
     insert(orderedSeeds, objId);
    }
    //ELSE IF new_r_dist<Object.reachability_distance THEN
    // Object already in OrderSeeds
    else if(newReachDist < ((CcReal*)obj->GetAttribute(REA))->GetValue())
    {
     //Object.reachability_distance := new_r_dist;
     CcReal rDist(newReachDist);
     obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
     decrease(orderedSeeds, objId);
    }
   }
  }
 }
/*
Method ~OpticsR::insert~

*/
 template <unsigned dim>
 void OpticsR<dim>::insert(std::list<TupleId>& orderedSeeds, TupleId objId)
 {
  std::list<TupleId>::iterator it;
  Tuple* obj;
  
  obj = objs->GetTuple(objId, true);
  
  for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
  {
   TupleId seedId = *it;
   Tuple* seed = objs->GetTuple(seedId, true);
   
   if(((CcReal*)seed->GetAttribute(REA))->GetValue()
    > ((CcReal*)obj->GetAttribute(REA))->GetValue())
   {
    orderedSeeds.insert(it, objId);
    return;
   }
  }
  
  orderedSeeds.push_back(objId);
 }
/*
Method ~OpticsR::decrease~

*/
 template <unsigned dim>
 void OpticsR<dim>::decrease(std::list<TupleId>& orderedSeeds, TupleId objId)
 {
  bool found    = false;
  bool decrease = false;
  Tuple* seed;
  Tuple* obj = objs->GetTuple(objId, true);
  
  std::list<TupleId>::iterator itObjId;
  for (itObjId = orderedSeeds.begin(); itObjId != orderedSeeds.end(); itObjId++)
  {
   seed = objs->GetTuple(*itObjId, true);

   //first find the position
   if(obj == seed)
   {
    found = true;
    break;
   }
  }

  if(found)
  {
   //Now raise the tuple up
   std::list<TupleId>::iterator itId;
   for (itId = itObjId; itId != orderedSeeds.begin(); --itId)
   {
    seed = objs->GetTuple(*itId, true);

    if(((CcReal*)obj->GetAttribute(REA))->GetValue()
     > ((CcReal*)seed->GetAttribute(REA))->GetValue())
    {
     decrease = true;
     break;
    }
   }
   
   if(decrease)
   {
    orderedSeeds.insert(++itId, 1, objId);
    orderedSeeds.erase(itObjId);
   }
  }
 }

 template <unsigned dim>
 double OpticsR<dim>::getReachableDist(TupleId objId, TupleId neighborId)
 {
  Tuple* obj = objs->GetTuple(objId, true);
  Tuple* neighbor = objs->GetTuple(neighborId, true);
  
  double distance = ((Rectangle<dim>*)obj->GetAttribute(PNT))->Distance(
      (*(Rectangle<dim>*)neighbor->GetAttribute(PNT)));
         
  return ((CcReal*)obj->GetAttribute(COR))->GetValue() > distance 
   ? ((CcReal*)obj->GetAttribute(COR))->GetValue() 
   : distance;
 } 
 
 template class OpticsR<2>;
 template class OpticsR<3>;
 template class OpticsR<4>;
 template class OpticsR<8>;
}
