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


#include "SetOfObjectsM.h"
#include "SetOfObjectsR.h"
#include "DBScanGen.h"

#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Stream/Stream.h"
#include "StandardTypes.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "DistFunction.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <utility>


extern NestedList* nl;
extern QueryProcessor* qp;

using namespace std;

namespace clusterdbscanalg
{

/*

1 Operator dbScanR


This operator realizes the db scan for rectangles 
using an r-tree as index structure.

1.1 Type Mapping


*/
 ListExpr dbscanRTM( ListExpr args ) {
  if(nl->ListLength(args)!=2) {
   ErrorReporter::ReportError("two elements expected. "
    "Stream and argument list");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args))) {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=4) {
   ErrorReporter::ReportError("non conform list of cluster attribute, "
    "attribute name as cluster ID, Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcReal::checkType(nl->Third(arguments))) {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments))) {
   return listutils::typeError("no numeric MinPts");
  }


//Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0) {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is no member of the tuple");
   return nl->TypeError();
  }

 if( !Rectangle<2>::checkType(attrType)
   && !Rectangle<3>::checkType(attrType)
   && !Rectangle<4>::checkType(attrType)
   && !Rectangle<8>::checkType(attrType) ) {
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
  if(pos!=0) {
   ErrorReporter::ReportError("Attribute "+ namestr +
                              " already member of the tuple");
   return nl->TypeError();
  }//endif

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList))) {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }

  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("IsCore"),
      nl->SymbolAtom(CcBool::BasicType()) ));

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
                                    
 }


/*
1.2 Value mapping 

*/
 template <int dim>
 int dbscanRVM1(Word* args, Word& result, int message, Word& local, Supplier s)
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
1.3 Struct ~dbscanInfoRT~

*/

 struct dbscanRInfo :  OperatorInfo
 {
  dbscanRInfo() : OperatorInfo()
  {
   name      = "dbscanR";
   signature = "stream(Tuple) x Id x Id x real x int  -> stream(Tuple)";
   syntax    = "_ dbscanR [_, _, _, _]";
   meaning   = "Detects cluster from a given stream using MMR-Tree as index "
   "structure. The first parameter has to be a bbox, the second parameter is "
   "the name for the cluster ID attribute, the  third paramter is eps and "
   "the fourth parameter is MinPts. A tuple stream will be returned but the "
   "tuple will have additional attributes as IsCore, Visited and ClusterID";
   example   = "query Kneipen feed extend[B : bbox(.GeoData)] dbscanR "
       "[B, No, 1000.0, 5] consume";
  }
 };

/*
1.4 Selection method ~dbscanRSel~

*/ 
 int dbscanRSel(ListExpr args)
 {
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  assert(found > 0);
   
  if(Rectangle<2>::checkType(attrType)) {
   return 0;
  } else if(Rectangle<3>::checkType(attrType)) {
   return 1;
  } else if(Rectangle<4>::checkType(attrType)) {
   return 2;
  } else if(Rectangle<8>::checkType(attrType)) {
   return 3;
  }
   
  return -1;
 }

/*
1.5 Value Mapping Array

*/
 
 ValueMapping dbscanRVM[] = 
 {
    dbscanRVM1<2>,
    dbscanRVM1<3>,
    dbscanRVM1<4>,
    dbscanRVM1<8>
 };


/*
2 Operator dbScanM

*/
 
/*
2.1 Type mapping 

*/
 ListExpr dbscanMTM( ListExpr args ) {
  if(nl->ListLength(args)!=2) {
   ErrorReporter::ReportError("two elements expected. " 
            "Stream and argument list");
   return nl->TypeError();
  }

  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(nl->First(args))) {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=4) {
   ErrorReporter::ReportError("non conform list of cluster attribut, "
    "attribute name as cluster ID, Eps and MinPts");
   return nl->TypeError();
  }

  if(!CcReal::checkType(nl->Third(arguments))) {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments))) {
   return listutils::typeError("no numeric MinPts");
  }


