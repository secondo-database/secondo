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
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "DistFunction.h"

#include "OpticsGen.h"
#include "SetOfObjectsR.h"
#include "SetOfObjectsM.h"

#include "Symbols.h"

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
1 R-tree based variant


1.1 Type mapping method ~opticsRTM~

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
1.2 Value Mapping for the R-tree variant

*/
 template <int dim>
 int opticsRVM1(Word* args, Word& result, int message, Word& local, Supplier s)
 {

  typedef OpticsGen<Rectangle<dim>,  
                    SetOfObjectsR<RectDist<dim>, dim>,
                    RectDist<dim> > opticsclass;

   opticsclass* info = (opticsclass*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    if(info){
      delete info;
      local.addr=0;
    }
        
    //set the result type of the tuple
    ListExpr resultType = nl->Second(GetTupleResultType(s));
    //set the given eps
    Supplier son = qp->GetSupplier(args[1].addr, 1);
    Word argument; 
    qp->Request(son, argument);
    CcReal* Eps = ((CcReal*) argument.addr);
    if(!Eps->IsDefined()){
      return 0;
    }
    double eps = Eps->GetValue();
    if(eps <= 0) {
       return 0;
    }
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    CcInt* MinPts  = ((CcInt*)argument.addr);
    if(!MinPts->IsDefined()){
      return 0;
    }
    int minPts = MinPts->GetValue();
    if(minPts < 1){
       return 0;
    }
    //set the index of the attribute in the tuple
    int attrPos = static_cast<CcInt*>(args[2].addr)->GetIntval();
    size_t maxMem = qp->GetMemorySize(s)*1024*1024; 
    double UNDEFINED = -1.0;

    RectDist<dim> df;
    local.addr = new opticsclass(
                          args[0], attrPos, eps, minPts, 
                          UNDEFINED, resultType, maxMem, df);
    return 0;
   } 
   case REQUEST : {
     result.addr = info?info->next():0;
     return result.addr?YIELD:CANCEL;
   }
   case CLOSE : {
    if(info){
      delete info;
      local.addr = 0;
    }
    return 0;
   }
  }
  return 0;
}

/*
1.3 Selection method for value mapping array ~opticsRRecSL~

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
1.4 ValueMapping array

*/

 ValueMapping opticsRVM[] = 
 {
  opticsRVM1<2>,
  opticsRVM1<3>,
  opticsRVM1<4>,
  opticsRVM1<8>
 };


/*
1.5 Struct ~opticsInfoR~

*/

 struct opticsInfoR : OperatorInfo
 {
  opticsInfoR() : OperatorInfo()
  {
   name      = "opticsR";
   signature = "stream(Tuple) x attrName x double x int -> stream(Tuple)";
   syntax    = "_ opticsR[_, _, _]";
   meaning   = "This operator will ordering data to identify the cluster "
               "structure. The operator uses the MMRTree index structure. The "
               "first paramater has to be a stream of tuple, the second is the "
               "attribute for clustering, the third is eps and the fourth is "
               "MinPts. The return value is a stream of tuples."
               "The supported type to cluster is the bbox.";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)]" 
               "opticsR[B, 1000.0, 5] consume";
  }
 };



/*
2 Variant using an M-tree


2.1 Type mapping method ~opticsMTM~

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
   && !CcString::checkType(attrType)
   && !Picture::checkType(attrType) )
  {
   return listutils::typeError("Attribute " + attrName + " not of type " 
    + CcInt::BasicType() + ", " 
    + CcReal::BasicType() + ", " 
    + Point::BasicType() + " , " 
    + CcString::BasicType() + " or " 
    + Picture::BasicType() );
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
2.2 Value Mapping Function for M-tree variant

*/

 template <class T, class DistComp>
 int opticsMVM1(Word* args, Word& result, int message, Word& local, Supplier s)
 {

  typedef OpticsGen<T,  
                   SetOfObjectsM<DistComp,T>,
                   DistComp> opticsclass;

   opticsclass* info = (opticsclass*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    if(info){
      delete info;
      local.addr=0;
    }
        
    //set the result type of the tuple
    ListExpr resultType = nl->Second(GetTupleResultType(s));
    //set the given eps
    Supplier son = qp->GetSupplier(args[1].addr, 1);
    Word argument; 
    qp->Request(son, argument);
    CcReal* Eps = ((CcReal*) argument.addr);
    if(!Eps->IsDefined()){
      return 0;
    }
    double eps = Eps->GetValue();
    if(eps <= 0) {
       return 0;
    }
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    CcInt* MinPts  = ((CcInt*)argument.addr);
    if(!MinPts->IsDefined()){
      return 0;
    }
    int minPts = MinPts->GetValue();
    if(minPts < 1){
       return 0;
    }
    //set the index of the attribute in the tuple
    int attrPos = static_cast<CcInt*>(args[2].addr)->GetIntval();
    size_t maxMem = qp->GetMemorySize(s)*1024*1024; 
    double UNDEFINED = -1.0;

    DistComp df;
    local.addr = new opticsclass(
                          args[0], attrPos, eps, minPts, 
                          UNDEFINED, resultType, maxMem, df);
    return 0;
   } 
   case REQUEST : {
     result.addr = info?info->next():0;
     return result.addr?YIELD:CANCEL;
   }
   case CLOSE : {
    if(info){
      delete info;
      local.addr = 0;
    }
    return 0;
   }
  }
  return 0;
}




