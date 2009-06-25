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

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;
double difftimeb( struct timeb* t1, struct timeb* t2 )
{
  double dt1 = t1->time + (double)t1->millitm / 1000.0;
  double dt2 = t2->time + (double)t2->millitm / 1000.0;
  return dt1 - dt2;
}

//#define mydebug 1

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
  if(!nl->IsEqual(quantity,"int")){
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
          ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, "rect") ) ||
          ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
            algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, "rect") &&
            nl->IsEqual(searchWindow, "rect") ) ||
          ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
             nl->IsEqual(searchWindow, "rect3") ) ||
          ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
            algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, "rect3") &&
            nl->IsEqual(searchWindow, "rect3") ) ||
          ( algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, "rect4") ) ||
          ( algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo) &&
            algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, "rect4") &&
            nl->IsEqual(searchWindow, "rect4") ) ||
          ( algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo) &&
            nl->IsEqual(searchWindow, "rect8") ) ||
          ( algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo) &&
            algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo) ) ||
          ( nl->IsEqual(rtreeKeyType, "rect8") &&
            nl->IsEqual(searchWindow, "rect8") ))){
    ErrorReporter::ReportError(errmsg + "(different dimensions)");
    return nl->TypeError();
   }
   return
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->Second(relDescription));
}

