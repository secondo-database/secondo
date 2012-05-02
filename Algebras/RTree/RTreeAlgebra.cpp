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

[1] Implementation of R-Tree Algebra

July 2003, Victor Almeida

October 2004, Herbert Schoenhammer, tested and divided in Header-File
and Implementation-File. Also R-Trees with three and four dimensions
are created.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2007, Christian Duentgen added operator for bulk loading
R-trees. Also added several operators for rtree insprospection.

October 2009, Christian Duentgen corrected type mapping for ~gettuples~ and
~gettuples2~ and replaced weird 'CHECK\_COND' macro in all type mappings.

[TOC]

0 Overview

This Algebra implements the creation of two-, three- and four-dimensional
R-Trees.

First the the type constructor ~rtree~, ~rtree3~ and ~rtree4~ are defined.

Second the operator ~creatertree~ to build the trees is defined. This operator
expects an attribute in the relation that implements the kind ~SPATIAL2D~,
~SPATIAL3D~, or ~SPATIAL4D~. Only the bounding boxes of the values of this
attribute are indexed in the R-Tree.

Finally, the operator ~windowintersects~ retrieves the tuples of the original
relation which bounding boxes of the indexed attribute intersect the window
(of type ~rect~, ~rect3~, or ~rect4~) given as argument to the operator.

1 Defines and Includes

*/
#include <iostream>
#include <stack>
#include <limits>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"
#include "RTreeAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "TupleIdentifier.h"
#include "Messages.h"
#include "Progress.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "AlmostEqual.h"
#include "Symbols.h"
#include "Stream.h"
#include "LRU.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle

/*
Implementation of Functions and Procedures

*/
int myCompare( const void* a, const void* b )
{
  if( ((SortedArrayItem *) a)->pri < ((SortedArrayItem *) b)->pri )
    return -1;
  else if( ((SortedArrayItem *) a)->pri >
           ((SortedArrayItem *) b)->pri )
    return 1;
  else
    return 0;
}

/*
1 Type constructor ~rtree~

1.1 Type property of type constructor ~rtree~

*/
ListExpr RTree2Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> creatertree [<attrname>]"
    " where <attrname> is the key of type rect");

  return
    (nl->TwoElemList(
         nl->TwoElemList(nl->StringAtom("Creation"),
                         nl->StringAtom("Example Creation")),
         nl->TwoElemList(examplelist,
                         nl->StringAtom("(let myrtree = countries"
                         " creatertree [boundary])"))));
}






/*
1.8 ~Check~-function of type constructor ~rtree~

1.8.1 Auxiliary function for all RTree Versions

*/
bool CheckRTreeX(ListExpr type, ListExpr& errorInfo, const string & treeType,
                 const string& indexKIND, const string& indexType)
{
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 4)
    && listutils::isSymbol(nl->First(type), treeType)) {

    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind(Kind::TUPLE(), nl->Second(type), errorInfo) &&
      (nl->IsEqual(nl->Third(type), indexType) ||
      algMgr->CheckKind(indexKIND, nl->Third(type), errorInfo)) &&
      nl->IsAtom(nl->Fourth(type)) &&
      nl->AtomType(nl->Fourth(type)) == BoolType;
  } else {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60),
        nl->SymbolAtom(treeType),
        type));
    return false;
  }
  return true;
}


bool CheckRTree2(ListExpr type, ListExpr& errorInfo){
  return CheckRTreeX(type, errorInfo,RTree2TID::BasicType(), Kind::SPATIAL2D(),
                     Rectangle<2>::BasicType());
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree( RTree2TID::BasicType(),
                        RTree2Prop,
                        OutRTree<2>,
                        InRTree<2>,
                        0,
                        0,
                        CreateRTree<2>,
                        DeleteRTree<2>,
                        OpenRTree<2>,
                        SaveRTree<2>,
                        CloseRTree<2>,
                        CloneRTree<2>,
                        CastRTree<2>,
                        SizeOfRTree<2>,
                        CheckRTree2 );

/*
2 Type constructor ~rtree3~

2.1 Type property of type constructor ~rtree3~

*/
ListExpr RTree3Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> creatertree [<attrname>]"
    " where <attrname> is the key of type rect3");

  return
    (nl->TwoElemList(
         nl->TwoElemList(nl->StringAtom("Creation"),
                         nl->StringAtom("Example Creation")),
         nl->TwoElemList(examplelist,
                         nl->StringAtom("(let myrtree = countries"
                         " creatrtree [boundary])"))));
}

/*
2.8 ~Check~-function of type constructor ~rtree3~

*/
bool CheckRTree3(ListExpr type, ListExpr& errorInfo)
{
  return CheckRTreeX(type, errorInfo,RTree3TID::BasicType(), Kind::SPATIAL3D(),
                     Rectangle<3>::BasicType());
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree3( RTree3TID::BasicType(),
                        RTree3Prop,
                        OutRTree<3>,
                        InRTree<3>,
                        0,
                        0,
                        CreateRTree<3>,
                        DeleteRTree<3>,
                        OpenRTree<3>,
                        SaveRTree<3>,
                        CloseRTree<3>,
                        CloneRTree<3>,
                        CastRTree<3>,
                        SizeOfRTree<3>,
                        CheckRTree3 );

/*
3 Type constructor ~rtree4~

3.1 Type property of type constructor ~rtree4~

*/
ListExpr RTree4Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> creatertree [<attrname>]"
    " where <attrname> is the key of type rect4");

  return (nl->TwoElemList(
            nl->TwoElemList(
              nl->StringAtom("Creation"),
              nl->StringAtom("Example Creation")),
            nl->TwoElemList(
              examplelist,
              nl->StringAtom("(let myrtree = countries"
                             " creatertree [boundary])"))));
}

/*
3.8 ~Check~-function of type constructor ~rtree4~

*/
bool CheckRTree4(ListExpr type, ListExpr& errorInfo)
{
  return CheckRTreeX(type, errorInfo,RTree4TID::BasicType(), Kind::SPATIAL4D(),
                     Rectangle<4>::BasicType());
}

/*
3.12 Type Constructor object for type constructor ~rtree~

*/
TypeConstructor rtree4( RTree4TID::BasicType(),
                        RTree4Prop,
                        OutRTree<4>,
                        InRTree<4>,
                        0,
                        0,
                        CreateRTree<4>,
                        DeleteRTree<4>,
                        OpenRTree<4>,
                        SaveRTree<4>,
                        CloseRTree<4>,
                        CloneRTree<4>,
                        CastRTree<4>,
                        SizeOfRTree<4>,
                        CheckRTree4 );

/*
3 Type constructor ~rtree8~

3.1 Type property of type constructor ~rtree8~

*/
ListExpr RTree8Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
    "<relation> creatertree [<attrname>]"
    " where <attrname> is the key of type rect8");

  return (nl->TwoElemList(
            nl->TwoElemList(
              nl->StringAtom("Creation"),
              nl->StringAtom("Example Creation")),
            nl->TwoElemList(
              examplelist,
              nl->StringAtom("(let myrtree = countries"
                             " creatertree [boundary])"))));
}

/*
3.8 ~Check~-function of type constructor ~rtree8~

*/
bool CheckRTree8(ListExpr type, ListExpr& errorInfo)
{
  return CheckRTreeX(type, errorInfo,RTree4TID::BasicType(), Kind::SPATIAL8D(),
                     Rectangle<8>::BasicType());
}

/*
3.12 Type Constructor object for type constructor ~rtree8~

*/
TypeConstructor rtree8( RTree8TID::BasicType(),
                        RTree8Prop,
                        OutRTree<8>,
                        InRTree<8>,
                        0,
                        0,
                        CreateRTree<8>,
                        DeleteRTree<8>,
                        OpenRTree<8>,
                        SaveRTree<8>,
                        CloseRTree<8>,
                        CloneRTree<8>,
                        CastRTree<8>,
                        SizeOfRTree<8>,
                        CheckRTree8 );

/*
7 Operators of the RTree Algebra

7.1 Operator ~creatertree~

The operator ~creatrtree~ creates a R-Tree for a given Relation. The exact type
of the desired R-Tree is defind in RTreeAlgebra.h. The variables ~do\_linear\_split~,
~do\_quadratic\_split~ or ~do\_axis\_split~ are defining, whether a R-Tree (Guttman 84) or
a R[*]-Tree is generated.

The following operator ~creatertree~ accepts relations with tuples of (at least) one
attribute of kind ~SPATIAL2D~, ~SPATIAL3D~ or ~SPATIAL4D~ or ~rect~, ~rect3~, and ~rect4~.
The attribute name is specified as argument of the operator.

7.1.1 Type Mapping of operator ~creatertree~

*/
ListExpr CreateRTreeTypeMap(ListExpr args)
{
  // Check for correct general argument list format
  if(nl->IsEmpty(args) || !(nl->ListLength(args) == 2)){
    return listutils::typeError("Expects exactly 2 arguments.");
  }
  ListExpr relDescription = nl->First(args),
           attrNameLE = nl->Second(args);

  if(!listutils::isSymbol(attrNameLE)){
    return listutils::typeError("Expects key attribute name as 2nd argument.");
  }
  string attrName = (nl->SymbolValue(attrNameLE));

  // Check for relation or tuplestream as first argument
  if( !(    listutils::isRelDescription(relDescription)
         || listutils::isTupleStream(relDescription)
       )
    ){
    return listutils::typeError("Expects a relation or tulestream as 1st "
                                "argument.");
  }

  // Test for index attribute
  ListExpr tupleDescription = nl->Second(relDescription);
  ListExpr attrList = nl->Second(tupleDescription);
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if(attrIndex <= 0){
    return listutils::typeError("Expects key attribute (2nd argument) to be a "
      "member of the relation/ stream (1st argument).");
  }
  if( !(    listutils::isSpatialType(attrType)
         || listutils::isRectangle(attrType)
       )
    ){
    return listutils::typeError("Expects key attribute to be of kind SPATIAL,"
        "SPATIAL3D, SPATIAL4D, SPATIAL8D, or have type rect, rect3, rect4, or "
        "rect8");
  }
  string rtreetype;
  if (    listutils::isKind(attrType, Kind::SPATIAL2D())
       || listutils::isSymbol(attrType, Rectangle<2>::BasicType()))
    rtreetype = RTree2TID::BasicType();
  else if (    listutils::isKind(attrType, Kind::SPATIAL3D())
            || listutils::isSymbol(attrType, Rectangle<3>::BasicType()))
    rtreetype = RTree3TID::BasicType();
  else if (    listutils::isKind(attrType, Kind::SPATIAL4D())
            || listutils::isSymbol(attrType, Rectangle<4>::BasicType()))
    rtreetype = RTree4TID::BasicType();
  else if (    listutils::isKind(attrType, Kind::SPATIAL8D())
            || listutils::isSymbol(attrType, Rectangle<8>::BasicType()))
    rtreetype = RTree8TID::BasicType();
  if( nl->IsEqual(nl->First(relDescription), Relation::BasicType()) )
  {
    return
      nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->FourElemList(
          nl->SymbolAtom(rtreetype),
          tupleDescription,
          attrType,
          nl->BoolAtom(false)));
  }
  else // nl->IsEqual(nl->First(relDescription), Symbol::STREAM())
  {
/*
Here we can have two possibilities:

- multi-entry indexing, or
- double indexing

For multi-entry indexing, one and only one of the attributes
must be a tuple identifier. In the latter, together with
a tuple identifier, the last two attributes must be of
integer type (~int~).

In the first case, a standard R-Tree is created possibly
containing several entries to the same tuple identifier, and
in the latter, a double index R-Tree is created using as low
and high parameters these two last integer numbers.

*/
    ListExpr first, rest, newAttrList, lastNewAttrList;
    int tidIndex = 0;
    string type;
    bool firstcall = true,
         doubleIndex = false;

    int nAttrs = nl->ListLength( attrList );
    rest = attrList;
    int j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      type = nl->SymbolValue(nl->Second(first));
      if (type == TupleIdentifier::BasicType())
      {
        if(tidIndex != 0){
          return listutils::typeError("Expects exactly one arribute of type "
                                      "'tid' within the 1st argument.");
        }
        tidIndex = j;
      }
      else if( j == nAttrs - 1 && type == CcInt::BasicType() &&
               nl->SymbolValue(
                 nl->Second(nl->First(rest))) == CcInt::BasicType() )
      { // the last two attributes are integers
        doubleIndex = true;
      }
      else
      {
        if (firstcall)
        {
          firstcall = false;
          newAttrList = nl->OneElemList(first);
          lastNewAttrList = newAttrList;
        }
        else
        {
          lastNewAttrList = nl->Append(lastNewAttrList, first);
        }
      }
      j++;
    }
    if(tidIndex <= 0){
      return listutils::typeError("Exects exactly one attribute of type 'tid'"
                                  " within 1st argument.");
    }

    return
      nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->FourElemList(
          nl->SymbolAtom(rtreetype),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            newAttrList),
          attrType,
          nl->BoolAtom(doubleIndex)));
  }
}

/*
4.1.2 Selection function of operator ~creatertree~

*/
int
CreateRTreeSelect (ListExpr args)
{
  ListExpr relDescription = nl->First(args),
           attrNameLE = nl->Second(args),
           tupleDescription = nl->Second(relDescription),
           attrList = nl->Second(tupleDescription);
  string attrName = nl->SymbolValue(attrNameLE);

  int attrIndex;
  ListExpr attrType;
  attrIndex = FindAttribute(attrList, attrName, attrType);

  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  int result;
  if ( algMgr->CheckKind(Kind::SPATIAL2D(), attrType, errorInfo) )
    result = 0;
  else if ( algMgr->CheckKind(Kind::SPATIAL3D(), attrType, errorInfo) )
    result = 1;
  else if ( algMgr->CheckKind(Kind::SPATIAL4D(), attrType, errorInfo) )
    result = 2;
  else if ( algMgr->CheckKind(Kind::SPATIAL8D(), attrType, errorInfo) )
    result = 3;
  else if( nl->SymbolValue(attrType) == Rectangle<2>::BasicType() )
    result = 4;
  else if( nl->SymbolValue(attrType) == Rectangle<3>::BasicType() )
    result = 5;
  else if( nl->SymbolValue(attrType) == Rectangle<4>::BasicType() )
    result = 6;
  else if( nl->SymbolValue(attrType) == Rectangle<8>::BasicType() )
    result = 7;
  else
    return -1; /* should not happen */

  if( nl->SymbolValue(nl->First(relDescription)) == Relation::BasicType())
    return result;
  if( nl->SymbolValue(nl->First(relDescription)) == Symbol::STREAM())
  {
    ListExpr first,
             rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
    if( nl->IsEqual( nl->Second( first ), CcInt::BasicType() ) )
      // Double indexing
      return result + 16;
    else
      // Multi-entry indexing
      return result + 8;
  }

  return -1;
}

/*
4.1.3 Value mapping function of operator ~creatertree~

*/
template<unsigned dim>
int CreateRTreeRelSpatial(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  Relation* relation;
  int attrIndex;
  GenericRelationIterator* iter;
  Tuple* tuple;

  result = qp->ResultStorage(s);

  R_Tree<dim, TupleId>*rtree = static_cast<R_Tree<dim, TupleId>*>(result.addr);

  relation = (Relation*)args[0].addr;
  attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    if( ((StandardSpatialAttribute<dim>*)(tuple->
          GetAttribute(attrIndex)))->IsDefined() )
    {
      BBox<dim> box = ((StandardSpatialAttribute<dim>*)(tuple->
                        GetAttribute(attrIndex)))->BoundingBox();
/*if(dim == 3){
            double min[3];
            double max[3];
            for(int i = 0;i < 3;i++){
              min[i] = box.MinD(i);
              max[i] = box.MaxD(i);
            }
            min[2] = min[2]*864000;
            max[2] = max[2]*864000;
            box.Set(true,min,max);
        }*/
      R_TreeLeafEntry<dim, TupleId> e( box, tuple->GetTupleId() );
      rtree->Insert( e );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

template<unsigned dim>
int CreateRTreeStreamSpatial(Word* args, Word& result, int message,
                             Word& local, Supplier s)
{
  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
        ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() )
    {
      BBox<dim> box = ((StandardSpatialAttribute<dim>*)tuple->
                        GetAttribute(attrIndex))->BoundingBox();
      R_TreeLeafEntry<dim, TupleId>
        e( box,
           ((TupleIdentifier *)tuple->
             GetAttribute(tidIndex))->GetTid() );
      rtree->Insert( e );
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

template<unsigned dim>
int CreateRTreeRelRect(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  Relation* relation;
  int attrIndex;
  GenericRelationIterator* iter;
  Tuple* tuple;

  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );
  relation = (Relation*)args[0].addr;
  attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  iter = relation->MakeScan();
  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
    if( box->IsDefined() )
    {
      R_TreeLeafEntry<dim, TupleId> e( *box, tuple->GetTupleId() );
      rtree->Insert( e );
    }
    tuple->DeleteIfAllowed();
  }
  delete iter;

  return 0;
}

template<unsigned dim>
int CreateRTreeStreamRect(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
        ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() )
    {
      BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
      if( box->IsDefined() )
      {
        R_TreeLeafEntry<dim, TupleId>
          e( *box,
             ((TupleIdentifier *)tuple->
               GetAttribute(tidIndex))->GetTid() );
        rtree->Insert( e );
      }
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

template<unsigned dim>
int CreateRTree2LSpatial(Word* args, Word& result, int message,
                         Word& local, Supplier s)
{
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
    (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
        ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() &&
        ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-2))->
          IsDefined() &&
        ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-1))->
          IsDefined() )
    {
      BBox<dim> box =
        ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->BoundingBox();
      R_TreeLeafEntry<dim, TwoLayerLeafInfo> e(
        box,
        TwoLayerLeafInfo(
          ((TupleIdentifier *)tuple->
            GetAttribute(tidIndex))->GetTid(),
          ((CcInt*)tuple->
            GetAttribute(tuple->GetNoAttributes()-2))->GetIntval(),
          ((CcInt*)tuple->
            GetAttribute(tuple->GetNoAttributes()-1))->GetIntval()));
      rtree->Insert( e );
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

template<unsigned dim>
int CreateRTree2LRect(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
    (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
      tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
        ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() &&
        ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-2))->
          IsDefined() &&
        ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-1))->
          IsDefined() )
    {
      BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
      if( box->IsDefined() )
      {
        R_TreeLeafEntry<dim, TwoLayerLeafInfo>
          e( *box,
             TwoLayerLeafInfo(
               ((TupleIdentifier *)tuple->
                 GetAttribute(tidIndex))->GetTid(),
               ((CcInt*)tuple->GetAttribute(
                 tuple->GetNoAttributes()-2))->GetIntval(),
               ((CcInt*)tuple->GetAttribute(
                 tuple->GetNoAttributes()-1))->GetIntval() ) );
        rtree->Insert( e );
      }
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  return 0;
}

/*
4.1.5 Definition of value mapping vectors

*/
ValueMapping rtreecreatertreemap [] = { CreateRTreeRelSpatial<2>,
                                        CreateRTreeRelSpatial<3>,
                                        CreateRTreeRelSpatial<4>,
                                        CreateRTreeRelSpatial<8>,
                                        CreateRTreeRelRect<2>,
                                        CreateRTreeRelRect<3>,
                                        CreateRTreeRelRect<4>,
                                        CreateRTreeRelRect<8>,
                                        CreateRTreeStreamSpatial<2>,
                                        CreateRTreeStreamSpatial<3>,
                                        CreateRTreeStreamSpatial<4>,
                                        CreateRTreeStreamSpatial<8>,
                                        CreateRTreeStreamRect<2>,
                                        CreateRTreeStreamRect<3>,
                                        CreateRTreeStreamRect<4>,
                                        CreateRTreeStreamRect<8>,
                                        CreateRTree2LSpatial<2>,
                                        CreateRTree2LSpatial<3>,
                                        CreateRTree2LSpatial<4>,
                                        CreateRTree2LSpatial<8>,
                                        CreateRTree2LRect<2>,
                                        CreateRTree2LRect<3>,
                                        CreateRTree2LRect<4>,
                                        CreateRTree2LRect<8> };

/*
4.1.6 Specification of operator ~creatertree~

*/

const string CreateRTreeSpec  =
  "( ( \"1st Signature\""
  " \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>((rel (tuple (x1 t1)...(xn tn)))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)\n"
  "((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)\n"
  "((stream (tuple (x1 t1)...(xn tn) "
  "(id tid)(low int)(high int))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti true)</text--->"
  "<text>_ creatertree [ _ ]</text--->"
  "<text>Creates an rtree<d>. The key type ti must "
  "be of kind SPATIAL2D, SPATIAL3D, SPATIAL4D or Spatial8D, "
  "or of type rect, rect2, rect3, rect4 or rect8.</text--->"
  "<text>let myrtree = Kreis feed extend[id: tupleid(.)] "
  "creatertree[Gebiet]</text--->"
  ") )";

/*
4.1.7 Definition of operator ~creatertree~

*/
Operator creatertree (
          "creatertree",       // name
          CreateRTreeSpec,     // specification
          24,                  // Number of overloaded functions
          rtreecreatertreemap, // value mapping
          CreateRTreeSelect,   // trivial selection function
          CreateRTreeTypeMap   // type mapping
);