/*
2.3 Selection method for value mapping array ~opticsMSL~

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
  else if(Picture::checkType(attrType))
  {
   return 4;
  }
  
  return -1; 
 };

/*
2.4 Value mapping array ~opticsMVM[]~

*/

 ValueMapping opticsMVM[] = 
 {
  opticsMVM1<CcInt, IntDist>
 ,opticsMVM1<CcReal, RealDist>
 ,opticsMVM1<Point, PointDist>
 ,opticsMVM1<CcString, StringDist>
 ,opticsMVM1<Picture, PictureDist>
 };


/*
2.5 Struct ~opticsInfoM~

*/
 struct opticsInfoM : OperatorInfo
 {
  opticsInfoM() : OperatorInfo()
  {
   name      = "opticsM";
   signature = "stream(Tuple) -> stream(Tuple)";
   syntax    = "_ opticsM [_, _, _]";
   meaning   = "This operator will ordering data to identify the cluster "
               "structure. The operator uses the MMMTree index structure. The "
               "first paramater has to be a stream of tuple, the second is the "
               "attribute for clustering, the third is eps and the fourth is "
               "MinPts. The return value is a stream of tuples."
               "The supported types to cluster are point, picture, int, real "
               "and string.";
   example   = "query Kneipen feed opticsM[Name, 10.0, 5] consume";
  }
 };

