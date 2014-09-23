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
#include "Optics.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include <vector>

namespace clusteropticsalg
{

/*
2.3 Implementation of the class ~Optics~

*/
/*
Default constructor ~Optics::Optics~

*/
 Optics::Optics()
 {
  return;
 }
/*
Method ~Optics::initialize~

*/ 
 void Optics::initialize(mtreeAlgebra::MTree* queryTree, TupleBuffer* objsToOrd
  ,gta::DistfunInfo* df, int idxDistData, int idxCDist, int idxRDist
  ,int idxPrc)
 {
  mtree = queryTree;
  distFun = df;
  objs = objsToOrd;
  PNT = idxDistData;
  COR = idxCDist;
  REA = idxRDist;
  PRC = idxPrc;
 }
/*
Method ~Optics::order~

*/
 void Optics::order(double pEps, unsigned int pMinPts, list<TupleId>* pOrder)
 {
  pOrder->clear();
  
  minPts = pMinPts;
  eps = pEps;
  result = pOrder;

  GenericRelationIterator *relIter = objs->MakeScan();
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
Method ~Optics::expandClusterOrder~

*/
 void Optics::expandClusterOrder(TupleId objId)
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
Method ~Optics::getNeighbors~

*/  
 std::list<TupleId>* Optics::getNeighbors(TupleId objId)
 {
  std::list<TupleId>* near(new list<TupleId>);
  Tuple* obj;
  
  obj = objs->GetTuple(objId, false);
  
  mtree->rangeSearch((Attribute*)obj->GetAttribute(PNT), eps, near);
  near->remove(objId);
  
  return near;
 }
/*
Method ~Optics::setCoreDistance~

*/
 void Optics::setCoreDistance(std::list<TupleId>* neighbors, TupleId objId)
 {
  double coreDist = UNDEFINED;
  Tuple* obj;
  std::list<TupleId>* near;
  near = new list<TupleId>;
  
  obj = objs->GetTuple(objId, false);

  if(neighbors->size() >= minPts)
  {
   mtree->nnSearch((Attribute*)obj->GetAttribute(PNT), minPts, near);
   
   std::list<TupleId>::iterator it;
   for (it = near->begin(); it != near->end(); it++)
   {
     double distance;

     Tuple* curObj = objs->GetTuple(*it, true);

     distFun->dist(distFun->getData(obj->GetAttribute(PNT))
                  ,distFun->getData(curObj->GetAttribute(PNT))
                  ,distance);
                  
     if(distance > coreDist)
     {
      coreDist = distance;
     }     
   }
  }
  
  CcReal cDist(coreDist);
  obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
 }
/*
Method ~Optics::update~

*/
 void Optics::update(std::list<TupleId>* neighbors, TupleId centerId
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
Method ~Optics::insert~

*/
 void Optics::insert(std::list<TupleId>& orderedSeeds, TupleId objId)
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
Method ~Optics::decrease~

*/
 void Optics::decrease(std::list<TupleId>& orderedSeeds, TupleId objId)
 {
  std::list<TupleId>::iterator itId;
  std::list<TupleId>::iterator itObjId;
  
  Tuple* obj = objs->GetTuple(objId, true);
   
  for (itObjId = orderedSeeds.begin(); itObjId != orderedSeeds.end(); itObjId++)
  {
   TupleId seedId = *itObjId;
   Tuple* seed = objs->GetTuple(seedId, true);
   
   //first find the position and then raise the tuple up
   if(obj == seed)
   {
    for (itId = itObjId; itId != orderedSeeds.begin(); --itId)
    {
     seedId = *itId;
     seed = objs->GetTuple(seedId, true);
     
     if(((CcReal*)seed->GetAttribute(REA))->GetValue() 
      > ((CcReal*)obj->GetAttribute(REA))->GetValue())
     {
      orderedSeeds.splice(itObjId, orderedSeeds, itId);
      return;
     }
    }
   }
  }
 }

 double Optics::getReachableDist(TupleId objId, TupleId neighborId)
 {
  Tuple* obj = objs->GetTuple(objId, true);
  Tuple* neighbor = objs->GetTuple(neighborId, true);
  
  double distance;
  
  distFun->dist(distFun->getData(obj->GetAttribute(PNT))
               ,distFun->getData(neighbor->GetAttribute(PNT))
               ,distance);
               
  //new_r_dist:=max(c_dist,CenterObject.dist(Object));    
  return ((CcReal*)obj->GetAttribute(COR))->GetValue() > distance 
   ? ((CcReal*)obj->GetAttribute(COR))->GetValue() 
   : distance;
 } 
}
