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

1 Source file "OpticsAlgebra.cpp"[4]

March-October 2014, Marius Haug

1.1 Overview

This file contains the implementation of the OpticsAlgebra.

1.2 Includes

*/
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "LogMsg.h"
#include "Stream.h"
#include "MTreeAlgebra.h"
#include "Optics.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"

#include <limits.h>
#include <float.h>
#include <iostream>
#include <string>
#include <algorithm>

extern NestedList* nl;
extern QueryProcessor* qp;

namespace clusteropticsalg
{
 /* 
 --- Type mapping ---
   --- START --- 
 */
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
//     return listutils::typeError("stream(Tuple(Points)) expected");
//   }

//  ListExpr sortSpecification = nl->Second(args);

//  int numberOfSortAttrs = nl->ListLength(sortSpecification);

//  if(numberOfSortAttrs != 2)
//  {
//   return listutils::typeError("eps and MinPts expected");
//  } 

  // copy attrlist to newattrlist
  ListExpr attrList  = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn  = newAttrList;

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
 /* 
   --- ENDE --- 
 --- Type mapping ---
 */

 /* 
 --- Value mapping ---
   --- START --- 
 */
 int optics1Fun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  //Default values
  int defEps;
  int defMinPts;
  
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
  mtreeAlgebra::MTree mtree;
  
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
    
    DistDataId id;
    string distfunName = DFUN_DEFAULT;
    string configName = "default";
  
    if(!DistfunReg::isInitialized())
    {
     DistfunReg::initialize();
    }
    
    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);

     
     //Copy points from given tuple to the new tuple
     for( int i = 0; i < tup->GetNoAttributes(); i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i);
     }
     

     //Initialize the result tuple with default values
     CcReal coreDist(-1.0);
     newTuple->PutAttribute( 1, ((Attribute*) &coreDist)->Clone());
     CcReal reachDist(-1.0);
     newTuple->PutAttribute( 2, ((Attribute*) &reachDist)->Clone());
     CcBool processed(true, false);
     newTuple->PutAttribute( 3, ((Attribute*) &processed)->Clone());
      
     DistDataAttribute* attr = (DistDataAttribute*) ((Point*) 
                               newTuple->GetAttribute(0));
     
     if(attr->IsDefined())
     {
      if (mtree.isInitialized())
      {
      /*
       if(attr->distdataId() != id)
       {
       
        printf("---- distdata of different types\n");
        return CANCEL;
        
       }
       */
      }
      else
      {      
       // initialize mtree
       id = attr->distdataId();
       DistDataInfo info = DistDataReg::getInfo(Point::BasicType()
                                               ,DDATA_NATIVE);
       string dataName = info.name();
       string typeName = info.typeName();
       
       if(distfunName == DFUN_DEFAULT)
       {
        distfunName = DistfunReg::defaultName(typeName);
       }
       
       if(!DistfunReg::isDefined(distfunName, typeName, dataName))
       {
printf("---- distance function %s for type %s not defined\n"
        , distfunName.c_str(), typeName.c_str());
        return CANCEL;
       }
        
       if(!DistfunReg::getInfo(distfunName, typeName, dataName).isMetric())
       {
printf("---- distance function %s with %s data for type %s is not metric\n"
, distfunName.c_str(), dataName.c_str(), typeName.c_str());
        return CANCEL;
       }
       
       mtree.initialize(info.id(), distfunName, configName);
      }
      
      mtree.insert(attr, newTuple->GetTupleId());
     }
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed();
    }

    optics.setQueryTree(&mtree);
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
 
 int opticsFun(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  //Default values
  int defEps;
  int defMinPts;
  
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
printf("\n----------------defMinPts = %d\n", defMinPts); 
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
//     Point pointOrder(true, 1, 1);
//     newTuple->PutAttribute( 4, ((Attribute*) &pointOrder)->Clone());

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
 /* 
   --- ENDE --- 
 --- Value mapping ---
 */


 /* 
 --- Operator Info ---
   --- START --- 
 */
 struct opticsInfo : OperatorInfo
 {
   opticsInfo() : OperatorInfo()
   {
     name   = "optics";
     signature = "stream(Tuple) -> stream(Tuple)";
     syntax  = "_ optics [list]";
     meaning  = "Order points to identify the cluster structure";
     example  = "query Kneipen feed project [GeoData] optics \
            [9999999, 5] consume";
   }
 };
 
 /* 
 struct optics1Info : OperatorInfo
 {
   optics1Info() : OperatorInfo()
   {
     name   = "optics1";
     signature = "stream(Tuple) -> stream(Tuple)";
     syntax  = "_ optics1 [list]";
     meaning  = "Order points to identify the cluster structure";
     example  = "query Kneipen feed project [GeoData] optics1 \
            [9999999, 5] consume";
   }
 };
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
    AddOperator(opticsInfo(), opticsFun, opticsType);
    //AddOperator(optics1Info(), optics1Fun, opticsType);
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



