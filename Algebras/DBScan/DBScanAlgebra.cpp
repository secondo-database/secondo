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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#include "Stream.h"
#include "Sort.h"
#include "MTreeAlgebra.h"
#include "MTreeConfig.h"
#include "MTree.h"
#include "DBScan.h"
#include "DBScanDC.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <iterator>



extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusterdbscanalg
{
  


/*
Type mapping method ~dbscanType~ MTree

*/
 ListExpr dbscanType( ListExpr args )
 {
  if(!DistfunReg::isInitialized())
  {
   DistfunReg::initialize();
  }
 
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

  if(nl->ListLength(arguments)!=4)
  {
   ErrorReporter::ReportError("non conform list of cluster attribut, \
   attribute name as cluster ID, \
            Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcReal::checkType(nl->Third(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }


//Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is no member of the tuple");
   return nl->TypeError();
  }


  string dataName;
  string typeName = nl->ToString(attrType);
  dataName = DistDataReg::defaultName(typeName);
  
  if(!DistDataReg::isDefined(typeName, dataName))
  {
   ErrorReporter::ReportError("Attribute type "
    + typeName + " is not supported");
   return nl->TypeError();
  }

  ListExpr typeList;

  // check functions
  //set<string> usedNames;
  
   ListExpr name = nl->Second(arguments); 
  
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

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList)))
  {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
      
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("XKoord"),
      nl->SymbolAtom(CcReal::BasicType()) ));


  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->TwoElemList(nl->IntAtom(found-1), nl->StringAtom(typeName))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }


/*
Value mapping method ~dbscanFun~ MTree

*/
 int dbscanFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  GenericRelationIterator *relIter = 0;
  
  switch (message)
  {
   case OPEN :
   {
    Word argument;
    Supplier supplier;
    TupleType *resultTupleType;
    ListExpr resultType;
    TupleBuffer *tp;
    long MaxMem;
    double defEps    = 0;
    int defMinPts = 0;
    int idxClusterAttr = -1;
    int attrCnt = 0;
    string type;
    string distFun;
    string data;
    string config;
    mtreeAlgebra::MTree mtree;
   
    qp->Open(args[0].addr);
    resultType = GetTupleResultType( s );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    Stream<Tuple> stream(args[0]);
      
    Tuple* tup;
    tp = 0;
    MaxMem = qp->FixedMemory();
    tp = new TupleBuffer(MaxMem);
    relIter = 0;

    supplier = qp->GetSupplier(args[1].addr, 2);
    qp->Request(supplier, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();

    supplier = qp->GetSupplier(args[1].addr, 3);
    qp->Request(supplier, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();

    idxClusterAttr = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    //set the type of the attribute in the tuple
    type = static_cast<CcString*>(args[3].addr)->GetValue();
    
    string distfunName = DFUN_DEFAULT;
    string configName = mtreeAlgebra::CONFIG_DEFAULT;
    
    DistDataId id = DistDataReg::getId(type
     ,DistDataReg::defaultName(type));

    DistfunInfo df = DistfunReg::getInfo(distfunName, id);
    
    mtree.initialize(id, distfunName, configName);
    
    stream.open();

    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);

     //Copy points from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) //tup->GetNoAttributes(); i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i );
     }
 
     //Initialize the result tuple with default values
     CcInt clusterID(-1);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &clusterID)->Clone());
  
     CcBool visited(true, false);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &visited)->Clone());
     
     CcReal xKoord(-1.0);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &xKoord)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
    }
    
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     Attribute* attr = obj->GetAttribute(idxClusterAttr);
     
     if(attr->IsDefined())
     {
      mtree.insert(attr, objId);
     }
    }

    DBScan<0> cluster; 
    cluster.clusterAlgo(&mtree, tp, defEps, defMinPts, idxClusterAttr, 
        attrCnt, attrCnt+1);

    relIter = tp->MakeScan();
    local.setAddr( relIter );

    return 0;
   }
   case REQUEST :
   {
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
   }
   case CLOSE :
   return 0;
   }
   return 0;
 } 