/*
3 Variant using an M-tree and used defined functions

3.1 Type mapping method ~opticsFTM~

*/
 ListExpr opticsFTM( ListExpr args )
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

  if(nl->ListLength(arguments)!=4)
  {
   ErrorReporter::ReportError("non conform list (four arguments expected)");
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
  
  if(!listutils::isMap<2>(nl->Fourth(arguments)))
  {
   return listutils::typeError("arg4 is not a map");
  }
  ListExpr funres = nl->Fourth(nl->Fourth(arguments));
  if(!CcInt::checkType(funres) &&
     !CcReal::checkType(funres)){
    return listutils::typeError("function reault not of type int or real");
  }

  
  
  //Check the attribute name, is it in the tuple list
  ListExpr attrType;
  ListExpr attrList = nl->Second(nl->Second(stream));
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0)
  {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is not a member of the tuple");
   return nl->TypeError();
  }
  
  if(!nl->Equal(attrType, nl->Second(nl->Fourth(arguments))))
  {
   return listutils::typeError("Clustervalue type and function value type"
    "different");
  }
  
  if(!nl->Equal(attrType, nl->Third(nl->Fourth(arguments))))
  {
   return listutils::typeError("Clustervalue type and function value type"
    "different");
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
3.2 Value Mapping Function

*/

 template <class T, class DistComp>
 int opticsMFVM1(Word* args, Word& result, 
                  int message, Word& local, Supplier s)
 {

  typedef OpticsGen<T,  
                   SetOfObjectsM<DistComp,T>,
                   DistComp> opticsclass;

   opticsclass* info = (opticsclass*) local.addr;

  switch (message)
  {
   case OPEN :
   {
    if(info){
      delete info;
      local.addr=0;
    }
        
    //set the result type of the tuple
    ListExpr resultType = nl->Second(GetTupleResultType(s));
    //set the given eps
    Supplier son = qp->GetSupplier(args[1].addr, 1);
    Word argument; 
    qp->Request(son, argument);
    CcReal* Eps = ((CcReal*) argument.addr);
    if(!Eps->IsDefined()){
      return 0;
    }
    double eps = Eps->GetValue();
    if(eps <= 0) {
       return 0;
    }
    
    //set the given minPts
    son = qp->GetSupplier(args[1].addr, 2);
    qp->Request(son, argument);
    CcInt* MinPts  = ((CcInt*)argument.addr);
    if(!MinPts->IsDefined()){
      return 0;
    }
    int minPts = MinPts->GetValue();
    if(minPts < 1){
       return 0;
    }
    //set the index of the attribute in the tuple
    int attrPos = static_cast<CcInt*>(args[2].addr)->GetIntval();
    size_t maxMem = qp->GetMemorySize(s)*1024*1024; 
    double UNDEFINED = -1.0;

    son = qp->GetSupplier(args[1].addr, 3);
    DistComp df(qp,son);
    //df.initialize(qp, son);
    local.addr = new opticsclass(
                          args[0], attrPos, eps, minPts, 
                          UNDEFINED, resultType, maxMem, df);
    return 0;
   } 
   case REQUEST : {
     result.addr = info?info->next():0;
     return result.addr?YIELD:CANCEL;
   }
   case CLOSE : {
    if(info){
      delete info;
      local.addr = 0;
    }
    return 0;
   }
  }
  return 0;
}


/*
3.3 Selection method for value mapping array ~opticsFDisSL~

*/
 int opticsFDisSL(ListExpr args)
 {
  ListExpr funResType = nl->Fourth(nl->Fourth(nl->Second(args)));
  if(CcInt::checkType(funResType)) return 0;
  if(CcReal::checkType(funResType)) return 1;
  
  return -1; 
 };

/*
3.4 Value mapping array ~opticsFVM[]~

*/

 ValueMapping opticsFVM[] = 
 {
  opticsMFVM1<Attribute, CustomDist<Attribute*, CcInt> >,
  opticsMFVM1<Attribute, CustomDist<Attribute*, CcReal> >
 };


/*
3.5 Struct ~opticsInfoF~

*/

 struct opticsInfoF : OperatorInfo
 {
  opticsInfoF() : OperatorInfo()
  {
   name      = "opticsF";
   signature = "stream(Tuple) x Id x real x int x fun -> stream(Tuple)";
   syntax    = "_ opticsF [_, _, _, fun]";
   meaning   = "This operator will ordering data to identify the cluster "
               "structure. The operator uses the MMMTree index structure. The "
               "first paramater has to be a stream of tuple, the second is the "
               "attribute for clustering, the third is eps, the fourth is "
               "MinPts and the sixth is the distance function. The return "
               "value is a stream of tuples."
               "The supported types to cluster are point, picture, int, real "
               "and string.";
   example   = "query plz feed opticsF[PLZ, 10.0, 5, fun(i1: int, i2: int)"
               "i1 - i2] consume";
   }
 };


/*
4 Operator ~extractDbScan~

4.1 Type Mapping

*/
ListExpr extractDbScanTM(ListExpr args){

  if(!nl->HasLength(args,2)){
     return listutils::typeError("two arguments expected");
  }
  ListExpr stream = nl->First(args);
  if(!Stream<Tuple>::checkType(stream)){
     return listutils::typeError("first argument must be a tuple stream");
  }
  if(!CcReal::checkType(nl->Second(args))){
     return listutils::typeError("first argument must be a real");
  }
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr type;
  int coreDistPos = listutils::findAttribute(attrList, "CoreDist", type); 
  if(!coreDistPos){
     return listutils::typeError("Attribute CoreDist not member of the stream");
  }
  if(!CcReal::checkType(type)){
     return listutils::typeError("Attribute CoreDist not of type real");
  }
  int reachDistPos = listutils::findAttribute(attrList, "ReachDist", type); 
  if(!reachDistPos){
     return listutils::typeError("Attribute reachDist not "
                                  "member of the stream");
  }
  if(!CcReal::checkType(type)){
     return listutils::typeError("Attribute reachDist not of type real");
  }
  
  int EpsPos = listutils::findAttribute(attrList, "Eps", type); 
  if(!EpsPos){
     return listutils::typeError("Attribute Eps not member of the stream");
  }
  if(!CcReal::checkType(type)){
     return listutils::typeError("Attribute Eps not of type real");
  }
  int cidPos = listutils::findAttribute(attrList,"Cid", type);
  if(cidPos){
     return listutils::typeError("Attribute Cid already "
                                 "part of the attributes");
  }  
 


  ListExpr newAttr = nl->OneElemList( 
               nl->TwoElemList( nl->SymbolAtom("Cid"),
                                listutils::basicSymbol<CcInt>()));
  ListExpr newAttrList = listutils::concat(attrList, newAttr);
  ListExpr appendList = nl->ThreeElemList( nl->IntAtom(coreDistPos-1), 
                                           nl->IntAtom(reachDistPos-1), 
                                           nl->IntAtom(EpsPos-1));
  return nl->ThreeElemList(
                 nl->SymbolAtom(Symbols::APPEND()),
                 appendList,
                 nl->TwoElemList( listutils::basicSymbol<Stream<Tuple> >(),
                                  nl->TwoElemList( 
                                         listutils::basicSymbol<Tuple>(),
                                          newAttrList)));
}

/*
4.2 LocalInfo

*/

class extractLocal{

  public:
     extractLocal(Word _stream, double _eps, int _coreDistPos, 
                  int _reachDistPos, int _epsPos, ListExpr type):
       stream(_stream), eps(_eps), coreDistPos(_coreDistPos), 
       reachDistPos(_reachDistPos), epsPos(_epsPos), id(0){
       tt = new TupleType(type);
       stream.open();
     }

     ~extractLocal(){
         stream.close();
         tt->DeleteIfAllowed();
      }

      Tuple* next(){
         Tuple* inTuple = stream.request();
         if(!inTuple){
            return 0;
         }
         Tuple* resTuple = new Tuple(tt);
         int attrCnt = inTuple->GetNoAttributes();
         // copy attributes to resTuple
         for(int i=0;i<attrCnt; i++){
            resTuple->CopyAttribute(i,inTuple,i);
         }
         inTuple->DeleteIfAllowed();
         CcReal* Eps = (CcReal*) inTuple->GetAttribute(epsPos);
         CcReal* ReachDist = (CcReal*) inTuple->GetAttribute(reachDistPos);
         CcReal* CoreDist = (CcReal*) inTuple->GetAttribute(coreDistPos);
         // optics never creates undefined attributes.
         // undefined is simulated by a value < 0
         if(    !Eps->IsDefined() || !ReachDist->IsDefined()
             || !CoreDist->IsDefined()){
            resTuple->PutAttribute(attrCnt, new CcInt(false,0));
            return resTuple;
         }
         double oldEps = Eps->GetValue();
         if(eps > oldEps){  // check condition
            resTuple->PutAttribute(attrCnt, new CcInt(false,0));
            return resTuple;
         }
         double reachDist = ReachDist->GetValue();
         double coreDist = CoreDist->GetValue();
         if((reachDist > eps) || (reachDist < 0)){
            if((coreDist <= eps) && !(coreDist < 0)){
               // start new cluster
               id++;
               resTuple->PutAttribute(attrCnt, new CcInt(true,id));  
            } else {
               // mark as noise
               resTuple->PutAttribute(attrCnt, new CcInt(true, -2));
            }
         } else {
            resTuple->PutAttribute(attrCnt, new CcInt(true,id));  
         }
         return resTuple; 
      }


  private:
     Stream<Tuple> stream;
     double eps;
     int coreDistPos;
     int reachDistPos;
     int epsPos;
     TupleType* tt; 
     int id;
};

/*
4.3 Value Mapping

*/

 int extractDbScanVM(Word* args, Word& result, 
                     int message, Word& local, Supplier s) {
   extractLocal* info = (extractLocal*) local.addr;
   switch(message){
      case OPEN: {
            if(info) {
               delete info;
               local.addr=0;
            }
            int coreDistPos = ((CcInt*)args[2].addr)->GetValue();
            int reachDistPos = ((CcInt*)args[3].addr)->GetValue();
            int epsPos = ((CcInt*)args[4].addr)->GetValue();
            ListExpr type = nl->Second(GetTupleResultType(s));
            CcReal* Eps = (CcReal*) args[1].addr;
            if(!Eps->IsDefined()){
                return 0;
            }
            double eps = Eps->GetValue();
            if(eps<=0){
               return 0;
            }
            info = new extractLocal(args[0], eps, coreDistPos, 
                                    reachDistPos, epsPos, type);
            local.addr = info;
            return 0;
         }
      case REQUEST: {
         result.addr = info?info->next():0;
         return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
           if(info){
              delete info;
              local.addr = 0;
           }
           return 0;
      }
   }
   return -1;
 }

/*
4.4. Specification

*/
 OperatorSpec extractDbScanSpec(
  "stream x real -> stream",
  "_ extractDbScan",
  " Extract cluster from a stream processed via optics.",
  "query Kneipen feed extend[B : bbox(.GeoData)] "
    "opticsR[B, 2000.0, 10] extractDbScan[500.0] consume"
);


/*
4.5 Operator definition

*/
Operator extractDbScanOP(
  "extractDbScan",
  extractDbScanSpec.getStr(),
  extractDbScanVM,
  Operator::SimpleSelect,
  extractDbScanTM
);




/*
5 Algebra class ~ClusterOpticsAlgebra~

*/
 class ClusterOpticsAlgebra : public Algebra
 {
  public:
   ClusterOpticsAlgebra() : Algebra()
   {
    
    Operator* opr = 
        AddOperator(opticsInfoR(), opticsRVM, opticsRRecSL, opticsRTM);
    opr->SetUsesMemory();

    Operator* opm = 
        AddOperator(opticsInfoM(), opticsMVM, opticsMDisSL, opticsMTM);
    opm->SetUsesMemory();

    Operator* opf = 
        AddOperator(opticsInfoF(), opticsFVM, opticsFDisSL, opticsFTM);
    opf->SetUsesMemory();

    AddOperator(&extractDbScanOP);

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