//Check the cluster attribute name, if it is in the tuple
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  if(found == 0) {
   ErrorReporter::ReportError("Attribute "
    + attrName + " is no member of the tuple");
   return nl->TypeError();
  }

  if( !CcInt::checkType(attrType)
   && !CcReal::checkType(attrType)
   && !Point::checkType(attrType)
   && !CcString::checkType(attrType) 
   && !Picture::checkType(attrType) ) {
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
  if(pos!=0) {
   ErrorReporter::ReportError("Attribute "+ namestr +
                              " already member of the tuple");
   return nl->TypeError();
  }//endif

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList))) {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("IsCore"),
      nl->SymbolAtom(CcBool::BasicType()) ));
      
  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }


/*
1.2 Value mapping method ~dbscanM~ 

*/

template <class T, class DistComp> 
int dbscanMVM1(Word* args, Word& result, 
               int message, Word& local, Supplier s) {

  typedef  dbscan::DBScanGen< 
                   dbscan::SetOfObjectsM<DistComp,T >, 
                   DistComp > 
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
   
    DistComp dist; 

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
1.3 Struct ~dbscanMInfo~

*/
 struct dbscanMInfo :  OperatorInfo
 {
  dbscanMInfo() : OperatorInfo()
  {
   name      = "dbscanM";
   signature = "stream(Tuple) x Id x Id x real x int ->> stream(Tuple)";
   syntax    = "_ dbscanM [_, _, _, _]";
   meaning   = "Detects cluster from a given stream using an MMM-Tree as index "
   "structure. The first parameter is the attribute to cluster, the second "
   "parameter is the name for the cluster ID attribute, the  third parameter "
   "is eps and the fourth parameter is MinPts. A tuple stream will be returned "
   "but the tuple will have additional attributes as Visited and the clusterID";
   example   = "query Kneipen feed dbscanM[GeoData, CID, 1000.0, 5] consume";
   
  }
 };


/*
1.4 Selection Function

*/ 
 int dbscanMSel(ListExpr args) {

  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr attrType;
  string attrName = nl->SymbolValue(nl->First(nl->Second(args)));
  int found = FindAttribute(attrList, attrName, attrType);
  assert(found > 0);
  
  if(CcInt::checkType(attrType)) {
   return 0;
  } else if(CcReal::checkType(attrType)) {
   return 1;
  } else if(Point::checkType(attrType)) {
   return 2;
  } else if(CcString::checkType(attrType)) {
   return 3;
  } else if(Picture::checkType(attrType)) {
   return 4;
  }
  
  return -1; 
 };

 
/*
Value mapping array ~dbscanMVM~

*/

ValueMapping dbscanMVM[] = 
 {
  dbscanMVM1<CcInt, IntDist>,
  dbscanMVM1<CcReal, RealDist>,
  dbscanMVM1<Point, PointDist>,
  dbscanMVM1<CcString, StringDist>,
  dbscanMVM1<Picture, PictureDist>
 };
 





/*
3  Operator ~dbscanF~

This operator works the same as the ~dbScanM~ operator.
The difference is that this operator allows the user to define
its own distance function instead of using a predefined one.
While the ~dbScanM~ operator has a small set of attribute 
data type which can be processed, the ~dbscanF~ operator
is able to process arbitrary attribute data types.

3.1 Type Mapping

*/
ListExpr dbscanFTM( ListExpr args ) {
  if(nl->ListLength(args)!=2) {
   return listutils::typeError("two elements expected. " 
            "Stream and argument list");
  }
  ListExpr stream = nl->First(args);

  if(!Stream<Tuple>::checkType(stream)) {
   return listutils::typeError("first argument is not stream(Tuple)");
  }

  ListExpr arguments = nl->Second(args);

  if(nl->ListLength(arguments)!=5) {
   return listutils::typeError("non conform list of cluster attribut, "
    "attribute name as cluster ID, Eps and MinPts, distfun");
  }

  if(!CcReal::checkType(nl->Third(arguments))) {
   return listutils::typeError("no numeric Eps");
  }

  if(!CcInt::checkType(nl->Fourth(arguments))) {
   return listutils::typeError("no numeric MinPts");
  }

  ListExpr fun = nl->Fifth(arguments);

  if(!listutils::isMap<2>(fun)) {
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
  if(found == 0) {
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
  if(pos!=0) {
   return listutils::typeError("Attribute "+ namestr +
                              " already member of the tuple");
  }//endif
  pos = FindAttribute(attrList,"Visited",typeList);
  if(pos!=0) {
   return listutils::typeError("Attribute Visited" 
                              " already member of the tuple");
  }//endif
  pos = FindAttribute(attrList,"IsCore",typeList);
  if(pos!=0) {
   return listutils::typeError("Attribute IsCore" 
                              " already member of the tuple");
  }//endif

  //Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(stream));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  
  attrList = nl->Rest(attrList);
  
  while(!(nl->IsEmpty(attrList))) {
   lastlistn = nl->Append(lastlistn,nl->First(attrList));
   attrList = nl->Rest(attrList);
  }


  lastlistn = nl->Append(lastlistn, 
     (nl->TwoElemList(name, nl->SymbolAtom(CcInt::BasicType()) )));
 
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("Visited"),
      nl->SymbolAtom(CcBool::BasicType()) ));
  
  lastlistn = nl->Append(lastlistn, 
      nl->TwoElemList(nl->SymbolAtom("IsCore"),
      nl->SymbolAtom(CcBool::BasicType()) ));

  return nl->ThreeElemList(
   nl->SymbolAtom(Symbol::APPEND())
        ,nl->OneElemList(nl->IntAtom(found-1))
        ,nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM())
                        ,nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType())
                                      ,newAttrList)));
 }