/*
Type mapping method ~dbscanType~ RTree

*/
 ListExpr dbscanTypeRT( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two elements expected."
           " Stream and argument list");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=4)
  {
   ErrorReporter::ReportError("non conform list of cluster attribut, "
    "attribute name as cluster ID, Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcReal::checkType(nl->Third(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }


//Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is no member of the tuple");
   return nl->TypeError();
  }

  if( !Rectangle<2>::checkType(attrType)
   && !Rectangle<3>::checkType(attrType)
   && !Rectangle<4>::checkType(attrType)
   && !Rectangle<8>::checkType(attrType) )
  {
     return listutils::typeError("Attribute " + attrName + " not of type " 
      + Rectangle<2>::BasicType() + ", " 
      + Rectangle<3>::BasicType() + ", " 
      + Rectangle<4>::BasicType() + " or " 
      + Rectangle<8>::BasicType() );
  }  

  ListExpr typeList;

  // check functions
  //set<string> usedNames;
  
   ListExpr name = nl->Second(arguments); 
  
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

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList)))
  {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
      
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("XKoord"),
      nl->SymbolAtom(CcReal::BasicType()) ));


  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }


/*
Value mapping method ~dbscanRT~ RTree

*/
 template <int dim>
 int dbscanRT(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  GenericRelationIterator *relIter = 0;
  
  switch (message)
  {
   case OPEN :
   {
    Word argument;
    Supplier supplier;
    TupleType *resultTupleType;
    ListExpr resultType;
    TupleBuffer *tp;
    long MaxMem;
    double defEps    = 0;
    int defMinPts = 0;
    int idxClusterAttr = -1;
    int attrCnt = 0;
    int minLeafs  = 2;
    int maxLeafs  = 8;


   
    qp->Open(args[0].addr);
    resultType = GetTupleResultType( s );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    Stream<Tuple> stream(args[0]);
      
    Tuple* tup;
    tp = 0;
    MaxMem = qp->FixedMemory();
    tp = new TupleBuffer(MaxMem);
    relIter = 0;

    supplier = qp->GetSupplier(args[1].addr, 2);
    qp->Request(supplier, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();

    supplier = qp->GetSupplier(args[1].addr, 3);
    qp->Request(supplier, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();

    idxClusterAttr = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    
    mmrtree::RtreeT<dim, TupleId> rtree(minLeafs, maxLeafs);
    
    stream.open();

    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);

     //Copy points from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) //tup->GetNoAttributes(); i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i );
     }
 
     //Initialize the result tuple with default values
     CcInt clusterID(-1);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &clusterID)->Clone());
  
     CcBool visited(true, false);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &visited)->Clone());
     
     CcReal xKoord(-1.0);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &xKoord)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
    }
    
    stream.close();
    
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     Rectangle<dim>* attr = (Rectangle<dim>*) obj->GetAttribute(idxClusterAttr);
     
     if(attr->IsDefined())
     {
      rtree.insert(*attr, objId);
     }
     obj->DeleteIfAllowed();
    }

    DBScan<dim> cluster; 
    cluster.clusterAlgo(&rtree, tp, defEps, defMinPts, idxClusterAttr, 
        attrCnt, attrCnt+1);

    relIter = tp->MakeScan();
    local.setAddr( relIter );

    return 0;
   }
   case REQUEST :
   {
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
   }
   case CLOSE :
   return 0;
   }
   return 0;
 }
 


/*
Type mapping method ~dbscanTypeDC~ Divide and Conquer

*/
 ListExpr dbscanTypeDC( ListExpr args )
 {
 
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two elements expected.  \
   Stream and argument list");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=4)
  {
   ErrorReporter::ReportError("non conform list of cluster attribut, \
   attribute name as cluster ID, \
            Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcReal::checkType(nl->Third(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }


//Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is no member of the tuple");
   return nl->TypeError();
  }


  ListExpr typeList;

  // check functions
  //set<string> usedNames;
  
   ListExpr name = nl->Second(arguments); 
  
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

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList)))
  {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
      
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("XKoord"),
      nl->SymbolAtom(CcReal::BasicType()) ));


  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }


