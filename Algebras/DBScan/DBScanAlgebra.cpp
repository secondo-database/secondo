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
 
 class DBScan;

 class DBScan
 {
  public:
   const static int UNDEFINED = -1;
   const static int NOISE   = -2;

   const static int PNT = 0;
   const static int CID = 1;
   const static int VIS = 2;

   DBScan();  
   void clusterAlgo(TupleBuffer* objs, int eps, int minPts);
   int nextId(){return ++id;};

  private:
   int id;
   bool expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
          int eps, int minPts);
   std::list<Tuple*> regionQuery(TupleBuffer* objs, Tuple* obj, int eps);
   void changeClustId(std::list<Tuple*>& seeds, Tuple* obj, 
          int clusterId);
 };

 DBScan::DBScan()
 {
  id = 0;
  return;
 }
 
 void DBScan::clusterAlgo(TupleBuffer* objs, int eps, int minPts)
 {
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* obj;

  int clusterId = 0;
  
  while((obj = relIter->GetNextTuple()))
  {
   if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
   {
    CcBool visited(true, true);
    obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

    std::list<Tuple*> N = regionQuery(objs, obj, eps);
    int nSize = N.size();    

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
     
     std::list<Tuple*>::iterator it;

     for (it = N.begin(); it != N.end(); it++)
     {
      Tuple* point = *it;
      
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

 bool DBScan::expandCluster(TupleBuffer* objs, Tuple* obj, int clusterId, 
          int eps, int minPts)
 {

  CcInt* distI = new CcInt;
  distI->Set(clusterId);
  obj->PutAttribute( CID, ((Attribute*)distI)->Clone() );

  if( !((CcBool*)obj->GetAttribute(VIS))->GetValue() )
  {
   CcBool visited(true, true);
   obj->PutAttribute(VIS, ((Attribute*) &visited)->Clone());

   std::list<Tuple*> N = regionQuery(objs, obj, eps);

   int nSize = N.size();
   
   if(nSize >= minPts) 
   { 
    std::list<Tuple*>::iterator it;

    for (it = N.begin(); it != N.end(); it++)
    {
     Tuple* point = *it;
      
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
 

 std::list<Tuple*> DBScan::regionQuery(TupleBuffer* objs, Tuple* obj, int eps)
 {
  GenericRelationIterator *relIter = objs->MakeScan();
  Tuple* point;

  std::list<Tuple*> near;

  while((point = relIter->GetNextTuple()))
  {   
   //if(((Point*)obj->GetAttribute(0))->GetX() != ((Point*)point
   //->GetAttribute(0))->GetX() && ((Point*)obj->GetAttribute(0))->GetY() 
   //!= ((Point*)point->GetAttribute(0))->GetY())
    //if(! (Point*)obj == (Point*)point)
    {
     if( ( ((Point*)obj->GetAttribute(0))->GetX() - 
       ((Point*)point->GetAttribute(0))->GetX()) * 
       (((Point*)obj->GetAttribute(0))->GetX() - 
       ((Point*)point->GetAttribute(0))->GetX()) + 
       (((Point*)obj->GetAttribute(0))->GetY() - 
       ((Point*)point->GetAttribute(0))->GetY()) * 
       (((Point*)obj->GetAttribute(0))->GetY() - 
       ((Point*)point->GetAttribute(0))->GetY()) < eps*eps ) 
     {
      Tuple* pPoint;
      pPoint = &(*point);
      near.push_back(pPoint);
     }
    }
   }
  return near;
 }


 void DBScan::changeClustId(std::list<Tuple*>& seeds, Tuple* obj, 
          int clusterId)
 {
  std::list<Tuple*>::iterator it;

  for (it = seeds.begin(); it != seeds.end(); it++)
  {
   
   Tuple* point = *it; 
   CcInt* distI = new CcInt;
   distI->Set(clusterId);
   point->PutAttribute( 1, ((Attribute*)distI)->Clone() );  
   
  }
  
  CcInt* distI = new CcInt;
  distI->Set(clusterId);
  obj->PutAttribute( 1, ((Attribute*)distI)->Clone() ); 
 }



 ListExpr dbscanType( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two elements expected. \
            Stream and argument list");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
     return listutils::typeError("first argument is not stream(Tuple)");
   }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=3)
  {
   ErrorReporter::ReportError("non conform list of attribute name, \
            Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcInt::checkType(nl->Second(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Third(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }

  // copy attrlist to newattrlist
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList)); 
  ListExpr lastlistn = newAttrList; 

  // reset attrList
  attrList = nl->Second(nl->Second(stream)); //.No
  ListExpr typeList;


  // check functions
  set<string> usedNames;
  
  ListExpr name = nl->First(arguments); 
  
  string errormsg;
  if(!listutils::isValidAttributeName(name, errormsg)){
   return listutils::typeError(errormsg);
  }//endif

  string namestr = nl->SymbolValue(name);
  int pos = FindAttribute(attrList,namestr,typeList);
  if(pos!=0)
  {
    ErrorReporter::ReportError("Attribute "+ namestr +
                  " already member of the tuple");
    return nl->TypeError();
  }//endif


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));

  nl->First(arguments);
  arguments = nl->Rest(arguments);

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
    ,nl->TwoElemList(nl->IntAtom(2), arguments)
    ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
            ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                    ,newAttrList)));

  //return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
   //    nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),newAttrList));

 }




 int dbscanFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  Word argument;
  //Tuple* tup;
  Supplier supplier;//, supplier2, supplier3;
  //int nooffun;
  //ArgVectorPointer funargs;
  TupleType *resultTupleType;
  ListExpr resultType;
  TupleBuffer *tp;
  long MaxMem;
  GenericRelationIterator *relIter = 0;
  int defEps  = 0;
  int defMinPts = 0;

   switch (message)
   {
   case OPEN :
   {
    qp->Open(args[0].addr);
    resultType = GetTupleResultType( s );
    resultTupleType = new TupleType( nl->Second( resultType ) );


    Stream<Tuple> stream(args[0]);
     stream.open();
    Tuple* tup;
    tp = 0;
    MaxMem = qp->FixedMemory();
    tp = new TupleBuffer(MaxMem);
    relIter = 0;

    supplier = qp->GetSupplier(args[1].addr, 1);
    qp->Request(supplier, argument);
    defEps = ((CcInt*)argument.addr)->GetIntval();

    supplier = qp->GetSupplier(args[1].addr, 2);
    qp->Request(supplier, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();

    while( (tup = stream.request()) != 0)
    {
  
     Tuple *newTuple = new Tuple(resultTupleType);

     //Copy points from given tuple to the new tuple
     for( int i = 0; i < 1; i++ ) //tup->GetNoAttributes(); i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i );
     }
 
     //Initialize the result tuple with default values
     CcInt clusterID(-1);
     newTuple->PutAttribute( 1, ((Attribute*) &clusterID)->Clone());
  
     CcBool visited(true, false);
     newTuple->PutAttribute( 2, ((Attribute*) &visited)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
    }

    DBScan cluster; 
    cluster.clusterAlgo(tp, defEps, defMinPts);

    relIter = tp->MakeScan();
    local.setAddr( relIter );

    return 0;
   }
    case REQUEST :

   relIter = (GenericRelationIterator *)local.addr;

   Tuple* curtup;

   if((curtup = relIter->GetNextTuple()))
   { 
    curtup->DeleteIfAllowed();
    result.setAddr(curtup);
    return YIELD;
   }
   else
   {
    return CANCEL;
   }

    case CLOSE :
    return 0;
   }
   return 0;
 }



 struct dbscanInfo : OperatorInfo
 {
  dbscanInfo() : OperatorInfo()
  {
   name   = "dbscan_";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax  = "_ dbscan_ [list]";
   meaning  = "Detects cluster from a given stream.";
   example  = "query Kneipen feed project [GeoData] dbscan_ \
       [No, 1000, 5] consume";
  }
 };
 

 class ClusterDBScanAlgebra : public Algebra
 {
  public:
   ClusterDBScanAlgebra() : Algebra()
   {
    AddOperator(dbscanInfo(), dbscanFun, dbscanType);
   }

   ~ClusterDBScanAlgebra() {};
 };

} 
 extern "C"
 Algebra* InitializeDBScanAlgebra( NestedList* nlRef, QueryProcessor* qpRef)
 {
  nl = nlRef;
  qp = qpRef;

  return (new clusterdbscanalg::ClusterDBScanAlgebra());
 } 
 




