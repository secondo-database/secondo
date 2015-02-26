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

1 Source file "DBScanAlgebra.cpp"[4]

March-October 2014, Natalie Jaeckel

1.1 Overview

This file contains the implementation of the DBScanAlgebra.

1.2 Includes

*/


#include "SetOfObjectsR.h"
#include "DBScanGen.h"

#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "Stream.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "DBScanMT.h"
#include "DistFunction.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <utility>


extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusterdbscanalg
{

/*
Type mapping method ~dbscanType~ MMR-Tree

*/
 ListExpr dbscanTypeRT( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two elements expected. "
    "Stream and argument list");
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

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
                                    
 }




/*
Value mapping method ~dbscanRT~ MMR-Tree

*/

struct dbscanLI{
  TupleType* tt;
  TupleBuffer* tp;
  GenericRelationIterator* relIter;
};

 template <int dim>
 int dbscanRT(Word* args, Word& result, int message, Word& local, Supplier s)
 {

  typedef clusterdbscanalg::RectDist<dim> Dist;
  typedef  dbscan::DBScanGen< 
                   dbscan::SetOfObjectsR <dim,Dist >, 
                   Dist > 
           dbscanclass;
  dbscanclass* li = (dbscanclass*) local.addr;  
  switch (message)
  {
   case OPEN :
   {
    // arg0 : stream
    Word stream = args[0];
    Supplier supplier = qp->GetSupplier(args[1].addr, 2);
    Word argument;
    qp->Request(supplier, argument);
    CcReal* eps = ((CcReal*)argument.addr);
    supplier = qp->GetSupplier(args[1].addr, 3);
    qp->Request(supplier, argument);
    CcInt* minPts = ((CcInt*)argument.addr);
    int cid = ((CcInt*)args[2].addr)->GetValue();
    ListExpr resultType = GetTupleResultType( s );
    ListExpr tt = ( nl->Second( resultType ) );
    if(li) { 
        delete li;
        local.addr=0;
    }
    size_t maxMem = (qp->GetMemorySize(s) * 1024);
    if(!eps->IsDefined() || eps->GetValue() < 0){
      return 0;
    }
    if(!minPts->IsDefined() || minPts->GetValue() < 0){
      return 0;
    }
   
    Dist dist; 

    local.addr = new dbscanclass(stream,tt, 
                                 eps->GetValue(),
                                 minPts->GetValue(),
                                 maxMem, 
                                 cid, dist);
    return 0;
  } 
  case REQUEST:
     result.addr= li?li->next():0;
     return result.addr?YIELD:CANCEL;
  case CLOSE:{
     if(li){
        delete li;
        local.addr=0;
     }
  }
  }
  return 0;
}
 
 
/*
Type mapping method ~dbscanTypeMT~ MMM-Tree

*/
 ListExpr dbscanTypeMT( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two elements expected. " 
            "Stream and argument list");
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

  if( !CcInt::checkType(attrType)
   && !CcReal::checkType(attrType)
   && !Point::checkType(attrType)
   && !CcString::checkType(attrType) 
   && !Picture::checkType(attrType) )
  {
   return listutils::typeError("Attribute " + attrName + " not of type " 
    + CcInt::BasicType() + ", " 
    + CcReal::BasicType() + ", " 
    + Point::BasicType() + ", " 
    + CcString::BasicType() + " or " 
    + Picture::BasicType() );
  }

  ListExpr typeList;

  // check functions
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
      
  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }




