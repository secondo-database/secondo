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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]

[1] Implementation of the Cluster Algebra

June, 2006.
Basic functionality, one operator with default values and one
with maximal distance and minimal number of points as values.
Only the type 'points' has been implemented so far.

[TOC]

1 Overview

This implementation file essentially contains the implementation of the
classes ~ClusterAlgebra~ and ~DBccan~ which contains the actual
cluster algorithm.

2 Defines and Includes

Eps is used for the clusteralgorithm as the maximal distance, the
minimum points (MinPts) may be apart. If there are further points
in the Eps-range to one of the points in the cluster, this point
(and further points from this on) belong to the same cluster.

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

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusteropticsalg
{
 /*
 Struct to save the result
 */
 struct OpticsOrder
 {
 bool processed;
 double x;
 double y;
 double distReach;
 double distCore;
 };

 /* 
 --- Class declaration of Optics ---
 --- START --- 
 */
 class Optics;

 class Optics
 {
 public:
 const static int UNDEFINED = UINT_MAX;
 Optics();
 void order(std::list<OpticsOrder>& objs, int eps, int minPts);

 private:
 void expandClusterOrder(std::list<OpticsOrder>& objs
, OpticsOrder* obj, int eps, int minPts);
 std::list<OpticsOrder*> getNeighbors(std::list<OpticsOrder>& objs
, OpticsOrder* obj, int eps);
 void setCoreDistance(std::list<OpticsOrder*>& objs, OpticsOrder* obj
, int eps, int minPts);
 void update(std::list<OpticsOrder*>& objs, OpticsOrder* obj
, std::list<OpticsOrder*>& orderedSeeds);
 double getReachableDist(OpticsOrder* obj, OpticsOrder* neighbor);
 };
 /* 
 --- END --- 
 --- Class declaration of Optics ---
 */
 

 /* 
 --- Class definition of Optics ---
 --- START --- 
 */
 Optics::Optics()
 {
 return;
 }

 void Optics::order(std::list<OpticsOrder>& objs, int eps, int minPts)
 {
 //OrderedFile.open();
 std::list<OpticsOrder>::iterator it = std::list<OpticsOrder>::iterator();

 for (it = objs.begin(); it != objs.end(); it++)
 {
 OpticsOrder* obj = &(*it);

 if(!obj->processed)
 {
 expandClusterOrder(objs, obj, eps, minPts);
 }
 }

 //OrderedFile.close();
 }

 void Optics::expandClusterOrder(std::list<OpticsOrder>& objs, OpticsOrder* obj
, int eps, int minPts)
 {
 std::list<OpticsOrder*> orderedSeeds;
 
 std::list<OpticsOrder*> neighbors = getNeighbors(objs, obj, eps);

 obj->processed = true;
 obj->distReach = UNDEFINED;

 setCoreDistance(neighbors, obj, eps, minPts);
 //orderedOut add the Point obj

 if(obj->distCore != UNDEFINED)
 {
 update(neighbors, obj, orderedSeeds);

 std::list<OpticsOrder*>::iterator it;
 for (it = orderedSeeds.begin(); it != orderedSeeds.end(); it++)
 {
 OpticsOrder* currentObject = &(*(*it));

 std::list<OpticsOrder*> neighbors = getNeighbors(objs, currentObject, eps);
 currentObject->processed = true;
 setCoreDistance(neighbors, currentObject, eps, minPts);
 //orderedOut add the Point currentObject

 if(currentObject->distCore != UNDEFINED)
 {
 update(neighbors, currentObject, orderedSeeds);
 }
 }
 }
 }
 
 std::list<OpticsOrder*> Optics::getNeighbors(std::list<OpticsOrder>& objs
, OpticsOrder* obj, int eps)
 {
 std::list<OpticsOrder*> near;
 std::list<OpticsOrder>::iterator it;

 for (it = objs.begin(); it != objs.end(); it++)
 {
 OpticsOrder* point = &(*it);
 if(obj->x != point->x || obj->y != point->y)
 {
 if( (obj->x - point->x) * (obj->x - point->x) 
 + (obj->y - point->y) * (obj->y - point->y) < eps*eps ) 
 {
 //OpticsOrder* pPoint;
 //pPoint = &(*it);
 near.push_back(&(*it));
 }
 }
 }

/* printf("--------- INSERTED AFTER REGION QUERY\n");
 std::list<OpticsOrder*>::iterator it2;
 for (it2=near.begin(); it2!=near.end(); it2++)
 {
 OpticsOrder* point;
 point = &(*(*it2)); 
 printf("Processed = %d\n", point->processed);
 printf("DistReach = %g\n", point->distReach);
 printf("DistCore = %g\n", point->distCore);
 printf("X = %g\n", point->x);
 printf("Y = %g\n", point->y);
 }
 printf("----------------------------------------------\n");*/
 return near;
 }


 void Optics::setCoreDistance(std::list<OpticsOrder*>& neighbors
, OpticsOrder* obj, int eps, int minPts)
 {
 double curDis = UNDEFINED;
 double lastDist = UNDEFINED;
 double coreDist = UNDEFINED;
 int count = 0;
 int biggest = 0;
 OpticsOrder* nearest[minPts];

 std::list<OpticsOrder*>::iterator it;
 for (it = neighbors.begin(); it != neighbors.end(); it++)
 {
 OpticsOrder* neighbor = &(*(*it));

 if(count < minPts)
 {
 nearest[count++] = neighbor;
 double curDist = sqrt(((obj->x - neighbor->x) * (obj->x - neighbor->x))
 + ((obj->y - neighbor->y) * (obj->y - neighbor->y)));

 if(curDist > lastDist)
 {
 biggest = count-1;
 coreDist = curDist;
 }

 double lastDist = curDist;
 }
 else
 {

 double curDist = sqrt(((obj->x - neighbor->x) * (obj->x - neighbor->x))
 + ((obj->y - neighbor->y) * (obj->y - neighbor->y)));

 for (int i = 0; i < count; i++)
 {
 OpticsOrder* curNear = nearest[i];
 double distance = sqrt(((obj->x - curNear->x) * (obj->x - curNear->x)) 
+ ((obj->y - curNear->y) * (obj->y - curNear->y)));

 if(distance > lastDist)
 {
 biggest = i;
 coreDist = distance;
 }

 double lastDist = distance;
 }

 OpticsOrder* curNear = nearest[biggest];
 double distance = sqrt(((obj->x - curNear->x) * (obj->x - curNear->x)) 
+ ((obj->y - curNear->y) * (obj->y - curNear->y)));

 if(distance > curDist)
 {
 nearest[biggest] = neighbor;
 }
 }
 }

 obj->distCore = coreDist;
 }

 void Optics::update(std::list<OpticsOrder*>& neighbors, OpticsOrder* center
, std::list<OpticsOrder*>& orderedSeeds)
 {
 //c_dist := CenterObject.core_distance;
 double coreDist = center->distCore;
 //FORALL Object FROM neighbors DO
 std::list<OpticsOrder*>::iterator it = neighbors.begin();
 for (it = neighbors.begin(); it != neighbors.end(); it++)
 {
 OpticsOrder* obj = *it;

 //IF NOT Object.Processed THEN 
 if(!obj->processed)
 {
 //new_r_dist:=max(c_dist,CenterObject.dist(Object));
 double newReachDist = getReachableDist(center, obj);

 //IF Object.reachability_distance=UNDEFINED THEN
 if(obj->distReach == UNDEFINED)
 {
 //Object.reachability_distance := new_r_dist;
 obj->distReach = newReachDist;
 //insert(Object, new_r_dist) !!!MUSS NOCH IMPLEMENTIERT WERDEN!!!
 orderedSeeds.push_back(*it);

 }
 //ELSE IF new_r_dist<Object.reachability_distance THEN
 else if(newReachDist < obj->distReach)// Object already in OrderSeeds
 {
 //Object.reachability_distance := new_r_dist;
 obj->distReach = newReachDist;
 //decrease(Object, new_r_dist) !!!MUSS NOCH IMPLEMENTIERT WERDEN
 }
 }
 }
 }


 double Optics::getReachableDist(OpticsOrder* obj, OpticsOrder* neighbor)
 {
 double distance = sqrt(((obj->x - neighbor->x) * (obj->x - neighbor->x))
 + ((obj->y - neighbor->y) * (obj->y - neighbor->y)));

 //new_r_dist:=max(c_dist,CenterObject.dist(Object)); 
 return obj->distCore > distance ? obj->distCore : distance;
 }
 /* 
 --- END --- 
 --- Class definition of Optics ---
 */

 /* 
 --- Type mapping ---
 --- START --- 
 */
 static ListExpr optics_aType( ListExpr args )
 {
 if ( nl->ListLength(args) == 1 )
 {
 ListExpr arg1 = nl->First(args);

 if ( nl->IsEqual(arg1, Points::BasicType()) )
 {
 return nl->SymbolAtom(Points::BasicType());
 }
 }

 return nl->SymbolAtom(Symbol::TYPEERROR());
 }

 ListExpr opticsStrType( ListExpr args )
 {
 if(!nl->HasLength(args,1))
 {
 return listutils::typeError("One argument expected");
 }

 if(!Stream<CcInt>::checkType(nl->First(args)))
 {
 return listutils::typeError("stream(int) expected");
 }

 return nl->TwoElemList(nl->SymbolAtom(Stream<CcInt>::BasicType())
 ,nl->SymbolAtom(CcInt::BasicType()));
 }

 ListExpr opticsStrTplType( ListExpr args )
 {
 if(!nl->HasLength(args, 1))
 {
 return listutils::typeError("One argument expected");
 }

 if(!Stream<Tuple>::checkType(nl->First(args)))
 {
 return listutils::typeError("stream(Tuple) expected");
 }

// return nl->TwoElemList(nl->SymbolAtom(Stream<Tuple>::BasicType())
// ,nl->SymbolAtom(Tuple::BasicType()));

// return nl->Cons(nl->SymbolAtom(Symbol::STREAM()), nl->Rest(nl->First(args)));
 return nl->SymbolAtom(Points::BasicType());
 }

 /* 
 --- ENDE --- 
 --- Type mapping ---
 */

 /* 
 --- Value mapping ---
 --- START --- 
 */
 int optics_aFun (Word* args, Word& result, int message, Word& local
, Supplier s)
 {
 //Default values
 int defEps = 9999999;
 int defMinPts = 2;
 
 //Optics instance
 Optics optics;

 //Points array to the initial points
 Points* ps = ((Points*)args[0].addr);
 
 std::list<OpticsOrder> allPoints;
 std::list<OpticsOrder>::iterator it;

 printf("--------- eps = %d MinPts = %d\n", defEps, defMinPts);
 printf("--------- INITIALIZE STRUCT WITH POINTS\n");
 printf("Processed\tDistReach\tDistCore\tX\tY\n");
 for(int i = 0; i < ps->Size(); i++)
 {
 OpticsOrder* point = new OpticsOrder;
 Point obj;
 
 ps->Get(i, obj);
 point->x = obj.GetX();
 point->y = obj.GetY();
 point->distReach = -1;
 point->distCore = -1;
 point->processed = false;
 allPoints.push_back(*point);

 printf("%d\t%g\t%g\t%g\t%g\n", point->processed, point->distReach
, point->distCore, point->x, point->y);
 }
 printf("----------------------------------------------\n");

 //Start the optics ordering
 optics.order(allPoints, defEps, defMinPts);

 printf("--------- POINTS AFTER CLUSTERING\n");
 printf("Processed\tDistReach\tDistCore\tX\tY\n");
 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 OpticsOrder* point = &(*it);
 printf("%d\t%g\t%g\t%g\t%g\n", point->processed, point->distReach
, point->distCore, point->x, point->y);
 }
 printf("----------------------------------------------\n");

 // --- JUST FOR OUT --- 
 result = qp->ResultStorage( s ); // Query Processor provided Points
 
 //instance for the result
 // copy x/y from cluster array back into result (only cluster members)
 ((Points*)result.addr)->Clear();
 ((Points*)result.addr)->StartBulkLoad();

 for(int a = 0; a < ps->Size(); a++)
 {
 Point obj;
 ps->Get(a, obj);
 Point p(true, obj.GetX(), obj.GetY());
 (*((Points*)result.addr)) += p;
 }

 ((Points*)result.addr)->EndBulkLoad();

 return 0;
 }

 int opticsStrFun(Word* args, Word& result, int message, Word& local
, Supplier s)
 {
 switch(message)
 {
 case OPEN: 
 {
 qp->Open(args[0].addr);
 return 0;
 }
 case REQUEST: 
 {
 Word elem(Address(0));
 qp->Request(args[0].addr, elem);
 
 if ( qp->Received(args[0].addr) )
 {
 cout << static_cast<CcInt*>(elem.addr)->GetIntval() << endl;
 result = elem;
 return YIELD;
 }
 else
 {
 result.addr = 0;
 return CANCEL;
 }
 }
 case CLOSE: 
 {
 qp->Close(args[0].addr);
 return 0;
 }
 default: 
 {
 /* should not happen */
 return -1;
 }
 }
 }

 int opticsStrTplFun(Word* args, Word& result, int message, Word& local
, Supplier s)
 {
 Stream<Tuple> stream(args[0]);
 stream.open();

 //Default values
 int defEps = 9999999;
 int defMinPts = 2;
 
 //Optics instance
 Optics optics;


 std::list<OpticsOrder> allPoints;
 std::list<OpticsOrder>::iterator it;

 printf("--------- eps = %d MinPts = %d\n", defEps, defMinPts);
 printf("--------- INITIALIZE STRUCT WITH POINTS\n");
 printf("Processed\tDistReach\tDistCore\tX\tY\n");
 Tuple* tup;
 while( (tup = stream.request()) != 0)
 {
 Point* obj = (Point*) ((Tuple*) tup)->GetAttribute(0);

 OpticsOrder* point = new OpticsOrder;
 
 point->x = obj->GetX();
 point->y = obj->GetY();
 point->distReach = -1;
 point->distCore = -1;
 point->processed = false;
 allPoints.push_back(*point);

 tup->DeleteIfAllowed(); 

 printf("%d\t%g\t%g\t%g\t%g\n", point->processed, point->distReach
, point->distCore, point->x, point->y);
 }


 printf("----------------------------------------------\n");

 //Start the optics ordering
 optics.order(allPoints, defEps, defMinPts);

 printf("--------- POINTS AFTER CLUSTERING\n");
 printf("Processed\tDistReach\tDistCore\tX\tY\n");
 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 OpticsOrder point = *it;
 printf("%d\t%g\t%g\t%g\t%g\n", point.processed, point.distReach
, point.distCore, point.x, point.y);
 }
 printf("----------------------------------------------\n");


 stream.close();


 // --- JUST FOR OUT --- 
 result = qp->ResultStorage( s ); // Query Processor provided Points
 
 //instance for the result
 // copy x/y from cluster array back into result (only cluster members)
 ((Points*)result.addr)->Clear();
 ((Points*)result.addr)->StartBulkLoad();

 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 Point obj;
 OpticsOrder point = *it;
 Point p(true, point.x, point.y);
 (*((Points*)result.addr)) += p;
 }

 ((Points*)result.addr)->EndBulkLoad();

 return 0;

 }
 /* 
 --- ENDE --- 
 --- Value mapping ---
 */


 /* 
 --- Operator Info ---
 --- START --- 
 */
 struct optics_aInfo : OperatorInfo
 {
 optics_aInfo() : OperatorInfo()
 {
 name = "optics_a";
 signature = "???";
 syntax = "optics_a ( _ )";
 meaning = "query optics_a( [const points value ( (2.0 3.0) \
 (3.0 2.0) (10.0 11))] )";
 }
 };

 struct opticsStrInfo : OperatorInfo
 {
 opticsStrInfo() : OperatorInfo()
 {
 name = "opticsStr";
 signature = "???";
 syntax = "_ opticsStr";
 meaning = "query intstream (1, 5) opticsStr countintstream";
 }
 };

 struct opticsStrTplInfo : OperatorInfo
 {
 opticsStrTplInfo() : OperatorInfo()
 {
 name = "opticsStrTpl";
 signature = "???";
 syntax = "_ opticsStrTpl";
 meaning = "query Kneipen feed project [GeoData] opticsStrTpl";
 }
 };
 /* 
 --- ENDE --- 
 --- Operator Info ---
 */

 /*
 --- Creating the cluster algebra ---
 --- START ---
 */
 class ClusterOpticsAlgebra : public Algebra
 {
 public:
 ClusterOpticsAlgebra() : Algebra()
 {
 AddOperator(optics_aInfo(), optics_aFun, optics_aType);
// AddOperator(opticsStrInfo(), opticsStrFun, opticsStrType);
 AddOperator(opticsStrTplInfo(), opticsStrTplFun, opticsStrTplType);
 }

 ~ClusterOpticsAlgebra() {};
 };
 /*
 --- ENDE ---
 --- Creating the cluster algebra ---
 */
}

extern "C"
 Algebra* InitializeOpticsAlgebra( NestedList* nlRef, QueryProcessor* qpRef)
 {
 nl = nlRef;
 qp = qpRef;

 return (new clusteropticsalg::ClusterOpticsAlgebra());
 }