/*
3.1 Value Mapping

The template argument specifies the result type of the distance function and
may be CcInt or CcReal

*/ 

template <class R> 
int dbscanFVM1(Word* args, Word& result, 
               int message, Word& local, Supplier s) {

  typedef  CustomDist<Attribute*,R> DistComp;
  typedef  dbscan::DBScanGen< 
                   dbscan::SetOfObjectsM<DistComp,Attribute >, 
                   DistComp > 
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
   
    DistComp dist;
    Supplier supplier2 = qp->GetSupplier(args[1].addr, 4);
    dist.initialize(qp, supplier2); 
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
1.3 Struct ~dbscanFInfo~

*/ 
 struct dbscanFInfo :  OperatorInfo
 {
  dbscanFInfo() : OperatorInfo()
  {
   name      = "dbscanF";
   signature = "stream(tuple) x ID x ID x real x int x fun -> stream(tuple)";
   syntax    = "_ dbscanF [_, _, _, _, fun]";
   meaning   = "Detects cluster from a given stream using an MMM-Tree as index "
   "structure. The first argument is a tuple stream containing the data. "
   "The second argument is the attribute to cluster, the third "
   "argument is the name for the cluster ID attribute, the  fourth argument "
   "is eps and the fifth argument is MinPts. "
   "The last argument is a function mapping from a pair of cluster attributes "
   " to the distance between their values."
   "A tuple stream will be returned "
   "that will have additional attributes as visited "
   "and clusterID. ";
   example   = "query plz feed dbscanF[PLZ, CID, 1000.0, 5, "
   "fun(i1: int, i2: int) abs(i1 - i2)] consume";  
  }
 };
 
 
/*
1.4 Selection method 

*/
 int dbscanFSel(ListExpr args)
 {
  ListExpr funResult= nl->Fourth(nl->Fifth(nl->Second(args)));
  if(CcInt::checkType(funResult)) return 0;
  if(CcReal::checkType(funResult)) return 1;
  return -1; 
 };
 
 
/*
1.5. Value mapping array 

*/

 ValueMapping dbscanFVM[] = 
 {
  dbscanFVM1<CcInt>,
  dbscanFVM1<CcReal> 
 };
 

/*
3  Operator ~dbscanTF~

This operator works similarly to the ~dbScanF~ operator.
The difference is that this operator allows the user to define
a distance function based on tuples, including the possibility to process
several attributes of a tuple.

3.1 Type Mapping

*/
ListExpr dbscanTFTM(ListExpr args) {
  if (nl->ListLength(args) != 2) {
   return listutils::typeError("2 elements expected: Stream and argument list");
  }
  if(!Stream<Tuple>::checkType(nl->First(args))) {
   return listutils::typeError("first argument is not a tuple stream");
  }
  ListExpr arguments = nl->Second(args);
  if (nl->ListLength(arguments) != 4) {
    return listutils::typeError("four arguments required after stream");
  }
  if (!CcReal::checkType(nl->Second(arguments))) {
    return listutils::typeError("eps must have type real");
  }
  if (!CcInt::checkType(nl->Third(arguments))) {
    return listutils::typeError("MinPts must have type int");
  }
  ListExpr fun = nl->Fourth(arguments);
  if (!listutils::isMap<2>(fun)) {
    return listutils::typeError("function with two arguments required");
  }
  if (!nl->Equal(nl->Second(fun), nl->Third(fun)) || 
  (!CcReal::checkType(nl->Fourth(fun)) && !CcInt::checkType(nl->Fourth(fun)))) {
    return listutils::typeError("fun is not of type: T x T -> {int, real}");
  }
  ListExpr attrList = nl->Second(nl->Second(nl->First(args)));
  ListExpr typeList;
  ListExpr cidname = nl->First(arguments); 
  string errormsg;
  if (!listutils::isValidAttributeName(cidname, errormsg)) {
    return listutils::typeError(errormsg);
  }
  string cidnamestr = nl->SymbolValue(cidname);
  int pos = FindAttribute(attrList, cidnamestr, typeList);
  if (pos!=0) {
    return listutils::typeError("Attribute "+ cidnamestr + " already exists");
  }
  pos = FindAttribute(attrList, "Visited", typeList);
  if (pos!=0) {
    return listutils::typeError("Attribute Visited already exists");
  }
  pos = FindAttribute(attrList, "IsCore", typeList);
  if (pos!=0) {
    return listutils::typeError("Attribute IsCore already exists");
  }
  // Copy attrlist to newattrlist
  attrList             = nl->Second(nl->Second(nl->First(args)));
  ListExpr newAttrList = nl->OneElemList(nl->First(attrList));
  ListExpr lastlistn   = newAttrList;
  attrList = nl->Rest(attrList);
  while (!(nl->IsEmpty(attrList))) {
    lastlistn = nl->Append(lastlistn, nl->First(attrList));
    attrList = nl->Rest(attrList);
  }
  lastlistn = nl->Append(lastlistn, nl->TwoElemList(cidname, 
                                           nl->SymbolAtom(CcInt::BasicType())));
  lastlistn = nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("Visited"),
                                          nl->SymbolAtom(CcBool::BasicType())));
  lastlistn = nl->Append(lastlistn, nl->TwoElemList(nl->SymbolAtom("IsCore"),
                                          nl->SymbolAtom(CcBool::BasicType())));
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                         newAttrList));
}