/*
Value mapping method ~dbscanFunMT~ MMM-Tree

*/
 template <class T, class DistComp>
 int dbscanFunMT(Word* args, Word& result, int message, Word& local, 
  Supplier s)
 {
  GenericRelationIterator *relIter = 0;
  
  switch (message)
  {
   case OPEN :
   {
    cout << "called open" << endl;

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
    DBScanMT<T, DistComp> dbscan;

   
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
    
    MMMTree<pair<T, TupleId>, DistComp >* mtree;
    
    stream.open();
    dbscan.no_Objects = 0;

    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);

     //Copy points from given tuple to the new tuple
     newTuple->SetTupleId(dbscan.no_Objects);
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
          
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
     newTuple->DeleteIfAllowed();
     dbscan.no_Objects++;
    }
    
    stream.close();

    
    DistComp dc;
    mtree = new MMMTree<pair<T, TupleId>, DistComp >(minLeafs, maxLeafs, dc);
    
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     T attr = (T) obj->GetAttribute(idxClusterAttr);
     
     pair<T, TupleId> p(attr, objId);
     mtree->insert(p);
     obj->DeleteIfAllowed();
    }
    delete relIter;



    dbscan.clusterAlgo(mtree, tp, defEps, defMinPts, idxClusterAttr, 
        attrCnt, attrCnt+1);

    
    relIter = tp->MakeScan();
    dbscanLI* li = new dbscanLI();
    li->tp = tp;
    li->tt = resultTupleType;
    li->relIter = relIter;
    local.setAddr( li );

    delete mtree;

    return 0;
   }
   case REQUEST :
   {
    dbscanLI* li = (dbscanLI*) local.addr;
    if(!li) return CANCEL;
    relIter = li->relIter;
    
    Tuple* curtup;
    
    if((curtup = relIter->GetNextTuple()))
    {  
     result.setAddr(curtup);
     return YIELD;
    }
    else
    {
     return CANCEL;
    }
   }
   case CLOSE :{
      dbscanLI* li = (dbscanLI*) local.addr;
      if(li){
         li->tt->DeleteIfAllowed();
         delete li->relIter;
         delete li->tp;
         delete li;
         local.addr=0;
      }
      
  
      return 0;
   }
   }
   return 0;
 }


/*
Type mapping method ~dbscanTypeMTF~ MMM-Tree with given distance function

*/
 ListExpr dbscanTypeMTF( ListExpr args )
 {
  if(nl->ListLength(args)!=2)
  {
   return listutils::typeError("two elements expected. " 
            "Stream and argument list");
  }
  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(stream))
  {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=5)
  {
   return listutils::typeError("non conform list of cluster attribut, "
    "attribute name as cluster ID, Eps and MinPts, distfun");
  }

  if(!CcReal::checkType(nl->Third(arguments)))
  {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments)))
  {
   return listutils::typeError("no numeric MinPts");
  }

  ListExpr fun = nl->Fifth(arguments);

  if(!listutils::isMap<2>(fun))
  {
   return listutils::typeError("arg4 is not a map with 2 arguments");
  }

  if(    !nl->Equal(nl->Second(fun), nl->Third(fun)) 
      || (   !CcReal::checkType(nl->Fourth(fun)) 
          && !CcInt::checkType(nl->Fourth(fun)) )) {
     return listutils::typeError("fun is not of type: T x T -> {int, real} ");
   
   }


  

  //Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  
  ListExpr clusterAttr = nl->First(arguments);
  if(nl->AtomType(clusterAttr)!=SymbolType){
    return listutils::typeError("First arg of the parameter list "
           + nl->ToString(clusterAttr) + "  is not "
           "a valid attribute name");
  }
  string attrName = nl->SymbolValue(clusterAttr);
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   return listutils::typeError("Attribute "
    + attrName + " is no member of the tuple");
  }

  if(!nl->Equal(attrType, nl->Second(fun))) {
   return listutils::typeError("Clustervalue type and function value type"
    "different");
  }

  ListExpr typeList;

  // check functions
  ListExpr name = nl->Second(arguments); 
  
  string errormsg;
  if(!listutils::isValidAttributeName(name, errormsg)){
   return listutils::typeError(errormsg);
  }//endif

  string namestr = nl->SymbolValue(name);
  int pos = FindAttribute(attrList,namestr,typeList);
  if(pos!=0)
  {
   return listutils::typeError("Attribute "+ namestr +
                              " already member of the tuple");
  }//endif
  pos = FindAttribute(attrList,"Visited",typeList);
  if(pos!=0)
  {
   return listutils::typeError("Attribute Visisted" 
                              " already member of the tuple");
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

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }
 
 