/*
7.2 Operator ~windowintersects~

7.2.1 Type mapping function of operator ~windowintersects~

*/
ListExpr WindowIntersectsTypeMap(ListExpr args)
{

  if( nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 3){
    return listutils::typeError("Expects exactly 3 arguments.");
  }
  /* Split argument in three parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);
  // first must be an rtree<dim>
  if(!listutils::isRTreeDescription(rtreeDescription)){
    return listutils::typeError("Expects 1st argument to be of type "
                                "'rtree<dim>(<tuple-type>,bool)'.");
  }
  // second a relation
  if(!listutils::isRelDescription(relDescription)){
    return listutils::typeError("Expects 2nd argument to be of type "
                                "'rel(<tuple-type>)'.");
  }
  // third a type with an MBR
  if(!(    listutils::isSpatialType(searchWindow)
        || listutils::isRectangle(searchWindow))){
    return listutils::typeError("Expects 3nd argument to be of a type of kind "
      "SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D; or of type 'rect', 'rect3', "
      "'rect4', or 'rect8'.");
  }
  // check that rtree and rel have the same associated tuple type
  ListExpr tupleDescription = nl->Second(relDescription),
           rtreeKeyType = nl->Third(rtreeDescription);
  if(!(    listutils::isSpatialType(rtreeKeyType)
        || listutils::isRectangle(rtreeKeyType))){
    return listutils::typeError("Expects key type of the rtree<dim> passed as "
      "1st argument to to be of a type of kind "
      "SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D; or of type 'rect', 'rect3', "
      "'rect4', or 'rect8'.");
  }
  // This test is omitted so that an R-tree built by bulk load
  // can be used with this operator. (RHG 13.6.2008)
  // ListExpr rtreeTupleDescription = nl->Second(rtreeDescription):
  //if(!(nl->Equal(rtreeTupleDescription, rtreeAttrList))){
  // return listutils::typeError("Expects matching tuple-types for 1st and "
  //                             "2nd argument.");
  //}
  // Check whether key type and rtree dimension match
  if( !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
             && listutils::isSymbol(searchWindow, Rectangle<2>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
             && listutils::isKind(searchWindow, Kind::SPATIAL2D()))
         || (listutils::isSymbol(searchWindow, Rectangle<2>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<2>::BasicType()))
       ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
             && listutils::isSymbol(searchWindow, Rectangle<3>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
             && listutils::isKind(searchWindow, Kind::SPATIAL3D()))
         || (listutils::isSymbol(searchWindow, Rectangle<3>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<3>::BasicType()))
       ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
             && listutils::isSymbol(searchWindow, Rectangle<4>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
             && listutils::isKind(searchWindow, Kind::SPATIAL4D()))
         || (listutils::isSymbol(searchWindow, Rectangle<4>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<4>::BasicType()))
       ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
             && listutils::isSymbol(searchWindow, Rectangle<8>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
             && listutils::isKind(searchWindow, Kind::SPATIAL8D()))
         || (listutils::isSymbol(searchWindow, Rectangle<8>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<8>::BasicType()))
       )
    ){
    return listutils::typeError("Expects same dimensions for the rtree-type "
                        "(1st argument) and for the key type (3rd argument).");
  }
  return
    nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      tupleDescription);
}

/*
5.1.2 Selection function of operator ~windowintersects~

*/
int
WindowIntersectsSelection( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr searchWindow = nl->Third(args),
           errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

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
           algMgr->CheckKind(Kind::SPATIAL4D(), searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
}

/*
5.1.3 Value mapping function of operator ~windowintersects~

*/

#ifndef USE_PROGRESS

// standard version

template <unsigned dim>
struct WindowIntersectsLocalInfo
{
  Relation* relation;
  R_Tree<dim, TupleId>* rtree;
  BBox<dim> *searchBox;
  bool first;
};

template <unsigned dim>
int WindowIntersects( Word* args, Word& result,
                      int message, Word& local,
                      Supplier s )
{
  WindowIntersectsLocalInfo<dim> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new WindowIntersectsLocalInfo<dim>;
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[2].addr)->
            BoundingBox()) );

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;
      R_TreeLeafEntry<dim, TupleId> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
        if(localInfo->first)
        {
          localInfo->first = false;
          if( localInfo->rtree->First( *localInfo->searchBox, e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info);
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
        else
        {
          if( localInfo->rtree->Next( e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info);
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
      }
    }

    case CLOSE :
    {
      if(local.addr)
      {
        localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;
        delete localInfo->searchBox;
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}



# else

// progress version

template <unsigned dim>
class WindowIntersectsLocalInfo: public ProgressLocalInfo
{
public:
  Relation* relation;
  R_Tree<dim, TupleId>* rtree;
  BBox<dim> *searchBox;
  bool first;
  int completeCalls;
  int completeReturned;
};

template <unsigned dim>
int WindowIntersects( Word* args, Word& result,
                      int message, Word& local,
                      Supplier s )
{
  WindowIntersectsLocalInfo<dim> *localInfo;
  localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;

  switch (message)
  {
    case OPEN :
    {
      //local info kept over many calls of OPEN!
      //useful for loopjoin

      if ( !localInfo )  // first time
      {
        localInfo = new WindowIntersectsLocalInfo<dim>;
        localInfo->completeCalls = 0;
        localInfo->completeReturned = 0;

        localInfo->sizesInitialized = false;
        localInfo->sizesChanged = false;

        localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
        localInfo->relation = (Relation*)args[1].addr;
      }

      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[2].addr)->
            BoundingBox()) );

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      R_TreeLeafEntry<dim, TupleId> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
        if(localInfo->first)
        {
          localInfo->first = false;
          if( localInfo->rtree->First( *localInfo->searchBox, e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info,false);
            result.setAddr(tuple);
            localInfo->returned++;
            return YIELD;
          }
          else
            return CANCEL;
        }
        else
        {
          if( localInfo->rtree->Next( e ) )
          {
            Tuple *tuple = localInfo->relation->GetTuple(e.info, false);
            result.setAddr(tuple);
            localInfo->returned++;
            return YIELD;
          }
          else
            return CANCEL;
        }
      }
    }


    case CLOSE :
    {
      if(local.addr)
      {
        delete localInfo->searchBox;
        localInfo->completeCalls++;
        localInfo->completeReturned += localInfo->returned;
        localInfo->returned = 0;

        //Do not delete localInfo data structure in CLOSE!
        //To be kept over several
        //calls for correct progress estimation when embedded in a loopjoin.
        //Is deleted at the end of the query in CLOSEPROGRESS.
      }
      return 0;
    }


    case CLOSEPROGRESS :
    {
      if(local.addr)
      {
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }


    case REQUESTPROGRESS :
    {
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      //Experiments described in file Cost functions
      const double uSearch = 0.45;    //milliseconds per search
      const double vResult = 0.092;  //milliseconds per result tuple

      if (!localInfo) return CANCEL;
      else
      {
        localInfo->sizesChanged = false;

        if ( !localInfo->sizesInitialized )
        {
          localInfo->returned = 0;
          localInfo->total = localInfo->relation->GetNoTuples();
          localInfo->defaultValue = 50;
          localInfo->Size = 0;
          localInfo->SizeExt = 0;
          localInfo->noAttrs =
            nl->ListLength(nl->Second(nl->Second(qp->GetType(s))));
          localInfo->attrSize = new double[localInfo->noAttrs];
          localInfo->attrSizeExt = new double[localInfo->noAttrs];
          for ( int i = 0;  i < localInfo->noAttrs; i++)
          {
            localInfo->attrSize[i] = localInfo->relation->GetTotalSize(i)
                                  / (localInfo->total + 0.001);
            localInfo->attrSizeExt[i] = localInfo->relation->GetTotalExtSize(i)
                                  / (localInfo->total + 0.001);

            localInfo->Size += localInfo->attrSize[i];
            localInfo->SizeExt += localInfo->attrSizeExt[i];
          }
          localInfo->sizesInitialized = true;
          localInfo->sizesChanged = true;
        }

        pRes->CopySizes(localInfo);

	if ( localInfo->completeCalls > 0 )     //called in a loopjoin
        {
          pRes->Card =
            (double) localInfo->completeReturned /
            (double) localInfo->completeCalls;
        }
        else	//single or first call
        {
          if (fabs(qp->GetSelectivity(s) - 0.1) < 0.000001) // default
            pRes->Card = (double) localInfo->defaultValue;
          else                                              // annotated
            pRes->Card = localInfo->total * qp->GetSelectivity(s);

          if ((double) localInfo->returned > pRes->Card) // more tuples
            pRes->Card = (double) localInfo->returned; // than calculated

          if (!localInfo->searchBox)         // rtree has been finished, but
            pRes->Card = pRes->Card * 1.1; // there are some tuples more (10%)

          if (pRes->Card > (double) localInfo->total) // more than all cannot be
            pRes->Card = (double) localInfo->total;
	}

        pRes->Time = uSearch + pRes->Card * vResult;

        pRes->Progress = (localInfo->returned + 1) / pRes->Card;

        return YIELD;
      }
    }
  }
  return 0;
}

#endif


/*
5.1.4 Definition of value mapping vectors

*/
ValueMapping rtreewindowintersectsmap [] = { WindowIntersects<2>,
                                             WindowIntersects<3>,
                                             WindowIntersects<4>,
                                             WindowIntersects<8> };


/*
5.1.5 Specification of operator ~windowintersects~

*/
const string windowintersectsSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T= rect<d> and ti in {rect<d>} U SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_ _ windowintersects [ _ ]</text--->"
      "<text>Uses the given rtree to find all tuples"
      " in the given relation with .xi intersects the "
      " argument value's bounding box.</text--->"
      "<text>query citiesInd cities windowintersects"
      " [r] consume; where citiesInd "
      "is e.g. created with 'let citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~windowintersects~

*/
Operator windowintersects (
         "windowintersects",        // name
         windowintersectsSpec,      // specification
         4,                         //number of overloaded functions
         rtreewindowintersectsmap,  // value mapping
         WindowIntersectsSelection, // trivial selection function
         WindowIntersectsTypeMap    // type mapping
);


/*
7.2 Operator ~windowintersectsS~

7.2.1 Type mapping function of operator ~windowintersectsS~

*/
ListExpr WindowIntersectsSTypeMap(ListExpr args)
{
  if( nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expects exactly 2 arguments.");
  }
  /* Split argument in two parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr searchWindow = nl->Second(args);
  // first must be an rtree<dim>
  if(!listutils::isRTreeDescription(rtreeDescription)){
    return listutils::typeError("Expects 1st argument to be of type "
                                "'rtree<dim>(<tuple-type>,bool)'.");
  }
  // second a type with an MBR
  if(!(    listutils::isSpatialType(searchWindow)
        || listutils::isRectangle(searchWindow))){
    return listutils::typeError("Expects 2nd argument to be of a type of kind "
      "SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D; or of type 'rect', 'rect3', "
      "'rect4', or 'rect8'.");
  }
  // check for supported rtree dimensionality
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);
  if(!(    listutils::isSpatialType(rtreeKeyType)
        || listutils::isRectangle(rtreeKeyType))){
    return listutils::typeError("Expects key type of the rtree<dim> passed as "
      "1st argument to to be of a type of kind "
      "SPATIAL, SPATIAL3D, SPATIAL4D, SPATIAL8D; or of type 'rect', 'rect3', "
      "'rect4', or 'rect8'.");
  }
  // Check whether key type and rtree dimension match
  if( !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
             && listutils::isSymbol(searchWindow, Rectangle<2>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
             && listutils::isKind(searchWindow, Kind::SPATIAL2D()))
         || (listutils::isSymbol(searchWindow, Rectangle<2>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<2>::BasicType()))
    ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
             && listutils::isSymbol(searchWindow, Rectangle<3>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
             && listutils::isKind(searchWindow, Kind::SPATIAL3D()))
         || (listutils::isSymbol(searchWindow, Rectangle<3>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<3>::BasicType()))
       ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
             && listutils::isSymbol(searchWindow, Rectangle<4>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
             && listutils::isKind(searchWindow, Kind::SPATIAL4D()))
         || (listutils::isKind(searchWindow, Rectangle<4>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<4>::BasicType()))
       ) &&
      !(   (listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
             && listutils::isSymbol(searchWindow, Rectangle<8>::BasicType()))
         || (listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
             && listutils::isKind(searchWindow, Kind::SPATIAL8D()))
         || (listutils::isKind(searchWindow, Rectangle<8>::BasicType())
             && listutils::isSymbol(rtreeKeyType, Rectangle<8>::BasicType()))
       )
    ){
    return listutils::typeError("Expects same dimensions for the rtree-type "
                        "(1st argument) and for the key type (2nd argument).");
  }
  // construct return type
  if( nl->BoolValue(nl->Fourth(rtreeDescription)) )
    return
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Id"),
              nl->SymbolAtom(TupleIdentifier::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("Low"),
              nl->SymbolAtom(CcInt::BasicType())),
            nl->TwoElemList(
              nl->SymbolAtom("High"),
              nl->SymbolAtom(CcInt::BasicType())))));
  else
    return
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          nl->OneElemList(
            nl->TwoElemList(
              nl->SymbolAtom("Id"),
              nl->SymbolAtom(TupleIdentifier::BasicType())))));
}

/*
5.1.2 Selection function of operator ~windowintersectsS~

*/
int
WindowIntersectsSSelection( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr searchWindow = nl->Second(args),
           errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool doubleIndex = nl->BoolValue(nl->Fourth(nl->First(args)));

  if (nl->SymbolValue(searchWindow) == Rectangle<2>::BasicType() ||
      algMgr->CheckKind(Kind::SPATIAL2D(), searchWindow, errorInfo))
    return 0 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == Rectangle<3>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL3D(), searchWindow, errorInfo))
    return 1 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == Rectangle<4>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL4D(), searchWindow, errorInfo))
    return 2 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == Rectangle<8>::BasicType() ||
           algMgr->CheckKind(Kind::SPATIAL8D(), searchWindow, errorInfo))
    return 3 + (!doubleIndex ? 0 : 4);

  return -1; /* should not happen */
}

/*
5.1.3 Value mapping function of operator ~windowintersectsS~

*/


#ifndef USE_PROGRESS

// standard version


template <unsigned dim, class LeafInfo>
struct WindowIntersectsSLocalInfo
{
  R_Tree<dim, LeafInfo>* rtree;
  BBox<dim> *searchBox;
  TupleType *resultTupleType;
  bool first;
};

template <unsigned dim>
int WindowIntersectsSStandard( Word* args, Word& result,
                               int message, Word& local,
                               Supplier s )
{
  switch (message)
  {
    case OPEN :
    {
      WindowIntersectsSLocalInfo<dim, TupleId> *localInfo =
        new WindowIntersectsSLocalInfo<dim, TupleId>();
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[1].addr)->
            BoundingBox()) );
      localInfo->resultTupleType =
        new TupleType(nl->Second(GetTupleResultType(s)));
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      WindowIntersectsSLocalInfo<dim, TupleId> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TupleId>*)local.addr;
      R_TreeLeafEntry<dim, TupleId> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
        if(localInfo->first)
        {
          localInfo->first = false;
          if( localInfo->rtree->First( *localInfo->searchBox, e ) )
          {
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
        else
        {
          if( localInfo->rtree->Next( e ) )
          {
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
            result.setAddr(tuple);
            return YIELD;
          }
          else
            return CANCEL;
        }
      }
    }

    case CLOSE :
    {
      if(local.addr)
      {
        WindowIntersectsSLocalInfo<dim, TupleId>* localInfo =
          (WindowIntersectsSLocalInfo<dim, TupleId>*)local.addr;
        delete localInfo->searchBox;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

template <unsigned dim>
int WindowIntersectsSDoubleLayer( Word* args, Word& result,
                                  int message, Word& local,
                                  Supplier s )
{
  switch (message)
  {
    case OPEN :
    {
      WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
        new WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>();
      localInfo->rtree =
        (R_Tree<dim, TwoLayerLeafInfo>*)args[0].addr;
      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[1].addr)->
            BoundingBox()) );
      localInfo->resultTupleType =
        new TupleType(nl->Second(GetTupleResultType(s)));
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)
        local.addr;
      R_TreeLeafEntry<dim, TwoLayerLeafInfo> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = new Tuple( localInfo->resultTupleType );
          tuple->PutAttribute(
            0, new TupleIdentifier( true, e.info.tupleId ) );
          tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
          tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
          result.setAddr(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
      else
      {
        if( localInfo->rtree->Next( e ) )
        {
          Tuple *tuple = new Tuple( localInfo->resultTupleType );
          tuple->PutAttribute(
            0, new TupleIdentifier( true, e.info.tupleId ) );
          tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
          tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
          result.setAddr(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
    }
}

    case CLOSE :
    {
      if(local.addr)
      {
        WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
          (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)
          local.addr;
        delete localInfo->searchBox;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

# else

// progress version

template <unsigned dim, class LeafInfo>
struct WindowIntersectsSLocalInfo: public ProgressLocalInfo
{
  R_Tree<dim, LeafInfo>* rtree;
  BBox<dim> *searchBox;
  TupleType *resultTupleType;
  bool first;
  int completeCalls;
  int completeReturned;
};

template <unsigned dim>
int WindowIntersectsSStandard( Word* args, Word& result,
                               int message, Word& local,
                               Supplier s )
{
  WindowIntersectsSLocalInfo<dim, TupleId> *localInfo;
  localInfo = (WindowIntersectsSLocalInfo<dim, TupleId>*)local.addr;

  switch (message)
  {
    case OPEN :
    {
      //local info kept over many calls of OPEN!
      //useful for loopjoin

      if ( !localInfo )  // first time
      {
        localInfo = new WindowIntersectsSLocalInfo<dim, TupleId>;
        localInfo->completeCalls = 0;
        localInfo->completeReturned = 0;

        localInfo->sizesInitialized = false;
        localInfo->sizesChanged = false;

        localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
        localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      }

      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[1].addr)->
            BoundingBox()) );

      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {

      R_TreeLeafEntry<dim, TupleId> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
        if(localInfo->first)
        {
          localInfo->first = false;
          if( localInfo->rtree->First( *localInfo->searchBox, e ) )
          {
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
            result.setAddr(tuple);
            localInfo->returned++;
            return YIELD;
          }
          else
            return CANCEL;
        }
        else
        {
          if( localInfo->rtree->Next( e ) )
          {
            Tuple *tuple = new Tuple( localInfo->resultTupleType );
            tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
            result.setAddr(tuple);
            localInfo->returned++;
            return YIELD;
          }
          else
            return CANCEL;
        }
      }
    }

    case CLOSE :
    {
      if( localInfo )
      {
        localInfo->completeCalls++;
        localInfo->completeReturned += localInfo->returned;
        localInfo->returned = 0;

        delete localInfo->searchBox;
        localInfo->searchBox = 0;

        //Do not delete localInfo data structure in CLOSE!
        //To be kept over several
        //calls for correct progress estimation when embedded in a loopjoin.
        //Is deleted at the end of the query in CLOSEPROGRESS.

      }
      return 0;
    }

    case CLOSEPROGRESS :
    {
      if( localInfo )
      {
	localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }


   case REQUESTPROGRESS :
    {
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      //Experiments described in file Cost functions
      const double uSearch = 0.45;    //milliseconds per search
      const double vResult = 0.019;  //milliseconds per result tuple

      if (!localInfo) return CANCEL;
      else
      {
        localInfo->sizesChanged = false;

        if ( !localInfo->sizesInitialized )
        {
          localInfo->returned = 0;
          localInfo->defaultValue = 50;
          localInfo->Size = 16;
          localInfo->SizeExt = 16;
          localInfo->noAttrs = 1;
          localInfo->attrSize = new double[localInfo->noAttrs];
          localInfo->attrSizeExt = new double[localInfo->noAttrs];
	  localInfo->attrSize[0] = 16;
	  localInfo->attrSizeExt[0] = 16;
          localInfo->sizesInitialized = true;
          localInfo->sizesChanged = true;
        }

        pRes->CopySizes(localInfo);

	if ( localInfo->completeCalls > 0 )     //called in a loopjoin
        {
          pRes->Card =
            (double) localInfo->completeReturned /
            (double) localInfo->completeCalls;
        }
        else	//single or first call
        {
          if (fabs(qp->GetSelectivity(s) - 0.1) < 0.000001) // default
            pRes->Card = (double) localInfo->defaultValue;
          else                                              // annotated
            pRes->Card = localInfo->total * qp->GetSelectivity(s);

          if ((double) localInfo->returned > pRes->Card) // more tuples
            pRes->Card = (double) localInfo->returned; // than calculated

          if (!localInfo->searchBox)         // rtree has been finished, but
            pRes->Card = pRes->Card * 1.1; // there are some tuples more (10%)

          if (pRes->Card > (double) localInfo->total) // more than all cannot be
            pRes->Card = (double) localInfo->total;
	}

        pRes->Time = uSearch + pRes->Card * vResult;

        pRes->Progress = (localInfo->returned + 1) / pRes->Card;

        return YIELD;
      }
    }
  }
  return 0;
}

template <unsigned dim>
int WindowIntersectsSDoubleLayer( Word* args, Word& result,
                                  int message, Word& local,
                                  Supplier s )
{
  WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo;
  localInfo = (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)local.addr;

  switch (message)
  {
    case OPEN :
    {
      //local info kept over many calls of OPEN!
      //useful for loopjoin

      if ( !localInfo )  // first time
      {
        localInfo = new WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>;
        localInfo->completeCalls = 0;
        localInfo->completeReturned = 0;

        localInfo->sizesInitialized = false;
        localInfo->sizesChanged = false;

        localInfo->rtree = (R_Tree<dim, TwoLayerLeafInfo>*)args[0].addr;
        localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      }

      localInfo->first = true;
      localInfo->searchBox =
        new BBox<dim> (
          (((StandardSpatialAttribute<dim> *)args[1].addr)->
            BoundingBox()) );

      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)
        local.addr;
      R_TreeLeafEntry<dim, TwoLayerLeafInfo> e;

      if ( !localInfo->searchBox->IsDefined() )
      { // search box is undefined -> no result!
        return CANCEL;
      }
      else
      {
      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = new Tuple( localInfo->resultTupleType );
          tuple->PutAttribute(
            0, new TupleIdentifier( true, e.info.tupleId ) );
          tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
          tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
          result.setAddr(tuple);
          localInfo->returned++;
          return YIELD;
        }
        else
          return CANCEL;
      }
      else
      {
        if( localInfo->rtree->Next( e ) )
        {
          Tuple *tuple = new Tuple( localInfo->resultTupleType );
          tuple->PutAttribute(
            0, new TupleIdentifier( true, e.info.tupleId ) );
          tuple->PutAttribute( 1, new CcInt( true, e.info.low ) );
          tuple->PutAttribute( 2, new CcInt( true, e.info.high ) );
          result.setAddr(tuple);
          localInfo->returned++;
          return YIELD;
        }
        else
          return CANCEL;
      }
    }
}

    case CLOSE :
    {
      if( localInfo )
      {
        localInfo->completeCalls++;
        localInfo->completeReturned += localInfo->returned;
        localInfo->returned = 0;

        delete localInfo->searchBox;

        //Do not delete localInfo data structure in CLOSE!
        //To be kept over several
        //calls for correct progress estimation when embedded in a loopjoin.
        //Is deleted at the end of the query in CLOSEPROGRESS.

      }
      return 0;
    }


    case CLOSEPROGRESS :
    {
      if( localInfo )
      {
	localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }


   case REQUESTPROGRESS :
    {
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      //Experiments described in file Cost functions
      const double uSearch = 0.45;    //milliseconds per search
      const double vResult = 0.02;  //milliseconds per result tuple
		// vResult to be adjusted

      if (!localInfo) return CANCEL;
      else
      {
        localInfo->sizesChanged = false;

        if ( !localInfo->sizesInitialized )
        {
          localInfo->returned = 0;
          localInfo->defaultValue = 50;
          localInfo->Size = 26;
          localInfo->SizeExt = 26;
          localInfo->noAttrs = 3;
          localInfo->attrSize = new double[localInfo->noAttrs];
          localInfo->attrSizeExt = new double[localInfo->noAttrs];
	  localInfo->attrSize[0] = 16;
	  localInfo->attrSizeExt[0] = 16;
	  localInfo->attrSize[1] = 5;
	  localInfo->attrSizeExt[1] = 5;
	  localInfo->attrSize[2] = 5;
	  localInfo->attrSizeExt[2] = 5;
          localInfo->sizesInitialized = true;
          localInfo->sizesChanged = true;
        }

        pRes->CopySizes(localInfo);

	if ( localInfo->completeCalls > 0 )     //called in a loopjoin
        {
          pRes->Card =
            (double) localInfo->completeReturned /
            (double) localInfo->completeCalls;
        }
        else	//single or first call
        {
          if (fabs(qp->GetSelectivity(s) - 0.1) < 0.000001) // default
            pRes->Card = (double) localInfo->defaultValue;
          else                                              // annotated
            pRes->Card = localInfo->total * qp->GetSelectivity(s);

          if ((double) localInfo->returned > pRes->Card) // more tuples
            pRes->Card = (double) localInfo->returned; // than calculated

          if (!localInfo->searchBox)         // rtree has been finished, but
            pRes->Card = pRes->Card * 1.1; // there are some tuples more (10%)

          if (pRes->Card > (double) localInfo->total) // more than all cannot be
            pRes->Card = (double) localInfo->total;
	}

        pRes->Time = uSearch + pRes->Card * vResult;

        pRes->Progress = (localInfo->returned + 1) / pRes->Card;

        return YIELD;
      }
    }

  }
  return 0;
}

#endif



/*
5.1.4 Definition of value mapping vectors

*/
ValueMapping rtreewindowintersectsSmap [] =
{ WindowIntersectsSStandard<2>,
  WindowIntersectsSStandard<3>,
  WindowIntersectsSStandard<4>,
  WindowIntersectsSStandard<8>,
  WindowIntersectsSDoubleLayer<2>,
  WindowIntersectsSDoubleLayer<3>,
  WindowIntersectsSDoubleLayer<4>,
  WindowIntersectsSDoubleLayer<8>};


/*
5.1.5 Specification of operator ~windowintersects~

*/
const string windowintersectsSSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>(rtree (tuple((x1 t1)...(xn tn)) ti) x T ->"
      " (stream (tuple ((id tid))))"
      "For T=rect<d> and ti in {rect<d>} U SPATIAL<d>D, where "
      " d in {2, 3, 4, 8}. </text--->"
      "<text>_ windowintersectsS [ _ ]</text--->"
      "<text>Uses the given rtree to find the tuple "
      "identifiers of all entries, whose bounding box intersects "
      "with the argument value's bounding box.</text--->"
      "<text>query citiesInd windowintersects"
      " [r] consume; where citiesInd "
      "is e.g. created with 'let citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~windowintersects~

*/
Operator windowintersectsS (
         "windowintersectsS",        // name
         windowintersectsSSpec,      // specification
         8,                         //number of overloaded functions
         rtreewindowintersectsSmap,  // value mapping
         WindowIntersectsSSelection, // trivial selection function
         WindowIntersectsSTypeMap    // type mapping
);

/*
7.2 Operator ~gettuples~

7.2.1 Type mapping function of operator ~gettuples~

*/
ListExpr GetTuplesTypeMap(ListExpr args)
{
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  // check for correct parameter list
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError(
      "\nExpects exactly 2 arguments.");
  }

  // Split arguments into two parts
  ListExpr streamDescription = nl->First(args),
           relDescription = nl->Second(args);

  // Handle the stream part of arguments
  if(!listutils::isTupleStream(streamDescription)){
    return listutils::typeError("Expects a valid tuplestream as 1st argument.");
  }
  // Handle the rel part of arguments
  if( !listutils::isRelDescription(relDescription) ){
    return listutils::typeError("Expects a valid relation as 2nd argument.");
  }

  // Check for existence of a single tid-attribute
  int tidIndex = 0;
  string tidAttrName = "";
  tidIndex = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                 nl->SymbolAtom(TupleIdentifier::BasicType()),
                                 tidAttrName);
  if( tidIndex <= 0 ){
    return listutils::typeError("Stream must contain an attribute of type "
                                "'tid'.");
  }
  else if( tidIndex > 0 ){
     int tidIndex2 = 0;
     string tidAttrName2 = "";
     tidIndex2 = listutils::findType(nl->Second(nl->Second(streamDescription)),
                                  nl->SymbolAtom(TupleIdentifier::BasicType()),
                                     tidAttrName2,
                                     tidIndex+1);
     if (tidIndex2 != 0) {
         return listutils::typeError("Stream must contain at most one attribute"
                                     " of type 'tid'.");
     }
  }

  // remove tid-attribute from stream-attrlist
  set<string> k;
  k.insert(tidAttrName);
  ListExpr tmp, tmpL;
  int noRemovedAttrs = 0;
  noRemovedAttrs =
      listutils::removeAttributes(nl->Second(nl->Second(streamDescription)),
                                  k,
                                  tmp,
                                  tmpL);
  if(noRemovedAttrs != 1){
    return listutils::typeError("Stream must contain at most one attribute of "
                                "type 'tid'.");
  }

  // append rel-attrlist to modified stream-attrlist
  ListExpr newAttrList =
     listutils::concat(tmp, nl->Second(nl->Second(relDescription)));

  // check whether result attrlist is valid
  if (!listutils::isAttrList(newAttrList)){
    return listutils::typeError("Result after merging tuples is not a "
                               "valid attribute list (Possible reasons: "
                               "duplicate attribute names or an attribute "
                               "type is not of kind DATA).");
  }

  // return resulttype and APPEND tid-attr index in stream-attrlist
  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->OneElemList(
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          newAttrList)));
}

