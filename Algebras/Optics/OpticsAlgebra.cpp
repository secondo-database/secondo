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
#include "MTreeConfig.h"
#include "Optics.h"
#include "OpticsM.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "MMRTreeAlgebra.h"

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
Struct ~opticsResult~ to save the ordering of algorithm.

*/
 struct opticsResult
 {
  TupleBuffer* buffer; 
  list<TupleId>* tupleIds;
  list<TupleId>::iterator it;
  bool init;

  opticsResult(TupleBuffer* buf)
  {
   buffer = buf;
   tupleIds = new list<TupleId>;
   init = false;
  }

  ~opticsResult() { delete tupleIds; }

  void initialize() { it = tupleIds->begin(); init = true; }

  TupleId next() { TupleId tid = *it; *it++;  return tid; }
   
  bool hasNext() { return init && (it != tupleIds->end()); }
 };
/*
Type mapping method ~opticsTM~

*/
 ListExpr opticsTM( ListExpr args )
 {
  if(!DistfunReg::isInitialized())
  {
   DistfunReg::initialize();
  }
    
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two element expected");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
     return listutils::typeError("stream(Tuple) expected");
  }

  //Check the arguments
  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=3)
  {
   ErrorReporter::ReportError("non conform list (three arguments expected)");
   return nl->TypeError();
  }
  
  if(!CcReal::checkType(nl->Second(arguments)))
  {
     return listutils::typeError("arg2 is not a real (Eps)");
  }
  
  if(!CcInt::checkType(nl->Third(arguments)))
  {
     return listutils::typeError("arg3 is not an int (MinPts)");
  }
  
  //Check the attribute name, is it in the tuple list
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is not a member of the tuple");
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

  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("CoreDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("ReachDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Processed")
    ,nl->SymbolAtom(CcBool::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Eps")
    ,nl->SymbolAtom(CcReal::BasicType())));

   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND())
   ,nl->TwoElemList(nl->IntAtom(found-1), nl->StringAtom(typeName))
   ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
    ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
     ,newAttrList)));
 }
/*
Value mapping method ~opticsVM~

*/
 int opticsVM(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  opticsResult* info = (opticsResult*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    //Default values
    double defEps;
    int defMinPts;
    int attrCnt;
    int idxData;
    string type;
    string distFun;
    string data;
    string config;
        
    //Optics instance
    Optics<0> optics;

    Tuple* tup;
    TupleType* resultTupleType;
    ListExpr resultType;
    TupleBuffer* tp;
    long maxMem;
    Word argument;
    Supplier son;
    mtreeAlgebra::MTree mtree;
    
    qp->Open(args[0].addr);
    
    //set the result type of the tuple
    resultType = GetTupleResultType(s);
    resultTupleType = new TupleType(nl->Second(resultType));
    Stream<Tuple> stream(args[0]);
    
    //set the given eps
    son = qp->GetSupplier(args[1].addr, 1);
    qp->Request(son, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();
    
    //set the index of the attribute in the tuple
    idxData = static_cast<CcInt*>(args[2].addr)->GetIntval();

    //set the type of the attribute in the tuple
    type = static_cast<CcString*>(args[3].addr)->GetValue();

    string distfunName = DFUN_DEFAULT;
    string configName = mtreeAlgebra::CONFIG_DEFAULT;
    
    DistDataId id = DistDataReg::getId(type
     ,DistDataReg::defaultName(type));

    DistfunInfo df = DistfunReg::getInfo(distfunName, id);
    
    mtree.initialize(id, distfunName, configName);
    
    stream.open();
    
    maxMem = qp->FixedMemory();
    tp = new TupleBuffer(maxMem);
    
    if(!DistfunReg::isInitialized())
    {
     DistfunReg::initialize();
    }
    
    while( (tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);
     
     //Copy points from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i);
     }
 
     //Initialize the result tuple with default values
     CcReal coreDist(-1.0);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &coreDist)->Clone());
     CcReal reachDist(-1.0);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &reachDist)->Clone());
     CcBool processed(true, false);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &processed)->Clone());
     CcReal eps(defEps);
     newTuple->PutAttribute( attrCnt+3, ((Attribute*) &eps)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed();
    }
    
    stream.close();
    
    if(info)
    {
     delete info;
    }
    
    info = new opticsResult(tp);

    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     Attribute* attr = obj->GetAttribute(idxData);
     
     if(attr->IsDefined())
     {
      mtree.insert(attr, objId);
     }
    }

    optics.initialize(&mtree, tp, &df, idxData, attrCnt, attrCnt+1, attrCnt+2);
    
    //Start the optics ordering
    optics.order(defEps, defMinPts, info->tupleIds);
    
    info->initialize();
    local.setAddr(info);
    
    return 0;
   }
   case REQUEST :
   {
    if(info->hasNext())
    {
     TupleId tid = info->next();
     Tuple* outTup = info->buffer->GetTuple(tid, false);
     outTup->DeleteIfAllowed();
     result = SetWord(outTup);
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
Type mapping method ~opticsRTM~

*/
 ListExpr opticsRTM( ListExpr args )
 {    
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two element expected");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
     return listutils::typeError("stream(Tuple) expected");
  }

  //Check the arguments
  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=3)
  {
   ErrorReporter::ReportError("non conform list (three arguments expected)");
   return nl->TypeError();
  }
  
  if(!CcReal::checkType(nl->Second(arguments)))
  {
     return listutils::typeError("arg2 is not a real (Eps)");
  }
  
  if(!CcInt::checkType(nl->Third(arguments)))
  {
     return listutils::typeError("arg3 is not an int (MinPts)");
  }
  
  //Check the attribute name, is it in the tuple list
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is not a member of the tuple");
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

  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("CoreDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("ReachDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Processed")
    ,nl->SymbolAtom(CcBool::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Eps")
    ,nl->SymbolAtom(CcReal::BasicType())));

   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND())
   ,nl->OneElemList(nl->IntAtom(found-1))
   ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
   ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
     ,newAttrList)));
 }