/*
Value mapping method ~dbscanFunMTF~ MMM-Tree with given distance function

*/
 template <class T, class DistComp>
 int dbscanFunMTF(Word* args, Word& result, int message, Word& local, 
  Supplier s)
 {

  GenericRelationIterator *relIter = 0;
  dbscanLI* li  = (dbscanLI*)local.addr;

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
    int minLeafs  = 4;
    int maxLeafs  = 10;
    DBScanMT<T, DistComp> dbscan;
   
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

    supplier = qp->GetSupplier(args[1].addr, 4);

    idxClusterAttr = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    MMMTree<pair<T, TupleId>, DistComp >* mtree;
    
    stream.open();

    TupleId id = 0;

    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);
     newTuple->SetTupleId(id++);
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
          
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed(); 
     newTuple->DeleteIfAllowed();
     dbscan.no_Objects++;
    }
    stream.close();
    
    DistComp dc;
    dc.initialize(qp, supplier);

    mtree = new MMMTree<pair<T, TupleId>, DistComp >(minLeafs, maxLeafs, dc);

    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;


    int count = 0;

    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     T attr = (T) obj->GetAttribute(idxClusterAttr);
     
     pair<T, TupleId> p(attr, objId);

     mtree->insert(p);
     count++;
     obj->DeleteIfAllowed();
    }

    delete relIter;
    mtree->getDistComp().reset();

    dbscan.clusterAlgo(mtree, tp, defEps, defMinPts, idxClusterAttr, 
        attrCnt, attrCnt+1);

    delete mtree;
    resultTupleType->DeleteIfAllowed();

    if(li){
      delete li->relIter;
      delete li->tp;
      delete li;
    }  
    relIter = tp->MakeScan();
    li = new dbscanLI();
    li->tt=0;
    li->tp=tp;
    li->relIter= relIter;
    local.setAddr( li );


    return 0;
   }


   case REQUEST :
   {
    if(!li){
      return CANCEL;
    }

    relIter = li->relIter;
    
    Tuple* curtup;
    
    if((curtup = relIter->GetNextTuple()))
    {  
     result.setAddr(curtup);
     return YIELD;
    }
    else
    {
     return CANCEL;
    }
   }
   case CLOSE :{
    if(li){
      delete li->relIter;
      delete li->tp;
      delete li;
      local.addr=0;
    }  
      
      return 0;
   }
   }
   return 0;
 }
 




/*
Struct ~dbscanInfoRT~

*/

 struct dbscanInfoRT :  OperatorInfo
 {
  dbscanInfoRT() : OperatorInfo()
  {
   name      = "dbscanR";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanR [_, _, _, _]";
   meaning   = "Detects cluster from a given stream using MMR-Tree as index "
   "structure. The first parameter has to be a bbox, the second parameter is "
   "the name for the cluster ID attribute, the  third paramter is eps and "
   "the fourth parameter is MinPts. A tuple stream will be returned but the "
   "tuple will have additional attributes as bbox, visited and clusterID";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)] dbscanRT "
       "[B, No, 1000.0, 5] consume";
  }
 };
 
 struct dbscanInfoRT2 :  OperatorInfo
 {
  dbscanInfoRT2() : OperatorInfo()
  {
   name      = "dbscanR2";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanR2 [_, _, _, _]";
   meaning   = "Detects cluster from a given stream using MMR-Tree as index "
   "structure. The first parameter has to be a bbox, the second parameter is "
   "the name for the cluster ID attribute, the  third paramter is eps and "
   "the fourth parameter is MinPts. A tuple stream will be returned but the "
   "tuple will have additional attributes as bbox, visited and clusterID";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)] dbscanRT "
       "[B, No, 1000.0, 5] consume";
  }
 };
 
  