/*
5.1.3 Value mapping function of operator ~gettuples~

The template parameter ~TidIndexPos~ specifies the argument number, where
the attribute index for the tid is stored within the stream argument's
tuple type. For ~gettuples~, it is ~2~, for ~gettuples2~, it is ~3~.

*/



#ifndef USE_PROGRESS

// standard version


struct GetTuplesLocalInfo
{
  Relation *relation;
  int tidIndex;
  TupleType *resultTupleType;
};

template<int TidIndexPos>
    int GetTuples( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  GetTuplesLocalInfo *localInfo;
  static MessageCenter* msg = MessageCenter::GetInstance();

  switch (message)
  {
    case OPEN :
    {
      assert( TidIndexPos == 2 || TidIndexPos == 3);
      qp->Open(args[0].addr);
      localInfo = new GetTuplesLocalInfo();
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      localInfo->tidIndex = ((CcInt*)args[TidIndexPos].addr)->GetIntval() - 1;
//    cerr << "GetTuples<" << TidIndexPos << ">(): localInfo->tidIndex = "
//         << localInfo->tidIndex << endl;
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (GetTuplesLocalInfo*)local.addr;

      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      while( qp->Received(args[0].addr) )
      {
        Tuple* sTuple = (Tuple*)wTuple.addr;
        Tuple* resultTuple = new Tuple( localInfo->resultTupleType );
        Tuple* relTuple = localInfo->relation->
            GetTuple(((TupleIdentifier *)sTuple->
            GetAttribute(localInfo->tidIndex))->GetTid());

        if(!relTuple){
          NList msg_list(NList("simple") ,
                    NList("Warning: invalid tuple id"));
          msg->Send(msg_list);
          qp->Request(args[0].addr, wTuple);
        } else {
          int j = 0;

          // Copy the attributes from the stream tuple
          for( int i = 0; i < sTuple->GetNoAttributes(); i++ )
          {
            if( i != localInfo->tidIndex )
              resultTuple->CopyAttribute( i, sTuple, j++ );
          }
          sTuple->DeleteIfAllowed();

          for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          {
            resultTuple->CopyAttribute( i, relTuple, j++ );
          }
          relTuple->DeleteIfAllowed();

          result.setAddr( resultTuple );
          return YIELD;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        localInfo = (GetTuplesLocalInfo*)local.addr;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

# else

// progress version


struct GetTuplesLocalInfo: public ProgressLocalInfo
{
  Relation *relation;
  int tidIndex;
  TupleType *resultTupleType;
};

template<int TidIndexPos>
    int GetTuples( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  GetTuplesLocalInfo *localInfo;
  localInfo = (GetTuplesLocalInfo*)local.addr;
  static MessageCenter* msg = MessageCenter::GetInstance();

  switch (message)
  {
    case OPEN :
    {
      assert( TidIndexPos == 2 || TidIndexPos == 3);
      qp->Open(args[0].addr);

      if ( !localInfo )  // first time
      {
        localInfo = new GetTuplesLocalInfo();
        localInfo->relation = (Relation*)args[1].addr;
        localInfo->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
        localInfo->tidIndex = ((CcInt*)args[TidIndexPos].addr)->GetIntval() - 1;

        local.setAddr(localInfo);
      }
      return 0;
    }

    case REQUEST :
    {
      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      while( qp->Received(args[0].addr) )
      {
        localInfo->read++;

        Tuple* sTuple = (Tuple*)wTuple.addr;
        Tuple* resultTuple = new Tuple( localInfo->resultTupleType );
        Tuple* relTuple = localInfo->relation->
            GetTuple(((TupleIdentifier *)sTuple->
            GetAttribute(localInfo->tidIndex))->GetTid(),true);

        if(!relTuple){
          NList msg_list(NList("simple") ,
                    NList("Warning: invalid tuple id"));
          msg->Send(msg_list);
          qp->Request(args[0].addr, wTuple);
        } else {
          int j = 0;

          // Copy the attributes from the stream tuple
          for( int i = 0; i < sTuple->GetNoAttributes(); i++ )
          {
            if( i != localInfo->tidIndex )
              resultTuple->CopyAttribute( i, sTuple, j++ );
          }
          sTuple->DeleteIfAllowed();

          for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          {
            resultTuple->CopyAttribute( i, relTuple, j++ );
          }
          relTuple->DeleteIfAllowed();

          result.setAddr( resultTuple );

          localInfo->returned++;
          return YIELD;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      return 0;
    }


    case CLOSEPROGRESS :
    {
      if ( localInfo )
      {
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
      }
      local.setAddr(Address(0));
      return 0;
    }


    case REQUESTPROGRESS :
    {
      ProgressInfo p1;
      ProgressInfo *pRes;
      pRes = (ProgressInfo*) result.addr;

      //Experiments described in file Cost functions
      const double uTuple = 0.061;    //milliseconds per tuple
      const double vByte = 0.0000628;  //milliseconds per byte

      if( !localInfo || !qp->RequestProgress(args[0].addr, &p1) ) {
        // ask stream argument
        return CANCEL;
      }
      localInfo->sizesChanged =
                            (!localInfo->sizesInitialized || p1.sizesChanged);
      if ( localInfo->sizesChanged ){
          localInfo->total = (int) p1.Card;
          localInfo->noAttrs =
            nl->ListLength(nl->Second(nl->Second(qp->GetType(s))));

          if(!localInfo->sizesInitialized){
             localInfo->attrSize = new double[localInfo->noAttrs];
             localInfo->attrSizeExt = new double[localInfo->noAttrs];
          }
          // ordering of attributes is:
          //  attributes from first argument (without the one at tidIndex)
          //  then relation attributes
          int no_stream_attrs = p1.noAttrs;
          int no_rel_attrs = localInfo->noAttrs - (no_stream_attrs - 1);
          // copy first part of stream attrs
          int j = 0;
          for (int i = 0;  i < localInfo->tidIndex; i++) {
            localInfo->attrSize[j]    = p1.attrSize[i];
            localInfo->Size          += p1.attrSize[i];
            localInfo->attrSizeExt[j] = p1.attrSizeExt[i];
            localInfo->SizeExt       += p1.attrSizeExt[i];
            j++;
          }
          // copy second part of stream attrs
          for (int i = localInfo->tidIndex+1;  i < no_stream_attrs; i++) {
            localInfo->attrSize[j]    = p1.attrSize[i];
            localInfo->Size          += p1.attrSize[i];
            localInfo->attrSizeExt[j] = p1.attrSizeExt[i];
            localInfo->SizeExt       += p1.attrSizeExt[i];
            j++;
          }
          // copy rel attrs
          for ( int i = 0;  i < no_rel_attrs; i++) {
            localInfo->attrSize[j] = localInfo->relation->GetTotalSize(i)
                                  / (localInfo->total + 0.001);
            localInfo->attrSizeExt[j] = localInfo->relation->GetTotalExtSize(i)
                                  / (localInfo->total + 0.001);
            localInfo->Size += localInfo->attrSize[j];
            localInfo->SizeExt += localInfo->attrSizeExt[j];
            j++;
          }
          localInfo->sizesInitialized = true;
      }
      pRes->CopySizes(localInfo);
      pRes->Card = p1.Card;
      pRes->Time = p1.Time + p1.Card * (uTuple + vByte * localInfo->SizeExt);
      if ( p1.BTime < 0.1 && pipelinedProgress ) { //non-blocking,
                                                   //use pipelining
        pRes->Progress = p1.Progress;
      } else {
        pRes->Progress =   ((p1.Progress * p1.Time)
                         + (localInfo->read / p1.Card)
                         * (uTuple + vByte * localInfo->SizeExt))
                         / pRes->Time;
      }
      pRes->CopyBlocking(p1);
      return YIELD;
    } // case REQUESTPROGRESS
  } // switch
  return 0;
}

#endif


/*
5.1.5 Specification of operator ~gettuples~

*/
const string gettuplesSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>(stream (tuple ((id tid) (x1 t1)...(xn tn)))) x"
      " (rel (tuple ((y1 t1)...(yn tn)))) ->"
      " (stream (tuple ((x1 t1)...(xn tn) (y1 t1)...(yn tn))))"
      "</text--->"
      "<text>_ _ gettuples</text--->"
      "<text>Retrieves the tuples in the relation in the second "
      "argument given by the tuple id in first argument stream. "
      "The result tuple type is a concatenation of both types "
      "without the 'tid' attribute.</text--->"
      "<text>query citiesInd windowintersectsS[r] cities gettuples; "
      "where citiesInd is e.g. created with 'letidAttrName2t citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~gettuples~

*/
Operator gettuples (
         "gettuples",            // name
         gettuplesSpec,          // specification
         GetTuples<2>,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         GetTuplesTypeMap        // type mapping
);

/*
7.2 Operator ~gettuples2~

7.2.1 Type mapping function of operator ~gettuples2~

*/
ListExpr GetTuples2TypeMap(ListExpr args)
{
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  // check for correct parameter list
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 3){
    return listutils::typeError(
      "\nExpects exactly 3 arguments.");
  }

  // Split arguments into three parts
  ListExpr streamDescription = nl->First(args),
           relDescription = nl->Second(args),
           tidArg = nl->Third(args);

  // Handle the stream part of arguments
  if(!listutils::isTupleStream(streamDescription)){
    return listutils::typeError("Expects a valid tuplestream as 1st argument.");
  }
  // Handle the rel part of arguments
  if( !listutils::isRelDescription(relDescription) ){
    return listutils::typeError("Expects a valid relation as 2nd argument.");
  }

  // Check type of third argument (attribute name)
  if( !nl->IsAtom(tidArg) || !(nl->AtomType(tidArg) == SymbolType) ){
    return listutils::typeError("Expects attribute name as 3rd argument.");
  }
  string tidAttrName = nl->SymbolValue(tidArg);
  int tidIndex = 0;
  ListExpr attrType;
  tidIndex = listutils::findAttribute(nl->Second(nl->Second(streamDescription)),
                                      tidAttrName,
                                      attrType);
  if(!listutils::isSymbol(attrType, TupleIdentifier::BasicType())){
    return listutils::typeError("Expects attribute to be of type 'tid'.");
  }

  // remove tid-attribute from stream-attrlist
  set<string> k;
  k.insert(tidAttrName);
  ListExpr tmp, tmpL;
  int noRemovedAttrs = 0;
  noRemovedAttrs =
      listutils::removeAttributes(nl->Second(nl->Second(streamDescription)),
                                  k,
                                  tmp,
                                  tmpL);
  if(noRemovedAttrs != 1){
    return listutils::typeError("Stream must contain at most one attribute of "
                                "type 'tid'.");
  }

  // append rel-attrlist to modified stream-attrlist
  ListExpr newAttrList =
     listutils::concat(tmp, nl->Second(nl->Second(relDescription)));

  // check whether result attrlist is valid
  if (!listutils::isAttrList(newAttrList)){

    cout << "ResultTupleType is " << nl->ToString(newAttrList) << endl;


    return listutils::typeError("Result after merging tuples is not a "
                               "valid attribute list (Possible reasons: "
                               "duplicate attribute names or an attribute "
                               "type is not of kind DATA).");
  }

  // return resulttype and APPEND tid-attr index in stream-attrlist
  return
      nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
  nl->OneElemList(
      nl->IntAtom(tidIndex)),
  nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
  nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
  newAttrList)));
}

/*
5.1.3 Value mapping function of operator ~gettuples2~

The same as for ~gettuples~, but template parameter is ~3~.

*/


/*
5.1.5 Specification of operator ~gettuples2~

*/
const string gettuples2Spec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>(stream (tuple ((id tid) (x1 t1)...(xn tn)))) x"
    " (rel (tuple ((y1 t1)...(yn tn)))) x id ->"
    " (stream (tuple ((x1 t1)...(xn tn) (y1 t1)...(yn tn))))"
    "</text--->"
    "<text>_ _  gettuples2[ attr ] </text--->"
    "<text>Retrieves the tuples in the relation 'rel' in the second "
    "argument given by the tuple id in argument 'attr' in the stream. "
    "(first argument). The result tuple type is a concatenation of both types "
    "without the 'id' attribute.</text--->"
    "<text>query citiesInd windowintersectsS[r] cities gettuples2[id]; "
    "where citiesInd is e.g. created with 'let citiesInd = "
    "cities creatertree [pos]'</text--->"
    ") )";

/*
5.1.6 Definition of operator ~gettuples2~

*/
Operator gettuples2 (
         "gettuples2",           // name
         gettuples2Spec,         // specification
         GetTuples<3>,           // value mapping
         Operator::SimpleSelect, // trivial selection function
         GetTuples2TypeMap       // type mapping
                   );

/*
7.2 Operator ~gettuplesdbl~

7.2.1 Type mapping function of operator ~gettuplesdbl~

*/
ListExpr GetTuplesDblTypeMap(ListExpr args)
{
  // check argument list length
  if(nl->IsEmpty(args) || nl->IsAtom(args) || !(nl->ListLength(args) == 3)){
    return listutils::typeError("Expects exactly 3 arguments.");
  }
  // Split arguments into three parts
  ListExpr streamDescription = nl->First(args),
           relDescription = nl->Second(args),
           attrnameDescription = nl->Third(args);
  // Handle the stream part of arguments
  if ( !listutils::isTupleStream(streamDescription) )
  {
    return listutils::typeError("Expects a valid tuplestream as 1st argument.");
  }
  // Handle the rel part of arguments
  if ( !listutils::isRelDescription(relDescription) )
  {
    return listutils::typeError("Expects a valid relation as 2nd argument.");
  }
  ListExpr sTupleDescription = nl->Second(streamDescription),
           sAttrList = nl->Second(sTupleDescription),
           rTupleDescription = nl->Second(relDescription),
           rAttrList = nl->Second(rTupleDescription);
  // handle attribute name
  if(!listutils::isSymbol(attrnameDescription)){
    return listutils::typeError("Expects the indexed attribute's name as 3rd "
                                "argument.");
  }
  string attrName = nl->SymbolValue(attrnameDescription);
  ListExpr attrType;
  int  attrIndex = listutils::findAttribute(rAttrList, attrName, attrType);
  if( attrIndex <= 0){
    return listutils::typeError("Expects the attribute named by the 3rd "
              "argument being part of the relation passed as 2nd argument.");
  }
  // Find the attribute with type tid
  ListExpr first, rest, newAttrList, lastNewAttrList;
  int j, tidIndex = 0;
  string type;
  bool firstcall = true,
       dblIdxFirst = false,
       dblIndex = false;

  rest = sAttrList;
  j = 1;
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);

    type = nl->SymbolValue(nl->Second(first));
    if (type == TupleIdentifier::BasicType())
    {
      if( tidIndex != 0){
        return listutils::typeError("Expects exactly one attribute of type "
                                    "'tid' in 1st argument.");
      }
      tidIndex = j;
    }
    else if( type == CcInt::BasicType() &&
             tidIndex == j-1 )
    {
      dblIdxFirst = true;
    }
    else if( type == CcInt::BasicType() &&
             dblIdxFirst &&
             tidIndex == j-2 )
    {
      dblIndex = true;
    }
    else
    {
      if (firstcall)
      {
        firstcall = false;
        newAttrList = nl->OneElemList(first);
        lastNewAttrList = newAttrList;
      }
      else
        lastNewAttrList = nl->Append(lastNewAttrList, first);
    }
    j++;
  }

  rest = rAttrList;
  while(!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);

    if (firstcall)
    {
      firstcall = false;
      newAttrList = nl->OneElemList(first);
      lastNewAttrList = newAttrList;
    }
    else
      lastNewAttrList = nl->Append(lastNewAttrList, first);
  }

  if( tidIndex == 0 ){
    return listutils::typeError("Expects a stream with one and only one "
                                "attribute of type tid as 1st argument.");
  }

  if( !listutils::isAttrList(newAttrList) ){
    return listutils::typeError("Result after merging tuples is not a "
                                "valid attribute list (Possible reasons: "
                                "duplicate attribute names or an attribute "
                                "type is not of kind DATA).");
  }

  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
      nl->TwoElemList(
        nl->IntAtom(attrIndex),
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
          nl->SymbolAtom(Tuple::BasicType()),
          newAttrList)));
}

/*
5.1.3 Value mapping functions of operator ~gettuplesdbl~

*/


struct GetTuplesDblLocalInfo
{
  Relation *relation;
  int attrIndex;
  int tidIndex;
  Tuple *lastTuple;
  vector< pair<int, int> > intervals;
  TupleType *resultTupleType;
};

int GetTuplesDbl( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  GetTuplesDblLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      qp->Open(args[0].addr);
      localInfo = new GetTuplesDblLocalInfo();
      localInfo->relation = (Relation*)args[1].addr;
      localInfo->resultTupleType =
        new TupleType(nl->Second(GetTupleResultType(s)));
      localInfo->attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;
      localInfo->tidIndex = ((CcInt*)args[4].addr)->GetIntval() - 1;
      localInfo->lastTuple = 0;
      local.setAddr(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (GetTuplesDblLocalInfo*)local.addr;

      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      while( qp->Received(args[0].addr) )
      {
        Tuple *sTuple = (Tuple*)wTuple.addr;

        if( localInfo->lastTuple == 0 )
        {
          localInfo->lastTuple = sTuple;
          localInfo->intervals.push_back( make_pair(
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+1 ))->
              GetIntval(),
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+2 ))->
              GetIntval() ) );
        }
        else if( sTuple->GetAttribute( localInfo->tidIndex )->
                   Compare( localInfo->lastTuple->
                     GetAttribute( localInfo->tidIndex ) ) == 0 )
        {
          localInfo->intervals.push_back( make_pair(
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+1 ))->
              GetIntval(),
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+2 ))->
              GetIntval() ) );
          sTuple->DeleteIfAllowed();
        }
        else
        {
          Tuple *resultTuple = new Tuple( localInfo->resultTupleType ),
                *relTuple = localInfo->relation->
                  GetTuple( ((TupleIdentifier *)localInfo->lastTuple->
                              GetAttribute(localInfo->tidIndex))->GetTid(),
                            localInfo->attrIndex,
                            localInfo->intervals, false );
          localInfo->intervals.clear();
          localInfo->intervals.push_back( make_pair(
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+1 ))->
              GetIntval(),
            ((CcInt*)sTuple->GetAttribute( localInfo->tidIndex+2 ))->
              GetIntval() ) );

          // Copy the attributes from the stream tuple
          int j = 0;
          for( int i = 0; i < localInfo->lastTuple->GetNoAttributes(); i++ )
          {
            if( i < localInfo->tidIndex &&        // Strange thing: shoudn't it
                i > localInfo->tidIndex + 2 )     // be ( i<... || i>... ) ???
              resultTuple->CopyAttribute( j++, localInfo->lastTuple, i );
          }
          localInfo->lastTuple->DeleteIfAllowed();
          localInfo->lastTuple = sTuple;

          for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
            resultTuple->CopyAttribute( j++, relTuple, i );
          relTuple->DeleteIfAllowed();

          result.setAddr( resultTuple );
          return YIELD;
        }
        qp->Request(args[0].addr, wTuple);
      }

      if( localInfo->lastTuple != 0 )
      {
        Tuple *resultTuple = new Tuple( localInfo->resultTupleType ),
              *relTuple = localInfo->relation->
                  GetTuple(((TupleIdentifier *)localInfo->lastTuple->
                  GetAttribute(localInfo->tidIndex))->GetTid(),
                localInfo->attrIndex,
                localInfo->intervals, false );

        // Copy the attributes from the stream tuple
        int j = 0;
        for( int i = 0; i < localInfo->lastTuple->GetNoAttributes(); i++ )
        {
          if( i < localInfo->tidIndex &&
              i > localInfo->tidIndex + 2 )
            resultTuple->CopyAttribute( j++, localInfo->lastTuple, i );
        }
        localInfo->lastTuple->DeleteIfAllowed();
        localInfo->lastTuple = 0;

        for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          resultTuple->CopyAttribute( j++, relTuple, i );
        relTuple->DeleteIfAllowed();

        result.setAddr( resultTuple );
        return YIELD;
      }
      else
        return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      if(local.addr)
      {
        localInfo = (GetTuplesDblLocalInfo*)local.addr;
        localInfo->resultTupleType->DeleteIfAllowed();
        delete localInfo;
        local.setAddr(Address(0));
      }
      return 0;
    }
  }
  return 0;
}

/*
5.1.5 Specification of operator ~gettuplesdbl~

*/
const string GetTuplesDblSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>(stream (tuple ((id tid) (x1 t1)...(xn tn)))) x"
      " (rel (tuple ((y1 t1)...(yn tn)))) x yk ->"
      " (stream (tuple ((x1 t1)...(xn tn) (y1 t1)...(yn tn))))"
      "</text--->"
      "<text>_ _ gettuplesdbl[ _ ]</text--->"
      "<text>Retrieving tuples using double indexing. The tuples are "
      "retrieved from the relation in the second argument. "
      "The tuples to retrieve are given by a attribute of type 'tid' "
      "in first argument stream. "
      "The result tuple type is a concatenation of both types "
      "without the tid attribute. When the tid is followed by two int "
      "attributes in the first stream argument, these integers will be "
      "used to restrict the Flob belonging to the attribute indicated "
      "by its name in the 3rd argument to a specific interval.</text--->"
      "<text>query citiesInd windowintersectsS[r] cities gettuplesdbl; "
      "where citiesInd is e.g. created with 'let citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";

/*
5.1.6 Definition of operator ~gettuplesdbl~

*/
Operator gettuplesdbl (
         "gettuplesdbl",            // name
         GetTuplesDblSpec,          // specification
         GetTuplesDbl,              // value mapping
         Operator::SimpleSelect,    // selection function
         GetTuplesDblTypeMap        // type mapping
);


/*
5.2 Operator ~creatertree\_bulkload~

This operator will create a new r-tree from the scratch, applying a
simple bulkload mechanism. The input relation/stream is read and partition
them into $p =\lceil \frac{N}{c} \rceil$ leaf pages. Then start aggregating
the leafnodes into $p :=\lceil \frac{p}{c} \rceil$ internal nodes until only
a single node, the root, remains.

Problems: There may be many nodes to be handled, so we need to store the current working set of nodes and within a persistent buffer.

Sorting is not implemented within this algorithm. The input should already be sorted
with respect to the center or lower left corner of the input bounding boxes, e.g. using
a Z-order. You can apply other kinds of orderings to take some influence on the
overlapping within the r-tree.

*/

