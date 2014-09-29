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
#include "OpticsM.h"
#include "StandardTypes.h"
#include <vector>

namespace clusteropticsalg
{
/*
2.3 Implementation of the class ~Optics~

*/
/*
Default constructor ~Optics::Optics~

*/
 template<class T, class DistComp>
 OpticsM<T, DistComp>::OpticsM()
 {
  return;
 }
/*
Method ~Optics::initialize~

*/
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::initialize(
   MMMTree<pair<T, TupleId>, DistComp >* queryTree
  ,TupleBuffer* objsToOrd, int idxDistData, int idxCDist, int idxRDist
  ,int idxPrc)
 {
  mtree = queryTree;
  objs = objsToOrd;
  PNT = idxDistData;
  COR = idxCDist;
  REA = idxRDist;
  PRC = idxPrc;
 }
/*
Method ~Optics::order~

*/
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::order(double pEps, unsigned int pMinPts
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
Method ~Optics::expandClusterOrder~

*/
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::expandClusterOrder(TupleId objId)
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
 template<class T, class DistComp>
 std::list<TupleId>* OpticsM<T, DistComp>::getNeighbors(TupleId objId)
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
    near->push_back(p.second);
  }

  near->remove(objId);

  return near;
 }
/*
Method ~Optics::setCoreDistance~

*/
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::setCoreDistance(std::list<TupleId>* neighbors
  ,TupleId objId)
 {
  double coreDist = UNDEFINED;
  Tuple* obj;
  std::list<TupleId>* near;
  near = new list<TupleId>;

  obj = objs->GetTuple(objId, false);

  if(neighbors->size() >= minPts)
  {
   DistComp di;
   pair<T, TupleId> o((T) obj->GetAttribute(PNT), objId);
   NNIterator<pair<T,TupleId>, DistComp >*  it;
   it = mtree->nnSearch(o);

   unsigned int count = 0;

   while(it->hasNext() && count < minPts)
   {
    pair<T,TupleId> n = *(it->next());
    coreDist = di(o, n);
    count++;
   }
  }

  CcReal cDist(coreDist);
  obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
 }
/*
Method ~Optics::update~

*/
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::update(std::list<TupleId>* neighbors
  ,TupleId centerId, std::list<TupleId>& orderedSeeds)
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
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::insert(std::list<TupleId>& orderedSeeds
  ,TupleId objId)
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
 template<class T, class DistComp>
 void OpticsM<T, DistComp>::decrease(std::list<TupleId>& orderedSeeds
  ,TupleId objId)
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

 template<class T, class DistComp>
 double OpticsM<T, DistComp>::getReachableDist(TupleId objId
  ,TupleId neighborId)
 {
  Tuple* obj = objs->GetTuple(objId, true);
  Tuple* neighbor = objs->GetTuple(neighborId, true);

  DistComp di;

  double distance = UNDEFINED;
  pair<T, TupleId> o((T) obj->GetAttribute(PNT), objId);
  pair<T, TupleId> n((T) neighbor->GetAttribute(PNT), neighborId);
  distance = di(o, n);



  //new_r_dist:=max(c_dist,CenterObject.dist(Object));
  return ((CcReal*)obj->GetAttribute(COR))->GetValue() > distance
   ? ((CcReal*)obj->GetAttribute(COR))->GetValue()
   : distance;
 }

 template class OpticsM<CcInt*, IntDist>;
 template class OpticsM<CcReal*, RealDist>;
 template class OpticsM<Point*, PointDist>;
 template class OpticsM<CcString*, StringDist>;
}