/*
Struct ~dbscanInfoMT~

*/
 struct dbscanInfoMT :  OperatorInfo
 {
  dbscanInfoMT() : OperatorInfo()
  {
   name      = "dbscanM";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanM [_, _, _, _]";
   meaning   = "Detects cluster from a given stream using MMM-Tree as index "
   "structure. The first parameter is the attribute to cluster, the second "
   "parameter is the name for the cluster ID attribute, the  third paramter "
   "is eps and the fourth parameter is MinPts. A tuple stream will be returned "
   "but the tuple will have additional attributes as visited and clusterID";
   example   = "query Kneipen feed dbscanMT[GeoData, CID, 1000.0, 5] consume";
   
  }
 };
 
/*
Struct ~dbscanInfoMTF~

*/ 
 struct dbscanInfoMTF :  OperatorInfo
 {
  dbscanInfoMTF() : OperatorInfo()
  {
   name      = "dbscanF";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ dbscanF [_, _, _, _, fun]";
   meaning   = "Detects cluster from a given stream using MMM-Tree as index "
   "structure. The first parameter is the attribute to cluster, the second "
   "parameter is the name for the cluster ID attribute, the  third paramter "
   "is eps and the fourth parameter is MinPts. A tuple stream will be returned "
   "but the tuple will have additional attributes as visited "
   "and clusterID. In addition a distance function is required";
   example   = "query plz feed dbscanMT[PLZ, CID, 1000.0, 5, "
   "fun(i1: int, i2: int)i1 - i2] consume";  
  }
 };
 
 
/*
Selection method ~dbscanRRecSL~

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
 
 ValueMapping dbscanRTVM[] = 
 {
    dbscanRT<2>
   ,dbscanRT<3>
   ,dbscanRT<4>
   ,dbscanRT<8>
 };
 

/*
Selection method for value mapping array ~dbscanMTDisSL~

*/ 
 int dbscanMTDisSL(ListExpr args)
 {
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  assert(found > 0);
  
  if(CcInt::checkType(attrType))
  {
   return 0;
  }
  else if(CcReal::checkType(attrType))
  {
   return 1;
  }
  else if(Point::checkType(attrType))
  {
   return 2;
  }
  else if(CcString::checkType(attrType))
  {
   return 3;
  }
  else if(Picture::checkType(attrType))
  {
   return 4;
  }
  
  return -1; 
 };
 
/*
Value mapping array ~dbscanMTDisVM[]~

*/
 ValueMapping dbscanMTDisVM[] = 
 {
  dbscanFunMT<CcInt*, IntDist>
 ,dbscanFunMT<CcReal*, RealDist>
 ,dbscanFunMT<Point*, PointDist>
 ,dbscanFunMT<CcString*, StringDist>
 ,dbscanFunMT<Picture*, PictureDist>
 };
 
 
/*
Selection method for value mapping array ~dbscanMTFDisSL~

*/
 int dbscanMTFDisSL(ListExpr args)
 {
  ListExpr funResult= nl->Fourth(nl->Fifth(nl->Second(args)));
  if(CcInt::checkType(funResult)) return 0;
  if(CcReal::checkType(funResult)) return 1;
  return -1; 
 };
 
 
/*
Value mapping array ~dbscanMTFDisVM[]~

*/
 ValueMapping dbscanMTFDisVM[] = 
 {
  dbscanFunMTF<Attribute*, CustomDist<Attribute*, CcInt> >,
  dbscanFunMTF<Attribute*, CustomDist<Attribute*, CcReal> >
 };


/*
Algebra class ~ClusterDBScanAlgebra~

*/
 class ClusterDBScanAlgebra : public Algebra
 {
  public:
   ClusterDBScanAlgebra() : Algebra()
   {
    AddOperator(dbscanInfoRT(), dbscanRTVM, 
                                dbscanRRecSL, dbscanTypeRT)->SetUsesMemory();
    AddOperator(dbscanInfoMT(), dbscanMTDisVM, dbscanMTDisSL, dbscanTypeMT);

    AddOperator(dbscanInfoMTF(),dbscanMTFDisVM, dbscanMTFDisSL, dbscanTypeMTF);

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