/*
Value mapping method ~opticsRVM~

*/
 template <int dim>
 int opticsRVM(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  opticsResult* info = (opticsResult*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    //Default values
    double defEps = 0.0;
    int defMinPts = 0;
    int attrCnt   = 0;
    int idxData   = -1;
    int minLeafs  = 2;
    int maxLeafs  = 8;
        
    //Optics instance
    Optics<dim> optics;

    Tuple* tup;
    TupleType* resultTupleType;
    ListExpr resultType;
    TupleBuffer* tp;
    long maxMem;
    Word argument;
    Supplier son;
    
    qp->Open(args[0].addr);
    
    //set the result type of the tuple
    resultType = GetTupleResultType(s);
    resultTupleType = new TupleType(nl->Second(resultType));
    Stream<Tuple> stream(args[0]);
    
    //set the given eps
    son = qp->GetSupplier(args[1].addr, 1);
    qp->Request(son, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();
    
    //set the index of the attribute in the tuple
    idxData = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    stream.open();
    
    maxMem = qp->FixedMemory();
    tp = new TupleBuffer(maxMem);
    
    while((tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);
     
     //Copy data from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i);
     }
 
     //Initialize the result tuple with default values
     CcReal coreDist(-1.0);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &coreDist)->Clone());
     CcReal reachDist(-1.0);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &reachDist)->Clone());
     CcBool processed(true, false);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &processed)->Clone());
     CcReal eps(defEps);
     newTuple->PutAttribute( attrCnt+3, ((Attribute*) &eps)->Clone());
     
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed();
    }
    
    stream.close();
    
    if(info)
    {
     delete info;
    }
    
    info = new opticsResult(tp);
    
    mmrtree::RtreeT<dim, TupleId> rtree(minLeafs, maxLeafs);
        
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     Rectangle<dim>* attr = (Rectangle<dim>*) obj->GetAttribute(idxData);
     
     if(attr->IsDefined())
     {
      rtree.insert(*attr, objId);
     }
     
     obj->DeleteIfAllowed();
    }
    
    optics.initialize(&rtree, tp, idxData, attrCnt, attrCnt+1, attrCnt+2);
    
    //Start the optics ordering
    optics.order(defEps, defMinPts, info->tupleIds);
    
    info->initialize();
    local.setAddr(info);
    
    return 0;
   }
   case REQUEST :
   {
    if(info->hasNext())
    {
     TupleId tid = info->next();
     Tuple* outTup = info->buffer->GetTuple(tid, false);
     outTup->DeleteIfAllowed();
     result = SetWord(outTup);
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
Selection method for value mapping array ~opticsRRecSL~

*/
 int opticsRRecSL(ListExpr args)
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
/*
Value mapping array ~opticsRRecVM[]~

*/
 ValueMapping opticsRRecVM[] = 
 {
  opticsRVM<2>
 ,opticsRVM<3>
 ,opticsRVM<4>
 ,opticsRVM<8>
 };
/*
Type mapping method ~opticsMTM~

*/
 ListExpr opticsMTM( ListExpr args )
 {    
  if(nl->ListLength(args)!=2)
  {
   ErrorReporter::ReportError("two element expected");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args)))
  {
     return listutils::typeError("stream(Tuple) expected");
  }

  //Check the arguments
  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=3)
  {
   ErrorReporter::ReportError("non conform list (three arguments expected)");
   return nl->TypeError();
  }
  
  if(!CcReal::checkType(nl->Second(arguments)))
  {
     return listutils::typeError("arg2 is not a real (Eps)");
  }
  
  if(!CcInt::checkType(nl->Third(arguments)))
  {
     return listutils::typeError("arg3 is not an int (MinPts)");
  }
  
  //Check the attribute name, is it in the tuple list
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is not a member of the tuple");
   return nl->TypeError();
  }
  
  if( !CcInt::checkType(attrType)
   && !CcReal::checkType(attrType)
   && !Point::checkType(attrType)
   && !CcString::checkType(attrType) )
  {
     return listutils::typeError("Attribute " + attrName + " not of type " 
      + CcInt::BasicType() + ", " 
      + CcReal::BasicType() + ", " 
      + Point::BasicType() + " or " 
      + CcString::BasicType() );
  }
  
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

  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("CoreDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("ReachDist")
    ,nl->SymbolAtom(CcReal::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Processed")
    ,nl->SymbolAtom(CcBool::BasicType())));
  lastlistn = nl->Append(lastlistn
   ,nl->TwoElemList(nl->SymbolAtom("Eps")
    ,nl->SymbolAtom(CcReal::BasicType())));

   return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND())
   ,nl->OneElemList(nl->IntAtom(found-1))
   ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
   ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
     ,newAttrList)));
 }