/*
3.1 Value Mapping

The template argument specifies the result type of the distance function and
may be CcInt or CcReal

*/
template <class R> 
int dbscanTFVM(Word* args, Word& result, int message, Word& local, Supplier s) {
  typedef  TupleDist<R> TupleComp;
  typedef  dbscan::DBScanGen<dbscan::SetOfObjectsM<TupleComp, Tuple>, 
                             TupleComp> dbscanclass;
  dbscanclass* li = (dbscanclass*) local.addr;  
  switch (message) {
    case OPEN : {
      Word stream = args[0];
      Supplier supplier = qp->GetSupplier(args[1].addr, 1);
      Word argument;
      qp->Request(supplier, argument);
      CcReal* eps = ((CcReal*)argument.addr);
      supplier = qp->GetSupplier(args[1].addr, 2);
      qp->Request(supplier, argument);
      CcInt* minPts = ((CcInt*)argument.addr);
      if (!eps->IsDefined() || !minPts->IsDefined()) {
        return 0;
      }
      if (eps->GetValue() < 0 || minPts->GetValue() < 0) {
        return 0;
      }
      ListExpr tt = (nl->Second(GetTupleResultType(s)));
      if (li) {
        delete li;
        local.addr = 0;
      }
      size_t maxMem = (qp->GetMemorySize(s) * 1024);
      TupleComp dist;
      Supplier supplier2 = qp->GetSupplier(args[1].addr, 3);
      dist.initialize(qp, supplier2);
      local.addr = new dbscanclass(stream, tt, eps->GetValue(), 
                                   minPts->GetValue(), maxMem, -1, dist);
      return 0;
    }
    case REQUEST: {
      result.addr = li ? li->next() : 0;
      return result.addr ? YIELD : CANCEL;
    }
    case CLOSE: {
      if (li) {
        delete li;
        local.addr = 0;
      }
    }
  }
  return 0;
}
 
