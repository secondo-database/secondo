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
#include <list>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusterdbscanalg
{
 /* 
 Struct to save the result
 */
 struct DBScanCluster
 {
 double x;
 double y;
 int clusterID;
 };

 /* 
 --- Class declaration of Optics ---
 --- START --- 
 */
 class DBScan;

 class DBScan
 {
 public:
 const static int UNDEFINED = -1;
 const static int NOISE = -2;
 DBScan();
 void dbscan(std::list<DBScanCluster>& objs, int eps, int minPts);
 int nextId(){return ++id;};

 private:
 int id;
 bool expandCluster(std::list<DBScanCluster>& objs, DBScanCluster* obj
, int clusterId, int eps, int minPts);
 std::list<DBScanCluster*> regionQuery(std::list<DBScanCluster>& objs
, DBScanCluster* obj, int eps);
 void changeClustId(std::list<DBScanCluster*>& seeds, DBScanCluster* obj
, int clusterId);
 };
 /* 
 --- END --- 
 --- Class declaration of Optics ---
 */
 

 /* 
 --- Class definition of Optics ---
 --- START --- 
 */

 DBScan::DBScan()
 {
 id = 0;
 return;
 }

 void DBScan::dbscan(std::list<DBScanCluster>& objs, int eps, int minPts)
 {
 std::list<DBScanCluster>::iterator it = std::list<DBScanCluster>::iterator();

 int clusterId = nextId();
 
 for (it = objs.begin(); it != objs.end(); it++)
 {
 DBScanCluster* obj = &(*it);

 if(obj->clusterID == UNDEFINED)
 {
 if(expandCluster(objs, obj, clusterId, eps, minPts))
 {
 clusterId = nextId();
 }
 }
 }
 }

 bool DBScan::expandCluster(std::list<DBScanCluster>& objs, DBScanCluster* obj
, int clusterId, int eps, int minPts)
 {
 std::list<DBScanCluster*> seeds = regionQuery(objs, obj, eps);

 if(seeds.size() < minPts)
 {
 changeClustId(seeds, obj, NOISE);
 return false;
 }
 else
 {
 changeClustId(seeds, obj, clusterId);
 //--seeds.delete(Point);\n"); !!!MUSS NOCH IMPLEMENTIERT WERDEN!!!!!
 //Derzeit über regionQuery geloest (Punkt wird nicht
// in seeds eingefügt, deshalb changeClustId mit Uebergabeparameter obj
 std::list<DBScanCluster*>::iterator it = seeds.begin();
 
 while(!seeds.empty())
 {
 DBScanCluster* currentP = &(*(seeds.front()));

 std::list<DBScanCluster*> result = regionQuery(objs, currentP, eps);

 if(result.size() >= minPts)
 {
 std::list<DBScanCluster*>::iterator curIt;

 for (curIt = result.begin(); curIt != result.end(); curIt++)
 {
 DBScanCluster* resultP = *it;

 if(resultP->clusterID == UNDEFINED || resultP->clusterID == NOISE)
 {
 if(resultP->clusterID == UNDEFINED)
 {
 seeds.push_back(resultP);
 }

 resultP->clusterID = clusterId;
 }
 }
 }

 seeds.pop_front();
 }

 return true;
 }
 }
 
 std::list<DBScanCluster*> DBScan::regionQuery(std::list<DBScanCluster>& objs
, DBScanCluster* obj, int eps)
 {
 std::list<DBScanCluster*> near;
 std::list<DBScanCluster>::iterator it;

 it = objs.begin();
 for (it = objs.begin(); it != objs.end(); it++)
 {
 DBScanCluster* point = &(*it);
 
 if(obj->x != point->x && obj->y != point->y)
 {
 if( (obj->x - point->x) * (obj->x - point->x) 
 + (obj->y - point->y) * (obj->y - point->y) < eps*eps ) 
 {
 DBScanCluster* pPoint;// = new DBScanCluster;
 pPoint = &(*it);
 near.push_back(pPoint);
 }
 }
 }

 return near;
 }


 void DBScan::changeClustId(std::list<DBScanCluster*>& seeds
, DBScanCluster* obj, int clusterId)
 {
 std::list<DBScanCluster*>::iterator it;

 for (it = seeds.begin(); it != seeds.end(); it++)
 {
 DBScanCluster* point = *it; 
 point->clusterID = clusterId;
 }

 obj->clusterID = clusterId;
 }
 /* 
 --- END --- 
 --- Class definition of Optics ---
 */

 /* 
 --- Type mapping ---
 --- START --- 
 */
 static ListExpr dbscan_aType( ListExpr args )
 {
 if ( nl->ListLength(args) == 1 )
 {
 ListExpr arg1 = nl->First(args);

 if ( nl->IsEqual(arg1, Points::BasicType()) )
 {
 return nl->SymbolAtom(Points::BasicType());
 }
 }
 else if ( nl->ListLength(args) == 3 )
 {
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 ListExpr arg3 = nl->Third(args);

 if ( nl->IsEqual(arg1, CcInt::BasicType()) 
 && nl->IsEqual(arg2, CcInt::BasicType())
 && nl->IsEqual(arg3, Points::BasicType()) )
 {
 return nl->SymbolAtom(Points::BasicType());
 }
 }
 

 return nl->SymbolAtom(Symbol::TYPEERROR());
 }

 static ListExpr dbscan_bType( ListExpr args )
 {
 if ( nl->ListLength(args) == 3 )
 {
 ListExpr arg1 = nl->First(args);
 ListExpr arg2 = nl->Second(args);
 ListExpr arg3 = nl->Third(args);

 if ( nl->IsEqual(arg1, CcInt::BasicType()) 
 && nl->IsEqual(arg2, CcInt::BasicType())
 && nl->IsEqual(arg3, Points::BasicType()) )
 {
 return nl->SymbolAtom(Points::BasicType());
 }
 }
 

 return nl->SymbolAtom(Symbol::TYPEERROR());
 }

 ListExpr dbscanStrType( ListExpr args )
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

 ListExpr dbscanStrTplType( ListExpr args )
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
 int dbscan_aFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
 //Default values
 int defEps = 7;
 int defMinPts = 2;

 //DBScan instance
 DBScan cluster;

 //Points array to the initial points
 Points* ps = ((Points*)args[0].addr);
 
 std::list<DBScanCluster> allPoints;
 std::list<DBScanCluster>::iterator it;

 printf("--------- eps = %d MinPts = %d\n", defEps, defMinPts);
 printf("--------- INITIALIZE STRUCT WITH POINTS\n");
 for(int i = 0; i < ps->Size(); i++)
 {
 DBScanCluster* point = new DBScanCluster;
 Point obj;
 
 ps->Get(i, obj);
 point->x = obj.GetX();
 point->y = obj.GetY();
 point->clusterID = -1;
 allPoints.push_back(*point);

 printf("x = %g y = %g\n", obj.GetX(), obj.GetY());
 }
 printf("----------------------------------------------\n");

 //Start the dbscan clustering
 cluster.dbscan(allPoints, defEps, defMinPts);

 printf("--------- POINTS AFTER CLUSTERING\n");
 printf("ClusterID\tX\tY\n");
 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 DBScanCluster point = *it;
 printf("%d\t%g\t%g\n", point.clusterID, point.x, point.y);
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

 int dbscan_bFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
 //Default values
 int defEps = ((CcInt*)args[0].addr)->GetIntval();
 int defMinPts = ((CcInt*)args[1].addr)->GetIntval();

 //DBScan instance
 DBScan cluster;

 //Points array to the initial points
 Points* ps = ((Points*)args[2].addr);
 
 std::list<DBScanCluster> allPoints;
 std::list<DBScanCluster>::iterator it;

 printf("--------- eps = %d MinPts = %d\n", defEps, defMinPts);
 printf("--------- INITIALIZE STRUCT WITH POINTS\n");
 for(int i = 0; i < ps->Size(); i++)
 {
 DBScanCluster* point = new DBScanCluster;
 Point obj;
 
 ps->Get(i, obj);
 point->x = obj.GetX();
 point->y = obj.GetY();
 point->clusterID = -1;
 allPoints.push_back(*point);

 printf("x = %g y = %g\n", obj.GetX(), obj.GetY());
 }
 printf("----------------------------------------------\n");

 //Start the dbscan clustering
 cluster.dbscan(allPoints, defEps, defMinPts);

 printf("--------- POINTS AFTER CLUSTERING\n");
 printf("ClusterID\tX\tY\n");
 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 DBScanCluster point = *it;
 printf("%d\t%g\t%g\n", point.clusterID, point.x, point.y);
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

 int dbscanStrFun(Word* args, Word& result, int message, Word& local
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

 int dbscanStrTplFun(Word* args, Word& result, int message, Word& local
, Supplier s)
 {
 Stream<Tuple> stream(args[0]);
 stream.open();

 //Default values
 int defEps = 100;
 int defMinPts = 3;
 
 //DBScan instance
 DBScan cluster;

 std::list<DBScanCluster> allPoints;
 std::list<DBScanCluster>::iterator it;

 printf("--------- INITIALIZE STRUCT WITH POINTS\n");
 
 Tuple* tup;
 while( (tup = stream.request()) != 0)
 {
 
 Point* obj = (Point*) ((Tuple*) tup)->GetAttribute(0);

 DBScanCluster* point = new DBScanCluster;
 
 point->x = obj->GetX();
 point->y = obj->GetY();
 point->clusterID = -1;
 allPoints.push_back(*point);

 tup->DeleteIfAllowed(); 

 printf("x = %g y = %g\n", obj->GetX(), obj->GetY());
 }


 printf("----------------------------------------------\n");

 //Start the dbscan clustering
 cluster.dbscan(allPoints, defEps, defMinPts);

 printf("--------- POINTS AFTER CLUSTERING\n");
 printf("ClusterID\tX\tY\n");
 for (it=allPoints.begin(); it!=allPoints.end(); ++it)
 {
 DBScanCluster point = *it;
 printf("%d\t%g\t%g\n", point.clusterID, point.x, point.y);
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
 DBScanCluster point = *it;
 Point p(true, point.x, point.y);
 (*((Points*)result.addr)) += p;
 }

 ((Points*)result.addr)->EndBulkLoad();

 return 0;
/*
 switch(message)
 {
 case OPEN: 
 {
 qp->Open(args[0].addr);
 return 0;
 }
 case REQUEST: 
 {
 Word* tuple;
 qp->Request(args[0].addr, tuple);
 
 if ( qp->Received(args[0].addr) )
 {

 Point* obj = (Point*) ((Tuple*) tuple)->GetAttribute(0);

 printf("x = %g y = %g\n", obj->GetX(), obj->GetY());
 
 result = tuple;
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
 // should not happen 
 return -1;
 }
 }
*/
 }
 /* 
 --- ENDE --- 
 --- Value mapping ---
 */

 /* 
 --- Operator Info ---
 --- START --- 
 */
 struct dbscan_aInfo : OperatorInfo
 {
 dbscan_aInfo() : OperatorInfo()
 {
 name = "dbscan_a";
 signature = "???";
 syntax = "dbscan_a ( _ )";
 meaning = "query dbscan_a( [const points value ( (2.0 3.0) (3.0 2.0) \
 (10.0 11) (20.0 12.0) (23.0 11.0) (100.0 111.0) (111.0 105.0) \
 (30.0 3.0) (3.0 20.0))] )";
 }
 };

 struct dbscan_bInfo : OperatorInfo
 {
 dbscan_bInfo() : OperatorInfo()
 {
 name = "dbscan_b";
 signature = "arg0 = eps arg1 = MinPts arg3 = Points";
 syntax = "dbscan_b ( _, _, _ )";
 meaning = "query dbscan_b( 7 2 [const points value ( (2.0 3.0) \
 (3.0 2.0) \
 (10.0 11) (20.0 12.0) (23.0 11.0) (100.0 111.0) (111.0 105.0) \
 (30.0 3.0) (3.0 20.0))] )";
 }
 };

 struct dbscanStrInfo : OperatorInfo
 {
 dbscanStrInfo() : OperatorInfo()
 {
 name = "dbscanStr";
 signature = "???";
 syntax = "_ dbscanStr";
 meaning = "query intstream (1, 5) dbscanStr countintstream";
 }
 };

 struct dbscanStrTplInfo : OperatorInfo
 {
 dbscanStrTplInfo() : OperatorInfo()
 {
 name = "dbscanStrTpl";
 signature = "???";
 syntax = "_ dbscanStrTpl";
 meaning = "query Kneipen feed project [GeoData] dbscanStrTpl";
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
 class ClusterDBScanAlgebra : public Algebra
 {
 public:
 ClusterDBScanAlgebra() : Algebra()
 {
 AddOperator(dbscan_aInfo(), dbscan_aFun, dbscan_aType);
 AddOperator(dbscan_bInfo(), dbscan_bFun, dbscan_bType);
// AddOperator(dbscanStrInfo(), dbscanStrFun, dbscanStrType);
 AddOperator(dbscanStrTplInfo(), dbscanStrTplFun, dbscanStrTplType);
 }

 ~ClusterDBScanAlgebra() {};
 };
 /*
 --- ENDE ---
 --- Creating the cluster algebra ---
 */

}
 /*
 --- Initializing the cluster algebra ---
 --- START ---
 */
 extern "C"
 Algebra* InitializeDBScanAlgebra( NestedList* nlRef, QueryProcessor* qpRef)
 {
 nl = nlRef;
 qp = qpRef;

 return (new clusterdbscanalg::ClusterDBScanAlgebra());
 } 
 /*
 --- ENDE ---
 --- Initializing the cluster algebra ---
 */