/*
Value mapping method ~dbscanFunDC~ Divide and Conquer

*/

 int dbscanFunDC(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  GenericRelationIterator *relIter = 0;
  
  switch (message)
  {
   case OPEN :
   {
    Word argument;
    Supplier supplier;
    TupleType *resultTupleType;
    ListExpr resultType;
    TupleBuffer *tp;
    long MaxMem;
    double defEps    = 0;
    int defMinPts = 0;
    int idxClusterAttr = -1;
    int attrCnt = 0;
    int minLeafs  = 2;
    int maxLeafs  = 8;
    DBScanDC cluster; 
    int countDC = 0;
   


   
    qp->Open(args[0].addr);
    resultType = GetTupleResultType( s );
    resultTupleType = new TupleType( nl->Second( resultType ) );

    Stream<Tuple> stream(args[0]);
      
    Tuple* tup;
    tp = 0;
    MaxMem = qp->FixedMemory();
    tp = new TupleBuffer(MaxMem);
    relIter = 0;

    supplier = qp->GetSupplier(args[1].addr, 2);
    qp->Request(supplier, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();

    supplier = qp->GetSupplier(args[1].addr, 3);
    qp->Request(supplier, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();

    idxClusterAttr = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    
    //mmrtree::RtreeT<dim, TupleId> rtree(minLeafs, maxLeafs);
    
    stream.open();

    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);

     //Copy points from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) //tup->GetNoAttributes(); i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i );
     }
 
     //Initialize the result tuple with default values
     CcInt clusterID(-1);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &clusterID)->Clone());
  
     CcBool visited(true, false);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &visited)->Clone());
     
     CcReal xKoord(-1.0);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &xKoord)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
    }
    
    stream.close();
    
    
    
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
      std::list<TupleId>* tupleIDsleft(new list<TupleId>);
      std::list<TupleId>* tupleIDsright(new list<TupleId>);
printf("-----------------   vor while  ---- \n");    
    while((obj = relIter->GetNextTuple()) )
    {
  printf("-----------------  TupleId objId = relIter->GetTupleId();  ---- \n"); 
     TupleId objId = relIter->GetTupleId();

printf("-----------------   if( countDC <= defMinPts +1 ) ---- \n");      
     if( countDC <= defMinPts +1 )
     {
     printf("-----------------    tupleIDsleft->push_back(objId); ---- \n"); 
      tupleIDsleft->push_back(objId);
      printf("----------------- obj->DeleteIfAllowed(); ---- \n"); 
      obj->DeleteIfAllowed();
      printf("-----------------  countDC++;---- \n"); 
      countDC++;
     }
     else
     {
printf("-----------------   cluster.clusterAlgo(tp  ---- \n");
        cluster.clusterAlgo(tp, defEps, defMinPts, idxClusterAttr, 
        attrCnt, attrCnt+1, tupleIDsleft);
        
        tupleIDsleft->clear();
        tupleIDsleft->push_back(objId);
        countDC = 1;
     }
    }

   
    
    
    
    //cluster.clusterAlgo(tp, defEps, defMinPts, idxClusterAttr, 
     //   attrCnt, attrCnt+1, );

    relIter = tp->MakeScan();
    local.setAddr( relIter );

    return 0;
   }
   case REQUEST :
   {
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
   }
   case CLOSE :
   return 0;
   }
   return 0;
 }



 struct dbscanInfo :  OperatorInfo
 {
  dbscanInfo() : OperatorInfo()
  {
   name      = "dbscan_";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscan_ [list]";
   meaning   = "Detects cluster from a given stream.";
   example   = "query Kneipen feed dbscan_ \
       [GeoData, No, 1000, 5] consume";
  }
 };
 
 struct dbscanInfoRT :  OperatorInfo
 {
  dbscanInfoRT() : OperatorInfo()
  {
   name      = "dbscanRT";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanRT [list]";
   meaning   = "Detects cluster from a given stream.";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)] dbscanRT \
       [B, No, 1000, 5] consume";
  }
 };
 
 struct dbscanInfoDC :  OperatorInfo
 {
  dbscanInfoDC() : OperatorInfo()
  {
   name      = "dbscanDC";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanDC [list]";
   meaning   = "Detects cluster from a given stream.";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)] dbscanRT \
       [B, No, 1000, 5] consume";
  }
 };
 
 
/*
Selection method ~opticsRRecSL~

*/
 int dbscanRRecSL(ListExpr args)
 {
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  assert(found > 0);
   
  if(Rectangle<2>::checkType(attrType))
  {
   return 0;
  }
  else if(Rectangle<3>::checkType(attrType))
  {
   return 1;
  }
  else if(Rectangle<4>::checkType(attrType))
  {
   return 2;
  }
  else if(Rectangle<8>::checkType(attrType))
  {
   return 3;
  }
   
  return -1;
 }
 
 ValueMapping dbscanRRecVM[] = 
 {
    dbscanRT<2>
   ,dbscanRT<3>
   ,dbscanRT<4>
   ,dbscanRT<8>
 };

 class ClusterDBScanAlgebra : public Algebra
 {
  public:
   ClusterDBScanAlgebra() : Algebra()
   {
    AddOperator(dbscanInfo(), dbscanFun, dbscanType);
    AddOperator(dbscanInfoRT(), dbscanRRecVM, dbscanRRecSL, dbscanTypeRT);
    AddOperator(dbscanInfoDC(), dbscanFunDC, dbscanTypeDC);
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