/*
Value mapping method ~opticsMVM~

*/
 template <class T, class DistComp>
 int opticsMVM(Word* args, Word& result, int message, Word& local, Supplier s)
 {
  opticsResult* info = (opticsResult*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    //Default values
    double defEps = 0.0;
    int defMinPts = 0;
    int attrCnt   = 0;
    int idxData   = -1;
    int minLeafs = 2;
    int maxLeafs = 8;
        
    //Optics instance
    OpticsM<T, DistComp> optics;

    Tuple* tup;
    TupleType* resultTupleType;
    ListExpr resultType;
    TupleBuffer* tp;
    long maxMem;
    Word argument;
    Supplier son;
    MMMTree<pair<T, TupleId>, DistComp >* mtree;
    
    qp->Open(args[0].addr);
    
    //set the result type of the tuple
    resultType = GetTupleResultType(s);
    resultTupleType = new TupleType(nl->Second(resultType));
    Stream<Tuple> stream(args[0]);
    
    //set the given eps
    son = qp->GetSupplier(args[1].addr, 1);
    qp->Request(son, argument);
    defEps = ((CcReal*)argument.addr)->GetRealval();
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    defMinPts = ((CcInt*)argument.addr)->GetIntval();
    
    //set the index of the attribute in the tuple
    idxData = static_cast<CcInt*>(args[2].addr)->GetIntval();
    
    stream.open();
    
    maxMem = qp->FixedMemory();
    tp = new TupleBuffer(maxMem);
    int cou = 0;
    while((tup = stream.request()) != 0)
    {
     Tuple *newTuple = new Tuple(resultTupleType);
     
     //Copy data from given tuple to the new tuple
     attrCnt = tup->GetNoAttributes();
     for( int i = 0; i < attrCnt; i++ ) 
     {
      newTuple->CopyAttribute( i, tup, i);
     }
 
     //Initialize the result tuple with default values
     CcReal coreDist(-1.0);
     newTuple->PutAttribute( attrCnt, ((Attribute*) &coreDist)->Clone());
     CcReal reachDist(-1.0);
     newTuple->PutAttribute( attrCnt+1, ((Attribute*) &reachDist)->Clone());
     CcBool processed(true, false);
     newTuple->PutAttribute( attrCnt+2, ((Attribute*) &processed)->Clone());
     CcReal eps(defEps);
     newTuple->PutAttribute( attrCnt+3, ((Attribute*) &eps)->Clone());
     cou ++;
     tp->AppendTuple(newTuple);
     tup->DeleteIfAllowed();
    }
    
    stream.close();
    
    if(info)
    {
     delete info;
    }
    
    info = new opticsResult(tp);
    
    DistComp dc;
    mtree = new MMMTree<pair<T, TupleId>, DistComp >(minLeafs, maxLeafs, dc);
    
    GenericRelationIterator* relIter = tp->MakeScan();
    Tuple* obj;
    
    while((obj = relIter->GetNextTuple()))
    {
     TupleId objId = relIter->GetTupleId();
     T attr = (T) obj->GetAttribute(idxData);
     
     pair<T, TupleId> p(attr, objId);
     
     mtree->insert(p);
     
     obj->DeleteIfAllowed();
    }
    
    optics.initialize(mtree, tp, idxData, attrCnt, attrCnt+1, attrCnt+2);
    
    //Start the optics ordering
    optics.order(defEps, defMinPts, info->tupleIds);
    
    info->initialize();
    local.setAddr(info);
    
    return 0;
   }
   case REQUEST :
   {
    if(info->hasNext())
    {
     TupleId tid = info->next();
     Tuple* outTup = info->buffer->GetTuple(tid, false);
     outTup->DeleteIfAllowed();
     result = SetWord(outTup);
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
Selection method for value mapping array ~opticsRRecSL~

*/
int opticsMDisSL(ListExpr args)
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
 
 return -1; 
};
/*
Value mapping array ~opticsMDisVM[]~

*/
 ValueMapping opticsMDisVM[] = 
 {
  opticsMVM<CcInt*,IntDist>
 ,opticsMVM<CcReal*,RealDist>
 ,opticsMVM<Point*,PointDist>
 ,opticsMVM<CcString*,StringDist>
 };
/*
Struct ~opticsInfo~

*/
 struct opticsInfo : OperatorInfo
 {
   opticsInfo() : OperatorInfo()
   {
     name      = "optics";
     signature = "stream(Tuple) -> stream(Tuple)";
     syntax    = "_ optics [list]";
     meaning   = "Order points to identify the cluster structure";
     example   = "query Kneipen feed optics[GeoData, 1000.0, 5] consume";
   }
 };
/*
Struct ~opticsInfoR~

*/
 struct opticsInfoR : OperatorInfo
 {
   opticsInfoR() : OperatorInfo()
   {
     name      = "opticsR";
     signature = "stream(Tuple) -> stream(Tuple)";
     syntax    = "_ optics [list]";
     meaning   = "Order points to identify the cluster structure";
     example   = "query Kneipen feed extend[B : bbox(.GeoData)]" 
"opticsR[B, 1000.0, 5] consume";
   }
 };
/*
Struct ~opticsInfoM~

*/
 struct opticsInfoM : OperatorInfo
 {
   opticsInfoM() : OperatorInfo()
   {
     name      = "opticsM";
     signature = "stream(Tuple) -> stream(Tuple)";
     syntax    = "_ optics [list]";
     meaning   = "Order points to identify the cluster structure";
     example   = "query plz feed opticsM[PLZ, 10.0, 5] consume";
   }
 };
/*
Algebra class ~ClusterOpticsAlgebra~

*/
 class ClusterOpticsAlgebra : public Algebra
 {
  public:
   ClusterOpticsAlgebra() : Algebra()
   {
    AddOperator(opticsInfo(), opticsVM, opticsTM);
    AddOperator(opticsInfoR(), opticsRRecVM, opticsRRecSL, opticsRTM);
    AddOperator(opticsInfoM(), opticsMDisVM, opticsMDisSL, opticsMTM);
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



