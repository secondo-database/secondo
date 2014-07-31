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

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "SpatialAlgebra.h"
#include "LogMsg.h"
#include "Stream.h"
#include "MMRTree.h"

#include <limits.h>
#include <float.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusteropticsalg
{
 struct OpticsOrder
 {
  bool processed;
  double x;
  double y;
  double distReach;
  double distCore;
 };

 class Optics;

 class Optics
 {
  public:
   const static int PNT = 0;
   const static int COR = 1;
   const static int REA = 2;
   const static int PRC = 3;
   const static int ORD = 4;
   const static double UNDEFINED = -1.0;
   Optics();
   int getNextOrder() { return ++orderPoints * 100;};
   void order(TupleBuffer* objs, int eps, int minPts, TupleBuffer* order);

  private:
   int orderPoints;
   void expandClusterOrder(TupleBuffer* objs, Tuple* obj, int eps
   ,int minPts, TupleBuffer* order);
   std::list<Tuple*> getNeighbors(TupleBuffer* objs, Tuple* obj, int eps);
   void setCoreDistance(std::list<Tuple*>& objs, Tuple* obj
   ,int eps, int minPts);
   void update(std::list<Tuple*>& objs, Tuple* obj
   ,std::list<Tuple*>& orderedSeeds);
   void insert(std::list<Tuple*>& orderedSeeds, Tuple* obj);
   void decrease(std::list<Tuple*>& orderedSeeds, Tuple* obj);
   double getReachableDist(Tuple* obj, Tuple* neighbor);
 };
 
 Optics::Optics()
 {
  orderPoints = 0;
  return;
 }

 void Optics::order(TupleBuffer* objs, int eps, int minPts
 ,TupleBuffer* order)
 {
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* obj;

  while((obj = relIter->GetNextTuple()))
  {
   if(!((CcBool*)obj->GetAttribute(PRC))->GetValue())
   {
   expandClusterOrder(objs, obj, eps, minPts, order);
   }
  }
 }

 void Optics::expandClusterOrder(TupleBuffer* objs, Tuple* obj
 ,int eps, int minPts, TupleBuffer* order)
 {
  std::list<Tuple*> orderedSeeds;
  
  std::list<Tuple*> neighbors = getNeighbors(objs, obj, eps);

  CcBool processed(true, true);
  obj->PutAttribute(PRC, ((Attribute*) &processed)->Clone());
  
  CcReal rDist(UNDEFINED);
  obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());

  setCoreDistance(neighbors, obj, eps, minPts);
  //orderedOut add the Point obj
  order->AppendTuple(obj);

 //(obj->distCore != UNDEFINED)
  if(((CcReal*)obj->GetAttribute(COR))->GetValue() != UNDEFINED) 
  {
   update(neighbors, obj, orderedSeeds);

   std::list<Tuple*>::iterator it;
   for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
   {
   Tuple* currentObject = *it;

   std::list<Tuple*> neighbors = getNeighbors(objs, currentObject, eps);
   
   CcBool processed(true, true);
   currentObject->PutAttribute(PRC, ((Attribute*) &processed)->Clone());

   setCoreDistance(neighbors, currentObject, eps, minPts);
   //orderedOut add the Point currentObject
   order->AppendTuple(currentObject);

 //(currentObject->distCore != UNDEFINED)
   if(((CcReal*)currentObject->GetAttribute(COR))->GetValue()
    != UNDEFINED) 
   {
    update(neighbors, currentObject, orderedSeeds);
   }
   }
  }
 }
  
 std::list<Tuple*> Optics::getNeighbors(TupleBuffer* objs, Tuple* obj
 ,int eps)
 {
  std::list<Tuple*> near;

  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* point;

  while((point = relIter->GetNextTuple()))
  {
   if(((Point*)obj->GetAttribute(PNT))->GetX()
  != ((Point*)point->GetAttribute(PNT))->GetX()
 || ((Point*)obj->GetAttribute(PNT))->GetY()
 != ((Point*)point->GetAttribute(PNT))->GetY())
   {
   if(((((Point*)obj->GetAttribute(PNT))->GetX()
    - ((Point*)point->GetAttribute(PNT))->GetX()) 
  * (((Point*)obj->GetAttribute(PNT))->GetX()
  - ((Point*)point->GetAttribute(PNT))->GetX()))
  + ((((Point*)obj->GetAttribute(PNT))->GetY()
  - ((Point*)point->GetAttribute(PNT))->GetY()) 
  * (((Point*)obj->GetAttribute(PNT))->GetY() 
  - ((Point*)point->GetAttribute(PNT))->GetY())) < eps*eps) 
   {
    near.push_back(point);
   }
   }
  }

  return near;
 }


 void Optics::setCoreDistance(std::list<Tuple*>& neighbors, Tuple* obj
 ,int eps, int minPts)
 {
  double curDist = UNDEFINED;
  double lastDist = UNDEFINED;
  double coreDist = UNDEFINED;
  int count = 0;
  int biggest = 0;
  Tuple* nearest[minPts];

  std::list<Tuple*>::iterator it;
  for (it = neighbors.begin(); it != neighbors.end(); it++)
  {
   Tuple* neighbor = *it;

   if(count < minPts)
   {
   nearest[count++] = neighbor;

   curDist = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
     - ((Point*)neighbor->GetAttribute(PNT))->GetX())
   * (((Point*)obj->GetAttribute(PNT))->GetX()
   - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
   + ((((Point*)obj->GetAttribute(PNT))->GetY()
   - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
   * (((Point*)obj->GetAttribute(PNT))->GetY()
   - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

   if(curDist > lastDist)
   {
    biggest = count-1;
    coreDist = curDist;
   }

   lastDist = curDist;
   }
   else
   {
   curDist = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
     - ((Point*)neighbor->GetAttribute(PNT))->GetX()) 
   * (((Point*)obj->GetAttribute(PNT))->GetX()
   - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
   + ((((Point*)obj->GetAttribute(PNT))->GetY()
   - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
   * (((Point*)obj->GetAttribute(PNT))->GetY()
   - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

   for (int i = 0; i < count; i++)
   {
    Tuple* curNear = nearest[i];

    double cDst = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
       - ((Point*)curNear->GetAttribute(PNT))->GetX())
    * (((Point*)obj->GetAttribute(PNT))->GetX()
    - ((Point*)curNear->GetAttribute(PNT))->GetX()))
    + ((((Point*)obj->GetAttribute(PNT))->GetY()
    - ((Point*)curNear->GetAttribute(PNT))->GetY())
    * (((Point*)obj->GetAttribute(PNT))->GetY()
    - ((Point*)curNear->GetAttribute(PNT))->GetY())));

    if(cDst > lastDist)
    {
     biggest = i;
     coreDist = cDst;
    }

    lastDist = cDst;
   }

   Tuple* curNear = nearest[biggest];
   
   double dstnc = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX()
      - ((Point*)curNear->GetAttribute(PNT))->GetX()) 
    * (((Point*)obj->GetAttribute(PNT))->GetX()
    - ((Point*)curNear->GetAttribute(PNT))->GetX())) 
    + ((((Point*)obj->GetAttribute(PNT))->GetY()
    - ((Point*)curNear->GetAttribute(PNT))->GetY()) 
    * (((Point*)obj->GetAttribute(PNT))->GetY()
    - ((Point*)curNear->GetAttribute(PNT))->GetY())));

   if(dstnc > curDist)
   {
    nearest[biggest] = neighbor;
   }
   }
  }

  CcReal cDist(coreDist);
  obj->PutAttribute(COR, ((Attribute*) &cDist)->Clone());
 }

 void Optics::update(std::list<Tuple*>& neighbors, Tuple* center
 ,std::list<Tuple*>& orderedSeeds)
 {
  //c_dist := CenterObject.core_distance;
  //double coreDist = ((CcReal*)center->GetAttribute(COR))->GetValue();

  //FORALL Object FROM neighbors DO
  std::list<Tuple*>::iterator it = neighbors.begin();
  for (it = neighbors.begin(); it != neighbors.end(); it++)
  {
   Tuple* obj = *it;

   //IF NOT Object.Processed THEN   
   if(!((CcBool*)obj->GetAttribute(PRC))->GetValue()) //(!obj->processed)
   {
   //new_r_dist:=max(c_dist,CenterObject.dist(Object));
   double newReachDist = getReachableDist(center, obj);

   //IF Object.reachability_distance=UNDEFINED THEN
   if(((CcReal*)obj->GetAttribute(REA))->GetValue() == UNDEFINED) 
   {
    //Object.reachability_distance := new_r_dist;
    CcReal rDist(newReachDist);
    obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
    insert(orderedSeeds, obj);
   }
   //ELSE IF new_r_dist<Object.reachability_distance THEN
   // Object already in OrderSeeds
   else if(newReachDist < ((CcReal*)obj->GetAttribute(REA))->GetValue())
   {
    //Object.reachability_distance := new_r_dist;
    CcReal rDist(newReachDist);
    obj->PutAttribute(REA, ((Attribute*) &rDist)->Clone());
    decrease(orderedSeeds, obj);
   }
   }
  }
 }

 void Optics::insert(std::list<Tuple*>& orderedSeeds, Tuple* obj)
 {
  std::list<Tuple*>::iterator it;

  for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
  {
   Tuple* seed = *it;
   if(((CcReal*)seed->GetAttribute(REA))->GetValue()
   > ((CcReal*)obj->GetAttribute(REA))->GetValue())
   {
   orderedSeeds.insert(it, obj);
   return;
   }
  }
  
  orderedSeeds.push_back(obj);
 }

 void Optics::decrease(std::list<Tuple*>& orderedSeeds, Tuple* obj)
 {
  std::list<Tuple*>::iterator it;
  std::list<Tuple*>::iterator itObj;

  for (itObj = orderedSeeds.begin(); itObj != orderedSeeds.end(); itObj++)
  {
   if(obj == ((Tuple*) *itObj))
   {
   for (it = itObj; it != orderedSeeds.begin(); --it)
   {
    Tuple* seed = *it;
    if(((CcReal*)seed->GetAttribute(REA))->GetValue() 
    > ((CcReal*)obj->GetAttribute(REA))->GetValue())
    {
     orderedSeeds.splice(itObj, orderedSeeds, it);
     return;
    }
   }
   }
  }
 }

 double Optics::getReachableDist(Tuple* obj, Tuple* neighbor)
 {
  double distance = sqrt(((((Point*)obj->GetAttribute(PNT))->GetX() 
     - ((Point*)neighbor->GetAttribute(PNT))->GetX()) 
    * (((Point*)obj->GetAttribute(PNT))->GetX()
    - ((Point*)neighbor->GetAttribute(PNT))->GetX()))
    + ((((Point*)obj->GetAttribute(PNT))->GetY()
    - ((Point*)neighbor->GetAttribute(PNT))->GetY()) 
    * (((Point*)obj->GetAttribute(PNT))->GetY() 
    - ((Point*)neighbor->GetAttribute(PNT))->GetY())));

  //new_r_dist:=max(c_dist,CenterObject.dist(Object));   
  return ((CcReal*)obj->GetAttribute(COR))->GetValue() > distance 
  ? ((CcReal*)obj->GetAttribute(COR))->GetValue() 
  : distance;
 } 
 
 ListExpr opticsType( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("one element expected");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
   return listutils::typeError("stream(Tuple) expected");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=2)
  {
   ErrorReporter::ReportError("non conform list of Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcInt::checkType(nl->First(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Second(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }

//  ListExpr tuple = nl->Second(stream);

//  if(!Points::checkType(nl->First(tuple)))
//  {
//   return listutils::typeError("stream(Tuple(Points)) expected");
//  }

//  ListExpr sortSpecification = nl->Second(args);

//  int numberOfSortAttrs = nl->ListLength(sortSpecification);

//  if(numberOfSortAttrs != 2)
//  {
//   return listutils::typeError("eps and MinPts expected");
//  } 

  // copy attrlist to newattrlist
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn = newAttrList;

  lastlistn = nl->Append(lastlistn
  ,nl->TwoElemList(nl->SymbolAtom("CoreDist")
 ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
  ,nl->TwoElemList(nl->SymbolAtom("ReachDist")
 ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
  ,nl->TwoElemList(nl->SymbolAtom("Processed")
 ,nl->SymbolAtom(CcBool::BasicType())));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND())
 ,nl->TwoElemList(nl->IntAtom(2), arguments)
 ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
 ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
  ,newAttrList)));
 }
 
 int opticsFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  //Default values
  int defEps = 9999999;
  int defMinPts = 2;
  
  //Optics instance
  Optics optics;

  TupleType *resultTupleType;
  ListExpr resultType;
  TupleBuffer *tp;
  TupleBuffer *order;
  long MaxMem;
  GenericRelationIterator *relIter = 0;
  Word argument;
  Supplier son;
 
  switch (message)
  {
   case OPEN :
   {
   qp->Open(args[0].addr);
   resultType = GetTupleResultType(s);
   resultTupleType = new TupleType(nl->Second(resultType));
   Stream<Tuple> stream(args[0]);

   son = qp->GetSupplier(args[1].addr, 0);
   qp->Request(son, argument);
   defEps = ((CcInt*)argument.addr)->GetIntval();

   son = qp->GetSupplier(args[1].addr, 1);
   qp->Request(son, argument);
   defMinPts = ((CcInt*)argument.addr)->GetIntval();

   stream.open();
   Tuple* tup;
   tp = 0;
   MaxMem = qp->FixedMemory();
   tp = new TupleBuffer(MaxMem);
   order = new TupleBuffer(MaxMem);
   relIter = 0;

   while( (tup = stream.request()) != 0)
   {
    Tuple *newTuple = new Tuple(resultTupleType);

    //Copy points from given tuple to the new tuple
    for( int i = 0; i < tup->GetNoAttributes(); i++ ) 
    {
     newTuple->CopyAttribute( i, tup, i );
    }

    //Initialize the result tuple with default values
    CcReal coreDist(-1.0);
    newTuple->PutAttribute( 1, ((Attribute*) &coreDist)->Clone());
    CcReal reachDist(-1.0);
    newTuple->PutAttribute( 2, ((Attribute*) &reachDist)->Clone());
    CcBool processed(true, false);
    newTuple->PutAttribute( 3, ((Attribute*) &processed)->Clone());
//    Point pointOrder(true, 1, 1);
//    newTuple->PutAttribute( 4, ((Attribute*) &pointOrder)->Clone());

    tp->AppendTuple(newTuple);
    tup->DeleteIfAllowed(); 
   }

   //Start the optics ordering
   optics.order(tp, defEps, defMinPts, order);
   
   //Initialize the tuple buffer iterator and store it in local
   relIter = order->MakeScan();   
   local.setAddr(relIter);
   return 0;
   }
   case REQUEST :
   {
   relIter = (GenericRelationIterator*) local.addr;
   Tuple* outTup;

   //Put the tuple in the result
   if((outTup = relIter->GetNextTuple()))
   { 
    outTup->DeleteIfAllowed();
    result.setAddr(outTup);
    return YIELD;
   }
   else
   {
    return CANCEL;
   }
   }
   case CLOSE :
   {
   return 0;
   }
  }
  return 0;
 } 
 
 struct opticsInfo : OperatorInfo
 {
  opticsInfo() : OperatorInfo()
  {
   name = "optics";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax = "_ optics [list]";
   meaning = "Order points to identify the cluster structure";
   example = "query Kneipen feed project [GeoData] optics \
     [9999999, 5] consume";
  }
 };
 
 class ClusterOpticsAlgebra : public Algebra
 {
  public:
   ClusterOpticsAlgebra() : Algebra()
   {
   AddOperator(opticsInfo(), opticsFun, opticsType);
   }

   ~ClusterOpticsAlgebra() {};
 };
}

extern "C"
 Algebra* InitializeOpticsAlgebra( NestedList* nlRef, QueryProcessor* qpRef)
 {
  nl = nlRef;
  qp = qpRef;

  return (new clusteropticsalg::ClusterOpticsAlgebra());
 }