/*
5.2. TypeMapping for Operator ~creatertree\_bulkload<D>~

Signature is

----
         creatertree_bulkload<D>: stream
----

*/
ListExpr CreateRTreeBulkLoadTypeMap(ListExpr args)
{
  // check number of parameters
  int len = nl->ListLength(args);
  if( (len != 2) && (len!=3) ){
    return listutils::typeError("Expecting exactly 2 or 3 arguments.");
  }



  // split to parameters
  ListExpr tupleStream = nl->First(args),
           attrNameLE = nl->Second(args);
  // check stream
  if(!listutils::isTupleStream(tupleStream)){
    return listutils::typeError("Expecting a tuplestream as 1st argument.");
  }

  // check key attribute name
  if(!listutils::isSymbol(attrNameLE)){
    return listutils::typeError("Expecting an attribute name as 2nd argument.");
  }
  string attrName = nl->SymbolValue(attrNameLE);
  // check if key attribute is from stream
  ListExpr attrList = nl->Second(nl->Second(tupleStream));
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if(attrIndex <= 0){
    return listutils::typeError("Expecting the attribute (2nd argument) being "
                                "part of the tuplestream (1st argument).");
  }
  // check for type of key attribute
  if( !(listutils::isSpatialType(attrType)||listutils::isRectangle(attrType))){
      return listutils::typeError("Expecting the key attribute (2nd argument) "
            "being of kind SPATIAL2D, SPATIAL3D, Spatial4D or SPATIAL8D, of of "
            "type rect, rect3, rect4, or rect8.");
  }
  string rtreetype;
  if(    listutils::isKind(attrType, Kind::SPATIAL2D())
      || listutils::isSymbol(attrType, Rectangle<2>::BasicType())){
    rtreetype = RTree2TID::BasicType();
  }else if(    listutils::isKind(attrType, Kind::SPATIAL3D())
      || listutils::isSymbol(attrType, Rectangle<3>::BasicType())){
    rtreetype = RTree3TID::BasicType();
  }else if(    listutils::isKind(attrType, Kind::SPATIAL4D())
      || listutils::isSymbol(attrType, Rectangle<4>::BasicType())){
    rtreetype = RTree4TID::BasicType();
  }else if(    listutils::isKind(attrType, Kind::SPATIAL8D())
      || listutils::isSymbol(attrType, Rectangle<8>::BasicType())){
    rtreetype = RTree8TID::BasicType();
  }else{
    return listutils::typeError("Unsupported key type.");
  }

/*
Now we have two possibilities:

- multi-entry indexing, or
- double indexing

For multi-entry indexing, one and only one of the attributes
must be a tuple identifier. In the latter, together with
a tuple identifier, the last two attributes must be of
integer type (~int~).

In the first case, a standard R-Tree is created possibly
containing several entries to the same tuple identifier, and
in the latter, a double index R-Tree is created using as low
and high parameters these two last integer numbers.

*/

    ListExpr first, rest, newAttrList, lastNewAttrList;
    int tidIndex = 0;
    bool firstcall = true,
    doubleIndex = false;

    int nAttrs = nl->ListLength( attrList );
    rest = attrList;
    int j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      ListExpr type = nl->Second(first);   
      if (TupleIdentifier::checkType(type))
      {
        if( tidIndex != 0 && (len != 3)){
          return listutils::typeError("Expecting exactly one attribute of type "
                                      "'tid' in the 1st argument.");
        }
        tidIndex = j;
      }
      else if( j == nAttrs - 1 && CcInt::checkType(type) &&
               CcInt::checkType( nl->Second(nl->First(rest))) )
      { // the last two attributes are integers
        doubleIndex = true;
      }
      else
      {
        if (firstcall)
        {
          firstcall = false;
          newAttrList = nl->OneElemList(first);
          lastNewAttrList = newAttrList;
        }
        else
        {
          lastNewAttrList = nl->Append(lastNewAttrList, first);
        }
      }
      j++;
    }
   
    if(len==3){ // ignore the tid index computed before
       ListExpr tidName = nl->Third(args);
       if(!listutils::isSymbol(tidName)){
          return listutils::typeError("third argument "
                                      "must be an attribute name ");
       }
       string tidn = nl->SymbolValue(tidName);
       ListExpr tidType;
       tidIndex = listutils::findAttribute(attrList, tidn,tidType);
       if(tidIndex==0){
          return listutils::typeError("Attribute " + tidn + 
                                      " not present in tuple. ");
       }
       if(!TupleIdentifier::checkType(tidType)){
          return listutils::typeError("Attribute " + tidn + 
                                      " is not a tuple identifier ");
       }
    } else if( tidIndex == 0 ){
      return listutils::typeError("Expecting exactly one attribute of type "
                                  "'tid' in the 1st argument.");
    }

    ListExpr res = 
        nl->ThreeElemList(
          nl->SymbolAtom(Symbol::APPEND()),
          nl->TwoElemList(
             nl->IntAtom(attrIndex),
             nl->IntAtom(tidIndex)),
          nl->FourElemList(
            nl->SymbolAtom(rtreetype),
            nl->TwoElemList(
              nl->SymbolAtom(Tuple::BasicType()),
              newAttrList),
            attrType,
            nl->BoolAtom(doubleIndex)));

    return res; 

}

/*
5.2. Value Mapping for Operator ~creatertree\_bulkload<D>~

*/
template<unsigned dim>
int CreateRTreeBulkLoadStreamSpatial( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
// Step 0 - Initialization
  int argOffset = qp->GetNoSons(s)==4?0:1;

  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
      (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2+argOffset].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3+argOffset].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  bool BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    if ((count++ % 10000) == 0)
    {
      // build a two elem list (simple count)
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
      msg->Send(msgList);
    }
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
              GetAttribute(attrIndex))->IsDefined() &&
              ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
              IsDefined() )
    {
        BBox<dim> box = ((StandardSpatialAttribute<dim>*)tuple->
              GetAttribute(attrIndex))->BoundingBox();
/////only for Greece knn algorithm////

//        if(dim == 3){
//            double min[3];
//            double max[3];
//            for(int i = 0;i < 3;i++){
//              min[i] = box.MinD(i);
//              max[i] = box.MaxD(i);
//            }
//            min[2] = min[2]*864000;
//            max[2] = max[2]*864000;
//            box.Set(true,min,max);
//        }

//////////
      if(box.IsDefined()){
        R_TreeLeafEntry<dim, TupleId>
              e( box,
                 ((TupleIdentifier *)tuple->
                     GetAttribute(tidIndex))->GetTid() );
        rtree->InsertBulkLoad(e);
       }
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);
  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert( FinalizedBulkLoad );

  // build a two elem list (simple count)
  NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
  msg->Send(msgList);

  return 0;
}

// VM for double layer indexing
template<unsigned dim>
    int CreateRTreeBulkLoadStreamL2Spatial(Word* args, Word& result,
                                           int message,
                                           Word& local, Supplier s)
{

  int argsOffset = qp->GetNoSons(s)==4?0:1;
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
      (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2+argsOffset].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3+argsOffset].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  bool BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    if ((count++ % 10000) == 0)
    {
      // build a two elem list (simple count)
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
      msg->Send(msgList);
    }
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
          ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() &&
          ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-2))->
          IsDefined() &&
          ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-1))->
          IsDefined() )
    {
      BBox<dim> box =
          ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->BoundingBox();
      R_TreeLeafEntry<dim, TwoLayerLeafInfo> e(
          box,
          TwoLayerLeafInfo(
              ((TupleIdentifier *)tuple->
              GetAttribute(tidIndex))->GetTid(),
          ((CcInt*)tuple->
              GetAttribute(tuple->GetNoAttributes()-2))->GetIntval(),
          ((CcInt*)tuple->
              GetAttribute(tuple->GetNoAttributes()-1))->GetIntval()));
          rtree->InsertBulkLoad(e);
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);
  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert( FinalizedBulkLoad );

  // build a two elem list (simple count)
  NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
  msg->Send(msgList);

  return 0;
}

template<unsigned dim>
    int CreateRTreeBulkLoadStreamRect( Word* args, Word& result,
                                       int message,
                                       Word& local, Supplier s )
{


  int argOffset = qp->GetNoSons(s)==4?0:1;

  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
      (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2+argOffset].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3+argOffset].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  bool BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    if ((count++ % 10000) == 0)
    {
      // build a two elem list (simple count)
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
      msg->Send(msgList);
    }
    Tuple* tuple = (Tuple*)wTuple.addr;

    BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
    if( box->IsDefined() &&
        ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->IsDefined()
      )
    {
      R_TreeLeafEntry<dim, TupleId>
          e( *box,
             ((TupleIdentifier *)tuple->
                 GetAttribute(tidIndex))->GetTid() );
          rtree->InsertBulkLoad(e);
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert( FinalizedBulkLoad );
        // build a two elem list (simple count)
  NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
  msg->Send(msgList);

  return 0;
}

// VM for double layer indexing
template<unsigned dim>
int CreateRTreeBulkLoadStreamL2Rect(Word* args, Word& result, int message,
                                    Word& local, Supplier s)
{
  int argOffset = qp->GetNoSons(s)==4?0:1;
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
      (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  int attrIndex = ((CcInt*)args[2+argOffset].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3+argOffset].addr)->GetIntval() - 1;

        // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  bool BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);
  while (qp->Received(args[0].addr))
  {
    if ((count++ % 10000) == 0)
    {
      // build a two elem list (simple count)
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
      msg->Send(msgList);
    }
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
          GetAttribute(attrIndex))->IsDefined() &&
          ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
          IsDefined() &&
          ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-2))->
          IsDefined() &&
          ((CcInt*)tuple->GetAttribute(tuple->GetNoAttributes()-1))->
          IsDefined() )
    {
      BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
      if( box->IsDefined() )
      {
        R_TreeLeafEntry<dim, TwoLayerLeafInfo>
            e( *box,
                TwoLayerLeafInfo(
                    ((TupleIdentifier *)tuple->
                    GetAttribute(tidIndex))->GetTid(),
                ((CcInt*)tuple->GetAttribute(
                    tuple->GetNoAttributes()-2))->GetIntval(),
                ((CcInt*)tuple->GetAttribute(
                    tuple->GetNoAttributes()-1))->GetIntval() ) );
            rtree->InsertBulkLoad(e);
      }
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);

  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert( FinalizedBulkLoad );
        // build a two elem list (simple count)
  NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
  msg->Send(msgList);

  return 0;
}
/*
5.2. Selection Function for Operator ~creatertree\_bulkload<D>~

*/
int CreateRTreeBulkLoadSelect (ListExpr args)
{
  ListExpr relDescription = nl->First(args),
  attrNameLE = nl->Second(args),
  tupleDescription = nl->Second(relDescription),
  attrList = nl->Second(tupleDescription);
  string attrName = nl->SymbolValue(attrNameLE);
  int attrIndex;
  ListExpr attrType;
  attrIndex = FindAttribute(attrList, attrName, attrType);
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  int result;

  if ( algMgr->CheckKind(Kind::SPATIAL2D(), attrType, errorInfo) )
    result = 0;
  else if ( algMgr->CheckKind(Kind::SPATIAL3D(), attrType, errorInfo) )
    result = 1;
  else if ( algMgr->CheckKind(Kind::SPATIAL4D(), attrType, errorInfo) )
    result = 2;
  else if ( algMgr->CheckKind(Kind::SPATIAL8D(), attrType, errorInfo) )
    result = 3;
  else if( nl->SymbolValue(attrType) == Rectangle<2>::BasicType() )
    result = 4;
  else if( nl->SymbolValue(attrType) == Rectangle<3>::BasicType() )
    result = 5;
  else if( nl->SymbolValue(attrType) == Rectangle<4>::BasicType() )
    result = 6;
  else if( nl->SymbolValue(attrType) == Rectangle<8>::BasicType() )
    result = 7;
  else
    return -1; /* should not happen */

  if( nl->SymbolValue(nl->First(relDescription)) == Symbol::STREAM())
  {
    ListExpr first,
    rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
    if( nl->IsEqual( nl->Second( first ), CcInt::BasicType() ) )
          // Double indexing
      return result + 8;
    else
          // Multi-entry indexing
      return result + 0;
  }
  return -1;
}

ValueMapping CreateRTreeBulkLoad [] =
{ CreateRTreeBulkLoadStreamSpatial<2>,   //0
  CreateRTreeBulkLoadStreamSpatial<3>,
  CreateRTreeBulkLoadStreamSpatial<4>,
  CreateRTreeBulkLoadStreamSpatial<8>,
  CreateRTreeBulkLoadStreamRect<2>,      //4
  CreateRTreeBulkLoadStreamRect<3>,
  CreateRTreeBulkLoadStreamRect<4>,
  CreateRTreeBulkLoadStreamRect<8>,
  CreateRTreeBulkLoadStreamL2Spatial<2>, // 8
  CreateRTreeBulkLoadStreamL2Spatial<3>,
  CreateRTreeBulkLoadStreamL2Spatial<4>,
  CreateRTreeBulkLoadStreamL2Spatial<8>,
  CreateRTreeBulkLoadStreamL2Rect<2>,    // 12
  CreateRTreeBulkLoadStreamL2Rect<3>,
  CreateRTreeBulkLoadStreamL2Rect<4>,
  CreateRTreeBulkLoadStreamL2Rect<8>     // 15
};


/*
5.2. Specification for Operator ~creatertree\_bulkload~

*/
const string CreateRTreeBulkLoadSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)\n"
  "((stream (tuple (x1 t1)...(xn tn) "
  "(id tid)(low int)(high int))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti true)</text--->"
  "<text>bulkloadrtree [ _ ]</text--->"
  "<text>Creates an rtree<D> applying bulk loading. This means, "
  "the operator expects the input stream of tuples to be ordered "
  "in some meaningful way in order to reduce overlapping of "
  "bounding boxes (e.g. a Z-ordering on the bounding boxes). "
  "The R-Tree is created bottom up by gouping as many entries as "
  "possible into the leaf nodes and then creating the higher levels. "
  "The key type ti must be of kind SPATIAL2D, SPATIAL3D, SPATIAL4D "
  "or Spatial8D, or of type rect, rect2, rect3, rect4 or rect8.</text--->"
  "<text>let myrtree = Kreis feed projectextend[ ; TID: tupleid(.), "
  "MBR: bbox(.Gebiet)] sortby[MBR asc] bulkloadrtree[MBR]</text--->"
  "<text></text--->"
  ") )";
/*
5.2. Definition of Operator ~creatertree\_bulkload<D>~

*/
Operator bulkloadrtree(
         "bulkloadrtree",       // name
         CreateRTreeBulkLoadSpec,      // specification
         16,
         CreateRTreeBulkLoad,          // value mapping
         CreateRTreeBulkLoadSelect,    // selection function
         CreateRTreeBulkLoadTypeMap    // type mapping
);


/*
5.9 Inquiry Operators ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

This operators can be used to inquire some basic R-tree statistics.

*/

/*
5.9.1 Type Mapping Function for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/
ListExpr RTree2IntTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError("Expected exactly 1 argument.");
  }
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  return nl->SymbolAtom(CcInt::BasicType());
}

/*
5.9.1 Type Mapping Function for ~getFileInfo~

*/
ListExpr RTree2TextTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError("Expected exactly 1 argument.");
  };
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  return nl->SymbolAtom(FText::BasicType());
}


/*
5.9.1 Type Mapping Function for ~bbox~

*/
ListExpr RTree2RectTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError("Expected exactly 1 argument.");
  };
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  ListExpr rtreeSymbol = nl->First(rtreeDescription);
  if(nl->SymbolValue(rtreeSymbol) == RTree2TID::BasicType())
    return nl->SymbolAtom(Rectangle<2>::BasicType());
  else if(nl->SymbolValue(rtreeSymbol) == RTree3TID::BasicType())
    return nl->SymbolAtom(Rectangle<3>::BasicType());
  else if(nl->SymbolValue(rtreeSymbol) == RTree4TID::BasicType())
    return nl->SymbolAtom(Rectangle<4>::BasicType());
  else if(nl->SymbolValue(rtreeSymbol) == RTree8TID::BasicType())
    return nl->SymbolAtom(Rectangle<8>::BasicType());
  else
    return listutils::typeError("Unsupported rtee-type.");
}

/*
5.9.2 Value Mapping Functions for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/
template<unsigned dim, class LeafInfo>
int RTreeTreeHeightVM( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo>*) args[0].addr;
  result = qp->ResultStorage( s );
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, rtree->Height() );
  return 0;
}

template<unsigned dim, class LeafInfo>
int RTreeNoOfNodesVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo>*) args[0].addr;
  result = qp->ResultStorage( s );
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, rtree->NodeCount() );
  return 0;
}

template<unsigned dim, class LeafInfo>
int RTreeNoOfEntriesVM( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo>*) args[0].addr;
  result = qp->ResultStorage( s );
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, rtree->EntryCount() );
  return 0;
}

template<unsigned dim, class LeafInfo>
int RTreeBboxVM( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo>*) args[0].addr;
  result = qp->ResultStorage( s );
  Rectangle<dim> *res = (Rectangle<dim> *) result.addr;
  *res = rtree->BoundingBox();
  return 0;
}

template<unsigned dim, class LeafInfo>
int RTreeGetRootNodeVM( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo>*) args[0].addr;
  result = qp->ResultStorage( s );
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, rtree->RootRecordId() );
  return 0;
}


/*
5.9.3 Selection Function for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/
int RTreeInquirySelect (ListExpr args)
{
  int result = -100;
  ListExpr rtreeDescription = nl->First(args);
  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  if(nl->SymbolValue(rtreeSymbol) == RTree2TID::BasicType())
    result = 0;
  else if(nl->SymbolValue(rtreeSymbol) == RTree3TID::BasicType())
    result = 1;
  else if(nl->SymbolValue(rtreeSymbol) == RTree4TID::BasicType())
    result = 2;
  else if(nl->SymbolValue(rtreeSymbol) == RTree8TID::BasicType())
    result = 3;

  if( nl->BoolValue(rtreeTwoLayer) == true )
    result += 4;

  return result;
}

/*
5.9.3 Value Mapping Arrays for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/

ValueMapping RTreeTreeHeight [] =
{ RTreeTreeHeightVM<2, TupleId>,  // 0
  RTreeTreeHeightVM<3, TupleId>,  // 1
  RTreeTreeHeightVM<4, TupleId>,  // 2
  RTreeTreeHeightVM<8, TupleId>,  // 3
  RTreeTreeHeightVM<2, TwoLayerLeafInfo>,  // 4
  RTreeTreeHeightVM<3, TwoLayerLeafInfo>,  // 5
  RTreeTreeHeightVM<4, TwoLayerLeafInfo>,  // 6
  RTreeTreeHeightVM<8, TwoLayerLeafInfo>   // 7
  };

ValueMapping RTreeNoOfEntries [] =
{ RTreeNoOfEntriesVM<2, TupleId>,  // 0
  RTreeNoOfEntriesVM<3, TupleId>,  // 1
  RTreeNoOfEntriesVM<4, TupleId>,  // 2
  RTreeNoOfEntriesVM<8, TupleId>,  // 3
  RTreeNoOfEntriesVM<2, TwoLayerLeafInfo>,  // 4
  RTreeNoOfEntriesVM<3, TwoLayerLeafInfo>,  // 5
  RTreeNoOfEntriesVM<4, TwoLayerLeafInfo>,  // 6
  RTreeNoOfEntriesVM<8, TwoLayerLeafInfo>   // 7
};

ValueMapping RTreeNoOfNodes [] =
{ RTreeNoOfNodesVM<2, TupleId>,  // 0
  RTreeNoOfNodesVM<3, TupleId>,  // 1
  RTreeNoOfNodesVM<4, TupleId>,  // 2
  RTreeNoOfNodesVM<8, TupleId>,  // 3
  RTreeNoOfNodesVM<2, TwoLayerLeafInfo>,  // 4
  RTreeNoOfNodesVM<3, TwoLayerLeafInfo>,  // 5
  RTreeNoOfNodesVM<4, TwoLayerLeafInfo>,  // 6
  RTreeNoOfNodesVM<8, TwoLayerLeafInfo>   // 7
};

ValueMapping RTreeBbox [] =
{ RTreeBboxVM<2, TupleId>,  // 0
  RTreeBboxVM<3, TupleId>,  // 1
  RTreeBboxVM<4, TupleId>,  // 2
  RTreeBboxVM<8, TupleId>,  // 3
  RTreeBboxVM<2, TwoLayerLeafInfo>,  // 4
  RTreeBboxVM<3, TwoLayerLeafInfo>,  // 5
  RTreeBboxVM<4, TwoLayerLeafInfo>,  // 6
  RTreeBboxVM<8, TwoLayerLeafInfo>   // 7
};

ValueMapping RTreeGetRootNode [] =
{ RTreeGetRootNodeVM<2, TupleId>,  // 0
  RTreeGetRootNodeVM<3, TupleId>,  // 1
  RTreeGetRootNodeVM<4, TupleId>,  // 2
  RTreeGetRootNodeVM<8, TupleId>,  // 3
  RTreeGetRootNodeVM<2, TwoLayerLeafInfo>,  // 4
  RTreeGetRootNodeVM<3, TwoLayerLeafInfo>,  // 5
  RTreeGetRootNodeVM<4, TwoLayerLeafInfo>,  // 6
  RTreeGetRootNodeVM<8, TwoLayerLeafInfo>   // 7
};


/*
5.9.5 Specification Strings for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/
const string RTreeTreeHeightSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> -> int</text--->"
    "<text>treeheight( _ )</text--->"
    "<text>Returns the height of the R-tree.</text--->"
    "<text>query treeheight(strassen_geoData)</text--->"
    "<text>Always defined. Counting starts with level "
    "'0' for the root node level.</text--->"
    ") )";

const string RTreeNoOfEntriesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> -> int</text--->"
    "<text>no_entries( _ )</text--->"
    "<text>Returns the R-tree's number of entries (stored objects).</text--->"
    "<text>query no_entries(strassen_geoData)</text--->"
    "<text>Always defined.</text--->"
    ") )";

const string RTreeNoOfNodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> -> int</text--->"
    "<text>no_nodes( _ )</text--->"
    "<text>Returns the R-tree's number of nodes.</text--->"
    "<text>query no_nodes(strassen_geoData)</text--->"
    "<text>Always defined.</text--->"
    ") )";

const string RTreeBboxSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> -> rect<D></text--->"
    "<text>bbox( _ )</text--->"
    "<text>Returns the minimum bounding rectangle for "
    "the complete R-tree.</text--->"
    "<text>query bbox(strassen_geoData)</text--->"
    "<text>Rectangle will be undefined for an empty R-tree.</text--->"
    ") )";

const string RTreeGetRootNodeSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>rtree<D> -> int</text--->"
    "<text>getRootNode( _ )</text--->"
    "<text>Returns R-tree's record number of root node.</text--->"
    "<text>query getRootNode(strassen_geoData)</text--->"
    "<text>Always defined.</text--->"
    ") )";

/*
5.9.6 Definitions of ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~, ~getRootNode~

*/

Operator rtreetreeheight(
                     "treeheight",          // name
                      RTreeTreeHeightSpec,  // specification
                      8,
                      RTreeTreeHeight,      // value mapping
                      RTreeInquirySelect,   // selection function
                      RTree2IntTypeMap      // type mapping
                     );

Operator rtreenoofnodes(
                      "no_nodes",              // name
                      RTreeNoOfNodesSpec,      // specification
                      8,
                      RTreeNoOfNodes,          // value mapping
                      RTreeInquirySelect,      // selection function
                      RTree2IntTypeMap         // type mapping
                    );

Operator rtreenoofentries(
                      "no_entries",             // name
                      RTreeNoOfEntriesSpec,     // specification
                      8,
                      RTreeNoOfEntries,         // value mapping
                      RTreeInquirySelect,       // selection function
                      RTree2IntTypeMap          // type mapping
                    );

Operator rtreebbox(
                      "bbox",             // name
                      RTreeBboxSpec,      // specification
                      8,
                      RTreeBbox,          // value mapping
                      RTreeInquirySelect, // selection function
                      RTree2RectTypeMap    // type mapping
                    );

Operator rtreegetrootnode(
                       "getRootNode",           // name
                       RTreeGetRootNodeSpec,    // specification
                       8,
                       RTreeGetRootNode,        // value mapping
                       RTreeInquirySelect,      // selection function
                       RTree2IntTypeMap         // type mapping
                     );


/*
5.10 Operator ~nodes~

This operator allows introspection of an R-tree. It creates a stream
of tuples, each of which describe one node or entry of the R-Tree.

Signature is

----
    nodes: (rtree) --> stream(tuple((level int) (nodeId int) (MBR rect<D>)
                                    (fatherId int) (isLeaf bool)
                                    (minEntries int) (maxEntries int)
                                    (countEntries int)))

----

*/

/*
5.10.1 TypeMapping for Operator ~nodes~

*/

