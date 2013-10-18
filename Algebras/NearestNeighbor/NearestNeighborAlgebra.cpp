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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]
//[ue] [\"{u}]
//[toc] [\tableofcontents]

""[2]

[1] NearestNeighbor Algebra

November 2008, A. Braese
15 October 2009 Mahmoud Sakr: Added the isknn operator

[toc]

0 Overview

This example algebra provides a distance-scan for a point set
in a R-Tree. A new datatype is not given but there are some operators:

  1. ~distancescan~, which gives out k objects stored in a relation, ordered
                     in a r-tree in the order ascending to their distance to a
                     given object with the same dimension

  2. ~knearest~, which gives out the k-nearest elements to a moving point.

  3. ~knearestvector~, the same as knearest, but uses a vector to store
                       the active elements, than a unbalanced tree

1 Preliminaries

1.1 Includes

*/
#include <vector>
#include <queue>
#include <iterator>
#include <iostream>
#include <ostream>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "NearestNeighborAlgebra.h"
#include "TemporalAlgebra.h"
#include "BBTree.h"
#include "BTreeAlgebra.h"
#include "TBTree.h"
#include "ListUtils.h"
#include "CoverInterval.h"
#include "AlmostEqual.h"
#include "Symbols.h"


using namespace tbtree;

/*
The file "Algebra.h" is included, since the new algebra must be a subclass of
class Algebra. All of the data available in Secondo has a nested list
representation. Therefore, conversion functions have to be written for this
algebra, too, and "NestedList.h" is needed for this purpose. The result of an
operation is passed directly to the query processor. An instance of
"QueryProcessor" serves for this. Secondo provides some standard data types,
e.g. "CcInt", "CcReal", "CcString", "CcBool", which is needed as the
result type of the implemented operations. To use them "StandardTypes.h"
needs to be included. The file "RtreeAlgebra.h" is included because
some operators get a rtree as argument.

*/

extern NestedList* nl;
extern QueryProcessor *qp;

/*
The variables above define some global references to unique system-wide
instances of the query processor and the nested list storage.

1.2 Auxiliaries

Within this algebra module implementation, we have to handle values of
four different types defined in namespace ~symbols~: ~INT~ and ~REAL~, ~BOOL~
and ~STRING~.  They are constant values of the C++-string class.

Moreover, for type mappings some auxiliary helper functions are defined in the
file "TypeMapUtils.h" which defines a namespace ~mappings~.

*/

#include "TypeMapUtils.h"
#include "Symbols.h"
#include <sys/timeb.h>

using namespace mappings;

#include <string>
using namespace std;
double difftimeb( struct timeb* t1, struct timeb* t2 )
{
  double dt1 = t1->time + (double)t1->millitm / 1000.0;
  double dt2 = t2->time + (double)t2->millitm / 1000.0;
  return dt1 - dt2;
}


struct timeb gt1;
struct timeb gt2;

/*
The implementation of the algebra is embedded into
a namespace ~near~ in order to avoid name conflicts with other modules.

*/

namespace near {

const double DISTDELTA = 0.001;
const double RDELTA = -0.000000015;
const double QUADDIFF = 0.01;

/*
2 Creating Operators

2.1 Type Mapping Functions

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. Again we use interface ~NList.h~ for
manipulating list expressions. For every operator is one type mapping
function given.

The function distanceScanTypeMap is the type map for distancescan.

*/
ListExpr
distanceScanTypeMap( ListExpr args )
{

  string errmsg = "rtree x rel x rectangle x int expected";
  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError(errmsg + "(wrong number of arguments)");
    return nl->TypeError();
  }

  /* Split argument in four parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);

  // check for fourth argument type == int
  if(!nl->IsEqual(quantity,CcInt::BasicType())){
    ErrorReporter::ReportError(errmsg + "(fourth arg not of type int)");
    return nl->TypeError();
  }

  if( ! listutils::isSpatialType(searchWindow) &&
      ! listutils::isRectangle(searchWindow)){
    ErrorReporter::ReportError(errmsg +
              "(searchwindow neither of kind spatial nor a rectangle)");
    return listutils::typeError();
  }

  if(!listutils::isRTreeDescription(rtreeDescription)){
    ErrorReporter::ReportError(errmsg + "(invalid rtree description)");
    return listutils::typeError();
  }


  ListExpr rtreeKeyType = listutils::getRTreeType(rtreeDescription);

  if( !listutils::isSpatialType(rtreeKeyType) &&
      !listutils::isRectangle(rtreeKeyType)){
    ErrorReporter::ReportError(errmsg + "(tree not over a spatial attribute)");
    return listutils::typeError();
  }

  if(!listutils::isRelDescription(relDescription)){
    ErrorReporter::ReportError(errmsg + "(second arg is not a relation)");
    return listutils::typeError();
  }

  ListExpr rtreeTupleDescription = nl->Second(rtreeDescription);
  if(!nl->Equal(rtreeTupleDescription, nl->Second(relDescription))){
    ErrorReporter::ReportError(errmsg +
                 "(type of rtree and relation are different)");
    return listutils::typeError();
  }

  // check for same dimension in rtree and query window
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = listutils::emptyErrorInfo();
  if( !(
          ( algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, Rectangle<2>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL2D(), searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<2>::BasicType()) &&
            nl->IsEqual(searchWindow, Rectangle<2>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo) &&
             nl->IsEqual(searchWindow, Rectangle<3>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL3D(), searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<3>::BasicType()) &&
            nl->IsEqual(searchWindow, Rectangle<3>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, Rectangle<4>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL4D(), searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<4>::BasicType()) &&
            nl->IsEqual(searchWindow, Rectangle<4>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, Rectangle<8>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL8D(), searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<8>::BasicType()) &&
            nl->IsEqual(searchWindow, Rectangle<8>::BasicType()) ))){
    ErrorReporter::ReportError(errmsg + "(different dimensions)");
    return nl->TypeError();
   }
   return
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->Second(relDescription));
}


ListExpr
distanceScan2STypeMap( ListExpr args ) {

  string err = "rtree x SPATIAL x int expected";
  if(!nl->HasLength(args,3)){
      return listutils::typeError(err + " (wrong number of args)");
  }
  ListExpr rtree = nl->First(args);
  ListExpr spatial = nl->Second(args);
  ListExpr count = nl->Third(args);
  if(!CcInt::checkType(count)){
     return listutils::typeError(err + "(third arg is not an int)");
  }
  if( ! listutils::isSpatialType(spatial) &&
      ! listutils::isRectangle(spatial)){
     return listutils::typeError(err + "(second arg is not SPATIAL)");
  }
  if(!listutils::isRTreeDescription(rtree)){
     return listutils::typeError(err + "(first arg is not an r-tree)");
  }

  ListExpr rtreeKeyType = listutils::getRTreeType(rtree);
 
  if( !listutils::isSpatialType(rtreeKeyType) &&
      !listutils::isRectangle(rtreeKeyType)){
     return listutils::typeError(err + "(invalid keyTpe in r-tree)");
  }
  // check for equal dimension in rtreekeyType and spatial 

  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = listutils::emptyErrorInfo();
  if( !(
          ( algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(spatial, Rectangle<2>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL2D(), spatial, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<2>::BasicType()) &&
            nl->IsEqual(spatial, Rectangle<2>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo) &&
             nl->IsEqual(spatial, Rectangle<3>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL3D(), spatial, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<3>::BasicType()) &&
            nl->IsEqual(spatial, Rectangle<3>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(spatial, Rectangle<4>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL4D(), spatial, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<4>::BasicType()) &&
            nl->IsEqual(spatial, Rectangle<4>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo) &&
            nl->IsEqual(spatial, Rectangle<8>::BasicType()) ) ||
          ( algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo) &&
            algMgr->CheckKind(Kind::SPATIAL8D(), spatial, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, Rectangle<8>::BasicType()) &&
            nl->IsEqual(spatial, Rectangle<8>::BasicType()) ))){
       return listutils::typeError("different dimensions rtree x spatial");
   }

   ListExpr res =  
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
                       listutils::basicSymbol<TupleIdentifier>());
    return res;
}



ListExpr
distanceScan3TypeMap( ListExpr args ) {

   bool maxD = false;

   if(nl->ListLength(args) > 2){
     ListExpr last;
     ListExpr args2;
     bool first = true;
     while(!nl->IsEmpty(args)){
       ListExpr f = nl->First(args);
       args = nl->Rest(args);
       if(nl->IsEmpty(args)){ // f is the last argument
          if(CcReal::checkType(f)){
             maxD = true;            
          } else {
           last = nl->Append(last, f);
          }
       } else {
         if(first){
           args2 = nl->OneElemList(f);
           last = args2;
           first = false;
         } else {
           last = nl->Append(last, f);
         }
       }
     }
     args = args2;
   }


   ListExpr res1;
   ListExpr rtreetype;
   int j = -1;
   if(nl->ListLength(args)==5){
     ListExpr First4 = nl->FourElemList(
                           nl->First(args),
                           nl->Second(args),
                           nl->Third(args),
                           nl->Fourth(args));

     res1 = distanceScanTypeMap(First4);
     if(nl->Equal(res1,nl->TypeError())){
        return res1;
     }
     ListExpr attrNameList = nl->Fifth(args);
     if(nl->AtomType(attrNameList)!=SymbolType){
        ErrorReporter::ReportError("Invalid attribute name "
                                   "given as fifth element");
        return nl->TypeError();
     }
     string attrName = nl->SymbolValue(attrNameList);
     ListExpr attrList = nl->Second(nl->Second(nl->Second(args)));

     rtreetype = nl->Third(nl->First(args));
     ListExpr attrType;
     j = FindAttribute(attrList, attrName, attrType);
     if(j==0){
       ErrorReporter::ReportError("Attribute "+ attrName + " not found");
       return nl->TypeError();
     }
     if(!nl->Equal(attrType,rtreetype)){
        ErrorReporter::ReportError("different types of "
                                   "attribute and rtree key");
        return nl->TypeError();
     }
  } else if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError("Wronmg numer of parameters");
    return nl->TypeError();
  } else { // four parameter, spatial type must be unique
    res1 = distanceScanTypeMap(args);
    if(nl->Equal(res1,nl->TypeError())){
      return nl->TypeError();
    }
    ListExpr attrList = nl->Second(nl->Second(nl->Second(args)));
    rtreetype = nl->Third(nl->First(args));

    // search for rtreetype in attrList and check if unique
    int pos  = 0;
    while(!nl->IsEmpty(attrList)){
      pos++;
      ListExpr first = nl->First(attrList);
      attrList = nl->Rest(attrList);
      ListExpr type = nl->Second(first);
      if(nl->Equal(type,rtreetype)){
         if(j>0){
           ErrorReporter::ReportError("Attribute not unique, "
                                       "please append attribute name ");
           return nl->TypeError();
         } else {
           j = pos;
         }
      }
    }
    if(j<0){
      ErrorReporter::ReportError("tuple stream does not "
                                 "contain an indexed attribute");
      return nl->TypeError();
    }
  }

  // check for allowed combinations
  ListExpr queryTypeList = nl->Third(args);
  if(nl->AtomType(queryTypeList)!=SymbolType){
    ErrorReporter::ReportError("Unsupported type as querytype");
    return nl->TypeError();
  }
  if(nl->AtomType(rtreetype)!=SymbolType){
    ErrorReporter::ReportError("Unsupported type within the rtree");
    return nl->TypeError();
  }
  string rt = nl->SymbolValue(rtreetype);
  string qt = nl->SymbolValue(queryTypeList);

  if( (rt != Point::BasicType()) && (rt != Points::BasicType()) &&
      (rt != Line::BasicType()) && (rt != Region::BasicType()) &&
      (rt != Rectangle<2>::BasicType())){
    ErrorReporter::ReportError("Unsupported type within the rtree");
    return nl->TypeError();
  }
  if( (qt != Point::BasicType()) && (qt != Points::BasicType()) &&
      (qt != Line::BasicType()) && (qt != Region::BasicType()) &&
      (qt != Rectangle<2>::BasicType())){
    ErrorReporter::ReportError("Unsupported type for query object");
    return nl->TypeError();
  }
  return nl->ThreeElemList(
               nl->SymbolAtom(Symbol::APPEND()),
               nl->TwoElemList( nl->IntAtom(j-1),
                                nl->BoolAtom(maxD)),
               res1);
}

/*
  rtree3(upoint) x rel x point x instant x int [ x attrname]

*/

ListExpr distanceScan4TypeMap(ListExpr args){
  string err = "rtree3(upoint) x rel x point x "
               "instant x int [x attrname] expected";
  int len = nl->ListLength(args);
  if((len!=5)  && (len!=6)){
    ErrorReporter::ReportError(err + " (invalid number of arguments)");
    return listutils::typeError();
  }
  if(!listutils::isRTreeDescription(nl->First(args))){
    ErrorReporter::ReportError(err + " (invalid rtree)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->First(nl->First(args)),RTree3TID::BasicType())){
    ErrorReporter::ReportError(err + " (invalid rtree  dimension)");
    return listutils::typeError();
  }

  if(!listutils::isRelDescription(nl->Second(args))){
    ErrorReporter::ReportError(err + " (invalid relation)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Third(args),Point::BasicType())){
    ErrorReporter::ReportError(err + " (invalid point)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Fourth(args),Instant::BasicType())){
    ErrorReporter::ReportError(err + " (invalid instant)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Fifth(args),CcInt::BasicType())){
    ErrorReporter::ReportError(err + " (invalid int)");
    return listutils::typeError();
  }

  // relation and rtree must have the same tuple type
  if(!nl->Equal(nl->Second(nl->First(args)), nl->Second(nl->Second(args)))){
    ErrorReporter::ReportError(err + " (different type of rtree and rel)");
    return listutils::typeError();
  }
  ListExpr rtreekey = listutils::getRTreeType(nl->First(args));
  if(!nl->IsEqual(rtreekey,UPoint::BasicType())){
    ErrorReporter::ReportError(err + " (rtree not build on upoint)");
    return listutils::typeError();
  }

  ListExpr attrList = nl->Second(nl->Second(nl->Second(args)));
  ListExpr res1 = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                  nl->Second(nl->Second(args)));
  if(len==6){
    ListExpr aname = nl->Sixth(args);
    if(nl->AtomType(aname)!=SymbolType){
       ErrorReporter::ReportError(err + " (invalid attribute name)");
       return listutils::typeError();
    }
    string name = nl->SymbolValue(aname);
    ListExpr type;
    int j = listutils::findAttribute(attrList, name, type);
    if(!j){
      ErrorReporter::ReportError(err + " (attribute not found)");
      return listutils::typeError();
    }
    if(!nl->IsEqual(type,UPoint::BasicType()) ){
      ErrorReporter::ReportError(err + " (attribute not a upoint)");
      return listutils::typeError();
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->IntAtom(j)),
                             res1);
  } else { // length ==5
    string name;
    int j = listutils::findType(attrList,
                                nl->SymbolAtom(UPoint::BasicType()),name);
    if(!j){
       ErrorReporter::ReportError(err + " (no upoint stored in relation)");
       return listutils::typeError();
    }
    int j2 = listutils::findType(attrList,
                                 nl->SymbolAtom(UPoint::BasicType()),
                                 name,
                                 j+1);
    if(j2){
       ErrorReporter::ReportError(err + " (upoint stored twice in relation)");
       return listutils::typeError();
    }
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->TwoElemList(nl->IntAtom(j), nl->IntAtom(j)),
                             res1);
  }
}



/*
The function knearestTypeMap is the type map for the operator knearest

*/
ListExpr KnearestTypeMap( ListExpr args )
{
  string msg = "stream x attrname x mpoint x int expected";

  if(!(nl->ListLength(args)==4 || nl->ListLength(args)==5)){
    ErrorReporter::ReportError(msg + "(invalid number of arguments");
    return nl->TypeError();
  }

  if(nl->ListLength(args) == 4){
    ListExpr stream = nl->First(args);
    ListExpr attrName = nl->Second(args);
    ListExpr mpoint = nl->Third(args);
    ListExpr card = nl->Fourth(args);

    if(!IsStreamDescription(stream)){
      ErrorReporter::ReportError(msg+" (first arg is not a stream)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(mpoint,MPoint::BasicType())){
      ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(card,CcInt::BasicType())){
      ErrorReporter::ReportError(msg+" (fourth arg is not an int)");
      return nl->TypeError();
    }

    if(nl->AtomType(attrName)!=SymbolType){
      ErrorReporter::ReportError(msg+" (second arg is not an attrname)");
      return nl->TypeError();
    }

    int j;
    ListExpr attrType;
    j = FindAttribute(nl->Second(nl->Second(stream)),
                    nl->SymbolValue(attrName),attrType);

    if(j==0) {
      ErrorReporter::ReportError(msg+" (attrName not found in attributes)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(attrType,UPoint::BasicType())){
      ErrorReporter::ReportError(msg+" (attrName does not refer to an upoint)");
      return nl->TypeError();
    }


    return
        nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->OneElemList(nl->IntAtom(j)),
          stream);
  }else if(nl->ListLength(args) == 5){
  
   ListExpr stream = nl->First(args);
    ListExpr attrName = nl->Second(args);
    ListExpr mpoint = nl->Third(args);
    ListExpr card = nl->Fourth(args);

    ListExpr zone = nl->Fifth(args);
    
    if(!IsStreamDescription(stream)){
      ErrorReporter::ReportError(msg+" (first arg is not a stream)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(mpoint,MPoint::BasicType())){
      ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(card,CcInt::BasicType())){
      ErrorReporter::ReportError(msg+" (fourth arg is not an int)");
      return nl->TypeError();
    }

    if(nl->AtomType(attrName)!=SymbolType){
      ErrorReporter::ReportError(msg+" (second arg is not an attrname)");
      return nl->TypeError();
    }

    int j;
    ListExpr attrType;
    j = FindAttribute(nl->Second(nl->Second(stream)),
                    nl->SymbolValue(attrName),attrType);

    if(j==0) {
      ErrorReporter::ReportError(msg+" (attrName not found in attributes)");
      return nl->TypeError();
    }

    if(!nl->IsEqual(attrType,UPoint::BasicType())){
      ErrorReporter::ReportError(msg+" (attrName does not refer to an upoint)");
      return nl->TypeError();
    }

    
    if(!nl->IsEqual(zone,CcInt::BasicType())){
      ErrorReporter::ReportError(msg+" (fifth arg is not an int)");
      return nl->TypeError();
    }
    
    return
        nl->ThreeElemList(
            nl->SymbolAtom(Symbol::APPEND()),
            nl->OneElemList(nl->IntAtom(j)),
          stream);
          
  
  }else{
    assert(false);
  }
  
}


/*
The function knearestTypeMap is the type map for the operator knearest

*/
ListExpr knearestTypeMap( ListExpr args )
{
  string msg = "stream x attrname x mpoint x int expected";
  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError(msg + "(invalid number of arguments");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);
  ListExpr attrName = nl->Second(args);
  ListExpr mpoint = nl->Third(args);
  ListExpr card = nl->Fourth(args);

  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError(msg+" (first arg is not a stream)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(card,CcInt::BasicType())){
    ErrorReporter::ReportError(msg+" (fourth arg is not an int)");
    return nl->TypeError();
  }

  if(nl->AtomType(attrName)!=SymbolType){
    ErrorReporter::ReportError(msg+" (second arg is not an attrname)");
    return nl->TypeError();
  }

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(stream)),
                    nl->SymbolValue(attrName),attrType);

  if(j==0) {
    ErrorReporter::ReportError(msg+" (attrName not found in attributes)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(attrType,UPoint::BasicType())){
    ErrorReporter::ReportError(msg+" (attrName does not refer to an upoint)");
    return nl->TypeError();
  }


  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->IntAtom(j)),
      stream);
}
/*
The function knearestdistTypeMap is the type map for the operator knearestdist

*/
ListExpr knearestdistTypeMap( ListExpr args )
{
  string msg = "stream x attrname x mpoint x int expected";
  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError(msg + "(invalid number of arguments");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);
  ListExpr attrName = nl->Second(args);
  ListExpr mpoint = nl->Third(args);
  ListExpr card = nl->Fourth(args);

  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError(msg+" (first arg is not a stream)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
    return nl->TypeError();
  }

  if(!(nl->IsEqual(card,CcInt::BasicType()) ||
    nl->IsEqual(card,CcReal::BasicType()))){
    ErrorReporter::ReportError(msg+" (fourth arg is not an int)");
    return nl->TypeError();
  }

  if(nl->AtomType(attrName)!=SymbolType){
    ErrorReporter::ReportError(msg+" (second arg is not an attrname)");
    return nl->TypeError();
  }

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(stream)),
                    nl->SymbolValue(attrName),attrType);

  if(j==0) {
    ErrorReporter::ReportError(msg+" (attrName not found in attributes)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(attrType,UPoint::BasicType())){
    ErrorReporter::ReportError(msg+" (attrName does not refer to an upoint)");
    return nl->TypeError();
  }

  ListExpr res;
  if(nl->IsEqual(card,CcReal::BasicType())){
      res = nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),

//                 nl->TwoElemList(
//                     nl->TwoElemList(
//                         nl->SymbolAtom("KNeighbor"),
//                         nl->SymbolAtom(UPoint::BasicType())),
//                     nl->TwoElemList(
//                         nl->SymbolAtom("Dist"),
//                         nl->SymbolAtom(MReal::BasicType()))
//                     )));

                  nl->OneElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("KNeighbor"),
                        nl->SymbolAtom(UPoint::BasicType()))
                    )));

  }
    if(nl->IsEqual(card,CcInt::BasicType())){
      res = nl->SymbolAtom(MReal::BasicType());
  }


  return   nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->IntAtom(j)),
      res);


}

/*
The function oldknearestFilterTypeMap is the type map for the
operator knearestfilter

*/
ListExpr
oldknearestFilterTypeMap( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERROR" ) );

  string errmsg = "rtree x relation x mpoint x int expected";

  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr mpoint = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);

  // the third element has to be of type mpoint
  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,CcInt::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // an rtree description must have length 4
  if(nl->ListLength(rtreeDescription)!=4){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  if(!nl->IsEqual(rtreeSymbol, RTree3TID::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the keytype of the rtree must be of kind SPATIAL3D
  if(!algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo) &&
     !nl->IsEqual(rtreeKeyType,Rectangle<3>::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(nl->ListLength(rtreeTupleDescription)!=2){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!nl->IsEqual(nl->First(rtreeTupleDescription),Tuple::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!IsTupleDescription(nl->Second(rtreeTupleDescription))){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(nl->AtomType(rtreeTwoLayer)!=BoolType){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!IsRelDescription(relDescription)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr rtreeAttrList =  nl->Second(rtreeTupleDescription);
  ListExpr relAttrList = nl->Second(nl->Second(relDescription));

  if(!nl->Equal(rtreeAttrList, relAttrList)){
    ErrorReporter::ReportError("relation and rtree have"
                               " not the same attrlist");
    return nl->TypeError();
  }

  return
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->Second(nl->Second(args)));
}


/*
2.1.10 rect2periods TypeMap

signatur ist rect3 -> periods

*/
ListExpr rect2periodsTypeMap(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Invalid number of arguments");
    return nl->TypeError();
  }
  ListExpr arg = nl->First(args);
  if(!nl->IsEqual(arg,Rectangle<3>::BasicType())){
    ErrorReporter::ReportError("rect3 expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom(Periods::BasicType());
}

/*
2.1.11 isknn TypeMap

v in {int | string} x string(rel) x string(btree) x
    string(MBool) x string(ID) -> APPEND v mbool

*/
ListExpr isknnTypeMap(ListExpr args){
  string idtype="";
  if(nl->ListLength(args)!=5){
    ErrorReporter::ReportError("isknn: Invalid number of arguments");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  idtype = nl->ToString(arg1);
  if(!nl->IsEqual(arg1,CcInt::BasicType()) &&
    !nl->IsEqual(arg1,CcString::BasicType()) ){
    ErrorReporter::ReportError("isknn: expected int or string identifier");
    return nl->TypeError();
  }

  ListExpr arg2 = nl->Second(args);
  if(!nl->IsEqual(arg2,CcString::BasicType())){
    ErrorReporter::ReportError("isknn: expected relation name");
    return nl->TypeError();
  }

  ListExpr arg3 = nl->Third(args);
  if(!nl->IsEqual(arg3,CcString::BasicType())){
    ErrorReporter::ReportError("isknn: expected btree name");
    return nl->TypeError();
  }

  ListExpr arg4 = nl->Fourth(args);
  if(!nl->IsEqual(arg4,CcString::BasicType())){
    ErrorReporter::ReportError("isknn: expected a valid mpoint attribute name");
    return nl->TypeError();
  }

  ListExpr arg5 = nl->Fifth(args);
  if(!nl->IsEqual(arg5,CcString::BasicType())){
    ErrorReporter::ReportError("isknn: expected a valid id attribute name");
    return nl->TypeError();
  }

  ListExpr res= nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(nl->StringAtom(idtype)),
                      nl->SymbolAtom(MBool::BasicType()));
  cout<< nl->ToString(res);
  cout.flush();
  return res;
}



/*
2.2 Selection Function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; this has already been checked by the type mapping function.
A selection function is only called if the type mapping was successful. This
makes programming easier as one can rely on a correct structure of the list
~args~.

*/

int
distanceScanSelect( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr searchWindow;
  if(nl->HasLength(args,3)){
        searchWindow = nl->Second(args);
  } else {
        searchWindow = nl->Third(args);
  }
  ListExpr  errorInfo = nl->OneElemList( nl->SymbolAtom( Symbol::ERRORS() ) );

  if (nl->SymbolValue(searchWindow) == Rectangle<2>::BasicType() ||
      algMgr->CheckKind(Kind::SPATIAL2D(), searchWindow, errorInfo))
    return 0;
  else if (nl->SymbolValue(searchWindow) == Rectangle<3>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL3D(), searchWindow, errorInfo))
    return 1;
  else if (nl->SymbolValue(searchWindow) == Rectangle<4>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL4D(), searchWindow, errorInfo))
    return 2;
  else if (nl->SymbolValue(searchWindow) == Rectangle<8>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL8D(), searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
}

int distanceScan3Select(ListExpr args){
   // type indxed by the rtree
   string t1 = nl->SymbolValue(nl->Third(nl->First(args)));
   // typr of query object
   string t2 = nl->SymbolValue(nl->Third(args));

   if(t1==Point::BasicType()){
     if(t2==Point::BasicType()) return 0;
     if(t2==Points::BasicType()) return 1;
     if(t2==Line::BasicType()) return 2;
     if(t2==Region::BasicType()) return 3;
     if(t2==Rectangle<2>::BasicType()) return 4;
   }
   if(t1==Points::BasicType()){
     if(t2==Point::BasicType()) return 5;
     if(t2==Points::BasicType()) return 6;
     if(t2==Line::BasicType()) return 7;
     if(t2==Region::BasicType()) return 8;
     if(t2==Rectangle<2>::BasicType()) return 9;
   }
   if(t1==Line::BasicType()){
     if(t2==Point::BasicType()) return 10;
     if(t2==Points::BasicType()) return 11;
     if(t2==Line::BasicType()) return 12;
     if(t2==Region::BasicType()) return 13;
     if(t2==Rectangle<2>::BasicType()) return 14;
   }
   if(t1==Region::BasicType()){
     if(t2==Point::BasicType()) return 15;
     if(t2==Points::BasicType()) return 16;
     if(t2==Line::BasicType()) return 17;
     if(t2==Region::BasicType()) return 18;
     if(t2==Rectangle<2>::BasicType()) return 19;
   }
   if(t1==Rectangle<2>::BasicType()){
     if(t2==Point::BasicType()) return 20;
     if(t2==Points::BasicType()) return 21;
     if(t2==Line::BasicType()) return 22;
     if(t2==Region::BasicType()) return 23;
     if(t2==Rectangle<2>::BasicType()) return 24;
   }
   return 0;
}


int knearestdistSelect(ListExpr args)
{
  string msg = "stream x attrname x mpoint x int expected";
  if(nl->ListLength(args)!=4){
    ErrorReporter::ReportError(msg + "(invalid number of arguments");
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);
  ListExpr mpoint = nl->Third(args);
  ListExpr card = nl->Fourth(args);

  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError(msg+" (first arg is not a stream)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
    return nl->TypeError();
  }

  if(nl->IsEqual(card,CcReal::BasicType())){
    return 0;
  }

  if(nl->IsEqual(card,CcInt::BasicType())){
    return 1;
  }

  return -1;


}

int KnearestSelect(ListExpr args)
{

  if(nl->ListLength(args) == 4){
    return 0;
  }

  if(nl->ListLength(args) == 5){
    return 1;
  }

  return -1;


}
/*
2.3 Value Mapping Functions

For any operator a value mapping function must be defined. It contains
the code which gives an operator its functionality

The struct DistanceScanLocalInfo is needed to save the data
from one to next function call

*/

template <unsigned dim, class LeafInfo>
struct DistanceScanLocalInfo
{
  Relation* relation;
  R_Tree<dim, TupleId>* rtree;
  BBox<dim> position;
  int quantity, noFound;
  bool scanFlag;
};



/*
The ~distanceScan~ results a stream of all input tuples in the
right order. It returns the k tuples with the lowest distance to a given
reference point. The are ordered by distance to the given reference point.
If the last parameter k has a value <= 0, all tuples of the input stream
will returned.

*/

template <unsigned dim>
int distanceScanFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  DistanceScanLocalInfo<dim, TupleId> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new DistanceScanLocalInfo<dim, TupleId>;
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      StandardSpatialAttribute<dim>* pos =
          (StandardSpatialAttribute<dim> *)args[2].addr;
      localInfo->position = pos->BoundingBox();

      localInfo->quantity = ((CcInt*)args[3].addr)->GetIntval();
      localInfo->noFound = 0;
      localInfo->scanFlag = true;

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);

      localInfo->rtree->FirstDistanceScan(localInfo->position);

      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      if(!local.addr){
        return CANCEL;
      }
      localInfo = (DistanceScanLocalInfo<dim, TupleId>*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( localInfo->quantity > 0 && localInfo->noFound >=localInfo->quantity)
      {
        return CANCEL;
      }

      TupleId tid;
      if ( localInfo->rtree->NextDistanceScan( localInfo->position, tid ) )
      {
          Tuple *tuple = localInfo->relation->GetTuple(tid, false);
          result = SetWord(tuple);
          ++localInfo->noFound;
          return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      if(local.addr){
        localInfo = (DistanceScanLocalInfo<dim, TupleId>*)local.addr;
        localInfo->rtree->LastDistanceScan();
        delete localInfo;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;

}


/* alternative implementation using only standard functions of the rtree */
template <unsigned dim>
class DSEntry{
  public:

/*
~constructors~

*/
    DSEntry(const R_TreeNode<dim, TupleId>&  node1,
            const StandardSpatialAttribute<dim>* qo1,
            const int level1):
       isTuple(false), node( new R_TreeNode<dim, TupleId>(node1)),
       tupleId(0),level(level1){
       distance = qo1->Distance(node1.BoundingBox());
    }

    DSEntry(const TupleId id,
            const BBox<dim>& rect,
            const StandardSpatialAttribute<dim>* qo,
            const int level1):
       isTuple(true), node(0), tupleId(id), level(level1){
       distance = qo->Distance(rect);
    }

    DSEntry(const DSEntry<dim>& src):
      isTuple(src.isTuple), node(0), tupleId(src.tupleId),
      level(src.level), distance(src.distance){
      if(src.node){
         node = new R_TreeNode<dim, TupleId>(*src.node);
      }
    }

/*
~Assignment operator~

*/
    DSEntry<dim>& operator=(const DSEntry<dim>& src){
      isTuple = src.isTuple;
      if(node){
        delete node;
      }
      if(src.node){
        node = new R_TreeNode<dim, TupleId>(*src.node);
      } else {
        node = 0;
      }
      tupleId = src.tupleId;
      level = src.level;
      distance = src.distance;
      return *this;
    }

/*
~Destructor~

*/

    ~DSEntry(){ delete node;}


/*
Check for a tuple entry.

*/
    bool isTupleEntry() const{
       return isTuple;
    }

    bool getLevel() const{
       return level;
    }

    TupleId getTupleId() const{
     return tupleId;
    }

    const R_TreeNode<dim,TupleId>* getNode() const {
       return  node;
    }

    inline int compare(const DSEntry<dim>& e) const{
       if(distance < e.distance){
          return -1;
       }
       if(distance > e.distance){
          return 1;
       }
       return 0;
    }

  private:
    bool isTuple;
    R_TreeNode<dim, TupleId>* node;
    TupleId tupleId;
    int level;
    double distance;
};

template<unsigned dim>
class DSEComp {
public:
   bool operator()(const DSEntry<dim>* d1, const DSEntry<dim>* d2) const{
       return d1->compare(*d2) >= 0;
   }
};



template <unsigned dim>
class DSLocalInfo{
public:
   DSLocalInfo( R_Tree<dim, TupleId>* rtree, Relation* rel,
               StandardSpatialAttribute<dim>* queryObj, CcInt* k){
     // check if all things are defined
     if( (rel!=0 && rel->GetNoTuples()==0) ||
         !queryObj->IsDefined() ||
         queryObj->IsEmpty() ||
         !k->IsDefined()){
        this->tree=0;
        this->rel=0;
        this->queryObj = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }

      this->tree = rtree;
      this->rel   = rel;
      this->queryObj = queryObj;
      this->k = k->GetIntval();
      returned = 0;
      R_TreeNode<dim, TupleId> root = rtree->Root();
      // check for an emty tree
      BBox<dim> box = root.BoundingBox();
      if(!box.IsDefined()){
        this->tree=0;
        this->rel=0;
        this->queryObj = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }
      height = tree->Height();
      minInnerEntries = tree->MinEntries(0);
      maxInnerEntries = tree->MaxEntries(0);
      minLeafEntries = tree->MinEntries(height);
      maxLeafEntries = tree->MaxEntries(height);
      DSEntry<dim>* first = new DSEntry<dim>(root,queryObj, 0);
      dsqueue.push(first);
   }

   ~DSLocalInfo(){
      while(!dsqueue.empty()){
         DSEntry<dim>* t = dsqueue.top();
         delete t;
         dsqueue.pop();
      }
    }


   TupleId* nextTupleID1(){
     if(!tree){
       return 0;
     }
     if(k>0 && returned==k){
       return 0;
     }
     if(dsqueue.empty()){
       return 0;
     }


     DSEntry<dim>* top = dsqueue.top();
     // replace top entry of the queue by its sons
     // until it's a tuple entry
     while(!top->isTupleEntry()){
       dsqueue.pop(); // remove top element
       int level = top->getLevel();
       // insert all sons
       int minEntries,maxEntries;
       level++; // level for the sons is increased

       if(level>=height){
         minEntries = this->minLeafEntries;
         maxEntries = this->maxLeafEntries;
       } else {
         minEntries = this->minInnerEntries;
         maxEntries = this->maxInnerEntries;
       }

       if(!top->getNode()->IsLeaf()){
         for(int i = 0; i< top->getNode()->EntryCount();i++){
            R_TreeInternalEntry<dim>* next=top->getNode()->GetInternalEntry(i);
            SmiRecordId rid = next->pointer;
            R_TreeNode<dim, TupleId> nextNode(true, minEntries, maxEntries);
            tree->GetNode(rid, nextNode);
            DSEntry<dim>* nextEntry =
                  new DSEntry<dim>(nextNode, queryObj, level);
            dsqueue.push(nextEntry);
         }
       } else { // topmost node was a leaf
         for(int i=0;i<top->getNode()->EntryCount();i++){
           R_TreeLeafEntry<dim, TupleId>* le =
                top->getNode()->GetLeafEntry(i);
           DSEntry<dim>* nextEntry =
                new DSEntry<dim>(le->info, le->box, queryObj, level);
           dsqueue.push(nextEntry);
         }
       }
       delete top; // delete the replaced top element
       top = dsqueue.top();
     }
     dsqueue.pop();
     // now we have a tuple
     returned++;
     TupleId* res = new TupleId(top->getTupleId());
     delete top;
     return res;
   }

   Tuple* nextTuple(){
       TupleId* id = nextTupleID1();
       if(id==0){
          return 0;
       }  else {
         Tuple* res = rel->GetTuple(*id, false);
         delete id;
         return res;
       }
   } 

   TupleIdentifier* nextTupleID(){
       TupleId* id = nextTupleID1();
       if(id==0){
          return 0;
       }  else {
         TupleIdentifier* res = new TupleIdentifier(true,*id);
         delete id;
         return res;
       }
   } 



private:
   R_Tree<dim, TupleId>* tree;
   Relation* rel;
   StandardSpatialAttribute<dim>* queryObj;
   int k;
   int returned; // number of returned tuples
   int height;
   int minInnerEntries;
   int maxInnerEntries;
   int minLeafEntries;
   int maxLeafEntries;
   priority_queue< DSEntry<dim>*,
                   vector<DSEntry<dim>* >,
                   DSEComp<dim>  > dsqueue;
};


template <unsigned dim>
int distanceScan2Fun (Word* args, Word& result, int message,
             Word& local, Supplier s){
  switch(message){
    case OPEN : {
       if(local.addr){
         DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
         delete li;
       }
       R_Tree<dim, TupleId>* rtree =
                    static_cast<R_Tree<dim, TupleId> *>(args[0].addr);
       Relation* rel = static_cast<Relation*>(args[1].addr);
       StandardSpatialAttribute<dim>* queryObj =
             static_cast<StandardSpatialAttribute<dim>*>(args[2].addr);
       CcInt* k = static_cast<CcInt*>(args[3].addr);
       local.addr = new DSLocalInfo<dim>(rtree, rel, queryObj, k);
       return 0;
    }

    case REQUEST: {
      if(!local.addr){
        return CANCEL;
      } else {
        DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
        result.addr = li->nextTuple();
        return result.addr ? YIELD : CANCEL;
      }
    }

    case CLOSE:{
      if(local.addr){
        DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
        delete li;
        local.addr = 0;
      }
      return 0;
    }
    default : assert(false);
  }
}


template <unsigned dim>
int distanceScan2SFun (Word* args, Word& result, int message,
             Word& local, Supplier s){
  switch(message){
    case OPEN : {
       if(local.addr){
         DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
         delete li;
       }
       R_Tree<dim, TupleId>* rtree =
                    static_cast<R_Tree<dim, TupleId> *>(args[0].addr);
       StandardSpatialAttribute<dim>* queryObj =
             static_cast<StandardSpatialAttribute<dim>*>(args[1].addr);
       CcInt* k = static_cast<CcInt*>(args[2].addr);
       local.addr = new DSLocalInfo<dim>(rtree, 0, queryObj, k);
       return 0;
    }

    case REQUEST: {
      if(!local.addr){
        return CANCEL;
      } else {
        DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
        result.addr = li->nextTupleID();
        return result.addr ? YIELD : CANCEL;
      }
    }

    case CLOSE:{
      if(local.addr){
        DSLocalInfo<dim>* li = static_cast<DSLocalInfo<dim>*>(local.addr);
        delete li;
        local.addr = 0;
      }
      return 0;
    }
    default : assert(false);
  }
}


/*
~distanceScan 3~

The distance scan 3 works for all 2 dimensional spatial attributes.

*/

template <unsigned dim, class IndexedType, class QueryType, class DistFun>
class DS3Entry{
  public:

/*
~constructors~

*/
    DS3Entry(const R_TreeNode<dim, TupleId>&  node1,
             const QueryType* qo1,
             const int level1):
       isTuple(false), node( new R_TreeNode<dim, TupleId>(node1)),
       tupleId(0),level(level1){
       distance = qo1->Distance(node1.BoundingBox());
    }

    DS3Entry(const TupleId id,
            const BBox<dim>& rect,
            const QueryType* qo,
            const int level1,
            const Relation* relation,
            const int attrpos,
            const DistFun& df):
       isTuple(true), node(0), tupleId(id), level(level1){

       Tuple* tuple = relation->GetTuple(id,false);
       const IndexedType* obj =
             static_cast<IndexedType*>(tuple->GetAttribute(attrpos));
       distance = df(*qo,*obj);
       tuple->DeleteIfAllowed();
    }

    DS3Entry(const DS3Entry<dim, IndexedType, QueryType, DistFun>& src):
      isTuple(src.isTuple), node(0), tupleId(src.tupleId),
      level(src.level), distance(src.distance){
      if(src.node){
         node = new R_TreeNode<dim, TupleId>(*src.node);
      }
    }

/*
~Assignment operator~

*/
    DS3Entry<dim,IndexedType, QueryType, DistFun>&
       operator=(const DS3Entry<dim, IndexedType, QueryType, DistFun>& src){
      isTuple = src.isTuple;
      if(node){
        delete node;
      }
      if(src.node){
        node = new R_TreeNode<dim, TupleId>(*src.node);
      } else {
        node = 0;
      }
      tupleId = src.tupleId;
      level = src.level;
      distance = src.distance;
      return *this;
    }

/*
~Destructor~

*/

    ~DS3Entry(){ delete node;}


/*
Check for a tuple entry.

*/
    bool isTupleEntry() const{
       return isTuple;
    }

    bool getLevel() const{
       return level;
    }

    TupleId getTupleId() const{
     return tupleId;
    }

    const R_TreeNode<dim,TupleId>* getNode() const {
       return  node;
    }

    inline int compare(const DS3Entry<dim, IndexedType,
                                      QueryType, DistFun>& e) const{
       if(distance < e.distance){
          return -1;
       }
       if(distance > e.distance){
          return 1;
       }
       return 0;
    }

    double getDistance() const{
      return distance;
    }

  private:
    bool isTuple;
    R_TreeNode<dim, TupleId>* node;
    TupleId tupleId;
    int level;
    double distance;
};

template<unsigned dim, class IndexedType, class QueryType, class DistFun>
class DSE3Comp {
public:
   bool operator()(
       const DS3Entry<dim, IndexedType, QueryType, DistFun>* d1,
       const DS3Entry<dim, IndexedType, QueryType, DistFun>* d2) const{
       return d1->compare(*d2) >= 0;
   }
};

template <unsigned dim, class IndexedType, class QueryType, class DistFun>
class DS3LocalInfo{
public:
   DS3LocalInfo( R_Tree<dim, TupleId>* rtree, Relation* rel,
                QueryType* queryObj, CcInt* k, int position,
                CcReal& _maxDist){

     // process maxDist
     if(!_maxDist.IsDefined()){
        useMaxDist = false;
        maxDist = 0.0;
     } else {
        useMaxDist = true;
        maxDist = _maxDist.GetValue(); 
     }

     // check if all things are defined
     this->position = position;
     if(rel->GetNoTuples()==0 ||
         !queryObj->IsDefined() ||
         queryObj->IsEmpty() ||
         !k->IsDefined()){
        this->tree=0;
        this->rel=0;
        this->queryObj = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }
      this->tree = rtree;
      this->rel   = rel;
      this->queryObj = queryObj;
      this->k = k->GetIntval();
      returned = 0;
      R_TreeNode<dim, TupleId> root = rtree->Root();
      // check for an emty tree
      BBox<dim> box = root.BoundingBox();
      if(!box.IsDefined()){
        this->tree=0;
        this->rel=0;
        this->queryObj = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }
      height = tree->Height();
      minInnerEntries = tree->MinEntries(0);
      maxInnerEntries = tree->MaxEntries(0);
      minLeafEntries = tree->MinEntries(height);
      maxLeafEntries = tree->MaxEntries(height);
      DS3Entry<dim, IndexedType, QueryType, DistFun>* first =
          new DS3Entry<dim,IndexedType, QueryType, DistFun>(root,queryObj, 0);
      dsqueue.push(first);
   }

   ~DS3LocalInfo(){
      while(!dsqueue.empty()){
         DS3Entry<dim, IndexedType, QueryType, DistFun>* t = dsqueue.top();
         delete t;
         dsqueue.pop();
      }
    }


   Tuple* nextTuple(){
     if(!tree){
       return 0;
     }
     if(k>0 && returned==k){
       return 0;
     }
     if(dsqueue.empty()){
       return 0;
     }


     DS3Entry<dim, IndexedType, QueryType, DistFun>* top = dsqueue.top();
     // replace top entry of the queue by its sons
     // until it's a tuple entry
     while(!top->isTupleEntry()){
       dsqueue.pop(); // remove top element
       int level = top->getLevel();
       // insert all sons
       int minEntries,maxEntries;
       level++; // level for the sons is increased

       if(level>=height){
         minEntries = this->minLeafEntries;
         maxEntries = this->maxLeafEntries;
       } else {
         minEntries = this->minInnerEntries;
         maxEntries = this->maxInnerEntries;
       }

       if(!top->getNode()->IsLeaf()){
         for(int i = 0; i< top->getNode()->EntryCount();i++){
            R_TreeInternalEntry<dim>* next=top->getNode()->GetInternalEntry(i);
            SmiRecordId rid = next->pointer;
            R_TreeNode<dim, TupleId> nextNode(true, minEntries, maxEntries);
            tree->GetNode(rid, nextNode);
            DS3Entry<dim, IndexedType, QueryType, DistFun>* nextEntry =
                  new DS3Entry<dim, IndexedType,
                               QueryType, DistFun>(nextNode, queryObj, level);
            dsqueue.push(nextEntry);
         }
       } else { // topmost node was a leaf
         for(int i=0;i<top->getNode()->EntryCount();i++){
           R_TreeLeafEntry<dim, TupleId>* le =
                top->getNode()->GetLeafEntry(i);
           DS3Entry<dim, IndexedType, QueryType, DistFun>* nextEntry =
                new DS3Entry<dim, IndexedType,
                             QueryType, DistFun>(le->info, le->box,
                               queryObj, level, rel, position, distFun);
           dsqueue.push(nextEntry);
         }
       }
       delete top; // delete the replaced top element
       top = dsqueue.top();
     }
     dsqueue.pop();

     if(useMaxDist && (top->getDistance()>maxDist)){ 
       //cout << "maxDist reached" << endl;
       //cout << "currentDist = " << top->getDistance();
       //cout << "maxDist = " << maxDist << endl;
       k = returned;
       delete top;
       return 0;
     }

     // now we have a tuple
     returned++;
     Tuple* res = rel->GetTuple(top->getTupleId(), false);
     delete top;
     return res;
   }
private:
   R_Tree<dim, TupleId>* tree;
   Relation* rel;
   int position;
   const QueryType* queryObj;
   int k;
   int returned; // number of returned tuples
   int height;
   int minInnerEntries;
   int maxInnerEntries;
   int minLeafEntries;
   int maxLeafEntries;
   priority_queue< DS3Entry<dim, IndexedType, QueryType, DistFun>*,
                   vector<DS3Entry<dim, IndexedType, QueryType, DistFun>* >,
                   DSE3Comp<dim, IndexedType, QueryType, DistFun>  > dsqueue;
   DistFun distFun;

   bool useMaxDist;
   double maxDist;

};

template <unsigned dim, class IndexedType, class QueryType, class DistFun>
int distanceScan3Fun (Word* args, Word& result, int message,
             Word& local, Supplier s){
  switch(message){
    case OPEN : {
       if(local.addr){
         DS3LocalInfo<dim, IndexedType, QueryType, DistFun>* li =
             static_cast<DS3LocalInfo<dim, IndexedType,
                         QueryType, DistFun>*>(local.addr);
         delete li;
       }
       R_Tree<dim, TupleId>* rtree =
                    static_cast<R_Tree<dim, TupleId> *>(args[0].addr);
       Relation* rel = static_cast<Relation*>(args[1].addr);
       QueryType* queryObj = static_cast<QueryType*>(args[2].addr);
       CcInt* k = static_cast<CcInt*>(args[3].addr);
       int index = qp->GetNoSons(s)-2;
       int pos = (static_cast<CcInt*>(args[index].addr))->GetIntval();
       
       bool useMaxDist = (static_cast<CcBool*>
                          (args[qp->GetNoSons(s)-1].addr))->GetValue();
       CcReal maxD(false);
       if(useMaxDist){
          maxD.CopyFrom( (CcReal*)args[qp->GetNoSons(s)-3].addr);
       }
         

       local.addr = new
           DS3LocalInfo<dim,IndexedType,QueryType, DistFun>(rtree,
                                                rel, queryObj, k, pos, maxD);
       return 0;
    }

    case REQUEST: {
      if(!local.addr){
        return CANCEL;
      } else {
        DS3LocalInfo<dim, IndexedType, QueryType, DistFun>* li =
              static_cast<DS3LocalInfo<dim, IndexedType,
                                       QueryType,DistFun>*>(local.addr);
        result.addr = li->nextTuple();
        return result.addr ? YIELD : CANCEL;
      }
    }

    case CLOSE:{
      if(local.addr){
        DS3LocalInfo<dim, IndexedType, QueryType, DistFun>* li =
             static_cast<DS3LocalInfo<dim, IndexedType,
                                     QueryType, DistFun>*>(local.addr);
        delete li;
        local.addr = 0;
      }
      return 0;
    }
    default : assert(false);
  }
}


/*
Distancescan4 compute the k nearest mpoints for a given point for
a certain instant.

  rtree(upoint) x rel x point x instant x int x attrname x attrpos

*/

class DS4Entry{
  public:

/*
~constructors~

*/
    DS4Entry(const R_TreeNode<3, TupleId>&  node1,
             const Point* qo1,
             const int level1,
             double instantAsDouble):
       isTuple(false), node( new R_TreeNode<3, TupleId>(node1)),
       tupleId(0),level(level1){
       Rectangle<3> r(node1.BoundingBox());
       if(!Contains(r,instantAsDouble)){
         distance = -1;
       } else {
         distance = qo1->Distance(project(node1.BoundingBox()));
       }
    }


    DS4Entry(const TupleId id,
            const BBox<3>& rect,
            const Point* qo,
            const int level1,
            const Relation* relation,
            const int attrpos,
            const Instant& instant):
       isTuple(true), node(0), tupleId(id), level(level1){

       Tuple* tuple = relation->GetTuple(id, false);
       const UPoint* obj =
             static_cast<UPoint*>(tuple->GetAttribute(attrpos));
       distance = computeDistance(obj, qo, instant);
       tuple->DeleteIfAllowed();
    }

    DS4Entry(const DS4Entry& src):
      isTuple(src.isTuple), node(0), tupleId(src.tupleId),
      level(src.level), distance(src.distance){
      if(src.node){
         node = new R_TreeNode<3, TupleId>(*src.node);
      }
    }

/*
~Assignment operator~

*/
    DS4Entry& operator=(const DS4Entry& src){
      isTuple = src.isTuple;
      if(node){
        delete node;
      }
      if(src.node){
        node = new R_TreeNode<3, TupleId>(*src.node);
      } else {
        node = 0;
      }
      tupleId = src.tupleId;
      level = src.level;
      distance = src.distance;
      return *this;
    }

/*
~Destructor~

*/

    ~DS4Entry(){ delete node;}


/*
Check for a tuple entry.

*/
    bool isTupleEntry() const{
       return isTuple;
    }

    bool getLevel() const{
       return level;
    }

    TupleId getTupleId() const{
     return tupleId;
    }

    double getDistance() const { return distance; }

    const R_TreeNode<3,TupleId>* getNode() const {
       return  node;
    }

    inline int compare(const DS4Entry& e) const{
       if(distance < e.distance){
          return -1;
       }
       if(distance > e.distance){
          return 1;
       }
       return 0;
    }

  private:
    bool isTuple;
    R_TreeNode<3, TupleId>* node;
    TupleId tupleId;
    int level;
    double distance;


    inline Rectangle<2> project(const Rectangle<3> r3){
       Rectangle<2> result(true, r3.MinD(0),r3.MaxD(0),r3.MinD(1),r3.MaxD(1));
       return result;
    }

    inline double computeDistance(const UPoint* up,
                                const  Point* p,
                                const Instant& i){


        if(!up->timeInterval.Contains(i)){
           return -1;
        }
        Point p2(0,0);
        up->TemporalFunction(i, p2);
        return p->Distance(p2);
    }

/*
Checks whether the instant is inside the time interval stored within r.

*/
    inline bool Contains(const Rectangle<3> r, double instant){
      return (r.MinD(2) <= instant) && (instant <= r.MaxD(2));
    }

};

class DSE4Comp {
public:
   bool operator()(
       const DS4Entry* d1,
       const DS4Entry* d2) const{
       return d1->compare(*d2) >= 0;
   }
};

class DS4LocalInfo{
public:
   DS4LocalInfo( R_Tree<3, TupleId>* rtree,
                 Relation* rel,
                 Point* point,
                 Instant* instant,
                 CcInt* k, int position){
     // check if all things are defined
     this->position = position;
     if(rel->GetNoTuples()==0 ||
         !point->IsDefined() ||
         !instant->IsDefined() ||
         !k->IsDefined()){
        this->tree=0;
        this->rel=0;
        this->point = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }
      this->tree = rtree;
      this->rel   = rel;
      this->point = point;
      this->k = k->GetIntval();
      this->instant = *instant;
      this->instantAsDouble = instant->ToDouble();
      returned = 0;
      R_TreeNode<3, TupleId> root = rtree->Root();
      // check for an empty tree
      BBox<3> box = root.BoundingBox();
      if(!box.IsDefined()){
        this->tree=0;
        this->rel=0;
        this->point = 0;
        this->k = 0;
        this->returned = 0;
        return;
      }
      height = tree->Height();
      minInnerEntries = tree->MinEntries(0);
      maxInnerEntries = tree->MaxEntries(0);
      minLeafEntries = tree->MinEntries(height);
      maxLeafEntries = tree->MaxEntries(height);
      DS4Entry* first = new DS4Entry(root,point, 0, instantAsDouble);
      if(first->getDistance()>=0){
         dsqueue.push(first);
      }
   }

   ~DS4LocalInfo(){
      while(!dsqueue.empty()){
         DS4Entry* t = dsqueue.top();
         delete t;
         dsqueue.pop();
      }
    }


   Tuple* nextTuple(){
     if(!tree){
       return 0;
     }
     if(k>0 && returned==k){
       return 0;
     }
     if(dsqueue.empty()){
       return 0;
     }


     DS4Entry* top = dsqueue.top();
     // replace top entry of the queue by its sons
     // until it's a tuple entry
     while(!dsqueue.empty() && !top->isTupleEntry()){
       dsqueue.pop(); // remove top element
       int level = top->getLevel();
       // insert all sons
       int minEntries,maxEntries;
       level++; // level for the sons is increased

       if(level>=height){
         minEntries = this->minLeafEntries;
         maxEntries = this->maxLeafEntries;
       } else {
         minEntries = this->minInnerEntries;
         maxEntries = this->maxInnerEntries;
       }

       if(!top->getNode()->IsLeaf()){
         for(int i = 0; i< top->getNode()->EntryCount();i++){
            R_TreeInternalEntry<3>* next=top->getNode()->GetInternalEntry(i);
            SmiRecordId rid = next->pointer;
            R_TreeNode<3, TupleId> nextNode(true, minEntries, maxEntries);
            tree->GetNode(rid, nextNode);
            DS4Entry* nextEntry = new DS4Entry(nextNode, point,
                                               level, instantAsDouble);
            if(nextEntry->getDistance() >=0){
               dsqueue.push(nextEntry);
            } else {
               delete nextEntry;
            }
         }
       } else { // topmost node was a leaf
         for(int i=0;i<top->getNode()->EntryCount();i++){
           R_TreeLeafEntry<3, TupleId>* le =
                top->getNode()->GetLeafEntry(i);
           DS4Entry* nextEntry = new DS4Entry(le->info, le->box,
                               point, level, rel, position, instant);
           if(nextEntry->getDistance()>=0){
              dsqueue.push(nextEntry);
           } else {
             delete nextEntry;
           }
         }
       }
       delete top; // delete the replaced top element
       if(!dsqueue.empty()){
           top = dsqueue.top();
       }
     }
     if(dsqueue.empty()){
       return 0;
     }
     dsqueue.pop();
     // now we have a tuple
     returned++;
     Tuple* res = rel->GetTuple(top->getTupleId(), false);
     delete top;
     return res;
   }
private:
   R_Tree<3, TupleId>* tree;
   Relation* rel;
   int position;
   const Point* point;
   int k;
   int returned; // number of returned tuples
   int height;
   int minInnerEntries;
   int maxInnerEntries;
   int minLeafEntries;
   int maxLeafEntries;
   priority_queue< DS4Entry*,
                   vector<DS4Entry* >,
                   DSE4Comp  > dsqueue;
   Instant instant;
   double instantAsDouble;
};



int distanceScan4Fun (Word* args, Word& result, int message,
             Word& local, Supplier s){

  switch(message){
    case OPEN: {
        R_Tree<3, TupleId>* rtree =
                 static_cast<R_Tree<3,TupleId>*> (args[0].addr);
        Relation* rel   = static_cast<Relation*>(args[1].addr);
        Point*    point = static_cast<Point*>(args[2].addr);
        Instant*  instant = static_cast<Instant*>(args[3].addr);
        CcInt*    k =  static_cast<CcInt*>(args[4].addr);
        // ignore attrname
        int attrpos = (static_cast<CcInt*>(args[6].addr))->GetIntval()-1;
        if(!point->IsDefined() || !instant->IsDefined() || !k->IsDefined()){
           local.setAddr(0);
        } else {
          local.addr = new DS4LocalInfo(rtree, rel, point, instant,
                                        k, attrpos);
        }
        return 0;
     }
    case REQUEST: {
        if(!local.addr){
          return CANCEL;
        }
        DS4LocalInfo* li = static_cast<DS4LocalInfo*>(local.addr);
        result.addr = li->nextTuple();
        return result.addr ? YIELD : CANCEL;
     }
    case CLOSE: {
        if(local.addr){
           DS4LocalInfo* li = static_cast<DS4LocalInfo*>(local.addr);
           delete li;
           local.addr = 0;
        }
        return 0;
     }
    default: assert(false);
  }
  return 0;
}


/*
The ~knearest~ operator results a stream of all input tuples which
contains the k-nearest units to the given mpoint. The tuples are splitted
into multiple tuples with disjoint units if necessary. The tuples in the
result stream are not necessarily ordered by time or distance to the
given mpoint

The struct knearestLocalInfo is needed to save the data
from one to next function call
the type EventElem and ActiveElem are defined in NearestNeighborAlgebra.h

*/
class KNearestQueue;
Instant ActiveElem::currtime(instanttype);
typedef vector< ActiveElem >::iterator ITV;
typedef NNTree< ActiveElem >::iter IT;

/*
GetPosition calculates the position of a upoint at a specific time

*/
bool GetPosition( const UPoint& up, Instant t, Coord& x, Coord& y)
{
  //calculates the pos. at time t. x and y is the result
  Instant t0 = up.timeInterval.start;
  Instant t1 = up.timeInterval.end;
  if( t == t0 )
  {
    x = up.p0.GetX();
    y = up.p0.GetY();
    return true;
  }
  if( t == t1 )
  {
    x = up.p1.GetX();
    y = up.p1.GetY();
    return true;
  }
  if( t < t0 || t > t1 ){ return false;}
  double factor = (t - t0) / (t1 - t0);
  x = up.p0.GetX() + factor * (up.p1.GetX() - up.p0.GetX());
  y = up.p0.GetY() + factor * (up.p1.GetY() - up.p0.GetY());
  return true;
}

/*
the function GetDistance calculates the distance.
Result is the MReal erg. The function expects a pointer to an empty MReal.
mpos ist the startposition in mp where the function start to look
This is a optimization cause the upoints are sorted
by time. A lower time cannot come

*/
void GetDistance( const MPoint* mp, const UPoint* up,
                   int &mpos, MReal *erg)
{

  Instant time1 = up->timeInterval.start;
  Instant time2 = up->timeInterval.end;

  int noCo = mp->GetNoComponents();
  UPoint upn;

  for (int ii=mpos; ii < noCo; ii++)
  {
    mp->Get( ii, upn);  // get the current Unit
    if(time2 < upn.timeInterval.start ||
      time1 > upn.timeInterval.end ||
      (time2 == upn.timeInterval.start && up->timeInterval.rc == false))
      continue;

    if( time1 < upn.timeInterval.end ||
        (time1 == upn.timeInterval.end && upn.timeInterval.rc))
    { // interval of mp intersects the interval of up
      mpos = ii;
      UReal firstu(true);

      // take the bigger starting point
      Instant start = up->timeInterval.start < upn.timeInterval.start
        ? upn.timeInterval.start : up->timeInterval.start;

      // take the smaller end
      Instant end = up->timeInterval.end < upn.timeInterval.end
        ? up->timeInterval.end : upn.timeInterval.end;

      bool lc = up->timeInterval.lc;
      if( lc && (start > up->timeInterval.start || (up->timeInterval.start
        == upn.timeInterval.start && !up->timeInterval.lc)))
      {
        lc = false;
      }
      bool rc = up->timeInterval.rc;
      if( rc && (end < up->timeInterval.end || (up->timeInterval.end
        == upn.timeInterval.end && !upn.timeInterval.rc)))
      {
        rc = false;
      }

      Interval<Instant> iv(start, end, lc, rc);


      Coord x1, y1, x2, y2;
      GetPosition( *up, start, x1, y1);
      GetPosition( *up, end, x2, y2);
      UPoint up1(iv, x1, y1, x2, y2);
      GetPosition( upn, start, x1, y1);
      GetPosition( upn, end, x2, y2);
      UPoint upMp(iv, x1, y1, x2, y2);
      if( start != end)
      {
        up1.Distance(upMp, firstu);
      }
      else
      {
        //function Distance(UPoint...) has a bug if
        //starttime and endtime are equal
        Point rp1;
        upMp.TemporalFunction( start, rp1, true);
        up1.Distance(rp1,firstu);
      }
      erg->Add( firstu );
      int jj = ii;
      while( ++jj < noCo && (time2 > end
        || (time2 == end && !rc && up->timeInterval.rc)))
      {
        mp->Get( jj, upn);
        UReal nextu(true);
        start = end;
        lc = true;
        end = up->timeInterval.end < upn.timeInterval.end
        ? up->timeInterval.end : upn.timeInterval.end;
        rc = up->timeInterval.rc;
        if( rc && (end < up->timeInterval.end || (up->timeInterval.end
          == upn.timeInterval.end && !upn.timeInterval.rc)))
        {
          rc = false;
        }
        Interval<Instant> iv(start, end, lc, rc);
        Coord x1, y1, x2, y2;
        GetPosition( *up, start, x1, y1);
        GetPosition( *up, end, x2, y2);
        UPoint up1(iv, x1, y1, x2, y2);
        GetPosition( upn, start, x1, y1);
        GetPosition( upn, end, x2, y2);
        UPoint upMp(iv, x1, y1, x2, y2);
        if( start != end)
        {
          up1.Distance(upMp, nextu);
        }
        else
        {
          //function Distance(UPoint...) has a bug if
          //starttime and endtime are equal
          Point rp1;
          upMp.TemporalFunction( start, rp1, true);
          up1.Distance(rp1,nextu);
        }
        erg->Add( nextu );
     }
      break;
    }
  }
}

/*
input location data has a zone value

*/
void GetDistance2( const MPoint* mp, const UPoint* up, int &mpos, MReal *erg, 
                   int zone)
{
  WGSGK gk;
  gk.setMeridian(zone);

  Instant time1 = up->timeInterval.start;
  Instant time2 = up->timeInterval.end;

  int noCo = mp->GetNoComponents();
  UPoint upn;

  for (int ii=mpos; ii < noCo; ii++)
  {
    mp->Get( ii, upn);  // get the current Unit
    if(time2 < upn.timeInterval.start ||
      time1 > upn.timeInterval.end ||
      (time2 == upn.timeInterval.start && up->timeInterval.rc == false))
      continue;

    if( time1 < upn.timeInterval.end ||
        (time1 == upn.timeInterval.end && upn.timeInterval.rc))
    { // interval of mp intersects the interval of up
      mpos = ii;
      UReal firstu(true);

      // take the bigger starting point
      Instant start = up->timeInterval.start < upn.timeInterval.start
        ? upn.timeInterval.start : up->timeInterval.start;

      // take the smaller end
      Instant end = up->timeInterval.end < upn.timeInterval.end
        ? up->timeInterval.end : upn.timeInterval.end;

      bool lc = up->timeInterval.lc;
      if( lc && (start > up->timeInterval.start || (up->timeInterval.start
        == upn.timeInterval.start && !up->timeInterval.lc)))
      {
        lc = false;
      }
      bool rc = up->timeInterval.rc;
      if( rc && (end < up->timeInterval.end || (up->timeInterval.end
        == upn.timeInterval.end && !upn.timeInterval.rc)))
      {
        rc = false;
      }

      Interval<Instant> iv(start, end, lc, rc);


      Coord x1, y1, x2, y2;
      GetPosition( *up, start, x1, y1);
      GetPosition( *up, end, x2, y2);
//      UPoint up1(iv, x1, y1, x2, y2);
      //////////////////////////////////////////////////
      Point p1(true, x1, y1);
      Point p2(true, x2, y2);
//      UPoint up1(iv, p1, p2);
      Point newp1, newp2;
      gk.project(p1, newp1);
      gk.project(p2, newp2);
      UPoint up1(iv, newp1, newp2);
      //////////////////////////////////////////////////

      GetPosition( upn, start, x1, y1);
      GetPosition( upn, end, x2, y2);
//      UPoint upMp(iv, x1, y1, x2, y2);
      //////////////////////////////////////////////////
      Point q1(true, x1, y1);
      Point q2(true, x2, y2);
//      UPoint upMp(iv, q1, q2);
      Point newq1, newq2;
      gk.project(q1, newq1);
      gk.project(q2, newq2);
      UPoint upMp(iv, newq1, newq2);
      /////////////////////////////////////////////////
      if( start != end)
      {
        up1.Distance(upMp, firstu);
      }
      else
      {
        //function Distance(UPoint...) has a bug if
        //starttime and endtime are equal
        Point rp1;
        upMp.TemporalFunction( start, rp1, true);
        up1.Distance(rp1,firstu);
      }
      erg->Add( firstu );
      int jj = ii;
      while( ++jj < noCo && (time2 > end
        || (time2 == end && !rc && up->timeInterval.rc)))
      {
        mp->Get( jj, upn);
        UReal nextu(true);
        start = end;
        lc = true;
        end = up->timeInterval.end < upn.timeInterval.end
        ? up->timeInterval.end : upn.timeInterval.end;
        rc = up->timeInterval.rc;
        if( rc && (end < up->timeInterval.end || (up->timeInterval.end
          == upn.timeInterval.end && !upn.timeInterval.rc)))
        {
          rc = false;
        }
        Interval<Instant> iv(start, end, lc, rc);
        Coord x1, y1, x2, y2;
        GetPosition( *up, start, x1, y1);
        GetPosition( *up, end, x2, y2);
//        UPoint up1(iv, x1, y1, x2, y2);
        /////////////////////////////////////////////////////////////
        Point p3(true, x1, y1);
        Point p4(true, x2, y2);
//        UPoint up1(iv, p3, p4);
        Point newp3, newp4;
        gk.project(p3, newp3);
        gk.project(p4, newp4);
        UPoint up1(iv, newp3, newp4);
        /////////////////////////////////////////////////////////////
        GetPosition( upn, start, x1, y1);
        GetPosition( upn, end, x2, y2);
//        UPoint upMp(iv, x1, y1, x2, y2);
        ////////////////////////////////////////////////////////////////
        Point q3(true, x1, y1);
        Point q4(true, x2, y2);
//        UPoint upMp(iv, q3, q4);
        Point newq3, newq4;
        gk.project(q3, newq3);
        gk.project(q4, newq4);
        UPoint upMp(iv, newq3, newq4);
        //////////////////////////////////////////////////////////////
        if( start != end)
        {
          up1.Distance(upMp, nextu);
        }
        else
        {
          //function Distance(UPoint...) has a bug if
          //starttime and endtime are equal
          Point rp1;
          upMp.TemporalFunction( start, rp1, true);
          up1.Distance(rp1,nextu);
        }
        erg->Add( nextu );
     }
      break;
    }
  }
}

/*
CalcDistance calculate the result of the given MReal mr
at the given time t. The value of the derivation (slope)
is also calculated

*/
double CalcDistance( const MReal *mr, Instant t, double &slope)
{
  int noCo = mr->GetNoComponents();
  UReal ur;
  for (int ii=0; ii < noCo; ++ii)
  {
    mr->Get( ii, ur);
    if( t < ur.timeInterval.end
      || (t == ur.timeInterval.end && ur.timeInterval.rc)
      || (t == ur.timeInterval.end && ii+1 == noCo))
    {
      double time = (t - ur.timeInterval.start).ToDouble();
      double erg = ur.a * time * time + ur.b * time + ur.c;
      if( ur.r && erg < 0) erg = 0;
      erg = ( ur.r) ? sqrt(erg) : erg;
      //the slope is if not r: 2a * time + b
      // else 2a * time + b / 2 * erg
      slope = 2 * ur.a * time + ur.b;
      if( ur.r && erg){ slope /= 2 * erg; }
      if( ur.r && !erg && ur.b == 0 && ur.c == 0 && ur.a > 0)
      {
        slope = sqrt(ur.a);
      }
      return erg;
    }
  }
  return -1;
}

double CalcSlope( const UReal& ur, Instant t)
{
  if( t >= ur.timeInterval.start && t <= ur.timeInterval.end )
  {
    double time = (t - ur.timeInterval.start).ToDouble();
    //the slope is if not r: 2a * time + b
    // else 2a * time + b / div
    double erg = 2 * ur.a * time + ur.b;
    if( ur.r)
    {
      double div = ur.a * time * time + ur.b * time + ur.c;
      if( div > 0){
        erg /= 2 * sqrt(div);
      }
      else if(ur.b == 0 && ur.c == 0 && ur.a > 0)
      {
        erg = sqrt(ur.a);
      }
      else erg = 0;
    }
    return erg;
  }
  return -1;
}

/*
intersects calculate the next intersection of two MReals

*/
bool intersects( MReal* m1, MReal* m2, Instant &start, Instant& result,
                bool isInsert )
{
  //returns true if m1 intersects m2. The first intersection time is the result
  //Only intersections after the time start are calculated
  int noCo1 = m1->GetNoComponents();
  int ii = 0;
  UReal u1;
  if( !noCo1 ){ return false; }
  m1->Get( ii, u1);
  while( (start > u1.timeInterval.end || (start == u1.timeInterval.end
    && !u1.timeInterval.rc)) && ++ii < noCo1 )
  {
    m1->Get( ii, u1);
  }
  int noCo2 = m2->GetNoComponents();
  UReal u2;
  if( !noCo2 ){ return false; }
  int jj = 0;
  m2->Get( jj, u2);
  while( (start > u2.timeInterval.end || (start == u2.timeInterval.end
    && !u2.timeInterval.rc)) && ++jj < noCo2 )
  {
    m2->Get( jj, u2);
  }
  if( ii >= noCo1 || jj >= noCo2 )
  {
    return false;
  }
  //u1, u2 are the ureals with the time start to begin with the calculation
  //formula: t = (-b +- sqrt(b*b - 4ac)) / 2a where a,b,c are:
  //a=a2-a1; b=b2-b1-2*a2*x; c=a2*x*x - b2*x + c2 - c1;
  //x=u2->timeInterval.start - u1->timeInterval.start, u2 time is later
  //if b*b - 4ac > 0 the equation has a result
  bool hasResult = false;
  double a, b, c, d, r1, r2, x;
  double laterTime;
  while( !hasResult && ii < noCo1 && jj < noCo2)
  {
    if( u1.timeInterval.start <= u2.timeInterval.start )
    {
      laterTime = u2.timeInterval.start.ToDouble();;
      x = (u2.timeInterval.start - u1.timeInterval.start).ToDouble();
      a = u1.a - u2.a;
      b = u1.b - u2.b + 2 * u1.a * x;
      c = u1.a * x * x + u1.b * x + u1.c - u2.c;
    }
    else
    {
      laterTime = u1.timeInterval.start.ToDouble();;
      x = (u1.timeInterval.start - u2.timeInterval.start).ToDouble();
      a = u2.a - u1.a;
      b = u2.b - u1.b + 2 * u2.a * x;
      c = u2.a * x * x + u2.b * x + u2.c - u1.c;
    }
    d = b * b - 4 * a * c;
    if( abs(a) <= QUADDIFF || d >= 0)
    {
      if( abs(a) > QUADDIFF)
      {
        d = sqrt(d);
        r1 = (-b - d) / (2 * a);
        r2 = (-b + d) / (2 * a);
        if( r1 < 0 && r1 > RDELTA) r1 = 0;
        if( r2 < 0 && r2 > RDELTA) r2 = 0;
      }
      else
      {
        r1 = b ? -c / b : -1;
        r2 = -1;  //there is only one result
      }
      if( r1 > r2 ){ swap( r1, r2 );}
      if( (abs(r1 - r2) < 0.000000001)
        && ( (u1.a==0 && u2.a!=0) || (u2.a==0 && u1.a!=0) ))
      {
        //straight line intersects curve in one point, only boundary point
        r1 = -1;
        r2 = -1;
      }
      if( r1 >= 0 )
      {
        result.ReadFrom(r1 + laterTime);
        if( (result > start || (result == start && !isInsert))
          && result <= u1.timeInterval.end && result <= u2.timeInterval.end)
        {
          //now calculate the slope
          double slope1, slope2;
          slope1 = CalcSlope( u1,result);
          slope2 = CalcSlope( u2,result);
          if( slope1 > slope2 )
          {
            hasResult = true;
          }
        }
      }
      if( !hasResult && r2 >= 0 )
      {
        result.ReadFrom(r2 + laterTime);
        if( (result > start || (result == start && !isInsert))
          && result <= u1.timeInterval.end && result <= u2.timeInterval.end)
        {
          double slope1, slope2;
          slope1 = CalcSlope( u1,result);
          slope2 = CalcSlope( u2,result);
          if( slope1 > slope2 )
          {
            hasResult = true;
          }
        }
      }
    }
    if( !hasResult )
    {
      if( u1.timeInterval.end < u2.timeInterval.end )
      {
        ++ii;
      }
      else if( u2.timeInterval.end < u1.timeInterval.end )
      {
        ++jj;
      }
      else
      {
        //both ends are the same
        ++ii;
        ++jj;
      }
      if( ii < noCo1 && jj < noCo2)
      {
        m1->Get( ii, u1);
        m2->Get( jj, u2);
      }
    }
  }
  if( hasResult )
  {
    //if the intersection exact at the end of m1 or m2, do not take it
    m1->Get( m1->GetNoComponents()-1, u1);
    if( result == u1.timeInterval.end )
    {
      hasResult = false;
    }
  }
  if( hasResult )
  {
    //if the intersection exact at the end of m1 or m2, do not take it
    m2->Get( m2->GetNoComponents()-1, u1);
    if( result == u1.timeInterval.end )
    {
      hasResult = false;
    }
  }
  return hasResult;
}

/*
check is for debug purposes and to correct the order

*/
bool check(NNTree<ActiveElem> &t, Instant time)
{
  double slope;
  double d = 0;
  IT itOld = t.begin();
  for( IT ittest=t.begin(); ittest != t.end(); ++ittest)
  {
    if(d-0.05 > CalcDistance(ittest->distance,time,slope))
    {
      //swap the two entries in activeline
      ActiveElem e = *itOld;
      *itOld = *ittest;
      *ittest = e;
    }
    itOld = ittest;
    d = CalcDistance(ittest->distance,time,slope);
  }
  return true;
}

bool check(vector<ActiveElem> &v, Instant time)
{
  double slope;
  double d = 0;
  ITV itOld = v.begin();
  for( ITV ittest=v.begin(); ittest != v.end(); ++ittest)
  {
    if(d-0.05 > CalcDistance(ittest->distance,time,slope))
    {
      ActiveElem e = *itOld;
      *itOld = *ittest;
      *ittest = e;
    }
    itOld = ittest;
    d = CalcDistance(ittest->distance,time,slope);
  }
  return true;
}

/*
changeTupleUnit change the unit attribut of the given tuple
new start and end times can set

*/
Tuple* changeTupleUnit( Tuple *tuple, int attrNr, Instant start,
  Instant end, bool lc, bool rc)
{
  Interval<Instant> iv( start, end, lc, rc);
  if(! iv.IsValid())
    return NULL;

  Tuple* res = new Tuple( tuple->GetTupleType() );
  for( int ii = 0; ii < tuple->GetNoAttributes(); ++ii)
  {
    if( ii != attrNr )
    {
      res->CopyAttribute( ii, tuple, ii);
    }
    else
    {
      const UPoint* upointAttr
          = (const UPoint*)tuple->GetAttribute(attrNr);
      Coord x1, y1, x2, y2;
      GetPosition( *upointAttr, start, x1, y1);
      GetPosition( *upointAttr, end, x2, y2);
//      Interval<Instant> iv( start, end, lc, rc);
      UPoint* up = new UPoint( iv, x1, y1, x2, y2);
      res->PutAttribute( ii, up);
    }
  }
  return res;
}

/*
insertActiveElem inserts an element to the container v
with the active elements. The sort order is the distance
and the slope if the distance is the same

*/
unsigned int insertActiveElem( vector<ActiveElem> &v, ActiveElem &e,
                              Instant time)
{
  if( v.size() == 0)
  {
    v.push_back(e);
    return 0;
  }

  int pos, start, max;
  max = v.size() - 1;
  start = 0;
  bool havePos = false;
  double slope1, slope2;
  double dist = CalcDistance(e.distance,time,slope1);
  while( !havePos && start <= max)
  {
    pos = (max + start) / 2;
    double storeDistance = CalcDistance(v[pos].distance,time,slope2);
    if( dist + DISTDELTA < storeDistance
      || (abs(dist - storeDistance) <= DISTDELTA && slope1 < slope2))
    {
      max = pos - 1;
    }
    else if( dist + DISTDELTA > storeDistance
      || (abs(dist - storeDistance) <= DISTDELTA && slope1 > slope2))
    {
      start = pos + 1;
    }
    else //same distance and same slope
    {
      while( abs(dist - storeDistance) < DISTDELTA && slope1 == slope2
        && ++pos < (int)v.size() )
      {
        storeDistance = CalcDistance(v[pos].distance,time,slope2);
      }
      havePos = true;
    }
  }
  if( !havePos){ pos = start; }
  if( v.capacity() - v.size() < 1)
  {
    v.reserve(v.size() + 100);
  }

  v.insert(v.begin() + (unsigned)pos,e);
  return pos;
}

unsigned int findActiveElem( vector<ActiveElem> &v, MReal *distance,
                            Instant time, Tuple *tuple)
{
  int pos(0), max(v.size()-1), start(0);
  bool havePos = false;
  double slope1, slope2;
  double dist = CalcDistance(distance,time,slope1);
  while( !havePos && start <= max)
  {
    pos = (max + start) / 2;
    double storeDistance = CalcDistance(v[pos].distance,time,slope2);
    if( dist < storeDistance || (dist == storeDistance && slope1 < slope2) )
    {
      max = pos - 1;
    }
    else if( dist > storeDistance ||
            (dist == storeDistance && slope1 > slope2))
    {
      start = pos + 1;
    }
    else //same distance and same slope
    {
      havePos = true;
    }
  }
  int i = pos;
  if( tuple == v[pos].tuple){ havePos = true; }
  else { havePos = false; }
  while( !havePos && (pos < (int)v.size() || i > 0))
  {
    if( i > 0 )
    {
      --i;
      if( tuple == v[i].tuple)
      {
        pos = i;
        havePos = true;
      };
    }

    if( !havePos && pos < (int)v.size() )
    {
      ++pos;
      if( pos < (int)v.size() && tuple == v[pos].tuple)
      {
        havePos = true;
      }
    }
  }
  return pos;
}

/*
insertActiveElem inserts an element to the container t
with the active elements. The sort order is the distance
and the slope if the distance is the same
this function builds an unbalanced binary tree

*/

IT insertActiveElem( NNTree<ActiveElem> &t, ActiveElem &e,
                              Instant time)
{
  if( t.size() == 0)
  {
    return t.addFirst(e);
  }

  double slope1, slope2;
  double dist = CalcDistance(e.distance,time,slope1);
  IT it = t.root();
  while( true)
  {
    double storeDistance = CalcDistance(it->distance,time,slope2);
    if( dist + DISTDELTA < storeDistance
      || (abs(dist - storeDistance) <= DISTDELTA && slope1 < slope2))
    {
      if( it.hasLeft() )
      {
        it.leftItem();
      }
      else
      {
        return t.addLeft( e, it);
      }
    }
    else if( dist + DISTDELTA > storeDistance
      || (abs(dist - storeDistance) <= DISTDELTA && slope1 > slope2))
    {
      if( it.hasRight() )
      {
        it.rightItem();
      }
      else
      {
        return t.addRight( e, it);
      }
    }
    else //same distance and same slope
    {
      IT pos = it;
      ++pos;
      while( pos != t.end()
        && abs(dist - CalcDistance(pos->distance,time,slope2)) < DISTDELTA
        && slope1 == slope2)
      {
        it = pos;
        ++pos;
      }
      return t.addElem( e, it );
    }
  }
}

IT findActiveElem( NNTree<ActiveElem> &t, MReal *distance,
                            Instant time, Tuple *tuple)
{
  double slope1, slope2;
  double dist = CalcDistance(distance,time,slope1);
  IT it = t.root();
  bool havePos = false;
  while( !havePos && it != t.end())
  {
    double storeDistance = CalcDistance(it->distance,time,slope2);
    if( dist < storeDistance
      || (dist == storeDistance && slope1 < slope2))
    {
      if( it.hasLeft() )
        it.leftItem();
      else
      {
        havePos = true;
      }
    }
    else if( dist > storeDistance
    || (dist == storeDistance && slope1 > slope2))
    {
      if( it.hasRight() )
        it.rightItem();
      else
      {
        havePos = true;
      }
    }
    else //same distance and same slope
    {
      havePos = true;
    }
  }
  if( tuple == it->tuple){ havePos = true; }
  else { havePos = false; }

  IT pos1 = it;
  IT pos2 = it;
  while( !havePos && (pos1 != t.begin() || pos2 != t.end()))
  {
    if( pos1 != t.begin() )
    {
      --pos1;
      if( tuple == pos1->tuple)
      {
        pos2 = pos1;
        havePos = true;
      };
    }

    if( !havePos && pos2 != t.end() )
    {
      ++pos2;
      if( pos2 != t.end() && tuple == pos2->tuple)
      {
        havePos = true;
      }
    }
  }
  return pos2;
}


/*
tests if the given element is one of the first k
if yes then erg is set to true.
The k+1. element is returned if exist else v.end()

*/
IT checkFirstK(NNTree<ActiveElem> &t, unsigned int k, IT &it, bool &erg)
{
  unsigned int ii = 0;
  IT testit=t.begin();
  erg = false;
  for( ; testit != t.end() && ii < k; ++testit)
  {
    ++ii;
    if( testit == it )
    {
      erg = true;
    }
  }
  return testit;
}


/*
tests if the given element the k. element
if yes then true is returned.

*/
bool checkK(NNTree<ActiveElem> &t, unsigned int k, IT &it)
{
  unsigned int ii = 0;
  for( IT testit=t.begin(); testit != t.end() && ii < k; ++testit)
  {
    ++ii;
    if( ii == k && testit == it )
    {
      return true;
    }
  }


  return false;
}


/*
get the k ths element in the tree or list

*/
IT getKNeighbor(NNTree<ActiveElem> &t, unsigned int k, bool &erg,
                Instant t_point)
{
  unsigned int ii = 0;
  IT testit=t.begin();
  erg = false;
  double dist_k;
  double slope;

  for( ; testit != t.end() && ii < k; ++testit)
  {
    ++ii;
    if( ii == k){
      dist_k = CalcDistance(testit->distance, t_point, slope);
      erg = true;
      break;
    }
  }

//   const double delta_dist = 0.001;
//   for(; testit != t.end();++testit){
//     double d = CalcDistance(testit->distance, t_point, slope);
//     if(fabs(d - dist_k) < delta_dist){
//       erg = true;
//       return testit;
//     }
//     break;
//   }

//  if(erg)
//     cout<<"k neighbor "<<testit->tuple->GetTupleId()<<endl;

  return testit;
}

inline bool UPAtInstant(Tuple* tuple, Instant t, int attrNr)
{
  UPoint* up = (UPoint*)tuple->GetAttribute(attrNr);
  return up->timeInterval.Contains(t);

}


/*
  To use the plane sweep algrithmus a priority queue for the events and
  a NNTree to save the active segments is needed.

*/

class KNearestQueue{
public:
/*
~Constructor~

 consumes the stream until the first unit is found with an interval
 which intersects the definition time of the moving point

*/

   KNearestQueue(Word& src, QueryProcessor* qp1,
                 int pos1, const MPoint* mpoint1):
        stream(src),qp(qp1),pos(pos1),mpoint(mpoint1), mpos(0){
          
      zone_flag = false;

     if(!mpoint->IsDefined() || mpoint->IsEmpty()){
       lastTuple = 0;
       return;
     }
     UPoint tmp;
     mpoint->Get(0,tmp);
     Instant tmpstart = tmp.timeInterval.start;
     bool lc = tmp.timeInterval.lc;
     mpoint->Get(mpoint->GetNoComponents()-1, tmp);
     Instant tmpend = tmp.timeInterval.end;
     bool rc = tmp.timeInterval.rc;

     iv = Interval<Instant>(tmpstart, tmpend,lc,rc);

     Word current(Address(0));

     qp->Request(stream.addr,current);

     if(!qp->Received(stream.addr)){ // stream is empty
       lastTuple = 0;
       return;
     }

     bool finished = false;
     while(!finished){
        lastTuple = static_cast<Tuple*>(current.addr);
        UPoint* up = static_cast<UPoint*>(lastTuple->GetAttribute(pos));
        tupleStart = up->timeInterval.start;
        Instant tupleEnd(up->timeInterval.end);
        if(up->timeInterval.end <= iv.start){
          lastTuple->DeleteIfAllowed();
          lastTuple = 0;
          qp->Request(stream.addr,current);
          finished =  !qp->Received(stream.addr);
         } else {
           finished = true;
           if(up->timeInterval.start > iv.end){
              lastTuple->DeleteIfAllowed();
              lastTuple = 0;
           }
         }
     }
     if(lastTuple){
       tmp = *static_cast<UPoint*>(lastTuple->GetAttribute(pos));
       tupleStart = tmp.timeInterval.start;
     }
 }

    KNearestQueue(Word& src, QueryProcessor* qp1,
                 int pos1, const MPoint* mpoint1, int z):
        stream(src),qp(qp1),pos(pos1),mpoint(mpoint1), mpos(0), zone(z){
          
      zone_flag = true;

     if(!mpoint->IsDefined() || mpoint->IsEmpty()){
       lastTuple = 0;
       return;
     }
     UPoint tmp;
     mpoint->Get(0,tmp);
     Instant tmpstart = tmp.timeInterval.start;
     bool lc = tmp.timeInterval.lc;
     mpoint->Get(mpoint->GetNoComponents()-1, tmp);
     Instant tmpend = tmp.timeInterval.end;
     bool rc = tmp.timeInterval.rc;

     iv = Interval<Instant>(tmpstart, tmpend,lc,rc);

     Word current(Address(0));

     qp->Request(stream.addr,current);

     if(!qp->Received(stream.addr)){ // stream is empty
       lastTuple = 0;
       return;
     }

     bool finished = false;
     while(!finished){
        lastTuple = static_cast<Tuple*>(current.addr);
        UPoint* up = static_cast<UPoint*>(lastTuple->GetAttribute(pos));
        tupleStart = up->timeInterval.start;
        Instant tupleEnd(up->timeInterval.end);
        if(up->timeInterval.end <= iv.start){
          lastTuple->DeleteIfAllowed();
          lastTuple = 0;
          qp->Request(stream.addr,current);
          finished =  !qp->Received(stream.addr);
         } else {
           finished = true;
           if(up->timeInterval.start > iv.end){
              lastTuple->DeleteIfAllowed();
              lastTuple = 0;
           }
         }
     }
     if(lastTuple){
       tmp = *static_cast<UPoint*>(lastTuple->GetAttribute(pos));
       tupleStart = tmp.timeInterval.start;
     }
 }


/*
~Destructor~


*/
   ~KNearestQueue(){
      if(lastTuple){
         lastTuple->DeleteIfAllowed();
         lastTuple = 0;
      }
   }


   inline bool empty() const{
     if(lastTuple){
       return false;
     }
     return queue.empty();
   }

   inline void push(EventElem e){
      queue.push(e);
   }

   EventElem& top(){
     if(zone_flag == false)
      next();
     else
      next2();

     return const_cast<EventElem &>(queue.top());
   }

   void pop(){
     if(zone_flag == false)
      next();
     else
      next2();

     queue.pop();
   }
   int GetPos(){return pos;}

 private:
   Word stream;                       // source stream
   QueryProcessor* qp;                // query processor
   int pos;                           // position of the attribute
   const MPoint* mpoint;
   int mpos;
   Tuple* lastTuple;                  // last Tuple read from stream
   DateTime tupleStart;               // start instance of the next tuple
   priority_queue<EventElem> queue;   // the event queue
   Interval<Instant> iv;              // interval of the moving point
   int zone;
   bool zone_flag;

  // forbid access to copy constructor and assignment operator
  // => force call by reference
  KNearestQueue(const KNearestQueue& src){
     assert(false);
  }
  KNearestQueue& operator=(const KNearestQueue& src){
     assert(false);
  }

/*
~next~

This function checks whether the next Event comes from the stream or
from the queue. If it comes from the queue, the lastTuple is converted
to a EventElement and inserted into the queue.

*/
  inline void next(){
    if(lastTuple){ // otherwise, we have nothing to do
      if(queue.empty()){
        transfer(tupleStart);
      }  else {
//        EventElem ev = queue.top();
//        transfer(ev.pointInTime);
          DateTime i = queue.top().pointInTime;
          transfer(i);
      }
    }
  }

/*
~transfer~

  Transfers all tuple with starting time <= i into the queue

*/

  inline void transfer(DateTime& i){
     bool t = false;
     while( (lastTuple && tupleStart <= i)  ||
            (lastTuple && !t)){
       t = transfersingle();
     }
  }

/*
~ transfers the next tuple into the queue~
lasttuple must be exist.


*/
  inline bool transfersingle(){

    assert(lastTuple);

   // create a new Event for the queue
    MReal* mr = new MReal(0);
    const UPoint* up = static_cast<UPoint*>(lastTuple->GetAttribute(pos));

    bool transferred = false;

    if(iv.Intersects(up->timeInterval) ){ // intersecting interval
      GetDistance(mpoint, up, mpos, mr);
      Instant t1 = up->timeInterval.start;
      Instant t2 = up->timeInterval.end;
      Instant t3 = t1>= iv.start ? t1 : iv.start;
      Instant t4 = t2 <= iv.end ? t2 : iv.end;
      queue.push(EventElem(E_LEFT, t3, lastTuple, up, mr));
      queue.push(EventElem(E_RIGHT, t4, lastTuple, up, mr));
      // get the next tuple from the stream
      transferred = true;
    }else{
      delete mr;
      // if there is no Event created from the tuple, we just
      // delete it
      lastTuple->DeleteIfAllowed();
      lastTuple = 0;
    }

    Word current(Address(0));
    qp->Request(stream.addr,current);
    if(qp->Received(stream.addr)){
       lastTuple = static_cast<Tuple*>(current.addr);
         const UPoint* up =
               static_cast<const UPoint*>(lastTuple->GetAttribute(pos));
         tupleStart = up->timeInterval.start;
         if(tupleStart > iv.end){
            // tuple starts after the end of the query point
            lastTuple->DeleteIfAllowed();
            lastTuple = 0;
         }
    } else {
      lastTuple = 0;
      transferred = false;
    }
    return transferred;
  }
  
   ///////////////////////////////////////////////////////////////////////
   ////////////////if the input has zone value ///////////////////////////
   ////////////////////////////////////////////////////////////////////////
   
    inline void next2(){
//        cout<<"next2"<<endl;
        if(lastTuple){ // otherwise, we have nothing to do
          if(queue.empty()){
          transfer2(tupleStart);
        }  else {
            DateTime i = queue.top().pointInTime;
            transfer2(i);
        }
      }
    }
  
    inline void transfer2(DateTime& i){
//      cout<<"transfer2"<<endl;
     bool t = false;
     while( (lastTuple && tupleStart <= i)  ||
            (lastTuple && !t)){
       t = transfersingle2();
     }
    }
    
    
    
   inline bool transfersingle2(){

//   cout<<"transfersingl2"<<endl;
    assert(lastTuple);

   // create a new Event for the queue
    MReal* mr = new MReal(0);
    const UPoint* up = static_cast<UPoint*>(lastTuple->GetAttribute(pos));

    bool transferred = false;

    if(iv.Intersects(up->timeInterval) ){ // intersecting interval
      GetDistance2(mpoint, up, mpos, mr, zone);
      Instant t1 = up->timeInterval.start;
      Instant t2 = up->timeInterval.end;
      Instant t3 = t1>= iv.start ? t1 : iv.start;
      Instant t4 = t2 <= iv.end ? t2 : iv.end;
      queue.push(EventElem(E_LEFT, t3, lastTuple, up, mr));
      queue.push(EventElem(E_RIGHT, t4, lastTuple, up, mr));
      // get the next tuple from the stream
      transferred = true;
    }else{
      delete mr;
      // if there is no Event created from the tuple, we just
      // delete it
      lastTuple->DeleteIfAllowed();
      lastTuple = 0;
    }

    Word current(Address(0));
    qp->Request(stream.addr,current);
    if(qp->Received(stream.addr)){
       lastTuple = static_cast<Tuple*>(current.addr);
         const UPoint* up =
               static_cast<const UPoint*>(lastTuple->GetAttribute(pos));
         tupleStart = up->timeInterval.start;
         if(tupleStart > iv.end){
            // tuple starts after the end of the query point
            lastTuple->DeleteIfAllowed();
            lastTuple = 0;
         }
    } else {
      lastTuple = 0;
      transferred = false;
    }
    return transferred;
  }


};

/*
checks if there are intersections and puts them into the eventqueue
this is the right function if the eventqueue is a NNTree

*/
void checkIntersections(EventType type, Instant &time, IT pos,
                      NNTree< ActiveElem > &t,
                      KNearestQueue& evq, Instant &endTime)
{
  switch ( type ){
    case E_LEFT:
    {
      IT nextPos = pos;
      ++nextPos;
      Instant intersectTime( time.GetType() );
      if( nextPos != t.end())
      {
        if( intersects( pos->distance, nextPos->distance, time,
          intersectTime, true ))
        {
          evq.push( EventElem(E_INTERSECT,
            intersectTime, pos->tuple, nextPos->tuple, pos->distance) );
        }
      }
      if( pos != t.begin())
      {
        IT prevPos = pos;
        --prevPos;
        if( intersects( prevPos->distance, pos->distance,
          time, intersectTime, true ))
        {
          evq.push( EventElem(E_INTERSECT, intersectTime,
            prevPos->tuple, pos->tuple, prevPos->distance) );
        }
      }
      break;
    }
    case E_RIGHT:
    {
      if( pos != t.begin() && time < endTime)
      {
        IT posNext = pos;
        ++posNext;
        if( posNext != t.end() )
        {
          IT posPrev = pos;
          --posPrev;
          Instant intersectTime( time.GetType() );
          if( intersects( posPrev->distance, posNext->distance,
            time, intersectTime, false ) )
          {
            evq.push( EventElem(E_INTERSECT, intersectTime,
              posPrev->tuple, posNext->tuple, posPrev->distance) );
          }
        }
      }
      break;
    }
    case E_INTERSECT:
    {
      //look for intersections between the new neighbors
      IT posSec = pos;
      ++posSec;
      if( pos != t.begin() )
      {
        IT posPrev = pos;
        --posPrev;
        Instant intersectTime( time.GetType() );
        if( intersects( posPrev->distance, posSec->distance,
            time, intersectTime, false ) )
        {
          evq.push( EventElem(E_INTERSECT, intersectTime,
            posPrev->tuple, posSec->tuple, posPrev->distance) );
        }
      }
      IT posSecNext = posSec;
      ++posSecNext;
      if( posSecNext != t.end() )
      {
        Instant intersectTime( time.GetType() );
        if( intersects( pos->distance,posSecNext->distance,
          time, intersectTime, false ) )
        {
          evq.push( EventElem(E_INTERSECT, intersectTime,
            pos->tuple, posSecNext->tuple, pos->distance) );
        }
      }
      //look for new intersections between the old neighbors
      Instant intersectTime( time.GetType() );
      if( intersects( posSec->distance, pos->distance,
        time, intersectTime, true ) )
      {
        evq.push( EventElem(E_INTERSECT, intersectTime,
          posSec->tuple, pos->tuple, posSec->distance) );
      }
      break;
    }
  }
}
/*
Eliminates duplicate elements in the priority queue

*/
void deleteDupElements(KNearestQueue& evq, EventType type,
                       Tuple* tuple1, Tuple* tuple2, Instant& time)
{
  while( !evq.empty() )
  {
    //eleminate same elements
    if( type != evq.top().type || time != evq.top().pointInTime
      || tuple1 != evq.top().tuple || tuple2 != evq.top().tuple2)
    {
      break;
    }
    else
    {
      evq.pop();
    }
  }
}


struct KnearestLocalInfo
{
  KnearestLocalInfo(Word& stream, int attrPos,
                    const MPoint* mp, int k1, bool scanFlag1):
     k(k1),scanFlag(scanFlag1), eventQueue(stream, qp, attrPos, mp){}
     
  KnearestLocalInfo(Word& stream, int attrPos,
                    const MPoint* mp, int k1, bool scanFlag1, int zone):
     k(k1),scanFlag(scanFlag1), eventQueue(stream, qp, attrPos, mp, zone){}

  unsigned int k;
  bool scanFlag;
  KNearestQueue eventQueue;
  Instant startTime, endTime;
  NNTree< ActiveElem > activeLine;


  bool AddNewUnit(UPoint* up);
  void MergeUnit(UPoint* up);
  unsigned int count;
  vector<UPoint> up_list;
  vector<MReal> dist_list;
  TupleType* resulttype;

};

/*
check wheter the unit can be merged added into the list
1. time interval adjacent
2. evaluation function equal
3. the end point of the first movement equals to the start point of the second
movement
4. the start point of the first movement not equal to the end point of the
second movement

*/
bool KnearestLocalInfo::AddNewUnit(UPoint* up)
{
    if(up_list.size() > 0){
      UPoint last_up = up_list[up_list.size() - 1];

      Point p1_1 = last_up.p0;
      Point p1_2 = last_up.p1;

      Point p2_1 = up->p0;
      Point p2_2 = up->p1;

      Interval<Instant> iv( last_up.timeInterval.start,
                              up->timeInterval.end,
                              last_up.timeInterval.lc,
                              up->timeInterval.rc);

      if(AlmostEqual(p1_1,p1_2) && AlmostEqual(p1_2,p2_1) &&
         AlmostEqual(p2_1,p2_2)){

        UPoint* temp_up = new UPoint( iv, p1_1.GetX(), p1_1.GetY(),
                                          p1_1.GetX(), p1_1.GetY());

        up_list[up_list.size() - 1] = *temp_up;
        delete temp_up;
        return true;

      }else if(AlmostEqual(p1_1.GetX(), p1_2.GetX()) &&
               AlmostEqual(p2_1.GetX(), p2_2.GetX()) &&
               AlmostEqual(p1_2, p2_1) && !AlmostEqual(p1_1,p2_2)){

        double duration1 =
          (last_up.timeInterval.end.ToDouble() -
           last_up.timeInterval.start.ToDouble())*86400.0;

        double duration2 =
          (up->timeInterval.end.ToDouble() -
           up->timeInterval.start.ToDouble())*86400.0;
        double speed1 = p1_1.Distance(p1_2)/duration1;
        double speed2 = p2_1.Distance(p2_2)/duration2;

        if(AlmostEqual(speed1, speed2)){
            UPoint* temp_up = new UPoint( iv, p1_1.GetX(), p1_1.GetY(),
                                          p2_2.GetX(), p2_2.GetY());

            up_list[up_list.size() - 1] = *temp_up;
            delete temp_up;
            return true;
        }

      }else{

        double a1 = (p1_2.GetY() - p1_1.GetY())/(p1_2.GetX()- p1_1.GetX());
        double b1 = p1_1.GetY() - a1*p1_1.GetX();

        double a2 = (p2_2.GetY() - p2_1.GetY())/(p2_2.GetX()- p2_1.GetX());
        double b2 = p2_1.GetY() - a2*p2_1.GetX();
        if(AlmostEqual(p1_2, p2_1) && !AlmostEqual(p1_1,p2_2) &&
           AlmostEqual(a1,a2) && AlmostEqual(b1,b2)){

           if((p1_2.GetX() > p1_1.GetX() && p2_2.GetX() > p2_1.GetX()) ||
              (p1_2.GetX() < p1_1.GetX() && p2_2.GetX() < p2_1.GetX())){
              UPoint* temp_up = new UPoint( iv, p1_1.GetX(), p1_1.GetY(),
                                          p2_2.GetX(), p2_2.GetY());

              up_list[up_list.size() - 1] = *temp_up;
              delete temp_up;
              return true;
           }
        }
      }
    }
    return false;

}

void KnearestLocalInfo::MergeUnit(UPoint* up)
{
      UPoint last_up = up_list[up_list.size() - 1];

      Point p1_1 = last_up.p0;
      Point p1_2 = last_up.p1;

      Point p2_1 = up->p0;
      Point p2_2 = up->p1;

      Interval<Instant> iv( last_up.timeInterval.start,
                              up->timeInterval.end,
                              last_up.timeInterval.lc,
                              up->timeInterval.rc);

      UPoint* temp_up = new UPoint( iv, p1_1.GetX(), p1_1.GetY(),
                                          p2_2.GetX(), p2_2.GetY());

      up_list[up_list.size() - 1] = *temp_up;
      delete temp_up;
}

/*
newknearestFun is the main function of the operator knearest
The argument vector contains the following values:
args[0] = stream of tuples,
 the attribute given in args[1] has to be a unit
 the operator expects that the tuples are sorted by the time of the units
args[1] = attribute name
args[2] = mpoint
args[3] = int k, how many nearest are searched
args[4] = int j, attributenumber

*/

int knearestFun1 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  switch (message)
  {
    /*
    in open the eventqueue is initialized with the left
    and the right elements

    */
    case OPEN :
    {
      qp->Open(args[0].addr);
      CcInt* k = static_cast<CcInt*>(args[3].addr);
      const MPoint* mp = static_cast<const MPoint*>(args[2].addr);
      int attrPos = (static_cast<CcInt*>(args[4].addr))->GetIntval() - 1;
      if(!mp->IsDefined() || mp->IsEmpty() || !k->IsDefined()){
         local.addr = 0;
      } else{
         KnearestLocalInfo* localInfo =
                new KnearestLocalInfo(args[0], attrPos, mp,
                                      k->GetIntval(), true);
         UPoint up1, up2;
         mp->Get( 0, up1);
         mp->Get( mp->GetNoComponents() - 1, up2);
         localInfo->startTime = up1.timeInterval.start;
         localInfo->endTime = up2.timeInterval.end;
         local = SetWord(localInfo);
      }
      return 0;
    }

    /*
    in request the eventqueue is executed. new intersect events
    are computed

    */
    case REQUEST :
    {
     if(!local.addr){
       return CANCEL;
     }
     int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;
     KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      while ( !localInfo->eventQueue.empty() )
      {
        EventElem elem = localInfo->eventQueue.top();
        localInfo->eventQueue.pop();
        ActiveElem::currtime = elem.pointInTime;

        //eleminate same elements
        deleteDupElements( localInfo->eventQueue, elem.type,
          elem.tuple, elem.tuple2, elem.pointInTime);

        switch ( elem.type ){
          case E_LEFT:
          {

            bool lc = elem.pointInTime >= localInfo->startTime
              ? elem.up->timeInterval.lc : false;
            ActiveElem newElem(elem.distance, elem.tuple, elem.pointInTime,
              elem.up->timeInterval.end, lc, elem.up->timeInterval.rc);
            IT newPos = insertActiveElem( localInfo->activeLine,
              newElem, elem.pointInTime);

            // check intersections
            checkIntersections(E_LEFT, elem.pointInTime,
              newPos, localInfo->activeLine, localInfo->eventQueue,
              localInfo->endTime);

            bool hasChangedFirstK = false;
            IT posAfterK;
            if( localInfo->activeLine.size() > localInfo->k )
            {
              posAfterK = checkFirstK(localInfo->activeLine,
                localInfo->k,newPos,hasChangedFirstK);
            }
            if( hasChangedFirstK)
            {
              //something in the first k has changed
              //now give out the k. element, but only until this time.
              //Clone the tuple and change the unit-attribut to the
              //new time interval
              bool rc = newPos->lc ? false
                : (posAfterK->end == elem.pointInTime ? posAfterK->rc : true);
              if( posAfterK->start != elem.pointInTime
                || (rc && posAfterK->lc))
              {
                Tuple* cloneTuple = changeTupleUnit(
                  posAfterK->tuple, attrNr, posAfterK->start,
                  elem.pointInTime, posAfterK->lc, rc);
                posAfterK->start = elem.pointInTime;
                posAfterK->lc = false;
                if(cloneTuple != NULL)
                {
                  result = SetWord(cloneTuple);
                  return YIELD;
                }
              }
            }

            break;
          }
          case E_RIGHT:
          {
            //a unit ends. It has to be removed from the map
            IT posDel = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            Tuple* cloneTuple = NULL;
            if( posDel != localInfo->activeLine.end())
            {
              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 and the following to this time
              bool hasDelFirstK = false;
              IT posAfterK;
              posAfterK = checkFirstK(localInfo->activeLine,
                  localInfo->k,posDel,hasDelFirstK);

              if( hasDelFirstK && (posDel->start != elem.pointInTime
                || (posDel->lc && posDel->rc)))
              {
                if( posDel->start < localInfo->endTime )
                {
                  cloneTuple = changeTupleUnit(
                    posDel->tuple, attrNr, posDel->start, elem.pointInTime,
                    posDel->lc, posDel->rc);
                }
                for( ; posAfterK != localInfo->activeLine.end(); ++posAfterK)
                {
                  posAfterK->start = elem.pointInTime;
                  posAfterK->lc = false;
                }
              }
              //now calculate the intersection of the neighbors
              checkIntersections(E_RIGHT, elem.pointInTime,
                posDel, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              localInfo->activeLine.erase( posDel );
              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
            }
            else
            {
              //the program should never be here. This is a program error!
              double slope;
              for( IT it=localInfo->activeLine.begin();
                it!=localInfo->activeLine.end(); ++it)
              {
                cout << "dist: " << CalcDistance(it->distance,
                  elem.pointInTime,slope) << ", Tuple: " << it->tuple << endl;
              }
              assert(false);
            }
            if( cloneTuple != NULL)
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }
            break;
          }

          case E_INTERSECT:
          {
            IT posFirst = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            IT posNext = posFirst;
            ++posNext;

            IT posSec;
            if( posNext == localInfo->activeLine.end() )
            {
              //perhaps changed before
              bool t1 = check(localInfo->activeLine ,elem.pointInTime);
              assert(t1);

              break;
            }
            if( posNext->tuple != elem.tuple2 )
            {
              posSec = findActiveElem( localInfo->activeLine,
                elem.distance, elem.pointInTime, elem.tuple2);
            }
            else
              posSec = posNext;

            //check if the first of the inters.-tuples is the k. and give it out
            Tuple* cloneTuple = NULL;
            if( checkK(localInfo->activeLine,localInfo->k,posFirst))
            {
              cloneTuple = changeTupleUnit( posFirst->tuple,
                attrNr, posFirst->start, elem.pointInTime,
                posFirst->lc,posFirst->rc);
              posFirst->start = elem.pointInTime;
              posFirst->lc = true;
              if( posNext != localInfo->activeLine.end() )
              {
                posNext->start = elem.pointInTime;
                posNext->lc = true;
              }
            }

            if( posNext == posSec)
            {
              //look for intersections between the new neighbors
              checkIntersections(E_INTERSECT, elem.pointInTime,
                posFirst, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //swap the two entries in activeline
              ActiveElem e = *posFirst;
              *posFirst = *posSec;
              *posSec = e;
            }
            else
            {
              //the are some elements which has the same distance between
              //the two intersect elements, so calc like delete then insert
              vector<ActiveElem> va;
              set<Tuple*> s;
              pair< set<Tuple*>::iterator, bool > pr;

              ActiveElem newElem1(posFirst->distance,
                  posFirst->tuple, elem.pointInTime,
                  posFirst->end, posFirst->lc, posFirst->rc);
              va.push_back(newElem1);
              s.insert(posFirst->tuple);
              localInfo->activeLine.erase( posFirst );
              ActiveElem newElem2(posSec->distance,
                  posSec->tuple, elem.pointInTime,
                  posSec->end, posSec->lc, posSec->rc);
              va.push_back(newElem2);
              s.insert(posSec->tuple);
              localInfo->activeLine.erase( posSec );

              while( !localInfo->eventQueue.empty() )
              {
                //handle all intersections at the same time
                EventElem elemIs = localInfo->eventQueue.top();

                if( elemIs.type == E_INTERSECT
                  && elem.pointInTime == elemIs.pointInTime)
                {

                  localInfo->eventQueue.pop();
                  //eleminate same elements
                  deleteDupElements( localInfo->eventQueue, elemIs.type,
                      elemIs.tuple, elemIs.tuple2, elem.pointInTime);

                  pr = s.insert( elemIs.tuple );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                  pr = s.insert( elemIs.tuple2 );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple2);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                }
                else
                {

                  break;
                }
              }

              vector<IT> vit;
              for( unsigned int ii=0; ii < va.size(); ++ii )
              {
                  IT newPos = insertActiveElem( localInfo->activeLine,
                    va[ii], elem.pointInTime);
                  vit.push_back(newPos);
              }
              // check intersections
              for( unsigned int ii=0; ii < vit.size(); ++ii )
              {
                checkIntersections(E_LEFT, elem.pointInTime, vit[ii],
                  localInfo->activeLine, localInfo->eventQueue,
                  localInfo->endTime);
              }
            }
            if( cloneTuple != NULL )
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }

            break;
          }
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {

      qp->Close(args[0].addr);
      KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if(localInfo){

        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }
  }

  return 0;
}

/*
one more input parameter for converting longituide and latitude 

*/
int knearestFun2 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  switch (message)
  {

    case OPEN :
    {
      qp->Open(args[0].addr);
      CcInt* k = static_cast<CcInt*>(args[3].addr);
      const MPoint* mp = static_cast<const MPoint*>(args[2].addr);
//      const MPoint* oldmp = static_cast<const MPoint*>(args[2].addr);
      int attrPos = (static_cast<CcInt*>(args[5].addr))->GetIntval() - 1;
      int zone = (static_cast<CcInt*>(args[4].addr))->GetIntval();

//       MPoint* mp = new MPoint(0);
//       mp->StartBulkLoad();
//       for(int i = 0;i < oldmp->GetNoComponents();i++){
//         UPoint up;
//         oldmp->Get(i, up);
//         mp->MergeAdd(up);
//       }
//       mp->EndBulkLoad();

      if(zone >= 0){
          if(zone > 119){
            cout<<"invalid zone value "<<zone<<endl;
            local.addr = 0;
            return 0;
          }else{
            int de = 3;
            int count = 120;
            vector<int> de_list;
            for(int i = 0;i < count ;i ++){
              de_list.push_back(i*de);
            }

            double delta = numeric_limits<double>::max();
            UPoint up;
            mp->Get( 0, up);
            int index = -1;
            for(unsigned int i = 0;i < de_list.size();i++){
              if(i == 0){
                delta = fabs(up.p0.GetX() - de_list[i]);
                index = i;
              }else{
                if(fabs(up.p0.GetX() - de_list[i]) < delta){
                  delta = fabs(up.p0.GetX() - de_list[i]);
                  index = i;
                }
              }
            }
            if(index != zone){
              cout<<"input zone is not correct, should be "<<index<<endl;
              local.addr = 0;
              return 0;
            }
        }
      }else if(zone == -1){/////////calculate the value 
        int de = 3;
        int count = 120;
        vector<int> de_list;
        for(int i = 0;i < count ;i ++){
          de_list.push_back(i*de);
        }

        double delta = numeric_limits<double>::max();
        UPoint up;
        mp->Get( 0, up);
        int index = -1;
        for(unsigned int i = 0;i < de_list.size();i++){
          if(i == 0){
            delta = fabs(up.p0.GetX() - de_list[i]);
            index = i;
          }else{
            if(fabs(up.p0.GetX() - de_list[i]) < delta){
              delta = fabs(up.p0.GetX() - de_list[i]);
              index = i;
            }
          }
        }
        zone = index;
        cout<<"zone "<<zone<<endl;
      }else{
        cout<<"invalid zone value "<<zone<<endl;
        local.addr = 0;
        return 0;
      }

      if(!mp->IsDefined() || mp->IsEmpty() || !k->IsDefined()){
         local.addr = 0;
      } else{
         KnearestLocalInfo* localInfo =
                new KnearestLocalInfo(args[0], attrPos, mp,
                                      k->GetIntval(), true, zone);
         UPoint up1, up2;
         mp->Get( 0, up1);
         mp->Get( mp->GetNoComponents() - 1, up2);
         localInfo->startTime = up1.timeInterval.start;
         localInfo->endTime = up2.timeInterval.end;
         local = SetWord(localInfo);
      }
      return 0;
    }

    /*
    in request the eventqueue is executed. new intersect events
    are computed

    */
    case REQUEST :
    {
     if(!local.addr){
       return CANCEL;
     }
     int attrNr = ((CcInt*)args[5].addr)->GetIntval() - 1;
     KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      while ( !localInfo->eventQueue.empty() )
      {
        EventElem elem = localInfo->eventQueue.top();
        localInfo->eventQueue.pop();
        ActiveElem::currtime = elem.pointInTime;

        //eleminate same elements
        deleteDupElements( localInfo->eventQueue, elem.type,
          elem.tuple, elem.tuple2, elem.pointInTime);

        switch ( elem.type ){
          case E_LEFT:
          {

            bool lc = elem.pointInTime >= localInfo->startTime
              ? elem.up->timeInterval.lc : false;
            ActiveElem newElem(elem.distance, elem.tuple, elem.pointInTime,
              elem.up->timeInterval.end, lc, elem.up->timeInterval.rc);
            IT newPos = insertActiveElem( localInfo->activeLine,
              newElem, elem.pointInTime);

            // check intersections
            checkIntersections(E_LEFT, elem.pointInTime,
              newPos, localInfo->activeLine, localInfo->eventQueue,
              localInfo->endTime);

            bool hasChangedFirstK = false;
            IT posAfterK;
            if( localInfo->activeLine.size() > localInfo->k )
            {
              posAfterK = checkFirstK(localInfo->activeLine,
                localInfo->k,newPos,hasChangedFirstK);
            }
            if( hasChangedFirstK)
            {
              //something in the first k has changed
              //now give out the k. element, but only until this time.
              //Clone the tuple and change the unit-attribut to the
              //new time interval
              bool rc = newPos->lc ? false
                : (posAfterK->end == elem.pointInTime ? posAfterK->rc : true);
              if( posAfterK->start != elem.pointInTime
                || (rc && posAfterK->lc))
              {
                Tuple* cloneTuple = changeTupleUnit(
                  posAfterK->tuple, attrNr, posAfterK->start,
                  elem.pointInTime, posAfterK->lc, rc);
                posAfterK->start = elem.pointInTime;
                posAfterK->lc = false;
                if(cloneTuple != NULL)
                {
                  result = SetWord(cloneTuple);
                  return YIELD;
                }
              }
            }

            break;
          }
          case E_RIGHT:
          {
            //a unit ends. It has to be removed from the map
            IT posDel = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            Tuple* cloneTuple = NULL;
            if( posDel != localInfo->activeLine.end())
            {
              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 and the following to this time
              bool hasDelFirstK = false;
              IT posAfterK;
              posAfterK = checkFirstK(localInfo->activeLine,
                  localInfo->k,posDel,hasDelFirstK);

              if( hasDelFirstK && (posDel->start != elem.pointInTime
                || (posDel->lc && posDel->rc)))
              {
                if( posDel->start < localInfo->endTime )
                {
                  cloneTuple = changeTupleUnit(
                    posDel->tuple, attrNr, posDel->start, elem.pointInTime,
                    posDel->lc, posDel->rc);
                }
                for( ; posAfterK != localInfo->activeLine.end(); ++posAfterK)
                {
                  posAfterK->start = elem.pointInTime;
                  posAfterK->lc = false;
                }
              }
              //now calculate the intersection of the neighbors
              checkIntersections(E_RIGHT, elem.pointInTime,
                posDel, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              localInfo->activeLine.erase( posDel );
              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
            }
            else
            {
              //the program should never be here. This is a program error!
              double slope;
              for( IT it=localInfo->activeLine.begin();
                it!=localInfo->activeLine.end(); ++it)
              {
                cout << "dist: " << CalcDistance(it->distance,
                  elem.pointInTime,slope) << ", Tuple: " << it->tuple << endl;
              }
              assert(false);
            }
            if( cloneTuple != NULL)
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }
            break;
          }

          case E_INTERSECT:
          {
            IT posFirst = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            IT posNext = posFirst;
            ++posNext;

            IT posSec;
            if( posNext == localInfo->activeLine.end() )
            {
              //perhaps changed before
              bool t1 = check(localInfo->activeLine ,elem.pointInTime);
              assert(t1);

              break;
            }
            if( posNext->tuple != elem.tuple2 )
            {
              posSec = findActiveElem( localInfo->activeLine,
                elem.distance, elem.pointInTime, elem.tuple2);
            }
            else
              posSec = posNext;

            //check if the first of the inters.-tuples is the k. and give it out
            Tuple* cloneTuple = NULL;
            if( checkK(localInfo->activeLine,localInfo->k,posFirst))
            {
              cloneTuple = changeTupleUnit( posFirst->tuple,
                attrNr, posFirst->start, elem.pointInTime,
                posFirst->lc,posFirst->rc);
              posFirst->start = elem.pointInTime;
              posFirst->lc = true;
              if( posNext != localInfo->activeLine.end() )
              {
                posNext->start = elem.pointInTime;
                posNext->lc = true;
              }
            }

            if( posNext == posSec)
            {
              //look for intersections between the new neighbors
              checkIntersections(E_INTERSECT, elem.pointInTime,
                posFirst, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //swap the two entries in activeline
              ActiveElem e = *posFirst;
              *posFirst = *posSec;
              *posSec = e;
            }
            else
            {
              //the are some elements which has the same distance between
              //the two intersect elements, so calc like delete then insert
              vector<ActiveElem> va;
              set<Tuple*> s;
              pair< set<Tuple*>::iterator, bool > pr;

              ActiveElem newElem1(posFirst->distance,
                  posFirst->tuple, elem.pointInTime,
                  posFirst->end, posFirst->lc, posFirst->rc);
              va.push_back(newElem1);
              s.insert(posFirst->tuple);
              localInfo->activeLine.erase( posFirst );
              ActiveElem newElem2(posSec->distance,
                  posSec->tuple, elem.pointInTime,
                  posSec->end, posSec->lc, posSec->rc);
              va.push_back(newElem2);
              s.insert(posSec->tuple);
              localInfo->activeLine.erase( posSec );

              while( !localInfo->eventQueue.empty() )
              {
                //handle all intersections at the same time
                EventElem elemIs = localInfo->eventQueue.top();

                if( elemIs.type == E_INTERSECT
                  && elem.pointInTime == elemIs.pointInTime)
                {

                  localInfo->eventQueue.pop();
                  //eleminate same elements
                  deleteDupElements( localInfo->eventQueue, elemIs.type,
                      elemIs.tuple, elemIs.tuple2, elem.pointInTime);

                  pr = s.insert( elemIs.tuple );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                  pr = s.insert( elemIs.tuple2 );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple2);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                }
                else
                {

                  break;
                }
              }

              vector<IT> vit;
              for( unsigned int ii=0; ii < va.size(); ++ii )
              {
                  IT newPos = insertActiveElem( localInfo->activeLine,
                    va[ii], elem.pointInTime);
                  vit.push_back(newPos);
              }
              // check intersections
              for( unsigned int ii=0; ii < vit.size(); ++ii )
              {
                checkIntersections(E_LEFT, elem.pointInTime, vit[ii],
                  localInfo->activeLine, localInfo->eventQueue,
                  localInfo->endTime);
              }
            }
            if( cloneTuple != NULL )
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }

            break;
          }
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {

      qp->Close(args[0].addr);
      KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if(localInfo){

        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }
  }

  return 0;
}


/*
return kth neighbor

*/

int newknearest_distFun1 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  switch (message)
  {
    /*
    in open the eventqueue is initialized with the left
    and the right elements

    */
    case OPEN :
    {
      qp->Open(args[0].addr);
//      CcInt* k = static_cast<CcInt*>(args[3].addr);
      int k = (int)static_cast<CcReal*>(args[3].addr)->GetRealval();

      const MPoint* mp = static_cast<const MPoint*>(args[2].addr);
      int attrPos = (static_cast<CcInt*>(args[4].addr))->GetIntval() - 1;
      int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;

      if(!mp->IsDefined() || mp->IsEmpty() || k < 1){
         local.addr = 0;
      } else{
         KnearestLocalInfo* localInfo =
                new KnearestLocalInfo(args[0], attrPos, mp, k, true);
         UPoint up1, up2;
         mp->Get( 0, up1);
         mp->Get( mp->GetNoComponents() - 1, up2);
         localInfo->startTime = up1.timeInterval.start;
         localInfo->endTime = up2.timeInterval.end;

         localInfo->count = 0;
         localInfo->resulttype =
            new TupleType(nl->Second(GetTupleResultType(s)));

         unsigned int last_result_tid = 0;
         ////////////////first calculate the result/////////////////////////
         while ( !localInfo->eventQueue.empty() ){
            EventElem elem = localInfo->eventQueue.top();
            localInfo->eventQueue.pop();

//            cout<<elem.pointInTime<<endl;

            //eleminate same elements
            deleteDupElements( localInfo->eventQueue, elem.type,
            elem.tuple, elem.tuple2, elem.pointInTime);

        switch ( elem.type ){
          case E_LEFT:
          {
 //           cout<<"left "<<elem.pointInTime<<endl;
            bool lc = elem.pointInTime >= localInfo->startTime
              ? elem.up->timeInterval.lc : false;
            ActiveElem newElem(elem.distance, elem.tuple, elem.pointInTime,
              elem.up->timeInterval.end, lc, elem.up->timeInterval.rc);
            IT newPos = insertActiveElem( localInfo->activeLine,
              newElem, elem.pointInTime);

            // check intersections
            checkIntersections(E_LEFT, elem.pointInTime,
              newPos, localInfo->activeLine, localInfo->eventQueue,
              localInfo->endTime);

            bool hasChangedFirstK = false;
            IT posAfterK;
            if( localInfo->activeLine.size() > localInfo->k ){
               posAfterK = checkFirstK(localInfo->activeLine,
                 localInfo->k,newPos,hasChangedFirstK);

            }

            if( hasChangedFirstK){
              //something in the first k has changed
              //now give out the k. element, but only until this time.
              //Clone the tuple and change the unit-attribut to the
              //new time interval
              bool rc = newPos->lc ? false
                : (posAfterK->end == elem.pointInTime ? posAfterK->rc : true);
              if( posAfterK->start != elem.pointInTime
                || (rc && posAfterK->lc)){

                bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                    if(localInfo->up_list.size() == 0){
                      last_result_tid = posK->tuple->GetTupleId();
//                      cout<<"start "<<last_result_tid<<endl;
                    }

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                        bool l =
                          UPAtInstant(posK->tuple, posK->start, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;


                        if(up_end < elem.pointInTime){
                          bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                          bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                           localInfo->up_list.push_back(*up);
                      if(localInfo->up_list.size() > 0 &&
                        last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                      ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
//                      cout<<"left "<<last_result_tid<<endl;
                        last_result_tid = posK->tuple->GetTupleId();
                      }
                    }

                }

                 posAfterK->start = elem.pointInTime;
                 posAfterK->lc = false;
              }
            }


            break;
          }
          case E_RIGHT:
          {
//            cout<<"right "<<elem.pointInTime<<endl;

            //a unit ends. It has to be removed from the map
            IT posDel = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);

            if( posDel != localInfo->activeLine.end()){

              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 and the following to this time
              bool hasDelFirstK = false;
              IT posAfterK;

               posAfterK = checkFirstK(localInfo->activeLine,
                      localInfo->k,posDel,hasDelFirstK);


              if( hasDelFirstK && (posDel->start != elem.pointInTime
                || (posDel->lc && posDel->rc))){
                if( posDel->start < localInfo->endTime ){

                }
                for( ; posAfterK != localInfo->activeLine.end(); ++posAfterK){
                  posAfterK->start = elem.pointInTime;
                  posAfterK->lc = false;
                }
              }



                bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                  if(localInfo->up_list.size() == 0)
                      last_result_tid = posK->tuple->GetTupleId();

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                        bool l =
                            UPAtInstant(posK->tuple, posK->start, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;

                        bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        if(up_end < elem.pointInTime){
                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                         localInfo->up_list.push_back(*up);

                      if(localInfo->up_list.size() > 0 &&
                        last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                     ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
//                      cout<<"right "<<last_result_tid<<endl;
                        last_result_tid = posK->tuple->GetTupleId();
                      }
                    }



                }

              //now calculate the intersection of the neighbors
              checkIntersections(E_RIGHT, elem.pointInTime,
                posDel, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);


              localInfo->activeLine.erase( posDel );
              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
            }
            else{
              //the program should never be here. This is a program error!
              assert(false);
            }

            break;
          }

          case E_INTERSECT:
          {
//            cout<<"intersect "<<elem.pointInTime<<endl;

            IT posFirst = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            IT posNext = posFirst;
            ++posNext;

            IT posSec;
            if( posNext == localInfo->activeLine.end() ){
              //perhaps changed before
              bool t1 = check(localInfo->activeLine ,elem.pointInTime);
              assert(t1);

              break;
            }
            if( posNext->tuple != elem.tuple2 ){
              posSec = findActiveElem( localInfo->activeLine,
                elem.distance, elem.pointInTime, elem.tuple2);
            }
            else
              posSec = posNext;

              bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                   if(localInfo->up_list.size() == 0)
                      last_result_tid = posK->tuple->GetTupleId();

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                      bool l =
                            UPAtInstant(posK->tuple, posK->start, attrNr);
                      bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;

                        bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        if(up_end < elem.pointInTime){
                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                         localInfo->up_list.push_back(*up);

                      if(localInfo->up_list.size() > 0 &&
                         last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                      ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
                        last_result_tid = posK->tuple->GetTupleId();
  //                      cout<<"intersect "<<last_result_tid<<endl;
                      }

                    }

                }

          //check if the first of the inters.-tuples is the k. and give it out

            if( checkK(localInfo->activeLine,localInfo->k,posFirst)){

              posFirst->start = elem.pointInTime;
              posFirst->lc = true;
              if( posNext != localInfo->activeLine.end() ){
                posNext->start = elem.pointInTime;
                posNext->lc = true;
              }
            }

            if( posNext == posSec){
              //look for intersections between the new neighbors
              checkIntersections(E_INTERSECT, elem.pointInTime,
                posFirst, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //swap the two entries in activeline
              ActiveElem e = *posFirst;
              *posFirst = *posSec;
              *posSec = e;
            }else{
              //the are some elements which has the same distance between
              //the two intersect elements, so calc like delete then insert
              vector<ActiveElem> va;
              set<Tuple*> s;
              pair< set<Tuple*>::iterator, bool > pr;

              ActiveElem newElem1(posFirst->distance,
                  posFirst->tuple, elem.pointInTime,
                  posFirst->end, posFirst->lc, posFirst->rc);
              va.push_back(newElem1);
              s.insert(posFirst->tuple);
              localInfo->activeLine.erase( posFirst );
              ActiveElem newElem2(posSec->distance,
                  posSec->tuple, elem.pointInTime,
                  posSec->end, posSec->lc, posSec->rc);
              va.push_back(newElem2);
              s.insert(posSec->tuple);
              localInfo->activeLine.erase( posSec );

              while( !localInfo->eventQueue.empty() ){
                //handle all intersections at the same time
                EventElem elemIs = localInfo->eventQueue.top();

                if( elemIs.type == E_INTERSECT
                  && elem.pointInTime == elemIs.pointInTime)
                {

                  localInfo->eventQueue.pop();
                  //eleminate same elements
                  deleteDupElements( localInfo->eventQueue, elemIs.type,
                      elemIs.tuple, elemIs.tuple2, elem.pointInTime);

                  pr = s.insert( elemIs.tuple );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                  pr = s.insert( elemIs.tuple2 );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple2);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                }
                else{break;}
              }

              vector<IT> vit;
              for( unsigned int ii=0; ii < va.size(); ++ii ){
                  IT newPos = insertActiveElem( localInfo->activeLine,
                    va[ii], elem.pointInTime);
                  vit.push_back(newPos);
              }
              // check intersections
              for( unsigned int ii=0; ii < vit.size(); ++ii ){
                checkIntersections(E_LEFT, elem.pointInTime, vit[ii],
                  localInfo->activeLine, localInfo->eventQueue,
                  localInfo->endTime);
              }
            }

            break;
          }
        }

      }

         local = SetWord(localInfo);
      }
      return 0;
    }

    /*
    in request the eventqueue is executed. new intersect events
    are computed

    */
    case REQUEST :
    {
     if(!local.addr){
       return CANCEL;
     }
     KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }
      if(localInfo->count == localInfo->up_list.size()) return CANCEL;

          Tuple* tuple = new Tuple(localInfo->resulttype);

          tuple->PutAttribute(0,
                              new UPoint(localInfo->up_list[localInfo->count]));
/*          tuple->PutAttribute(1,
                          new MReal(localInfo->dist_list[localInfo->count]));*/
          result.setAddr(tuple);
          localInfo->count++;
          return YIELD;

    }

    case CLOSE :
    {

      qp->Close(args[0].addr);
      KnearestLocalInfo* localInfo = (KnearestLocalInfo*)local.addr;
      if(localInfo){
        if(localInfo->resulttype != NULL) delete localInfo->resulttype;

        delete localInfo;
        local.setAddr(0);
      }
      return 0;
    }
  }

  return 0;
}

/*
return mreal for the distance of kth neighbor

*/
int newknearest_distFun2 (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
      qp->Open(args[0].addr);
      CcInt* k = static_cast<CcInt*>(args[3].addr);

      const MPoint* mp = static_cast<const MPoint*>(args[2].addr);
      int attrPos = (static_cast<CcInt*>(args[4].addr))->GetIntval() - 1;
      int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;


      result = qp->ResultStorage(s);
      MReal* presult = (MReal*)result.addr;

      if(!mp->IsDefined() || mp->IsEmpty() || k->GetIntval() < 1){
        MReal* res = new MReal(0);
        *presult = *res;
        delete res;
        qp->Close(args[0].addr);
        return 0;
      }

         KnearestLocalInfo* localInfo =
             new KnearestLocalInfo(args[0], attrPos, mp, k->GetIntval(), true);
         UPoint up1, up2;
         mp->Get( 0, up1);
         mp->Get( mp->GetNoComponents() - 1, up2);
         localInfo->startTime = up1.timeInterval.start;
         localInfo->endTime = up2.timeInterval.end;

        ////////////////first calculate the result/////////////////////////
        unsigned int last_result_tid = 0;
         ////////////////first calculate the result/////////////////////////
         while ( !localInfo->eventQueue.empty() ){
            EventElem elem = localInfo->eventQueue.top();
            localInfo->eventQueue.pop();

//            cout<<elem.pointInTime<<endl;

            //eleminate same elements
            deleteDupElements( localInfo->eventQueue, elem.type,
            elem.tuple, elem.tuple2, elem.pointInTime);

        switch ( elem.type ){
          case E_LEFT:
          {
 //           cout<<"left "<<elem.pointInTime<<endl;
            bool lc = elem.pointInTime >= localInfo->startTime
              ? elem.up->timeInterval.lc : false;
            ActiveElem newElem(elem.distance, elem.tuple, elem.pointInTime,
              elem.up->timeInterval.end, lc, elem.up->timeInterval.rc);
            IT newPos = insertActiveElem( localInfo->activeLine,
              newElem, elem.pointInTime);

            // check intersections
            checkIntersections(E_LEFT, elem.pointInTime,
              newPos, localInfo->activeLine, localInfo->eventQueue,
              localInfo->endTime);

            bool hasChangedFirstK = false;
            IT posAfterK;
            if( localInfo->activeLine.size() > localInfo->k ){
               posAfterK = checkFirstK(localInfo->activeLine,
                 localInfo->k,newPos,hasChangedFirstK);

            }

            if( hasChangedFirstK){
              //something in the first k has changed
              //now give out the k. element, but only until this time.
              //Clone the tuple and change the unit-attribut to the
              //new time interval
              bool rc = newPos->lc ? false
                : (posAfterK->end == elem.pointInTime ? posAfterK->rc : true);
              if( posAfterK->start != elem.pointInTime
                || (rc && posAfterK->lc)){

                bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                    if(localInfo->up_list.size() == 0){
                      last_result_tid = posK->tuple->GetTupleId();
//                      cout<<"start "<<last_result_tid<<endl;
                    }

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                        bool l =
                          UPAtInstant(posK->tuple, posK->start, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;


                        if(up_end < elem.pointInTime){
                          bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                          bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                           localInfo->up_list.push_back(*up);
                      if(localInfo->up_list.size() > 0 &&
                        last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                      ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
//                      cout<<"left "<<last_result_tid<<endl;
                        last_result_tid = posK->tuple->GetTupleId();
                      }
                    }

                }

                 posAfterK->start = elem.pointInTime;
                 posAfterK->lc = false;
              }
            }


            break;
          }
          case E_RIGHT:
          {
//            cout<<"right "<<elem.pointInTime<<endl;

            //a unit ends. It has to be removed from the map
            IT posDel = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);

            if( posDel != localInfo->activeLine.end()){

              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 and the following to this time
              bool hasDelFirstK = false;
              IT posAfterK;

               posAfterK = checkFirstK(localInfo->activeLine,
                      localInfo->k,posDel,hasDelFirstK);


              if( hasDelFirstK && (posDel->start != elem.pointInTime
                || (posDel->lc && posDel->rc))){
                if( posDel->start < localInfo->endTime ){

                }
                for( ; posAfterK != localInfo->activeLine.end(); ++posAfterK){
                  posAfterK->start = elem.pointInTime;
                  posAfterK->lc = false;
                }
              }

                bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                  if(localInfo->up_list.size() == 0)
                      last_result_tid = posK->tuple->GetTupleId();

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                        bool l =
                            UPAtInstant(posK->tuple, posK->start, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;

                        bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        if(up_end < elem.pointInTime){
                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                         localInfo->up_list.push_back(*up);

                      if(localInfo->up_list.size() > 0 &&
                        last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                     ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
//                      cout<<"right "<<last_result_tid<<endl;
                        last_result_tid = posK->tuple->GetTupleId();
                      }
                    }
                }

              //now calculate the intersection of the neighbors
              checkIntersections(E_RIGHT, elem.pointInTime,
                posDel, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);


              localInfo->activeLine.erase( posDel );
              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
            }
            else{
              //the program should never be here. This is a program error!
              assert(false);
            }

            break;
          }

          case E_INTERSECT:
          {
//            cout<<"intersect "<<elem.pointInTime<<endl;

            IT posFirst = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);
            IT posNext = posFirst;
            ++posNext;

            IT posSec;
            if( posNext == localInfo->activeLine.end() ){
              //perhaps changed before
              bool t1 = check(localInfo->activeLine ,elem.pointInTime);
              assert(t1);

              break;
            }
            if( posNext->tuple != elem.tuple2 ){
              posSec = findActiveElem( localInfo->activeLine,
                elem.distance, elem.pointInTime, elem.tuple2);
            }
            else
              posSec = posNext;

                bool KNeighbor = false;
                IT posK = getKNeighbor(localInfo->activeLine,
                               localInfo->k, KNeighbor, elem.pointInTime);
                if(KNeighbor){
                   if(localInfo->up_list.size() == 0)
                      last_result_tid = posK->tuple->GetTupleId();

                    int index = localInfo->up_list.size() - 1;
                    Tuple* cloneTuple = NULL;
                    if(index < 0){
                      bool l =
                            UPAtInstant(posK->tuple, posK->start, attrNr);
                      bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        cloneTuple = changeTupleUnit(
                          posK->tuple, attrNr, posK->start,
                          elem.pointInTime, l, r);
                    }else{
                        UPoint temp_up = localInfo->up_list[index];
                        Instant up_end = temp_up.timeInterval.end;

                        bool l =
                            UPAtInstant(posK->tuple, up_end, attrNr);
                        bool r =
                          UPAtInstant(posK->tuple, elem.pointInTime, attrNr);

                        if(up_end < elem.pointInTime){
                          cloneTuple = changeTupleUnit(
                              posK->tuple, attrNr, up_end,
                              elem.pointInTime, l, r);
                        }
                     }
                    if(cloneTuple != NULL){
                      int pos = localInfo->eventQueue.GetPos();
                      UPoint* up = (UPoint*)cloneTuple->GetAttribute(pos);

//                       if(localInfo->AddNewUnit(up) == false)
//                         localInfo->up_list.push_back(*up);

                      if(localInfo->up_list.size() > 0 &&
                         last_result_tid == posK->tuple->GetTupleId())
                        localInfo->MergeUnit(up);
                      else
                        localInfo->up_list.push_back(*up);

                      delete cloneTuple;

                      ////////////record when the unit changes /////////////
                      if(posK->tuple->GetTupleId() != last_result_tid){
                        last_result_tid = posK->tuple->GetTupleId();
  //                      cout<<"intersect "<<last_result_tid<<endl;
                      }

                    }
                }

          //check if the first of the inters.-tuples is the k. and give it out

            if( checkK(localInfo->activeLine,localInfo->k,posFirst)){

              posFirst->start = elem.pointInTime;
              posFirst->lc = true;
              if( posNext != localInfo->activeLine.end() ){
                posNext->start = elem.pointInTime;
                posNext->lc = true;
              }
            }

            if( posNext == posSec){
              //look for intersections between the new neighbors
              checkIntersections(E_INTERSECT, elem.pointInTime,
                posFirst, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //swap the two entries in activeline
              ActiveElem e = *posFirst;
              *posFirst = *posSec;
              *posSec = e;
            }else{
              //the are some elements which has the same distance between
              //the two intersect elements, so calc like delete then insert
              vector<ActiveElem> va;
              set<Tuple*> s;
              pair< set<Tuple*>::iterator, bool > pr;

              ActiveElem newElem1(posFirst->distance,
                  posFirst->tuple, elem.pointInTime,
                  posFirst->end, posFirst->lc, posFirst->rc);
              va.push_back(newElem1);
              s.insert(posFirst->tuple);
              localInfo->activeLine.erase( posFirst );
              ActiveElem newElem2(posSec->distance,
                  posSec->tuple, elem.pointInTime,
                  posSec->end, posSec->lc, posSec->rc);
              va.push_back(newElem2);
              s.insert(posSec->tuple);
              localInfo->activeLine.erase( posSec );

              while( !localInfo->eventQueue.empty() ){
                //handle all intersections at the same time
                EventElem elemIs = localInfo->eventQueue.top();

                if( elemIs.type == E_INTERSECT
                  && elem.pointInTime == elemIs.pointInTime)
                {

                  localInfo->eventQueue.pop();
                  //eleminate same elements
                  deleteDupElements( localInfo->eventQueue, elemIs.type,
                      elemIs.tuple, elemIs.tuple2, elem.pointInTime);

                  pr = s.insert( elemIs.tuple );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                  pr = s.insert( elemIs.tuple2 );
                  if(pr.second == true)
                  {
                    //the element was not removed yet
                    IT pos = findActiveElem( localInfo->activeLine,
                      elemIs.distance, elemIs.pointInTime, elemIs.tuple2);
                    ActiveElem newElem(pos->distance,
                      pos->tuple, elem.pointInTime,
                      pos->end, pos->lc, pos->rc);
                    localInfo->activeLine.erase( pos );
                    va.push_back(newElem);
                  }
                }
                else{break;}
              }

              vector<IT> vit;
              for( unsigned int ii=0; ii < va.size(); ++ii ){
                  IT newPos = insertActiveElem( localInfo->activeLine,
                    va[ii], elem.pointInTime);
                  vit.push_back(newPos);
              }
              // check intersections
              for( unsigned int ii=0; ii < vit.size(); ++ii ){
                checkIntersections(E_LEFT, elem.pointInTime, vit[ii],
                  localInfo->activeLine, localInfo->eventQueue,
                  localInfo->endTime);
              }
            }

            break;
          }
        }

      }

      /////////////calculate the distance//////////////////////////////
      int mpos = 0;
      presult->Clear();
      presult->StartBulkLoad();
      for(unsigned int i = 0;i < localInfo->up_list.size();i++){
          MReal* mr = new MReal(0);
          GetDistance( mp, &localInfo->up_list[i], mpos, mr);
          for(int j = 0; j < mr->GetNoComponents(); j++){
             UReal uReal;
             mr->Get(j, uReal );
             presult->MergeAdd(uReal);
          }
          delete mr;
      }
      presult->EndBulkLoad( false, false );

      delete localInfo;

      qp->Close(args[0].addr);
      return 0;
}


/*
checks if there are intersections and puts them into the eventqueue
this is the right function if the eventqueue is a vector

*/
void checkIntersections(EventType type, Instant &time, unsigned int pos,
                      vector< ActiveElem > &v,
                      KNearestQueue &evq, Instant &endTime)
{
  switch ( type ){
    case E_LEFT:
    {
      Instant intersectTime( time.GetType() );
      if( pos + 1 < v.size() &&
        intersects( v[pos].distance, v[pos+1].distance,
        time, intersectTime, true ))
      {
        evq.push( EventElem(E_INTERSECT,
          intersectTime, v[pos].tuple, v[pos+1].tuple, v[pos].distance) );
      }
      if( pos > 0 &&
        intersects( v[pos-1].distance, v[pos].distance, time,
        intersectTime, true ))
      {
        evq.push( EventElem(E_INTERSECT, intersectTime,
          v[pos-1].tuple, v[pos].tuple, v[pos-1].distance) );
      }
      break;
    }
    case E_RIGHT:
    {
      if( pos > 0 && pos+1 < v.size() && time < endTime)
      {
        Instant intersectTime( time.GetType() );
        if( intersects( v[pos-1].distance, v[pos+1].distance,
          time, intersectTime, false ) )
        {
          evq.push( EventElem(E_INTERSECT, intersectTime,
            v[pos-1].tuple, v[pos+1].tuple, v[pos-1].distance) );
        }
      }
      break;
    }
    case E_INTERSECT:
    {
      //look for intersections between the new neighbors
      if( pos > 0 )
      {
        Instant intersectTime( time.GetType() );
        if( intersects( v[pos-1].distance, v[pos+1].distance,
          time, intersectTime, false ) )
        {
          evq.push( EventElem(E_INTERSECT, intersectTime, v[pos-1].tuple,
            v[pos+1].tuple, v[pos-1].distance) );
        }
      }
      if( pos+2 < v.size() )
      {
        Instant intersectTime( time.GetType() );
        if( intersects( v[pos].distance, v[pos+2].distance,
          time, intersectTime, false ) )
        {
          evq.push( EventElem(E_INTERSECT, intersectTime, v[pos].tuple,
            v[pos+2].tuple, v[pos].distance) );
        }
      }
      //look for new intersections between the old neighbors
      Instant intersectTime( time.GetType() );
      if( intersects( v[pos+1].distance, v[pos].distance,
        time, intersectTime, true ) )
      {
        evq.push( EventElem(E_INTERSECT, intersectTime,
          v[pos+1].tuple, v[pos].tuple, v[pos+1].distance) );
      }
      break;
    }
  }
}

/*
To use the plane sweep algrithmus a priority queue for the events and
a vector to save the active segments is needed.

*/
struct KnearestLocalInfoVector
{
  KnearestLocalInfoVector(Word& stream, int attrPos,
                    const MPoint* mp, int k1, bool scanFlag1):
     k(k1),scanFlag(scanFlag1), eventQueue(stream, qp, attrPos, mp){}
  unsigned int k;
  //int max;
  bool scanFlag;
  Instant startTime, endTime;
  vector< ActiveElem > activeLine;
  KNearestQueue eventQueue;
};

/*
knearestFunVector is the value function for the knearestvector operator
which uses a vector to handle the active elements. the functionality
is the same like knearestfun
The argument vector contains the following values:
args[0] = stream of tuples,
 the attribute given in args[1] has to be a unit
 the operator expects that the tuples are sorted by the time of the units
args[1] = attribute name
args[2] = mpoint
args[3] = int k, how many nearest are searched
args[4] = int j, attributenumber

*/
int knearestFunVector (Word* args, Word& result, int message,
             Word& local, Supplier s)
{

  KnearestLocalInfoVector *localInfo;

  switch (message)
  {
    case OPEN :
    {
      qp->Open(args[0].addr);
      CcInt* CcK = static_cast<CcInt*>(args[3].addr);
      if(!CcK->IsDefined()){
        local.setAddr(0);
        return 0;
      }
      int k = CcK->GetIntval();
      int attrPos = ((CcInt*)args[4].addr)->GetIntval() - 1;

      MPoint* mp = static_cast<MPoint*>(args[2].addr);
      if(!mp->IsDefined() || mp->IsEmpty()){
        local.setAddr(0);
        return 0;
      }

      Word stream(args[0]);
      localInfo = new KnearestLocalInfoVector(stream, attrPos, mp, k, true);
      localInfo->activeLine.reserve(100);
      local = SetWord(localInfo);
      UPoint up1, up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      localInfo->startTime = up1.timeInterval.start;
      localInfo->endTime = up2.timeInterval.end;
      return 0;
    }

    case REQUEST :
    {
      int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;
      localInfo = (KnearestLocalInfoVector*)local.addr;
      if(!localInfo){
        return CANCEL;
      }

      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      while ( !localInfo->eventQueue.empty() )
      {
        EventElem elem = localInfo->eventQueue.top();
        localInfo->eventQueue.pop();
        ActiveElem::currtime = elem.pointInTime;
        while( !localInfo->eventQueue.empty() )
        {
          //eleminate same elements
          EventElem elem2 = localInfo->eventQueue.top();
          if( elem.type != elem2.type
            || elem.pointInTime != elem2.pointInTime
            || elem.tuple != elem2.tuple || elem.tuple2 != elem2.tuple2)
          {
            break;
          }
          else
          {
            localInfo->eventQueue.pop();
          }
        }
        switch ( elem.type ){
          case E_LEFT:
          {
            // insert new element
            bool lc = elem.pointInTime >= localInfo->startTime
              ? elem.up->timeInterval.lc : false;
            ActiveElem newElem(elem.distance, elem.tuple, elem.pointInTime,
              elem.up->timeInterval.end, lc, elem.up->timeInterval.rc);
            unsigned int newPos = insertActiveElem( localInfo->activeLine,
              newElem, elem.pointInTime);

            // check intersections
            checkIntersections(E_LEFT, elem.pointInTime,
              newPos, localInfo->activeLine, localInfo->eventQueue,
              localInfo->endTime);

            // check if one element must given out
            if( localInfo->activeLine.size() > localInfo->k
              && newPos < localInfo->k)
            {
              //something in the first k has changed
              //now give out the k. element, but only until this time.
              //Clone the tuple and change the unit-attribut to the
              //new time interval
              bool rc = localInfo->activeLine[newPos].lc ? false
                : (localInfo->activeLine[localInfo->k].end == elem.pointInTime
                ? localInfo->activeLine[localInfo->k].rc : true);
              if( localInfo->activeLine[localInfo->k].start != elem.pointInTime
                || (rc && localInfo->activeLine[localInfo->k].lc))
              {
                Tuple* cloneTuple = changeTupleUnit(
                  localInfo->activeLine[localInfo->k].tuple,
                  attrNr, localInfo->activeLine[localInfo->k].start,
                  elem.pointInTime, localInfo->activeLine[localInfo->k].lc,
                  rc);
                localInfo->activeLine[localInfo->k].start = elem.pointInTime;
                localInfo->activeLine[localInfo->k].lc = false;
                result = SetWord(cloneTuple);
                return YIELD;
              }
            }
            break;
          }
          case E_RIGHT:
          {
            //a unit ends. It has to be removed from the map
            //look for the right tuple
            //(more elements can have the same distance)
            unsigned int posDel = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);

            Tuple* cloneTuple = NULL;
            if( posDel < localInfo->activeLine.size())
            {
              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 and the following to this time
              if( posDel < localInfo->k
                && (localInfo->activeLine[posDel].start != elem.pointInTime
                || (localInfo->activeLine[posDel].lc
                && localInfo->activeLine[posDel].rc)))
              {
                if( localInfo->activeLine[posDel].start < localInfo->endTime )
                {
                  cloneTuple = changeTupleUnit(
                    localInfo->activeLine[posDel].tuple, attrNr,
                    localInfo->activeLine[posDel].start, elem.pointInTime,
                    localInfo->activeLine[posDel].lc,
                    localInfo->activeLine[posDel].rc);
                }
                if( localInfo->k < localInfo->activeLine.size() )
                {
                  for( unsigned int ii = localInfo->k;
                      ii < localInfo->activeLine.size(); ++ii)
                  {
                    localInfo->activeLine[ii].start = elem.pointInTime;
                    localInfo->activeLine[ii].lc = false;
                  }
                }
              }
              //now calculate the intersection of the neighbors
              checkIntersections(E_RIGHT, elem.pointInTime,
                posDel, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //delete the element in the active elements
              localInfo->activeLine.erase( localInfo->activeLine.begin()
                                              + posDel );
              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
            }
            else
            {
              //the program should never be here. This is a program error!
              double slope;
              for( ITV it=localInfo->activeLine.begin();
                it!=localInfo->activeLine.end(); ++it)
              {
                cout << "dist: " << CalcDistance(it->distance,
                  elem.pointInTime,slope) << ", Tuple: " << it->tuple << endl;
              }
              assert(false);
            }
            if( cloneTuple )
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }
            break;
          }
          case E_INTERSECT:
            unsigned int pos = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple);

            unsigned int posnext = findActiveElem( localInfo->activeLine,
              elem.distance, elem.pointInTime, elem.tuple2);
            if( posnext < pos)
            {
              //perhaps changed before
              bool t1 = check(localInfo->activeLine ,elem.pointInTime);
              assert(t1);
              break;
            }

            //check if the first of the inters.-tuples is the k. and give it out
            Tuple* cloneTuple = NULL;
            if( pos + 1 == localInfo->k )
            {
              cloneTuple = changeTupleUnit( localInfo->activeLine[pos].tuple,
                attrNr, localInfo->activeLine[pos].start, elem.pointInTime,
                localInfo->activeLine[pos].lc,localInfo->activeLine[pos].rc);
              localInfo->activeLine[pos].start = elem.pointInTime;
              localInfo->activeLine[pos].lc = true;
              localInfo->activeLine[pos+1].start = elem.pointInTime;
              localInfo->activeLine[pos+1].lc = true;
            }

            if( posnext == pos + 1)
            {
              //look for intersections between the new neighbors
              //and the old neighbors
              checkIntersections(E_INTERSECT, elem.pointInTime,
                pos, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              //swap the two entries in activeline
              ActiveElem e = localInfo->activeLine[pos];
              localInfo->activeLine[pos] = localInfo->activeLine[pos+1];
              localInfo->activeLine[pos+1] = e;
            }
            else
            {
              //the are some elements which has the same distance between
              //the two intersect elements, so calc like delete then insert
              checkIntersections(E_RIGHT, elem.pointInTime,
                posnext, localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);

              ActiveElem newElem(localInfo->activeLine[posnext].distance,
                localInfo->activeLine[posnext].tuple, elem.pointInTime,
                localInfo->activeLine[posnext].end,
                localInfo->activeLine[posnext].lc,
                localInfo->activeLine[posnext].rc);
              localInfo->activeLine.erase( localInfo->activeLine.begin()
                                              + posnext );
              unsigned int newPos = insertActiveElem( localInfo->activeLine,
                newElem, elem.pointInTime);

              // check intersections
              checkIntersections(E_LEFT, elem.pointInTime, newPos,
                localInfo->activeLine, localInfo->eventQueue,
                localInfo->endTime);
            }
            if( cloneTuple )
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }
            break;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      localInfo = (KnearestLocalInfoVector*)local.addr;
      if(localInfo){
         delete localInfo;
         local.setAddr(0);
      }
      return 0;
    }
  }

  return 0;
}

/*
The ~oldknearestfilter~ operator results a stream of all input tuples which
can be the k-nearest units to the given mpoint. With the result of this
operator the knearest operator kann immediate be called. The tuples in the
result stream ordered by time.

The struct KnearestFilterLocalInfo is needed to save the data
from one to next function call.
All needet types are defined in NearestNeighborAlgebra.h

*/

/*
make a 2 dimensional box of a three dimensional

*/
const BBox<2> makexyBox( const BBox<3> box)
{
    return BBox<2>( true, box.MinD(0), box.MaxD(0),
                               box.MinD(1), box.MaxD(1));
}

/*
calculates the maximum distance of two 2 dimensional boxes

*/
double maxDistance( const BBox<2> box1, const BBox<2> box2)
{
  //the points of the vertices are (x,y) = (minD(0), minD(1))
  //and (maxD(0), maxD(1))
  double ax1, ay1, ax2, ay2;
  double bx1, by1, bx2, by2;
  ax1 = box1.MinD(0);
  ay1 = box1.MinD(1);
  ax2 = box1.MaxD(0);
  ay2 = box1.MaxD(1);
  bx1 = box2.MinD(0);
  by1 = box2.MinD(1);
  bx2 = box2.MaxD(0);
  by2 = box2.MaxD(1);
  //make four diagonals and calc the length, the the maximum
  double d1 = pow(bx2 - ax1,2) + pow(by2-ay1,2);
  double d2 = pow(bx2 - ax1,2) + pow(by1-ay2,2);
  double d3 = pow(bx1 - ax2,2) + pow(by2-ay1,2);
  double d4 = pow(bx1 - ax2,2) + pow(by1-ay2,2);
//  double d = MIN( (MIN( d1, d2)), (MIN( d3, d4)) );
  double d = MAX( (MAX( d1, d2)), (MAX( d3, d4)));
  return sqrt(d);
}

/*
the recursive helpfunction of the function coverage

*/
template<class timeType>
void helpcoverage(int cycle, map<timeType, int> &m, R_Tree<3, TupleId>* rtree,
                  timeType &t, SmiRecordId nodeid, int level)
{
  const int dim = 3;
  R_TreeNode<dim, TupleId> *tmp = rtree->GetMyNode(
            nodeid, false,
            rtree->MinEntries( level ),
            rtree->MaxEntries( level ) );
  for ( int ii = 0; ii < tmp->EntryCount(); ++ii )
  {
    if ( tmp->IsLeaf() )
    {
      R_TreeLeafEntry<dim, TupleId> e =
        (R_TreeLeafEntry<dim, TupleId>&)(*tmp)[ii];
      timeType ts(e.box.MinD(2));
      timeType te(e.box.MaxD(2));
      if( cycle == 0)
      {
        if( ts < t ) m[ts] = 0;
        if( te < t ) m[te] = 0;
      }
      else
      {
        typedef typename map<timeType, int>::iterator ITMAP;
        ITMAP it = m.find(ts);
        while( it != m.end() && it->first < te)
        {
          ++(it->second);
          ++it;
        }
      }
    }
    else
    {
      R_TreeInternalEntry<dim> e =
        (R_TreeInternalEntry<dim>&)(*tmp)[ii];
      helpcoverage<timeType>( cycle, m, rtree, t, e.pointer, level + 1);
    }
  }
  delete tmp;
}

/*
calculates the minimum coverage of a rtree-node
in its timeintervall

*/
template<class timeType>
int coverage(R_Tree<3, TupleId>* rtree, timeType &tEnd,
                 SmiRecordId nodeid, int level)
{
  map<timeType, int> m;
  helpcoverage<timeType>( 0, m, rtree, tEnd, nodeid, level );
  helpcoverage<timeType>( 1, m, rtree, tEnd, nodeid, level );
  typedef typename map<timeType , int>::const_iterator ITMAP;
  int result = 0;
  ITMAP it = m.begin();
  if( it != m.end() ) result = it->second;
  for( ; it != m.end(); ++it)
  {
    if( it->second < result ) result = it->second;
  }
  return result;
}
int newcoverage(R_Tree<3, TupleId>* rtree, Instant &tEnd,
                 SmiRecordId nodeid, int level,Relation* rel,BTree* btree)
{
  int result = 0;
  CcInt* id = new CcInt(true,nodeid);
  BTreeIterator* btreeiter = btree->ExactMatch(id);
  while(btreeiter->Next()){
      Tuple* tuple = rel->GetTuple(btreeiter->GetId(), false);
      UInt* ut = (UInt*)tuple->GetAttribute(2);//NodeId, RecId, Uint
      if(ut->timeInterval.Contains(tEnd))
        result += ut->constValue.GetValue();
  }
  delete id;
  return result;
}
/*
checks if the coverage k is reached. If yes, the new node or tuple
must not be inserted into the segment tree. If no, this
funtion inserts the node or tuple into the segment tree

*/
template<class timeType>
bool checkInsert( NNSegTree<timeType> &timeTree, SegEntry<timeType> &s,
                 BBox<2> mbox, int k, R_Tree<3, TupleId>* rtree, int level )
{
  double reachedCoverage = timeTree.calcCoverage( s.start, s.end, s.mindist );
  if( reachedCoverage >= k)
  {
    return false;
  }

  if( s.coverage != 1)
  {
    //for tuples the coverage must not be calculated
    s.coverage = coverage<timeType>(rtree, s.end, s.nodeid, level);
  }
  timeTree.insert( s, k );
  return true;
}

/*
Some elements are needed to save between the knearestfilter calls.

*/
template<class timeType>
struct KnearestFilterLocalInfo
{
  unsigned int k;
  //int max;
  bool scanFlag;
  timeType startTime, endTime;
  vector<FieldEntry<timeType> > vectorA;
  vector<FieldEntry<timeType> > vectorB;
  NNSegTree<timeType> timeTree;
  Relation* relation;
  R_Tree<3, TupleId>* rtree;
  Relation* hats; //new
  BTree* btreehats;//new

  map<SegEntry<timeType>, TupleId> resultMap;
  typedef typename map<SegEntry<timeType>, TupleId>::const_iterator CIMAP;
  CIMAP mapit;
  KnearestFilterLocalInfo( const timeType &s, const timeType &e) :
    startTime(s), endTime(e), timeTree( s, e )
    {}
};

/*
oldknearestFilterFun is the value function for the oldknearestfilter operator
It is a filter operator for the knearest operator. It can be called
if there exists a rtree for the unit attribute
The argument vector contains the following values:
args[0] = a rtree with the unit attribute as key
args[1] = the relation of the rtree
args[2] = mpoint
args[3] = int k, how many nearest are searched

*/
template<class timeType>
int oldknearestFilterFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  const int dim = 3;
  KnearestFilterLocalInfo<timeType> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      const MPoint *mp = (MPoint*)args[2].addr;
      UPoint up1, up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      BBTree<timeType>* t = new BBTree<timeType>(*mp);

      localInfo = new KnearestFilterLocalInfo<timeType>(
        up1.timeInterval.start.ToDouble(), up2.timeInterval.end.ToDouble());
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->k = (unsigned)((CcInt*)args[3].addr)->GetIntval();
      localInfo->scanFlag = true;
      local = SetWord(localInfo);
      if (mp->IsEmpty())
      {
        return 0;
      }
      /* build the segment tree */
      SmiRecordId adr = localInfo->rtree->RootRecordId();
      R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetMyNode(
                     adr,
                     false,
                     localInfo->rtree->MinEntries( 0 ),
                     localInfo->rtree->MaxEntries( 0 ) );

      timeType t1(tmp->BoundingBox().MinD(2));
      timeType t2(tmp->BoundingBox().MaxD(2));

      if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
      {
        localInfo->vectorA.push_back( FieldEntry<timeType>(
          localInfo->rtree->RootRecordId(), 0, t1, t2, 0));
        const BBox<2> xyBox = makexyBox( tmp->BoundingBox() );
        SegEntry<timeType> se(xyBox,t1, t2, 0.0, 0.0, 0,
              localInfo->rtree->RootRecordId(), -1);
        localInfo->timeTree.insert( se, localInfo->k );
      }
      delete tmp;
      while( !localInfo->vectorA.empty() )
      {
        unsigned int vpos;
        for( vpos = 0; vpos < localInfo->vectorA.size(); ++vpos)
        {
          FieldEntry<timeType> &f = localInfo->vectorA[ vpos ];
          SmiRecordId adr = f.nodeid;
          // check if this entry is not deleted
          if( localInfo->timeTree.erase( f.start, f.end, f.nodeid,
            f.maxdist) )
          {
            R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetMyNode(
                     adr, false,
                     localInfo->rtree->MinEntries( f.level ),
                     localInfo->rtree->MaxEntries( f.level ) );
            for ( int ii = 0; ii < tmp->EntryCount(); ++ii )
            {
              if ( tmp->IsLeaf() )
              {
                R_TreeLeafEntry<dim, TupleId> e =
                  (R_TreeLeafEntry<dim, TupleId>&)(*tmp)[ii];
                timeType t1(e.box.MinD(2));
                timeType t2(e.box.MaxD(2));
                if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
                {
                  const BBox<2> xyBox = makexyBox( e.box );
                  Interval<timeType> d(t1,t2,true,true);
                  const BBox<2> mBox(t->getBox(d));
                  SegEntry<timeType> se(xyBox,t1, t2,
                        xyBox.Distance( mBox),
                        maxDistance( xyBox, mBox), 1,
                        -1, e.info);
                  checkInsert<timeType>( localInfo->timeTree, se, mBox,
                    localInfo->k, localInfo->rtree, f.level+1);
                }
              }
              else
              {
                R_TreeInternalEntry<dim> e =
                  (R_TreeInternalEntry<dim>&)(*tmp)[ii];
                timeType t1(e.box.MinD(2));
                timeType t2(e.box.MaxD(2));
               if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
               {
                  const BBox<2> xyBox = makexyBox( e.box );
                  Interval<timeType> d(t1,t2,true,true);
                  const BBox<2> mBox(t->getBox(d));
                  SegEntry<timeType> se(xyBox,t1, t2,
                        xyBox.Distance( mBox),
                        maxDistance( xyBox, mBox), 0,
                        e.pointer, -1);
                  if( checkInsert<timeType>( localInfo->timeTree, se,
                        mBox, localInfo->k, localInfo->rtree, f.level+1))
                  {
                   localInfo->vectorB.push_back( FieldEntry<timeType>(
                          e.pointer, se.maxdist, t1, t2, f.level + 1));
                  }
                }
              }
            }
            delete tmp;
          }
        }
        localInfo->vectorA.clear();
        localInfo->vectorA.swap( localInfo->vectorB );
      }
      delete t;
      localInfo->timeTree.fillMap( localInfo->resultMap );
      localInfo->mapit = localInfo->resultMap.begin();
      return 0;
    }

    case REQUEST :
    {
     localInfo = (KnearestFilterLocalInfo<timeType>*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      /* give out alle elements of the resultmap */
      if ( localInfo->mapit != localInfo->resultMap.end() )
      {
          TupleId tid = localInfo->mapit->second;
          Tuple *tuple = localInfo->relation->GetTuple(tid, false);
          result = SetWord(tuple);
          ++localInfo->mapit;
          return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (KnearestFilterLocalInfo<timeType>*)local.addr;
      delete localInfo;
      return 0;
    }
  }

  return 0;
}



/*
2.3.4 Coverage

This function computes the coverage number for a three dimensional r-tree.


*/
ListExpr coverageTypeMapCommon(ListExpr args, ListExpr result){
  string err = " rtree(tuple(...) rect3 BOOL) expected";
  if(nl->ListLength(args) != 1){
    ErrorReporter::ReportError(err + "1");
    return nl->TypeError();
  }
  ListExpr arg = nl->First(args);
  if(nl->ListLength(arg)!=4){
    ErrorReporter::ReportError(err + "2");
    return nl->TypeError();
  }
  ListExpr rtree = nl->First(arg);
  ListExpr tuple = nl->Second(arg);
  /* The third element contains the type description for the
     attribute from which the r-tree was created. Because
     here we are not interested on that type, we can ignore it.
  */
  ListExpr dind  = nl->Fourth(arg);

  if(!nl->IsEqual(rtree,RTree3TID::BasicType())){
    ErrorReporter::ReportError(err + "3");
    return nl->TypeError();
  }

  if(nl->AtomType(dind)!=BoolType){
    ErrorReporter::ReportError(err + "5");
    return nl->TypeError();
  }

  if(nl->ListLength(tuple)!=2){
    ErrorReporter::ReportError(err + "6");
    return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(tuple),Tuple::BasicType())){
    ErrorReporter::ReportError(err + "7");
    return nl->TypeError();
  }
  return result;
}



inline ListExpr coverageTypeMap(ListExpr args){
  return coverageTypeMapCommon(args,
        nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("RecId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom(UInt::BasicType())
                    )))));
}


inline ListExpr coverage2TypeMap(ListExpr args){
  return coverageTypeMapCommon(args,
        nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Level"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom(MInt::BasicType())
                    )))));
}

const string coverageSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn)) ti) ->"
      " (stream (tuple ((Nodeid is)(Coverage uint))))</text--->"
      "<text> coverage(_) </text--->"
      "<text>Computes the coverage numbers for a given r-tree </text--->"
      "<text>query coverage(tree) count </text--->"
      ") )";

const string coverage2Spec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn)) ti) ->"
      " (stream (tuple ((Nodeid is)(Level int)(Coverage mint))))</text--->"
      "<text> coverage(_) </text--->"
      "<text>Computes the coverage numbers for a given r-tree </text--->"
      "<text>query coverage(tree) count </text--->"
      ") )";

struct CoverageEntry{

     CoverageEntry(R_TreeNode<3, TupleId>& node1,int nodeid1,
                    SmiRecordId recId1):
                      node(node1), position(0),nodeId(nodeid1),
                      recId(recId1){
        lastResult = new MInt(1);
     }

     CoverageEntry(const CoverageEntry& src):node(src.node),
                       position(src.position), lastResult(src.lastResult),
                       nodeId(src.nodeId),recId(src.recId){}

     CoverageEntry& operator=(const CoverageEntry& src){
        node = src.node;
        position = src.position;
        lastResult = src.lastResult;
        nodeId = src.nodeId;
        recId = src.recId;
        return *this;
     }

     void destroyResult(){
        if(lastResult){
           delete lastResult;
           lastResult = 0;
        }
     }

     R_TreeNode<3, TupleId> node;
     int position;
     MInt* lastResult;
     int nodeId; // id of the corrosponding node
     SmiRecordId recId;
};
/*
  for computing the newcoverage number

*/

class CoverageLocalInfo{
 public:

/*
~Constructor~

Creates a new CoverageLocalInfo object.

*/
   CoverageLocalInfo(R_Tree<3, TupleId>* tree, ListExpr tt){
     this->tree = tree;
     currentResult = 0;
     currentPos = 0;
     height = tree->Height();
     minInner = tree->MinEntries(0);
     maxInner = tree->MaxEntries(0);
     minLeaf  = tree->MinEntries(tree->Height());
     maxLeaf  = tree->MaxEntries(tree->Height());
     nodeidcounter = 0;
     // get the Root of the tree
     CoverageEntry entry(tree->Root(),nodeidcounter,tree->RootRecordId());
     nodeidcounter++;
     estack.push(entry);
     tupleType = new TupleType(tt);
     completeResult = 0;
     this->level = 0;
   }


/*
~Destructor~

Detroys this object.

*/
   ~CoverageLocalInfo(){
      while(!estack.empty()){
         CoverageEntry e = estack.top();
         estack.pop();
         if(e.lastResult){
            delete e.lastResult;
         }
      }
      if(currentResult){
        currentResult->Destroy();
        delete currentResult;
      }
      tupleType->DeleteIfAllowed();
      tupleType = 0;
      if(completeResult){
         completeResult->Destroy();
         delete completeResult;
         completeResult = 0;
      }
   }

/*
~Returns the next Tuple~

If the tree is exhausted, NULL is returned.

*/

   Tuple* nextTuple(){
      if(currentResult && currentPos>=currentResult->GetNoComponents()){
        // result completly processed
        currentResult->Destroy();
        delete currentResult;
        currentResult = 0;
        computeNextResult();
      }else if(!currentResult){
        // no result computed yet
        computeNextResult();
      }
      if(!currentResult){
         // tree exhausted
         return 0;
      }
      // get the next result
      UInt res;
      currentResult->Get(currentPos,res);
      currentPos++;
      Tuple* resTuple = new Tuple(tupleType);
      CcInt* ni = new CcInt(true,currentNodeId);
      resTuple->PutAttribute(0,ni);
      CcInt* ri = new CcInt(true,currentRecId);
      resTuple->PutAttribute(1,ri);
      resTuple->PutAttribute(2,new UInt(res));
      return resTuple;
   }

   Tuple* nextTuple2(){
      computeNextResult();
      if(!completeResult){
         return 0;
      } else {

//        completeResult->SortbyUnitTime();//to be sure sort by time
        Tuple* resTuple = new Tuple(tupleType);
        CcInt* ni = new CcInt(true,currentNodeId);
        CcInt* level  = new CcInt(true, this->level);
        resTuple->PutAttribute(0,ni);
        resTuple->PutAttribute(1,level);

        resTuple->PutAttribute(2,completeResult);
        completeResult = 0;
        return resTuple;
      }
   }

 private:
   R_Tree<3, TupleId>* tree;     // the tree
   stack<CoverageEntry> estack;  // the recursion stack
   MInt* currentResult;          // the currently computed result
   int currentPos;               // position within the current result
   int currentNodeId;            // the current nodeId
   int currentRecId;            // the current record id of the node
   TupleType* tupleType;         // the tupleType
   int maxInner;                 // maximum number of entries of inner nodes
   int minInner;                 // minimum number of entries of inner nodes
   int maxLeaf;                  // maximum number of entries of leaf nodes
   int minLeaf;                  // minimum number of entries of leaf nodes
   unsigned int height;          // height of the tree
   int nodeidcounter;            // counter for the current node id
   MInt* completeResult;         // result without applying the hat function
   int level;

/*
~rect2uint~

Supporting function creates a uint from a rect3.

*/
   UInt rect2uint(const Rectangle<3> rect){
     double min = rect.MinD(2);
     double max = rect.MaxD(2);
     DateTime dt1(instanttype);
     DateTime dt2(instanttype);
     dt1.ReadFrom(min);
     dt2.ReadFrom(max);
     Interval<DateTime> iv(dt1,dt2,true,true);
     CcInt v(true,1);
     UInt res(iv,v);
     return res;
   }

/*
~computeNextResult~

This function computes the next result.

*/
  void computeNextResult(){
     // for a new result, the position is 0
     currentPos = 0;
     // delete old currentResult if present
     if(currentResult){
       currentResult->Destroy();
       delete currentResult;
       currentResult=0;
     }
     if(completeResult){
       completeResult->Destroy();
       delete completeResult;
       completeResult=0;
     }

     if(estack.empty()){ // tree exhausted
       return;
     }

     // get the topmost stack element
     CoverageEntry coverageEntry = estack.top();
     this->level = estack.size()-1;
     if(!coverageEntry.node.IsLeaf()){
        // process an inner node
        if(coverageEntry.position>= coverageEntry.node.EntryCount()){
          // node finished
          currentResult = new MInt(1);
          // if a node is finished, the final result is lastResult of that node
          MInt* lastResult = coverageEntry.lastResult;
          // compute the Hat for lastResult
          assert(!completeResult);
          completeResult = new MInt(1);
          lastResult->fillUp(0,*completeResult);
          completeResult->Hat(*currentResult);
          currentPos = 0;
          currentNodeId = coverageEntry.nodeId;
          currentRecId = coverageEntry.recId;
          estack.pop(); // remove the finished node from the stack
          if(estack.empty()){ // root node
             coverageEntry.destroyResult();
             return;
          }
          // stack not empty -> update top element of the stack
          coverageEntry = estack.top();
          MInt tmp(1);
          coverageEntry.lastResult->PlusExtend(lastResult,tmp);
          coverageEntry.lastResult->CopyFrom(&tmp);
          lastResult->Destroy();
          delete lastResult;
          lastResult = 0;
          coverageEntry.position++; // this position has been computed
          // bring changes to the stack
          estack.pop();
          estack.push(coverageEntry);
          return; // computing the result finished
        } else {
         // not finished inner node
         // -> go to a leaf storing the path in the stack
         estack.pop(); // required to bring it again to the stack
         while(!coverageEntry.node.IsLeaf()){
            // push to stack
            estack.push(coverageEntry);
            R_TreeInternalEntry<3> next =
                    *(static_cast<R_TreeInternalEntry<3>*>(
                                  &coverageEntry.node[coverageEntry.position]));
            SmiRecordId rid = next.pointer;
            int min;
            int max;
            if(estack.size() == height){
               min = minLeaf;
               max = maxLeaf;
            } else {
               min = minInner;
               max = maxInner;
            }
            R_TreeNode<3, TupleId> nextNode(true,min,max);
            tree->GetNode(rid, nextNode);
            CoverageEntry nextEntry(nextNode,nodeidcounter,rid);
            nodeidcounter++;
            coverageEntry = nextEntry;
            // start debug
         }
         // now entry is a leaf node
         estack.push(coverageEntry);
        }
        this->level = estack.size()-1;
     }
     // the node to process is a leaf
     // build a single MInt from the whole node
     MInt res(1);
     MInt v(1);
     MInt tmp(1);
     for(int i=0;i<coverageEntry.node.EntryCount();i++){
       R_TreeLeafEntry<3, TupleId>* le = coverageEntry.node.GetLeafEntry(i);
       UInt nextUnit(rect2uint(le->box));
       v.Clear();
       v.SetDefined(true);
       v.Add(nextUnit);
       res.PlusExtend(&v,tmp);
       res.CopyFrom(&tmp);
     }
     nodeidcounter += coverageEntry.node.EntryCount();
     currentResult = new MInt(1);
     assert(!completeResult);
     completeResult = new MInt(1);
     res.fillUp(0,*completeResult);
     completeResult->Hat(*currentResult);
     currentPos = 0;
     currentNodeId = coverageEntry.nodeId;
     currentRecId = coverageEntry.recId;
     // delete the result pointer from the leaf node
     coverageEntry.destroyResult();
     estack.pop();
     if(estack.empty()){
       return;
     }
     coverageEntry = estack.top();
     tmp.Clear();
     coverageEntry.lastResult->PlusExtend(&res, tmp);
     coverageEntry.lastResult->CopyFrom(&tmp);
     coverageEntry.position++;
     estack.pop();
     estack.push(coverageEntry);
  }
};

int coverageFun (Word* args, Word& result, int message,
             Word& local, Supplier s){

  switch (message){
     case OPEN : {
                   ListExpr resultType =
                     SecondoSystem::GetCatalog()->NumericType(qp->GetType(s));
                   if(local.addr){
                     delete static_cast<CoverageLocalInfo*>(local.addr);
                   }
                   R_Tree<3, TupleId>* tree =
                         static_cast<R_Tree<3, TupleId>*>(args[0].addr);
                   local.addr = new CoverageLocalInfo(tree,
                                                nl->Second(resultType));
                   return 0;
                 }
     case REQUEST:{
                   CoverageLocalInfo* li =
                         static_cast<CoverageLocalInfo*>(local.addr);
                   Tuple* nextTuple = li->nextTuple();
                   result.addr = nextTuple;
                   return nextTuple?YIELD:CANCEL;
                  }
     case CLOSE:{
                   CoverageLocalInfo* li =
                         static_cast<CoverageLocalInfo*>(local.addr);
                   if(li){
                     delete li;
                     local.setAddr(0);
                   }
                   return 0;
                }

     default: assert(false); // unknown message
  }
}


int coverage2Fun (Word* args, Word& result, int message,
             Word& local, Supplier s){

  switch (message){
     case OPEN : {
                   ListExpr resultType =
                     SecondoSystem::GetCatalog()->NumericType(qp->GetType(s));
                   if(local.addr){
                     delete static_cast<CoverageLocalInfo*>(local.addr);
                   }
                   R_Tree<3, TupleId>* tree =
                         static_cast<R_Tree<3, TupleId>*>(args[0].addr);
                   local.addr = new CoverageLocalInfo(tree,
                                                nl->Second(resultType));
                   return 0;
                 }
     case REQUEST:{
                   CoverageLocalInfo* li =
                         static_cast<CoverageLocalInfo*>(local.addr);
                   Tuple* nextTuple = li->nextTuple2();
                   result.addr = nextTuple;
                   return nextTuple?YIELD:CANCEL;
                  }
     case CLOSE:{
                   CoverageLocalInfo* li =
                         static_cast<CoverageLocalInfo*>(local.addr);
                   if(li){
                     delete li;
                     local.setAddr(0);
                   }
                   return 0;
                }

     default: assert(false); // unknown message
  }
}

int isknnFun (Word* args, Word& result, int message,
             Word& local, Supplier s){
  bool debugme= false;

  string rel= ((CcString*)args[1].addr)->GetValue();
  string btree= ((CcString*)args[2].addr)->GetValue();
  string mboolAttr= ((CcString*)args[3].addr)->GetValue();
  string idAttr= ((CcString*)args[4].addr)->GetValue();
  string idType= ((CcString*)args[5].addr)->GetValue();
  string id="";
  //qp->Request(args[0].addr, idWord); //Strangely causes the program to crash
  Word idWord;
  Supplier arg0= qp->GetSupplierSon(s, 0);
  qp->Request(arg0, idWord);
  if(idType == CcString::BasicType())
    id= ((CcString*) idWord.addr)->GetValue() ;
  else if(idType == CcInt::BasicType())
    id= int2string( ((CcInt*) idWord.addr)->GetValue());

  if(debugme)
  {
    cout<<endl<<id<<endl<<rel<<endl<<btree<<
        endl<<mboolAttr<<endl<<idAttr<<endl<<idType;
    cout.flush();
  }

  string queryListStr=
    " (ifthenelse"
    "  (="
    "   (count"
    "       (exactmatch " + btree + " " + rel + " " + id + "))"
    "   0)"
    "  (mbool"
    "   ("
    "       ("
    "           (\"begin of time\" \"end of time\" FALSE FALSE)"
    "           FALSE)))"
    "  (extract"
    "   (exactmatch " + btree + " " + rel + " " + id + ")" +
        mboolAttr + "))";
  if(debugme)
  {
    cout<<endl<<queryListStr;
    cout.flush();
  }

  Word queryResult;
  QueryProcessor::ExecuteQuery( queryListStr, queryResult);

  result = qp->ResultStorage( s );
  ((MBool*)result.addr)->CopyFrom((MBool*)queryResult.addr);
  ((MBool*)queryResult.addr)->DeleteIfAllowed();
  return 0;
}

Operator coverageop (
         "coverage",        // name
         coverageSpec,      // specification
         coverageFun,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         coverageTypeMap    // type mapping
);

Operator coverage2op (
         "coverage2",        // name
         coverage2Spec,      // specification
         coverage2Fun,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         coverage2TypeMap    // type mapping
);

/*
2.4 Definition of value mapping vectors

*/

ValueMapping distanceScanMap [] = { distanceScanFun<2>,
                                    distanceScanFun<3>,
                                    distanceScanFun<4>,
                                    distanceScanFun<8>};

ValueMapping distanceScan2Map [] = {distanceScan2Fun<2>,
                                    distanceScan2Fun<3>,
                                    distanceScan2Fun<4>,
                                    distanceScan2Fun<8>};


ValueMapping distanceScan2SMap [] = {distanceScan2SFun<2>,
                                    distanceScan2SFun<3>,
                                    distanceScan2SFun<4>,
                                    distanceScan2SFun<8>};
template<class T1, class T2>
class DistFun{
public:
  inline double operator()(const T2& o1, const T1& o2) const{
     return o2.Distance(o1);
  }
};

template<class T1, class T2>
class DistFunSymm{
public:
  inline double operator()(const T2& o1, const T1& o2) const{
     return o1.Distance(o2);
  }
};

ValueMapping distanceScan3Map [] = {
   distanceScan3Fun<2, Point, Point, DistFun<Point,Point> >,
   distanceScan3Fun<2, Point, Points, DistFunSymm<Point,Points> >,
   distanceScan3Fun<2, Point, Line, DistFunSymm<Point,Line> >,
   distanceScan3Fun<2, Point, Region, DistFunSymm<Point,Region> >,
   distanceScan3Fun<2, Point, Rectangle<2>, DistFun<Point,Rectangle<2> > >,

   distanceScan3Fun<2, Points, Point, DistFun<Points,Point> >,
   distanceScan3Fun<2, Points, Points, DistFun<Points,Points> >,
   distanceScan3Fun<2, Points, Line, DistFunSymm<Points,Line> >,
   distanceScan3Fun<2, Points, Region, DistFunSymm<Points,Region> >,
   distanceScan3Fun<2, Points, Rectangle<2>, DistFun<Points,Rectangle<2> > >,

   distanceScan3Fun<2, Line, Point, DistFun<Line,Point> >,
   distanceScan3Fun<2, Line, Points, DistFun<Line,Points> >,
   distanceScan3Fun<2, Line, Line, DistFun<Line,Line> >,
   distanceScan3Fun<2, Line, Region, DistFunSymm<Line,Region> >,
   distanceScan3Fun<2, Line, Rectangle<2>, DistFun<Line,Rectangle<2> > >,

   distanceScan3Fun<2, Region, Point, DistFun<Region,Point> >,
   distanceScan3Fun<2, Region, Points, DistFun<Region,Points> >,
   distanceScan3Fun<2, Region, Line, DistFun<Region,Line> >,
   distanceScan3Fun<2, Region, Region, DistFun<Region,Region> >,
   distanceScan3Fun<2, Region, Rectangle<2>, DistFun<Region,Rectangle<2> > >,

   distanceScan3Fun<2, Rectangle<2>,Point,DistFunSymm<Rectangle<2> ,Point> >,
   distanceScan3Fun<2, Rectangle<2>,Points,DistFunSymm<Rectangle<2> ,Points > >,
   distanceScan3Fun<2, Rectangle<2>,Line,DistFunSymm<Rectangle<2> ,Line   > >,
   distanceScan3Fun<2, Rectangle<2>,Region,DistFunSymm<Rectangle<2>,Region > >,
   distanceScan3Fun<2, Rectangle<2>,Rectangle<2>,
                    DistFun<Rectangle<2>,Rectangle<2> > >

 };

ValueMapping newknearest_distMap [] = {
  newknearest_distFun1,
  newknearest_distFun2
};

int rect2periodsFun (Word* args, Word& result, int message,
             Word& local, Supplier s) {

  Rectangle<3>* arg  = static_cast<Rectangle<3>*>(args[0].addr);
  result = qp->ResultStorage(s);
  Periods* res = static_cast<Periods*>(result.addr);
  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  double min = arg->MinD(2);
  double max = arg->MaxD(2);
  DateTime dt1(instanttype);
  DateTime dt2(instanttype);
  dt1.ReadFrom(min);
  dt2.ReadFrom(max);
  Interval<DateTime> iv(dt1,dt2,true,true);
  res->Clear();
  res->Add(iv);
  return 0;
}


ValueMapping KnearestMap [] = {
  knearestFun1,
  knearestFun2
};

/*
2.5 Specification of operators

*/

const string distanceScanSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\"  )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_ _ distancescan [ _, _ ]</text--->"
      "<text> Computes the k nearest neighbors for a query object. "
      " The query object as well as the objects stored in the r-tree "
      " must be equal to their bounding boxes (i.e. point or retangle types)"
      " to produce correct results. The result comes ordered by the distance"
      " to the query object. If k <=0 all stored objects ordered by their"
      " distance to the query object is returned. </text--->"
      "<text>query kinos_geoData Kinos distancescan "
      "[[const point value (10539.0 14412.0)], 5] consume; </text--->"
      ") )";

const string distanceScan2Spec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_ _ distancescan2 [ _, _ ]</text--->"
      "<text> This operator works very similar to the distancescan "
      " operator but it allows an arbitrary spatial type for the "
      " query object. The objects indexed by the r-tree must be equal"
      " to their bounding boxes (point or rectangle). </text--->"
      "<text>query kinos_geoData Kinos distancescan2 "
      "[zoogarten, 5] tconsume; </text--->"
      ") )";

const string distanceScan2SSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x  T x k ->"
      " (stream (tupleId))"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_  distancescan2S [ _, _ ]</text--->"
      "<text> This operator is a variant od the distancescan2 "
      " operator. The differencs is that the relation is not required as"
      " an argument and the result is a stream of tupleIds instead of tuples." 
      "<text>query kinos_geoData distancescan2S "
      "[zoogarten, 5] transformstream tconsume; </text--->"
      ") )";


const string distanceScan3Spec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k  [x attrname] [x real] ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in {point, points, region, rect, rect},"
      " </text--->"
      "<text>_ _ distancescan [ _, _, _ ]</text--->"
      "<text>This operator returns the k nearest neighbours to a"
      " query object ordered by their distance to that object. "
      " Both, the query object and the objects indexed by the r-tree"
      " can be of an arbitrary supported spatial type."
      " If the tuples of the rtree (and the relation) contain more "
      " than one spatial attribute, the indexed attribute must be given"
      " by its name as an argument. If it is unique, that parameter "
      " can be omited. Because the distance calculation requires access to"
      " the tuples, this operator is slower than the distancescan2 operator."
      " For this reason, this operator should only ne used if the indexed "
      " spatial objects are not of type point or rectangle."
      " If the last argument is a real value, the output stream is finished,"
      " if the current distance overcomes the given value. "
      "</text--->"
      "<text> query Flaechen creatertree[geoData] Flaechen "
      "distancescan3[BGrenzenLine, 5] tconsume"
      "</text--->"
      ") )";

const string distanceScan4Spec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree3 x rel(T) x point x instant x int [ x attrname] "
      " -> stream(T) </text--->"
      "<text>_ _ distancescan4 [ _, _, _ ]</text--->"
      "<text> Computes the k nearest neighbours of a point from "
      " a set of moving point units (stored in rel and indexed by the rtree)"
      " for a given point in time. If the relation contains more than 1 "
      " attributes of type upoint, the attribute name of the indexed "
      " attribute must be a parameter of that operator </text--->"
      "<text>query UnitTrains_rtree UnitTrains distancescan4 "
      "  [[const point value (10539.0 14412.0)], now(),  5] tconsume;"
      "</text--->))";

const string knearestvectorSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ knearestvector [_, _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which contains the k-nearest units to the given mpoint. "
      "The tuples are splitted into multiple tuples with disjoint "
      "units if necessary. The tuples in the result stream are "
      "not necessarily ordered by time or distance to the given "
      "mpoint. The operator expects that the input stream with "
      "the tuples are sorted by the time of the units</text--->"
      "<text>query query UnitTrains feed head[20] knearestvector "
      "[UTrip,train1, 2] consume;</text--->"
      ") )";

const string knearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x int [x int] ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ knearest [_, _, _ [, _] ]</text--->"
      "<text>The purpose of the operator is to compute the time dependent"
      " k nearest neighbors to a given moving point mp within a set of "
      "moving objects S. Set S is provided as the first argument in the form"
      " of a stream of units, i.e., a stream of tuples having a upoint "
      "atribute. This stream of tuples MUST be ordered by unit start time. "
      "Given a call s knearest[attr, mp, k], the result is a stream of tuples"
      " in the same format as the input stream containing for each instant the"
      " k closest units to mp. Note that the original units may have been "
      "split into pieces. The tuples in the result stream are not "
      "necessarily ordered by time or distance to the given mpoint.\n\n"
      "The operator may not work correctly for geographical coordinates due"
      " to numerical problems with small distance differences, if called as"
      " described above. For moving objects with geographical (LON, LAT)"
      " coordinates, use the second form with an additional int parameter j,"
      " that is, s knearest[attr, mp, k, j]. The parameter, if present, "
      "specifies that Gauss Krueger projection is to be used before computing"
      " distances. The value of j, if in the range 0 <= j < 120 specifies a "
      "meridian to be used in the Gauss Krueger projection (see the "
      "explanation of the gk operator). If j is -1, then the location of the "
      "first unit in the input stream is used to determine the Gauss Krueger "
      "zone number. Other values of j lead to error messages. Also if a wrong"
      " zone is specified (input data do not lie in the given zone), an error"
      " message is generated. </text--->"
      "<text>query query UnitTrains feed head[20] knearest "
      "[UTrip,train1, 2] count;</text--->"
      ") )";

const string knearestdistSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x k -> mreal </text--->"
      "<text>_ knearest_dist [_, _, _ ]</text--->"
      "<text>The operator results the distance of kth neighbor. </text--->"
      "<text>query query UnitTrains feed head[20] knearest_dist "
      "[UTrip,train1, 2];</text--->"
      ") )";

const string oldknearestFilterSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ oldknearestfilter [ _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which are the k-nearest tupels to the given mpoint. "
      "The operator do not separate tupels if necessary. The "
      "result may have more than the k-nearest tupels. It is a "
      "filter operator for the knearest operator, if there are a great "
      "many input tupels. "
      "The operator expects a thee dimensional rtree where the "
      "third dimension is the time</text--->"
      "<text>query UTOrdered_RTree UTOrdered oldknearestfilter "
      "[train1, 5] count;</text--->"
      ") )";


const string rect2periodsSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rect3 -> periods </text--->"
      "  <text> rect2periods(_)</text--->"
      "  <text> Creates a single-interval periods value from "
      "  the third dimension of a rectangle </text--->"
      "  <text>query rect2periods(rect1) </text--->"
      ") )";

const string isknnSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>v in {int | string} x string(rel) x string(btree) x "
      "string(MBool) x string(ID) -> APPEND v mbool"
      "  <text> isknn(_, _, _,_,_)</text--->"
      "  <text> This operator is implemented only in SQL. Unless you are"
      " sure of what you are doing, do not use it. </text--->"
      "  <text>isknn(.Tripid,\"trainsnnTriptrain75NN\" , "
      "\"trainsnnTriptrain75NN_Tripid_btree\" , \"MBoolRes\" "
      ", \"Tripid\")]  </text--->"
      ") )";

/*
2.6 Definition of operators

*/
Operator distancescan (
         "distancescan",        // name
         distanceScanSpec,      // specification
         4,                     //number of overloaded functions
         distanceScanMap,       // value mapping
         distanceScanSelect,
         distanceScanTypeMap    // type mapping
);

Operator distancescan2 (
         "distancescan2",        // name
         distanceScan2Spec,      // specification
         4,                     //number of overloaded functions
         distanceScan2Map,       // value mapping
         distanceScanSelect,
         distanceScanTypeMap    // type mapping
);

Operator distancescan2S (
         "distancescan2S",        // name
         distanceScan2SSpec,      // specification
         4,                     //number of overloaded functions
         distanceScan2SMap,       // value mapping
         distanceScanSelect,
         distanceScan2STypeMap    // type mapping
);

Operator distancescan3 (
         "distancescan3",        // name
         distanceScan3Spec,      // specification
         25,                     //number of overloaded functions
         distanceScan3Map,       // value mapping
         distanceScan3Select,
         distanceScan3TypeMap    // type mapping
);

Operator distancescan4 (
         "distancescan4",        // name
         distanceScan4Spec,      // specification
         distanceScan4Fun,       // value mapping
         Operator::SimpleSelect, // trivial selection function
         distanceScan4TypeMap    // type mapping
);


Operator knearest (
         "knearest",            // name
         knearestSpec,          // specification
         2,
         KnearestMap,           // value mapping
         KnearestSelect,
         KnearestTypeMap        // type mapping
);

Operator knearest_dist (
         "knearest_dist",            // name
         knearestdistSpec,          // specification
         2,
         newknearest_distMap,           // value mapping
         knearestdistSelect,
         knearestdistTypeMap        // type mapping
);

Operator knearestvector (
         "knearestvector",      // name
         knearestvectorSpec,          // specification
         knearestFunVector,     // value mapping
         Operator::SimpleSelect,// trivial selection function
         knearestTypeMap        // type mapping
);

Operator oldknearestfilter (
         "oldknearestfilter",        // name
         oldknearestFilterSpec,      // specification
         oldknearestFilterFun<double>,       // value mapping
         //knearestFilterFun<Instant>,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         oldknearestFilterTypeMap    // type mapping
);


Operator rect2periods (
         "rect2periods",            // name
         rect2periodsSpec,          // specification
         rect2periodsFun,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         rect2periodsTypeMap        // type mapping
);

Operator isknn (
         "isknn",            // name
         isknnSpec,          // specification
         isknnFun,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         isknnTypeMap        // type mapping
);

/*
6 Operator ~bboxes~

The operator bbox is for test proposes only. It receives a stream of
Periods values and a moving point. It produces a stream of rectangles
which are the bounding boxes of the moving point for each box of the
periods value.

*/
ListExpr bboxesTM(ListExpr args){
  string err = "stream(periods) x mpoint expected";
  if(nl->ListLength(args) != 2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  ListExpr stream = nl->First(args);
  ListExpr mp = nl->Second(args);
  if(!nl->IsEqual(mp,MPoint::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(nl->ListLength(stream)!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(stream),Symbol::STREAM())   ||
     !nl->IsEqual(nl->Second(stream),Periods::BasicType())){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                         nl->SymbolAtom(Rectangle<2>::BasicType()));

}

int bboxesFun(Word* args, Word& result, int message,
              Word& local, Supplier s){

 switch(message){
   case OPEN: {
     qp->Open(args[0].addr);
     MPoint* mp = static_cast<MPoint*>(args[1].addr);
     if(local.addr){
       delete static_cast<BBTree<Instant>*>(local.addr);
       local.addr = 0;
     }
     if(mp->IsDefined()){
        BBTree<Instant>* t = new BBTree<Instant>(*mp);
        local.addr = t;
        //cout << "mp.size = " << mp->GetNoComponents() << endl;
        //cout << "no_Leafs = " << t->noLeaves() << endl;
        //cout << "noNodes = " << t->noNodes() << endl;
        //cout << "height = " << t->height() << endl;
        //cout << "tree " << endl << *t << endl << endl;
     }
     return 0;
   }
   case REQUEST: {
     if(!local.addr){
       return CANCEL;
     }

     Word elem;
     qp->Request(args[0].addr,elem);
     if(!qp->Received(args[0].addr)){
        return CANCEL;
     }
     Range<Instant>* periods = static_cast<Range<Instant>* >(elem.addr);
     if(!periods->IsDefined()){
         result.addr = new Rectangle<2>(false);
         periods->DeleteIfAllowed();
         return YIELD;
     }
     Instant minInst;
     periods->Minimum(minInst);
     Instant maxInst;
     periods->Maximum(maxInst);
     Interval<Instant> d(minInst,maxInst,true,true);
     BBTree<Instant>* t = static_cast<BBTree<Instant>*>(local.addr);
     result.addr = new Rectangle<2>(t->getBox(d));
     periods->DeleteIfAllowed();
     return YIELD;
   }
   case CLOSE : {
     qp->Close(args[0].addr);
     if(local.addr){
       delete static_cast<BBTree<Instant>*>(local.addr);
       local.addr = 0;
     }
     return 0;
   }
 }
 return 0;

}


const string bboxesSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(periods) x mpoint -> stream(rect)</text--->"
      "<text>_ bboxes [ _ ]</text--->"
      "<text>"
        "The Operator builds for each periods value its enclosing interval."
        "It returns for that interval the spatial bounding boc if the "
        "moving point would be restricted to that interval."
      "</text--->"

       "<text>query query UnitTrains feed  projectextend[; D : deftime[.UTrip]"
              " transformstream bboxes[Train6] transformstream consume"
      "</text--->"
      ") )";


Operator bboxes (
         "bboxes",        // name
         bboxesSpec,      // specification
         bboxesFun,      // value mapping
         Operator::SimpleSelect, // trivial selection function
         bboxesTM    // type mapping
);



/*

knearestFilterFun is the value function for the knearestfilter operator
It is a filter operator for the knearest operator. It can be called
if there exists a rtree for the unit attribute and a btree for the units number
relation built on rtree node
The argument vector contains the following values:
args[0] = a rtree with the unit attribute as key
args[1] = the relation of the rtree
args[2] = a btree with nodeid as key
args[3] = the relation of the btree
args[4] = mpoint
args[5] = int k, how many nearest are searched

*/
template<class timeType>
bool CmpFiledEntry(const FieldEntry<timeType>& fe1,
const FieldEntry<timeType>& fe2)
{
  if(fe1.start != fe2.start)
    return fe1.start < fe2.start;
  if(fe1.end != fe2.start)
    return fe1.end < fe2.end;
  return fe1.nodeid < fe2.nodeid;

}
template<class timeType>
int knearestFilterFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  const int dim = 3;
  KnearestFilterLocalInfo<timeType> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      const MPoint *mp = (MPoint*)args[5].addr;//5 th parameter
      UPoint up1, up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      BBTree<timeType>* t = new BBTree<timeType>(*mp);
      localInfo = new KnearestFilterLocalInfo<timeType>(
        up1.timeInterval.start.ToDouble(), up2.timeInterval.end.ToDouble());
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->btreehats = (BTree*)args[2].addr;
      localInfo->hats = (Relation*)args[3].addr;
      localInfo->k = (unsigned)((CcInt*)args[6].addr)->GetIntval();//the last
      localInfo->scanFlag = true;
      local = SetWord(localInfo);
      if (mp->IsEmpty())
      {
        return 0;
      }
      /* build the segment tree */
      SmiRecordId adr = localInfo->rtree->RootRecordId();
      R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetMyNode(
                     adr,
                     false,
                     localInfo->rtree->MinEntries( 0 ),
                     localInfo->rtree->MaxEntries( 0 ) );
      timeType t1(tmp->BoundingBox().MinD(2));
      timeType t2(tmp->BoundingBox().MaxD(2));

      if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
      {
        localInfo->vectorA.push_back( FieldEntry<timeType>(
          localInfo->rtree->RootRecordId(), 0, t1, t2, 0));
        const BBox<2> xyBox = makexyBox( tmp->BoundingBox() );
        SegEntry<timeType> se(xyBox,t1, t2, 0.0, 0.0, 0,
              localInfo->rtree->RootRecordId(), -1);
        localInfo->timeTree.insert( se, localInfo->k );
      }
      delete tmp;
///////////////////
      vector<unsigned int> randnum;
      srand(time(0));
      unsigned int entrycount = localInfo->rtree->MaxEntries(0);
      int count = 0;
      for (unsigned int i = 0 ;i < entrycount;){
        unsigned int num = rand() % entrycount;
        srand(count);
        count++;
        i++;
        unsigned int j = 0;
        for(;j < randnum.size();j++){
          if(randnum[j] == num){
             i--;
             break;
          }
        }
        if(j == randnum.size())
          randnum.push_back(num);
      }
///////////////////

      while( !localInfo->vectorA.empty() )
      {
        unsigned int vpos;
        stable_sort(localInfo->vectorA.begin(),localInfo->vectorA.end(),
        CmpFiledEntry<timeType>); //order by time
        for( vpos = 0; vpos < localInfo->vectorA.size(); ++vpos)
        {
          FieldEntry<timeType> &f = localInfo->vectorA[ vpos ];
          SmiRecordId adr = f.nodeid;
          // check if this entry is not deleted
          if( localInfo->timeTree.erase( f.start, f.end, f.nodeid,
            f.maxdist) )
          {
            R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetMyNode(
                     adr, false,
                     localInfo->rtree->MinEntries( f.level ),
                     localInfo->rtree->MaxEntries( f.level ) );

            if(tmp->IsLeaf()){
//             for ( int ii = 0; ii < tmp->EntryCount(); ++ii ){
                for(unsigned int index = 0; index < randnum.size();index++){
                int ii = randnum[index];
                if(ii >= tmp->EntryCount())
                  continue;

                R_TreeLeafEntry<dim, TupleId> e =
                  (R_TreeLeafEntry<dim, TupleId>&)(*tmp)[ii];
                timeType t1((double)(e.box.MinD(2)));
                timeType t2((double)(e.box.MaxD(2)));
                if(t1 > f.end)
                  break;
                if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime)){
                  const BBox<2> xyBox = makexyBox( e.box );
                  Interval<timeType> d(t1,t2,true,true);
                  const BBox<2> mBox(t->getBox(d));
                  SegEntry<timeType> se(xyBox,t1, t2,
                        xyBox.Distance( mBox),
                        maxDistance( xyBox, mBox), 1,
                        -1, e.info);

                double reachcov =
                localInfo->timeTree.calcCoverage(t1,t2,xyBox.Distance(mBox));
                if(reachcov < localInfo->k)
                  localInfo->timeTree.insert(se,localInfo->k);
                }
              }
            }else{ //Internal node
                vector<Interval<Instant> > interv;
                vector<int> covs;
                CcInt* id = new CcInt(true,f.nodeid);
                BTreeIterator* btreeiter = localInfo->btreehats->ExactMatch(id);
                while(btreeiter->Next()){
                  Tuple* tuple = localInfo->hats->GetTuple(btreeiter->GetId()
                                                           , false);
                  UInt* ut = (UInt*)tuple->GetAttribute(2);//NodeId, RecId, Uint
                  //////////////////////////////////////////////
                  if(ut->constValue.GetValue() > 0){
                    interv.push_back(ut->timeInterval);
                    covs.push_back(ut->constValue.GetValue());
                  }
                  //////////////////////////////////////////////
                  tuple->DeleteIfAllowed();
                }
                delete id;
                delete btreeiter;

//              for ( int ii = 0; ii < tmp->EntryCount(); ++ii ){
                for(unsigned int index = 0; index < randnum.size();index++){
                int ii = randnum[index];
                if(ii >= tmp->EntryCount())
                  continue;
                R_TreeInternalEntry<dim> e =
                  (R_TreeInternalEntry<dim>&)(*tmp)[ii];
                timeType t1(e.box.MinD(2));
                timeType t2(e.box.MaxD(2));
                if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime)){
                    const BBox<2> xyBox = makexyBox( e.box );
                    Interval<timeType> d(t1,t2,true,true);
                    const BBox<2> mBox(t->getBox(d));
                    if(interv.size() == 3){ //exit hat
                      int cov = 0;

//                      for(unsigned int j = 0;j < interv.size();j++){
//                        if(interv[j].Contains(t2)){
//                          cov = covs[j];
//                          break;
//                        }
//                      }
                      for(unsigned int j = 0;j < interv.size();j++){
                        if(interv[j].Contains(t2) && interv[j].Contains(t1)){
                          cov = covs[j];
                          break;
                        }
                        if(interv[0].Contains(t1) && interv[1].Contains(t2)){
                          cov = covs[0]<covs[1]?covs[0]:covs[1];
                          break;
                        }
                        if(interv[1].Contains(t1) && interv[2].Contains(t2)){
                          cov = covs[1]<covs[2]?covs[1]:covs[2];
                          break;
                        }
                      }

                      SegEntry<timeType> se(xyBox,t1, t2,
                          xyBox.Distance( mBox),
                          maxDistance( xyBox, mBox), cov,
                          e.pointer, -1);
                      double reachedCoverage =
                        localInfo->timeTree.calcCoverage( t1, t2, se.mindist );
                      if(reachedCoverage < localInfo->k){
                        localInfo->timeTree.insert( se, localInfo->k);
                        localInfo->vectorB.push_back(FieldEntry<timeType>(
                            se.nodeid, se.maxdist, se.start, se.end,
                            f.level+1));
                      }
                   }else{// no hat
                        SegEntry<timeType> se(xyBox,t1, t2,
                        xyBox.Distance( mBox),
                        maxDistance( xyBox, mBox), 0,
                        e.pointer, -1);
                    if( checkInsert( localInfo->timeTree, se,
                        mBox, localInfo->k, localInfo->rtree, f.level+1))
                      {
                          localInfo->vectorB.push_back( FieldEntry<timeType>(
                          e.pointer, se.maxdist, t1, t2, f.level + 1));
                      }
                    }
                }
            }
          }//else
            delete tmp;
          }
        }
        localInfo->vectorA.clear();
        localInfo->vectorA.swap( localInfo->vectorB );
      }
      delete t;

      localInfo->timeTree.fillMap( localInfo->resultMap );
      localInfo->mapit = localInfo->resultMap.begin();

      return 0;
    }

    case REQUEST :
    {
     localInfo = (KnearestFilterLocalInfo<timeType>*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      /* give out alle elements of the resultmap */
      if ( localInfo->mapit != localInfo->resultMap.end() )
      {
          TupleId tid = localInfo->mapit->second;
          Tuple *tuple = localInfo->relation->GetTuple(tid, false);
//split units
          const MPoint *mp = (MPoint*)args[5].addr;//5 th parameter
          UPoint up1, up2;
          mp->Get( 0, up1);
          mp->Get( mp->GetNoComponents() - 1, up2);
          int attrpos = ((CcInt*)args[7].addr)->GetIntval() - 1;
          UPoint* up = (UPoint*)tuple->GetAttribute(attrpos);
          Point p0;
          if(up->timeInterval.Contains(up1.timeInterval.start)){
            up->TemporalFunction(up1.timeInterval.start,p0,true);
            if(p0.IsDefined()){
              up->timeInterval.start = up1.timeInterval.start;
              up->p0 = p0;
            }
          }
          Point p1;
          if(up->timeInterval.Contains(up2.timeInterval.end)){
            up->TemporalFunction(up2.timeInterval.end,p1,true);
            if(p1.IsDefined()){
              up->timeInterval.end = up2.timeInterval.end;
              up->p1 = p1;
            }
          }
//
          result = SetWord(tuple);
          ++localInfo->mapit;
          return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (KnearestFilterLocalInfo<timeType>*)local.addr;
      delete localInfo;
      return 0;
    }
  }

  return 0;
}


/*
The function knearestFilterTypeMap is the type map for the
operator knearestfilter

*/
ListExpr
knearestFilterTypeMap( ListExpr args )
{

  if(nl->ListLength(args) != 7){
    return listutils::typeError("7 arguments expected");
  }

  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr btreeDescription = nl->Third(args);
  ListExpr brelDescription = nl->Fourth(args);
  ListExpr attrName = nl->Fifth(args);
  ListExpr queryobject = nl->Sixth(args);
  ListExpr quantity = nl->Nth(7,args);

  string err = "rtree x rel x btree x attrname x mpoint x int expected";

  if( !listutils::isRTreeDescription(rtreeDescription) ||
      !listutils::isRelDescription(relDescription) ||
      !listutils::isBTreeDescription(btreeDescription) ||
      !listutils::isRelDescription(brelDescription) ||
      !listutils::isSymbol(attrName) ||
      !listutils::isSymbol(queryobject,MPoint::BasicType()) ||
      !listutils::isSymbol(quantity,CcInt::BasicType())){
    return listutils::typeError(err);
  }

  ListExpr attrType;
  string aname = nl->SymbolValue(attrName);
  int j = listutils::findAttribute(nl->Second(nl->Second(relDescription)),
                                   aname,attrType);
  if(j==0 || !listutils::isSymbol(attrType,UPoint::BasicType())){
    return listutils::typeError("attr name " + aname + "  not found "
                                "or not of type upoint");
  }

  ListExpr rtreeSymbol = nl->First(rtreeDescription);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  if(!listutils::isSymbol(rtreeKeyType)){
    return listutils::typeError("rtree key type must be atomic");
  }
  if(!listutils::isKind(rtreeKeyType,Kind::SPATIAL3D()) &&
     !listutils::isSymbol(rtreeKeyType,Rectangle<3>::BasicType())){
   return listutils::typeError("rtree key type must in kind "
                               "SPATIAL3D or be  a rect3");
  }

  if(!listutils::isSymbol(rtreeSymbol,RTree3TID::BasicType())){
    return listutils::typeError("type of rtree is not rtree3");
  }

  /* check that rtree and rel have the same associated tuple type */
  ListExpr attrList = nl->Second(nl->Second(relDescription));
  ListExpr rtreeAttrList = nl->Second(nl->Second(rtreeDescription));
  if(!nl->Equal(attrList, rtreeAttrList)){
    return listutils::typeError("types of relation and rtree differ");
  }

  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->Second(relDescription));

  return  nl->ThreeElemList(
             nl->SymbolAtom(Symbol::APPEND()),
             nl->OneElemList(nl->IntAtom(j)),
            res);
}
const string knearestFilterSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
     " ti) x rel1(tuple ((x1 t1)...(xn tn))) x btree(tuple ((x1 t1)...(xn tn)))"
      " x rel2(tuple ((x1 t1)...(xn tn))) x xi x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ _ _ knearestfilter [_, _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which are the k-nearest tupels to the given mpoint. "
      "The operator do not separate tupels if necessary. The "
      "result may have more than the k-nearest tupels. It is a "
      "filter operator for the knearest operator, if there are a great "
      "many input tupels. "
      "The operator expects a three dimensional rtree where the "
      "third dimension is the time</text--->"
      "<text>query UTOrdered_RTree UTOrdered btreehats hats knearestfilter "
      "[UTrip,train1, 5] count;</text--->"
      "))";
Operator knearestfilter (
         "knearestfilter",        // name
         knearestFilterSpec,      // specification
         //newknearestFilterFun<Instant>,       // value mapping
         knearestFilterFun<double>,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         knearestFilterTypeMap    // type mapping
);


struct PointNeighbor{
  Point q; //query point
  Point p; //data point
  float dist;
  PointNeighbor(){dist = 0.0;}
  PointNeighbor(Point&a,Point& b):q(a),p(b){dist = q.Distance(p);}
  PointNeighbor(const PointNeighbor& pn):q(pn.q),p(pn.p),dist(pn.dist){}
};
bool CmpPointNeighbor(const PointNeighbor& e1,const PointNeighbor& e2)
{
  return e1.q.Distance(e1.p) < e2.q.Distance(e2.p);
}
struct PruneLeafNode{
  long nodeid;
  BBox<2> box;
  PruneLeafNode(long id,BBox<2> b):nodeid(id),box(b){}
};

struct MQKnearest{
  Relation* querypoints;
  Relation* datapoints;
  R_Tree<2,TupleId>* rtree;
  Relation* cov;
  BTree* btreecov;
  Relation* leafcov;
  BTree* btreeleafcov;
  unsigned int k;
  TupleType* resulttype;
  vector<PointNeighbor> results;
  vector<PointNeighbor>::iterator iter;
  bool nextblock;
  vector<Point> block; //split query objects into blocks
  vector<Point> data; //the candidate for current block
  int index; //position in rel querypoints
  MQKnearest(){}
};
template<class timeType>
bool CmpFE(const FieldEntry<timeType>& fe1, const FieldEntry<timeType>& fe2)
{
  if(fe1.mindist < fe2.mindist)
    return true;
  if(fe1.maxdist < fe2.maxdist)
    return true;
  return false;
}
template<class timeType>
bool CmpEFE(const EFieldEntry<timeType>& fe1, const EFieldEntry<timeType>& fe2)
{
  if(fe1.mindist < fe2.mindist)
    return true;
  if(fe1.maxdist < fe2.maxdist)
    return true;
  return false;
}
void Mqkfilter(MQKnearest* mqk,vector<TupleId>& datanode,BBox<2> query)
{
  int minentries = mqk->rtree->MinEntries(0);
  int maxentries = mqk->rtree->MaxEntries(0);
  SmiRecordId adr = mqk->rtree->RootRecordId();
  vector<EFieldEntry<double> > array1;
  vector<EFieldEntry<double> > array2;
  vector<EFieldEntry<double> > candidate;
  EFieldEntry<double> fe(adr,0,0,0,0,0);
  array1.push_back(fe);
  unsigned int pos;
  //BF-first method
  while(array1.empty() == false){
    array2.clear();
    for(pos = 0;pos < array1.size();pos++){
      fe = array1[pos];
      adr = fe.nodeid;
      R_TreeNode<2,TupleId>* node = mqk->rtree->GetMyNode(
          adr,false,minentries,maxentries);
      if(node->IsLeaf()){
        candidate.push_back(fe);//leaf node (id)
        continue;
     }
      for(int i = 0; i < node->EntryCount();i++){ //Internal node
        R_TreeInternalEntry<2> e = (R_TreeInternalEntry<2>&)(*node)[i];
        EFieldEntry<double> fe(e.pointer,query.Distance(e.box),
        maxDistance(query,e.box),fe.level+1,0,0);
        array2.push_back(fe);
      }
    }
    array1.clear();
    sort(array2.begin(),array2.end(),CmpEFE<double>);
    double dist = 0;
    double num = 0;
    for(unsigned int i = 0;i < array2.size();i++){
      CcInt* id = new CcInt(true,array2[i].nodeid);
      BTreeIterator* iter = mqk->btreecov->ExactMatch(id);
      assert(iter->Next() != false);
      Tuple* tuple = mqk->cov->GetTuple(iter->GetId(), false);
      int val = ((CcInt*)tuple->GetAttribute(1))->GetIntval();
      delete id;
      tuple->DeleteIfAllowed();
      if(num + val > mqk->k){
        dist = array2[i].maxdist;
        break;
      }
      num += val;
    }
    for(unsigned int i = 0;i < array2.size();i++){
      if(array2[i].mindist > dist)
        break;
      array1.push_back(array2[i]);
    }
  }
  //process leaf node
  sort(candidate.begin(),candidate.end(),CmpEFE<double>);
  double dist = 0;
  double num = 0;

  for(unsigned int i = 0;i < candidate.size();i++){
      CcInt* id = new CcInt(true,candidate[i].nodeid);
      BTreeIterator* iter = mqk->btreecov->ExactMatch(id);
      assert(iter->Next() != false);
      Tuple* tuple = mqk->cov->GetTuple(iter->GetId(), false);
      int val = ((CcInt*)tuple->GetAttribute(1))->GetIntval();
      delete id;
      tuple->DeleteIfAllowed();
      if(num + val > mqk->k){
        dist = candidate[i].maxdist;
        break;
      }
      num += val;
  }
  for(unsigned int i = 0;i < candidate.size();i++){
    if(candidate[i].mindist > dist)
      break;
    datanode.push_back(candidate[i].nodeid);
  }

  //load data points into memory
  for(unsigned int i = 0;i < datanode.size();i++){
    adr = datanode[i];
    R_TreeNode<2,TupleId>* node = mqk->rtree->GetMyNode(
          adr,false,minentries,maxentries);
    assert(node->IsLeaf());
    for(int j = 0;j < node->EntryCount();j++){
      R_TreeLeafEntry<2,TupleId> e =
        (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
      Tuple* tuple = mqk->datapoints->GetTuple(e.info, false);
      Point* p = (Point*)tuple->GetAttribute(0);
      mqk->data.push_back(*p);
      tuple->DeleteIfAllowed();
    }
  }
//  cout<<"after R-tree filter data candidate size "<<mqk->data.size()<<endl;
}
struct Subleafnode{
  int nodeid;
  double mindist,maxdist;
  BBox<2> box;
  int quad;
  Subleafnode(){}
  Subleafnode(int id,double min,double max,BBox<2> b,int q)
  :nodeid(id),mindist(min),maxdist(max),box(b),quad(q){}
  Subleafnode& operator = (const Subleafnode& sln)
  {
    nodeid = sln.nodeid;
    mindist = sln.mindist;
    maxdist = sln.maxdist;
    box = sln.box;
    quad = sln.quad;
    return *this;
  }

  Subleafnode(const Subleafnode& sln)
  :nodeid(sln.nodeid),
   mindist(sln.mindist),
   maxdist(sln.maxdist),
   box(sln.box),
   quad(sln.quad)
  {

  }
};
bool CmpSLN(const Subleafnode& sln1,const Subleafnode& sln2)
{
  if(sln1.mindist < sln2.mindist)
    return true;
  if(sln1.maxdist < sln2.maxdist)
    return true;
  if(sln1.nodeid < sln2.nodeid)
    return true;
  return false;
}
bool CmpSLNNodeid(const Subleafnode& sln1,const Subleafnode& sln2)
{
  if(sln1.nodeid < sln2.nodeid)
    return true;
  return false;
}

void MqkPartitionNode(MQKnearest* mqk, vector<Subleafnode> candidate,
vector<Point>& ps,BBox<2> box) //ps--query objects, box -- query box
{
  for(unsigned int i = 0;i < candidate.size();i++){
      BBox<2> b(candidate[i].box);
      candidate[i].mindist = b.Distance(box);
      candidate[i].maxdist = maxDistance(b,box);
  }
  stable_sort(candidate.begin(),candidate.end(),CmpSLN);
  //set threshold distance value
  double dist = 0;
  double num = 0;
  for(unsigned int i = 0;i < candidate.size();i++){
    CcInt* id = new CcInt(true,candidate[i].nodeid);
    BTreeIterator* iter = mqk->btreeleafcov->ExactMatch(id);
    assert(iter->Next() != false);
    Tuple* tuple = mqk->leafcov->GetTuple(iter->GetId(), false);
    int val = ((CcInt*)tuple->GetAttribute(candidate[i].quad))->GetIntval();
    delete id;
    tuple->DeleteIfAllowed();
    if(num + val > mqk->k){
      dist = candidate[i].maxdist;
      break;
    }
    num += val;
  }
  vector<Subleafnode> candata;//real candidate
  for(unsigned int i = 0;i < candidate.size();i++){

    if(candidate[i].mindist > dist)
      break;
    candata.push_back(candidate[i]);
  }
  stable_sort(candata.begin(),candata.end(),CmpSLNNodeid);//sort by nodeid
//  cout<<"subleaf node "<<candata.size()<<endl;
  vector<Point> datapoints;
  R_TreeNode<2,TupleId>* node;
  SmiRecordId adr;
  int minentries = mqk->rtree->MinEntries(0);
  int maxentries = mqk->rtree->MaxEntries(0);
  for(unsigned int i = 0;i < candata.size();){
    int nodeid = candata[i].nodeid;
    vector<BBox<2> > range;
    while(candata[i].nodeid == nodeid){
      range.push_back(candata[i].box);
      i++;
    }
    adr = nodeid;
    node = mqk->rtree->GetMyNode(adr,false,minentries,maxentries);
    for(int j = 0;j < node->EntryCount();j++){//open leaf node
       R_TreeLeafEntry<2,TupleId> e =
        (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
      Tuple* tuple = mqk->datapoints->GetTuple(e.info, false);
      Point* p = (Point*)tuple->GetAttribute(0);
      double x = p->GetX();
      double y = p->GetY();
      for(unsigned int k = 0;k < range.size();k++){
        if(range[k].MinD(0) <= x && range[k].MinD(1) <= y &&
           x < range[k].MaxD(0) && y < range[k].MaxD(1)){
          datapoints.push_back(*p);
          break;
        }
      }
      tuple->DeleteIfAllowed();
    }
  }
//  cout<<"query size "<<ps.size()<<endl;
//  cout<<"datapoints size "<<datapoints.size()<<endl;

  double x = 0;
  double y = 0;
  for(unsigned int i = 0;i < ps.size();i++){
    x += ps[i].GetX();
    y += ps[i].GetY();
  }

  Point* seedp = new Point(true, // choose centroid point as seed point
  x/ps.size(),y/ps.size());
  vector<PointNeighbor> distance; //data points ordered
  for(unsigned int j = 0;j < datapoints.size();j++){
      PointNeighbor* pd = new PointNeighbor(*seedp,datapoints[j]);
      distance.push_back(*pd);
      delete pd;
  }
  sort(distance.begin(),distance.end(),CmpPointNeighbor);
   vector<PointNeighbor> tempstore;
  for(unsigned int i = 0;i < ps.size();i++){
    Point* p1 = &ps[i];
    double threshold = 0;
    for(unsigned int i = 0; i < mqk->k; i++)
     if(p1->Distance(distance[i].p) > threshold)
        threshold = p1->Distance(distance[i].p);

    for(unsigned int j = 0;j < distance.size();j++){
      double dist = distance[j].dist-p1->Distance(*seedp);
      if(dist > threshold)
        break;
      PointNeighbor* pn = new PointNeighbor(*p1,distance[j].p);
      tempstore.push_back(*pn);
      delete pn;
    }
    sort(tempstore.begin(),tempstore.end(),CmpPointNeighbor);
    for(unsigned int i = 0;i < mqk->k;i++)
      mqk->results.push_back(tempstore[i]);
    tempstore.clear();
  }

  delete seedp;
}
void MqkPartition(MQKnearest* mqk, vector<TupleId>& datanode,BBox<2>& box)
{
   double minx = box.MinD(0);
   double miny = box.MinD(1);
   double maxx = box.MaxD(0);
   double maxy = box.MaxD(1);
   vector<Point> ps1;
   vector<Point> ps2;
   vector<Point> ps3;
   vector<Point> ps4;
  //partition query points into four quadrants
   for(unsigned int i = 0;i < mqk->block.size();i++){
    double x = mqk->block[i].GetX();
    double y = mqk->block[i].GetY();
    if(minx <= x && x< (minx+(maxx-minx)/2)){
          if(miny <= y && y < (miny+(maxy-miny)/2))
            ps3.push_back(mqk->block[i]);
          else
            ps2.push_back(mqk->block[i]);
        }else{
          if(miny <= y && y < (miny+(maxy-miny)/2))
            ps4.push_back(mqk->block[i]);
          else
            ps1.push_back(mqk->block[i]);
        }
   }
 assert(ps1.size() + ps2.size() + ps3.size() + ps4.size() == mqk->block.size());
 double min[2],max[2];
  //quadrant 3
  min[0] = minx;
  min[1] = miny;
  max[0] = minx + (maxx-minx)/2;
  max[1] = miny + (maxy-miny)/2;
  BBox<2> b3(true,min,max);
   //4 quadrant
  min[0] = minx + (maxx-minx)/2;
  min[1] = miny;
  max[0] = maxx;
  max[1] = miny + (maxy-miny)/2;
  BBox<2> b4(true,min,max);
 //1 quadrant
  min[0] = minx + (maxx-minx)/2;
  min[1] = miny + (maxy-miny)/2;
  max[0] = maxx;
  max[1] = maxy;
  BBox<2> b1(true,min,max);
  //2 quadrant
  min[0] = minx;
  min[1] = miny + (maxy-miny)/2;
  max[0] = minx + (maxx-minx)/2;
  max[1] = maxy;
  BBox<2> b2(true,min,max);
  //
  vector<Subleafnode> candidate;//all subleaf node
  R_TreeNode<2,TupleId>* node;
  SmiRecordId adr;
  int minentries = mqk->rtree->MinEntries(0);
  int maxentries = mqk->rtree->MaxEntries(0);

  for(unsigned int i = 0;i < datanode.size();i++){
      adr = datanode[i];
      node = mqk->rtree->GetMyNode(adr,false,minentries,maxentries);
      assert(node->IsLeaf());
      BBox<2> b = node->BoundingBox();
      double minx = b.MinD(0);
      double miny = b.MinD(1);
      double maxx = b.MaxD(0);
      double maxy = b.MaxD(1);
      //3 quadrant
      min[0] = minx;
      min[1] = miny;
      max[0] = minx + (maxx-minx)/2;
      max[1] = miny + (maxy-miny)/2;
      BBox<2> b3(true,min,max);
      Subleafnode sln3(datanode[i],0,0,b3,3);
      candidate.push_back(sln3);
      //4 quadrant
      min[0] = minx + (maxx-minx)/2;
      min[1] = miny;
      max[0] = maxx;
      max[1] = miny + (maxy-miny)/2;
      BBox<2> b4(true,min,max);
      Subleafnode sln4(datanode[i],0,0,b4,4);
      candidate.push_back(sln4);
      //1 quadrant
      min[0] = minx + (maxx-minx)/2;
      min[1] = miny + (maxy-miny)/2;
      max[0] = maxx;
      max[1] = maxy;
      BBox<2> b1(true,min,max);
      Subleafnode sln1(datanode[i],0,0,b1,1);
      candidate.push_back(sln1);
      //2 quadrant
      min[0] = minx;
      min[1] = miny + (maxy-miny)/2;
      max[0] = minx + (maxx-minx)/2;
      max[1] = maxy;
      BBox<2> b2(true,min,max);
      Subleafnode sln2(datanode[i],0,0,b2,2);
      candidate.push_back(sln2);
  }
//  cout<<"total subleaf node "<<candidate.size()<<endl;
  MqkPartitionNode(mqk,candidate,ps1,b1);
  MqkPartitionNode(mqk,candidate,ps2,b2);
  MqkPartitionNode(mqk,candidate,ps3,b3);
  MqkPartitionNode(mqk,candidate,ps4,b4);
  mqk->block.clear();
  mqk->iter = mqk->results.begin();
}

//for each point in querypoints, find its k closest point, put into vectors
void Mqknearest(MQKnearest* mqk)
{
  double x = 0;
  double y = 0;
  //load query points into memory
  const int blocksize = 128;
  int start = 0;
  double min[2];
  double max[2];
  Tuple* tuple = mqk->querypoints->GetTuple(1, false);
  Point* p = (Point*)tuple->GetAttribute(0);
  min[0] = max[0] = p->GetX();
  min[1] = max[1] = p->GetY();
  tuple->DeleteIfAllowed();
  for(;start < blocksize &&
      (start+mqk->index) <= mqk->querypoints->GetNoTuples(); start++){
    Tuple* tuple = mqk->querypoints->GetTuple(start+mqk->index, false);
    Point* p = (Point*)tuple->GetAttribute(0);
    double xx = p->GetX();
    double yy = p->GetY();
    x += xx;
    y += yy;
    if(xx < min[0])
      min[0] = xx;
    if(xx > max[0])
      max[0] = xx;
    if(yy < min[1])
      min[1] = yy;
    if(yy > max[1])
      max[1] = yy;
    mqk->block.push_back(*p);
    tuple->DeleteIfAllowed();
  }
  if((start+mqk->index) > mqk->querypoints->GetNoTuples()){
    mqk->nextblock = false;
  }else{
      mqk->index += start;
  }
  BBox<2> box(true,min,max);

  //process with R-tree rel on data points, return a set of leaf nodeid
  vector<TupleId> datanode;
  Mqkfilter(mqk,datanode,box);
  MqkPartition(mqk,datanode,box);//partition method according to quadrant

  //following does not partition
  vector<PointNeighbor> distance; //data points ordered

//brute force algorithm
//  for(int i = 1;i <= mqk->querypoints->GetNoTuples();i++){
//    Tuple* tuple1 = mqk->querypoints->GetTuple(i);
//    Point* p1 = (Point*)tuple1->GetAttribute(0);
//    for(int j = 1;j <= mqk->datapoints->GetNoTuples();j++){
//      Tuple* tuple2 = mqk->datapoints->GetTuple(j);
//      Point* p2 = (Point*)tuple2->GetAttribute(0);
//     PointNeighbor* pd = new PointNeighbor(*p1,*p2);
//      distance.push_back(*pd);
//      delete pd;
//    }
//    sort(distance.begin(),distance.end(),CmpPointNeighbor);
//    for(unsigned int i = 0;i < mqk->k;i++){
//      mqk->results.push_back(mqk->distance[i]);
//    }
//    distance.clear();
//  }
//  mqk->iter = mqk->results.begin();

//random seed point
//  srand(time(0));
//  int seed = 1 + rand()%(mqk->querypoints->GetNoTuples()-1);
//  tuple = mqk->datapoints->GetTuple(seed);
//  Point* seedp = (Point*)tuple->GetAttribute(0);
//  tuple->DeleteIfAllowed();


//  Point* seedp = new Point(true, // choose centroid point as seed point
//  x/mqk->querypoints->GetNoTuples(),y/mqk->querypoints->GetNoTuples());
//  vector<PointNeighbor> tempstore;
//  for(int j = 1;j <= mqk->datapoints->GetNoTuples();j++){
//      Tuple* tuple = mqk->datapoints->GetTuple(j);
//      Point* p2 = (Point*)tuple->GetAttribute(0);
//      PointNeighbor* pd = new PointNeighbor(*seedp,*p2);
//      distance.push_back(*pd);
//      delete pd;
//      tuple->DeleteIfAllowed();
//  }
//  sort(distance.begin(),distance.end(),CmpPointNeighbor);

//  for(unsigned int i = 0;i < mqk->block.size();i++){
//    Point* p1 = &mqk->block[i];
//    double threshold = 0;
//    for(unsigned int i = 0; i < mqk->k; i++)
//     if(p1->Distance(distance[i].p) > threshold)
//        threshold = p1->Distance(distance[i].p);
//    for(unsigned int j = 0;j < distance.size();j++){
//      double dist = distance[j].dist-p1->Distance(*seedp);
//      if(dist > threshold)
//        break;
//      PointNeighbor* pn = new PointNeighbor(*p1,distance[j].p);
//      tempstore.push_back(*pn);
//      delete pn;
//    }
//    sort(tempstore.begin(),tempstore.end(),CmpPointNeighbor);
//    for(unsigned int i = 0;i < mqk->k;i++)
//      mqk->results.push_back(tempstore[i]);
//    tempstore.clear();
//  }
//  mqk->block.clear();
//  mqk->iter = mqk->results.begin();
//  delete seedp;


}

const string covleafnodeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> datarel(tuple ((x1 t1)...(xn tn))) "
      " x rtree(tuple((x1 t1)...(xn tn)))"
      " x covrel(tuple((x1 t1)))->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>covleafnode(_ ,_,_ )</text--->"
      "<text>The operator returns a stream of tuples "
      "recording the number of units in leafnode of R-tree"
      "according to different quadrants"
      "</text--->"
      "<text>not finished yet;</text--->"
      ") )";
ListExpr covleafnodeTypeMap(ListExpr args){
  if(nl->ListLength(args) != 3){
      ErrorReporter::ReportError("3 parameters expected");
      return nl->TypeError();
  }
  ListExpr rtree2 = nl->Second(args);
  string rtreedescription2;
  nl->WriteToString(rtreedescription2,rtree2);
  ListExpr rtsymbol2 = nl->First(rtree2);

  if(nl->IsAtom(rtsymbol2) &&
    nl->AtomType(rtsymbol2) == SymbolType &&
    nl->SymbolValue(rtsymbol2) == RTree2TID::BasicType())

  return
        nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Nodeid"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//1 quadrant
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//2 quadrant
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//3 quadrant
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//4 quadrant
                        nl->SymbolAtom(CcInt::BasicType())
                    ))));
  ErrorReporter::ReportError("rtree expected");
  return nl->TypeError();

}
struct Covleafnode{
  Relation* data;
  R_Tree<2,TupleId>* rtree;
  Relation* cov;
  TupleType* resulttype;
  int covtid;
  Covleafnode(){}
};
/*
  Calculate the coverage number for each leafnode according to different
  quadrants

*/
int covleafnodeFun(Word* args, Word& result, int message,
              Word& local, Supplier s){
  Covleafnode* localInfo;
  switch(message){
     case OPEN: {
     localInfo = new Covleafnode();
     localInfo->data = (Relation*)args[0].addr;
     localInfo->rtree = (R_Tree<2,TupleId>*)args[1].addr;
     localInfo->cov = (Relation*)args[2].addr;
     localInfo->resulttype = new TupleType(nl->Second(GetTupleResultType(s)));
     localInfo->covtid = 1;
     local = SetWord(localInfo);
     return 0;
   }
   case REQUEST: {
      localInfo = (Covleafnode*)local.addr;
      R_TreeNode<2,TupleId>* node = NULL;
      int nodeid = 0;
      while(localInfo->covtid <= localInfo->cov->GetNoTuples()){
        Tuple* tuple = localInfo->cov->GetTuple(localInfo->covtid, false);
        assert(tuple != NULL);
        localInfo->covtid++;
        nodeid = ((CcInt*)tuple->GetAttribute(0))->GetIntval();
        tuple->DeleteIfAllowed();
        SmiRecordId adr = nodeid;
        node = localInfo->rtree->GetMyNode(
         adr,false,
        localInfo->rtree->MinEntries(0),localInfo->rtree->MaxEntries(0)
        );
        if(node->IsLeaf())
          break;

      }
      if(localInfo->covtid > localInfo->cov->GetNoTuples())
        return CANCEL;
      assert(node != NULL);
      BBox<2> box = node->BoundingBox();
      double minx = box.MinD(0);
      double miny = box.MinD(1);
      double maxx = box.MaxD(0);
      double maxy = box.MaxD(1);
      int quad1,quad2,quad3,quad4;
      quad1 = quad2 = quad3 = quad4 = 0;
      for(int i = 0;i < node->EntryCount();i++){
        R_TreeLeafEntry<2,TupleId> e = (R_TreeLeafEntry<2,TupleId>&)(*node)[i];
        TupleId info = e.info;
        Tuple* tuple = localInfo->data->GetTuple(info, false);
        Point* p = (Point*)tuple->GetAttribute(0);
        tuple->DeleteIfAllowed();
        double x = p->GetX();
        double y = p->GetY();
        if(minx <= x && x< (minx+(maxx-minx)/2)){
          if(miny <= y && y < (miny+(maxy-miny)/2))
            quad3++;
          else
            quad2++;
        }else{
          if(miny <= y && y < (miny+(maxy-miny)/2))
            quad4++;
          else
            quad1++;
        }
      }
      assert(quad1+quad2+quad3+quad4 == node->EntryCount());
      //coverage number for each quadrant
      Tuple* tuple = new Tuple(localInfo->resulttype);
      tuple->PutAttribute(0,new CcInt(true,nodeid));
      tuple->PutAttribute(1,new CcInt(true,quad1));
      tuple->PutAttribute(2,new CcInt(true,quad2));
      tuple->PutAttribute(3,new CcInt(true,quad3));
      tuple->PutAttribute(4,new CcInt(true,quad4));
      result.setAddr(tuple);
      return YIELD;

   }
   case CLOSE : {
      localInfo = (Covleafnode*)local.addr;
      localInfo->resulttype->DeleteIfAllowed();
      delete localInfo;
      return 0;
   }
 }
 return 0;
}
Operator covleafnode (
         "covleafnode",        // name
         covleafnodeSpec,      // specification
         covleafnodeFun,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         covleafnodeTypeMap    // type mapping
);

/*
Description message for knearest operators
Chinese algorithm and Greece algorithm

*/
const string GreeceknearestfilterSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn)))"
      " x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ greeceknearest [_, _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which are the k-nearest tupels to the given mpoint. "
      "The operator do not separate tupels if necessary. The "
      "result may have more than the k-nearest tupels. It is a "
      "filter operator for the knearest operator, if there are a great "
      "many input tupels. "
      "The operator expects an r-tree built on moving objects</text--->"
      "<text>query UnitTrains_UTrip UnitTrains "
      " greeceknearest [UTrip,train1, 5] count;</text--->"
      ") )";
const string ChinaknearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>tbtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ hcknnknearest [_, _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which are the k-nearest tupels to the given mpoint. "
      "The operator do not separate tupels if necessary. The "
      "result may have more than the k-nearest tupels. It is a "
      "filter operator for the knearest operator, if there are a great "
      "many input tupels. "
      "The operator expects a tb-tree built on moving objects </text--->"
      "<text>query UnitTrains_UTrip_tbtree UnitTrains chinaknearest"
      "[UTrip,train1, 5] count;</text--->"
      ") )";

/*
the same as UReal, but not store a,b,c the distance function

*/

struct myureal{
  double a,b,c;
  myureal():a(0.0),b(0.0),c(0.0){}
  myureal(double x,double y,double z):a(x),b(y),c(z){}
  myureal(const myureal& u):a(u.a),b(u.b),c(u.c){}
  myureal& operator=(const myureal& u)
  {
    a = u.a;
    b = u.b;
    c = u.c;
    return *this;
  }
};


/*
the same as UPoint, but not store p0,p1

*/
struct myupoint{
  Point p0,p1;
  myupoint():p0(true,0,0),p1(true,0,0){}
  myupoint(Point& a,Point& b):p0(a),p1(b){}
  myupoint(const myupoint& u):p0(u.p0),p1(u.p1){}
  myupoint& operator=(const myupoint& u)
  {
    p0 = u.p0;
    p1 = u.p1;
    return *this;
  }
};

/*
element in Nearestlist, store nodeid, tid, mindist,maxdist, time interval

*/

struct hpelem{
  myureal movdist;
//  TupleId tid;
  long tid;
  myupoint dataup;
  double mind,maxd;
  long nodeid;
  hpelem* next;
  double nodets,nodete; //absolute value
  bool operator<(const hpelem& e)const
  {
    if(mind < e.mind) return false;
    return true;

  }

  hpelem(const hpelem& le)
  :movdist(le.movdist),tid(le.tid),dataup(le.dataup),
   mind(le.mind),maxd(le.maxd),nodeid(le.nodeid),
   next(le.next), nodets(le.nodets),nodete(le.nodete)
   {
      assert(nodets<=nodete);
   }


  hpelem(TupleId id1,double d1,double d2,long id2)
  :tid(id1),mind(d1),maxd(d2),nodeid(id2),next(0), 
   nodets(0),nodete(0){
  }

  inline hpelem& operator=(const hpelem& le);
  inline void AssignURUP(UReal* movdist,UPoint* dataup);
  inline void MyTemporalFunction(double& t,Point& result);
  inline void URealMin(double& start);
  inline void URealMax(double& start);
  inline void URealTranslate(double& start);
};

inline hpelem& hpelem::operator=(const hpelem& le)
{
    tid = le.tid;
    nodeid = le.nodeid;
    mind = le.mind;
    maxd = le.maxd;
    movdist = le.movdist;
    dataup = le.dataup;
    nodets = le.nodets;
    nodete = le.nodete;
    next = NULL;
    return *this;
}

inline void hpelem::AssignURUP(UReal* movdist,UPoint* dataup)
{
  this->movdist.a = movdist->a;
  this->movdist.b = movdist->b;
  this->movdist.c = movdist->c;
  this->dataup.p0 = dataup->p0;
  this->dataup.p1 = dataup->p1;
}

/*
The same function as ureal TemporalFunction

*/
inline void hpelem::MyTemporalFunction(double& t,Point& result)
{
  if(t == this->nodets){
    result = this->dataup.p0;
    result.SetDefined(true);
    return;
  }
  if(t == this->nodete){
    result = this->dataup.p1;
    result.SetDefined(true);
    return;
  }
  Point p0 = this->dataup.p0;
  Point p1 = this->dataup.p1;
  double t0 = this->nodets;
  double t1 = this->nodete;
  double x = (p1.GetX()-p0.GetX())*((t - t0)/(t1-t0)) + p0.GetX();
  double y = (p1.GetY()-p0.GetY())*((t - t0)/(t1-t0)) + p0.GetY();
  result.Set(x,y);
  result.SetDefined(true);

}

/*
the same function as Min in UReal

*/
inline void hpelem::URealMin(double& start)
{
    double a = this->movdist.a;
    double b = this->movdist.b;
    double c = this->movdist.c;

    double t_start = this->nodets - start;
    double t_end = this->nodete - start;
    double v1 = sqrt(a*t_start*t_start + b*t_start + c);
    double v2 = sqrt(a*t_end*t_end + b*t_end + c);
    double min = v1;
    if(v2 < min)
      min = v2;
    if(AlmostEqual(a,0.0))
      this->mind = min;

    double asymx = (-1.0*b)/(2*a);
    double v3 = min;
    if(t_start < asymx && asymx < t_end){
      v3 = sqrt(a*asymx*asymx+b*asymx+c);
      if(v3 < min)
        min = v3;
    }

    this->mind = min;
}

/*
the same function as Max in UReal

*/

inline void hpelem::URealMax(double& start)
{
    double a = this->movdist.a;
    double b = this->movdist.b;
    double c = this->movdist.c;
    double t_start = this->nodets - start;
    double t_end = this->nodete - start;

    double v1 = sqrt(a*t_start*t_start + b*t_start + c);
    double v2 = sqrt(a*t_end*t_end + b*t_end + c);
    double max = v1;
    if(v2 > max)
      max = v2;
    if(AlmostEqual(a,0.0))
      this->maxd = max;

    double asymx = (-1.0*b)/(2*a);
    double v3 = 0;
    if(t_start < asymx && asymx < t_end){ //three values
      v3 = sqrt(a*asymx*asymx+b*asymx+c);
      if(v3 > max)
        max = v3;
    }
    this->maxd = max;
}

/*
translate the start time of parabolas

*/
inline void hpelem::URealTranslate(double& start)
{
  double b,c;
  double ts = this->nodets - start;
  b = 2*this->movdist.a*ts + this->movdist.b;
  c = this->movdist.c + this->movdist.b*ts + this->movdist.a*ts*ts;
  this->movdist.b = b;
  this->movdist.c = c;
}


/*
Nearestlist structure, the top structure, it has k list structure

*/

struct Nearestlist{
  double mind;
  double maxd;
  hpelem* head;
  double startTime,endTime;
  Nearestlist(double min,double max,hpelem* p,double s,double e)
  :mind(min),maxd(max),head(p),startTime(s),endTime(e){}
  Nearestlist(const Nearestlist& nl)
  :mind(nl.mind),maxd(nl.maxd),head(nl.head),
  startTime(nl.startTime),endTime(nl.endTime){}
};

/*
record prunedist, actuall the max value in nearestlist[k], and time

*/

struct  Prunedist{
  double dist;
  double ts,te;
  bool define;
  Prunedist():dist(0),ts(0),te(0),define(false){}
};

/*
main structur for chinese and greece knn algorithm

*/

struct TBKnearestLocalInfo
{
  unsigned int k;
  unsigned int attrpos;
  bool scanFlag;
  Relation* relation;
  TBTree* tbtree;
  R_Tree<3,TupleId>* rtree;
  double startTime,endTime;
  Line* mptraj;
  CIC<double>* ci;//detect a time interval is fully covered
  bool iscovered; //used to detect time interval in Nearest_list
  vector<Nearestlist> nlist;//main structure for pruning and record results
  Prunedist prunedist;
  BBox<2> mpbox;
  priority_queue<hpelem> hp;
  vector<hpelem> result;
  unsigned int counter;
  CoverInterval<double>* cic;


  TBKnearestLocalInfo(unsigned int nn):k(nn)
  {
      ci = NULL;
      cic = NULL;
      mptraj = NULL;
      iscovered = false;
  }
  ~TBKnearestLocalInfo()
  {
    if( ci != NULL)
      delete ci;
    if( cic != NULL)
      delete cic;
    if(mptraj != NULL)
      delete mptraj;
  }
  /*public function*/
  void UpdateInfoInNL(hpelem* elem,int i);
  void ReportResult();
  void ParabolasTM(hpelem& elem,vector<hpelem>& nextupdatelist,int i,
                  hpelem*& head,hpelem*& cur,double& intersect,
                  double& elemstart,double& curstart);
  void ParabolasMT(hpelem& elem,vector<hpelem>& nextupdatelist,int i,
                  hpelem*& head,hpelem*& cur,double& intersect,
                  double& elemstart,double& curstart);
  void Parabolas(hpelem& elem,vector<hpelem>& nextupdatelist,int i,
                  hpelem*& head,hpelem*& cur,double& elemstart,
                  double& curstart);
  void UpdateNearest(hpelem& elem,vector<hpelem>& nextupdatelist,int i);
/*Greece algorithm function*/
  bool CheckPrune(hpelem& elem);
  void GreeceknnFunDF(MPoint* mp,int level,hpelem& elem);
  void GreeceknnFunBF(MPoint* mp,int level,hpelem& elem);
  void UpdatekNearestG(hpelem& elem);
  void GreeceknearestInitialize(MPoint* mp);
  void CheckCoveredG(hpelem& elem);
/*Chinese algorithm function*/
  void ChinaknnInitialize(MPoint* mp);
  void ChinaknnFun(MPoint* mp);
  void UpdatekNearest(hpelem& elem);
  void InitializePrunedist();
  void CheckCovered(hpelem& elem);
};
/*
Initialization, knearestlist prunedist

*/
void TBKnearestLocalInfo::ChinaknnInitialize(MPoint* mp)
{
  //Initialization NearestList and prunedist

  for(unsigned int i = 0;i < k;i++){
    double mind = numeric_limits<double>::max();
    double maxd = numeric_limits<double>::min();
    double st = startTime;
    double et = startTime;
    Nearestlist nl(mind,maxd,new hpelem(-1,0,0,-1),st,et);
    nlist.push_back(nl);
  }

  prunedist.dist = numeric_limits<double>::max();
  prunedist.define = false;


  SmiRecordId adr = tbtree->getRootId();
  tbtree::BasicNode<3>* root = tbtree->getNode(adr);
  double t1((double)root->getBox().MinD(2));
  double t2((double)root->getBox().MaxD(2));
  if(!(t1 >= endTime || t2 <= startTime)){
    Line* line = new Line(0);
    mp->Trajectory(*line);
    mptraj = new Line(*line);
    delete line;
    tbtree::InnerNode<3,InnerInfo>* innernode =
            dynamic_cast<InnerNode<3,InnerInfo>*>(root);
    //insert all the entries of the root into hp
    for(unsigned int i = 0;i < innernode->entryCount();i++){
        const Entry<3,InnerInfo>* entry = innernode->getEntry(i);
        double tt1((double)entry->getBox().MinD(2));
        double tt2((double)entry->getBox().MaxD(2));
        if(!(tt1 >= endTime || tt2 <= startTime)){
          BBox<2> entrybox = makexyBox(entry->getBox());//entry box
          double mindist = mptraj->Distance(entrybox);
          double maxdist = numeric_limits<double>::max();
          hpelem le(-1,mindist,maxdist,entry->getInfo().getPointer());
          le.nodets = tt1;
          le.nodete = tt2;
          hp.push(le);
        }
    }
  }
}
/*
Interpolate for between data entry and query entry

*/

void CreateUPoint_ne(const UPoint* up,UPoint*& ne,UPoint* data)
{

  Instant start;
  Instant end;
  Point p0,p1;

  if(data->timeInterval.start > up->timeInterval.start){
    start = data->timeInterval.start;
    p0 = data->p0;
  }
  else{
    start = up->timeInterval.start;
    data->TemporalFunction(start,p0,true);
  }
  if(data->timeInterval.end < up->timeInterval.end){
    end = data->timeInterval.end;
    p1 = data->p1;
  }
  else{
    end = up->timeInterval.end;
    data->TemporalFunction(end,p1,true);
  }
  ne->timeInterval.start = start;
  ne->timeInterval.end = end;
  ne->p0 = p0;
  ne->p1 = p1;

}
/*
Interpolate for entry in MQ, split entries

*/


void CreateUPoint_nqe(const UPoint* up,UPoint*& nqe,UPoint* data)
{
  Instant start;
  Instant end;
  Point p0,p1;

  if(data->timeInterval.start > up->timeInterval.start){
    start = data->timeInterval.start;
    up->TemporalFunction(start,p0,true);
  }
  else{
    start = up->timeInterval.start;
    p0 = up->p0;
  }
  if(data->timeInterval.end < up->timeInterval.end){
    end = data->timeInterval.end;
    up->TemporalFunction(end,p1,true);
  }
  else{
    end = up->timeInterval.end;
    p1 = up->p1;
  }
  nqe->timeInterval.start = start;
  nqe->timeInterval.end = end;
  nqe->p0 = p0;
  nqe->p1 = p1;

}


/*
Update information in each NearestList, mind,maxd, startTime,endTime
after each new elem is inserted

*/
void TBKnearestLocalInfo::UpdateInfoInNL(hpelem* elem,int i)
{
  if(elem->mind < nlist[i].mind)
    nlist[i].mind = elem->mind;
  if(elem->maxd > nlist[i].maxd)
    nlist[i].maxd = elem->maxd;
  if(elem->nodets < nlist[i].startTime)
    nlist[i].startTime = elem->nodets;
  if(elem->nodete > nlist[i].endTime)
    nlist[i].endTime = elem->nodete;

  //update prune_dist here
  if(elem->nodets > prunedist.te ||
     elem->nodete < prunedist.ts){   //time interval disjoint
      if(elem->maxd > prunedist.dist){
        prunedist.dist = elem->maxd;
        prunedist.ts = elem->nodets;
        prunedist.te = elem->nodete;
    }
  }else{
        hpelem* head = nlist[k-1].head;
        hpelem* cur = head->next;
        hpelem* bigelem;
        if(cur != NULL){
          double max = cur->maxd;
          bigelem = cur;
          while(cur != NULL){
            if(cur->maxd > max){
              max = cur->maxd;
              bigelem = cur;
            }
            cur = cur->next;
          }
          prunedist.dist = max;
          prunedist.ts = bigelem->nodets;
          prunedist.te = bigelem->nodete;
        }
    }
}

/*
interpolation parabolas
entry unit (in list) is first smaller than data (new elem), and then new
elem is smaller than entry (the one already stored in Nearestlist)
T is smaller first

*/


void TBKnearestLocalInfo::ParabolasTM(hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& intersect,double& elemstart,double& curstart)
{

    hpelem* newelem1 = new hpelem(*cur);
    newelem1->nodets = intersect;
    Point start;

    newelem1->MyTemporalFunction(intersect,start);
    newelem1->dataup.p0 = start;
    newelem1->next= NULL;
    newelem1->URealMin(curstart);
    newelem1->URealMax(curstart);
    newelem1->URealTranslate(curstart);

    cur->nodete = intersect;
    Point end;

    cur->MyTemporalFunction(intersect,end);
    cur->dataup.p1 = end;
    cur->URealMin(curstart);
    cur->URealMax(curstart);

    hpelem* newelem2 = new hpelem(elem);
    newelem2->nodete = intersect;

    newelem2->MyTemporalFunction(intersect,end);
    newelem2->dataup.p1 = end;
    newelem2->URealMin(elemstart);
    newelem2->URealMax(elemstart);

    nextupdatelist.push_back(*newelem1);
    nextupdatelist.push_back(*newelem2);

    delete newelem1;
    delete newelem2;


    hpelem* newelem3 = new hpelem(elem);
    newelem3->nodets = intersect;

    newelem3->MyTemporalFunction(intersect,start);
    newelem3->dataup.p0 = start;
    newelem3->next = cur->next;


    cur->next = newelem3;
////////////////////////////////////
    head = cur;
    cur = newelem3;
////////////////////////////////
    newelem3->URealMin(elemstart);
    newelem3->URealMax(elemstart);
    newelem3->URealTranslate(elemstart);

    UpdateInfoInNL(cur,i);
    UpdateInfoInNL(newelem3,i);
}
/*
interpolation parabolas
entry unit (in list) is first larger than data (new elem), and then new
elem is larger than entry (the one already stored in Nearestlist)
M is smaller first

*/

void TBKnearestLocalInfo::ParabolasMT(hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& intersect,double& elemstart,double& curstart)
{

  Point end;
  hpelem* newelem1 = new hpelem(elem);

  newelem1->nodete = intersect;

  newelem1->MyTemporalFunction(intersect,end);
  newelem1->dataup.p1 = end;
  newelem1->next = NULL;
  newelem1->URealMin(elemstart);
  newelem1->URealMax(elemstart);

  hpelem* newelem2 = new hpelem(*cur);

  newelem2->nodete = intersect;

  newelem2->MyTemporalFunction(intersect,end);
  newelem2->dataup.p1 = end;
  newelem2->next= NULL;
  newelem2->URealMin(curstart);
  newelem2->URealMax(curstart);


  cur->nodets = intersect;
  Point start;

  cur->MyTemporalFunction(intersect,start);
  cur->dataup.p0 = start;
  cur->URealMin(curstart);
  cur->URealMax(curstart);
  cur->URealTranslate(curstart);



  head->next = newelem1;
  newelem1->next = cur;
///////////////////////
  head = newelem1;
////////////////////////

  UpdateInfoInNL(newelem1,i);
  UpdateInfoInNL(cur,i);


  hpelem* newelem3 = new hpelem(elem);

  newelem3->nodets = intersect;

  newelem3->MyTemporalFunction(intersect,start);
  newelem3->dataup.p0 = start;
  newelem3->next = NULL;
  newelem3->URealMin(elemstart);
  newelem3->URealMax(elemstart);
  newelem3->URealTranslate(elemstart);


  nextupdatelist.push_back(*newelem2);
  nextupdatelist.push_back(*newelem3);

  delete newelem2;
  delete newelem3;
}

/*
Process different splitting cases of parabolas
restrict to the same time interval and check whether the moving distance
function has to be split or not

*/

void TBKnearestLocalInfo::Parabolas(hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& elemstart,double& curstart)
{
//  printf("%.12f %.12f\n",elem.nodets,elem.nodete);
//  printf("%.12f %.12f\n",cur->nodets,cur->nodete);
//  double factor = 0.000000001;
  double ma,mb,mc; //data
  double ta,tb,tc; //entry in list
  double ttelem = elemstart;
  double ttcur = curstart;

  //a1(t-t1)*(t-t1) + b1(t-t1) + c1
  //a2(t-t2)*(t-t2) + b2(t-t2) + c2

  ma = elem.movdist.a;
  mb = elem.movdist.b - 2*ma*ttelem;
  mc = elem.movdist.c + ma*ttelem*ttelem- mb*ttelem;
  ta = cur->movdist.a;
  tb = cur->movdist.b - 2*ta*ttcur;
  tc = cur->movdist.c + ta*ttcur*ttcur - tb*ttcur;


  double start_m,start_t;
  if(AlmostEqual(ttelem, elem.nodets)){
//  if(fabs(ttelem- elem.nodets) < factor){
      start_m = sqrt(elem.movdist.c);
  }
  else{
    double delta_m = elem.nodets-elemstart;
    start_m =
        sqrt(ma*delta_m*delta_m + elem.movdist.b*delta_m + elem.movdist.c);
  }


  if(AlmostEqual(ttcur, cur->nodets)){
//  if(fabs(ttcur - cur->nodets) < factor){
    start_t = sqrt(cur->movdist.c);
  }
  else{

    double delta_t = cur->nodets-curstart;
    start_t =
    sqrt(ta*delta_t*delta_t + cur->movdist.b*delta_t + cur->movdist.c);
  }

  if(AlmostEqual(ma ,ta ) && AlmostEqual(mb ,tb) && AlmostEqual(mc ,tc)){
//  if(fabs(ma -ta ) < factor && fabs(mb-tb) < factor && fabs(mc -tc)<factor){

      nextupdatelist.push_back(elem);
      return;//for next
  }

    if(AlmostEqual(ma, ta) && AlmostEqual(mb ,tb)){
//    if(fabs(ma - ta) < factor && fabs(mb - tb)< factor ){
        if(start_t <= start_m){

          nextupdatelist.push_back(elem);
          return;
      }else{// entry is to be replaced by data

            hpelem* temppointer = cur->next;
            UpdateInfoInNL(&elem,i);
            cur->next = NULL;
            nextupdatelist.push_back(*cur);
            *cur = elem;
            cur->next = temppointer;
            return;
      }
  }

    if(AlmostEqual(ma ,ta)){
//    if(fabs(ma -ta) < factor){
      assert(mb != tb);
      double v1 = elem.nodets;
      double v2 = elem.nodete;

      double time_m,time_t; //start time for cur and elem
      time_m = ttelem;//real start time
      time_t = ttcur; //real start time
      double t = (tc-mc)/(mb-tb);

      if(v1 >= t || t >= v2){ //intersection point is no use
            if(start_t <= start_m){

              nextupdatelist.push_back(elem);
              return;//for next
          }else{ //entry is to be replaced by data

                hpelem* temppointer = cur->next;
                UpdateInfoInNL(&elem,i);
                cur->next = NULL;
                nextupdatelist.push_back(*cur);
                *cur = elem;
                cur->next = temppointer;
                return;
               }
      }else{ //needs to be interpolation
            double intersect = t;

            if(start_t <= start_m){
              ParabolasTM(elem,nextupdatelist,i,head,cur,
              intersect,elemstart,curstart);
              return;
          }else{ //
              ParabolasMT(elem,nextupdatelist,i,head,cur,
              intersect,elemstart,curstart);
              return;
          }
        }
  }

  //a is not equal to a'
  double mind_m = start_m;
  double mind_t = start_t;

  double time_m,time_t; //start time for cur and elem
  time_m = ttelem;//real start time
  time_t = ttcur; //real start time

  double delta_a = ma-ta;
  double delta_b = mb-tb;
  double delta_c = mc-tc;
  double delta = delta_b*delta_b - 4*delta_a*delta_c;

  if(fabs(delta) < 0.001){
      if(mind_t <= mind_m){

        nextupdatelist.push_back(elem);
        return;
      }else{ //entry is to be replaced by data

            hpelem* temppointer = cur->next;
            UpdateInfoInNL(&elem,i);
            cur->next = NULL;
            nextupdatelist.push_back(*cur);
            *cur = elem;
            cur->next = temppointer;

            return;
      }
  }

  if(delta < 0) { //no intersects
    if(mind_t <= mind_m){

        nextupdatelist.push_back(elem);
        return;
    }else{ //entry is to be replaced by data
        hpelem* temppointer = cur->next;

        UpdateInfoInNL(&elem,i);
        cur->next = NULL;
        nextupdatelist.push_back(*cur);
        *cur = elem;
        cur->next = temppointer;
        return;
    }
  }
  //delta > 0
  double x1 = (-delta_b-sqrt(delta))/(2*delta_a);
  double x2 = (-delta_b+sqrt(delta))/(2*delta_a);
  double intersect_t1,intersect_t2;

  if(x1 < x2){
      intersect_t1 = x1;
      intersect_t2 = x2;
  }else{
      intersect_t1 = x2;
      intersect_t2 = x1;
  }
  double intersect1 = intersect_t1;
  double intersect2 = intersect_t2;


    double elemts = elem.nodets;
    double elemte = elem.nodete;

  //intersection is no use

  if(intersect_t2 <= elemts ||
    intersect_t1 >= elemte ||
    (intersect_t1 <= elemts&&
     intersect_t2 >= elemte)){

    if(mind_t <= mind_m){//entry is smaller

      nextupdatelist.push_back(elem);
      return;
    }else{ //entry is to be replaced by data


        hpelem* temppointer = cur->next;
        UpdateInfoInNL(&elem,i);
        cur->next = NULL;
        nextupdatelist.push_back(*cur);
        *cur = elem;
        cur->next = temppointer;
        return;
     }
  }
  //one intersection point

    if(intersect_t1 > elemts &&
        intersect_t1 < elemte&&
        intersect_t2 > elemte){
        if(start_t <= start_m){
          ParabolasTM(elem,nextupdatelist,i,head,cur,
            intersect1,elemstart,curstart);
          return;
      }else{ //
          ParabolasMT(elem,nextupdatelist,i,head,cur,
            intersect1,elemstart,curstart);
          return;
      }
  }

    if(intersect_t1 < elemts &&
       intersect_t2 > elemts &&
       intersect_t2 < elemte){

       if(start_t <= start_m){
        ParabolasTM(elem,nextupdatelist,i,head,cur,
          intersect2,elemstart,curstart);
        return;
     }else{ //
        ParabolasMT(elem,nextupdatelist,i,head,cur,
          intersect2,elemstart,curstart);
        return;
     }
  }
  //two intersection points
  //six sub parts
  if(start_t <= start_m){

    hpelem* next = cur->next;
    Point start, end;
    Point p0,p1;

    hpelem* newhp1 = new hpelem(*cur);
    newhp1->nodete = intersect1;

    newhp1->MyTemporalFunction(intersect1,end);
    newhp1->dataup.p1 = end;
    newhp1->URealMin(curstart);
    newhp1->URealMax(curstart);


    hpelem* newhp2 = new hpelem(elem);
    newhp2->nodets = intersect1;
    newhp2->nodete = intersect2;

    newhp2->MyTemporalFunction(intersect1,p0);
    newhp2->MyTemporalFunction(intersect2,p1);
    newhp2->dataup.p0 = p0;
    newhp2->dataup.p1 = p1;
    newhp2->URealMin(elemstart);
    newhp2->URealMax(elemstart);
    newhp2->URealTranslate(elemstart);



    hpelem* newhp3 = new hpelem(*cur);
    newhp3->nodets = intersect2;

    newhp3->MyTemporalFunction(intersect2,start);
    newhp3->dataup.p0 = start;
    newhp3->URealMin(curstart);
    newhp3->URealMax(curstart);
    newhp3->URealTranslate(curstart);



    hpelem* newhp4 = new hpelem(elem);
    newhp4->nodete = intersect1;

    newhp4->MyTemporalFunction(intersect1,end);
    newhp4->dataup.p1 = end;
    newhp4->next = NULL;
    newhp4->URealMin(elemstart);
    newhp4->URealMax(elemstart);



    hpelem* newhp5 = new hpelem(*(cur));
    newhp5->nodets = intersect1;
    newhp5->nodete = intersect2;

    newhp5->MyTemporalFunction(intersect1,p0);
    newhp5->MyTemporalFunction(intersect2,p1);
    newhp5->dataup.p0 = p0;
    newhp5->dataup.p1 = p1;
    newhp5->next = NULL;
    newhp5->URealMin(curstart);
    newhp5->URealMax(curstart);
    newhp5->URealTranslate(curstart);



    hpelem* newhp6 = new hpelem(elem);
    newhp6->nodets = intersect2;

    newhp6->MyTemporalFunction(intersect2,start);
    newhp6->dataup.p0 = start;
    newhp6->next = NULL;
    newhp6->URealMin(elemstart);
    newhp6->URealMax(elemstart);
    newhp6->URealTranslate(elemstart);


    head->next = newhp1;
    newhp1->next = newhp2;
    newhp2->next = newhp3;
    newhp3->next = next;

    UpdateInfoInNL(newhp2,i);
    head = newhp2;
    cur = newhp3;


    nextupdatelist.push_back(*newhp4);
    nextupdatelist.push_back(*newhp5);
    nextupdatelist.push_back(*newhp6);

    return;
  }else{

    hpelem* next = cur->next;
    Point start, end;
    Point p0,p1;

    hpelem* newhp1 = new hpelem(elem);
    newhp1->nodete = intersect1;

    newhp1->MyTemporalFunction(intersect1,end);
    newhp1->dataup.p1 = end;
    newhp1->next = NULL;
    newhp1->URealMin(elemstart);
    newhp1->URealMax(elemstart);


    hpelem* newhp2 = new hpelem(*(cur));
    newhp2->nodets = intersect1;
    newhp2->nodete = intersect2;

    newhp2->MyTemporalFunction(intersect1,p0);
    newhp2->MyTemporalFunction(intersect2,p1);
    newhp2->dataup.p0 = p0;
    newhp2->dataup.p1 = p1;
    newhp2->next = NULL;
    newhp2->URealMin(curstart);
    newhp2->URealMax(curstart);
    newhp2->URealTranslate(curstart);


    hpelem* newhp3 = new hpelem(elem);
    newhp3->nodets = intersect2;

    newhp3->MyTemporalFunction(intersect2,start);
    newhp3->dataup.p0 = start;
    newhp3->next = NULL;
    newhp3->URealMin(elemstart);
    newhp3->URealMax(elemstart);
    newhp3->URealTranslate(elemstart);


    hpelem* newhp4 = new hpelem(*cur);
    newhp4->nodete = intersect1;

    newhp4->MyTemporalFunction(intersect1,end);
    newhp4->dataup.p1 = end;
    newhp4->URealMin(curstart);
    newhp4->URealMax(curstart);


    hpelem* newhp5 = new hpelem(elem);
    newhp5->nodets = intersect1;
    newhp5->nodete = intersect2;

    newhp5->MyTemporalFunction(intersect1,p0);
    newhp5->MyTemporalFunction(intersect2,p1);
    newhp5->dataup.p0 = p0;
    newhp5->dataup.p1 = p1;
    newhp5->URealMin(elemstart);
    newhp5->URealMax(elemstart);
    newhp5->URealTranslate(elemstart);


    hpelem* newhp6 = new hpelem(*cur);
    newhp6->nodets = intersect2;

    newhp6->MyTemporalFunction(intersect2,start);
    newhp6->dataup.p0 = start;
    newhp6->URealMin(curstart);
    newhp6->URealMax(curstart);
    newhp6->URealTranslate(curstart);


    head->next = newhp1;
    newhp1->next = newhp2;
    newhp2->next = newhp3;
    newhp3->next = next;

    UpdateInfoInNL(newhp2,i);
    head = newhp2;
    cur = newhp3;


    nextupdatelist.push_back(*newhp4);
    nextupdatelist.push_back(*newhp5);
    nextupdatelist.push_back(*newhp6);
    return;
  }

}
/*
Update k NearestList structure traverse the k nearest list

*/

void TBKnearestLocalInfo::UpdateNearest(hpelem& elem,
vector<hpelem>& nextupdatelist,int i)
{

  double factor = 0.000000001;
  hpelem* head = nlist[i].head;
  assert(head != NULL);
  hpelem* cur = head->next;

  if(cur == NULL){

    hpelem* newhp = new hpelem(elem);
    head->next = newhp;
    newhp->next = NULL;
    nlist[i].mind = elem.mind;
    nlist[i].maxd = elem.maxd;

    nlist[i].startTime = elem.nodets;
    nlist[i].endTime = elem.nodete;

    return;
  }else{
      //the first elem
        if(elem.nodete <= cur->nodets){
           hpelem* newhp = new hpelem(elem);
           head->next = newhp;
           newhp->next = cur;
           UpdateInfoInNL(newhp,i);
           return;// no update list
       }
      while(cur != NULL){ //big program

        if(cur->nodete <= elem.nodets){
          if(cur->next == NULL){//the last element
            hpelem* newhp = new hpelem(elem);
            cur->next = newhp;
            newhp->next = NULL;
            UpdateInfoInNL(newhp,i);
            return;// no update list
          }else{
            head = cur;
            cur = cur->next;
            continue;
          }
        }
        if(head->tid != -1 &&   //not the head element
          head->nodete <= elem.nodets && elem.nodete <= cur->nodets){
           hpelem* newhp = new hpelem(elem);
           head->next = newhp;
           newhp->next = cur;
           UpdateInfoInNL(newhp,i);
           return;//no updatelist
        }
         if(elem.nodete < cur->nodets ||elem.nodets > cur->nodete){
          head = cur;
          cur = cur->next;
          continue;
        }

        double mts = elem.nodets;
        double mte = elem.nodete;
        double tts = cur->nodets;
        double tte = cur->nodete;

        if(AlmostEqual(mts ,tts) && AlmostEqual(mte ,tte)){
//        if(fabs(mts -tts) < factor && fabs(mte -tte) < factor){
            mts = mts > tts ? mts:tts;
            mte = mte < tte ? mte:tte;
            if(mts > mte){
                head = cur;
                cur = cur->next;
                continue;
            }
            elem.nodets = cur->nodets = mts;
            elem.nodete = cur->nodete = mte;

            //magic thing   double not equal to double, but almost equal,split
/*if(elem.nodets < cur->nodets){
            cur->nodets = elem.nodets;
            tts = mts;
          }else{
            elem.nodets = cur->nodets;
            mts = tts;
          }
          if(elem.nodete > cur->nodete){
            cur->nodete = elem.nodete;
            tte = mte;
          }else{
            elem.nodete = cur->nodete;
            mte = tte;
          }*/

            elem.nodets = cur->nodets;
            elem.nodete = cur->nodete;

            mts = tts;

            Parabolas(elem,nextupdatelist,i,head,cur,mts,tts);
            return;
        }else{// needs to be interpolation

          //overlap

          double ts =  mts > tts ? mts:tts;
          double te =  mte < tte ? mte:tte;

          if(mts < ts){

            hpelem* newhp = new hpelem(elem);
            newhp->nodete = ts;

            Point end;
            newhp->MyTemporalFunction(ts,end);
            newhp->dataup.p1 = end;

            newhp->URealMin(mts);
            newhp->URealMax(mts);

            head->next = newhp;
            newhp->next = cur;
            head = newhp;

            UpdateInfoInNL(newhp,i);

            elem.nodets = ts;
            Point start;

            elem.MyTemporalFunction(ts,start);
            elem.dataup.p0 = start;
            elem.URealMin(mts);
            elem.URealMax(mts);

            elem.URealTranslate(mts);

          }

          if(tts < ts){

            hpelem* newhp = new hpelem(*cur);

            newhp->nodete = ts;
            Point end;

            newhp->MyTemporalFunction(ts,end);
            newhp->dataup.p1 = end;
            newhp->URealMin(tts);
            newhp->URealMax(tts);


            head->next = newhp;
            newhp->next = cur;
            head = newhp;

            cur->nodets = ts;
            Point start;

            cur->MyTemporalFunction(ts,start);
            cur->dataup.p0 = start;
            cur->URealMin(tts);
            cur->URealMax(tts);
            cur->URealTranslate(tts);


          }
          //up to now,  cur and elem start at the same time interval
          if(tte > te){ //M is finished here

            hpelem* newhp = new hpelem(*cur);

            newhp->nodets = te;
            Point start;

            newhp->MyTemporalFunction(te,start);
            newhp->dataup.p0 = start;
            newhp->URealMin(ts);
            newhp->URealMax(ts);
            newhp->URealTranslate(ts);


            newhp->next = cur->next;
            cur->next = newhp;

            Point end;
            cur->nodete = te;

            cur->MyTemporalFunction(te,end);
            cur->dataup.p1 = end;
            cur->URealMin(ts);
            cur->URealMax(ts);


            Parabolas(elem,nextupdatelist,i,head,cur,ts,ts);
            return;
          }else{ //M is longer

//            if(AlmostEqual(elem.nodete ,cur->nodete)){
            if(fabs(elem.nodete - cur->nodete) < factor){
              Parabolas(elem,nextupdatelist,i,head,cur,ts,ts);
              return;
            }else{
              hpelem* newhp = new hpelem(elem);

              newhp->nodets = te;
              Point start;
              newhp->MyTemporalFunction(te,start);
              newhp->dataup.p0 = start;
              newhp->URealMin(ts);
              newhp->URealMax(ts);
              newhp->URealTranslate(ts);


              elem.nodete = te;
              Point end;

              elem.MyTemporalFunction(te,end);
              elem.dataup.p1 = end;
              elem.URealMin(ts);
              elem.URealMax(ts);

              Parabolas(elem,nextupdatelist,i,head,cur,ts,ts);
              elem = *newhp;

              delete newhp;

              head = cur;
              cur = cur->next;
              continue;
            }
          }
        }
      } //end big while

   }
}


/*
Check whether a node or entry has to be inserted or not
traverse the k nearest list

*/

bool TBKnearestLocalInfo::CheckPrune(hpelem& elem)
{
    //linear traverse method
      hpelem* head = nlist[k-1].head;
      if(head->next == NULL)
        return false; //not prune
      hpelem* cur = head->next;
      double curts = cur->nodets;
      double curte = cur->nodete;

      if(curts > elem.nodets)
        return false;
      while(cur != NULL){
        curts = cur->nodets;
        curte = cur->nodete;

        if(elem.nodets < curts)
          return false;

        if(elem.nodets >= curte){
          head = cur;
          cur = cur->next;
          continue;
        }

        if(curts <= elem.nodets && elem.nodete <= curte){
          if(cur->maxd <= elem.mind)
            return true;//prune
        else
          return false;
        }

        if(curts <= elem.nodets && elem.nodete > curte){
          if(cur->maxd <= elem.mind){
            if(cur->next == NULL)
             return false;
            hpelem* next = cur->next;
            double nextts,nextte;
            while(next != NULL){
               nextts = next->nodets;
               curte = cur->nodete;
               if(nextts > curte)
                return false;

                nextte = next->nodete;

                if(nextte >= elem.nodete && next->maxd < elem.mind)
                  return true;//prune

                if(nextte < elem.nodete && next->maxd < elem.mind){
                    cur = next;
                    next = cur->next;
                    continue;
                }
                return false;
            }
            return false;
          }
          else
            return false;
        }
        head = cur;
        cur = cur->next;
      }
      return false;
}

/*
Detect whether the time interval in Nerestlist(k) is full

*/

void TBKnearestLocalInfo::CheckCovered(hpelem& elem)
{
  ci->insert(elem.nodets,elem.nodete);
  iscovered = ci->IsCovered();
}

void TBKnearestLocalInfo::CheckCoveredG(hpelem& elem)
{

//  ci->insert(elem.nodets,elem.nodete);
  struct CoverNode<double>* n = new CoverNode<double>(elem.nodets,elem.nodete);
  cic->insert(n);
//  iscovered = ci->IsCovered();
  iscovered = cic->IsCovered();
}

/*
Initialize Prunedist check the maxdist in Nearestlist(k)

*/

void TBKnearestLocalInfo::InitializePrunedist()
{
  if(prunedist.define == false && iscovered){
      hpelem* head = nlist[k-1].head;
      hpelem* cur = head->next;
      hpelem* bigelem;
      if(cur != NULL){
        double max = cur->maxd;
        bigelem = cur;
        while(cur != NULL){
          if(cur->maxd > max){
            max = cur->maxd;
            bigelem = cur;
          }
          cur = cur->next;
        }
        prunedist.dist = max;
        prunedist.define = true;
        prunedist.ts = bigelem->nodets;
        prunedist.te = bigelem->nodete;
      }
//    ftime(&gt2);
//    cout<<difftimeb(&gt2,&gt1)<<endl;
  }
}

/*
Main function, update the k nearest list structure and prunedist
traverse k nearestlist, and update it

*/

void TBKnearestLocalInfo::UpdatekNearest(hpelem& elem)
{
  list<hpelem> updatelist;

//    if(CheckPrune(local,elem) == false){ //faster
    if(elem.mind < prunedist.dist){

      updatelist.push_back(elem);
      vector<hpelem> auxiliarylist;
      vector<hpelem> templist;
      for(unsigned int i = 0;i < k;i++){//for each NearestList
        while(updatelist.empty() == false){
            hpelem top = updatelist.front();
            updatelist.pop_front();

//          if(CheckPrune(local,top) == false) //faster
            if(top.mind < prunedist.dist)
              UpdateNearest(top,templist,i); //key function

            //check whether the time interval covers
            if(i == k - 1 && iscovered == false)
              CheckCovered(top);

            for(unsigned int j = 0;j < templist.size();j++)//transfer step1
              auxiliarylist.push_back(templist[j]);
            templist.clear();
        }
        for(unsigned int j = 0;j < auxiliarylist.size();j++)//transfer step2
          updatelist.push_back(auxiliarylist[j]);
        auxiliarylist.clear();

        if(iscovered && i == k - 1) //check prunedist
          InitializePrunedist();
      }
  }
}

/*
Main function for Greece algorithm,
update the k nearest list structure and prunedist using
checkprune function

*/

void TBKnearestLocalInfo::UpdatekNearestG(hpelem& elem)
{
  list<hpelem> updatelist;

//    if(elem.mind < prunedist.dist){
//     if(CheckPrune(elem) == false){

      updatelist.push_back(elem);
      vector<hpelem> auxiliarylist;
      vector<hpelem> templist;
      for(unsigned int i = 0;i < k;i++){//for each NearestList
        while(updatelist.empty() == false){
            hpelem top = updatelist.front();
            updatelist.pop_front();

            if(top.mind < prunedist.dist && CheckPrune(top) == false)
//            if(CheckPrune(top) == false)
            UpdateNearest(top,templist,i); //key function

            if(i == k - 1 && iscovered == false)
              CheckCoveredG(top);
//                CheckCovered(top);

            for(unsigned int j = 0;j < templist.size();j++)//transfer step1
              auxiliarylist.push_back(templist[j]);
            templist.clear();
        }
        for(unsigned int j = 0;j < auxiliarylist.size();j++)//transfer step2
          updatelist.push_back(auxiliarylist[j]);
        auxiliarylist.clear();
        if(iscovered && i == k - 1) //check prunedist
          InitializePrunedist();
      }

//  }
}

/*
element is ordered by start time

*/
bool HpelemCompare(const hpelem& e1,const hpelem& e2)
{
    return e1.nodets < e2.nodets;
}

/*
Report result

*/

void TBKnearestLocalInfo::ReportResult()
{
  for(unsigned int i = 0; i < k;i++){ //traverse NearestList
    hpelem* head = nlist[i].head;
    hpelem* cur = head->next;
    while(cur != NULL){
      hpelem top = *cur;
      assert(top.tid != -1 && top.nodeid == -1);
      if(top.nodets != top.nodete)
        result.push_back(top);
      delete head;
      head = cur;
      cur = cur->next;
      if(cur == NULL)
        delete head;
    }

  }
  stable_sort(result.begin(),result.end(),HpelemCompare);
}


/*
ChinaknnFun is the value function for the chineseknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists an r-tree for the unit attribute
The main Function of chinese algorithm
using bf-first method
for leaf node, try to insert the entry into nearestlist
at the same time, update the prunedist
for each new element, using prunedist to check whether it needs to be inserted
into the heap structure

*/

void TBKnearestLocalInfo::ChinaknnFun(MPoint* mp)
{
  BBox<2> mpbox = mptraj->BoundingBox();
  if(mpbox.IsDefined() == false)
    mpbox = makexyBox(mp->BoundingBox());

  while(hp.empty() == false){
    hpelem top = hp.top();
    hp.pop();

    if(top.mind > prunedist.dist)
      break;
    if(top.tid != -1 && top.nodeid == -1){ //an actual trajectory segment
        UpdatekNearest(top);
    }
    else if(top.tid == -1 && top.nodeid != -1){ // a node
        tbtree::BasicNode<3>* tbnode = tbtree->getNode(top.nodeid);
        if(tbnode->isLeaf()){ //leaf node
            tbtree::TBLeafNode<3,TBLeafInfo>* leafnode =
            dynamic_cast<TBLeafNode<3,TBLeafInfo>* > (tbnode);
            for(unsigned int i = 0;i < leafnode->entryCount();i++){
              const Entry<3,TBLeafInfo>* entry = leafnode->getEntry(i);
              double t1((double)entry->getBox().MinD(2));
              double t2((double)entry->getBox().MaxD(2));
              if(!(t1 >= endTime || t2 <= startTime)){
                  //for each unit in mp
                UPoint up;
                TupleId tid = entry->getInfo().getTupleId();
                Tuple* tuple = this->relation->GetTuple(tid, false);
                UPoint* data = (UPoint*)tuple->GetAttribute(attrpos);

                for(int j = 0;j < mp->GetNoComponents();j++){
                    mp->Get(j,up);
                    double tt1 = (double)(up.timeInterval.start.ToDouble());
                    double tt2 = (double)(up.timeInterval.end.ToDouble());
                    if(tt1 > t2) //mq's time interval is larger than entry
                      break;
                    if(!(t1 >= tt2 || t2 <= tt1)){
                      Point p0(true,0,0);
                      Point p1(true,0,0);
                      UPoint* ne = new UPoint(up.timeInterval,p0,p1);
                      UPoint* nqe = new UPoint(up.timeInterval,p0,p1);

                      //interpolation restrict to the same time interval

                      CreateUPoint_ne(&up,ne,data);
                      CreateUPoint_nqe(&up,nqe,data);

                      double nodets = ne->timeInterval.start.ToDouble();
                      double nodete = ne->timeInterval.end.ToDouble();

                      if(AlmostEqual(nodets,nodete)){
                        delete ne;
                        delete nqe;
                        continue;
                      }

                      UReal* mdist = new UReal(true);
                      ne->Distance(*nqe,*mdist);
                      bool def = true;
                      double mind = mdist->Min(def);
                      double maxd = mdist->Max(def);

                      hpelem le(entry->getInfo().getTupleId(),mind,maxd,-1);
                      le.nodets = nodets;
                      le.nodete = nodete;

                      le.AssignURUP(mdist,ne);

                      if(le.mind < prunedist.dist)
                          hp.push(le);

                      delete mdist;
                      delete ne;
                      delete nqe;
                    }
                }//end for
                tuple->DeleteIfAllowed();
              }
           }
        }else{ //inner node
            tbtree::InnerNode<3,InnerInfo>* innernode =
              dynamic_cast<InnerNode<3,InnerInfo>* > (tbnode);
            for(unsigned int i = 0;i < innernode->entryCount();i++){
              const Entry<3,InnerInfo>* entry = innernode->getEntry(i);
              double t1((double)entry->getBox().MinD(2));
              double t2((double)entry->getBox().MaxD(2));
              if(!(t1 >= endTime || t2 <= startTime)){
                BBox<2> entrybox = makexyBox(entry->getBox());//entry box
                double mindist =  mpbox.Distance(entrybox);
                double maxdist = maxDistance(entrybox,mpbox);

                  hpelem le(-1,mindist,maxdist,entry->getInfo().getPointer());
                  le.nodets = t1;
                  le.nodete = t2;
                  if(le.mind < prunedist.dist)
                      hp.push(le);
              }
            }
        }
    }
  }
  ReportResult(); //results are stored in NearestList
}

/*
Initialization for greeceknearest operator
1---initialize nearest list structure
2---boundingbox, prunedist

*/

void TBKnearestLocalInfo::GreeceknearestInitialize(MPoint* mp)
{
  //Initialization NearestList and prunedist

  for(unsigned int i = 0;i < k;i++){
    double mind = numeric_limits<double>::max();
    double maxd = numeric_limits<double>::min();
    double st = startTime;
    double et = startTime;
    Nearestlist nl(mind,maxd,new hpelem(-1,0,0,-1),st,et);
    nlist.push_back(nl);
  }

  prunedist.dist = numeric_limits<double>::max();
  prunedist.define = false;

  Line* line = new Line(0);
  mp->Trajectory(*line);
  mptraj = new Line(*line);
  delete line;

  mpbox = mptraj->BoundingBox();
  if(mpbox.IsDefined() == false)
    mpbox = makexyBox(mp->BoundingBox());
}

/*
GreeceknnFun is the value function for the greeceknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists an r-tree for the unit attribute
The argument vector contains the following values:
args[0] = an r-tree with the unit attribute as key
args[1] = the relation of the r-tree
args[2] = attribute UPoint
args[3] = mpoint
args[4] = int k, how many nearest are searched

*/
struct idtime{
  long id;
  double ts;
  idtime(long a,double time):id(a),ts(time){}
  idtime(const idtime& idt):id(idt.id),ts(idt.ts){}
  bool operator<(const idtime& idt2) const
  {
    return ts < idt2.ts;
  }
};

void TBKnearestLocalInfo::GreeceknnFunBF(MPoint* mp,int level,hpelem& elem)
{
  hp.push(elem);
  SmiRecordId adr;

  while(hp.empty() == false){
    hpelem top = hp.top();
    hp.pop();

    if(top.mind > prunedist.dist)
      break;
    if(top.tid != -1 && top.nodeid == -1){ //an actual trajectory segment
        UpdatekNearestG(top);
    }
    else if(top.tid == -1 && top.nodeid != -1){ // a node
        adr = top.nodeid;
        R_TreeNode<3,TupleId>* tbnode =
                rtree->GetMyNode(adr,false,
                                 rtree->MinEntries(level),
                                 rtree->MaxEntries(level));

        if(tbnode->IsLeaf()){ //leaf node
            vector<idtime> entryarray;
            for(int i = 0; i < tbnode->EntryCount();i++){
                R_TreeLeafEntry<3,TupleId> entry =
                       (R_TreeLeafEntry<3,TupleId>&)(*tbnode)[i];
                double ts((double)entry.box.MinD(2));
                idtime idt(i,ts);
                entryarray.push_back(idt);
            }
            stable_sort(entryarray.begin(),entryarray.end());
            for(unsigned int j = 0;j < entryarray.size();j++){
//            for(unsigned int i = 0;i < tbnode->EntryCount();i++){
              int i = entryarray[j].id;
              R_TreeLeafEntry<3,TupleId> entry =
                       (R_TreeLeafEntry<3,TupleId>&)(*tbnode)[i];

              double t1((double)entry.box.MinD(2));
              double t2((double)entry.box.MaxD(2));
    // The following lines were used in published experiments, that
    // were done on a modified R-tree that scaled up the R-tree temporal
    // dimension by the same factor. They are commented out for use with
    // the standard R-tree.

//        t1 = t1/864000.0;
//        t2 = t2/864000.0;

              if(!(t1 >= endTime || t2 <= startTime)){
                  //for each unit in mp
                UPoint up;
                TupleId tid = entry.info;
                Tuple* tuple = this->relation->GetTuple(tid, false);
                UPoint* data = (UPoint*)tuple->GetAttribute(attrpos);

                for(int j = 0;j < mp->GetNoComponents();j++){
                    mp->Get(j,up);
                    double tt1 = (double)(up.timeInterval.start.ToDouble());
                    double tt2 = (double)(up.timeInterval.end.ToDouble());
                    if(tt1 > t2) //mq's time interval is larger than entry
                      break;
                    if(!(t1 >= tt2 || t2 <= tt1)){
                      Point p0(true,0,0);
                      Point p1(true,0,0);
                      UPoint* ne = new UPoint(up.timeInterval,p0,p1);
                      UPoint* nqe = new UPoint(up.timeInterval,p0,p1);

                      //interpolation restrict to the same time interval

                      CreateUPoint_ne(&up,ne,data);
                      CreateUPoint_nqe(&up,nqe,data);

                      double nodets = ne->timeInterval.start.ToDouble();
                      double nodete = ne->timeInterval.end.ToDouble();

                      if(AlmostEqual(nodets,nodete)){
                        delete ne;
                        delete nqe;
                        continue;
                      }

                      UReal* mdist = new UReal(true);
                      ne->Distance(*nqe,*mdist);
                      bool def = true;
                      double mind = mdist->Min(def);
                      double maxd = mdist->Max(def);

                      hpelem le(tid,mind,maxd,-1);
                      le.nodets = nodets;
                      le.nodete = nodete;

                      le.AssignURUP(mdist,ne);

                      if(le.mind < prunedist.dist)
                          hp.push(le);

                      delete mdist;
                      delete ne;
                      delete nqe;
                    }
                }//end for
                tuple->DeleteIfAllowed();
              }
           }
        }else{ //inner node

                vector<hpelem> branchlist;
                for(int i = 0;i < tbnode->EntryCount();i++){
                      R_TreeInternalEntry<3> e =
                        (R_TreeInternalEntry<3>&)(*tbnode)[i];
                      double t1((double)e.box.MinD(2));
                      double t2((double)e.box.MaxD(2));

            // The following lines were used in published experiments, that
           // were done on a modified R-tree that scaled up the R-tree temporal
           // dimension by the same factor. They are commented out for use with
           // the standard R-tree.

//                      t1 = t1/864000.0;
//                      t2 = t2/864000.0;

                    if(!(t1 >= endTime || t2 <= startTime)){
                        BBox<2> entrybox = makexyBox(e.box);//entry box
                        double mindist =  mpbox.Distance(entrybox);
                        double maxdist = maxDistance(entrybox,mpbox);

                        hpelem le(-1,mindist,maxdist,e.pointer);
                        le.nodets = t1;
                        le.nodete = t2;
                        branchlist.push_back(le);
                      }
                }
                stable_sort(branchlist.begin(),branchlist.end());
                vector<hpelem> prunelist;

                for(unsigned int i = 0; i < branchlist.size();i++){
                  if(branchlist[i].mind <  prunedist.dist)
                      if(CheckPrune(branchlist[i]) == false)
                          prunelist.push_back(branchlist[i]);
                }
                for(unsigned int i = 0;i < prunelist.size();){
                   hp.push(prunelist[i]);
                   unsigned int j = i + 1;
                   while(j < prunelist.size() &&
                         prunelist[j].mind > prunedist.dist){
                        if(CheckPrune(prunelist[j]))
                        j++;//prune branchlist
                        else
                          break;
                  }
                  i = j;
                }

        }
        delete tbnode;
    }
  }
}

void TBKnearestLocalInfo::GreeceknnFunDF(MPoint* mp,int level,hpelem& elem)
{
  const int dim = 3;

  SmiRecordId adr = elem.nodeid;
  R_TreeNode<dim,TupleId>* tbnode = rtree->GetMyNode(
            adr,false,rtree->MinEntries(level),rtree->MaxEntries(level));

  if(tbnode->IsLeaf()){ //leaf node

      vector<idtime> entryarray;
      for(int i = 0;i < tbnode->EntryCount();i++){
        R_TreeLeafEntry<dim,TupleId> e =
            (R_TreeLeafEntry<dim,TupleId>&)(*tbnode)[i];
        double ts((double)e.box.MinD(2));
        idtime idt(i,ts);
        entryarray.push_back(idt);
      }
      stable_sort(entryarray.begin(),entryarray.end());//sort by start time

//      for(int i = 0;i < tbnode->EntryCount();i++){
      for(unsigned int j = 0;j < entryarray.size();j++){
        int i = entryarray[j].id;
        R_TreeLeafEntry<dim,TupleId> e =
            (R_TreeLeafEntry<dim,TupleId>&)(*tbnode)[i];
        double t1((double)e.box.MinD(2));
        double t2((double)e.box.MaxD(2));

	// The following lines were used in published experiments, that
	// were done on a modified R-tree that scaled up the R-tree temporal
	// dimension by the same factor. They are commented out for use with
	// the standard R-tree.

//         t1 = t1/864000.0;
//         t2 = t2/864000.0;


        if(!(t1 >= endTime || t2 <= startTime)){
            //for each unit in mp
            UPoint up;
            TupleId tid = e.info;
            Tuple* tuple = relation->GetTuple(tid, false);
            UPoint* data = (UPoint*)tuple->GetAttribute(attrpos);

            for(int j = 0;j < mp->GetNoComponents();j++){
              mp->Get(j,up);
              double tt1 = (double)(up.timeInterval.start.ToDouble());
              double tt2 = (double)(up.timeInterval.end.ToDouble());

              if(tt1 > t2) //mq's time interval is larger than entry
                  break;
              if(!(t1 >= tt2 || t2 <= tt1)){
                    Point p0(true,0,0);
                    Point p1(true,0,0);
                    UPoint* ne = new UPoint(up.timeInterval,p0,p1);
                    UPoint* nqe = new UPoint(up.timeInterval,p0,p1);
                  //interpolation restrict to the same time interval

                  CreateUPoint_ne(&up,ne,data);
                  CreateUPoint_nqe(&up,nqe,data);

                  double nodets = ne->timeInterval.start.ToDouble();
                  double nodete = ne->timeInterval.end.ToDouble();
                  if(AlmostEqual(nodets,nodete)){
                      delete ne;
                      delete nqe;
                      continue;
                  }
                  UReal* mdist = new UReal(true);
                  ne->Distance(*nqe,*mdist);
                  bool def = true;
                  double mind = mdist->Min(def);
                  double maxd = mdist->Max(def);
                  hpelem le(tid,mind,maxd,-1);
                  le.nodets = nodets;
                  le.nodete = nodete;
                  le.AssignURUP(mdist,ne);
//                  if(CheckPrune(le) == false)
//                      UpdatekNearestG(le);
//                  if(le.mind < prunedist.dist)
//                    UpdatekNearest(le);

                  if(le.mind < prunedist.dist)
                      UpdatekNearestG(le);
                  delete mdist;
                  delete ne;
                  delete nqe;
              }
            }//end for
                tuple->DeleteIfAllowed();
        }
      }
  }else{ //an internal node
    vector<hpelem> branchlist;
    for(int i = 0;i < tbnode->EntryCount();i++){
       R_TreeInternalEntry<dim> e =
          (R_TreeInternalEntry<dim>&)(*tbnode)[i];
          double t1((double)e.box.MinD(2));
          double t2((double)e.box.MaxD(2));

	// The following lines were used in published experiments, that
	// were done on a modified R-tree that scaled up the R-tree temporal
	// dimension by the same factor. They are commented out for use with
	// the standard R-tree.

//        t1 = t1/864000.0;
//        t2 = t2/864000.0;

          if(!(t1 >= endTime || t2 <= startTime)){
              BBox<2> entrybox = makexyBox(e.box);//entry box
              double mindist =  mpbox.Distance(entrybox);
              double maxdist = maxDistance(entrybox,mpbox);

              hpelem le(-1,mindist,maxdist,e.pointer);
              le.nodets = t1;
              le.nodete = t2;
              branchlist.push_back(le);
          }
    }
    stable_sort(branchlist.begin(),branchlist.end());
    vector<hpelem> prunelist;

    for(unsigned int i = 0; i < branchlist.size();i++){
//        if(CheckPrune(branchlist[i]) == false)
//        prunelist.push_back(branchlist[i]);
      if(branchlist[i].mind <  prunedist.dist)
        if(CheckPrune(branchlist[i]) == false)
            prunelist.push_back(branchlist[i]);
    }
    for(unsigned int i = 0;i < prunelist.size();){
        GreeceknnFunDF(mp,level+1,prunelist[i]);
        unsigned int j = i + 1;

        while(j < prunelist.size() && prunelist[j].mind > prunedist.dist){
          if(CheckPrune(prunelist[j]))
            j++;//prune branchlist
          else
            break;
        }
        i = j;

//        while(j < prunelist.size() &&
//          prunelist[j].mind > prunedist.dist)j++;//prune branchlist
//        i = j;

    }
  }
  delete tbnode;
}

/*
Greeceknearest is the value function for the greeceknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists an r-tree for the unit attribute
The argument vector contains the following values:
args[0] = an r-tree with the unit attribute as key
args[1] = the relation of the r-tree
args[2] = attribute UPoint
args[3] = mpoint
args[4] = int k, how many nearest are searched

*/

int Greeceknearest(Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  const int dim = 3;
  TBKnearestLocalInfo *localInfo;
  switch (message)
  {
    case OPEN :
    {
      //Initialize hp,kNearestLists and PruneDist
      MPoint* mp = (MPoint*)args[3].addr;
      if(mp->IsEmpty())
        return 0;
      UPoint up1;
      UPoint up2;
      mp->Get(0,up1);
      mp->Get(mp->GetNoComponents()-1,up2);
      const unsigned int k = (unsigned int)((CcInt*)args[4].addr)->GetIntval();
      int attrpos = ((CcInt*)args[5].addr)->GetIntval()-1;
      localInfo = new TBKnearestLocalInfo(k);
      local = SetWord(localInfo);
      localInfo->attrpos = attrpos;
      localInfo->startTime = up1.timeInterval.start.ToDouble();
      localInfo->endTime = up2.timeInterval.end.ToDouble();
      localInfo->ci =
        new CIC<double>(localInfo->startTime,localInfo->endTime);

      localInfo->cic =
        new CoverInterval<double>(localInfo->startTime,localInfo->endTime);

      localInfo->rtree = (R_Tree<dim,TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->counter = 0;
      localInfo->scanFlag = true;
      localInfo->GreeceknearestInitialize(mp);

      SmiRecordId adr = localInfo->rtree->RootRecordId();
      R_TreeNode<dim,TupleId>* root = localInfo->rtree->GetMyNode(adr,
      false,localInfo->rtree->MinEntries(0),localInfo->rtree->MaxEntries(0));

      double t1(root->BoundingBox().MinD(2));
      double t2(root->BoundingBox().MaxD(2));

	// The following lines were used in published experiments, that
	// were done on a modified R-tree that scaled up the R-tree temporal
	// dimension by the same factor. They are commented out for use with
	// the standard R-tree.

//       t1 = t1/864000.0;
//       t2 = t2/864000.0;

      if(!(t1 >= localInfo->endTime || t2 <= localInfo->startTime)){
        BBox<2> entrybox = makexyBox(root->BoundingBox());
        double mindist = localInfo->mptraj->Distance(entrybox);
        double maxdist = maxDistance(entrybox,localInfo->mpbox);
        hpelem le(-1,mindist,maxdist,adr);
        le.nodets = t1;
        le.nodete = t2;
//        ftime(&gt1);
        localInfo->GreeceknnFunDF(mp,0,le); //2,12s train1,5
//        localInfo->GreeceknnFunBF(mp,0,le);//0.73s train1,5
        localInfo->ReportResult();
      }
      delete root;
      return 0;
    }
    case REQUEST :
    {
        localInfo = (TBKnearestLocalInfo*)local.addr;
        if(localInfo->k == 0)
          return CANCEL;
        if(localInfo->counter < localInfo->result.size()){
            TupleId tid = localInfo->result[localInfo->counter].tid;
            Tuple* tuple = localInfo->relation->GetTuple(tid, false);
            UPoint* up = (UPoint*)tuple->GetAttribute(localInfo->attrpos);
            Point p0;
            Point p1;
            Instant t1(instanttype);
            Instant t2(instanttype);
            t1.ReadFrom(localInfo->result[localInfo->counter].nodets);
            t2.ReadFrom(localInfo->result[localInfo->counter].nodete);
            Interval<Instant> interv(t1,t2,true,true);
            UPoint* temp = new UPoint(*up);
            temp->TemporalFunction(t1,p0,true);
            temp->TemporalFunction(t2,p1,true);
            temp->p0 = p0;
            temp->timeInterval.start = t1;
            temp->p1 = p1;
            temp->timeInterval.end = t2;
            double factor = 0.0000001;
            if((localInfo->result[localInfo->counter].nodete -
                localInfo->result[localInfo->counter].nodets) < factor)
              temp->timeInterval.lc = temp->timeInterval.rc = true;

            tuple->PutAttribute(localInfo->attrpos,new UPoint(*temp));
            delete temp;

            result = SetWord(tuple);
            localInfo->counter++;
            return YIELD;
        }else
          return CANCEL;
    }

    case CLOSE :
    {
      localInfo = (TBKnearestLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;

}

/*
ChinaknearestFun is the value function for the chinaknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists a tb-tree for the unit attribute
The argument vector contains the following values:
args[0] = a tbtree with the unit attribute as key
args[1] = the relation of the tbtree
args[2] = attribute UPoint
args[3] = mpoint
args[4] = int k, how many nearest are searched

*/

int ChinaknearestFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  TBKnearestLocalInfo* localInfo;
  switch (message)
  {
    case OPEN :
    {
      //Initialize hp,kNearestLists and PruneDist
      MPoint* mp = (MPoint*)args[3].addr;
      if(mp->IsEmpty())
        return 0;
      UPoint up1;
      UPoint up2;
      mp->Get(0,up1);
      mp->Get(mp->GetNoComponents()-1,up2);
      const unsigned int k = (unsigned int)((CcInt*)args[4].addr)->GetIntval();
      int attrpos = ((CcInt*)args[5].addr)->GetIntval()-1;
      localInfo = new TBKnearestLocalInfo(k);
      local = SetWord(localInfo);
      localInfo->attrpos = attrpos;
      localInfo->startTime = up1.timeInterval.start.ToDouble();
      localInfo->endTime = up2.timeInterval.end.ToDouble();

      localInfo->ci =
        new CIC<double>(localInfo->startTime,localInfo->endTime);

      localInfo->cic =
        new CoverInterval<double>(localInfo->startTime,localInfo->endTime);

      localInfo->tbtree = (TBTree*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->counter = 0;
      localInfo->scanFlag = true;
      localInfo->ChinaknnInitialize(mp);
      localInfo->ChinaknnFun(mp);
      return 0;
    }
    case REQUEST :
    {
        localInfo = (TBKnearestLocalInfo*)local.addr;
        if(localInfo->k == 0)
          return CANCEL;
        if(localInfo->counter < localInfo->result.size()){
            TupleId tid = localInfo->result[localInfo->counter].tid;
            Tuple* tuple = localInfo->relation->GetTuple(tid, false);
            UPoint* up = (UPoint*)tuple->GetAttribute(localInfo->attrpos);
            Point p0;
            Point p1;
            Instant t1(instanttype);
            Instant t2(instanttype);
            t1.ReadFrom(localInfo->result[localInfo->counter].nodets);
            t2.ReadFrom(localInfo->result[localInfo->counter].nodete);
            Interval<Instant> interv(t1,t2,true,true);
            UPoint* temp = new UPoint(*up);
            temp->TemporalFunction(t1,p0,true);
            temp->TemporalFunction(t2,p1,true);
            temp->p0 = p0;
            temp->timeInterval.start = t1;
            temp->p1 = p1;
            temp->timeInterval.end = t2;
            double factor = 0.0000001;
            if((localInfo->result[localInfo->counter].nodete -
                localInfo->result[localInfo->counter].nodets) < factor)
              temp->timeInterval.lc = temp->timeInterval.rc = true;

            tuple->PutAttribute(localInfo->attrpos,new UPoint(*temp));
            delete temp;

            result = SetWord(tuple);
            localInfo->counter++;
            return YIELD;
        }else
          return CANCEL;
    }

    case CLOSE :
    {
      localInfo = (TBKnearestLocalInfo*)local.addr;

      delete localInfo;
      return 0;
    }
  }
  return 0;
}

/*
The function hcknnknearestTypeMap is the type map for the
operator knearestfilter

*/
ListExpr ChinaknearestTypeMap( ListExpr args )
{

  string errmsg = "tbtree x relation x utrip x mpoint x int expected";

  if(nl->ListLength(args)!=5){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }
  ListExpr tbtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr attrName = nl->Third(args);
  ListExpr mpoint = nl->Fourth(args);
  ListExpr quantity = nl->Fifth(args);

  // the third element has to be of type mpoint
  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,CcInt::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // an tbtree description must have length 4
  if(nl->ListLength(tbtreeDescription)!=4){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr tbtreeSymbol = nl->First(tbtreeDescription);

  if(!nl->IsEqual(tbtreeSymbol, "tbtree")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!IsRelDescription(relDescription)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }
  ListExpr attrType;
  int j = listutils::findAttribute(nl->Second(nl->Second(relDescription)),
      nl->SymbolValue(attrName),attrType);
  if(j==0 || !listutils::isSymbol(attrType,UPoint::BasicType())){
     return listutils::typeError("upoint expected");
  }
  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->Second(nl->Second(args)));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
          nl->OneElemList(nl->IntAtom(j)),res);

}

/*
The function rknearestFilterTypeMap is the type map for the
operator knearestfilter

*/
ListExpr GreeceknearestTypeMap( ListExpr args )
{

  string errmsg = "rtree x relation x utrip x mpoint x int expected";

  if(nl->ListLength(args)!=5){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }
  ListExpr tbtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr attrName = nl->Third(args);
  ListExpr mpoint = nl->Fourth(args);
  ListExpr quantity = nl->Fifth(args);

  // the third element has to be of type mpoint
  if(!nl->IsEqual(mpoint,MPoint::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,CcInt::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // an tbtree description must have length 4
  if(nl->ListLength(tbtreeDescription)!=4){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr tbtreeSymbol = nl->First(tbtreeDescription);

  if(!nl->IsEqual(tbtreeSymbol, RTree3TID::BasicType())){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!IsRelDescription(relDescription)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr attrType;
  int j = FindAttribute(nl->Second(nl->Second(relDescription)),
      nl->SymbolValue(attrName),attrType);
  if(j==0 || !listutils::isSymbol(attrType,UPoint::BasicType())){
    return listutils::typeError("upoint expected");
  }
  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->Second(nl->Second(args)));

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
          nl->OneElemList(nl->IntAtom(j)),res);
}

Operator greeceknearest (
         "greeceknearest",        // name
         GreeceknearestfilterSpec,      // specification
         Greeceknearest,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         GreeceknearestTypeMap    // type mapping
);
Operator chinaknearest (
         "chinaknearest",        // name
         ChinaknearestSpec,      // specification
         ChinaknearestFun,// value mapping
         Operator::SimpleSelect,  // trivial selection function
         ChinaknearestTypeMap    // type mapping
);

const string CellIndexSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x cellno ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text> cellindex (_, _)</text--->"
      "<text>The operator results a stream of tuples "
      "where each tuple corresponds to a cell. "
      "and for each cell it records all nodes of index that intersect "
      "it in space"
      "The operator expects an r-tree and cell number  </text--->"
      "<text>query cellindex(strassen_geoData_rtree,10) count;</text--->"
      ") )";

/*
The function CellIndexTypeMap is the type map for the
operator cellindex

*/
ListExpr CellIndexTypeMap( ListExpr args )
{

  string errmsg = "rtree x cellnumber expected";

  if(nl->ListLength(args)!=2 ){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr rtree = nl->First(args);

  if(!listutils::isRTreeDescription(rtree)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }


  string rtreedescription;
  nl->WriteToString(rtreedescription,rtree);




  ListExpr cellnumber = nl->Second(args);
  ListExpr MBR_ATOM;

  if(!listutils::isRTreeDescription(rtree)){
    return listutils::typeError("first argument must be an rtree");
  }
  ListExpr rtsymbol = nl->First(rtree);


  if(nl->IsAtom(rtsymbol) &&
    nl->AtomType(rtsymbol) == SymbolType &&
    (nl->SymbolValue(rtsymbol) == RTree2TID::BasicType() ||
     nl->SymbolValue(rtsymbol) == RTree3TID::BasicType())&&
    nl->SymbolValue(cellnumber) == CcInt::BasicType()){
    if(nl->SymbolValue(rtsymbol) == RTree2TID::BasicType())
      MBR_ATOM = nl->SymbolAtom(Rectangle<2>::BasicType());
    if(nl->SymbolValue(rtsymbol) == RTree3TID::BasicType())
      MBR_ATOM = nl->SymbolAtom(Rectangle<3>::BasicType());
   return
        nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->FourElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("cellid"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("MBR"),
                        MBR_ATOM
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("nodeid"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("nodelevel"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ))));
  }
  ErrorReporter::ReportError(errmsg);
  return nl->TypeError();
}
/*
Partition the space into several cells, and for each cell it records the node of
index that intersects the cell and the level of node in index

*/
struct SubCell{
  int nodeid;
  int nodelevel;
  SubCell(){}
  SubCell(int id,int l):nodeid(id),nodelevel(l){}
  SubCell(const SubCell& sc):nodeid(sc.nodeid),nodelevel(sc.nodelevel){}
  SubCell& operator=(const SubCell& sc)
  {
    nodeid = sc.nodeid;
    nodelevel = sc.nodelevel;
    return *this;
  }
};
template<unsigned dim>
struct Cell:public SubCell{
  int id;
  BBox<dim> box;
  Cell(int i,BBox<dim> b,int nodeid,int l):SubCell(nodeid,l),id(i),box(b){}
  Cell(const Cell& cell):SubCell(cell),id(cell.id),box(cell.box){}
  Cell& operator=(const Cell& cell)
  {
    SubCell::operator=(cell);
    id = cell.id;
    box = cell.box;
    return *this;
  }
};
template<unsigned dim>
struct CellIndex{
  R_Tree<dim,TupleId>* rtree;
  long cellno;
  CellIndex(R_Tree<dim,TupleId>*r, long no)
  :rtree(r),cellno(no){}

  vector<Cell<dim> > cellq;
  vector<double> gmin;
  vector<double> gmax;
  vector<double> cellsize;
  SubCell tempcell;

  unsigned int counter;
  TupleType* resulttype;
  ~CellIndex(){delete resulttype;}
  void Partition_Cell();
  void Accessfunction(int,vector<long>&,vector<long>&,vector<int>&);
};
/*
It partitions the space covered by an R-tree into several cells and for each
cell it records all nodes from the R-tree that their bounding boxes intersect
the cell.

*/

template<unsigned dim>
void CellIndex<dim>::Accessfunction(int depth,vector<long>& start_index,
vector<long>& end_index,vector<int>& pos)
{
  if(depth == dim - 1){
    for(long i = start_index[depth]; i <= end_index[depth];i++){
      pos[depth] = i;
      long cellid = 1;
      for(int j = depth;j >= 0;j--){
        int k = j - 1;
        int size_low = 1;
        while( k >= 0){
          size_low = size_low * cellno;
          k--;
        }
        cellid += size_low * pos[j];
//        cout<<cellid<<endl;
      }
      double min[dim];
      double max[dim];
      for(unsigned int j = 0;j < dim;j++){
        min[j] = gmin[j] + pos[j]*cellsize[j];
        max[j] = gmin[j] + (pos[j]+1)*cellsize[j];
      }
      Rectangle<dim> cellbox(true,min,max);
      cellq.push_back(Cell<dim>(cellid,cellbox,
                                tempcell.nodeid,tempcell.nodelevel));
    }
  }else{
    for(long i = start_index[depth];i <= end_index[depth];i++){
      pos[depth] = i;
      Accessfunction(depth+1,start_index,end_index,pos);
    }
  }

}
template<unsigned dim>
void CellIndex<dim>::Partition_Cell()
{
//  cout<<CI->cellq.max_size()<<endl;
  BBox<dim> r_box = rtree->BoundingBox();
////

  for(unsigned int i = 0; i < dim;i++){
    gmin.push_back(r_box.MinD(i));
    gmax.push_back(r_box.MaxD(i));
  }

/*double minx = r_box.MinD(0);
  double maxx = r_box.MaxD(0);
  double miny = r_box.MinD(1);
  double maxy = r_box.MaxD(1);*/


  for(unsigned int i = 0; i < dim;i++){
//    cout<<(long)floor((gmax[i]-gmin[i])/cellno)<<endl;
    cellsize.push_back(floor((gmax[i]-gmin[i])/cellno));
  }
/////////
/*double sizex = (long)floor((maxx-minx)/cellno);
  double sizey = (long)floor((maxy-miny)/cellno);*/

  queue<SubCell> q_node;
  SmiRecordId adr = rtree->RootRecordId();
  R_TreeNode<dim,TupleId>* node =
        rtree->GetMyNode(adr,false,
        rtree->MinEntries(0),
        rtree->MaxEntries(0));
  if(!node->IsLeaf()){
  //skip root node
    for(int i = 0; i < node->EntryCount();i++){
        R_TreeInternalEntry<dim> e = (R_TreeInternalEntry<dim>&)(*node)[i];
        q_node.push(SubCell(e.pointer,1));
    }
  }
  delete node;

  vector<int> pos;

  vector<double> lower;
  vector<double> upper;
  vector<long> start_index;
  vector<long> end_index;

  while(q_node.empty() == false){
    SubCell subcell = q_node.front();
    q_node.pop();
    adr = subcell.nodeid;
    node =
        rtree->GetMyNode(adr,false,
        rtree->MinEntries(subcell.nodelevel),
        rtree->MaxEntries(subcell.nodelevel));

/*double lowerx = node->BoundingBox().MinD(0);
    double upperx = node->BoundingBox().MaxD(0);
    double lowery = node->BoundingBox().MinD(1);
    double uppery = node->BoundingBox().MaxD(1);
    long start_i = (long)floor((lowerx-minx)/sizex);//starts from 1
    long end_i = (long)floor((upperx-minx)/sizex);
    long start_j = (long)floor((lowery-miny)/sizey);
    long end_j = (long)floor((uppery-miny)/sizey);*/

///////////////
    lower.clear();
    upper.clear();
    start_index.clear();
    end_index.clear();

    for(unsigned int i = 0;i < dim;i++){
        lower.push_back(node->BoundingBox().MinD(i));
        upper.push_back(node->BoundingBox().MaxD(i));
    }
    for(unsigned int i = 0;i < dim;i++){
        start_index.push_back((long)floor((lower[i]-gmin[i])/cellsize[i]));
        end_index.push_back((long)floor((upper[i]-gmin[i])/cellsize[i]));
    }
//    cout<<pos.size()<<endl;
      tempcell = subcell;
      pos.clear();
      for(unsigned int i = 0;i < dim;i++)
        pos.push_back(-1);

      Accessfunction(0,start_index,end_index,pos);

////////////
/*for(;start_i <= end_i;start_i++){
      for(long temp_j = start_j;temp_j <= end_j; temp_j++){
      long cellid = temp_j*cellno + start_i + 1;
//        long cellid = start_i*cellno + temp_j + 1;
        double min[2];
        double max[2];
        min[0] = minx + start_i*sizex;
        max[0] = minx + (start_i+1)*sizex;
        min[1] = miny + temp_j*sizey;
        max[1] = miny + (temp_j+1)*sizey;
        BBox<dim> cellbox(true,min,max);
        cellq.push_back(
                Cell<dim>(cellid,cellbox,subcell.nodeid,subcell.nodelevel));
      }
    }*/

    if(node->IsLeaf()){
      delete node;
      continue;
    }
    for(int i = 0; i < node->EntryCount();i++){
      R_TreeInternalEntry<dim> e = (R_TreeInternalEntry<dim>&)(*node)[i];
      q_node.push(SubCell(e.pointer,subcell.nodelevel+1));
    }
    delete node;
  }
}
template<unsigned dim>
int CellIndexFun(Word* args, Word& result, int message,
              Word& local, Supplier s){

  CellIndex<dim>* localInfo;
  switch(message){
     case OPEN: {
     localInfo = new CellIndex<dim>((R_Tree<dim,TupleId>*)args[0].addr,
        ((CcInt*)args[1].addr)->GetIntval());
     localInfo->counter = 0;
     localInfo->resulttype = new
                      TupleType(nl->Second(GetTupleResultType(s)));
     localInfo->Partition_Cell();
     local = SetWord(localInfo);
     return 0;
   }
   case REQUEST: {
      localInfo = (CellIndex<dim>*)local.addr;
      if(localInfo->counter == localInfo->cellq.size())
        return CANCEL;
      Tuple* tuple = new Tuple(localInfo->resulttype);
      Cell<dim> cell = localInfo->cellq[localInfo->counter] ;
      tuple->PutAttribute(0,new CcInt(true,cell.id));
      tuple->PutAttribute(1,new Rectangle<dim>(cell.box));
      tuple->PutAttribute(2,new CcInt(true,cell.nodeid));
      tuple->PutAttribute(3,new CcInt(true,cell.nodelevel));
      localInfo->counter++;
      result.setAddr(tuple);
      return YIELD;
   }
   case CLOSE : {
      localInfo = (CellIndex<dim>*)local.addr;
      delete localInfo;
      return 0;
   }
 }
 return 0;
}
ValueMapping CellIndexFunMap [] =
{
  CellIndexFun<2>,
  CellIndexFun<3>
};
int CellIndexSelect(ListExpr args)
{
  ListExpr rtree = nl->First(args);
  string rtreedescription;
  nl->WriteToString(rtreedescription,rtree);
  ListExpr rtsymbol = nl->First(rtree);

  if(nl->IsAtom(rtsymbol) &&
    nl->AtomType(rtsymbol) == SymbolType &&
    nl->SymbolValue(rtsymbol) == RTree2TID::BasicType())
  return 0;
  if(nl->IsAtom(rtsymbol) &&
    nl->AtomType(rtsymbol) == SymbolType &&
    nl->SymbolValue(rtsymbol) == RTree3TID::BasicType())
  return 1;
  return -1;
}
Operator cellindex(
        "cellindex",
        CellIndexSpec,
        2,
        CellIndexFunMap,
        CellIndexSelect,
        CellIndexTypeMap
);



ListExpr GnuplotNodeTypeMap( ListExpr args )
{

  string errmsg = "stream x attribute x string expected";

  if(nl->ListLength(args)!=3){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr stream = nl->First(args);
  ListExpr attrName = nl->Second(args);
  ListExpr filename = nl->Third(args);
  if(!IsStreamDescription(stream)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }
  if(nl->AtomType(attrName) != SymbolType){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!(nl->SymbolValue(filename) == CcString::BasicType())){
      ErrorReporter::ReportError(errmsg);
      return nl->TypeError();
  }
  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(stream)),
                    nl->SymbolValue(attrName),attrType);

  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                         nl->OneElemList(nl->IntAtom(j)),
                         nl->SymbolAtom(CcInt::BasicType()));
}

/*
It outputs the coordinate of each box into a data file which will be
read by gnuplot program to visualize the data

*/
struct Box_data{
  double minx;
  double miny;
  double mint;
  double maxx;
  double maxy;
  double maxt;
  Box_data(double a,double b,double c,double x,double y,double z)
  :minx(a),miny(b),mint(c),maxx(x),maxy(y),maxt(z){}
  Box_data(const Box_data& bd)
  :minx(bd.minx),miny(bd.miny),mint(bd.mint),
  maxx(bd.maxx),maxy(bd.maxy),maxt(bd.maxt){}
  Box_data& operator=(const Box_data& bd)
  {
    minx = bd.minx;
    miny = bd.miny;
    mint = bd.mint;
    maxx = bd.maxx;
    maxy = bd.maxy;
    maxt = bd.maxt;
    return *this;
  }
};
int gnuplotnodeFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  int count = 0;
  result = qp->ResultStorage(s);
  Word stream = args[0];
  Word current(Address(0));
  qp->Open(args[0].addr);
  qp->Request(stream.addr,current);
  if(!qp->Received(stream.addr)){
    ((CcInt*)result.addr)->Set(true,0);
    return 0;
  }
  CcString* file = (CcString*)args[2].addr;
  ofstream datafile(file->GetValue().c_str());
  int pos = ((CcInt*)args[3].addr)->GetIntval() - 1;
  bool  finished = false;
  vector<Box_data> data;
  double min = numeric_limits<double>::max();
  while(!finished){
    if(!finished){
      Tuple* tuple = static_cast<Tuple*>(current.addr);
      Rectangle<3>* box = (Rectangle<3>*)tuple->GetAttribute(pos);
//      cout<<*box<<endl;

      data.push_back(Box_data(box->MinD(0),box->MinD(1),box->MinD(2),
                    box->MaxD(0),box->MaxD(1),box->MaxD(2)));
      if(box->MinD(2) < min)
        min = box->MinD(2);

      tuple->DeleteIfAllowed();
      count++;
//      if(count >= 50){
//        cout<<"maximum number of nodes to plot is 50"<<endl;
//        break;
//      }
    }
    qp->Request(stream.addr,current);
    finished = !qp->Received(stream.addr);
  }
  for(unsigned int i = 0; i < data.size();i++){
    Box_data box_data = data[i];
    datafile<<box_data.minx<<" "<<box_data.miny<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.miny<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.miny<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.miny<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.miny<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.miny<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.miny<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.miny<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<box_data.minx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.mint-min)<<endl;
    datafile<<box_data.maxx<<" "<<box_data.maxy<<" "
    <<864000*(box_data.maxt-min)<<endl;
    datafile<<endl<<endl;

  }
  cout<<count<<" nodes output in file "<<file->GetValue()<<endl;
  ((CcInt*)result.addr)->Set(true,count);
  return 0;
}


const string GnuplotnodeSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>a stream of tuples with 3D box attribute x output_file->"
      " output a file"
      "</text--->"
      "<text> gnuplotnode (_, _)</text--->"
       "it outputs the data of bounding box from a certain level into "
       "a file read by gnuplot"
      "<text>query nodes(UnitTrains_UTrip) filter [.level=1] "
      "gnuplot(MBR,\"file\"] count;</text--->"
      "))";

Operator gnuplotnode(
        "gnuplotnode",
         GnuplotnodeSpec,
         gnuplotnodeFun,
         Operator::SimpleSelect,
         GnuplotNodeTypeMap
);


struct PairElem{
  myureal movdist;
  TupleId tid1,tid2;
  myupoint p1,p2;
  long nid1,nid2;
  double mind,maxd;
  double stime,etime;
  PairElem* next;
  PairElem(TupleId id1,TupleId id2,long n1,long n2,double min,double max,
          double s,double e)
  :tid1(id1),tid2(id2),nid1(n1),nid2(n2),mind(min),maxd(max),stime(s),etime(e){}
};

struct PairList{
  double mind,maxd;
  PairElem* head;
  double stime,etime;
  PairList(double min,double max,PairElem* pe,double s,double e)
  :mind(min),maxd(max),head(pe),stime(s),etime(e){}
  PairList(const PairList& pl)
  :mind(pl.mind),maxd(pl.maxd),head(pl.head),stime(pl.stime),etime(pl.etime){}
};


/*
It partitions the space into a list of cubes, and for each cube it records all
upoints inside. If the upoint intersect more than one cube, interpolates the
cube and put each sub part into its corresponding cube.
args[0] - 3D box
args[1] - an relation with attribute upoint
args[2] - attrName for upoint
args[3,4,5] - the number of cell for each dimension,x,y,t

*/
struct CellPart{
  Rectangle<3>* box;
  int attrpos;
  int cellno[3];
  int cellx,celly,cellt;
  TupleType* resulttype;
  unsigned int count;
  unsigned int id;
  Tuple* lasttuple;
  double global_min;
  vector<double> gmin;
  vector<double> gmax;
  vector<double> cellsize;
  CellPart(Rectangle<3>* bbox,int x,int y,int z)
  :box(bbox),cellx(x),celly(y),cellt(z){
    count = 0;
    id = 1;
    resulttype = NULL;
    for(unsigned int i = 0;i < 3;i++){
      gmin.push_back(bbox->MinD(i));
      gmax.push_back(bbox->MaxD(i));
    }
    cellsize.push_back(floor((gmax[0]-gmin[0])/cellx));
    cellsize.push_back(floor((gmax[1]-gmin[1])/celly));
    cellsize.push_back(floor((gmax[2]-gmin[2])/cellt));
    cellno[0] = cellx;
    cellno[1] = celly;
    cellno[2] = cellt;
//
    gmax[2] = gmax[2] - gmin[2];
    global_min = gmin[2];
    gmin[2] = 0;
//
    for(unsigned int i = 0;i < 3;i++)
      cout<<"size dimension "<<i<<" "<<cellsize[i]<<endl;
  }
  ~CellPart(){if(resulttype != NULL)delete resulttype;}

  vector<UPoint> ups;
  vector<BBox<3> > cellbox; //store the box of each cell

  void AssignUPinCell();
  void AccessFunction(int depth,vector<long>& start_index,
       vector<long>& end_index,vector<int>& pos,UPoint* up);
};
void CellPart::AccessFunction(int depth,vector<long>& start_index,
       vector<long>& end_index,vector<int>& pos,UPoint* up)
{
  if(depth == 2){
    for(long i = start_index[depth]; i <= end_index[depth];i++){
      pos[depth] = i;
      long cellid = 1;
      for(int j = depth;j >= 0;j--){
        int k = j - 1;
        int size_low = 1;
        while( k >= 0){
          size_low = size_low * cellno[k];
          k--;
        }
        cellid += size_low * pos[j];
//        cout<<cellid<<endl;
      }
      double min[3];
      double max[3];
      for(unsigned int j = 0;j < 3;j++){
        min[j] = gmin[j] + pos[j]*cellsize[j];
        max[j] = gmin[j] + (pos[j]+1)*cellsize[j];
      }
      Rectangle<3> cell_box(true,min,max);
//      cell_box.Print(cout);
      //interpolate up to small one
      UPoint* tempup = new UPoint(*up);
      if(tempup->timeInterval.start.ToDouble()*86400.0 - global_min < min[2]){
          Instant start(instanttype);
          start.ReadFrom((min[2] + global_min)/86400.0);
          Point p0;
          tempup->TemporalFunction(start,p0,true);
          tempup->p0 = p0;
          tempup->timeInterval.start = start;

      }
      if(tempup->timeInterval.end.ToDouble()*86400.0 - global_min > max[2]){
          Instant end(instanttype);
          end.ReadFrom((max[2] + global_min)/86400.0);
          Point p1;
          tempup->TemporalFunction(end,p1,true);
          tempup->p1 = p1;
          tempup->timeInterval.end = end;

      }
      if(tempup->p0.GetX() < min[0]){
          double x = min[0];
          double y = tempup->p0.GetY();
          tempup->p0.Set(x,y);
      }
      if(tempup->p1.GetX() < min[0]){
          double x = min[0];
          double y = tempup->p1.GetY();
          tempup->p0.Set(x,y);
      }
      if(tempup->p0.GetX() > max[0]){
          double x = max[0];
          double y = tempup->p0.GetY();
          tempup->p0.Set(x,y);
      }
      if(tempup->p1.GetX() > max[0]){
          double x = max[0];
          double y = tempup->p1.GetY();
          tempup->p0.Set(x,y);
      }
      if(tempup->p0.GetY() < min[1]){
          double x = tempup->p0.GetX();
          double y = min[1];
          tempup->p0.Set(x,y);
      }
      if(tempup->p1.GetY() < min[1]){
          double x = tempup->p0.GetX();
          double y = min[1];
          tempup->p1.Set(x,y);
      }
      if(tempup->p0.GetY() > max[1]){
          double x = tempup->p0.GetX();
          double y = max[1];
          tempup->p0.Set(x,y);
      }
      if(tempup->p1.GetY() > max[1]){
          double x = tempup->p0.GetX();
          double y = max[1];
          tempup->p1.Set(x,y);
      }
//      cout<<*tempup<<endl;
      if(!AlmostEqual(tempup->timeInterval.start.ToDouble(),
                      tempup->timeInterval.end.ToDouble())){
        cellbox.push_back(cell_box);
        ups.push_back(*tempup);
      }
      delete tempup;
    }
  }else{
    for(long i = start_index[depth];i <= end_index[depth];i++){
      pos[depth] = i;
      AccessFunction(depth+1,start_index,end_index,pos,up);
    }
  }
}
void CellPart::AssignUPinCell()
{
/*ups.clear();
  cellbox.clear();

  UPoint* up = (UPoint*)(lasttuple->GetAttribute(attrpos));
  cout<<"proces unit "<<*up<<endl;
  BBox<3> box = up->BoundingBox();
  double boxmin[3],boxmax[3];

  for(unsigned int i = 0;i < 3;i++){
      boxmin[i] = box.MinD(i);
      boxmax[i] = box.MaxD(i);
  }
  boxmin[2] = boxmin[2] * 86400.0 - global_min;
  boxmax[2] = boxmax[2] * 86400.0 - global_min;

  vector<double> lower;
  vector<double> upper;
  vector<long> start_index;
  vector<long> end_index;
  vector<int> pos;

  for(unsigned int i = 0;i < 3;i++){
    lower.push_back(boxmin[i]);
    upper.push_back(boxmax[i]);
  }

  for(unsigned int i = 0;i < 3;i++){
    start_index.push_back((long)floor((lower[i]-gmin[i])/cellsize[i]));
    end_index.push_back((long)floor((upper[i]-gmin[i]) /cellsize[i]));
//    cout<<"start "<<i<<" "<<start_index[i]<<endl;
//    cout<<"end "<<i<<" "<<end_index[i]<<endl;
  }


//  cout<<"range "<<endl;
//  for(unsigned int i = 0;i < 3;i++)
//    cout<<boxmin[i]<<" "<<boxmax[i]<<endl;

//  AccessFunction(0,start_index,end_index,pos,up);

///
  vector<UPoint> tempups;
  for(int i = start_index[2];i <= end_index[2];i++){
      double t_min = gmin[2] + i*cellsize[2];
      double t_max = gmin[2] + (i+1)*cellsize[2];
//      cout<<t_min<<" "<<t_max<<endl;
      Instant start(instanttype);
      start.ReadFrom((t_min + global_min)/86400.0);
      Instant end(instanttype);
      end.ReadFrom((t_max + global_min)/86400.0);
//      cout<<start<<" "<<end<<endl;
      if(start < up->timeInterval.start)
        start = up->timeInterval.start;
      if(end > up->timeInterval.end)
        end = up->timeInterval.end;
      UPoint* upoint = new UPoint(*up);
      Point p0,p1;
      up->TemporalFunction(start,p0,true);;
      up->TemporalFunction(end,p1,true);
      upoint->p0 = p0;
      upoint->p1 = p1;
      upoint->timeInterval.start = start;
      upoint->timeInterval.end = end;
      tempups.push_back(*upoint);
      cout<<*upoint<<endl;
      delete upoint;
  }

  for(unsigned int i = 0;i < tempups.size();i++){
      box = tempups[i].BoundingBox();
      for(unsigned int j = 0;j < 3;j++){
        boxmin[j] = box.MinD(j);
        boxmax[j] = box.MaxD(j);
      }
      boxmin[2] = boxmin[2] * 86400.0 - global_min;
      boxmax[2] = boxmax[2] * 86400.0 - global_min;
      lower.clear();
      upper.clear();
      start_index.clear();
      end_index.clear();
      pos.clear();
      for(unsigned int j = 0;j < 3;j++){
        lower.push_back(boxmin[j]);
        upper.push_back(boxmax[j]);
      }

      for(unsigned int j = 0;j < 3;i++){
        start_index.push_back((long)floor((lower[j]-gmin[i])/cellsize[j]));
        end_index.push_back((long)floor((upper[j]-gmin[i]) /cellsize[j]));
        cout<<"start "<<j<<" "<<start_index[j]<<endl;
        cout<<"end "<<j<<" "<<end_index[j]<<endl;
      }
      cout<<endl;

      for(unsigned int i = 0;i < 3;i++)
        pos.push_back(-1);

      AccessFunction(0,start_index,end_index,pos,&tempups[i]);
/////////////

      double min[3];
      double max[3];
      int position[2];
      for(int i = start_index[0];i <= end_index[0];i++){
        for(int j = start_index[1];j <= end_index[j];j++){
          position[0] = i;
          position[1] = j;
          for(unsigned int k = 0;k < 2;k++){
            min[k] = gmin[k] + position[k]*cellsize[k];
            max[k] = gmin[k] + (position[k]+1)*cellsize[k];
          }
          //to be continued
        }
      }

  }*/

}

/*const string MergertreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(rtree1<d> (tuple ((x1 t1)...(xn tn))) ti) x \n"
  "(rtree2<d> (tuple ((x1 t1)...(xn tn))) ti) x "
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti)</text--->"
  "<text>mergertree (_, _ )</text--->"
  "<text>Merge Two RTrees (stored in the same file) </text--->"
  "<text>query mergertree(rtree_1,rtree_2)</text--->"
  "<text></text--->"
  ") )";*/

const string MergertreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(rtree1<d> (tuple ((x1 t1)...(xn tn))) ti) x \n"
  "(rtree2<d> (tuple ((x1 t1)...(xn tn))) ti) x "
  " -> (stream (tuple ((x1 t1)...(xn tn))) ti)</text--->"
  "<text>mergertree (_, _ )</text--->"
  "<text>Merge Two RTrees (stored in the same file) and "
  "outputs the root node tuple</text--->"
  "<text>query mergertree(rtree_1,rtree_2) consume</text--->"
  "<text></text--->"
  ") )";

const string MergeCovSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(rtree1<d> (tuple ((x1 t1)...(xn tn))) ti) x \n"
  "(rel1 (tuple ((x1 t1)...(xn tn)))) x (rel2 (tuple ((x1 t1)...(xn tn)))) x"
  "(btree1 (tuple ((x1 t1)...(xn tn)))  ti) x"
  "(btree2 (tuple ((x1 t1)...(xn tn))) ti)"
  " -> (stream (tuple ((x1 t1)...(xn tn)))) </text--->"
  "<text>mergecov (_, _,_,_,_ )</text--->"
  "<text>Merge Two Coverage Numbers </text--->"
  "<text>query mergecov(rtree_1,rel1,rel2,btree1,btree2) count</text--->"
  "<text></text--->"
  ") )";

const string MergeCov2Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(rtree1<d> (tuple ((x1 t1)...(xn tn))) ti) x \n"
  "(rel1 (tuple ((x1 t1)...(xn tn)))) x (rel2 (tuple ((x1 t1)...(xn tn)))) x"
  "(btree1 (tuple ((x1 t1)...(xn tn))) ti) x"
  "(btree2 (tuple ((x1 t1)...(xn tn))) ti)"
  " -> (stream (tuple ((x1 t1)...(xn tn)))) </text--->"
  "<text>mergecov2(_,_,_)</text--->"
  "<text>Update Coverage Numbers in some nodes in the rtree </text--->"
  "<text>query mergecov2(rtree_1,rel,btree) count</text--->"
  "<text></text--->"
  ") )";

/*
TypeMap fun for operator mergertree

*/
/*ListExpr MergeRTreeTypeMap(ListExpr args)
{

// check number of parameters
  if( nl->IsEmpty(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expecting exactly 2 arguments.");
  }

/////////////////////////////////////////////////////////
  ListExpr firstpara = nl->First(args);
  if(nl->ListLength(firstpara) != 4){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!listutils::isRTreeDescription(firstpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }


  if(!(nl->IsEqual(nl->First(firstpara),RTree2TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
///////////////////////////////////////////////////////////

  ListExpr secondpara = nl->Second(args);
  if(nl->ListLength(secondpara) != 4){
    string err = "rtree(tuple(...) rect BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!listutils::isRTreeDescription(secondpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!(nl->IsEqual(nl->First(firstpara),"rtree2") ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
    return nl->First(args);
}*/

/*
TypeMap fun for operator mergertree

*/
ListExpr MergeRTreeTypeMap(ListExpr args)
{

// check number of parameters
  if( nl->IsEmpty(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expecting exactly 2 arguments.");
  }

/////////////////////////////////////////////////////////
  ListExpr firstpara = nl->First(args);
  if(nl->ListLength(firstpara) != 4){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!listutils::isRTreeDescription(firstpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }


  if(!(nl->IsEqual(nl->First(firstpara),RTree2TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
///////////////////////////////////////////////////////////

  ListExpr secondpara = nl->Second(args);
  if(nl->ListLength(secondpara) != 4){
    string err = "rtree(tuple(...) rect BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
  if(!listutils::isRTreeDescription(secondpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!(nl->IsEqual(nl->First(firstpara),"rtree2") ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
//    return nl->First(args);
    ListExpr reslist = nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->TwoElemList(
            nl->TwoElemList(nl->SymbolAtom("nodeId"),
                            nl->SymbolAtom(CcInt::BasicType())),
            nl->TwoElemList(nl->SymbolAtom("level"),
                            nl->SymbolAtom(CcInt::BasicType()))
          )
        )
      );
    return reslist;
}

/*
TypeMap fun for operator mergecov

*/
ListExpr MergeCovTypeMap(ListExpr args)
{

// check number of parameters
  if( nl->IsEmpty(args) || nl->ListLength(args) != 5){
    return listutils::typeError("Expecting exactly 5 arguments.");
  }

/////////////////////////////////////////////////////////
  ListExpr firstpara = nl->First(args);
  if(nl->ListLength(firstpara) != 4){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!listutils::isRTreeDescription(firstpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }


  if(!(nl->IsEqual(nl->First(firstpara),RTree2TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
///////////////////////////////////////////////////////////

  ListExpr second = nl->Second(args);
  ListExpr third = nl->Third(args);
  ListExpr fourth = nl->Fourth(args);
  ListExpr fifth = nl->Fifth(args);
  if(!listutils::isRelDescription(second) ||
     !listutils::isRelDescription(third) ||
     !listutils::isBTreeDescription(fourth) ||
     !listutils::isBTreeDescription(fifth)){
    string err = "rtree x rel1 x rel2 x btree1 x btree2 expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
   ListExpr res = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
           nl->Second(nl->Third(args)));
   return res;

/*   return nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("RecId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom(UInt::BasicType())
                    ))));*/
}

/*
TypeMap fun for operator mergecov2

*/
ListExpr MergeCov2TypeMap(ListExpr args)
{

// check number of parameters
  if( nl->IsEmpty(args) || nl->ListLength(args) != 3){
    return listutils::typeError("Expecting exactly 3 arguments.");
  }

/////////////////////////////////////////////////////////
  ListExpr firstpara = nl->First(args);
  if(nl->ListLength(firstpara) != 4){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!listutils::isRTreeDescription(firstpara)){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }


  if(!(nl->IsEqual(nl->First(firstpara),RTree2TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree3TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree4TID::BasicType()) ||
     nl->IsEqual(nl->First(firstpara),RTree8TID::BasicType()))){
    string err = "rtree(tuple(...) rect3 BOOL) expected";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
///////////////////////////////////////////////////////////

  ListExpr second = nl->Second(args);
  ListExpr third = nl->Third(args);
  if(!listutils::isRelDescription(second) ||
     !listutils::isBTreeDescription(third)){
    string err = "rtree x rel x btree";
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }
   ListExpr res = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
           nl->Second(nl->Second(args)));
   return res;

/*   return nl->TwoElemList(
            nl->SymbolAtom(Symbol::STREAM()),
            nl->TwoElemList(
                nl->SymbolAtom(Tuple::BasicType()),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("RecId"),
                        nl->SymbolAtom(CcInt::BasicType())
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom(UInt::BasicType())
                    ))));*/
}

/*
Merge two input r-trees into one r-tree and record in the same file

*/
/*int MergeRTreeFun(Word* args, Word& result, int message,Word& local,
Supplier s)
{
  R_Tree<3,TupleId>* rtree_in1 = static_cast<R_Tree<3,TupleId>*>(args[0].addr);
  R_Tree<3,TupleId>* rtree_in2 = static_cast<R_Tree<3,TupleId>*>(args[1].addr);

  R_Tree<3,TupleId>* rtree_temp = (R_Tree<3,TupleId>*)qp->ResultStorage(s).addr;
  rtree_temp->CloseFile();

  result = qp->ResultStorage(s);
  R_Tree<3, TupleId> *rtree = new R_Tree<3,TupleId>(rtree_in1->FileId(),true);
  rtree->MergeRtree(rtree_in1,rtree_in2);
  result.setAddr(rtree);
  return 0;
}*/

int MergeRTreeFun(Word* args, Word& result, int message,Word& local,
Supplier s)
{
  static int flag = 0;
  switch(message){
    case OPEN:
      return 0;
    case REQUEST:
      if(flag == 0){
        R_Tree<3,TupleId>* rtree_in1 =
                      static_cast<R_Tree<3,TupleId>*>(args[0].addr);
        rtree_in1->MergeRtree();
        Tuple* t = new Tuple(nl->Second(GetTupleResultType(s)));
        t->PutAttribute(0,new CcInt(true,rtree_in1->RootRecordId()));
        t->PutAttribute(1,new CcInt(true,0));
        result.setAddr(t);
        flag = 1;
        return YIELD;
      }else{
          flag = 0;
          return CANCEL;
      }
    case CLOSE:

        qp->SetModified(qp->GetSon(s,0));
//        qp->SetModified(qp->GetSon(s,1));
        local.setAddr(Address(0));
        return 0;
  }
  return 0;
}

/*
Merge two input coverage numbers

*/
struct Cov{
  R_Tree<3,TupleId>* rtree;
  Relation* cov1;
  Relation* cov2;
  BTree* btree1;
  BTree* btree2;
  TupleType* tupletype;
  int index1;
  int index2;
  int index3;
  bool first;
  bool second;
  bool third;
  Relation* cov3;
  Cov( R_Tree<3,TupleId>* rt, Relation* r1,Relation* r2,
      BTree* bt1,BTree* bt2,ListExpr tt)
      :rtree(rt),cov1(r1),cov2(r2),btree1(bt1),btree2(bt2)
  {
    index1 = index2 = index3 = 1;
    first = true;
    second = true;
    third = false;
    tupletype = new TupleType(tt);
    cov3 = NULL;
  }
  Cov( R_Tree<3,TupleId>* rt, Relation* r1, BTree* bt1,ListExpr tt)
      :rtree(rt),cov1(r1),cov2(NULL),btree1(bt1),btree2(NULL)
  {
    index3 = 1;
    third = false;
    tupletype = new TupleType(tt);
    cov3 = NULL;
  }

  ~Cov()
  {
    tupletype->DeleteIfAllowed();
    tupletype = NULL;
    if(cov3 != NULL)
      delete cov3;
  }
  void CalCov()
  {
    cov3 = new Relation(tupletype,true);
//    rtree->MergeCov(cov1,cov2,cov3,btree1,btree2);
    const int dim = 3;
    SmiRecordId header_path_rec_id = rtree->Record_Path_Id();
    R_TreeNode<dim,TupleId>* node = rtree->GetMyNode(header_path_rec_id,
                    false, rtree->MinEntries(0),rtree->MaxEntries(0));

    for(int i = 0;i < node->EntryCount();i++){
      R_TreeInternalEntry<dim> e =
            (R_TreeInternalEntry<dim>&)(*node)[i];
//      cout<<"rec_id "<<e.pointer<<endl;
      CcInt* id = new CcInt(true,e.pointer);
      BTreeIterator* iter1 = btree1->ExactMatch(id);
      BTreeIterator* iter2 = btree2->ExactMatch(id);
      bool flag1 = false;
      bool flag2 = false;
      //get coverage from the first relation
      //delete them from the relation and create new ones into it
      int NodeId1;

      while(iter1->Next()){
        flag1 = true;
        Tuple* tuple1 = cov1->GetTuple(iter1->GetId(), false);
        NodeId1 = ((CcInt*)tuple1->GetAttribute(0))->GetIntval();
//        assert(cov1->DeleteTuple(tuple1));
        tuple1->DeleteIfAllowed();
      }

      //calculate the new coverage tuple and insert back to the relation
      if(flag1){
        R_TreeNode<dim,TupleId>* n = rtree->GetMyNode(e.pointer,false,
                        rtree->MinEntries(0),rtree->MaxEntries(0));
        MInt tmp(0);
        for(int j = 0;j < n->EntryCount();j++){
            R_TreeInternalEntry<dim> entry =
              (R_TreeInternalEntry<dim>&)(*n)[j];
            CcInt* cur_id = new CcInt(true,entry.pointer);
            BTreeIterator* iter1_1 = btree1->ExactMatch(cur_id);
            MInt tmp1(0);
            while(iter1_1->Next()){
                Tuple* tuple1 = cov1->GetTuple(iter1_1->GetId(), false);
                UInt* ui = (UInt*)tuple1->GetAttribute(2);
                tmp1.Add(*ui);
                tuple1->DeleteIfAllowed();
            }
            delete iter1_1;
            BTreeIterator* iter2_2 = btree2->ExactMatch(cur_id);
            MInt tmp2(0);
            while(iter2_2->Next()){
                Tuple* tuple2 = cov2->GetTuple(iter2_2->GetId(), false);
                UInt* ui = (UInt*)tuple2->GetAttribute(2);
                tmp2.Add(*ui);
                tuple2->DeleteIfAllowed();
            }
            delete iter2_2;
            delete cur_id;
//            tmp1.Print(cout);
//            tmp2.Print(cout);
            MInt temp(0);
            tmp.PlusExtend(&tmp1,temp);
            tmp.CopyFrom(&temp);
            tmp.PlusExtend(&tmp2,temp);
            tmp.CopyFrom(&temp);
        }
//        tmp.Print(cout);
        MInt result(0);
        tmp.Hat(result);
//        result.Print(cout);
        for(int i = 0;i < result.GetNoComponents();i++){
          UInt ui;
          result.Get(i,ui);
          Tuple* resTuple = new Tuple(cov3->GetTupleType());
          CcInt* ni = new CcInt(true,NodeId1);
          resTuple->PutAttribute(0,ni);
          CcInt* ri = new CcInt(true,e.pointer);
          resTuple->PutAttribute(1,ri);
          resTuple->PutAttribute(2,new UInt(ui));
          cov3->AppendTuple(resTuple);
//          cout<<*resTuple<<endl;
          resTuple->DeleteIfAllowed();
        }
        delete n;
      }
      delete iter1;

      int NodeId2;
      //get coverage from the second relation
      while(iter2->Next()){
        flag2 = true;
        Tuple* tuple2 = cov2->GetTuple(iter2->GetId(), false);
        NodeId2 = ((CcInt*)tuple2->GetAttribute(0))->GetIntval();
//        assert(cov2->DeleteTuple(tuple2));
        tuple2->DeleteIfAllowed();
      }

      //calculate the new coverage tuple and insert back to the relation
      if(flag2){
          R_TreeNode<dim,TupleId>* n = rtree->GetMyNode(e.pointer,false,
                        rtree->MinEntries(0),rtree->MaxEntries(0));
          MInt tmp(0);
          for(int j = 0;j < n->EntryCount();j++){
              R_TreeInternalEntry<dim> entry =
                (R_TreeInternalEntry<dim>&)(*n)[j];
              CcInt* cur_id = new CcInt(true,entry.pointer);
              BTreeIterator* iter1_1 = btree1->ExactMatch(cur_id);
              MInt tmp1(0);
              while(iter1_1->Next()){
                Tuple* tuple1 = cov1->GetTuple(iter1_1->GetId(), false);
                UInt* ui = (UInt*)tuple1->GetAttribute(2);
                tmp1.Add(*ui);
                tuple1->DeleteIfAllowed();
              }
              delete iter1_1;
              BTreeIterator* iter2_2 = btree2->ExactMatch(cur_id);
              MInt tmp2(0);
              while(iter2_2->Next()){
                Tuple* tuple2 = cov2->GetTuple(iter2_2->GetId(), false);
                UInt* ui = (UInt*)tuple2->GetAttribute(2);
                tmp2.Add(*ui);
                tuple2->DeleteIfAllowed();
              }
              delete iter2_2;
              delete cur_id;
              MInt temp(0);
              tmp.PlusExtend(&tmp1,temp);
              tmp.CopyFrom(&temp);
              tmp.PlusExtend(&tmp2,temp);
              tmp.CopyFrom(&temp);
          }

          MInt result(0);
          tmp.Hat(result);
          for(int i = 0;i < result.GetNoComponents();i++){
            UInt ui;
            result.Get(i,ui);
            Tuple* resTuple = new Tuple(cov3->GetTupleType());
            CcInt* ni = new CcInt(true,NodeId1);
            resTuple->PutAttribute(0,ni);
            CcInt* ri = new CcInt(true,e.pointer);
            resTuple->PutAttribute(1,ri);
            resTuple->PutAttribute(2,new UInt(ui));
            cov3->AppendTuple(resTuple);
            resTuple->DeleteIfAllowed();
          }
          delete n;
      }
      delete iter2;

      //a new node, get its sons from both coverage relation
      if(flag1 == false && flag2 == false){
        R_TreeNode<dim,TupleId>* n = rtree->GetMyNode(e.pointer,false,
                        rtree->MinEntries(0),rtree->MaxEntries(0));
        MInt tmp(0);
        assert(n->EntryCount() == 2);
        for(int j = 0;j < n->EntryCount();j++){
            R_TreeInternalEntry<dim> entry =
              (R_TreeInternalEntry<dim>&)(*n)[j];
            CcInt* cur_id = new CcInt(true,entry.pointer);
            BTreeIterator* iter1_1 = btree1->ExactMatch(cur_id);
            MInt tmp1(0);
            while(iter1_1->Next()){
                Tuple* tuple1 = cov1->GetTuple(iter1_1->GetId(), false);
                UInt* ui = (UInt*)tuple1->GetAttribute(2);
                tmp1.Add(*ui);
                tuple1->DeleteIfAllowed();
            }
            delete iter1_1;
            BTreeIterator* iter2_2 = btree2->ExactMatch(cur_id);
            MInt tmp2(0);
            while(iter2_2->Next()){
                Tuple* tuple2 = cov2->GetTuple(iter2_2->GetId(), false);
                UInt* ui = (UInt*)tuple2->GetAttribute(2);
                tmp2.Add(*ui);
                tuple2->DeleteIfAllowed();
            }
            delete iter2_2;
            delete cur_id;
//            tmp1.Print(cout);
//            tmp2.Print(cout);
            MInt temp(0);
            tmp.PlusExtend(&tmp1,temp);
            tmp.CopyFrom(&temp);
            tmp.PlusExtend(&tmp2,temp);
            tmp.CopyFrom(&temp);
        }
//        tmp.Print(cout);
        MInt result(0);
        tmp.Hat(result);
//        result.Print(cout);
        for(int i = 0;i < result.GetNoComponents();i++){
          UInt ui;
          result.Get(i,ui);
          Tuple* resTuple = new Tuple(cov3->GetTupleType());
          CcInt* ni = new CcInt(true,e.pointer);
          resTuple->PutAttribute(0,ni);
          CcInt* ri = new CcInt(true,e.pointer);
          resTuple->PutAttribute(1,ri);
          resTuple->PutAttribute(2,new UInt(ui));
          cov3->AppendTuple(resTuple);
//          cout<<*resTuple<<endl;
          resTuple->DeleteIfAllowed();
        }
        delete n;
      }

      delete id;
      assert(flag1 == false || flag2 == false);
  }

  delete node;

//    cout<<"cov3 no_of_tuples "<<cov3->GetNoTuples()<<endl;
    if(cov3->GetNoTuples() > 0)
      third = true;

  }

  void CalCov2()
  {
    int attr_pos = 2;
    cov3 = new Relation(tupletype,true);
//    rtree->MergeCov(cov1,cov2,cov3,btree1,btree2);
    const int dim = 3;
    SmiRecordId header_path_rec_id = rtree->Record_Path_Id();
    R_TreeNode<dim,TupleId>* node = rtree->GetMyNode(header_path_rec_id,
                    false, rtree->MinEntries(0),rtree->MaxEntries(0));

    for(int i = 0;i < node->EntryCount();i++){
      R_TreeInternalEntry<dim> e =
            (R_TreeInternalEntry<dim>&)(*node)[i];
//      cout<<"rec_id "<<e.pointer<<endl;
      CcInt* id = new CcInt(true,e.pointer);
      BTreeIterator* iter1 = btree1->ExactMatch(id);

      bool flag1 = false;
      //get coverage from the first relation
      //delete them from the relation and create new ones into it
      int NodeId1;

      while(iter1->Next()){
        flag1 = true;
        Tuple* tuple1 = cov1->GetTuple(iter1->GetId(), false);
        NodeId1 = ((CcInt*)tuple1->GetAttribute(0))->GetIntval();
//        assert(cov1->DeleteTuple(tuple1));
        tuple1->DeleteIfAllowed();
      }

      //calculate the new coverage tuple and insert back to the relation
      if(flag1){
        R_TreeNode<dim,TupleId>* n = rtree->GetMyNode(e.pointer,false,
                        rtree->MinEntries(0),rtree->MaxEntries(0));
        MInt tmp(0);
        for(int j = 0;j < n->EntryCount();j++){
            R_TreeInternalEntry<dim> entry =
              (R_TreeInternalEntry<dim>&)(*n)[j];
            CcInt* cur_id = new CcInt(true,entry.pointer);
            BTreeIterator* iter1_1 = btree1->ExactMatch(cur_id);
            MInt tmp1(0);
            while(iter1_1->Next()){
                Tuple* tuple1 = cov1->GetTuple(iter1_1->GetId(), false);
//                cout<<"before "<<*tuple1<<endl;
                UInt* ui = (UInt*)tuple1->GetAttribute(2);
                tmp1.Add(*ui);
                tuple1->DeleteIfAllowed();
            }
            delete iter1_1;
            delete cur_id;
//            tmp1.Print(cout);
//            tmp2.Print(cout);
            MInt temp(0);
            tmp.PlusExtend(&tmp1,temp);
            tmp.CopyFrom(&temp);
        }
//        tmp.Print(cout);
        MInt result(0);
        tmp.Hat(result);
//        result.Print(cout);

    ///////////////////Update the Attribute////////////////////
      BTreeIterator* iter2 = btree1->ExactMatch(id);
      while(iter2->Next()){
        Tuple* tuple1 = cov1->GetTuple(iter2->GetId(), false);
//        cout<<"before "<<endl;
        UInt* ui = (UInt*)tuple1->GetAttribute(2);
        vector<int> xindices;
        vector<Attribute*> xattrs;
        xindices.push_back(attr_pos);
        UInt* newui = new UInt(*ui);
        newui->constValue.Set(-1);
        xattrs.push_back(new UInt(*newui));
        delete newui;
        cov1->UpdateTuple(tuple1,xindices,xattrs);
//        cout<<"after "<<*tuple1<<endl;
      //////////////////////////////////////////////////////////
        tuple1->DeleteIfAllowed();
      }
      delete iter2;


        for(int i = 0;i < result.GetNoComponents();i++){
          UInt ui;
          result.Get(i,ui);
          Tuple* resTuple = new Tuple(cov3->GetTupleType());
          CcInt* ni = new CcInt(true,NodeId1);
          resTuple->PutAttribute(0,ni);
          CcInt* ri = new CcInt(true,e.pointer);
          resTuple->PutAttribute(1,ri);
          resTuple->PutAttribute(2,new UInt(ui));
          cov3->AppendTuple(resTuple);
//          cout<<"newresult "<<*resTuple<<endl;
          resTuple->DeleteIfAllowed();
        }
        delete n;
      }
      delete iter1;


      //a new node, get its sons from both coverage relation
      if(flag1 == false){
        R_TreeNode<dim,TupleId>* n = rtree->GetMyNode(e.pointer,false,
                        rtree->MinEntries(0),rtree->MaxEntries(0));
        MInt tmp(0);
//        assert(n->EntryCount() == 2);/////
        assert(1 <= n->EntryCount() && n->EntryCount() <= 2);
        for(int j = 0;j < n->EntryCount();j++){
            R_TreeInternalEntry<dim> entry =
              (R_TreeInternalEntry<dim>&)(*n)[j];
            CcInt* cur_id = new CcInt(true,entry.pointer);
            BTreeIterator* iter1_1 = btree1->ExactMatch(cur_id);
            MInt tmp1(0);
            while(iter1_1->Next()){
                Tuple* tuple1 = cov1->GetTuple(iter1_1->GetId(), false);
                UInt* ui = (UInt*)tuple1->GetAttribute(2);
                tmp1.Add(*ui);
                tuple1->DeleteIfAllowed();
            }
            delete iter1_1;

            delete cur_id;
//            tmp1.Print(cout);
//            tmp2.Print(cout);
            MInt temp(0);
            tmp.PlusExtend(&tmp1,temp);
            tmp.CopyFrom(&temp);
        }
//        tmp.Print(cout);
        MInt result(0);
        tmp.Hat(result);
//        result.Print(cout);
        for(int i = 0;i < result.GetNoComponents();i++){
          UInt ui;
          result.Get(i,ui);
          Tuple* resTuple = new Tuple(cov3->GetTupleType());
          CcInt* ni = new CcInt(true,e.pointer);
          resTuple->PutAttribute(0,ni);
          CcInt* ri = new CcInt(true,e.pointer);
          resTuple->PutAttribute(1,ri);
          resTuple->PutAttribute(2,new UInt(ui));
          cov3->AppendTuple(resTuple);
//          cout<<*resTuple<<endl;
          resTuple->DeleteIfAllowed();
        }
        delete n;
      }

      delete id;

  }

  delete node;

//    cout<<"cov3 no_of_tuples "<<cov3->GetNoTuples()<<endl;
    if(cov3->GetNoTuples() > 0)
      third = true;

  }
};

/*
Merge two input coverage numbers

*/

int MergeCovFun(Word* args, Word& result, int message,Word& local,
Supplier s)
{
  switch(message){
    case OPEN:{
//      cout<<"Open"<<endl;
      if(local.addr)
        delete static_cast<Cov*>(local.addr);
      ListExpr resultType =
    SecondoSystem::GetCatalog()->NumericType(qp->GetType(s));

   R_Tree<3,TupleId>* rtree_in = static_cast<R_Tree<3,TupleId>*>(args[0].addr);

      Relation* cov1 = (Relation*)args[1].addr;
      Relation* cov2 = (Relation*)args[2].addr;
      BTree* btree1 = (BTree*)args[3].addr;
      BTree* btree2 = (BTree*)args[4].addr;
      local.addr =
        new Cov(rtree_in,cov1,cov2,btree1,btree2,nl->Second(resultType));
      Cov* cov = static_cast<Cov*>(local.addr);
      cov->CalCov();
      return 0;

//  R_Tree<3, TupleId> *rtree = new R_Tree<3,TupleId>(rtree_in1->FileId(),true);
//  rtree->MergeRtree(rtree_in1,rtree_in2);

    }
    case REQUEST:{
//      cout<<"request"<<endl;
      Cov* cov = static_cast<Cov*>(local.addr);
      if(cov->first){ //copy the first coverage number rel
          assert(cov->index1 <= cov->cov1->GetNoTuples());
          Tuple* tuple = cov->cov1->GetTuple(cov->index1, false);
          result.addr = tuple;
          cov->index1++;
          if(cov->index1 > cov->cov1->GetNoTuples())
             cov->first = false;
          return YIELD;
      }
      if(cov->second){
          assert(cov->index2 <= cov->cov2->GetNoTuples());
          Tuple* tuple = cov->cov2->GetTuple(cov->index2, false);
          result.addr = tuple;
          cov->index2++;
          if(cov->index2 > cov->cov2->GetNoTuples())
             cov->second = false;
          return YIELD;
      }
      if(cov->third){
          assert(cov->index3 <= cov->cov3->GetNoTuples());
          Tuple* tuple = cov->cov3->GetTuple(cov->index3, false);
          result.addr = tuple;
          cov->index3++;
          if(cov->index3 > cov->cov3->GetNoTuples())
             cov->third = false;
          return YIELD;
      }

      return CANCEL;
    }
    case CLOSE:{
      Cov* cov = static_cast<Cov*>(local.addr);
      if(cov){
        delete cov;
        local.setAddr(NULL);
      }
      return 0;
    }
    default: assert(false);
  }
}

/*
Merge two input coverage numbers

*/

int MergeCov2Fun(Word* args, Word& result, int message,Word& local,
Supplier s)
{
  switch(message){
    case OPEN:{
//      cout<<"Open"<<endl;
      if(local.addr)
        delete static_cast<Cov*>(local.addr);
      ListExpr resultType =
    SecondoSystem::GetCatalog()->NumericType(qp->GetType(s));

   R_Tree<3,TupleId>* rtree_in = static_cast<R_Tree<3,TupleId>*>(args[0].addr);

      Relation* cov1 = (Relation*)args[1].addr;

      BTree* btree1 = (BTree*)args[2].addr;
      local.addr =
        new Cov(rtree_in,cov1,btree1,nl->Second(resultType));
      Cov* cov = static_cast<Cov*>(local.addr);
      cov->CalCov2();
      return 0;

//  R_Tree<3, TupleId> *rtree = new R_Tree<3,TupleId>(rtree_in1->FileId(),true);
//  rtree->MergeRtree(rtree_in1,rtree_in2);

    }
    case REQUEST:{
//      cout<<"request"<<endl;
      Cov* cov = static_cast<Cov*>(local.addr);
      if(cov->third){
          assert(cov->index3 <= cov->cov3->GetNoTuples());
          Tuple* tuple = cov->cov3->GetTuple(cov->index3, false);
          result.addr = tuple;
          cov->index3++;
          if(cov->index3 > cov->cov3->GetNoTuples())
             cov->third = false;
          return YIELD;
      }

      return CANCEL;
    }
    case CLOSE:{
      Cov* cov = static_cast<Cov*>(local.addr);
      if(cov){
        delete cov;
        local.setAddr(NULL);
      }
      return 0;
    }
    default: assert(false);
  }
}
Operator mergertree(
        "mergertree",
        MergertreeSpec,
        MergeRTreeFun,
        Operator::SimpleSelect,
        MergeRTreeTypeMap
);


Operator mergecov(
        "mergecov",
        MergeCovSpec,
        MergeCovFun,
        Operator::SimpleSelect,
        MergeCovTypeMap
);

Operator mergecov2(
        "mergecov2",
        MergeCov2Spec,
        MergeCov2Fun,
        Operator::SimpleSelect,
        MergeCov2TypeMap
);

/*
4 Implementation of the Algebra Class

*/

class NearestNeighborAlgebra : public Algebra
{
 public:
  NearestNeighborAlgebra() : Algebra()
  {

/*
5 Registration of Operators

*/
    AddOperator( &distancescan );
    AddOperator( &distancescan2 );
    AddOperator( &distancescan2S );
    AddOperator( &distancescan3 );
    AddOperator( &knearest );
    AddOperator( &knearestvector );
    AddOperator( &oldknearestfilter );
    AddOperator( &bboxes );
    AddOperator( &rect2periods );
    AddOperator( &coverageop );
    AddOperator( &coverage2op );
    AddOperator( &knearestfilter);
    AddOperator( &distancescan4);
    AddOperator( &greeceknearest);
    AddOperator( &chinaknearest);
//    AddOperator( &covleafnode);
    AddOperator( &cellindex);
//    AddOperator( &gnuplotnode);
    AddOperator( &isknn);
    AddOperator( &mergertree);
//    AddOperator( &mergecov);
    AddOperator( &mergecov2);

    AddOperator( &knearest_dist);//return the k th nearghbor distance
  }
  ~NearestNeighborAlgebra() {};
};

/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>".

To link the algebra together with the system you must create an
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/

} // end of namespace ~near~

extern "C"
Algebra*
InitializeNearestNeighborAlgebra( NestedList* nlRef,
                               QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name
  return new near::NearestNeighborAlgebra;
}

/*
8 Examples and Tests

The file "NearestNeighbor.examples" contains for every operator one example.
This allows one to verify that the examples are running and to provide a
coarse regression test for all algebra modules. The command "Selftest <file>"
will execute the examples. Without any arguments, the examples for all active
algebras are executed. This helps to detect side effects, if you have touched
central parts of Secondo or existing types and operators.

In order to setup more comprehensive automated test procedures one can write a
test specification for the ~TestRunner~ application. You will find the file
"example.test" in directory "bin" and others in the directory "Tests/Testspecs".
There is also one for this algebra.

Accurate testing is often treated as an unpopular daunting task. But it is
absolutely inevitable if you want to provide a reliable algebra module.

Try to write tests covering every signature of your operators and consider
special cases, as undefined arguments, illegal argument values and critical
argument value combinations, etc.


*/