ListExpr
distanceScan3TypeMap( ListExpr args ) {

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

  if( (rt != "point") && (rt != "points") &&
      (rt != "line") && (rt != "region") &&
      (rt != "rect")){
    ErrorReporter::ReportError("Unsupported type within the rtree");
    return nl->TypeError();
  }
  if( (qt != "point") && (qt != "points") &&
      (qt != "line") && (qt != "region") &&
      (qt != "rect")){
    ErrorReporter::ReportError("Unsupported type for query object");
    return nl->TypeError();
  }
  return nl->ThreeElemList(
               nl->SymbolAtom("APPEND"),
               nl->OneElemList( nl->IntAtom(j-1)),
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
  if(!nl->IsEqual(nl->First(nl->First(args)),"rtree3")){
    ErrorReporter::ReportError(err + " (invalid rtree  dimension)");
    return listutils::typeError();
  }

  if(!listutils::isRelDescription(nl->Second(args))){
    ErrorReporter::ReportError(err + " (invalid relation)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Third(args),"point")){
    ErrorReporter::ReportError(err + " (invalid point)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Fourth(args),"instant")){
    ErrorReporter::ReportError(err + " (invalid instant)");
    return listutils::typeError();
  }
  if(!nl->IsEqual(nl->Fifth(args),"int")){
    ErrorReporter::ReportError(err + " (invalid int)");
    return listutils::typeError();
  }

  // relation and rtree must have the same tuple type
  if(!nl->Equal(nl->Second(nl->First(args)), nl->Second(nl->Second(args)))){
    ErrorReporter::ReportError(err + " (different type of rtree and rel)");
    return listutils::typeError();
  }
  ListExpr rtreekey = listutils::getRTreeType(nl->First(args));
  if(!nl->IsEqual(rtreekey,"upoint")){
    ErrorReporter::ReportError(err + " (rtree not build on upoint)");
    return listutils::typeError();
  }

  ListExpr attrList = nl->Second(nl->Second(nl->Second(args)));
  ListExpr res1 = nl->TwoElemList(nl->SymbolAtom("stream"),
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
    if(!nl->IsEqual(type,"upoint") ){
      ErrorReporter::ReportError(err + " (attribute not a upoint)");
      return listutils::typeError();
    }
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                             nl->OneElemList(nl->IntAtom(j)),
                             res1);
  } else { // length ==5
    string name;
    int j = listutils::findType(attrList, nl->SymbolAtom("upoint"),name);
    if(!j){
       ErrorReporter::ReportError(err + " (no upoint stored in relation)");
       return listutils::typeError();
    }
    int j2 = listutils::findType(attrList,
                                 nl->SymbolAtom("upoint"),
                                 name,
                                 j+1);
    if(j2){
       ErrorReporter::ReportError(err + " (upoint stored twice in relation)");
       return listutils::typeError();
    }
    return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                             nl->TwoElemList(nl->IntAtom(j), nl->IntAtom(j)),
                             res1);
  }
}



/*
The function knearestTypeMap is the type map for the operator knearest

*/
ListExpr
knearestTypeMap( ListExpr args )
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

  if(!nl->IsEqual(mpoint,"mpoint")){
    ErrorReporter::ReportError(msg+" (third arg is not an mpoint)");
    return nl->TypeError();
  }

  if(!nl->IsEqual(card,"int")){
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

  if(!nl->IsEqual(attrType,"upoint")){
    ErrorReporter::ReportError(msg+" (attrName does not refer to an upoint)");
    return nl->TypeError();
  }


  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(nl->IntAtom(j)),
      stream);
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
  if(!nl->IsEqual(mpoint,"mpoint")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,"int")){
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

  if(!nl->IsEqual(rtreeSymbol, "rtree3")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the keytype of the rtree must be of kind SPATIAL3D
  if(!algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
     !nl->IsEqual(rtreeKeyType,"rect3")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(nl->ListLength(rtreeTupleDescription)!=2){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!nl->IsEqual(nl->First(rtreeTupleDescription),"tuple")){
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
      nl->SymbolAtom("stream"),
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
  if(!nl->IsEqual(arg,"rect3")){
    ErrorReporter::ReportError("rect3 expected");
    return nl->TypeError();
  }
  return nl->SymbolAtom("periods");
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
  ListExpr searchWindow = nl->Third(args),
           errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  if (nl->SymbolValue(searchWindow) == "rect" ||
      algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo))
    return 0;
  else if (nl->SymbolValue(searchWindow) == "rect3" ||
           algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo))
    return 1;
  else if (nl->SymbolValue(searchWindow) == "rect4" ||
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 2;
  else if (nl->SymbolValue(searchWindow) == "rect8" ||
           algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
}

int distanceScan3Select(ListExpr args){
   // type indxed by the rtree
   string t1 = nl->SymbolValue(nl->Third(nl->First(args)));
   // typr of query object
   string t2 = nl->SymbolValue(nl->Third(args));

   if(t1=="point"){
     if(t2=="point") return 0;
     if(t2=="points") return 1;
     if(t2=="line") return 2;
     if(t2=="region") return 3;
     if(t2=="rect") return 4;
   }
   if(t1=="points"){
     if(t2=="point") return 5;
     if(t2=="points") return 6;
     if(t2=="line") return 7;
     if(t2=="region") return 8;
     if(t2=="rect") return 9;
   }
   if(t1=="line"){
     if(t2=="point") return 10;
     if(t2=="points") return 11;
     if(t2=="line") return 12;
     if(t2=="region") return 13;
     if(t2=="rect") return 14;
   }
   if(t1=="region"){
     if(t2=="point") return 15;
     if(t2=="points") return 16;
     if(t2=="line") return 17;
     if(t2=="region") return 18;
     if(t2=="rect") return 19;
   }
   if(t1=="rect"){
     if(t2=="point") return 20;
     if(t2=="points") return 21;
     if(t2=="line") return 22;
     if(t2=="region") return 23;
     if(t2=="rect") return 24;
   }
   return 0;
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
          Tuple *tuple = localInfo->relation->GetTuple(tid);
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
      localInfo = (DistanceScanLocalInfo<dim, TupleId>*)local.addr;
      localInfo->rtree->LastDistanceScan();
      delete localInfo;
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
     Tuple* res = rel->GetTuple(top->getTupleId());
     delete top;
     return res;
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

       Tuple* tuple = relation->GetTuple(id);
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
                QueryType* queryObj, CcInt* k, int position){
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
     // now we have a tuple
     returned++;
     Tuple* res = rel->GetTuple(top->getTupleId());
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
       int index = qp->GetNoSons(s)-1;
       int pos = (static_cast<CcInt*>(args[index].addr))->GetIntval();
       local.addr = new
           DS3LocalInfo<dim,IndexedType,QueryType, DistFun>(rtree,
                                                rel, queryObj, k, pos);
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

       Tuple* tuple = relation->GetTuple(id);
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
     Tuple* res = rel->GetTuple(top->getTupleId());
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
bool GetPosition( const UPoint* up, Instant t, Coord& x, Coord& y)
{
  //calculates the pos. at time t. x and y is the result
  Instant t0 = up->timeInterval.start;
  Instant t1 = up->timeInterval.end;
  if( t == t0 )
  {
    x = up->p0.GetX();
    y = up->p0.GetY();
    return true;
  }
  if( t == t1 )
  {
    x = up->p1.GetX();
    y = up->p1.GetY();
    return true;
  }
  if( t < t0 || t > t1 ){ return false;}
  double factor = (t - t0) / (t1 - t0);
  x = up->p0.GetX() + factor * (up->p1.GetX() - up->p0.GetX());
  y = up->p0.GetY() + factor * (up->p1.GetY() - up->p0.GetY());
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
  const UPoint *upn;

  for (int ii=mpos; ii < noCo; ii++)
  {
    mp->Get( ii, upn);  // get the current Unit
    if(time2 < upn->timeInterval.start ||
      time1 > upn->timeInterval.end ||
      (time2 == upn->timeInterval.start && up->timeInterval.rc == false))
      continue;

    if( time1 < upn->timeInterval.end ||
        (time1 == upn->timeInterval.end && upn->timeInterval.rc))
    { // interval of mp intersects the interval of up
      mpos = ii;
      UReal firstu(true);

      // take the bigger starting point
      Instant start = up->timeInterval.start < upn->timeInterval.start
        ? upn->timeInterval.start : up->timeInterval.start;

      // take the smaller end
      Instant end = up->timeInterval.end < upn->timeInterval.end
        ? up->timeInterval.end : upn->timeInterval.end;

      bool lc = up->timeInterval.lc;
      if( lc && (start > up->timeInterval.start || (up->timeInterval.start
        == upn->timeInterval.start && !up->timeInterval.lc)))
      {
        lc = false;
      }
      bool rc = up->timeInterval.rc;
      if( rc && (end < up->timeInterval.end || (up->timeInterval.end
        == upn->timeInterval.end && !upn->timeInterval.rc)))
      {
        rc = false;
      }

      Interval<Instant> iv(start, end, lc, rc);


      Coord x1, y1, x2, y2;
      GetPosition( up, start, x1, y1);
      GetPosition( up, end, x2, y2);
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
        end = up->timeInterval.end < upn->timeInterval.end
        ? up->timeInterval.end : upn->timeInterval.end;
        rc = up->timeInterval.rc;
        if( rc && (end < up->timeInterval.end || (up->timeInterval.end
          == upn->timeInterval.end && !upn->timeInterval.rc)))
        {
          rc = false;
        }
        Interval<Instant> iv(start, end, lc, rc);
        Coord x1, y1, x2, y2;
        GetPosition( up, start, x1, y1);
        GetPosition( up, end, x2, y2);
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
CalcDistance calculate the result of the given MReal mr
at the given time t. The value of the derivation (slope)
is also calculated

*/
double CalcDistance( const MReal *mr, Instant t, double &slope)
{
  int noCo = mr->GetNoComponents();
  const UReal *ur;
  for (int ii=0; ii < noCo; ++ii)
  {
    mr->Get( ii, ur);
    if( t < ur->timeInterval.end
      || (t == ur->timeInterval.end && ur->timeInterval.rc)
      || (t == ur->timeInterval.end && ii+1 == noCo))
    {
      double time = (t - ur->timeInterval.start).ToDouble();
      double erg = ur->a * time * time + ur->b * time + ur->c;
      if( ur->r && erg < 0) erg = 0;
      erg = ( ur->r) ? sqrt(erg) : erg;
      //the slope is if not r: 2a * time + b
      // else 2a * time + b / 2 * erg
      slope = 2 * ur->a * time + ur->b;
      if( ur->r && erg){ slope /= 2 * erg; }
      if( ur->r && !erg && ur->b == 0 && ur->c == 0 && ur->a > 0)
      {
        slope = sqrt(ur->a);
      }
      return erg;
    }
  }
  return -1;
}

double CalcSlope( const UReal *ur, Instant t)
{
  if( t >= ur->timeInterval.start && t <= ur->timeInterval.end )
  {
    double time = (t - ur->timeInterval.start).ToDouble();
    //the slope is if not r: 2a * time + b
    // else 2a * time + b / div
    double erg = 2 * ur->a * time + ur->b;
    if( ur->r)
    {
      double div = ur->a * time * time + ur->b * time + ur->c;
      if( div > 0){
        erg /= 2 * sqrt(div);
      }
      else if(ur->b == 0 && ur->c == 0 && ur->a > 0)
      {
        erg = sqrt(ur->a);
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
  const UReal* u1;
  if( !noCo1 ){ return false; }
  m1->Get( ii, u1);
  while( (start > u1->timeInterval.end || (start == u1->timeInterval.end
    && !u1->timeInterval.rc)) && ++ii < noCo1 )
  {
    m1->Get( ii, u1);
  }
  int noCo2 = m2->GetNoComponents();
  const UReal* u2;
  if( !noCo2 ){ return false; }
  int jj = 0;
  m2->Get( jj, u2);
  while( (start > u2->timeInterval.end || (start == u2->timeInterval.end
    && !u2->timeInterval.rc)) && ++jj < noCo2 )
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
    if( u1->timeInterval.start <= u2->timeInterval.start )
    {
      laterTime = u2->timeInterval.start.ToDouble();;
      x = (u2->timeInterval.start - u1->timeInterval.start).ToDouble();
      a = u1->a - u2->a;
      b = u1->b - u2->b + 2 * u1->a * x;
      c = u1->a * x * x + u1->b * x + u1->c - u2->c;
    }
    else
    {
      laterTime = u1->timeInterval.start.ToDouble();;
      x = (u1->timeInterval.start - u2->timeInterval.start).ToDouble();
      a = u2->a - u1->a;
      b = u2->b - u1->b + 2 * u2->a * x;
      c = u2->a * x * x + u2->b * x + u2->c - u1->c;
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
      if( abs(r1 - r2) < 0.000000001
        && (u1->a==0 && u2->a!=0 || u2->a==0 && u1->a!=0))
      {
        //straight line intersects curve in one point, only boundary point
        r1 = -1;
        r2 = -1;
      }
      if( r1 >= 0 )
      {
        result.ReadFrom(r1 + laterTime);
        if( (result > start || (result == start && !isInsert))
          && result <= u1->timeInterval.end && result <= u2->timeInterval.end)
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
          && result <= u1->timeInterval.end && result <= u2->timeInterval.end)
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
      if( u1->timeInterval.end < u2->timeInterval.end )
      {
        ++ii;
      }
      else if( u2->timeInterval.end < u1->timeInterval.end )
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
    if( result == u1->timeInterval.end )
    {
      hasResult = false;
    }
  }
  if( hasResult )
  {
    //if the intersection exact at the end of m1 or m2, do not take it
    m2->Get( m2->GetNoComponents()-1, u1);
    if( result == u1->timeInterval.end )
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
      GetPosition( upointAttr, start, x1, y1);
      GetPosition( upointAttr, end, x2, y2);
      Interval<Instant> iv( start, end, lc, rc);
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

     if(!mpoint->IsDefined() || mpoint->IsEmpty()){
       lastTuple = 0;
       return;
     }
     const UPoint* tmp;
     mpoint->Get(0,tmp);
     Instant tmpstart = tmp->timeInterval.start;
     bool lc = tmp->timeInterval.lc;
     mpoint->Get(mpoint->GetNoComponents()-1, tmp);
     Instant tmpend = tmp->timeInterval.end;
     bool rc = tmp->timeInterval.rc;

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
       tmp = static_cast<UPoint*>(lastTuple->GetAttribute(pos));
       tupleStart = tmp->timeInterval.start;
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
     next();
     return const_cast<EventElem &>(queue.top());
   }

   void pop(){
     next();
     queue.pop();
   }

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
  unsigned int k;
  bool scanFlag;
  KNearestQueue eventQueue;
  Instant startTime, endTime;
  NNTree< ActiveElem > activeLine;
};

/*
knearestFun is the main function of the operator knearest
The argument vector contains the following values:
args[0] = stream of tuples,
 the attribute given in args[1] has to be a unit
 the operator expects that the tuples are sorted by the time of the units
args[1] = attribute name
args[2] = mpoint
args[3] = int k, how many nearest are searched
args[4] = int j, attributenumber

*/
int knearestFun (Word* args, Word& result, int message,
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
         const UPoint *up1, *up2;
         mp->Get( 0, up1);
         mp->Get( mp->GetNoComponents() - 1, up2);
         localInfo->startTime = up1->timeInterval.start;
         localInfo->endTime = up2->timeInterval.end;
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
                result = SetWord(cloneTuple);

                return YIELD;
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
            if( cloneTuple )
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
            if( cloneTuple )
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
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      localInfo->startTime = up1->timeInterval.start;
      localInfo->endTime = up2->timeInterval.end;
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
      Tuple* tuple = rel->GetTuple(btreeiter->GetId());
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
  TBTree* tbtree;
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
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      BBTree<timeType>* t = new BBTree<timeType>(*mp);

      localInfo = new KnearestFilterLocalInfo<timeType>(
        up1->timeInterval.start.ToDouble(), up2->timeInterval.end.ToDouble());
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
          Tuple *tuple = localInfo->relation->GetTuple(tid);
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

  if(!nl->IsEqual(rtree,"rtree3")){
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
  if(!nl->IsEqual(nl->First(tuple),"tuple")){
    ErrorReporter::ReportError(err + "7");
    return nl->TypeError();
  }
  return result;
}



inline ListExpr coverageTypeMap(ListExpr args){
  return coverageTypeMapCommon(args,
        nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("RecId"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom("uint")
                    )))));
}


inline ListExpr coverage2TypeMap(ListExpr args){
  return coverageTypeMapCommon(args,
        nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->ThreeElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Level"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),
                        nl->SymbolAtom("mint")
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
        delete currentResult;
      }
      tupleType->DeleteIfAllowed();
      tupleType = 0;
      if(completeResult){
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
      const UInt* res;
      currentResult->Get(currentPos,res);
      currentPos++;
      Tuple* resTuple = new Tuple(tupleType);
      CcInt* ni = new CcInt(true,currentNodeId);
      resTuple->PutAttribute(0,ni);
      CcInt* ri = new CcInt(true,currentRecId);
      resTuple->PutAttribute(1,ri);
      resTuple->PutAttribute(2,new UInt(*res));
      return resTuple;
   }

   Tuple* nextTuple2(){
      computeNextResult();
      if(!completeResult){
         return 0;
      } else {
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
       delete currentResult;
       currentResult=0;
     }
     if(completeResult){
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

const string distanceScan3Spec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k  x attrname->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in {point, points, region, rect, rect?},"
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

const string knearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ knearest [_, _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which contains the k-nearest units to the given mpoint. "
      "The tuples are splitted into multiple tuples with disjoint "
      "units if necessary. The tuples in the result stream are "
      "not necessarily ordered by time or distance to the given "
      "mpoint. The operator expects that the input stream with "
      "the tuples are sorted by the time of the units</text--->"
      "<text>query query UnitTrains feed head[20] knearest "
      "[UTrip,train1, 2] consume;</text--->"
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
         knearestFun,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         knearestTypeMap        // type mapping
);

Operator knearestvector (
         "knearestvector",      // name
         knearestSpec,          // specification
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
  if(!nl->IsEqual(mp,symbols::MPOINT)){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(nl->ListLength(stream)!=2){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  if(!nl->IsEqual(nl->First(stream),symbols::STREAM)   ||
     !nl->IsEqual(nl->Second(stream),"periods")){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }
  return nl->TwoElemList(nl->SymbolAtom(symbols::STREAM),
                         nl->SymbolAtom("rect"));

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
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      BBTree<timeType>* t = new BBTree<timeType>(*mp);
      localInfo = new KnearestFilterLocalInfo<timeType>(
        up1->timeInterval.start.ToDouble(), up2->timeInterval.end.ToDouble());
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
                  Tuple* tuple = localInfo->hats->GetTuple(btreeiter->GetId());
                  UInt* ut = (UInt*)tuple->GetAttribute(2);//NodeId, RecId, Uint
                  interv.push_back(ut->timeInterval);
                  covs.push_back(ut->constValue.GetValue());
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
          Tuple *tuple = localInfo->relation->GetTuple(tid);
//split units
          const MPoint *mp = (MPoint*)args[5].addr;//5 th parameter
          const UPoint *up1, *up2;
          mp->Get( 0, up1);
          mp->Get( mp->GetNoComponents() - 1, up2);
          int attrpos = ((CcInt*)args[7].addr)->GetIntval() - 1;
          UPoint* up = (UPoint*)tuple->GetAttribute(attrpos);
          Point p0;
          if(up->timeInterval.Contains(up1->timeInterval.start)){
            up->TemporalFunction(up1->timeInterval.start,p0,true);
            if(p0.IsDefined()){
              up->timeInterval.start = up1->timeInterval.start;
              up->p0 = p0;
            }
          }
          Point p1;
          if(up->timeInterval.Contains(up2->timeInterval.end)){
            up->TemporalFunction(up2->timeInterval.end,p1,true);
            if(p1.IsDefined()){
              up->timeInterval.end = up2->timeInterval.end;
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
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator knearestfilter.";
  string rtreeDescriptionStr, relDescriptionStr;
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 7,
    "Operator knearest expects a list of length four.");

  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr btreeDescription = nl->Third(args);
  ListExpr brelDescription = nl->Fourth(args);
  ListExpr attrName = nl->Fifth(args);
  ListExpr queryobject = nl->Sixth(args);
  ListExpr quantity = nl->Nth(7,args);

  CHECK_COND(listutils::isRTreeDescription(rtreeDescription),
             "first argument must be an rtree");

  CHECK_COND (listutils::isRelDescription(relDescription),
             "Second argument must be a relation");

  CHECK_COND(listutils::isBTreeDescription(btreeDescription),
             "third argument must be an btree");

  CHECK_COND(listutils::isRelDescription(brelDescription),
             "fourth argument must be a relation");

  CHECK_COND(nl->AtomType(attrName)==SymbolType,
             "fifth argument must be an attribute name");

  CHECK_COND(nl->IsEqual(queryobject,"mpoint"),
             "sixth argument must be of type mpoint");

  CHECK_COND(nl->IsEqual(quantity,"int"),
             "seventh argument must be of type int");

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(relDescription)),
      nl->SymbolValue(attrName),attrType);
  CHECK_COND( (j>0) && (nl->IsEqual( attrType, "upoint" )),
  "Operator knearestfilter expects as a fifth argument an attribute"
  "name, where the attribute is of type upoint\n"
  "operator knearestfilter gets '" + argstr + "'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
    nl->AtomType(rtreeKeyType) == SymbolType &&
    (algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
     nl->IsEqual(rtreeKeyType, "rect3")),
   "Operator knearestfilter expects a R-Tree with key type\n"
   "of kind SPATIAL3D\n"
   "or rect3.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
     nl->SymbolValue(rtreeSymbol) == "rtree3" ,
   "Operator knearestfilter expects a R-Tree \n"
   "of type rtree3.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator knearestfilter expects a R-Tree with structure "
    "(rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator knearestfilter expects a R-Tree with structure "
    "(rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
    nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator distancescan expects a R-Tree with structure "
   "(rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
   "bool)\nbut gets a first list with wrong tuple description in "
   "structure \n'"+rtreeDescriptionStr+"'.");

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(!nl->IsAtom(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol) &&
    nl->AtomType(relSymbol) == SymbolType &&
    nl->SymbolValue(relSymbol) == "rel" &&
    !nl->IsEmpty(tupleDescription) &&
    !nl->IsAtom(tupleDescription) &&
    nl->ListLength(tupleDescription) == 2,
    "Operator knearestfilter expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
    nl->AtomType(tupleSymbol) == SymbolType &&
    nl->SymbolValue(tupleSymbol) == "tuple" &&
    IsTupleDescription(attrList),
    "Operator knearestfilter expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  /* check that rtree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList),
   "Operator knearestfilter: The tuple type of the R-tree\n"
   "differs from the tuple type of the relation.");



  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);

  return  nl->ThreeElemList(
             nl->SymbolAtom("APPEND"),
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

const string mqknearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text> datarel(tuple ((x1 t1)...(xn tn))) "
      " x rtree(tuple((x1 t1)...(xn tn)))"
      " x rel(tuple((x1 t1)...(xn tn)))"
      " x btree(tuple ((x1 t1)...(xn tn)))"
      " x queryrel(tuple ((x1 t1)...(xn tn))) x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ _ _ mqknearest[_ _]</text--->"
      "<text>The operator results a stream of all input tuples "
      "where for each point in Points1, returns its k nearest neighbor"
      "in Points2 </text--->"
      "<text>not finished yet;</text--->"
      ") )";

ListExpr mqknearestTypeMap(ListExpr args){
  string err = "Points2 x R-Tree x rel x btree x Points1 x  k expected";
  if(nl->ListLength(args) != 8){
     ErrorReporter::ReportError(err);
     return nl->TypeError();
  }

  ListExpr rtree2 = nl->Second(args);
  string rtreedescription2;
  nl->WriteToString(rtreedescription2,rtree2);
  ListExpr rtsymbol2 = nl->First(rtree2);

  ListExpr k = nl->Nth(8,args);

  if(nl->IsAtom(rtsymbol2) &&
    nl->AtomType(rtsymbol2) == SymbolType &&
    nl->SymbolValue(rtsymbol2) == "rtree" &&
    nl->IsEqual(k,"int")){
    ListExpr reslist =  nl->TwoElemList(
    nl->SymbolAtom("stream"),
    nl->TwoElemList(
      nl->SymbolAtom("tuple"),
      nl->TwoElemList(
      nl->TwoElemList(nl->SymbolAtom("query"),nl->SymbolAtom("point")),
      nl->TwoElemList(nl->SymbolAtom("data"),nl->SymbolAtom("point")))
    ));
    return reslist;
  }
  return nl->TypeError();
}

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
    double dist;
    double num = 0;
    for(unsigned int i = 0;i < array2.size();i++){
      CcInt* id = new CcInt(true,array2[i].nodeid);
      BTreeIterator* iter = mqk->btreecov->ExactMatch(id);
      assert(iter->Next() != false);
      Tuple* tuple = mqk->cov->GetTuple(iter->GetId());
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
  double dist;
  double num = 0;

  for(unsigned int i = 0;i < candidate.size();i++){
      CcInt* id = new CcInt(true,candidate[i].nodeid);
      BTreeIterator* iter = mqk->btreecov->ExactMatch(id);
      assert(iter->Next() != false);
      Tuple* tuple = mqk->cov->GetTuple(iter->GetId());
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
      Tuple* tuple = mqk->datapoints->GetTuple(e.info);
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
  double dist;
  double num = 0;
  for(unsigned int i = 0;i < candidate.size();i++){
    CcInt* id = new CcInt(true,candidate[i].nodeid);
    BTreeIterator* iter = mqk->btreeleafcov->ExactMatch(id);
    assert(iter->Next() != false);
    Tuple* tuple = mqk->leafcov->GetTuple(iter->GetId());
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
      Tuple* tuple = mqk->datapoints->GetTuple(e.info);
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
  Tuple* tuple = mqk->querypoints->GetTuple(1);
  Point* p = (Point*)tuple->GetAttribute(0);
  min[0] = max[0] = p->GetX();
  min[1] = max[1] = p->GetY();
  tuple->DeleteIfAllowed();
  for(;start < blocksize &&
      (start+mqk->index) <= mqk->querypoints->GetNoTuples(); start++){
    Tuple* tuple = mqk->querypoints->GetTuple(start+mqk->index);
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
int mqknearestFun(Word* args, Word& result, int message,
              Word& local, Supplier s){
  MQKnearest* mqk;
  switch(message){
     case OPEN: {
     mqk = new MQKnearest();
     mqk->datapoints = (Relation*)args[0].addr;
     mqk->rtree = (R_Tree<2,TupleId>*)args[1].addr;
     mqk->cov = (Relation*)args[2].addr;
     mqk->btreecov = (BTree*)args[3].addr;
     mqk->leafcov = (Relation*)args[4].addr;//leafnode cov
     mqk->btreeleafcov = (BTree*)args[5].addr;
     mqk->querypoints = (Relation*)args[6].addr;
     mqk->k = (unsigned)((CcInt*)args[7].addr)->GetIntval();
     mqk->resulttype = new TupleType(nl->Second(GetTupleResultType(s)));
     mqk->nextblock = true;  //deal with query objects by block
     mqk->index = 1;  //start position in queryobjects rel
     local = SetWord(mqk);
     return 0;
   }
   case REQUEST: {
      mqk = (MQKnearest*)local.addr;
      if(mqk->nextblock && mqk->results.size() == 0){ //next query block
          Mqknearest(mqk);
      }
      if(mqk->iter == mqk->results.end()){
        mqk->results.clear();
        if(mqk->nextblock == false)
          return CANCEL;
        else
          Mqknearest(mqk);
      }
      Tuple* tuple = new Tuple(mqk->resulttype);
      tuple->PutAttribute(0,new Point(mqk->iter->q));
      tuple->PutAttribute(1,new Point(mqk->iter->p));
      result.setAddr(tuple);
      mqk->iter++;
      return YIELD;
   }
   case CLOSE : {
      mqk = (MQKnearest*)local.addr;
      mqk->resulttype->DeleteIfAllowed();
      delete mqk;
      return 0;
   }
 }
 return 0;
}
Operator mqknearest (
         "mqknearest",        // name
         mqknearestSpec,      // specification
         mqknearestFun,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         mqknearestTypeMap    // type mapping
);
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
    nl->SymbolValue(rtsymbol2) == "rtree")

  return
        nl->TwoElemList(
            nl->SymbolAtom("stream"),
            nl->TwoElemList(
                nl->SymbolAtom("tuple"),
                nl->FiveElemList(
                    nl->TwoElemList(
                        nl->SymbolAtom("Nodeid"),
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//1 quadrant
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//2 quadrant
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//3 quadrant
                        nl->SymbolAtom("int")
                    ),
                    nl->TwoElemList(
                        nl->SymbolAtom("Coverage"),//4 quadrant
                        nl->SymbolAtom("int")
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
      int nodeid;
      while(localInfo->covtid <= localInfo->cov->GetNoTuples()){
        Tuple* tuple = localInfo->cov->GetTuple(localInfo->covtid);
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
        Tuple* tuple = localInfo->data->GetTuple(info);
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

const string RknearestfilterSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>tbtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x rtree(tuple((x1 t1)...(xn tn)))"
      " x rel(tuple ((x1 t1)...(xn tn))) x mpoint x k ->"
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
const string hcknnknearestSpec  =
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
      "<text>query UnitTrains_UTrip_tbtree UnitTrains hcknnknearest"
      "[UTrip,train1, 5] count;</text--->"
      ") )";


/*
rknearestFilterFun is the value function for the rknearestfilter operator
It is a filter operator for the knearest operator. It can be called
if there exists a tb-tree for the unit attribute
The argument vector contains the following values:
args[0] = an rtree with the unit attribute as key
args[1] = the relation of the tbtree
args[2] = mpoint
args[3] = int k, how many nearest are searched

*/

template<class timeType,int dim>
void RHCTNNSearch(R_TreeNode<dim,TupleId>* rnode, MPoint* mp,
KnearestFilterLocalInfo<timeType>* localInfo,BBTree<timeType>* t,int level,
SegEntry<timeType>& se)
{
  if(localInfo->timeTree.erase(se.start,se.end,se.nodeid,se.maxdist) == false)
    return;
  if(rnode->IsLeaf()){ //leaf node
    for(int j = 0;j < rnode->EntryCount();j++){
      R_TreeLeafEntry<dim, TupleId> e =
                  (R_TreeLeafEntry<dim, TupleId>&)(*rnode)[j];
      timeType t1((double)(e.box.MinD(2)));
      timeType t2((double)(e.box.MaxD(2)));

      t1 = t1/864000.0;
      t2 = t2/864000.0;

      if(!(t1 >= localInfo->endTime || t2 <= localInfo->startTime)){
        Interval<timeType> d(t1,t2,true,true);
        BBox<2> entrybox = makexyBox(e.box);
        BBox<2> mBox(t->getBox(d));
        double mindist,maxdist;

        mindist = mBox.Distance(entrybox);
        maxdist = maxDistance(mBox,entrybox);
        SegEntry<timeType> se(entrybox,t1,t2,mindist,
                              maxdist,1,-1,e.info);
        unsigned int cov = localInfo->timeTree.calcCoverage(se.start,
                                      se.end,se.mindist);
        if(cov < localInfo->k){
           localInfo->timeTree.insert(se,localInfo->k);
 //          localInfo->resultMap.insert(make_pair(se,se.tpid));
        }
      }
    }
  }else{
      SmiRecordId adr;
      vector<EFieldEntry<timeType> > candidate;
      for(int j = 0;j < rnode->EntryCount();j++){
          R_TreeInternalEntry<dim> e =
                  (R_TreeInternalEntry<dim>&)(*rnode)[j];
          timeType t1(e.box.MinD(2));
          timeType t2(e.box.MaxD(2));

          t1 = t1/864000.0;
          t2 = t2/864000.0;

          if(!(t1 >= localInfo->endTime || t2 <= localInfo->startTime)){
            adr = e.pointer;
            BBox<2> entrybox = makexyBox(e.box);
            Interval<timeType> d(t1,t2,true,true);
            BBox<2> mBox(t->getBox(d));
            double mindist,maxdist;
            mindist = mBox.Distance(entrybox);
            maxdist = maxDistance(mBox,entrybox);
            candidate.push_back(
            EFieldEntry<timeType>(adr,mindist,maxdist,level+1,t1,t2));
          }
      }
      //sort by mindist
      stable_sort(candidate.begin(),candidate.end(),CmpEFE<timeType>);
      for(unsigned int i = 0;i < candidate.size();i++){
        adr = candidate[i].nodeid;
        R_TreeNode<dim, TupleId> *node = localInfo->rtree->GetMyNode(
                     adr, false,
                     localInfo->rtree->MinEntries( candidate[i].level ),
                     localInfo->rtree->MaxEntries( candidate[i].level ));

        BBox<2> xybox = makexyBox(node->BoundingBox());
        SegEntry<timeType> se(xybox,candidate[i].start,candidate[i].end,
                candidate[i].mindist,candidate[i].maxdist,0,adr,-1);
        if(checkInsert<timeType>(localInfo->timeTree, se,xybox,
              localInfo->k, localInfo->rtree, candidate[i].level)){
          RHCTNNSearch<timeType,dim>(node,mp,localInfo,t,candidate[i].level,se);
        }
        delete node;
      }

  } //else if inner node

}


/* the same as UReal, but not store a,b,c the distance function at^2+bt+c */
struct myureal{
  double a,b,c;
  myureal(){}
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

/* the same as UPoint, but not store p0,p1 */
struct myupoint{
  Point p0,p1;
  myupoint(){}
  myupoint(Point& a,Point& b):p0(a),p1(b){}
  myupoint(const myupoint& u):p0(u.p0),p1(u.p1){}
  myupoint& operator=(const myupoint& u)
  {
    p0 = u.p0;
    p1 = u.p1;
    return *this;
  }
};

struct hpelem{
  myureal movdist;
  TupleId tid;
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
  hpelem(){}
  hpelem(const hpelem& le)
  :movdist(le.movdist),dataup(le.dataup),tid(le.tid),
   mind(le.mind),maxd(le.maxd),nodeid(le.nodeid),
   nodets(le.nodets),nodete(le.nodete){next = le.next;}
  hpelem(TupleId id1,double d1,double d2,long id2)
  :tid(id1),mind(d1),maxd(d2),nodeid(id2){
    next = NULL;
  }
  hpelem& operator=(const hpelem& le)
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
};

void AssignURUP(hpelem* elem,UReal* movdist,UPoint* dataup)
{
  elem->movdist.a = movdist->a;
  elem->movdist.b = movdist->b;
  elem->movdist.c = movdist->c;
  elem->dataup.p0 = dataup->p0;
  elem->dataup.p1 = dataup->p1;
}
/*
The same function as ureal->TemporalFunction

*/
void MyTemporalFunction(hpelem* elem,double& t,Point& result)
{
  if(t == elem->nodets){
    result = elem->dataup.p0;
    result.SetDefined(true);
    return;
  }
  if(t == elem->nodete){
    result = elem->dataup.p1;
    result.SetDefined(true);
    return;
  }
  Point p0 = elem->dataup.p0;
  Point p1 = elem->dataup.p1;
  double t0 = elem->nodets;
  double t1 = elem->nodete;
  double x = (p1.GetX()-p0.GetX())*((t - t0)/(t1-t0)) + p0.GetX();
  double y = (p1.GetY()-p0.GetY())*((t - t0)/(t1-t0)) + p0.GetY();
  result.Set(x,y);
  result.SetDefined(true);

}

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

template<class timeType>
struct TBKnearestLocalInfo
{
  unsigned int k;
  unsigned int attrpos;
  bool scanFlag;
  Relation* relation;
  TBTree* tbtree;
  R_Tree<3,TupleId>* rtree;
  timeType startTime,endTime;
  Line* mptraj;
//  Nearestlist* nlist;
//  double* prunedist;
  vector<Nearestlist> nlist;
  vector<double> prunedist;
  priority_queue<hpelem> hp;
  vector<hpelem> result;
  unsigned int counter;
  bool iscovered; //used to detect time interval in Nearest_list

  TBKnearestLocalInfo(unsigned int nn):k(nn)
  {
 //     nlist = new Nearestlist[k](); //k buffers for NearestLists
 //     prunedist = new double[k];
       iscovered = false;
  }
  ~TBKnearestLocalInfo()
  {

  }

};
/*
Initialization, knearestlist prunedist

*/
template<class timeType>
void hcknnInitialize(TBKnearestLocalInfo<timeType>* local,MPoint* mp)
{
  //Initialization NearestList and prunedist

  for(unsigned int i = 0;i < local->k;i++){
    double mind = numeric_limits<double>::max();
    double maxd = numeric_limits<double>::min();
    double st = local->startTime;
    double et = local->startTime;
    Nearestlist nl(mind,maxd,new hpelem(-1,0,0,-1),st,et);
//    local->nlist[i].mind = numeric_limits<double>::max();
//    local->nlist[i].maxd = numeric_limits<double>::min();
//    local->nlist[i].startTime = local->startTime;
//    local->nlist[i].endTime = local->startTime;
//    local->nlist[i].head = new hpelem(-1,0,0,-1);
    local->nlist.push_back(nl);
//    local->prunedist[i] = numeric_limits<double>::max();
    local->prunedist.push_back(mind);
  }

  SmiRecordId adr = local->tbtree->getRootId();
  tbtree::BasicNode<3>* root = local->tbtree->getNode(adr);
  timeType t1(root->getBox().MinD(2));
  timeType t2(root->getBox().MaxD(2));
  if(!(t1 >= local->endTime || t2 <= local->startTime)){
    Line* line = new Line(0);
    mp->Trajectory(*line);
    local->mptraj = new Line(*line);
    delete line;
    tbtree::InnerNode<3,InnerInfo>* innernode =
            dynamic_cast<InnerNode<3,InnerInfo>*>(root);
    //insert all the entries of the root into hp
    for(unsigned int i = 0;i < innernode->entryCount();i++){
        const Entry<3,InnerInfo>* entry = innernode->getEntry(i);
        timeType tt1((double)entry->getBox().MinD(2));
        timeType tt2((double)entry->getBox().MaxD(2));
        if(!(tt1 >= local->endTime || tt2 <= local->startTime)){
          BBox<2> entrybox = makexyBox(entry->getBox());//entry box
          double mindist = local->mptraj->Distance(entrybox);
          double maxdist = numeric_limits<double>::max();
          hpelem le(-1,mindist,maxdist,entry->getInfo().getPointer());
          le.nodets = tt1;
          le.nodete = tt2;
          local->hp.push(le);
        }
    }
  }
}
/*
Interpolate for entry in TB-tree

*/
template<class timeType>
void CreateUPoint_ne(TBKnearestLocalInfo<timeType>* local,const UPoint* up,
UPoint*& ne,UPoint* data)
{

  Instant start;
  Instant end;
  Point p0,p1;

//  if(data->timeInterval.start > up->timeInterval.start){
  if(data->timeInterval.start.ToDouble() > up->timeInterval.start.ToDouble()){
    start = data->timeInterval.start;
    p0 = data->p0;
  }
  else{
    start = up->timeInterval.start;
    data->TemporalFunction(start,p0,true);
  }
//  if(data->timeInterval.end < up->timeInterval.end){
  if(data->timeInterval.end.ToDouble() < up->timeInterval.end.ToDouble()){
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
Interpolate for entry in MQ

*/

template<class timeType>
void CreateUPoint_nqe(TBKnearestLocalInfo<timeType>* local,const UPoint* up,
UPoint*& nqe,UPoint* data)
{
  Instant start;
  Instant end;
  Point p0,p1;

//  if(data->timeInterval.start > up->timeInterval.start){
  if(data->timeInterval.start.ToDouble() > up->timeInterval.start.ToDouble()){
    start = data->timeInterval.start;
    up->TemporalFunction(start,p0,true);
  }
  else{
    start = up->timeInterval.start;
    p0 = up->p0;
  }
//  if(data->timeInterval.end < up->timeInterval.end){
    if(data->timeInterval.end.ToDouble() < up->timeInterval.end.ToDouble()){
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
template<class timeType>
void UpdateInfoInNL(TBKnearestLocalInfo<timeType>* local,hpelem* elem,int i)
{
  if(elem->mind < local->nlist[i].mind)
    local->nlist[i].mind = elem->mind;
  if(elem->maxd > local->nlist[i].maxd)
    local->nlist[i].maxd = elem->maxd;
  if(elem->nodets < local->nlist[i].startTime)
    local->nlist[i].startTime = elem->nodets;
  if(elem->nodete > local->nlist[i].endTime)
    local->nlist[i].endTime = elem->nodete;
  if(local->nlist[i].maxd > local->prunedist[i])
    local->prunedist[i] = local->nlist[i].maxd;

}
/*
the same function as Min in UReal

*/
double URealMin(hpelem* elem,double& start)
{
    double a = elem->movdist.a;
    double b = elem->movdist.b;
    double c = elem->movdist.c;

    double t_start = elem->nodets - start;
    double t_end = elem->nodete - start;
    double v1 = sqrt(a*t_start*t_start + b*t_start + c);
    double v2 = sqrt(a*t_end*t_end + b*t_end + c);
    double min = v1;
    if(v2 < min)
      min = v2;
    if(AlmostEqual(a,0.0))
      return min;
    double asymx = (-1.0*b)/(2*a);
    double v3 = min;
    if(t_start < asymx && asymx < t_end){
      v3 = sqrt(a*asymx*asymx+b*asymx+c);
      if(v3 < min)
        min = v3;
    }
    return min;

}

/*
the same function as Max in UReal

*/

double URealMax(hpelem* elem,double& start)
{
    double a = elem->movdist.a;
    double b = elem->movdist.b;
    double c = elem->movdist.c;
    double t_start = elem->nodets - start;
    double t_end = elem->nodete - start;

    double v1 = sqrt(a*t_start*t_start + b*t_start + c);
    double v2 = sqrt(a*t_end*t_end + b*t_end + c);
    double max = v1;
    if(v2 > max)
      max = v2;
    if(AlmostEqual(a,0.0))
      return max;
    double asymx = (-1.0*b)/(2*a);
    double v3 = 0;
    if(t_start < asymx && asymx < t_end){ //three values
      v3 = sqrt(a*asymx*asymx+b*asymx+c);
      if(v3 > max)
        max = v3;
    }
    return max;

}


/*
translate the start time of parabolas

*/
void URealTranslate(hpelem* elem,double& start)
{
  double b,c;
  double ts = elem->nodets - start;
  b = 2*elem->movdist.a*ts + elem->movdist.b;
  c = elem->movdist.c + elem->movdist.b*ts + elem->movdist.a*ts*ts;
  elem->movdist.b = b;
  elem->movdist.c = c;
}


/*
interpolation parabolas
entry unit (in list) is first smaller than data (new elem), and then new
elem is smaller than entry (the one already stored in Nearestlist)
T is smaller first

*/

template<class timeType>
void ParabolasTM(TBKnearestLocalInfo<timeType>* local,hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& intersect,double& elemstart,double& curstart)
{

    hpelem* newelem1 = new hpelem(*cur);
    newelem1->nodets = intersect;
    Point start;
    MyTemporalFunction(newelem1,intersect,start);
    newelem1->dataup.p0 = start;
    newelem1->next= NULL;
    newelem1->mind = URealMin(newelem1,curstart);
    newelem1->maxd = URealMax(newelem1,curstart);
    URealTranslate(newelem1,curstart);


    cur->nodete = intersect;
    Point end;
    MyTemporalFunction(cur,intersect,end);
    cur->dataup.p1 = end;
    cur->mind = URealMin(cur,curstart);
    cur->maxd = URealMax(cur,curstart);



    hpelem* newelem2 = new hpelem(elem);
    newelem2->nodete = intersect;
    MyTemporalFunction(newelem2,intersect,end);
    newelem2->dataup.p1 = end;
    newelem2->mind = URealMin(newelem2,elemstart);
    newelem2->maxd = URealMax(newelem2,elemstart);



    nextupdatelist.push_back(*newelem1);
    nextupdatelist.push_back(*newelem2);

    hpelem* newelem3 = new hpelem(elem);
    newelem3->nodets = intersect;
    MyTemporalFunction(newelem3,intersect,start);
    newelem3->dataup.p0 = start;
    newelem3->next = cur->next;


    cur->next = newelem3;
////////////////////////////////////
    head = cur;
    cur = newelem3;
////////////////////////////////
    newelem3->mind = URealMin(newelem3,elemstart);
    newelem3->maxd = URealMax(newelem3,elemstart);
    URealTranslate(newelem3,elemstart);


    UpdateInfoInNL(local,cur,i);
    UpdateInfoInNL(local,newelem3,i);
}
/*
interpolation parabolas
entry unit (in list) is first larger than data (new elem), and then new
elem is larger than entry (the one already stored in Nearestlist)
M is smaller first

*/
template<class timeType>
void ParabolasMT(TBKnearestLocalInfo<timeType>* local,hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& intersect,double& elemstart,double& curstart)
{

  Point end;
  hpelem* newelem1 = new hpelem(elem);

  newelem1->nodete = intersect;
  MyTemporalFunction(newelem1,intersect,end);
  newelem1->dataup.p1 = end;
  newelem1->next = NULL;
  newelem1->mind = URealMin(newelem1,elemstart);
  newelem1->maxd = URealMax(newelem1,elemstart);



  hpelem* newelem2 = new hpelem(*cur);

  newelem2->nodete = intersect;
  MyTemporalFunction(newelem2,intersect,end);
  newelem2->dataup.p1 = end;
  newelem2->next= NULL;
  newelem2->mind = URealMin(newelem2,curstart);
  newelem2->maxd = URealMax(newelem2,curstart);



  cur->nodets = intersect;
  Point start;
  MyTemporalFunction(cur,intersect,start);
  cur->dataup.p0 = start;
  cur->mind = URealMin(cur,curstart);
  cur->maxd = URealMax(cur,curstart);
  URealTranslate(cur,curstart);


  head->next = newelem1;
  newelem1->next = cur;
///////////////////////
  head = newelem1;
////////////////////////
  UpdateInfoInNL(local,newelem1,i);
  UpdateInfoInNL(local,cur,i);

  hpelem* newelem3 = new hpelem(elem);

  newelem3->nodets = intersect;
  MyTemporalFunction(newelem3,intersect,start);
  newelem3->dataup.p0 = start;
  newelem3->next = NULL;
  newelem3->mind = URealMin(newelem3,elemstart);
  newelem3->maxd = URealMax(newelem3,elemstart);
  URealTranslate(newelem3,elemstart);


  nextupdatelist.push_back(*newelem2);
  nextupdatelist.push_back(*newelem3);
}
/*
Process different splitting cases of parabolas
restrict to the same time interval and check whether the moving distance
function has to be split or not

*/
template<class timeType>
void Parabolas(TBKnearestLocalInfo<timeType>* local,hpelem& elem
,vector<hpelem>& nextupdatelist,int i,hpelem*& head,hpelem*& cur,
double& elemstart,double& curstart)
{

  double ma,mb,mc; //data
  double ta,tb,tc; //entry in list
  double ttelem = elemstart;
  double ttcur = curstart;

  //a1(t-t1)^2 + b1(t-t1) + c1
  //a2(t-t2)^2 + b2(t-t2) + c2

  ma = elem.movdist.a;
  mb = elem.movdist.b - 2*ma*ttelem;
  mc = elem.movdist.c + ma*ttelem*ttelem- mb*ttelem;
  ta = cur->movdist.a;
  tb = cur->movdist.b - 2*ta*ttcur;
  tc = cur->movdist.c + ta*ttcur*ttcur - tb*ttcur;


  double start_m,start_t;

  if(ttelem == elem.nodets){
      start_m = sqrt(elem.movdist.c);
  }
  else{
    double delta_m = elem.nodets-elemstart;
    start_m =
        sqrt(ma*delta_m*delta_m + elem.movdist.b*delta_m + elem.movdist.c);
  }

  if(ttcur == cur->nodets){
    start_t = sqrt(cur->movdist.c);
  }
  else{

    double delta_t = cur->nodets-curstart;
    start_t =
    sqrt(ta*delta_t*delta_t + cur->movdist.b*delta_t + cur->movdist.c);
  }


  if(ma == ta && mb == tb && mc == tc){
      nextupdatelist.push_back(elem);
      return;//for next
  }
  if(ma == ta && mb == tb){
        if(start_t <= start_m){
          nextupdatelist.push_back(elem);
          return;
      }else{// entry is to be replaced by data
          hpelem* newhp = new hpelem(elem);
          head->next = newhp;
          newhp->next = cur->next;
          UpdateInfoInNL(local,newhp,i);
          cur->next = NULL;
          nextupdatelist.push_back(*cur);
          cur = newhp; //new cur
          return;
      }
  }
  if(ma == ta){
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
                hpelem* newhp = new hpelem(elem);
                head->next = newhp;
                newhp->next = cur->next;
                UpdateInfoInNL(local,newhp,i);
                cur->next = NULL;
                nextupdatelist.push_back(*cur);
                cur = newhp; //new cur
                return;
               }
      }else{ //needs to be interpolation
            double intersect = t;

            if(start_t <= start_m){
              ParabolasTM(local,elem,nextupdatelist,i,head,cur,
              intersect,elemstart,curstart);
              return;
          }else{ //
              ParabolasMT(local,elem,nextupdatelist,i,head,cur,
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
//      if(maxd_t <= maxd_m){
      if(mind_t <= mind_m){
        nextupdatelist.push_back(elem);
        return;
      }else{ //entry is to be replaced by data
          hpelem* newhp = new hpelem(elem);
          head->next = newhp;
          newhp->next = cur->next;
          UpdateInfoInNL(local,newhp,i);
          cur->next = NULL;
          nextupdatelist.push_back(*cur);
          cur = newhp;  //new cur
          return;
      }
  }

  if(delta < 0) { //no intersects
#ifdef mydebug
    cout<<"delta < 0"<<endl;
#endif
    if(mind_t <= mind_m){
#ifdef mydebug
        cout<<"update "<<endl;
#endif
        nextupdatelist.push_back(elem);
        return;
    }else{ //entry is to be replaced by data
#ifdef mydebug
        cout<<"replaced "<<endl;
#endif
        hpelem* newhp = new hpelem(elem);
        head->next = newhp;
        newhp->next = cur->next;
        UpdateInfoInNL(local,newhp,i);
        cur->next = NULL;
        nextupdatelist.push_back(*cur);
        cur = newhp; //new cur
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
       hpelem* newhp = new hpelem(elem);
       head->next = newhp;
       newhp->next = cur->next;
       UpdateInfoInNL(local,newhp,i);
       cur->next = NULL;
       nextupdatelist.push_back(*cur);
       cur = newhp; //new cur
       return;
     }
  }
  //one intersection point

    if(intersect_t1 > elemts &&
        intersect_t1 < elemte&&
        intersect_t2 > elemte){
        if(start_t <= start_m){
          ParabolasTM(local,elem,nextupdatelist,i,head,cur,
            intersect1,elemstart,curstart);
          return;
      }else{ //
          ParabolasMT(local,elem,nextupdatelist,i,head,cur,
            intersect1,elemstart,curstart);
          return;
      }
  }

    if(intersect_t1 < elemts &&
       intersect_t2 > elemts &&
       intersect_t2 < elemte){

       if(start_t <= start_m){
        ParabolasTM(local,elem,nextupdatelist,i,head,cur,
          intersect2,elemstart,curstart);
        return;
     }else{ //
        ParabolasMT(local,elem,nextupdatelist,i,head,cur,
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
    MyTemporalFunction(newhp1,intersect1,end);
    newhp1->dataup.p1 = end;
    newhp1->mind = URealMin(newhp1,curstart);
    newhp1->maxd = URealMax(newhp1,curstart);


    hpelem* newhp2 = new hpelem(elem);
    newhp2->nodets = intersect1;
    newhp2->nodete = intersect2;
    MyTemporalFunction(newhp2,intersect1,p0);
    MyTemporalFunction(newhp2,intersect2,p1);
    newhp2->dataup.p0 = p0;
    newhp2->dataup.p1 = p1;
    newhp2->mind = URealMin(newhp2,elemstart);
    newhp2->maxd = URealMax(newhp2,elemstart);
    URealTranslate(newhp2,elemstart);


    hpelem* newhp3 = new hpelem(*cur);
    newhp3->nodets = intersect2;
    MyTemporalFunction(newhp3,intersect2,start);
    newhp3->dataup.p0 = start;
    newhp3->mind = URealMin(newhp3,curstart);
    newhp3->maxd = URealMax(newhp3,curstart);
    URealTranslate(newhp3,curstart);



    hpelem* newhp4 = new hpelem(elem);
    newhp4->nodete = intersect1;
    MyTemporalFunction(newhp4,intersect1,end);
    newhp4->dataup.p1 = end;
    newhp4->next = NULL;
    newhp4->mind = URealMin(newhp4,elemstart);
    newhp4->maxd = URealMax(newhp4,elemstart);



    hpelem* newhp5 = new hpelem(*(cur));
    newhp5->nodets = intersect1;
    newhp5->nodete = intersect2;
    MyTemporalFunction(newhp5,intersect1,p0);
    MyTemporalFunction(newhp5,intersect2,p1);
    newhp5->dataup.p0 = p0;
    newhp5->dataup.p1 = p1;
    newhp5->next = NULL;
    newhp5->mind = URealMin(newhp5,curstart);
    newhp5->maxd = URealMax(newhp5,curstart);
    URealTranslate(newhp5,curstart);


    hpelem* newhp6 = new hpelem(elem);
    newhp6->nodets = intersect2;
    MyTemporalFunction(newhp6,intersect2,start);
    newhp6->dataup.p0 = start;
    newhp6->next = NULL;
    newhp6->mind = URealMin(newhp6,elemstart);
    newhp6->maxd = URealMax(newhp6,elemstart);
    URealTranslate(newhp6,elemstart);


    head->next = newhp1;
    newhp1->next = newhp2;
    newhp2->next = newhp3;
    newhp3->next = next;
    UpdateInfoInNL(local,newhp2,i);
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
    MyTemporalFunction(newhp1,intersect1,end);
    newhp1->dataup.p1 = end;
    newhp1->next = NULL;
    newhp1->mind = URealMin(newhp1,elemstart);
    newhp1->maxd = URealMax(newhp1,elemstart);


    hpelem* newhp2 = new hpelem(*(cur));
    newhp2->nodets = intersect1;
    newhp2->nodete = intersect2;
    MyTemporalFunction(newhp2,intersect1,p0);
    MyTemporalFunction(newhp2,intersect2,p1);
    newhp2->dataup.p0 = p0;
    newhp2->dataup.p1 = p1;
    newhp2->next = NULL;
    newhp2->mind = URealMin(newhp2,curstart);
    newhp2->maxd = URealMax(newhp2,curstart);
    URealTranslate(newhp2,curstart);


    hpelem* newhp3 = new hpelem(elem);
    newhp3->nodets = intersect2;
    MyTemporalFunction(newhp3,intersect2,start);
    newhp3->dataup.p0 = start;
    newhp3->next = NULL;
    newhp3->mind = URealMin(newhp3,elemstart);
    newhp3->maxd = URealMax(newhp3,elemstart);
    URealTranslate(newhp3,elemstart);


    hpelem* newhp4 = new hpelem(*cur);
    newhp4->nodete = intersect1;
    MyTemporalFunction(newhp4,intersect1,end);
    newhp4->dataup.p1 = end;
    newhp4->mind = URealMin(newhp4,curstart);
    newhp4->maxd = URealMax(newhp4,curstart);


    hpelem* newhp5 = new hpelem(elem);
    newhp5->nodets = intersect1;
    newhp5->nodete = intersect2;
    MyTemporalFunction(newhp5,intersect1,p0);
    MyTemporalFunction(newhp5,intersect2,p1);
    newhp5->dataup.p0 = p0;
    newhp5->dataup.p1 = p1;
    newhp5->mind = URealMin(newhp5,elemstart);
    newhp5->maxd = URealMax(newhp5,elemstart);
    URealTranslate(newhp5,elemstart);


    hpelem* newhp6 = new hpelem(*cur);
    newhp6->nodets = intersect2;
    MyTemporalFunction(newhp6,intersect2,start);
    newhp6->dataup.p0 = start;
    newhp6->mind = URealMin(newhp6,curstart);
    newhp6->maxd = URealMax(newhp6,curstart);
    URealTranslate(newhp6,curstart);


    head->next = newhp1;
    newhp1->next = newhp2;
    newhp2->next = newhp3;
    newhp3->next = next;
    UpdateInfoInNL(local,newhp2,i);
    head = newhp2;
    cur = newhp3;

    nextupdatelist.push_back(*newhp4);
    nextupdatelist.push_back(*newhp5);
    nextupdatelist.push_back(*newhp6);
    return;
  }

}
/*
Update k NearestList structure
traverse the k nearest list

*/
template<class timeType>
void UpdateNearest(TBKnearestLocalInfo<timeType>* local,hpelem& elem
,vector<hpelem>& nextupdatelist,int i)
{
  hpelem* head = local->nlist[i].head;
  assert(head != NULL);
  hpelem* cur = head->next;

  if(cur == NULL){

    hpelem* newhp = new hpelem(elem);
    head->next = newhp;
    newhp->next = NULL;
    local->nlist[i].mind = elem.mind;
    local->nlist[i].maxd = elem.maxd;

    local->nlist[i].startTime = elem.nodets;
    local->nlist[i].endTime = elem.nodete;

    local->prunedist[i] = local->nlist[i].maxd;
    return;
  }else{
      //the first elem
//      if(elem.dataup->timeInterval.end <= cur->dataup->timeInterval.start){
        if(elem.nodete <= cur->nodets){
           hpelem* newhp = new hpelem(elem);
           head->next = newhp;
           newhp->next = cur;
           UpdateInfoInNL(local,newhp,i);
           return;// no update list
       }
#ifdef mydebug
      cout<<"Update Nearest elem "<<elem.dataup->timeInterval<<" min "
          <<elem.movdist->Min(def)<<" max "
          <<elem.movdist->Max(def)<<" tid "<<elem.tid<<endl;
#endif
      while(cur != NULL){ //big program

#ifdef mydebug
//        cout<<"while cur "<<cur->dataup->timeInterval<<" min "
//            <<cur->movdist->Min(def)<<" max "
//            <<cur->movdist->Max(def)<<" tid "<<cur->tid<<endl;
#endif


        if(cur->nodete <= elem.nodets){
          if(cur->next == NULL){//the last element
            hpelem* newhp = new hpelem(elem);
            cur->next = newhp;
            newhp->next = NULL;
            UpdateInfoInNL(local,newhp,i);
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
           UpdateInfoInNL(local,newhp,i);
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


        if(mts == tts && mte == tte){ //the same time interval
#ifdef mydebug
            cout<<"equal "<<endl;
#endif
            Parabolas(local,elem,nextupdatelist,i,head,cur,mts,tts);
            return;
        }else{// needs to be interpolation

          //overlap

          double ts =  mts > tts ? mts:tts;
          double te = mte < tte ? mte:tte;

          if(mts < ts){

            hpelem* newhp = new hpelem(elem);
            newhp->nodete = ts;

            Point end;
            MyTemporalFunction(newhp,ts,end);
            newhp->dataup.p1 = end;


            newhp->mind = URealMin(newhp,mts);
            newhp->maxd = URealMax(newhp,mts);


            head->next = newhp;
            newhp->next = cur;
            head = newhp;
            UpdateInfoInNL(local,newhp,i);

            elem.nodets = ts;
            Point start;
            MyTemporalFunction(&elem,ts,start);
            elem.dataup.p0 = start;
            elem.mind = URealMin(&elem,mts);
            elem.maxd = URealMax(&elem,mts);
            URealTranslate(&elem,mts);

          }

          if(tts < ts){

            hpelem* newhp = new hpelem(*cur);

            newhp->nodete = ts;
            Point end;
            MyTemporalFunction(newhp,ts,end);
            newhp->dataup.p1 = end;
            newhp->mind = URealMin(newhp,tts);
            newhp->maxd = URealMax(newhp,tts);

            head->next = newhp;
            newhp->next = cur;
            head = newhp;

            cur->nodets = ts;
            Point start;
            MyTemporalFunction(cur,ts,start);
            cur->dataup.p0 = start;
            cur->mind = URealMin(cur,tts);
            cur->maxd = URealMax(cur,tts);
            URealTranslate(cur,tts);

          }
          //up to now,  cur and elem start at the same time interval
          if(tte > te){ //M is finished here

            hpelem* newhp = new hpelem(*cur);

            newhp->nodets = te;
            Point start;
            MyTemporalFunction(newhp,te,start);
            newhp->dataup.p0 = start;
            newhp->mind = URealMin(newhp,ts);
            newhp->maxd = URealMax(newhp,ts);
            URealTranslate(newhp,ts);


            newhp->next = cur->next;
            cur->next = newhp;

            Point end;
            cur->nodete = te;
            MyTemporalFunction(cur,te,end);
            cur->dataup.p1 = end;
            cur->mind = URealMin(cur,ts);
            cur->maxd = URealMax(cur,ts);


            Parabolas(local,elem,nextupdatelist,i,head,cur,ts,ts);
            return;
          }else{ //M is longer
#ifdef mydebug
            cout<<"elem is longer"<<endl;
#endif
//            if(elem.movdist->timeInterval.end ==
//                cur->movdist->timeInterval.end){
            if(elem.nodete == cur->nodete){
#ifdef mydebug
              cout<<"elem equal to cur in end time"<<endl;
#endif
              Parabolas(local,elem,nextupdatelist,i,head,cur,ts,ts);
              return;
            }else{
#ifdef mydebug
              cout<<"next loop"<<endl;
#endif

              hpelem* newhp = new hpelem(elem);

              newhp->nodets = te;
              Point start;
              MyTemporalFunction(newhp,te,start);
              newhp->dataup.p0 = start;
              newhp->mind = URealMin(newhp,ts);
              newhp->maxd = URealMax(newhp,ts);
              URealTranslate(newhp,ts);


              elem.nodete = te;
              Point end;
              MyTemporalFunction(&elem,te,end);
              elem.dataup.p1 = end;
              elem.mind = URealMin(&elem,ts);
              elem.maxd = URealMax(&elem,ts);

              Parabolas(local,elem,nextupdatelist,i,head,cur,ts,ts);
#ifdef mydebug
              if(head->tid != -1){
                cout<<"after parabolas head "<<head->dataup->timeInterval
                <<" min "<<head->movdist->Min(def)<<" max "
                <<head->movdist->Max(def)<<" tid "<<head->tid<<endl;
              }

              cout<<"after parabolas cur "<<cur->dataup->timeInterval<<" min "
              <<cur->movdist->Min(def)<<" max "
              <<cur->movdist->Max(def)<<" tid "<<cur->tid<<endl;
#endif
              elem = *newhp;
              head = cur;
              cur = cur->next;
#ifdef mydebug
              cout<<"newelem "<<elem.dataup->timeInterval<<" min "
              <<elem.movdist->Min(def)<<" max "
              <<elem.movdist->Max(def)<<" tid "<<elem.tid<<endl;

              cout<<"new cur "<<cur->dataup->timeInterval<<" min "
              <<cur->movdist->Min(def)<<" max "
              <<cur->movdist->Max(def)<<" tid "<<cur->tid<<endl;
#endif
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
template<class timeType>
bool CheckPrune(TBKnearestLocalInfo<timeType>* local, hpelem& elem)
{
  if(local->iscovered){//if the whole time interval covers


  }else{
      hpelem* head = local->nlist[local->k-1].head;
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
}
/*
Main function, update the k nearest list structure and prunedist

*/
template<class timeType>
void UpdatekNearest(TBKnearestLocalInfo<timeType>* local,hpelem& elem)
{
  list<hpelem> updatelist;

//  if(elem.mind < local->prunedist[local->k-1]){
    if(CheckPrune(local,elem) == false){

      updatelist.push_back(elem);
      vector<hpelem> auxiliarylist;
      vector<hpelem> templist;
      for(unsigned int i = 0;i < local->k;i++){//for each NearestList
        while(updatelist.empty() == false){
            hpelem top = updatelist.front();
            updatelist.pop_front();
//            if(top.mind >= local->prunedist[i]) continue;
            if(CheckPrune(local,top) == false)
              UpdateNearest(local,top,templist,i); //key function

            for(unsigned int j = 0;j < templist.size();j++)//transfer step1
              auxiliarylist.push_back(templist[j]);
            templist.clear();
        }
        for(unsigned int j = 0;j < auxiliarylist.size();j++)//transfer step2
          updatelist.push_back(auxiliarylist[j]);
        auxiliarylist.clear();

      }
  }
}
bool HpelemCompare(const hpelem& e1,const hpelem& e2)
{
    return e1.nodets < e2.nodets;
}
/*
Store results into an array

*/
template<class timeType>
void ReportResult(TBKnearestLocalInfo<timeType>* local)
{
  for(unsigned int i = 0; i < local->k;i++){ //traverse NearestList
    hpelem* head = local->nlist[i].head;
    hpelem* cur = head->next;
    while(cur != NULL){
      hpelem top = *cur;
      assert(top.tid != -1 && top.nodeid == -1);
      if(top.nodets != top.nodete)
        local->result.push_back(top);
      head = cur;
      cur = cur->next;
    }
  }
  stable_sort(local->result.begin(),local->result.end(),HpelemCompare);
}
/*
The main Function of chinese algorithm

*/
template<class timeType>
void hcknnFun(TBKnearestLocalInfo<timeType>* local,MPoint* mp)
{
  BBox<2> mpbox = local->mptraj->BoundingBox();
  if(mpbox.IsDefined() == false)
    mpbox = makexyBox(mp->BoundingBox());

  while(local->hp.empty() == false){
    hpelem top = local->hp.top();
    local->hp.pop();
    if(top.tid != -1)
      if(CheckPrune(local,top))
        continue;
    else{
      if(CheckPrune(local,top))
        continue;
    }

    if(top.tid != -1 && top.nodeid == -1){ //an actual trajectory segment
        UpdatekNearest(local,top);
    }
    else if(top.tid == -1 && top.nodeid != -1){ // a node
        tbtree::BasicNode<3>* tbnode = local->tbtree->getNode(top.nodeid);
        if(tbnode->isLeaf()){ //leaf node
            tbtree::TBLeafNode<3,TBLeafInfo>* leafnode =
            dynamic_cast<TBLeafNode<3,TBLeafInfo>* > (tbnode);
            for(unsigned int i = 0;i < leafnode->entryCount();i++){
              const Entry<3,TBLeafInfo>* entry = leafnode->getEntry(i);
              timeType t1((double)entry->getBox().MinD(2));
              timeType t2((double)entry->getBox().MaxD(2));
              if(!(t1 >= local->endTime || t2 <= local->startTime)){
                  //for each unit in mp
                const UPoint* up;
                TupleId tid = entry->getInfo().getTupleId();
                Tuple* tuple = local->relation->GetTuple(tid);
                UPoint* data = (UPoint*)tuple->GetAttribute(local->attrpos);

                for(int j = 0;j < mp->GetNoComponents();j++){
                    mp->Get(j,up);
                    timeType tt1 = (double)(up->timeInterval.start.ToDouble());
                    timeType tt2 = (double)(up->timeInterval.end.ToDouble());
                    if(tt1 > t2) //mq's time interval is larger than entry
                      break;
                    if(!(t1 >= tt2 || t2 <= tt1)){
                      UPoint* ne = new UPoint(true);
                      UPoint* nqe = new UPoint(true);
                      /*interpolation restrict to the same time interval*/

                      CreateUPoint_ne(local,up,ne,data);
                      CreateUPoint_nqe(local,up,nqe,data);

                      UReal* mdist = new UReal(true);
                      ne->Distance(*nqe,*mdist);
                      bool def = true;
                      double mind = mdist->Min(def);
                      double maxd = mdist->Max(def);

                      hpelem le(entry->getInfo().getTupleId(),mind,maxd,-1);
                      le.nodets = mdist->timeInterval.start.ToDouble();
                      le.nodete = mdist->timeInterval.end.ToDouble();
                      AssignURUP(&le,mdist,ne);

                      if(CheckPrune(local,le) == false){
                        local->hp.push(le);
                      }
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
              timeType t1((double)entry->getBox().MinD(2));
              timeType t2((double)entry->getBox().MaxD(2));
              if(!(t1 >= local->endTime || t2 <= local->startTime)){
                BBox<2> entrybox = makexyBox(entry->getBox());//entry box
                double mindist =  mpbox.Distance(entrybox);
                double maxdist = maxDistance(entrybox,mpbox);

                  hpelem le(-1,mindist,maxdist,entry->getInfo().getPointer());
                  le.nodets = t1;
                  le.nodete = t2;
                  if(CheckPrune(local,le) == false)
                    local->hp.push(le);
              }
            }
        }
    }
  }
  //results are stored in Nearest_List
  ReportResult(local);

}
/*
Initialization,

*/
template<class timeType>
void rknearestInitialize(TBKnearestLocalInfo<timeType>* local,MPoint* mp)
{
  //Initialization NearestList and prunedist
  const int dim = 3;

  for(unsigned int i = 0;i < local->k;i++){
    double mind = numeric_limits<double>::max();
    double maxd = numeric_limits<double>::min();
    double st = local->startTime;
    double et = local->startTime;
    Nearestlist nl(mind,maxd,new hpelem(-1,0,0,-1),st,et);
//    local->nlist[i].mind = numeric_limits<double>::max();
//    local->nlist[i].maxd = numeric_limits<double>::min();
//    local->nlist[i].startTime = local->startTime;
//    local->nlist[i].endTime = local->startTime;
//    local->nlist[i].head = new hpelem(-1,0,0,-1);
    local->nlist.push_back(nl);
//    local->prunedist[i] = numeric_limits<double>::max();
    local->prunedist.push_back(mind);
  }


  SmiRecordId adr = local->rtree->RootRecordId();
  R_TreeNode<dim,TupleId>* root = local->rtree->GetMyNode(adr,
      false,local->rtree->MinEntries(0),local->rtree->MaxEntries(0));

  timeType t1(root->BoundingBox().MinD(2));
  timeType t2(root->BoundingBox().MaxD(2));
  t1 = t1/864000.0;
  t2 = t2/864000.0;

  if(!(t1 >= local->endTime || t2 <= local->startTime)){
    Line* line = new Line(0);
    mp->Trajectory(*line);
    local->mptraj = new Line(*line);
    delete line;

    //insert all the entries of the root into hp
    for(int i = 0;i < root->EntryCount();i++){
        R_TreeInternalEntry<dim> e = (R_TreeInternalEntry<dim>&)(*root)[i];
        timeType tt1((double)e.box.MinD(2));
        timeType tt2((double)e.box.MaxD(2));
        tt1 = tt1/864000.0;
        tt2 = tt2/864000.0;

        if(!(tt1 >= local->endTime || tt2 <= local->startTime)){
          BBox<2> entrybox = makexyBox(e.box);//entry box
          double mindist = local->mptraj->Distance(entrybox);
          double maxdist = numeric_limits<double>::max();
          hpelem le(-1,mindist,maxdist,e.pointer);
          le.nodets = tt1;
          le.nodete = tt2;
          local->hp.push(le);
        }
    }
  }
}

/*
Greece algorithm

*/
template<class timeType>
void rknearestFun(TBKnearestLocalInfo<timeType>* local,MPoint* mp,int level)
{
  const int dim = 3;
  BBox<2> mpbox = local->mptraj->BoundingBox();
  if(mpbox.IsDefined() == false)
    mpbox = makexyBox(mp->BoundingBox());

  while(local->hp.empty() == false){
    hpelem top = local->hp.top();
    local->hp.pop();
    if(top.tid != -1)
      if(CheckPrune(local,top))
        continue;
    else{
      if(CheckPrune(local,top))
        continue;
    }

    if(top.tid != -1 && top.nodeid == -1){ //an actual trajectory segment
        UpdatekNearest(local,top);
    }
    else if(top.tid == -1 && top.nodeid != -1){ // a leaf node
        SmiRecordId adr = top.nodeid;
        R_TreeNode<dim,TupleId>* tbnode = local->rtree->GetMyNode(
            adr,false,local->rtree->MinEntries(0),
                             local->rtree->MaxEntries(0));
        if(tbnode->IsLeaf()){ //leaf node

            for(int i = 0;i < tbnode->EntryCount();i++){
              R_TreeLeafEntry<dim,TupleId> e =
                        (R_TreeLeafEntry<dim,TupleId>&)(*tbnode)[i];

              timeType t1((double)e.box.MinD(2));
              timeType t2((double)e.box.MaxD(2));
              t1 = t1/864000.0;
              t2 = t2/864000.0;
              if(!(t1 >= local->endTime || t2 <= local->startTime)){
                  //for each unit in mp
                const UPoint* up;
                TupleId tid = e.info;
                Tuple* tuple = local->relation->GetTuple(tid);
                UPoint* data = (UPoint*)tuple->GetAttribute(local->attrpos);

                for(int j = 0;j < mp->GetNoComponents();j++){
                    mp->Get(j,up);
                    timeType tt1 = (double)(up->timeInterval.start.ToDouble());
                    timeType tt2 = (double)(up->timeInterval.end.ToDouble());

                    if(tt1 > t2) //mq's time interval is larger than entry
                      break;
                    if(!(t1 >= tt2 || t2 <= tt1)){
                      UPoint* ne = new UPoint(true);
                      UPoint* nqe = new UPoint(true);
                      /*interpolation restrict to the same time interval*/

                      CreateUPoint_ne(local,up,ne,data);
                      CreateUPoint_nqe(local,up,nqe,data);

                      double nodets = ne->timeInterval.start.ToDouble();
                      double nodete = ne->timeInterval.end.ToDouble();
                      if(AlmostEqual(nodets,nodete))
                        continue;

                      UReal* mdist = new UReal(true);
                      ne->Distance(*nqe,*mdist);
                      bool def = true;
                      double mind = mdist->Min(def);
                      double maxd = mdist->Max(def);

                      hpelem le(tid,mind,maxd,-1);
                      le.nodets = nodets;
                      le.nodete = nodete;
                      AssignURUP(&le,mdist,ne);

                      if(CheckPrune(local,le) == false){
                        local->hp.push(le);
                      }
                      delete mdist;
                      delete ne;
                      delete nqe;
                    }
                }//end for
                tuple->DeleteIfAllowed();
              }
           }
        }else{ //inner node
            for(int i = 0;i < tbnode->EntryCount();i++){
              R_TreeInternalEntry<dim> e =
                    (R_TreeInternalEntry<dim>&)(*tbnode)[i];
              timeType t1((double)e.box.MinD(2));
              timeType t2((double)e.box.MaxD(2));
              t1 = t1/864000.0;
              t2 = t2/864000.0;
              if(!(t1 >= local->endTime || t2 <= local->startTime)){
                BBox<2> entrybox = makexyBox(e.box);//entry box
                double mindist =  mpbox.Distance(entrybox);
                double maxdist = maxDistance(entrybox,mpbox);

                  hpelem le(-1,mindist,maxdist,e.pointer);
                  le.nodets = t1;
                  le.nodete = t2;
                  if(CheckPrune(local,le) == false)
                    local->hp.push(le);
              }
            }
        }
    }
  }
  //results are stored in Nearest_List
  ReportResult(local);

}
/*
rknearestfilterFun is the value function for the rknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists an r-tree for the unit attribute
The argument vector contains the following values:
args[0] = an r-tree with the unit attribute as key
args[1] = the relation of the tbtree
args[2] = attribute UPoint
args[3] = mpoint
args[4] = int k, how many nearest are searched

*/


template<class timeType>
int rknearest(Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  const int dim = 3;
  TBKnearestLocalInfo<timeType> *localInfo;
  switch (message)
  {
    case OPEN :
    {
      //Initialize hp,kNearestLists and PruneDist
      MPoint* mp = (MPoint*)args[3].addr;
      if(mp->IsEmpty())
        return 0;
      const UPoint* up1;
      const UPoint* up2;
      mp->Get(0,up1);
      mp->Get(mp->GetNoComponents()-1,up2);
      const unsigned int k = (unsigned int)((CcInt*)args[4].addr)->GetIntval();
      int attrpos = ((CcInt*)args[5].addr)->GetIntval()-1;
      localInfo = new TBKnearestLocalInfo<timeType>(k);
      local = SetWord(localInfo);
      localInfo->attrpos = attrpos;
      localInfo->startTime = up1->timeInterval.start.ToDouble();
      localInfo->endTime = up2->timeInterval.end.ToDouble();
      localInfo->rtree = (R_Tree<dim,TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->counter = 0;
      localInfo->scanFlag = true;
      rknearestInitialize(localInfo,mp);
      rknearestFun(localInfo,mp,0);
      return 0;
    }
    case REQUEST :
    {
        localInfo = (TBKnearestLocalInfo<timeType>*)local.addr;
        if(localInfo->k == 0)
          return CANCEL;
        if(localInfo->counter < localInfo->result.size()){
            TupleId tid = localInfo->result[localInfo->counter].tid;
            Tuple* tuple = localInfo->relation->GetTuple(tid);
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
      localInfo = (TBKnearestLocalInfo<timeType>*)local.addr;
      delete localInfo;
      return 0;
    }
  }
  return 0;

}

/*

hcknnknearestFun is the value function for the hcknnknearest operator
It is a filter operator for the knearest operator. It can be called
if there exists a tb-tree for the unit attribute
The argument vector contains the following values:
args[0] = a tbtree with the unit attribute as key
args[1] = the relation of the tbtree
args[2] = attribute UPoint
args[3] = mpoint
args[4] = int k, how many nearest are searched

*/
template<class timeType>
int hcknnknearestFun (Word* args, Word& result, int message,
             Word& local, Supplier s)
{
  TBKnearestLocalInfo<timeType> *localInfo;
  switch (message)
  {
    case OPEN :
    {
      //Initialize hp,kNearestLists and PruneDist
      MPoint* mp = (MPoint*)args[3].addr;
      if(mp->IsEmpty())
        return 0;
      const UPoint* up1;
      const UPoint* up2;
      mp->Get(0,up1);
      mp->Get(mp->GetNoComponents()-1,up2);
      const unsigned int k = (unsigned int)((CcInt*)args[4].addr)->GetIntval();
      int attrpos = ((CcInt*)args[5].addr)->GetIntval()-1;
      localInfo = new TBKnearestLocalInfo<timeType>(k);
      local = SetWord(localInfo);
      localInfo->attrpos = attrpos;
      localInfo->startTime = up1->timeInterval.start.ToDouble();
      localInfo->endTime = up2->timeInterval.end.ToDouble();


      localInfo->tbtree = (TBTree*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->counter = 0;
      localInfo->scanFlag = true;
      hcknnInitialize(localInfo,mp);
      hcknnFun(localInfo,mp);
      return 0;
    }
    case REQUEST :
    {
        localInfo = (TBKnearestLocalInfo<timeType>*)local.addr;
        if(localInfo->k == 0)
          return CANCEL;
        if(localInfo->counter < localInfo->result.size()){
            TupleId tid = localInfo->result[localInfo->counter].tid;
            Tuple* tuple = localInfo->relation->GetTuple(tid);
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
      localInfo = (TBKnearestLocalInfo<timeType>*)local.addr;

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
ListExpr hcknnknearestTypeMap( ListExpr args )
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
  if(!nl->IsEqual(mpoint,"mpoint")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,"int")){
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
  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(relDescription)),
      nl->SymbolValue(attrName),attrType);
  CHECK_COND((j > 0) && (nl->IsEqual(attrType,"upoint")),"expect upoint");
  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom("stream"),
      nl->Second(nl->Second(args)));

  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
          nl->OneElemList(nl->IntAtom(j)),res);

}

/*
The function rknearestFilterTypeMap is the type map for the
operator knearestfilter

*/
ListExpr rknearestFilterTypeMap( ListExpr args )
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
  if(!nl->IsEqual(mpoint,"mpoint")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // the third element has to be of type mpoint
  if(!nl->IsEqual(quantity,"int")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  // an tbtree description must have length 4
  if(nl->ListLength(tbtreeDescription)!=4){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  ListExpr tbtreeSymbol = nl->First(tbtreeDescription);

  if(!nl->IsEqual(tbtreeSymbol, "rtree3")){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  if(!IsRelDescription(relDescription)){
    ErrorReporter::ReportError(errmsg);
    return nl->TypeError();
  }

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(relDescription)),
      nl->SymbolValue(attrName),attrType);
  CHECK_COND((j > 0) && (nl->IsEqual(attrType,"upoint")),"expect upoint");
  ListExpr res = nl->TwoElemList(
      nl->SymbolAtom("stream"),
      nl->Second(nl->Second(args)));

  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
          nl->OneElemList(nl->IntAtom(j)),res);
}

Operator greeceknearest (
         "greeceknearest",        // name
         RknearestfilterSpec,      // specification
         rknearest<double>,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         rknearestFilterTypeMap    // type mapping
);
Operator hcknnknearest (
         "hcknnknearest",        // name
         hcknnknearestSpec,      // specification
         hcknnknearestFun<double>,// value mapping
         Operator::SimpleSelect,  // trivial selection function
         hcknnknearestTypeMap    // type mapping
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
    AddOperator( &mqknearest);
    AddOperator( &covleafnode);
    AddOperator( &greeceknearest);
    AddOperator( &hcknnknearest);

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