/*
1.3 Struct ~dbscanTFInfo~

*/ 
struct dbscanTFInfo :  OperatorInfo {
  dbscanTFInfo() : OperatorInfo() {
    name      = "dbscanTF";
    signature = "stream(tuple) x Id x real x int x fun -> stream(tuple)";
    syntax    = "_ dbscanTF [_, _, _, fun]";
    meaning   = "Detects clusters in a given stream using MMM-Tree as index "
    "structure. The first parameter is the name for the new attribute "
    "containing the cluster number." 
    "The second  parameter is eps and the third parameter is MinPts. The last "
    "parameter is a function defining the distance between the incoming tuples."
    " A tuple "
    "stream will be returned but the tuple will have additional attributes as "
    "visited and clusterID. ";
    example   = "query Kneipen feed dbscanTF[No, 500.0, 5, fun(t1: tuple((Name "
    "string) (Strasse string) (GeoData point)), t2: tuple((Name string) ("
    "Strasse string) (GeoData point))) distance(attr(t1, GeoData), attr(t2, "
    "GeoData))] sortby[No] groupby[No ; C : group count] count";  
  }
};

/*
1.4 Selection method 

*/
int dbscanTFSel(ListExpr args) {
  ListExpr funResultType = nl->Fourth(nl->Fourth(nl->Second(args)));
  if (CcInt::checkType(funResultType))  return 0;
  if (CcReal::checkType(funResultType)) return 1;
  return -1; 
};

/*
1.5. Value mapping array 

*/

ValueMapping dbscanTFVMs[] = {dbscanTFVM<CcInt>, dbscanTFVM<CcReal>};

/*
Algebra class ~ClusterDBScanAlgebra~

*/
 class ClusterDBScanAlgebra : public Algebra
 {
  public:
   ClusterDBScanAlgebra() : Algebra()
   {
    AddOperator(dbscanRInfo(), dbscanRVM, 
                                dbscanRSel, 
                                dbscanRTM)->SetUsesMemory();

    AddOperator(dbscanMInfo(), dbscanMVM, 
                 dbscanMSel, dbscanMTM)->SetUsesMemory();

    AddOperator(dbscanFInfo(),dbscanFVM, 
                dbscanFSel, dbscanFTM)->SetUsesMemory();

    AddOperator(dbscanTFInfo(),dbscanTFVMs, 
                dbscanTFSel, dbscanTFTM)->SetUsesMemory();
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