ListExpr RTreeNodesTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError("Expected exactly 1 argument.");
  };
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);
  ListExpr MBR_ATOM;
  if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<2>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<2>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<3>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<3>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<4>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<4>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<8>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<8>::BasicType()); }
  else return listutils::typeError("Unsupported rtree-type.");

  ListExpr reslist =
   nl->TwoElemList(
    nl->SymbolAtom(Symbol::STREAM()),
    nl->TwoElemList(
     nl->SymbolAtom(Tuple::BasicType()),
     nl->Cons(
      nl->TwoElemList(nl->SymbolAtom("Level"),
                      nl->SymbolAtom(CcInt::BasicType())),
      nl->Cons(
       nl->TwoElemList(nl->SymbolAtom("NodeId"),
                       nl->SymbolAtom(CcInt::BasicType())),
       nl->SixElemList(
        nl->TwoElemList(nl->SymbolAtom("MBR"), MBR_ATOM),
        nl->TwoElemList(nl->SymbolAtom("FatherID"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("IsLeaf"),
                        nl->SymbolAtom(CcBool::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("MinEntries"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("MaxEntries"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("CountEntries"),
                        nl->SymbolAtom(CcInt::BasicType()))
       )
      )
     )
    )
   );
  return reslist;
}


/*
5.10.2 Value Mapping for Operator ~nodes~

*/

template<unsigned dim>
int RTreeNodesVM( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  RTreeNodesLocalInfo<dim> *lci;

  switch( message )
  {
    case OPEN:
    {
      lci = new RTreeNodesLocalInfo<dim>;
      local.setAddr(lci);
      lci->firstCall = true;
      lci->finished = false;
      lci->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      lci->rtree = (R_Tree<dim, TupleId>*) args[0].addr;

      return 0;
    }

    case REQUEST :
    {
      if(local.addr == NULL)
        return CANCEL;
      lci = (RTreeNodesLocalInfo<dim> *)local.addr;
      if(lci->finished)
        return CANCEL;

      IntrospectResult<dim> node;
      if(lci->firstCall)
      {
        lci->firstCall = false;
        lci->finished = !lci->rtree->IntrospectFirst(node);
      }
      else
      {
        lci->finished = !lci->rtree->IntrospectNext(node);
      }
      if( lci->finished )
      {
        return CANCEL;
      }
      else
      {
        Tuple *tuple = new Tuple( lci->resultTupleType );
        tuple->PutAttribute(0, new CcInt(true, node.level));
        tuple->PutAttribute(1, new CcInt(true, node.nodeId));
        tuple->PutAttribute(2, new Rectangle<dim>(node.MBR));
        tuple->PutAttribute(3, new CcInt(true, node.fatherId));
        tuple->PutAttribute(4, new CcBool(true, node.isLeaf));
        tuple->PutAttribute(5, new CcInt(true, node.minEntries));
        tuple->PutAttribute(6, new CcInt(true, node.maxEntries));
        tuple->PutAttribute(7, new CcInt(true, node.countEntries));
        result.setAddr(tuple);
        return YIELD;
      }
    }

    case CLOSE :
    {
      if(local.addr)
      {
        lci = (RTreeNodesLocalInfo<dim> *)local.addr;
        lci->resultTupleType->DeleteIfAllowed();
        delete lci;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "RTreeNodesVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
5.10.3 Specification for Operator ~nodes~

*/

const string RTreeNodesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> (tuple ((x1 t1)...(xn tn))) ti false) "
    "-> stream(tuple((level int) (nodeId int) (MBR rect<D>) \n"
    "                (fatherId int) (isLeaf bool) \n"
    "                (minEntries int) (maxEntries int) \n"
    "                (countEntries int)))</text--->"
    "<text>nodes( _ )</text--->"
    "<text>Iterates the complete R-tree creating a stream of tuples"
    "describing all nodes and leaf entries.</text--->"
    "<text></text--->"
    "<text></text--->"
    ") )";

/*
5.10.4 Selection Function for Operator ~nodes~

*/

int RTreeNodesSelect (ListExpr args)
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  string rtreeDescriptionStr;

  /* handle rtree part of argument */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  if(         nl->IsEqual(rtreeKeyType, Rectangle<2>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo))
    return 0;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<3>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo))
    return 1;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<4>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo))
    return 2;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<8>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo))
    return 3;

  return -1;
}

ValueMapping RTreeNodes [] =
{ RTreeNodesVM<2>,  // 0
  RTreeNodesVM<3>,  // 1
  RTreeNodesVM<4>,  // 2
  RTreeNodesVM<8>   // 3
};



/*
5.10.2 Definition of Operator ~nodes~

*/
Operator rtreenodes(
         "nodes",             // name
         RTreeNodesSpec,      // specification
         4,
         RTreeNodes,          // value mapping
         RTreeNodesSelect,    // selection function
         RTreeNodesTypeMap    // type mapping
        );

/*
5.10.3 Definition of Operator ~entries~

*/
const string RTreeEntriesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> (tuple ((x1 t1)...(xn tn))) ti false) "
    "-> stream(tuple((nodeid int)(bbox key)(tid tupleid))</text--->"
    "<text>entries( _ )</text--->"
    "<text>Returns a stream of tuples which are all the nodes contain bbox in"
    "a leaf entry.</text--->"
    "<text></text--->"
    "<text></text--->"
    ") )";

template<unsigned dim,class LeafInfo>
int RTreeEntriesVM( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  RTreeNodesLocalInfo<dim> *lci;
   IntrospectResult<dim> node;

  switch( message )
  {
    case OPEN:
    {
      lci = new RTreeNodesLocalInfo<dim>;
      local.setAddr(lci);
      lci->firstCall = true;
      lci->finished = false;
      lci->resultTupleType =
          new TupleType(nl->Second(GetTupleResultType(s)));
      lci->rtree = (R_Tree<dim, TupleId>*) args[0].addr;
    if(lci->firstCall)
      {
        lci->firstCall = false;
        lci->finished = !lci->rtree->IntrospectFirst(node);
      }
      return 0;
    }
    case REQUEST :
    {
    static int level = 0;
    unsigned long nodeid;
    static unsigned long tupleid;
    static BBox<dim> box;
    if(local.addr == NULL)
        return CANCEL;
      lci = (RTreeNodesLocalInfo<dim> *)local.addr;
      if(lci->finished)
        return CANCEL;
    if(level == 0){
          lci->finished = !lci->rtree->IntrospectNextE(nodeid,box,tupleid);
      level = lci->rtree->Height();
          if( lci->finished ){
            level = 0;
        return CANCEL;
      }
       Tuple *tuple = new Tuple( lci->resultTupleType );
       tuple->PutAttribute(0, new CcInt(true, nodeid));
       tuple->PutAttribute(1, new Rectangle<dim>(box));
       tuple->PutAttribute(2, new CcInt(true, tupleid));
       result.setAddr(tuple);
     return YIELD;
     }else{
     level--;
     nodeid = lci->rtree->SmiNodeId(level);
         Tuple *tuple = new Tuple( lci->resultTupleType );
       tuple->PutAttribute(0, new CcInt(true, nodeid));
       tuple->PutAttribute(1, new Rectangle<dim>(box));
     tuple->PutAttribute(2, new CcInt(true, tupleid));
         result.setAddr(tuple);
     return YIELD;
      }
    }

    case CLOSE :
    {
      if(local.addr)
      {
        lci = (RTreeNodesLocalInfo<dim> *)local.addr;
        lci->resultTupleType->DeleteIfAllowed();
        delete lci;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "RTreeEntriesVM(): Received UNKNOWN message!" << endl;
  return 0;
}
ValueMapping RTreeEntries[] =
{ RTreeEntriesVM<2,TupleId>,  // 0
  RTreeEntriesVM<3,TupleId>,  // 1
  RTreeEntriesVM<4,TupleId>,  // 2
  RTreeEntriesVM<8,TupleId>   // 3
};

int RTreeEntriesSelect (ListExpr args)
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  string rtreeDescriptionStr;

  /* handle rtree part of argument */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  if(         nl->IsEqual(rtreeKeyType, Rectangle<2>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo))
    return 0;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<3>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo))
    return 1;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<4>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo))
    return 2;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<8>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo))
    return 3;

  return -1;
}

/*
Value mapping for ~entries~

*/

ListExpr RTreeEntriesTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 1){
    return listutils::typeError("Expected exactly 1 argument.");
  };
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);
  ListExpr MBR_ATOM;
  if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<2>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<2>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<3>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<3>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<4>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<4>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<8>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<8>::BasicType()); }
  else return listutils::typeError("Unsupported rtree-type.");

  ListExpr reslist =
   nl->TwoElemList(
    nl->SymbolAtom(Symbol::STREAM()),
    nl->TwoElemList(
     nl->SymbolAtom(Tuple::BasicType()),
     nl->Cons(
      nl->TwoElemList(nl->SymbolAtom("Nodeid"),
                      nl->SymbolAtom(CcInt::BasicType())),
    nl->TwoElemList(nl->TwoElemList(nl->SymbolAtom("MBR"), MBR_ATOM),
          nl->TwoElemList(nl->SymbolAtom("Tupleid"),
                          nl->SymbolAtom(CcInt::BasicType()))
      )
     )
    )
   );
  return reslist;
}
Operator rtreeentries(
         "entries",             // name
         RTreeEntriesSpec,      // specification
         4,
         RTreeEntries,          // value mapping
         RTreeEntriesSelect,    // selection function
         RTreeEntriesTypeMap    // type mapping
        );


/*
5.12 Operator ~getFileInfo~

Returns a text object with statistical information on all files used by the
rtree.

The result has format ~file\_stat\_result~:

----
file_stat_result --> (file_stat_list)
file_stat_list   -->   epsilon
                     | file_statistics file_stat_list
file_statistics  -->   epsilon
                     | file_stat_field file_statistics
file_stat_field  --> ((keyname) (value))
keyname          --> string
value            --> string
----

5.12.1 TypeMapping of operator ~getFileInfo~

Uses ~RTree2TextTypeMap~.


5.12.2 ValueMapping of operator ~getFileInfo~

*/

template<unsigned dim,class LeafInfo>
int getFileInfoRtreeValueMap(Word* args, Word& result, int message,
                        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  FText* restext = (FText*)(result.addr);
  R_Tree<dim, LeafInfo> *rtree = (R_Tree<dim, LeafInfo> *)(args[0].addr);
  SmiStatResultType resVector(0);

  if ( (rtree != 0) && rtree->getFileStats(resVector) ){
    string resString = "[[\n";
    for(SmiStatResultType::iterator i = resVector.begin();
        i != resVector.end(); ){
      resString += "\t[['" + i->first + "'],['" + i->second + "']]";
      if(++i != resVector.end()){
        resString += ",\n";
      } else {
        resString += "\n";
      }
    }
    resString += "]]";
    restext->Set(true,resString);
  } else {
    restext->Set(false,"");
  }
  return 0;
};

ValueMapping getFileInfoRtreeValueMapArray[] =
{ getFileInfoRtreeValueMap<2, TupleId>,  // 0
  getFileInfoRtreeValueMap<3, TupleId>,  // 1
  getFileInfoRtreeValueMap<4, TupleId>,  // 2
  getFileInfoRtreeValueMap<8, TupleId>,  // 3
  getFileInfoRtreeValueMap<2, TwoLayerLeafInfo>,  // 4
  getFileInfoRtreeValueMap<3, TwoLayerLeafInfo>,  // 5
  getFileInfoRtreeValueMap<4, TwoLayerLeafInfo>,  // 6
  getFileInfoRtreeValueMap<8, TwoLayerLeafInfo>   // 7
};

/*
5.12.3 Selection Function for operator ~getFileInfo~

Uses ~RTreeInquirySelect~.


5.12.4 Specification of operator ~getFileInfo~

*/
const string getFileInfoRtreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(btree(tuple(x) ti) xi -> text)</text--->"
  "<text>getFileInfo( _ )</text--->"
  "<text>Retrieve statistical infomation on the file(s) used by the rtree "
  "instance.</text--->"
  "<text>query getFileInfo(Trains_Trip_rtree)</text--->"
  ") )";


/*
5.12.5 Definition of operator ~getFileInfo~

*/
Operator getfileinfortree(
         "getFileInfo",                 // name
         getFileInfoRtreeSpec,          // specification
         8,                             // number of value mapping functions
         getFileInfoRtreeValueMapArray, // value mapping
         RTreeInquirySelect,            // selection function
         RTree2TextTypeMap              // type mapping
        );



const string UpdatebulkloadrtreeSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" \"Comment\" ) "
  "(<text>(stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)\n"
  "((stream (tuple (x1 t1)...(xn tn) "
  "(id tid)(low int)(high int))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti true)</text--->"
  "<text>_ _ updatebulkloadrtree [ _ ]</text--->"
  "<text>Creates an rtree<D> applying bulk loading on an existing file."
  "This means, the operator expects the input stream of tuples to be ordered "
  "in some meaningful way in order to reduce overlapping of "
  "bounding boxes (e.g. a Z-ordering on the bounding boxes). "
  "The R-Tree is created bottom up by gouping as many entries as "
  "possible into the leaf nodes and then creating the higher levels. "
  "The key type ti must be of kind SPATIAL2D, SPATIAL3D, SPATIAL4D "
  "or Spatial8D, or of type rect, rect2, rect3, rect4 or rect8.</text--->"
  "<text>let UTOrdered_2 = UTOrdered_1 UTOrdered feed addid sortby[UTrip asc] "
  "filter[tid2int(.TID) > 30000] updatebulkloadrtree[UTrip]</text--->"
  "<text></text--->"
  ") )";


/*
UpdateBulkLoad for r-tree

*/

ListExpr UpdateBulkLoadTypeMap(ListExpr args)
{

//  cout<<"UpdateBulkLoadTypeMap"<<endl;
// check number of parameters
  if( nl->IsEmpty(args) || nl->ListLength(args) != 3){
    return listutils::typeError("Expecting exactly 3 arguments.");
  }

  // split to parameters
  ListExpr tupleStream = nl->Second(args),
           attrNameLE = nl->Third(args);

/////////////////////////////////////////////////////////
  ListExpr firstpara = nl->First(args);
  if(nl->ListLength(firstpara) != 4){
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

  // check stream
  if(!listutils::isTupleStream(tupleStream)){
    return listutils::typeError("Expecting a tuplestream as 1st argument.");
  }
  // check key attribute name
  if(!listutils::isSymbol(attrNameLE)){
    return listutils::typeError("Expecting an attribute name as 2nd argument.");
  }
  string attrName = nl->SymbolValue(attrNameLE);
  // check if key attribute is from stream
  ListExpr attrList = nl->Second(nl->Second(tupleStream));
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if(attrIndex <= 0){
    return listutils::typeError("Expecting the attribute (2nd argument) being "
                                "part of the tuplestream (1st argument).");
  }
  // check for type of key attribute
  if( !(listutils::isSpatialType(attrType) || listutils::isRectangle(attrType)))
  {
      return listutils::typeError("Expecting the key attribute (2nd argument) "
            "being of kind SPATIAL2D, SPATIAL3D, Spatial4D or SPATIAL8D, of of "
            "type rect, rect3, rect4, or rect8.");
  }
  string rtreetype;
  if(    listutils::isKind(attrType, Kind::SPATIAL2D())
      || listutils::isSymbol(attrType, Rectangle<2>::BasicType())){
    rtreetype = RTree2TID::BasicType();
  }
  else if(    listutils::isKind(attrType, Kind::SPATIAL3D())
      || listutils::isSymbol(attrType, Rectangle<3>::BasicType())){
    rtreetype = RTree3TID::BasicType();
  }
  else if(    listutils::isKind(attrType, Kind::SPATIAL4D())
      || listutils::isSymbol(attrType, Rectangle<4>::BasicType())){
    rtreetype = RTree4TID::BasicType();
  }
  else if(    listutils::isKind(attrType, Kind::SPATIAL8D())
      || listutils::isSymbol(attrType, Rectangle<8>::BasicType())){
    rtreetype = RTree8TID::BasicType();
  }
  else  return listutils::typeError("Unsupported key type.");

/*
Now we have two possibilities:

- multi-entry indexing, or
- double indexing

For multi-entry indexing, one and only one of the attributes
must be a tuple identifier. In the latter, together with
a tuple identifier, the last two attributes must be of
integer type (~int~).

In the first case, a standard R-Tree is created possibly
containing several entries to the same tuple identifier, and
in the latter, a double index R-Tree is created using as low
and high parameters these two last integer numbers.

*/

    ListExpr first, rest, newAttrList, lastNewAttrList;
    int tidIndex = 0;
    string type;
    bool firstcall = true,
    doubleIndex = false;

    int nAttrs = nl->ListLength( attrList );
    rest = attrList;
    int j = 1;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);

      type = nl->SymbolValue(nl->Second(first));
      if (type == TupleIdentifier::BasicType())
      {
        if( tidIndex != 0 ){
          return listutils::typeError("Expecting exactly one attribute of type "
                                      "'tid' in the 1st argument.");
        }
        tidIndex = j;
      }
      else if( j == nAttrs - 1 && type == CcInt::BasicType() &&
               nl->SymbolValue(
               nl->Second(nl->First(rest))) == CcInt::BasicType() )
      { // the last two attributes are integers
        doubleIndex = true;
      }
      else
      {
        if (firstcall)
        {
          firstcall = false;
          newAttrList = nl->OneElemList(first);
          lastNewAttrList = newAttrList;
        }
        else
        {
          lastNewAttrList = nl->Append(lastNewAttrList, first);
        }
      }
      j++;
    }
    if( tidIndex == 0 ){
      return listutils::typeError("Expecting exactly one attribute of type "
                                  "'tid' in the 1st argument.");
    }

    return
        nl->ThreeElemList(
        nl->SymbolAtom(Symbol::APPEND()),
    nl->TwoElemList(
        nl->IntAtom(attrIndex),
    nl->IntAtom(tidIndex)),
    nl->FourElemList(
        nl->SymbolAtom(rtreetype),
    nl->TwoElemList(
        nl->SymbolAtom(Tuple::BasicType()),
    newAttrList),
    attrType,
    nl->BoolAtom(doubleIndex)));

}



/*
Bulkload an R-tree with an input R-tree so that they map to the same file

*/
template<unsigned dim>
int UpdateBulkLoadFun(Word* args, Word& result, int message,Word& local,
Supplier s)
{
//  const int dim = 3;
  Word wTuple;
//R_Tree<3,TupleId>* rtree_in1 = static_cast<R_Tree<3,TupleId>*>(args[0].addr);
//R_Tree<3,TupleId>* rtree_temp = (R_Tree<3,TupleId>*)qp->ResultStorage(s).addr;
  R_Tree<dim,TupleId>* rtree_in1 =
                        static_cast<R_Tree<dim,TupleId>*>(args[0].addr);
  R_Tree<dim,TupleId>* rtree_temp =
                        (R_Tree<dim,TupleId>*)qp->ResultStorage(s).addr;

  rtree_temp->CloseFile();


//  R_Tree<dim, TupleId> *rtree =
//                new R_Tree<3,TupleId>(rtree_in1->FileId(),4000);
  R_Tree<dim, TupleId> *rtree =
                new R_Tree<dim,TupleId>(rtree_in1->FileId(),4000);

  int attrIndex = ((CcInt*)args[3].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[4].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  bool BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);

  qp->Open(args[1].addr);
  qp->Request(args[1].addr, wTuple);
  while (qp->Received(args[1].addr))
  {
    if ((count++ % 10000) == 0)
    {
      // build a two elem list (simple count)
      NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
      msg->Send(msgList);
    }
    Tuple* tuple = (Tuple*)wTuple.addr;

    if( ((StandardSpatialAttribute<dim>*)tuple->
              GetAttribute(attrIndex))->IsDefined() &&
              ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->
              IsDefined() )
    {
        BBox<dim> box = ((StandardSpatialAttribute<dim>*)tuple->
              GetAttribute(attrIndex))->BoundingBox();

        R_TreeLeafEntry<dim, TupleId>
              e( box,
                 ((TupleIdentifier *)tuple->
                     GetAttribute(tidIndex))->GetTid() );
        rtree->InsertBulkLoad(e);
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[1].addr, wTuple);
  }
  qp->Close(args[1].addr);

  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert( FinalizedBulkLoad );

  // build a two elem list (simple count)
  NList msgList( NList("simple"), NList(count) );
      // send the message, the message center will call
      // the registered handlers. Normally the client applications
      // will register them.
  msg->Send(msgList);
  rtree->SwitchHeader(rtree_in1);
  result.setAddr(rtree);
  return 0;
}
/*
Build RTree on new coming units and store it into an existing RTree file

*/

ValueMapping VMUpdateBulkLoadFun[]=
{
  UpdateBulkLoadFun<2>,
  UpdateBulkLoadFun<3>,
  UpdateBulkLoadFun<4>,
  UpdateBulkLoadFun<8>,
};

/*
5.2. Selection Function for Operator ~creatertree\_bulkload<D>~

*/
int UpdateBulkLoadSelect (ListExpr args)
{
//   ListExpr relDescription = nl->First(args),
//   attrNameLE = nl->Second(args),
//   tupleDescription = nl->Second(relDescription),
//   attrList = nl->Second(tupleDescription);

   ListExpr relDescription = nl->Second(args),
   attrNameLE = nl->Third(args),
   tupleDescription = nl->Second(relDescription),
   attrList = nl->Second(tupleDescription);

  string attrName = nl->SymbolValue(attrNameLE);
  int attrIndex;
  ListExpr attrType;
  attrIndex = FindAttribute(attrList, attrName, attrType);
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  int result;

  if ( algMgr->CheckKind(Kind::SPATIAL2D(), attrType, errorInfo) )
    result = 0;
  else if ( algMgr->CheckKind(Kind::SPATIAL3D(), attrType, errorInfo) )
    result = 1;
  else if ( algMgr->CheckKind(Kind::SPATIAL4D(), attrType, errorInfo) )
    result = 2;
  else if ( algMgr->CheckKind(Kind::SPATIAL8D(), attrType, errorInfo) )
    result = 3;
  else
    return -1; /* should not happen */

  if( nl->SymbolValue(nl->First(relDescription)) == Symbol::STREAM())
  {
    ListExpr first,
    rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
     return result;
  }
  return -1;
}

Operator updatebulkloadrtree(
        "updatebulkloadrtree",
        UpdatebulkloadrtreeSpec,
//        UpdateBulkLoadFun,
        4,
        VMUpdateBulkLoadFun,
//        Operator::SimpleSelect,
        UpdateBulkLoadSelect,
        UpdateBulkLoadTypeMap
);



/*
5.13 Operator ~getNodeInfo~

This operator allows introspection of an RTree. It creates a stream
of a tuple, which describes one node of the R-Tree. The node is
selected by the given recordnumber (int-value).
The stream is empty, if the given recordnumber is not existing/not valid.
MBRsize is the size of the node-MBR,
MBRdead is the size of the node-MBR, which is not covered by a son-MBR,
MBRoverlapSize is the size of the node-MBR, where two (or more) son-MBRs overlap,
MBRoverlapsNo is the count of overlapping son-MBRs and
MBRdensity is the number of son-MBRs per MBRsize.

Signature is

----
    getNodeInfo: (rtree<D>) x int --> stream(tuple((NodeId int) (MBR rect<D>)
                                             (NoOfSons int) (IsLeafNode bool)
                                             (IsRootNode bool) (MBRsize real)
                                         (MBRdead real) (MBRoverlapSize real)
                                        (MBRoverlapsNo int) (MBRdensity real)))

----

*/

/*
5.13.1 TypeMapping for Operator ~getNodeInfo~

The typemapping function uses a template parameter InfoType to differ between
the typemapping of the operators ~getNodeInfo~, ~getNodeSons~ and
~getLeafEntries~.
The typemapping of operator ~getNodeInfo~ uses InfoType = NODEINFO.

*/

enum InfoType { NODEINFO, NODESONS, LEAFENTRIES };

template <InfoType iTyp>
ListExpr RTreeGetInfoTypeTypeMap(ListExpr args)
{
  if(nl->IsEmpty(args) || nl->IsAtom(args) || nl->ListLength(args) != 2){
    return listutils::typeError("Expected exactly 2 arguments.");
  };
  // test first argument for rtree
  ListExpr rtreeDescription = nl->First(args);
  if( !listutils::isRTreeDescription(rtreeDescription ) ){
    return listutils::typeError("Expected rtree<dim> as 1st argument.");
  }
  // check type of rtree
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);
  ListExpr MBR_ATOM;
  if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL2D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<2>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<2>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL3D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<3>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<3>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL4D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<4>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<4>::BasicType()); }
  else if(    listutils::isKind(rtreeKeyType, Kind::SPATIAL8D())
      || listutils::isSymbol(rtreeKeyType, Rectangle<8>::BasicType()) ){
      MBR_ATOM = nl->SymbolAtom(Rectangle<8>::BasicType()); }
  else return listutils::typeError("Unsupported rtree-type.");

  // test second argument for integer
  ListExpr RecNumber = nl->Second(args);
  if ( !nl->IsAtom(RecNumber) || !listutils::isNumericType(RecNumber) )
  {
    return listutils::typeError("Expecting int as second argument.");
  }

  ListExpr reslist = nl->Empty();
  switch (iTyp) {
    case NODEINFO:
    {
      reslist =
       nl->TwoElemList(
        nl->SymbolAtom(Symbol::STREAM()),
        nl->TwoElemList(
         nl->SymbolAtom(Tuple::BasicType()),
         nl->Cons(
         nl->TwoElemList(nl->SymbolAtom("NodeId"),
                         nl->SymbolAtom(CcInt::BasicType())),
         nl->Cons(
          nl->TwoElemList(nl->SymbolAtom("MBR"), MBR_ATOM),
         nl->Cons(
          nl->TwoElemList(nl->SymbolAtom("NoOfSons"),
                          nl->SymbolAtom(CcInt::BasicType())),
         nl->Cons(
          nl->TwoElemList(nl->SymbolAtom("IsLeafNode"),
                          nl->SymbolAtom(CcBool::BasicType())),
          nl->SixElemList(
          nl->TwoElemList(nl->SymbolAtom("IsRootNode"),
                          nl->SymbolAtom(CcBool::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("MBRsize"),
                          nl->SymbolAtom(CcReal::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("MBRdead"),
                          nl->SymbolAtom(CcReal::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("MBRoverlapSize"),
                                          nl->SymbolAtom(CcReal::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("MBRoverlapsNo"),
                                          nl->SymbolAtom(CcInt::BasicType())),
          nl->TwoElemList(nl->SymbolAtom("MBRdensity"),
                          nl->SymbolAtom(CcReal::BasicType()))
      )))))));
      break;
    }
    case NODESONS:
    {
      reslist =
        nl->TwoElemList(
          nl->SymbolAtom(Symbol::STREAM()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            nl->Cons(
        nl->TwoElemList(nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(
        nl->TwoElemList(nl->SymbolAtom("SonId"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("SonMBR"), MBR_ATOM)
      ))));
      break;
    }
    case LEAFENTRIES:
    {
      reslist =
        nl->TwoElemList(
          nl->SymbolAtom(Symbol::STREAM()),
          nl->TwoElemList(
            nl->SymbolAtom(Tuple::BasicType()),
            nl->Cons(
        nl->TwoElemList(nl->SymbolAtom("NodeId"),
                        nl->SymbolAtom(CcInt::BasicType())),
        nl->TwoElemList(
        nl->TwoElemList(nl->SymbolAtom("TupleID"),
                        nl->SymbolAtom(TupleIdentifier::BasicType())),
        nl->TwoElemList(nl->SymbolAtom("EntryMBR"), MBR_ATOM)
      ))));
      break;
    }
  }
  return reslist;
}


/*
5.13.2 Value Mapping for Operator ~getNodeInfo~

*/

template<unsigned dim, class LeafInfo>
struct GetSonsInfo{
  int index;
  int maxEntries;
  R_TreeNode<dim, LeafInfo>* father;
  TupleType* ttype;
  long nodeId;
  bool IsRoot;

  GetSonsInfo( int i, int m, R_TreeNode<dim, LeafInfo>* f,
                              TupleType* tt, long ni, bool ir) :
                                         index(i), maxEntries(m),
                                         father(f), ttype(tt),
                                         nodeId(ni), IsRoot(ir) {}
};

/*
Structure to handle the information of the interesting node.

*/

template <unsigned dim, class LeafInfo>
bool checkRecordId(R_Tree<dim, LeafInfo>* rtree, int nodeId)
{
  bool findRID = false;
  if (   (nodeId > 0)
      && (nodeId != static_cast<int>(rtree->HeaderRecordId()))
      && (nodeId < rtree->NodeCount()+2))
      findRID = true;
  return findRID;
}

/*
Checks if the given nodeId is valid in rtree.

*/



struct seg_node {
double bottom, top;
int count;
double measure;
seg_node *left, *right;

seg_node(double b, double t): bottom(b), top(t), count(0),
                          measure(0.0), left(0), right(0) {}
};
/*
node-structure of the segment tree used for 2D-sweepline

*/

struct quad_node {
double left, right, bottom, top;
int count;
double measure;
quad_node *bottomleft, *bottomright, *topleft, *topright;

quad_node(double l, double r, double b, double t):
          left(l), right(r), bottom(b), top(t),
          count(0), measure(0.0),
          bottomleft(0), bottomright(0), topleft(0), topright(0)  {}
};
/*
node-structure of the quad tree used for sweepline with dimensions dim $ >=$ 3

*/

struct event {
double xVal;
int index;

event(): xVal(0.), index(0) {}
event(double x, int i): xVal(x), index(i) {}
};

struct eventComp {
  bool operator() (const event& lhs, const event& rhs) const
  {
    return lhs.xVal<rhs.xVal;
  }
};
/*
insertion or deletion event at x-value for MBR with named index
eventComp is the comparator for the sorted set

*/

seg_node* buildTree(set<double> &yVals)
{
  seg_node* sTree;
  set<double>::iterator yValiter = yVals.begin();
  if (yVals.size()==2) { //segment is the new leaf
    double b = *yValiter++;
    double t = *yValiter;
    sTree = new seg_node(b,t);
  }
  else
  {  //divide the list in the middle
    int subsetSize = yVals.size()/2;
    for (int i=0; i<subsetSize; i++)
      yValiter++;
    set<double> yValsRight (yValiter++, yVals.end());
    set<double> yValsLeft (yVals.begin(), yValiter);
    //left element of right list is equal to
    //right element of the left list
    seg_node* sTreeLeft = buildTree(yValsLeft);
    seg_node* sTreeRight = buildTree(yValsRight);
    //create new node with trees of the two sublists
    sTree = new seg_node(sTreeLeft->bottom, sTreeRight->top);
    sTree->left = sTreeLeft;
    sTree->right = sTreeRight;
  }
  return sTree;
}
/*
builds the segment tree for the given sorted set of y-values

*/

void deleteSegTree(seg_node* segTree)
{
  if (segTree==0) return;
  deleteSegTree(segTree->left);
  deleteSegTree(segTree->right);
  delete segTree;
  segTree = 0;
}
/*
deletes the segment tree

*/

quad_node* buildTree(set<double> &yVals, set<double> &zVals)
{
  quad_node* qTree;
  set<double>::iterator yValiter = yVals.begin();
  set<double>::iterator zValiter = zVals.begin();
  if ((yVals.size()==2)&&(zVals.size()==2)) {
    //quad is the new leaf
    double l = *yValiter++;
    double r = *yValiter;
    double b = *zValiter++;
    double t = *zValiter;
    qTree = new quad_node(l, r, b, t);
  }
  else
  if (yVals.size()==2) { //divide the z-list in the middle
    int subsetSizeZ = zVals.size()/2;
    for (int i=0; i<subsetSizeZ; i++)
      zValiter++;
    set<double> zValsTop (zValiter++, zVals.end());
    set<double> zValsBottom (zVals.begin(), zValiter);
    //top element of bottom list is equal to
    //bottom element of the top list

    quad_node* qTreeBottom = buildTree(yVals, zValsBottom);
    quad_node* qTreeTop = buildTree(yVals, zValsTop);

    //create new node with trees of the two sublists
    qTree = new quad_node(qTreeBottom->left, qTreeTop->right,
                          qTreeBottom->bottom, qTreeTop->top);
    qTree->bottomright = qTreeBottom;
    qTree->topright = qTreeTop;
  }
  else
  if (zVals.size()==2) { //divide the y-list in the middle
    int subsetSizeY = yVals.size()/2;
    for (int i=0; i<subsetSizeY; i++)
      yValiter++;
    set<double> yValsRight (yValiter++, yVals.end());
    set<double> yValsLeft (yVals.begin(), yValiter);
    //left element of right list is equal to
    //right element of the left list

    quad_node* qTreeLeft = buildTree(yValsLeft, zVals);
    quad_node* qTreeRight = buildTree(yValsRight, zVals);

    //create new node with trees of the two sublists
    qTree = new quad_node(qTreeLeft->left, qTreeRight->right,
                          qTreeLeft->bottom, qTreeRight->top);
    qTree->topleft = qTreeLeft;
    qTree->topright = qTreeRight;
  }
  else
  { //divide the two list in the middle
    int subsetSizeY = yVals.size()/2;
    for (int i=0; i<subsetSizeY; i++)
      yValiter++;
    set<double> yValsRight (yValiter++, yVals.end());
    set<double> yValsLeft (yVals.begin(), yValiter);
    //left element of right list is equal to
    //right element of the left list

    int subsetSizeZ = zVals.size()/2;
    for (int i=0; i<subsetSizeZ; i++)
      zValiter++;
    set<double> zValsTop (zValiter++, zVals.end());
    set<double> zValsBottom (zVals.begin(), zValiter);
    //top element of bottom list is equal to
    //bottom element of the top list

    quad_node* qTreeBottomLeft = buildTree(yValsLeft, zValsBottom);
    quad_node* qTreeBottomRight = buildTree(yValsRight, zValsBottom);
    quad_node* qTreeTopLeft = buildTree(yValsLeft, zValsTop);
    quad_node* qTreeTopRight = buildTree(yValsRight, zValsTop);

    //create new node with trees of the two sublists
    qTree = new quad_node(qTreeBottomLeft->left, qTreeTopRight->right,
                          qTreeBottomLeft->bottom, qTreeTopRight->top);
    qTree->bottomleft = qTreeBottomLeft;
    qTree->bottomright = qTreeBottomRight;
    qTree->topleft = qTreeTopLeft;
    qTree->topright = qTreeTopRight;
  }
  return qTree;
}
/*
builds the quad tree for the given sorted set of y- and z-values

*/

void deleteQuadTree(quad_node* quadTree)
{
  if (quadTree==0) return;
  deleteQuadTree(quadTree->bottomleft);
  deleteQuadTree(quadTree->bottomright);
  deleteQuadTree(quadTree->topleft);
  deleteQuadTree(quadTree->topright);
  delete quadTree;
  quadTree = 0;
}
/*
deletes the quad tree

*/

void deleteNode(seg_node* sTree, double b, double t)
{
  if ((sTree->bottom >= b)&&(sTree->top <= t))
  {
    sTree->count--;
    if (sTree->count==0)
      sTree->measure = 0;
  }
  else
  {
    if (b < sTree->left->top) deleteNode(sTree->left, b, t);
    if (sTree->right->bottom < t) deleteNode(sTree->right, b, t);
    if (sTree->count == 0)
      sTree->measure = sTree->left->measure + sTree->right->measure;
  }
}
/*
deletes the entry of segment (b)-(t) in the segment tree

*/

void insertNode(seg_node* sTree, double b, double t)
{
  if ((sTree->bottom >= b)&&(sTree->top <= t))
  {
    sTree->count++;
    if (sTree->count==1)
      sTree->measure = sTree->top - sTree->bottom;
  }
  else
  {
    if (b < sTree->left->top) insertNode(sTree->left, b, t);
    if (sTree->right->bottom < t) insertNode(sTree->right, b, t);
    if (sTree->count == 0)
      sTree->measure = sTree->left->measure + sTree->right->measure;
  }
}
/*
inserts the entry of segment (b)-(t) in the segment tree

*/


void deleteNode(quad_node* sTree, double l, double r, double b, double t)
{
  if ((sTree->left >= l)&&(sTree->right <= r)
    &&(sTree->bottom >= b)&&(sTree->top <= t))
  {
    sTree->count--;
    if (sTree->count==0)
      sTree->measure = 0.0;
  }
  else
  {
    if ((sTree->bottomleft)
      &&(sTree->bottomleft->left < r)&&(sTree->bottomleft->bottom < t))
      deleteNode(sTree->bottomleft, l, r, b, t);
    if ((sTree->bottomright)
      &&(l < sTree->bottomright->right)&&(sTree->bottomright->bottom < t))
      deleteNode(sTree->bottomright, l, r , b, t);
    if ((sTree->topleft)
      &&(sTree->topleft->left < r)&&(b < sTree->topleft->top))
      deleteNode(sTree->topleft, l, r, b, t);
    if ((sTree->topright)
      &&(l < sTree->topright->right)&&(b < sTree->topright->top))
      deleteNode(sTree->topright, l, r , b, t);

    if (sTree->count == 0)
    {
      sTree->measure = 0.0;
      if (sTree->bottomleft) sTree->measure += sTree->bottomleft->measure;
      if (sTree->bottomright) sTree->measure += sTree->bottomright->measure;
      if (sTree->topleft) sTree->measure += sTree->topleft->measure;
      if (sTree->topright) sTree->measure += sTree->topright->measure;
    }
  }
}
/*
deletes the entry of rectangle (l,b)-(r,t) in the quad tree

*/

void insertNode(quad_node* sTree, double l, double r, double b, double t)
{
  if ((sTree->left >= l)&&(sTree->right <= r)
    &&(sTree->bottom >= b)&&(sTree->top <= t))
  {
    sTree->count++;
    if (sTree->count==1)
      sTree->measure = (sTree->right - sTree->left)
                     * (sTree->top - sTree->bottom);
  }
  else
  {
    if ((sTree->bottomleft)
      &&(sTree->bottomleft->left < r)&&(sTree->bottomleft->bottom < t))
      insertNode(sTree->bottomleft, l, r, b, t);
    if ((sTree->bottomright)
      &&(l < sTree->bottomright->right)&&(sTree->bottomright->bottom < t))
      insertNode(sTree->bottomright, l, r , b, t);
    if ((sTree->topleft)
      &&(sTree->topleft->left < r)&&(b < sTree->topleft->top))
      insertNode(sTree->topleft, l, r, b, t);
    if ((sTree->topright)
      &&(l < sTree->topright->right)&&(b < sTree->topright->top))
      insertNode(sTree->topright, l, r , b, t);

    if (sTree->count == 0)
    {
      sTree->measure = 0.0;
      if (sTree->bottomleft) sTree->measure += sTree->bottomleft->measure;
      if (sTree->bottomright) sTree->measure += sTree->bottomright->measure;
      if (sTree->topleft) sTree->measure += sTree->topleft->measure;
      if (sTree->topright) sTree->measure += sTree->topright->measure;
    }
  }
}
/*
inserts the entry of rectangle (l,b)-(r,t) in the quad tree

*/

template<unsigned dim>
double getSweepAreas2D(vector< Rectangle<dim> > BBox)
{
  if (dim!=2) return 0.0; //SweepLine with segmenttree only for dim=2
  if (BBox.empty()) return 0.0; //area of empty rectangle is 0

  unsigned int it = 0;  //remove rectangles with size=0 (e.g. points/lines)
  while (it<BBox.size())
  {
    if ((abs((BBox[it].MinD(0)-BBox[it].MaxD(0))/BBox[it].MaxD(0))<1E-8)
      ||(abs((BBox[it].MinD(1)-BBox[it].MaxD(1))/BBox[it].MaxD(1))<1E-8))
    { BBox.erase(BBox.begin()+it); }
    else
    { it++;
    }
  }
  if (BBox.empty()) return 0.0;  //area of empty rectangle is 0

  seg_node* segmentTree;
  set<double> xVals;
  set<double> yVals;
  multiset<event, eventComp> insertMBR;
  multiset<event, eventComp> deleteMBR;
  multiset<event, eventComp>::iterator iterateInsert;
  multiset<event, eventComp>::iterator iterateDelete;

  for (unsigned int i=0; i<BBox.size(); i++)
  //build sorted lists of values and events
  {
    xVals.insert(BBox[i].MinD(0));
    xVals.insert(BBox[i].MaxD(0));
    insertMBR.insert(event(BBox[i].MinD(0),i));
    deleteMBR.insert(event(BBox[i].MaxD(0),i));
    yVals.insert(BBox[i].MinD(1));
    yVals.insert(BBox[i].MaxD(1));
  }
  segmentTree = buildTree(yVals);

  //initialization of the x-value-list
  double cover = 0.0;
  double x = 0.0;
  set<double>::iterator xValiter;
  xValiter = xVals.begin();
  iterateInsert = insertMBR.begin();
  iterateDelete = deleteMBR.begin();
  x = *xValiter;
  xValiter++;

  while (xValiter != xVals.end()) {
  //sweep line for all x-values
    while ((iterateDelete!=deleteMBR.end())&&
          ((*iterateDelete).xVal == x)) {
      //delete segments from tree at position x
      int index = (*iterateDelete).index;
      deleteNode(segmentTree, BBox[index].MinD(1),
                              BBox[index].MaxD(1));
      iterateDelete++;
    }
    while ((iterateInsert!=insertMBR.end())&&
           ((*iterateInsert).xVal == x)) {
      //insert segments from tree at position x
      int index = (*iterateInsert).index;
      insertNode(segmentTree, BBox[index].MinD(1),
                              BBox[index].MaxD(1));
      iterateInsert++;
    }
    cover += (segmentTree->measure) * (*xValiter - x);
    x = *xValiter;
    xValiter++;
  }
  deleteSegTree(segmentTree);
  return cover;
}
/*
sweep line algorithm with segment tree only for dimension 2

*/

template<unsigned dim>
double getSweepAreas(vector< Rectangle<dim> > BBox)
{
  if (dim<1) return 0.0; //no Area available
  if (dim==2) return getSweepAreas2D(BBox); //2D-Sweepline
  //SweepLine with Quadtree is only for dim>2
  if (BBox.size()<1) return 0.0;  //area of empty rectangle is 0

  unsigned int it = 0;  //remove rectangles with size=0 (e.g. points/lines)
  bool zero = false;
  while (it<BBox.size())
  {
    zero = false;
    for (unsigned int j=0; j<dim; j++)
      zero |= (abs((BBox[it].MinD(j)-BBox[it].MaxD(j))/BBox[it].MaxD(j))<1E-8);
    if (zero)
    { BBox.erase(BBox.begin()+it); }
    else
    { it++; }
  }
  if (BBox.empty()) return 0.0;  //area of empty rectangle is 0

  quad_node* quadTreeYZ;
  set<double> xVals[dim-1];
  set<double> yVals;
  set<double> zVals;
  multiset<event, eventComp> insertMBR[dim-1];
  multiset<event, eventComp> deleteMBR[dim-1];
  multiset<event, eventComp>::iterator iterateInsert[dim-1];
  multiset<event, eventComp>::iterator iterateDelete[dim-1];

  for (unsigned int i=0; i<BBox.size(); i++)
  //build sorted lists of values and events
  {
    for (unsigned int j=0; j<dim-2; j++)
    {
      xVals[j].insert(BBox[i].MinD(j));
      xVals[j].insert(BBox[i].MaxD(j));
      insertMBR[j].insert(event(BBox[i].MinD(j),i));
      deleteMBR[j].insert(event(BBox[i].MaxD(j),i));
    }
    yVals.insert(BBox[i].MinD(dim-2));
    yVals.insert(BBox[i].MaxD(dim-2));
    zVals.insert(BBox[i].MinD(dim-1));
    zVals.insert(BBox[i].MaxD(dim-1));
  }
  quadTreeYZ = buildTree(yVals, zVals);

  set<double>::iterator xValiter[dim-1];
  double coversweep[dim];
  for (unsigned int j=0; j<dim; j++)
  {
    coversweep[j] = 0.0;
  }

  double x[dim-1];
  set<int> xindex[dim-1];
  unsigned int di;
  bool again;
  set<int>::iterator indexIter;

  //initialization of the lists
  for (unsigned int j=0; j<dim-3; j++)
  {
    xValiter[j] = xVals[j].begin();
    iterateInsert[j] = insertMBR[j].begin();
    iterateDelete[j] = deleteMBR[j].begin();
    x[j] = *xValiter[j];
    xValiter[j]++;
    while ((iterateInsert[j]!=insertMBR[j].end())&&
           ((*iterateInsert[j]).xVal == x[j])) {
      //insert index of valid rectangles at position <x>
      int index = (*iterateInsert[j]).index;
      xindex[j].insert(index);
      iterateInsert[j]++;
    }
  }
  xValiter[dim-3] = xVals[dim-3].begin();
  iterateInsert[dim-3] = insertMBR[dim-3].begin();
  iterateDelete[dim-3] = deleteMBR[dim-3].begin();
  x[dim-3] = *xValiter[dim-3];
  xValiter[dim-3]++;

  while (xValiter[0] != xVals[0].end())
  //sweep "hypearea" for the values of all dimensions except the last two
  {
    while ((iterateDelete[dim-3]!=deleteMBR[dim-3].end())&&
          ((*iterateDelete[dim-3]).xVal == x[dim-3])) {
      //delete rectangles from tree at position <x>
      int index = (*iterateDelete[dim-3]).index;
      xindex[dim-3].erase(index);
      iterateDelete[dim-3]++;
      bool all = true;
      for (unsigned int j=0; j<dim-3;j++)
        all &=(xindex[j].find(index)!=xindex[j].end());
      if (all)
        deleteNode(quadTreeYZ,
                BBox[index].MinD(dim-2),
                BBox[index].MaxD(dim-2),
                BBox[index].MinD(dim-1),
                BBox[index].MaxD(dim-1));
    }
    while ((iterateInsert[dim-3]!=insertMBR[dim-3].end())&&
          ((*iterateInsert[dim-3]).xVal == x[dim-3])) {
      //insert rectangles from tree at position <x>
      int index = (*iterateInsert[dim-3]).index;
      xindex[dim-3].insert(index);
      iterateInsert[dim-3]++;
      bool all = true;
      for (unsigned int j=0; j<dim-3;j++)
        all &=(xindex[j].find(index)!=xindex[j].end());
      if (all)
        insertNode(quadTreeYZ,
                BBox[index].MinD(dim-2),
                BBox[index].MaxD(dim-2),
                BBox[index].MinD(dim-1),
                BBox[index].MaxD(dim-1));
    }
    coversweep[dim-2] = (quadTreeYZ->measure);

    di = dim-3;
    again = true;
    while ((di>=0)&&again)
    {
      //set "sweepline" to next position <x>
      coversweep[di]  += coversweep[di+1] * (*xValiter[di] - x[di]);
      coversweep[di+1] = 0.0;
      x[di] = *xValiter[di];
      xValiter[di]++;
      again = false;
      if ((xValiter[di] == xVals[di].end())&&(di>0))
      {
        if (di==dim-3) {
          //clear the tree from resting rectangles...
          while  ((iterateDelete[di]!=deleteMBR[di].end())&&
                 ((*iterateDelete[di]).xVal == x[di])) {
            int index = (*iterateDelete[di]).index;
            xindex[di].erase(index);
            iterateDelete[di]++;
            bool all = true;
            for (unsigned int j=0; j<dim-3;j++)
              all &=(xindex[j].find(index)!=xindex[j].end());
            if (all)
              deleteNode(quadTreeYZ,
                BBox[index].MinD(dim-2),
                BBox[index].MaxD(dim-2),
                BBox[index].MinD(dim-1),
                BBox[index].MaxD(dim-1));
          }
        }
        xValiter[di] = xVals[di].begin();
        iterateInsert[di] = insertMBR[di].begin();
        iterateDelete[di] = deleteMBR[di].begin();
        x[di] = *xValiter[di];
        xValiter[di]++;
        again = true;
      }
      while ((di!=dim-3)&&
             (iterateDelete[di]!=deleteMBR[di].end())&&
             ((*iterateDelete[di]).xVal == x[di])) {
        //delete index of non-valid rectangles at position <x>
        int index = (*iterateDelete[di]).index;
        xindex[di].erase(index);
        iterateDelete[di]++;
      }
      while ((di!=dim-3)&&
             (iterateInsert[di]!=insertMBR[di].end())&&
             ((*iterateInsert[di]).xVal == x[di])) {
        //insert index of valid rectangles at position <x>
        int index = (*iterateInsert[di]).index;
        xindex[di].insert(index);
        iterateInsert[di]++;
      }
      di--;
    }
  }
  deleteQuadTree(quadTreeYZ);
  return coversweep[0];
}
/*
sweep line algorithm with quad tree for dimensions $ >$ 2



...and finally:

Value mapping for operator ~getNodeInfo~:

*/


template<unsigned dim>
int RTreeGetNodeInfoVM( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  GetSonsInfo<dim, TupleId>* gsi;

  switch( message )
  {
    case OPEN:
    {
      int nodeId = ((CcInt*)args[1].addr)->GetIntval();
      R_Tree<dim, TupleId>* tree = (R_Tree<dim, TupleId>*) args[0].addr;
      if (checkRecordId(tree, nodeId))
      {
        int maxIntEntries = tree->MaxEntries(0);
        int maxLeafEntries = tree->MaxEntries(tree->Height());
        int maxEntr = maxIntEntries > maxLeafEntries ? maxIntEntries
            : maxLeafEntries;//needed to properly read the node.
        R_TreeNode<dim, TupleId>* f = new R_TreeNode<dim, TupleId>
                                           (false, 0, maxEntr);
        tree->GetNode(nodeId, *f);

        int m = f->EntryCount();

        bool isRootNode = (nodeId==static_cast<int>(tree->RootRecordId()));
        gsi = new GetSonsInfo<dim, TupleId>(-1, m, f, new TupleType
                      (nl->Second(GetTupleResultType(s))), nodeId, isRootNode);
        local.setAddr(gsi);
      }
      else
        local.setAddr(0);
      return 0;
    }

    case REQUEST :
    {
      if(local.addr == NULL)
      {
        return CANCEL;
      }
      gsi = (GetSonsInfo<dim, TupleId>*)local.addr;

      if (gsi->index == gsi->maxEntries)
      {
        return CANCEL;
      }

      double MBRsize = (gsi->father->BoundingBox()).Area();

      int numSons = gsi->father->IsLeaf()? 0 : gsi->maxEntries;

      double MBRdead = 0.0;
      vector< Rectangle<dim> > sonBB; //vector of BoundingBoxes of sons
      for (int i=0; i<gsi->maxEntries; i++)
      {
        sonBB.push_back(gsi->father->BoundingBox(i));
      }
      MBRdead = MBRsize - getSweepAreas(sonBB);

      int MBRoverlapsNo = 0;
      double MBRoverlapSize = 0.0;
      sonBB.clear();
      for (int i=0; i<gsi->maxEntries-1; i++)
      {
        for (int j= i+1; j<gsi->maxEntries; j++)
        {
        //if intersection between sonMBR_i and sonMBR_j
          if ((gsi->father->BoundingBox(i)).Intersects(
                                     gsi->father->BoundingBox(j)))
            {
            MBRoverlapsNo++;  //count the overlappings
            //and build a vector of the overlapped MBRs
            sonBB.push_back((gsi->father->BoundingBox(i)).Intersection(
                                     gsi->father->BoundingBox(j)));
            }
        }
      }
      MBRoverlapSize = getSweepAreas(sonBB); //get size of overlapped MBRs

      double MBRdensity = gsi->maxEntries / MBRsize;

      Tuple *tuple = new Tuple( gsi->ttype );
      tuple->PutAttribute(0, new CcInt(true, gsi->nodeId));
      tuple->PutAttribute(1, new Rectangle<dim>(gsi->father->BoundingBox()));
      tuple->PutAttribute(2, new CcInt(true, numSons));
      tuple->PutAttribute(3, new CcBool(true, gsi->father->IsLeaf()));
      tuple->PutAttribute(4, new CcBool(true, gsi->IsRoot));
      tuple->PutAttribute(5, new CcReal(true, MBRsize));
      tuple->PutAttribute(6, new CcReal(true, MBRdead));
      tuple->PutAttribute(7, new CcReal(true, MBRoverlapSize));
      tuple->PutAttribute(8, new CcInt(true, MBRoverlapsNo));
      tuple->PutAttribute(9, new CcReal(true, MBRdensity));
      result.setAddr(tuple);
      gsi->index = gsi->maxEntries;
      return YIELD;
    }

    case CLOSE :
    {
      if(local.addr)
      {
        gsi = (GetSonsInfo<dim, TupleId>*)local.addr;
        delete gsi->father;
        gsi->ttype->DeleteIfAllowed();
        delete gsi;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "RTreeGetNodeInfoVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
5.13.3 Specification for Operator ~getNodeInfo~

*/

const string RTreeGetNodeInfoSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> (tuple ((x1 t1)...(xn tn))) ti false) x int"
    "-> stream(tuple((NodeId int) (MBR rect<D>) (NoOfSons int) \n"
    "                (IsLeafNode bool) (IsRootNode bool) \n"
    "                (MBRsize real) (MBRdead real) (MBRoverlapSize real) \n"
    "                (MBRoverlapsNo int) (MBRdensity real)))</text--->"
    "<text>getNodeInfo( _ , _ )</text--->"
    "<text>This operator allows introspection of an RTree. It creates a "
    "stream of a tuple, which describes one node of the R-Tree.</text--->"
    "<text>query getNodeInfo(strassen_geoData_rtree, 2);</text--->"
    "<text>Stream is empty, if given record no is not existing.</text--->"
    ") )";

/*
5.13.4 Selection Function for Operator ~getNodeInfo~

The selection function is used for the operators ~getNodeInfo~,
~getNodeSons~ and ~getLeafEntries~.

*/

int RTreeGetInfoTypeSelect (ListExpr args)
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  string rtreeDescriptionStr;

  /* handle rtree part of argument */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  if(         nl->IsEqual(rtreeKeyType, Rectangle<2>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL2D(), rtreeKeyType, errorInfo))
    return 0;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<3>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL3D(), rtreeKeyType, errorInfo))
    return 1;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<4>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL4D(), rtreeKeyType, errorInfo))
    return 2;
  else if(    nl->IsEqual(rtreeKeyType, Rectangle<8>::BasicType())
           || algMgr->CheckKind(Kind::SPATIAL8D(), rtreeKeyType, errorInfo))
    return 3;

  return -1;
}

ValueMapping RTreeGetNodeInfo [] =
{ RTreeGetNodeInfoVM<2>,  // 0
  RTreeGetNodeInfoVM<3>,  // 1
  RTreeGetNodeInfoVM<4>,  // 2
  RTreeGetNodeInfoVM<8>   // 3
};


/*
5.13.5 Definition of Operator ~getNodeInfo~

*/
Operator rtreegetnodeinfo(
         "getNodeInfo",                       // name
         RTreeGetNodeInfoSpec,                // specification
         4,
         RTreeGetNodeInfo,                    // value mapping
         RTreeGetInfoTypeSelect,              // selection function
         RTreeGetInfoTypeTypeMap<NODEINFO>    // type mapping
        );




/*
5.14 Operator ~getNodeSons~

This operator allows introspection of an R-tree. It creates a stream
of tuples, each describe a son node of the node with the specified record no.
The stream is empty, if the given recordnumber is not existing/not valid.

Signature is

----
   getNodeSons: (rtree<D>) x int --> stream(tuple((NodeId int) (SonId int)
                                                  (SonMBR rect<D>)))

----

*/

/*
5.14.1 TypeMapping for Operator ~getNodeSons~

The typemapping function is the one of the operator ~getNodeInfo~ and uses
a template parameter InfoType to differ between the typemapping of the
operators ~getNodeInfo~, ~getNodeSons~ and ~getLeafEntries~.
The typemapping of operator ~getNodeSons~ uses InfoType = NODESONS.

*/


/*
5.14.2 Value Mapping for Operator ~getNodeSons~

*/

template<unsigned dim>
int RTreeGetNodeSonsVM( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  GetSonsInfo<dim, TupleId>* gsi;

  switch( message )
  {
    case OPEN:
    {
      int nodeId = ((CcInt*)args[1].addr)->GetIntval();
      R_Tree<dim, TupleId>* tree = (R_Tree<dim, TupleId>*) args[0].addr;
      if (checkRecordId(tree, nodeId))
      {
        int maxIntEntries = tree->MaxEntries(0);
        int maxLeafEntries = tree->MaxEntries(tree->Height());
        int maxEntr = maxIntEntries > maxLeafEntries ? maxIntEntries
            : maxLeafEntries;//needed to properly read the node.
        R_TreeNode<dim, TupleId>* f = new R_TreeNode<dim, TupleId>
                                           (false, 0, maxEntr);
        tree->GetNode(nodeId, *f);
        int m = f->EntryCount();
        bool isRootNode = (nodeId==static_cast<int>(tree->RootRecordId()));
        gsi = new GetSonsInfo<dim, TupleId>(0, m, f, new TupleType
                      (nl->Second(GetTupleResultType(s))), nodeId, isRootNode);
        local.setAddr(gsi);
      }
      else
        local.setAddr(0);
      return 0;
    }

    case REQUEST :
    {
      if(local.addr == NULL)
      {
        return CANCEL;
      }
      gsi = (GetSonsInfo<dim, TupleId>*)local.addr;

      if (gsi->index == gsi->maxEntries)
      {
        return CANCEL;
      }

      if (gsi->father->IsLeaf())
      {
        Tuple *tuple = new Tuple( gsi->ttype );
        tuple->PutAttribute(0, new CcInt(true, gsi->nodeId));
        tuple->PutAttribute(1, new CcInt(false, 0));
        tuple->PutAttribute(2, new Rectangle<dim>(false));
        result.setAddr(tuple);
        gsi->index = gsi->maxEntries;
/*
ensures that CANCEL will be returned at next call.

*/
        return YIELD;
      }

      Tuple *tuple = new Tuple( gsi->ttype );
      tuple->PutAttribute(0, new CcInt(true, gsi->nodeId));
      R_TreeInternalEntry<dim>* son = gsi->father->
                                GetInternalEntry(gsi->index);
      tuple->PutAttribute(1, new CcInt(true, son->pointer));
      tuple->PutAttribute(2, new Rectangle<dim>(son->box));
      result.setAddr(tuple);
      gsi->index++;
      return YIELD;
    }

    case CLOSE :
    {
      if(local.addr)
      {
        gsi = (GetSonsInfo<dim, TupleId>*)local.addr;
        delete gsi->father;
        gsi->ttype->DeleteIfAllowed();
        delete gsi;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "RTreeGetNodeSonsVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
5.14.3 Specification for Operator ~getNodeSons~

*/

const string RTreeGetNodeSonsSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> (tuple ((x1 t1)...(xn tn))) ti false) x int "
    "-> stream(tuple((NodeId int) (SonId int) (SonMBR rect<D>)))</text--->"
    "<text>getNodeSons( _ , _ )</text--->"
    "<text>This operator allows introspection of an RTree. It creates a "
    "stream of tuples, each describe a "
    "son node of the node with the specified record no.</text--->"
    "<text>query getNodeSons(strassen_geodata_rtree, 2);</text--->"
    "<text>Stream is empty, if given record no is not existing.</text--->"
    ") )";

/*
5.14.4 Selection Function for Operator ~getNodeSons~

The selection function is the one of the operator ~getNodeInfo~.

*/

ValueMapping RTreeGetNodeSons [] =
{ RTreeGetNodeSonsVM<2>,  // 0
  RTreeGetNodeSonsVM<3>,  // 1
  RTreeGetNodeSonsVM<4>,  // 2
  RTreeGetNodeSonsVM<8>   // 3
};


/*
5.14.5 Definition of Operator ~getNodeSons~

*/
Operator rtreegetnodesons(
         "getNodeSons",                       // name
         RTreeGetNodeSonsSpec,                // specification
         4,
         RTreeGetNodeSons,                    // value mapping
         RTreeGetInfoTypeSelect,              // selection function
         RTreeGetInfoTypeTypeMap<NODESONS>    // type mapping
        );


/*
5.14 Operator ~getLeafEntries~

This operator allows introspection of an R-tree. It creates a stream
of tuples, each describe an entry of the leafnode with the specified
record no. The entries of the leafnode are tupleidentifiers (tid).
The stream is empty, if the given recordnumber is not existing/not valid
or the specified node is not a leafnode.

Signature is

----
   getLeafEntries: (rtree<D>) x int --> stream(tuple((NodeId int) (TupleID tid)))
                                                  (EntryMBR rect<D>)))

----

*/

/*
5.14.1 TypeMapping for Operator ~getLeafEntries~

The typemapping function is the one of the operator ~getNodeInfo~ and uses
a template parameter InfoType to differ between the typemapping of the
operators ~getNodeInfo~, ~getNodeSons~ and ~getLeafEntries~.
The typemapping of operator ~getLeafEntries~ uses InfoType = LEAFENTRIES.

*/


/*
5.14.2 Value Mapping for Operator ~getLeafEntries~

*/

template<unsigned dim>
int RTreeGetLeafEntriesVM( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  GetSonsInfo<dim, TupleId>* gsi;

  switch( message )
  {
    case OPEN:
    {
      int nodeId = ((CcInt*)args[1].addr)->GetIntval();
      R_Tree<dim, TupleId>* tree = (R_Tree<dim, TupleId>*) args[0].addr;
      if (checkRecordId(tree, nodeId))
      {
        int maxIntEntries = tree->MaxEntries(0);
        int maxLeafEntries = tree->MaxEntries(tree->Height());
        int maxEntr = maxIntEntries > maxLeafEntries ? maxIntEntries
            : maxLeafEntries;//needed to properly read the node.
        R_TreeNode<dim, TupleId>* f = new R_TreeNode<dim, TupleId>
                                           (false, 0, maxEntr);
        tree->GetNode(nodeId, *f);
        int m = f->EntryCount();
        bool isRootNode = (nodeId==static_cast<int>(tree->RootRecordId()));
        gsi = new GetSonsInfo<dim, TupleId>(0, m, f, new TupleType
                      (nl->Second(GetTupleResultType(s))), nodeId, isRootNode);
        local.setAddr(gsi);
      }
      else
        local.setAddr(0);
      return 0;
    }

    case REQUEST :
    {
      if(local.addr == NULL)
      {
        return CANCEL;
      }
      gsi = (GetSonsInfo<dim, TupleId>*)local.addr;

      if (gsi->index == gsi->maxEntries)
      {
        return CANCEL;
      }

      if (!(gsi->father->IsLeaf()))
      {
        return CANCEL;
      }

      Tuple *tuple = new Tuple( gsi->ttype );
      tuple->PutAttribute(0, new CcInt(true, gsi->nodeId));
      R_TreeLeafEntry<dim,TupleId>* entry = gsi->father->
                                GetLeafEntry(gsi->index);
      tuple->PutAttribute(1, new TupleIdentifier(true, entry->info));
      tuple->PutAttribute(2, new Rectangle<dim>(entry->box));
      result.setAddr(tuple);
      gsi->index++;
      return YIELD;
    }

    case CLOSE :
    {
      if(local.addr)
      {
        gsi = (GetSonsInfo<dim, TupleId>*)local.addr;
        delete gsi->father;
        gsi->ttype->DeleteIfAllowed();
        delete gsi;
        local.setAddr(Address(0));
      }
      return 0;
    }
  } // end switch
  cout << "RTreeGetLeafEntriesVM(): Received UNKNOWN message!" << endl;
  return 0;
}

/*
5.14.3 Specification for Operator ~getLeafEntries~

*/

const string RTreeGetLeafEntriesSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text>(rtree<D> (tuple ((x1 t1)...(xn tn))) ti false) x int "
    "-> stream(tuple((NodeId int)(TupleID tid)(EntryMBR rect<D>)))</text--->"
    "<text>getLeafEntries( _ , _ )</text--->"
    "<text>This operator allows introspection of an RTree. It creates a "
    "stream of tupleids, each describe an entry "
    "of the leafnode with the specified record no.</text--->"
    "<text>query getLeafEntries(strassen_geodata_rtree, 2);</text--->"
    "<text>Stream is empty, if given record no is not existing.</text--->"
    ") )";

/*
5.14.4 Selection Function for Operator ~getLeafEntries~

The selection function is the one of the operator ~getNodeInfo~.

*/

ValueMapping RTreeGetLeafEntries [] =
{ RTreeGetLeafEntriesVM<2>,  // 0
  RTreeGetLeafEntriesVM<3>,  // 1
  RTreeGetLeafEntriesVM<4>,  // 2
  RTreeGetLeafEntriesVM<8>   // 3
};


/*
5.14.5 Definition of Operator ~getLeafEntries~

*/
Operator rtreegetleafentries(
         "getLeafEntries",                    // name
         RTreeGetLeafEntriesSpec,             // specification
         4,
         RTreeGetLeafEntries,                 // value mapping
         RTreeGetInfoTypeSelect,              // selection function
         RTreeGetInfoTypeTypeMap<LEAFENTRIES> // type mapping
        );



/*

5.15 Operator ~cyclicbulkload~

5.15.1. Data structures

5.15.1.1 GridArea

The GridArea structure defines the boundary of a 2D-grid.

*/

struct GridArea
{
  GridArea(){}
  GridArea( double X1, double Y1, double X2, double Y2 ):
            x1( X1 ), y1( Y1 ), x2( X2 ), y2( Y2 ){}
  double x1;
  double y1;
  double x2;
  double y2;
};

/*

5.15.1.2 Unit

Unit structure constists of a TupleId and an UPoint.

*/

struct Unit
{
  TupleId tupId;
  UPoint  up;
};

/*

5.15.1.2 Cell

Every Cell stores a map container for Units.

*/

struct Cell
{
  GridArea area;
  multimap<double,Unit> units;
};

/*

5.15.1.3 Grid (2D-Cell-Array)

The grid is represented by a 2D-Cell-Array

*/

const int maxSplits = 64;
Cell* cells[maxSplits][maxSplits];

/*

5.15.2 Auxiliary methods

5.15.2.1 ModifyArea

Modifies the given area. x1/x2 and y1/y2 will be swapped if necessary.

*/

GridArea ModifyArea(GridArea AREA)
{
  double tempX = 0;
  double tempY = 0;

  if (AREA.x1 > AREA.x2)
  {
    tempX   = AREA.x2;
    AREA.x2 = AREA.x1;
    AREA.x1 = tempX;
  }
  if (AREA.y1 > AREA.y2)
  {
    tempY   = AREA.y2;
    AREA.y2 = AREA.y1;
    AREA.y1 = tempY;
  }
  AREA.x1 = AREA.x1 - 0.00001;
  AREA.y1 = AREA.y1 - 0.00001;
  AREA.x2 = AREA.x2 + 0.00001;
  AREA.y2 = AREA.y2 + 0.00001;
  return AREA;
}

/*

5.15.2.2 ComputeLine

Identifies a column or a row in a grid in dependence of a given position.

*/

int ComputeLine(double BORDER1 ,double BORDER2, int SPLITS, double POS)
{
  double len = abs(BORDER1-BORDER2) / SPLITS;
  int i = 0;

  while ((BORDER1 + (i*len)) <= POS) i++;

  return i-1;
}

/*

5.15.2.3 InsertUnits

Creates bounding boxes for the units and inserts them in Z-Order by bulk load
into the RTree.

*/

void InsertUnits (int SQUARES, int RIGHTCOL, int TOPROW,
                  R_Tree<3, TupleId>* RTREE)
{
  if (SQUARES > 4)
  {
    InsertUnits( (SQUARES/4), (RIGHTCOL-((int)sqrt(SQUARES)/2)),
                             (TOPROW-((int)sqrt(SQUARES)/2)), RTREE);
    InsertUnits( (SQUARES/4), (RIGHTCOL-((int)sqrt(SQUARES)/2)),TOPROW,RTREE);
    InsertUnits( (SQUARES/4), RIGHTCOL, (TOPROW-((int)sqrt(SQUARES)/2)),RTREE);
    InsertUnits( (SQUARES/4), RIGHTCOL, TOPROW, RTREE );
  }
  else
  {
    for (int i = 0; i < 4; i++)
    {
      int col = RIGHTCOL-1;
      int row = TOPROW-1;

      // Select cell
      if ( i == 0 ) { col = col-1; row = row-1; } // leftBottomCell
      if ( i == 1 ) { col = col-1;              } // leftTopCell
      if ( i == 2 ) {              row = row-1; } // rightBottomCell

      map<double,Unit>::iterator it;

      for ( it = cells[col][row]->units.begin();
            it != cells[col][row]->units.end(); )
      {
        // Get all UPoints from cell
        double x1 = it->second.up.p0.GetX();
        double x2 = it->second.up.p1.GetX();
        double y1 = it->second.up.p0.GetY();
        double y2 = it->second.up.p1.GetY();
        if (x1 > x2)
        {
          x1 = it->second.up.p1.GetX();
          x2 = it->second.up.p0.GetX();
        }
        if (y1 > y2)
        {
          y1 = it->second.up.p1.GetY();
          y2 = it->second.up.p0.GetY();
        }

        // Create bounding box
        const double tol = 0.00001;
        double start = it->second.up.timeInterval.start.ToDouble();
        double end   = it->second.up.timeInterval.end.ToDouble();
        if ( start < end )
        {
          Rectangle<3>* box = new Rectangle<3>(true, x1-tol, x2+tol,
                                                     y1-tol, y2+tol,
                                                     start, end );
          // Insert entry into RTree
          R_TreeLeafEntry<3, TupleId> e(*box, it->second.tupId);
          RTREE->InsertBulkLoad(e);
          delete box;
        }
        cells[col][row]->units.erase(it++);
      }
    }
  }
}

/*

5.15.3 TypeMapping for operator ~cyclicbulkload~

*/

bool isMPoint = false;

ListExpr CyclicBulkloadTM(ListExpr args)
{
  NList typeList(args);
  if ( !typeList.hasLength(5) )
  {
   return listutils::typeError("Expecting five arguments.");
  }

  // Check first argument
  ListExpr stream = nl->First(args);
  if(!listutils::isTupleStream(stream))
  {
     return listutils::typeError("First arg is not a tuple stream!");
  }

  // Check grid size
  if ( !nl->IsEqual(nl->Second(args), Rectangle<2>::BasicType()) )
  {
    return NList::typeError( "Rectangle for second argument expected!" );
  }

  // Check number of partitions
  if ( !nl->IsEqual(nl->Third(args), CcInt::BasicType()) )
  {
    return NList::typeError( "Integer for third argument expected!" );
  }

  // Check cycle time
  if ( !nl->IsEqual(nl->Fourth(args), CcInt::BasicType()) )
  {
    return NList::typeError( "Integer for fourth argument expected!" );
  }

  // Check for index attribute
  ListExpr fifth = nl->Fifth(args);
  if(nl->AtomType(fifth)!=SymbolType)
  {
    return listutils::typeError("Fifth argument is not a valid attr name!");
  }

  string attrName = nl->SymbolValue(fifth);
  ListExpr attrList = nl->Second(nl->Second(stream));
  ListExpr attrType;
  int attrIndex = listutils::findAttribute(attrList, attrName, attrType);
  if(attrIndex <= 0){
    return listutils::typeError("Expecting the attribute for fifth argument.");
  }

  if ( nl->SymbolValue(attrType) != MPoint::BasicType() &&
       nl->SymbolValue(attrType) != UPoint::BasicType() )
  {
    return NList::typeError("Attribute type is not of type mpoint or upoint.");
  }

  if ( nl->SymbolValue(attrType) == MPoint::BasicType() ) isMPoint = true;
  else isMPoint = false;

  ListExpr first, rest, newAttrList, lastNewAttrList;
  int tidIndex = 0;
  string type;
  bool firstcall = true;

  rest = attrList;
  int j = 1;
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);

    type = nl->SymbolValue(nl->Second(first));
    if (type == TupleIdentifier::BasicType())
    {
      if( tidIndex != 0 ){
        return listutils::typeError("Expecting exactly one attribute of type "
                                    "'tid' in the 1st argument.");
      }
      tidIndex = j;
    }
    else
    {
      if (firstcall)
      {
        firstcall = false;
        newAttrList = nl->OneElemList(first);
        lastNewAttrList = newAttrList;
      }
      else
      {
        lastNewAttrList = nl->Append(lastNewAttrList, first);
      }
    }
    j++;
  }
  if( tidIndex == 0 )
  {
    return listutils::typeError("Expecting exactly one attribute of type "
                                "'tid' in the 1st argument.");
  }

  return
    nl->ThreeElemList(
      nl->SymbolAtom(Symbol::APPEND()),
  nl->TwoElemList(
      nl->IntAtom(attrIndex),
  nl->IntAtom(tidIndex)),
  nl->FourElemList(
      nl->SymbolAtom(RTree3TID::BasicType()),
  nl->TwoElemList(
      nl->SymbolAtom(Tuple::BasicType()),
  newAttrList),
  attrType,
  nl->BoolAtom(false)));
}

/*

5.15.4 ValueMapping for operator ~cyclicbulkload~

*/

int CyclicBulkloadVM(Word* args, Word& result, int message,
                                    Word& local, Supplier s)
{
  Rectangle<2>* rect  = static_cast<Rectangle<2>*>(args[1].addr);
  int       numCells  = static_cast<CcInt*>(args[2].addr)->GetValue();
  int       cycleTime = static_cast<CcInt*>(args[3].addr)->GetValue();
  int       entries = 0;

  // Create new RTree
  R_Tree<3, TupleId>* rtree = (R_Tree<3, TupleId>*)qp->ResultStorage(s).addr;
  result.setAddr( rtree );

  // Initialize RTree for bulk load
  int BulkLoadInitialized = rtree->InitializeBulkLoad();
  assert(BulkLoadInitialized);

  // Create grid area
  double x1(rect->MinD(0));
  double x2(rect->MaxD(0));
  double y1(rect->MinD(1));
  double y2(rect->MaxD(1));
  GridArea area(x1,y1,x2,y2);
  area = ModifyArea(area);

  // Check number of cells
  if ( !( numCells == 16   ||
          numCells == 64   ||
          numCells == 256  ||
          numCells == 4096 )) numCells = 4;
  const int splits = (int)sqrt(numCells);

  // Check cycle time
  if ( cycleTime < 1000 ) cycleTime = 1000;

  // Calculate x/y cell length
  double areaLenX = abs(area.x2 - area.x1);
  double areaLenY = abs(area.y2 - area.y1);
  double cellLenX = areaLenX / splits;
  double cellLenY = areaLenY / splits;

  GridArea partition(0,0,0,0);
  for (int i = 0; i < splits; i++)
  {
    for (int j = 0; j < splits; j++)
    {
      partition.x1 = area.x1 + (cellLenX * j);
      partition.x2 = partition.x1 + cellLenX;
      partition.y1 = area.y1 + (cellLenY * i);
      partition.y2 = partition.y1 + cellLenY;
      cells[j][i] = new Cell();
      cells[j][i]->area = partition;
    }
  }

  // Create an instant object for systemTime
  Instant* systemTime = new DateTime(0,0,instanttype);
  systemTime->Now();
  int32_t startTime = systemTime->GetAllMilliSeconds();

  Word wTuple;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wTuple);

  while (qp->Received(args[0].addr))
  {
    Tuple*  tuple = (Tuple*)wTuple.addr;
    int attrIndex = ((CcInt*)args[5].addr)->GetIntval() - 1;
    int tidIndex = ((CcInt*)args[6].addr)->GetIntval() - 1;

    systemTime->Now();
    if ( systemTime->GetAllMilliSeconds() > (startTime + cycleTime))
    {
      // Insert units into RTree (Z-Order)
      InsertUnits(numCells, splits, splits, rtree);

      // Start time for the next slice
      startTime += cycleTime;

      // Clear grid
      for (int i = 0; i < splits; i++)
      {
        for (int j = 0; j < splits; j++)
        {
          cells[j][i]->units.clear();
        }
      }
    }

    if ( isMPoint )
    {
      // Attribute type is mpoint
      MPoint* mp = static_cast<MPoint*>(tuple->GetAttribute(attrIndex));
      int i = 0;
      while ( i < mp->GetNoComponents() )
      {

        Unit u;
        u.tupId = ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->GetTid();
        mp->Get(i, u.up);

        // Insert upoint into cell, sorted by start time
        systemTime->Now();
        if ( u.up.timeInterval.end < *systemTime &&
             u.up.p0.GetX() > area.x1 && u.up.p0.GetX() < area.x2 &&
             u.up.p1.GetX() > area.x1 && u.up.p1.GetX() < area.x2 &&
             u.up.p0.GetY() > area.y1 && u.up.p0.GetY() < area.y2 &&
             u.up.p1.GetY() > area.y1 && u.up.p1.GetY() < area.y2 )
        {
          int col = ComputeLine(area.x1, area.x2, splits, u.up.p0.GetX());
          int row = ComputeLine(area.y1, area.y2, splits, u.up.p0.GetY());

          cells[col][row]->
               units.insert(make_pair(u.up.timeInterval.start.ToDouble(),u));
        }
        i++;
        entries++;
      }
    }
    else
    {
      // Attribute type is upoint
      UPoint* up = static_cast<UPoint*>(tuple->GetAttribute(attrIndex));
      Unit u;
      u.tupId = ((TupleIdentifier *)tuple->GetAttribute(tidIndex))->GetTid();
      u.up = *up;

      // Insert upoint into cell, sorted by start time
      systemTime->Now();
      if ( u.up.timeInterval.end < *systemTime &&
           u.up.p0.GetX() > area.x1 && u.up.p0.GetX() < area.x2 &&
           u.up.p1.GetX() > area.x1 && u.up.p1.GetX() < area.x2 &&
           u.up.p0.GetY() > area.y1 && u.up.p0.GetY() < area.y2 &&
           u.up.p1.GetY() > area.y1 && u.up.p1.GetY() < area.y2 )
      {
        int col = ComputeLine(area.x1, area.x2, splits, u.up.p0.GetX());
        int row = ComputeLine(area.y1, area.y2, splits, u.up.p0.GetY());

        cells[col][row]->
             units.insert(make_pair(u.up.timeInterval.start.ToDouble(),u));
        entries++;
      }
    }
    delete tuple;
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);
  // Insert the last entries into the RTree
  InsertUnits(numCells, splits, splits, rtree);

  // Insert dummy if the rtree is empty
  if (entries == 0)
  {
     Rectangle<3>* box = new Rectangle<3>(true,0.0,1.0,0.0,1.0,0.0,1.0);
     R_TreeLeafEntry<3, TupleId> e(*box,0);
     rtree->InsertBulkLoad(e);
  }

  // Finalize bulk load
  int FinalizedBulkLoad = rtree->FinalizeBulkLoad();
  assert(FinalizedBulkLoad);
  return 0;
}

/*

5.15.5 Specification for operator ~cyclicbulkload~

*/

struct CyclicBulkloadInfo : OperatorInfo {
  CyclicBulkloadInfo()
  {
    name      = "cyclicbulkload";
    signature = "((stream (tuple([a1:d1, ..., {aj:mpoint, aj:upoint}, ...,"
                " an:dn, id:tid]))) x rect x int int x aj) -> rtree3";
    syntax    = "_ cyclicbulkload [ _, _, _, _ ]";
    meaning   = "Cyclic bulkload method for moving points.";
  }
};


/*
5.16 Operator ~SpatialJoin~

This operator gets two rtrees and returns such pairs of tuple id's with 
intersecting bounding boxes.


5.16.1 Type Mapping

Signature is rtree [x] rtree -> stream(tuple([TID1 : tid, TID2 : tid]))

*/

ListExpr dspatialJoinTM(ListExpr args){

  string err = "rtree x rel x rtree x rel [ x renaming] expected";

  int len = nl->ListLength(args);
  if(len!=4 && len!=5){
    return listutils::typeError(err);
  } 


  // check r-trees
  ListExpr tree1 = nl->First(args);
  ListExpr tree2 = nl->Third(args);
  if(!listutils::isRTreeDescription(tree1) || 
     !listutils::isRTreeDescription(tree2)){
     return listutils::typeError(err);
  }
  if(!nl->Equal(nl->First(tree1) , nl->First(tree2))){
    return listutils::typeError("Rtree with different dimensions found");
  }
  if(!nl->Equal(nl->Fourth(tree1) , nl->Fourth(tree2))){
    return listutils::typeError("One of the rtrees supports double indexing");
  }
  if(nl->BoolValue(nl->Fourth(tree1))){
    return listutils::typeError("double indexing is not allowed here");
  }

  // check relations
  ListExpr rel1 = nl->Second(args);
  ListExpr rel2 = nl->Fourth(args);

  if(!Relation::checkType(rel1) || !Relation::checkType(rel2)){
    return listutils::typeError("One of the rtrees supports double indexing");
  }
  ListExpr al1 = nl->Second(nl->Second(rel1));
  ListExpr al2 = nl->Second(nl->Second(rel2));

  ListExpr attrList;
  if(len==4){
     attrList = listutils::concat(al1,al2);
  } else {
    ListExpr ren = nl->Fifth(args);
    if(!listutils::isSymbol(ren)){
        return listutils::typeError(err);
    }
    string renaming = "_" + nl->SymbolValue(ren);
    ListExpr ral2;
    ListExpr last;
    bool first = true;
    while(!nl->IsEmpty(al2)){
      ListExpr f = nl->First(al2);
      al2 = nl->Rest(al2);
      ListExpr nf = nl->TwoElemList( 
                         nl->SymbolAtom( 
                             nl->SymbolValue(nl->First(f)) +renaming),
                         nl->Second(f));
      if(first){
        ral2 = nl->OneElemList(nf);
        last = ral2;
        first = false;
      } else { 
         last = nl->Append(last,nf);
      }
    }
    attrList = listutils::concat(al1,ral2); 
  }
  if(!listutils::isAttrList(attrList)){
    return listutils::typeError("Name conflicts in resulting tuple");
  }
  
  return nl->TwoElemList( nl->SymbolAtom(Stream<Tuple>::BasicType()),
             nl->TwoElemList( nl->SymbolAtom(Tuple::BasicType()),
                              attrList));
}





/*

LocalInfo class for directSpatialJoin

*/
template<int dim>
class DspatialJoinLocal{
  public:
     DspatialJoinLocal(R_Tree<dim, TupleId>* _r1,
                       GenericRelation* _rel1,
                       R_Tree<dim, TupleId>* _r2,
                       GenericRelation* _rel2,
                       ListExpr ttl, size_t maxMem): r1(_r1), r2(_r2), 
                       rel1(_rel1), rel2(_rel2),
                       tt(0), 
                       q_ii(), q_il(), q_li(),  q_ll(){


        tt = new TupleType(ttl);
        R_TreeNode<dim, TupleId> n1 = r1->Root();
        R_TreeNode<dim, TupleId> n2 = r2->Root();
        processNodes(n1,n2);
        max1 = max(r1->MaxLeafEntries(), r1->MaxInternalEntries());
        max2 = max(r2->MaxLeafEntries(), r2->MaxInternalEntries());

        // compute size for storing the stacks
        size_t s_q_ii = max1*max2 * min(r1->Height(), r2->Height());
        size_t s_q_li = max1*max2 * abs(r1->Height() - r2->Height()); 
        // s_q_li covers also s_q_il 
        size_t s_q_ll = max1*max2;
        size_t sizes = (s_q_ii + s_q_li) * sizeof(R_TreeInternalEntry<dim>) +
                       s_q_ll * sizeof(pair<TupleId,TupleId>);

        maxMem = maxMem - sizes;
        if(maxMem < 0){
          maxMem = 0;
        }

        if((rel1->GetNoTuples()>0) && (rel2->GetNoTuples()>0)){

          // compute amount of tuples to be stored within caches
          // following the rules:
          // 1. tupleNumber1 * tupleSize1 + tupleNumber2 * TupleSize2 = maxMem
          // 2. tupleNumber1 / tupleNumber2 = tupleCount1 / tupleCount2
          // cond. 1 ensures that the memory is sufficient to store the tuples
          // cond. 2 distributes the available memory according to the number 
          // of tuples within the relations
       
          int tupleSize1 = (int) floor(rel1->GetTotalExtSize() / 
                                       rel1->GetNoTuples());
          int tupleSize2 = (int) floor(rel2->GetTotalExtSize() / 
                                       rel2->GetNoTuples());
          int tupleCount1 = rel1->GetNoTuples();
          int tupleCount2 = rel2->GetNoTuples();
          size_t tmp = tupleSize1 + (tupleSize2*tupleCount2)/tupleCount1;
          size_t noTuples1 = maxMem / tmp;
          size_t noTuples2 = (tupleCount2*noTuples1)/tupleCount1;


           // force caches of minimum size 5
          if(noTuples1 < 5){
             noTuples1 = 5;
          }
          if(noTuples2 < 5){
             noTuples2 = 5;
          }
          lru1 = new LRU<TupleId, Tuple*>(noTuples1);
          lru2 = new LRU<TupleId, Tuple*>(noTuples2);
       } else { 
          // at least one of the relations is empty
          // avoid crash if using rtree not fit to the relations
          // so, just create empty caches
          lru1 = new LRU<TupleId,Tuple*>(3);
          lru2 = new LRU<TupleId, Tuple*>(3);
       }
     }   

     ~DspatialJoinLocal(){
        tt->DeleteIfAllowed();

         // cache statistics 
        // cout << "Cache1 : "  << endl;
        // lru1->printStats(cout);
        // cout << endl << "Cache 2 : " << endl;
        // lru2->printStats(cout);
        // cout << endl;


        // remove elements from LRU
        while(!lru1->empty()){
          LRUEntry<TupleId,Tuple*>* victim = lru1->deleteLast();
          victim->value->DeleteIfAllowed();
          delete victim;
        }
        while(!lru2->empty()){
          LRUEntry<TupleId,Tuple*>* victim = lru2->deleteLast();
          victim->value->DeleteIfAllowed();
          delete victim;
        }
        delete lru1;
        delete lru2;
     }

     Tuple* nextTuple(){
       Tuple* res =  getRes();
       if(res != 0) {
          return res;
       }

       while(!allempty()){ 
         if(!processIL()){
            if(!processLI()){
               processII();
            }
         }
         res = getRes();
         if(res){
           return res;
         }
       }
       return 0;
     }


  private:
    R_Tree<dim,TupleId>* r1;
    R_Tree<dim,TupleId>* r2;
    GenericRelation* rel1;
    GenericRelation* rel2;
    TupleType* tt;
    std::stack< pair<R_TreeInternalEntry<dim>, 
                     R_TreeInternalEntry<dim> > > q_ii;
    std::stack< pair<R_TreeInternalEntry<dim>, 
                     R_TreeLeafEntry<dim, TupleId> > > q_il;
    std::stack< pair<R_TreeLeafEntry<dim, TupleId>, 
                     R_TreeInternalEntry<dim> > > q_li;
    std::queue< pair<TupleId, TupleId > > q_ll;
    int max1;
    int max2;
    LRU<TupleId, Tuple*>* lru1;
    LRU<TupleId, Tuple*>* lru2;


    bool allempty() const{
      return q_ll.empty() && q_li.empty() && q_il.empty() && q_ii.empty();
    }

    void processNodes(const R_TreeNode<dim, TupleId> & n1,
                      const R_TreeNode<dim, TupleId> & n2){

        bool l1 = n1.IsLeaf();
        bool l2 = n2.IsLeaf();
 
        if(l1 && l2){
           processLeafLeaf(n1,n2);
        } else if (l1 && !l1){
           processLeafInner(n1,n2);
        } else if (!l1 && l2){
           processInnerLeaf(n1,n2);
        } else {
           processInnerInner(n1,n2);
        }
    }

    void printStats(){
       cout << " q_ii : "  << q_ii.size() << " elements" << endl;
       cout << " q_il : "  << q_il.size() << " elements" << endl;
       cout << " q_li : "  << q_li.size() << " elements" << endl;
       cout << " q_ll : "  << q_ll.size() << " elements" << endl;
    }

  
/*
Converts an element in q[_]ll into a result tuple.

*/    
   Tuple* getRes(){
      while(!q_ll.empty()){
        pair<TupleId, TupleId> p = q_ll.front();
        q_ll.pop();
        Tuple* t1 = getTuple1(p.first);
        if(t1){
           Tuple* t2 = getTuple2(p.second);
           if(t2){
              Tuple* res = new Tuple(tt);
              Concat(t1,t2,res);
              return res;
           } 
           t1->DeleteIfAllowed();
        }
      } 
      return 0;
    }


    Tuple* getTuple1(const TupleId id){
        Tuple** t = lru1->get(id);
        if(t){
          return *t;
        } 
        Tuple* t1 = rel1->GetTuple(id,true);
        if(!t1){
          return 0;
        }
        // insert into lru
        LRUEntry<TupleId,Tuple*>* victim = lru1->use(id,t1);
        if(victim){
          victim->value->DeleteIfAllowed();
          delete victim;
        }
        return t1;
    }

    Tuple* getTuple2(const TupleId id){
        Tuple** t = lru2->get(id);
        if(t){
          return *t;
        } 
        Tuple* t1 = rel2->GetTuple(id,true);
        if(!t1){
          return 0;
        }
        // insert into lru
        LRUEntry<TupleId,Tuple*>* victim = lru2->use(id,t1);
        if(victim){
          victim->value->DeleteIfAllowed();
          delete victim;
        }
        return t1;
    }

   
/*
Processes two leaf nodes

*/    
   void processLeafLeaf( const R_TreeNode<dim, TupleId> & n1,
                         const  R_TreeNode<dim, TupleId> & n2){

       for(int i1=0; i1 < n1.EntryCount(); i1++){
          R_TreeLeafEntry<dim,TupleId>* e1 = n1.GetLeafEntry(i1);
          for(int i2=0;i2 < n2.EntryCount(); i2++){
             R_TreeLeafEntry<dim,TupleId>* e2 = n2.GetLeafEntry(i2);
             if(e1->box.Intersects(e2->box)){
                 pair<TupleId,TupleId> p(e1->info, e2->info);
                 q_ll.push(p);
             }
          }
       }
       
   }
   void processLeafInner( const R_TreeNode<dim, TupleId> & n1,
                         const  R_TreeNode<dim, TupleId> & n2){
       for(int i1=0; i1 < n1.EntryCount(); i1++){
          R_TreeLeafEntry<dim,TupleId> e1 = *n1.GetLeafEntry(i1);
          for(int i2=0;i2 < n2.EntryCount(); i2++){
             R_TreeInternalEntry<dim> e2 = *n2.GetInternalEntry(i2);
             if(e1.box.Intersects(e2.box)){
                 pair< R_TreeLeafEntry<dim,TupleId>, 
                       R_TreeInternalEntry<dim> > p(e1,e2);
                 q_li.push(p);
             }
          }
       }
   }
   void processInnerLeaf( const R_TreeNode<dim, TupleId> & n1,
                          const  R_TreeNode<dim, TupleId> & n2){
       for(int i1=0; i1 < n1.EntryCount(); i1++){
          R_TreeInternalEntry<dim> e1 = *n1.GetInternalEntry(i1);
          for(int i2=0;i2 < n2.EntryCount(); i2++){
             R_TreeLeafEntry<dim, TupleId> e2 = *n2.GetLeafEntry(i2);
             if(e1.box.Intersects(e2.box)){
                 pair<R_TreeInternalEntry<dim>,
                      R_TreeLeafEntry<dim, TupleId> > p(e1,e2);
                 q_il.push(p);
             }
          }
       }
   }
   void processInnerInner( const R_TreeNode<dim, TupleId> & n1,
                         const  R_TreeNode<dim, TupleId> & n2){
       for(int i1=0; i1 < n1.EntryCount(); i1++){
          R_TreeInternalEntry<dim> e1 = *n1.GetInternalEntry(i1);
          for(int i2=0;i2 < n2.EntryCount(); i2++){
             R_TreeInternalEntry<dim> e2 = *n2.GetInternalEntry(i2);
             if(e1.box.Intersects(e2.box)){
                 pair<R_TreeInternalEntry<dim>,
                      R_TreeInternalEntry<dim> > p(e1,e2);
                 q_ii.push(p);
             }
          }
       }
   }

   bool processIL(){
      if(q_il.empty()){
         return false;
      } 
      pair<R_TreeInternalEntry<dim>,
           R_TreeLeafEntry<dim, TupleId> > p = q_il.top();
      q_il.pop();
      R_TreeNode<dim,TupleId> n1(true,1,max1);
      r1->GetNode(p.first.pointer,n1);
      if(n1.IsLeaf()){
         for(int i1=0;i1<n1.EntryCount();i1++){
           R_TreeLeafEntry<dim,TupleId> e1 = *n1.GetLeafEntry(i1);
           if(p.second.box.Intersects(e1.box)){
               pair<TupleId,TupleId> pn(e1.info, p.second.info);
               q_ll.push(pn);
           }     
         }
      } else {
         for(int i1=0;i1<n1.EntryCount();i1++){
           R_TreeInternalEntry<dim> e1 = *n1.GetInternalEntry(i1);
           if(p.second.box.Intersects(e1.box)){
               pair<R_TreeInternalEntry<dim>, 
                    R_TreeLeafEntry<dim,TupleId> > pn(e1, p.second);
               q_il.push(pn);
           }     
         }
      }
      return true;
      
   }
   bool processLI(){
      if(q_li.empty()){
         return false;
      } 
      pair<R_TreeLeafEntry<dim, TupleId>, 
           R_TreeInternalEntry<dim> > p = q_li.top();
      q_li.pop();
      R_TreeNode<dim,TupleId> n2(true,1,max2);
      r2->GetNode(p.second.pointer,n2);

      if(n2.IsLeaf()){
         for(int i2=0;i2<n2.EntryCount();i2++){
           R_TreeLeafEntry<dim,TupleId> e2 = *n2.GetLeafEntry(i2);
           if(p.first.box.Intersects(e2.box)){
               pair<TupleId,TupleId> pn(p.first.info, e2.info);
               q_ll.push(pn);
           }     
         }
      } else {
         for(int i2=0;i2<n2.EntryCount();i2++){
           R_TreeInternalEntry<dim> e2 = *n2.GetInternalEntry(i2);
           if(p.first.box.Intersects(e2.box)){
               pair<R_TreeLeafEntry<dim, TupleId>, 
                    R_TreeInternalEntry<dim> > pn(p.first, e2);
               q_li.push(pn);
           }     
         }
      }
      return true;


   }


   bool processII(){
      if(q_ii.empty()){
        return false;
      } 
      pair<R_TreeInternalEntry<dim>, R_TreeInternalEntry<dim> > p = q_ii.top();
      q_ii.pop();
      R_TreeNode<dim,TupleId> n1(true,1,max1);
      R_TreeNode<dim,TupleId> n2(true,1,max2);
      r1->GetNode(p.first.pointer, n1);
      r2->GetNode(p.second.pointer, n2);
      processNodes(n1,n2);
      return true;
   }
  
 


};




template<int dim>
int dspatialJoinVM1(Word* args, Word& result, int message,
                    Word& local, Supplier s){

   DspatialJoinLocal<dim>* li = (DspatialJoinLocal<dim>*) local.addr;
   switch(message){
      case OPEN: {
                if(li){
                  delete li;
                }
                ListExpr ttl = nl->Second(GetTupleResultType(s));
                R_Tree<dim,TupleId>* r1 = 
                             (R_Tree<dim,TupleId>*) args[0].addr; 
                GenericRelation* rel1 = (GenericRelation*) args[1].addr;
                R_Tree<dim,TupleId>* r2 = 
                             (R_Tree<dim,TupleId>*) args[2].addr; 
                GenericRelation* rel2 = (GenericRelation*) args[3].addr;
                int maxMem = qp->GetMemorySize(s) * 1024 * 1024;
                local.addr = new DspatialJoinLocal<dim>(r1,rel1,
                                                        r2,rel2,
                                                        ttl, maxMem);
                return 0;
                 }
      case REQUEST: {
                if(!li){
                   result.addr=0;
                   return CANCEL;
                }
                result.addr = li->nextTuple();
                return result.addr?YIELD:CANCEL;
      }
      case CLOSE: {
                    if(li){
                      delete li;
                      local.addr =0;
                    }
                    return 0;
                 }
   }
   return -1;
}


/*
5.16.3 ValueMapping array 

*/

ValueMapping dspatialJoinVM[] = {
               dspatialJoinVM1<2>,
               dspatialJoinVM1<3>,
               dspatialJoinVM1<4>,
               dspatialJoinVM1<8>
             };

/*
5.16.4 Selection Function

*/
int dspatialJoinSelect(ListExpr args){
   int d = listutils::getRTreeDim(nl->First(args));
   switch(d){
      case 2 : return 0;
      case 3 : return 1;
      case 4 : return 2;
      case 8 : return 3;
      default : return -1;
   }  
}

/*
5.16..5 Operator Spec

*/

const string dspatialJoinSpec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" \"Comment\" ) "
    "(<text> rtree x rtree -> stream(tuple([TID1 : tid, TID2 : tid]))" 
    "</text--->"
    "<text> _ _ dspatialJoin </text--->"
    "<text>This operator returns such pairs of tuple id's "
    "having overlapping bounding boxes within the rtrees"
    ".</text--->"
    "<text>query strassen_geoData_rtree "
    "strassen_geoData_rtree dspatialJoin count</text--->"
    "<text></text--->"
    ") )";

/*
5.16.6 Operator instance

*/

Operator dspatialJoin(
         "dspatialJoin",                    // name
         dspatialJoinSpec,             // specification
         4,
         dspatialJoinVM,                 // value mapping
         dspatialJoinSelect,              // selection function
         dspatialJoinTM // type mapping
        );



/*
6 Definition and initialization of RTree Algebra

*/
class RTreeAlgebra : public Algebra
{
 public:
  RTreeAlgebra() : Algebra()
  {
    AddTypeConstructor( &rtree );
    AddTypeConstructor( &rtree3 );
    AddTypeConstructor( &rtree4 );
    AddTypeConstructor( &rtree8 );

    AddOperator( &creatertree );
    AddOperator( &bulkloadrtree );
    AddOperator( &windowintersects );
    AddOperator( &windowintersectsS );
    AddOperator( &gettuples );
    AddOperator( &gettuplesdbl );
    AddOperator( &gettuples2 );
    AddOperator( &rtreenodes );
    AddOperator( &rtreetreeheight );
    AddOperator( &rtreenoofnodes );
    AddOperator( &rtreenoofentries );
    AddOperator( &rtreebbox );
    AddOperator( &rtreeentries);
    AddOperator( &getfileinfortree);
    AddOperator( &updatebulkloadrtree);
    AddOperator( &rtreegetrootnode );
    AddOperator( &rtreegetnodeinfo );
    AddOperator( &rtreegetnodesons );
    AddOperator( &rtreegetleafentries );
    AddOperator( CyclicBulkloadInfo(), CyclicBulkloadVM, CyclicBulkloadTM );
    AddOperator(&dspatialJoin);
     dspatialJoin.SetUsesMemory();

#ifdef USE_PROGRESS
    windowintersects.EnableProgress();
    windowintersectsS.EnableProgress();
    gettuples.EnableProgress();
    gettuples2.EnableProgress();
#endif

  }
  ~RTreeAlgebra() {};
};



extern "C"
Algebra*
InitializeRTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RTreeAlgebra);
}

