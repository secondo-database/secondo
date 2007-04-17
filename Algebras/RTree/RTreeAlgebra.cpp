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

[1] Implementation of R-Tree Algebra

July 2003, Victor Almeida

October 2004, Herbert Schoenhammer, tested and divided in Header-File
and Implementation-File. Also R-Trees with three and four dimensions
are created.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

February 2007, Christian Duentgen added operator for bulk loading
R-trees.


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

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"
#include "RTreeAlgebra.h"
#include "CPUTimeMeasurer.h"
#include "TupleIdentifier.h"
#include "Messages.h"

extern NestedList* nl;
extern QueryProcessor* qp;

#define BBox Rectangle

#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };

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

*/
bool CheckRTree2(ListExpr type, ListExpr& errorInfo)
{
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 4)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo) &&
      (nl->IsEqual(nl->Third(type), "rect") ||
      algMgr->CheckKind("SPATIAL2D", nl->Third(type), errorInfo)) &&
      nl->IsAtom(nl->Fourth(type)) &&
      nl->AtomType(nl->Fourth(type)) == BoolType;
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60),
        nl->SymbolAtom("RTREE2"),
        type));
    return false;
  }
  return true;
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree( "rtree",
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
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree3")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo) &&
      algMgr->CheckKind("SPATIAL3D", nl->Third(type), errorInfo);
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60),
        nl->SymbolAtom("RTREE3"),
        type));
    return false;
  }
  return true;
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree3( "rtree3",
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
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree4")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      &&nl->Equal(nl->Third(type), nl->SymbolAtom("rect4"));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60),
        nl->SymbolAtom("RTREE4"),
        type));
    return false;
  }
  return true;
}

/*
3.12 Type Constructor object for type constructor ~rtree~

*/
TypeConstructor rtree4( "rtree4",
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
  AlgebraManager* algMgr;

  if((!nl->IsAtom(type))
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree8")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      &&nl->Equal(nl->Third(type), nl->SymbolAtom("rect8"));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(
        nl->IntAtom(60),
        nl->SymbolAtom("RTREE8"),
        type));
    return false;
  }
  return true;
}

/*
3.12 Type Constructor object for type constructor ~rtree8~

*/
TypeConstructor rtree8( "rtree8",
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
  string attrName, relDescriptionStr, argstr;
  string errmsg = "Incorrect input for operator creatertree.";
  int attrIndex;
  ListExpr attrType;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  CHECK_COND(!nl->IsEmpty(args) &&
    nl->ListLength(args) == 2,
    errmsg + "\nOperator creatertree expects two arguments.");

  ListExpr relDescription = nl->First(args),
           attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE) &&
    nl->AtomType(attrNameLE) == SymbolType,
    errmsg + "\nThe second argument must be the name of "
    "the attribute to index.");
  attrName = nl->SymbolValue(attrNameLE);

  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(!nl->IsEmpty(relDescription) &&
    nl->ListLength(relDescription) == 2,
    errmsg +
    "\nOperator creatertree expects a first argument with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) or "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND((nl->IsEqual(nl->First(relDescription), "rel") ||
    nl->IsEqual(nl->First(relDescription), "stream")) &&
    nl->ListLength(tupleDescription) == 2,
    errmsg +
    "\nOperator creatertree expects a first argument with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) or "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr attrList = nl->Second(tupleDescription);
  CHECK_COND(nl->IsEqual(nl->First(tupleDescription), "tuple") &&
    IsTupleDescription(attrList),
    errmsg +
    "\nOperator creatertree expects a first argument with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) or "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "but gets it with structure '" + relDescriptionStr + "'.");

  CHECK_COND(
    (attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
    errmsg +
    "\nOperator creatertree expects that the attribute " +
    attrName + "\npassed as second argument to be part of "
    "the relation or stream description\n'" +
    relDescriptionStr + "'.");

  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)||
    algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)||
    algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)||
    algMgr->CheckKind("SPATIAL8D", attrType, errorInfo)||
    nl->IsEqual(attrType, "rect")||
    nl->IsEqual(attrType, "rect3")||
    nl->IsEqual(attrType, "rect4")||
    nl->IsEqual(attrType, "rect8"),
    errmsg +
    "\nOperator creatertree expects that attribute "+attrName+"\n"
    "belongs to kinds SPATIAL2D, SPATIAL3D, Spatial4D or SPATIAL8D\n"
    "or rect, rect3, rect4 and rect8.");

  string rtreetype;

  if ( algMgr->CheckKind("SPATIAL2D", attrType, errorInfo) ||
       nl->IsEqual( attrType, "rect" ) )
    rtreetype = "rtree";
  else if ( algMgr->CheckKind("SPATIAL3D", attrType, errorInfo) ||
            nl->IsEqual( attrType, "rect3" ) )
    rtreetype = "rtree3";
  else if ( algMgr->CheckKind("SPATIAL4D", attrType, errorInfo) ||
       nl->IsEqual( attrType, "rect4" ) )
    rtreetype = "rtree4";
  else if ( algMgr->CheckKind("SPATIAL8D", attrType, errorInfo) ||
       nl->IsEqual( attrType, "rect8" ) )
    rtreetype = "rtree8";
  if( nl->IsEqual(nl->First(relDescription), "rel") )
  {
    return
      nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->OneElemList(
          nl->IntAtom(attrIndex)),
        nl->FourElemList(
          nl->SymbolAtom(rtreetype),
          tupleDescription,
          attrType,
          nl->BoolAtom(false)));
  }
  else // nl->IsEqual(nl->First(relDescription), "stream")
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
      if (type == "tid")
      {
        CHECK_COND( tidIndex == 0,
          "Operator creatertree expects as first argument a stream "
          "with\none and only one attribute of type 'tid'\n'"
          "but gets\n'" + relDescriptionStr + "'.");

        tidIndex = j;
      }
      else if( j == nAttrs - 1 && type == "int" &&
               nl->SymbolValue(
                 nl->Second(nl->First(rest))) == "int" )
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
    CHECK_COND( tidIndex != 0,
      "Operator creatertree expects as first argument a stream "
      "with\none and only one attribute of type 'tid'\n'"
      "but gets\n'" + relDescriptionStr + "'.");

    return
      nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
        nl->TwoElemList(
          nl->IntAtom(attrIndex),
          nl->IntAtom(tidIndex)),
        nl->FourElemList(
          nl->SymbolAtom(rtreetype),
          nl->TwoElemList(
            nl->SymbolAtom("tuple"),
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
  if ( algMgr->CheckKind("SPATIAL2D", attrType, errorInfo) )
    result = 0;
  else if ( algMgr->CheckKind("SPATIAL3D", attrType, errorInfo) )
    result = 1;
  else if ( algMgr->CheckKind("SPATIAL4D", attrType, errorInfo) )
    result = 2;
  else if ( algMgr->CheckKind("SPATIAL8D", attrType, errorInfo) )
    result = 3;
  else if( nl->SymbolValue(attrType) == "rect" )
    result = 4;
  else if( nl->SymbolValue(attrType) == "rect3" )
    result = 5;
  else if( nl->SymbolValue(attrType) == "rect4" )
    result = 6;
  else if( nl->SymbolValue(attrType) == "rect8" )
    result = 7;
  else
    return -1; /* should not happen */

  if( nl->SymbolValue(nl->First(relDescription)) == "rel")
    return result;
  if( nl->SymbolValue(nl->First(relDescription)) == "stream")
  {
    ListExpr first,
             rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
    if( nl->IsEqual( nl->Second( first ), "int" ) )
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
  RelationIterator* iter;
  Tuple* tuple;

  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

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
  result = SetWord( rtree );

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
  RelationIterator* iter;
  Tuple* tuple;

  R_Tree<dim, TupleId> *rtree =
    (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );
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
  result = SetWord( rtree );

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
  result = SetWord( rtree );

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
  result = SetWord( rtree );

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
  "( ( \"1st Signature\" \"2nd Signature\" "
  "\"3rd Signature\" \"Syntax\" \"Meaning\" "
  "\"1st Example\" \"2nd Example\" "
  "\"3rd Example\" ) "
  "( <text>((rel (tuple (x1 t1)...(xn tn)))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)</text--->"
  "<text>((stream (tuple (x1 t1)...(xn tn) (id tid))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti false)</text--->"
  "<text>((stream (tuple (x1 t1)...(xn tn) "
  "(id tid)(low int)(high int))) xi)"
  " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti true)</text--->"
  "<text>_ creatertree [ _ ]</text--->"
  "<text>Creates an rtree<d>. The key type ti must "
  "be of kind SPATIAL2D, SPATIAL3D, SPATIAL4D or Spatial8D, "
  "or of type rect, rect2, rect3, rect4 or rect8.</text--->"
  "<text>let myrtree = Kreis creatertree [Gebiet]</text--->"
  "<text>let myrtree = Kreis feed extend[id: tupleid(.)] "
  "creatertree[Gebiet]</text--->"
  "<text>let myrtree = Kreis feed extend[id: tupleid(.)] "
  "extend[low: 0, high: 0] creatertree[Gebiet]</text--->"
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
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator windowintersects.";
  string rtreeDescriptionStr, relDescriptionStr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 3, errmsg);

  /* Split argument in three parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);

  /* Query window: find out type of key */
  CHECK_COND(nl->IsAtom(searchWindow) &&
    nl->AtomType(searchWindow) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo)||
     nl->SymbolValue(searchWindow) == "rect"  ||
     nl->SymbolValue(searchWindow) == "rect3" ||
     nl->SymbolValue(searchWindow) == "rect4" ||
     nl->SymbolValue(searchWindow) == "rect8" ),
    "Operator windowintersects expects that the search window\n"
    "is of TYPE rect, rect3, rect4, rect8 or "
    "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
    !nl->IsAtom(rtreeDescription) &&
    nl->ListLength(rtreeDescription) == 4,
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a R-Tree list with structure '"
    +rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
    nl->AtomType(rtreeKeyType) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo)||
     nl->IsEqual(rtreeKeyType, "rect")||
     nl->IsEqual(rtreeKeyType, "rect3")||
     nl->IsEqual(rtreeKeyType, "rect4")||
     nl->IsEqual(rtreeKeyType, "rect8")),
   "Operator windowintersects expects a R-Tree with key type\n"
   "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D\n"
   "or rect, rect3, rect4, rect8.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
    (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
     nl->SymbolValue(rtreeSymbol) == "rtree3" ||
     nl->SymbolValue(rtreeSymbol) == "rtree4" ||
     nl->SymbolValue(rtreeSymbol) == "rtree8") ,
   "Operator windowintersects expects a R-Tree \n"
   "of type rtree, rtree3, rtree4,  or rtree8.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
    nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator windowintersects expects a R-Tree with structure "
   "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
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
    "Operator windowintersects expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
    nl->AtomType(tupleSymbol) == SymbolType &&
    nl->SymbolValue(tupleSymbol) == "tuple" &&
    IsTupleDescription(attrList),
    "Operator windowintersects expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  /* check that rtree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList),
   "Operator windowintersects: The tuple type of the R-tree\n"
   "differs from the tuple type of the relation.");

  string attrTypeRtree_str, attrTypeWindow_str;
  nl->WriteToString (attrTypeRtree_str, rtreeKeyType);
  nl->WriteToString (attrTypeWindow_str, searchWindow);

  CHECK_COND(
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
      nl->IsEqual(searchWindow, "rect8") ),
    "Operator windowintersects expects joining attributes of same "
    "dimension.\nBut gets "+attrTypeRtree_str+
    " as left type and "+attrTypeWindow_str+" as right type.\n");

  return
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
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
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
}

/*
5.1.3 Value mapping function of operator ~windowintersects~

*/

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
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;
      R_TreeLeafEntry<dim, TupleId> e;

      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = localInfo->relation->GetTuple(e.info);
          result = SetWord(tuple);
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
          result = SetWord(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;
      delete localInfo->searchBox;
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

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
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator windowintersectsS.";
  string rtreeDescriptionStr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 2, errmsg);

  /* Split argument into two parts */
  ListExpr rtreeDescription = nl->First(args),
           searchWindow = nl->Second(args);

  /* Query window: find out type of key */
  CHECK_COND(nl->IsAtom(searchWindow) &&
    nl->AtomType(searchWindow) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo)||
     nl->SymbolValue(searchWindow) == "rect"  ||
     nl->SymbolValue(searchWindow) == "rect3" ||
     nl->SymbolValue(searchWindow) == "rect4" ||
     nl->SymbolValue(searchWindow) == "rect8"),
    "Operator windowintersectsS expects that the search window\n"
    "is of TYPE rect, rect3, rect4, rect8 or "
    "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
             !nl->IsAtom(rtreeDescription) &&
             nl->ListLength(rtreeDescription) == 4,
    "Operator windowintersectsS expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a R-Tree list with structure '"+
    rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
    nl->AtomType(rtreeKeyType) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo)||
     nl->IsEqual(rtreeKeyType, "rect")||
     nl->IsEqual(rtreeKeyType, "rect3")||
     nl->IsEqual(rtreeKeyType, "rect4")||
     nl->IsEqual(rtreeKeyType, "rect8")),
   "Operator windowintersectsS expects a R-Tree with key type\n"
   "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D\n"
   "or rect, rect3, rect4, rect8.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
    (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
     nl->SymbolValue(rtreeSymbol) == "rtree3" ||
     nl->SymbolValue(rtreeSymbol) == "rtree4" ||
     nl->SymbolValue(rtreeSymbol) == "rtree8"
    ) ,
   "Operator windowintersectsS expects a R-Tree \n"
   "of type rtree, rtree3, rtree4 or rtree8.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator windowintersectsS expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator windowintersectsS expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  string attrTypeRtree_str, attrTypeWindow_str;
  nl->WriteToString (attrTypeRtree_str, rtreeKeyType);
  nl->WriteToString (attrTypeWindow_str, searchWindow);

  CHECK_COND(
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
      nl->IsEqual(searchWindow, "rect8") ),
    "Operator windowintersectsS expects joining attributes of "
    "same dimension.\nBut gets "+attrTypeRtree_str+
    " as left type and "+attrTypeWindow_str+" as right type.\n");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
             nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator windowintersectsS expects a R-Tree with structure "
   "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  if( nl->BoolValue(rtreeTwoLayer) == true )
    return
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->ThreeElemList(
            nl->TwoElemList(
              nl->SymbolAtom("id"),
              nl->SymbolAtom("tid")),
            nl->TwoElemList(
              nl->SymbolAtom("low"),
              nl->SymbolAtom("int")),
            nl->TwoElemList(
              nl->SymbolAtom("high"),
              nl->SymbolAtom("int")))));
  else
    return
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          nl->OneElemList(
            nl->TwoElemList(
              nl->SymbolAtom("id"),
              nl->SymbolAtom("tid")))));
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

  if (nl->SymbolValue(searchWindow) == "rect" ||
      algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo))
    return 0 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == "rect3" ||
           algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo))
    return 1 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == "rect4" ||
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 2 + (!doubleIndex ? 0 : 4);
  else if (nl->SymbolValue(searchWindow) == "rect8" ||
           algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo))
    return 3 + (!doubleIndex ? 0 : 4);

  return -1; /* should not happen */
}

/*
5.1.3 Value mapping function of operator ~windowintersectsB~

*/
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
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      WindowIntersectsSLocalInfo<dim, TupleId> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TupleId>*)local.addr;
      R_TreeLeafEntry<dim, TupleId> e;

      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = new Tuple( localInfo->resultTupleType );
          tuple->PutAttribute(0, new TupleIdentifier(true, e.info));
          result = SetWord(tuple);
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
          result = SetWord(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
    }

    case CLOSE :
    {
      WindowIntersectsSLocalInfo<dim, TupleId>* localInfo =
        (WindowIntersectsSLocalInfo<dim, TupleId>*)local.addr;
      delete localInfo->searchBox;
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;
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
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)
        local.addr;
      R_TreeLeafEntry<dim, TwoLayerLeafInfo> e;

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
          result = SetWord(tuple);
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
          result = SetWord(tuple);
          return YIELD;
        }
        else
          return CANCEL;
      }
    }
    case CLOSE :
    {
      WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo> *localInfo =
        (WindowIntersectsSLocalInfo<dim, TwoLayerLeafInfo>*)
        local.addr;
      delete localInfo->searchBox;
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

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
  string errmsg = "Incorrect input for operator gettuples.";
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  string argStr;
  nl->WriteToString(argStr, args);

  CHECK_COND(!nl->IsEmpty(args) &&
             !nl->IsAtom(args) &&
             nl->ListLength(args) == 2,
    errmsg +
    "\nOperator gettuples expects two arguments, but gets '" +
    argStr + "'.");

  // Split arguments into two parts
  ListExpr streamDescription = nl->First(args),
           relDescription = nl->Second(args);
  string streamDescriptionStr, relDescriptionStr;

  // Handle the stream part of arguments
  nl->WriteToString (streamDescriptionStr, streamDescription);
  CHECK_COND(IsStreamDescription(streamDescription),
    errmsg +
    "\nOperator gettuples expects a first argument with structure "
    "(stream (tuple ((id tid) (a1 t1)...(an tn))))\n"
    "but gets it with structure '" + streamDescriptionStr + "'.");

  // Handle the rel part of arguments
  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(IsRelDescription(relDescription),
    errmsg +
    "\nOperator gettuples expects a second argument with structure "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr sTupleDescription = nl->Second(streamDescription),
           sAttrList = nl->Second(sTupleDescription),
           rTupleDescription = nl->Second(relDescription),
           rAttrList = nl->Second(rTupleDescription);

  // Find the attribute with type tid
  ListExpr first, rest, newAttrList, lastNewAttrList;
  int j, tidIndex = 0;
  string type;
  bool firstcall = true;

  rest = sAttrList;
  j = 1;
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);

    type = nl->SymbolValue(nl->Second(first));
    if (type == "tid")
    {
      CHECK_COND(tidIndex == 0,
       "Operator gettuples expects as first argument a stream with\n"
       "one and only one attribute of type tid but gets\n'" +
       streamDescriptionStr + "'.");
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

  CHECK_COND( tidIndex != 0,
    "Operator gettuples expects as first argument a stream with\n"
    "one and only one attribute of type tid but gets\n'" +
    streamDescriptionStr + "'.");

  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->OneElemList(
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
          newAttrList)));
}

/*
5.1.3 Value mapping function of operator ~gettuples~

The template parameter ~TidIndexPos~ specifies the argument number, where 
the attribute index for the tid is stored within the stream argument's 
tuple type. For ~gettuples~, it is ~2~, for ~gettuples2~, it is ~3~.

*/
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
      cerr << "GetTuples<" << TidIndexPos << ">(): localInfo->tidIndex = " 
          << localInfo->tidIndex << endl;
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (GetTuplesLocalInfo*)local.addr;

      Word wTuple;
      qp->Request(args[0].addr, wTuple);
      if( qp->Received(args[0].addr) )
      {
        Tuple *sTuple = (Tuple*)wTuple.addr,
        *resultTuple = new Tuple( localInfo->resultTupleType ),
        *relTuple = localInfo->relation->
            GetTuple(((TupleIdentifier *)sTuple->
            GetAttribute(localInfo->tidIndex))->GetTid());

        int j = 0;

        // Copy the attributes from the stream tuple
        for( int i = 0; i < sTuple->GetNoAttributes(); i++ )
        {
          if( i != localInfo->tidIndex )
            resultTuple->CopyAttribute( j++, sTuple, i );
        }
        sTuple->DeleteIfAllowed();

        for( int i = 0; i < relTuple->GetNoAttributes(); i++ )
          resultTuple->CopyAttribute( j++, relTuple, i );
        relTuple->DeleteIfAllowed();

        result = SetWord( resultTuple );
        return YIELD;
      }
      else
        return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      localInfo = (GetTuplesLocalInfo*)local.addr;
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;
      return 0;
    }
  }
  return 0;
}

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
      "without the tid attribute.</text--->"
      "<text>query citiesInd windowintersectsS[r] cities gettuples; "
      "where citiesInd is e.g. created with 'let citiesInd = "
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
  string errmsg = "Incorrect input for operator gettuples2. ";
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  string argStr;
  nl->WriteToString(argStr, args);

  CHECK_COND(!nl->IsEmpty(args) &&
      !nl->IsAtom(args) &&
      nl->ListLength(args) == 3,
  errmsg +
      "\nOperator gettuples2 expects three arguments, but gets '" +
      argStr + "'.");

  // Get all three arguments
  ListExpr 
      streamDescription = nl->First(args),
      relDescription = nl->Second(args),
      tidArg = nl->Third(args);
  string streamDescriptionStr, relDescriptionStr, TidArgStr;

  // Handle the stream part of arguments
  nl->WriteToString (streamDescriptionStr, streamDescription);
  CHECK_COND(IsStreamDescription(streamDescription),
             errmsg +
             "\nOperator gettuples2 expects a first argument with structure "
             "(stream (tuple ((id tid) (a1 t1)...(an tn))))\n"
             "but gets it with structure '" + streamDescriptionStr + "'.");
  cerr << "GetTuples2TypeMap: Stream type is: " << streamDescriptionStr << endl;

  // Handle the rel part of arguments
  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(IsRelDescription(relDescription),
             errmsg +
             "\nOperator gettuples2 expects a second argument with structure "
             "(rel (tuple ((a1 t1)...(an tn))))\n"
             "but gets it with structure '" + relDescriptionStr + "'.");
  cerr << "GetTuples2TypeMap: Relation type is: " << relDescriptionStr << endl;

  ListExpr sTupleDescription = nl->Second(streamDescription),
  sAttrList = nl->Second(sTupleDescription),
  rTupleDescription = nl->Second(relDescription),
  rAttrList = nl->Second(rTupleDescription);

  // Check type of third argument (attribute name)
  nl->WriteToString(TidArgStr, tidArg);
  CHECK_COND(nl->IsAtom(tidArg) &&
      nl->AtomType(tidArg) == SymbolType,
  errmsg + "\nOperator gettuples2: The third argument must be the name of "
      "an attribute holding the tuple identifier, but is '"+TidArgStr+"'.");
  string attrName = nl->SymbolValue(tidArg);

  // Create result tuple type
  ListExpr attrList = nl->Second(sTupleDescription);
  int attrIndex = 0;
  ListExpr attrType;

  CHECK_COND(
      (attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
      errmsg +
      "\nOperator gettuples2 expects the attribute " +
      attrName + "\npassed as third argument to be part of "
      "the stream description\n'" +
      relDescriptionStr + "'.");

  string type = nl->SymbolValue(attrType);
  CHECK_COND((type == "tid"),
      errmsg +
      "\nOperator gettuples2 expects the attribute " +
      attrName + "\npassed as third argument to be of type "
      "'tid', but it is '"+ +"'.\n'" + type + "'.");

  ListExpr first, rest, newAttrList, lastNewAttrList;

  rest = sAttrList;
  int j = 1;
  bool firstcall = true;
  // copy attrs from stream (skipping the tid)
  while (!nl->IsEmpty(rest))
  {
    first = nl->First(rest);
    rest = nl->Rest(rest);
    if (j != attrIndex)
    {
      if ( firstcall )
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
  // copy attrs from relation
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

  ListExpr restype = 
      nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
  nl->OneElemList(
      nl->IntAtom(attrIndex)),
  nl->TwoElemList(
      nl->SymbolAtom("stream"),
  nl->TwoElemList(
      nl->SymbolAtom("tuple"),
  newAttrList)));

  string restypeStr;
  nl->WriteToString(restypeStr, restype);
  cerr << "GetTuples2TypeMap returns '" << restypeStr << "'." << endl;

  return restype;
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
         gettuplesSpec,          // specification
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
  string errmsg = "Incorrect input for operator gettuplesdbl.";
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();

  string argStr;
  nl->WriteToString(argStr, args);

  CHECK_COND(!nl->IsEmpty(args) &&
             !nl->IsAtom(args) &&
             nl->ListLength(args) == 3,
    errmsg +
    "\nOperator gettuplesdbl expects three arguments, but gets '" +
    argStr + "'.");

  // Split arguments into three parts
  ListExpr streamDescription = nl->First(args),
           relDescription = nl->Second(args),
           attrnameDescription = nl->Third(args);
  string streamDescriptionStr, relDescriptionStr, attrnameDescriptionStr;

  // Handle the stream part of arguments
  nl->WriteToString (streamDescriptionStr, streamDescription);

  CHECK_COND(IsStreamDescription(streamDescription),
    errmsg +
    "\nOperator gettuplesdbl expects a first argument with structure "
    "(stream (tuple ((id tid) (a1 t1)...(an tn))))\n"
    "but gets it with structure '" + streamDescriptionStr + "'.");

  // Handle the rel part of arguments
  nl->WriteToString (relDescriptionStr, relDescription);

  CHECK_COND(IsRelDescription(relDescription),
    errmsg +
    "\nOperator gettuplesdbl expects a second argument with structure "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr sTupleDescription = nl->Second(streamDescription),
           sAttrList = nl->Second(sTupleDescription),
           rTupleDescription = nl->Second(relDescription),
           rAttrList = nl->Second(rTupleDescription);

  nl->WriteToString (attrnameDescriptionStr, attrnameDescription);

  CHECK_COND(nl->IsAtom(attrnameDescription) &&
    nl->AtomType(attrnameDescription) == SymbolType,
    errmsg + "\nOperator gettuplesdbl expects as third argument the name "
    "of the indexed attribute but gets '" + attrnameDescriptionStr + "'" );

  string attrName = nl->SymbolValue(attrnameDescription);

  int attrIndex;
  ListExpr attrType;

  CHECK_COND(
    (attrIndex = FindAttribute(rAttrList, attrName, attrType)) > 0,
    errmsg +
    "\nOperator gettuplesdbl expects that the attribute " +
    attrName + "\npassed as third argument to be part of "
    "the relation passed as second\n'" +
    relDescriptionStr + "'.");

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
    if (type == "tid")
    {
      CHECK_COND(tidIndex == 0,
       "Operator gettuplesdbl expects as first argument a stream with\n"
       "one and only one attribute of type tid but gets\n'" +
       streamDescriptionStr + "'.");
      tidIndex = j;
    }
    else if( type == "int" &&
             tidIndex == j-1 )
    {
      dblIdxFirst = true;
    }
    else if( type == "int" &&
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

  CHECK_COND( tidIndex != 0,
    "Operator gettuplesdbl expects as first argument a stream with\n"
    "one and only one attribute of type tid but gets\n'" +
    streamDescriptionStr + "'.");

  return
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->TwoElemList(
        nl->IntAtom(attrIndex),
        nl->IntAtom(tidIndex)),
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        nl->TwoElemList(
          nl->SymbolAtom("tuple"),
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
      local = SetWord(localInfo);
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
                            localInfo->intervals );
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

          result = SetWord( resultTuple );
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
                localInfo->intervals );

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

        result = SetWord( resultTuple );
        return YIELD;
      }
      else
        return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      localInfo = (GetTuplesDblLocalInfo*)local.addr;
      localInfo->resultTupleType->DeleteIfAllowed();
      delete localInfo;
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
  string attrName, relDescriptionStr, argstr;
  string errmsg = "Incorrect input for operator creatertree_bulkload.";
  int attrIndex;
  ListExpr attrType;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  CHECK_COND(!nl->IsEmpty(args) &&
      nl->ListLength(args) == 2,
  errmsg + "\nOperator creatertree_bulkload expects two arguments.");

  ListExpr relDescription = nl->First(args),
  attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE) &&
      nl->AtomType(attrNameLE) == SymbolType,
  errmsg + "\nThe second argument must be the name of "
      "the attribute to index.");
  attrName = nl->SymbolValue(attrNameLE);

  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(!nl->IsEmpty(relDescription) &&
      nl->ListLength(relDescription) == 2,
  errmsg +
      "\nOperator creatertree expects a first argument with structure "
      "(stream (tuple ((a1 t1)...(an tn))))\n"
      "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND( (nl->IsEqual(nl->First(relDescription), "stream")) &&
      nl->ListLength(tupleDescription) == 2,
  errmsg +
      "\nOperator creatertree_bulkload expects a first argument with "
      "structure (stream (tuple ((a1 t1)...(an tn))))\n"
      "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr attrList = nl->Second(tupleDescription);
  CHECK_COND(nl->IsEqual(nl->First(tupleDescription), "tuple") &&
      IsTupleDescription(attrList),
  errmsg +
      "\nOperator creatertree_bulkload expects a first argument with "
      "structure (stream (tuple ((a1 t1)...(an tn))))\n"
      "but gets it with structure '" + relDescriptionStr + "'.");

  CHECK_COND(
      (attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
  errmsg +
      "\nOperator creatertree_bulkload expects that the attribute " +
      attrName + "\npassed as second argument to be part of "
      "the stream description\n'" +
      relDescriptionStr + "'.");

  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)||
      algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)||
      algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)||
      algMgr->CheckKind("SPATIAL8D", attrType, errorInfo)||
      nl->IsEqual(attrType, "rect")||
      nl->IsEqual(attrType, "rect3")||
      nl->IsEqual(attrType, "rect4")||
      nl->IsEqual(attrType, "rect8"),
  errmsg +
      "\nOperator creatertree_bulkload expects that attribute "+attrName+"\n"
      "belongs to kinds SPATIAL2D, SPATIAL3D, Spatial4D or SPATIAL8D\n"
      "or rect, rect3, rect4 and rect8.");

  string rtreetype;

  if ( algMgr->CheckKind("SPATIAL2D", attrType, errorInfo) ||
       nl->IsEqual( attrType, "rect" ) )
    rtreetype = "rtree";
  else if ( algMgr->CheckKind("SPATIAL3D", attrType, errorInfo) ||
            nl->IsEqual( attrType, "rect3" ) )
    rtreetype = "rtree3";
  else if ( algMgr->CheckKind("SPATIAL4D", attrType, errorInfo) ||
            nl->IsEqual( attrType, "rect4" ) )
    rtreetype = "rtree4";
  else if ( algMgr->CheckKind("SPATIAL8D", attrType, errorInfo) ||
            nl->IsEqual( attrType, "rect8" ) )
    rtreetype = "rtree8";

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
      if (type == "tid")
      {
        CHECK_COND( tidIndex == 0,
                    "Operator creatertree_bulkload expects as first "
                    "argument a stream "
                    "with\none and only one attribute of type 'tid'\n'"
                    "but gets\n'" + relDescriptionStr + "'.");

        tidIndex = j;
      }
      else if( j == nAttrs - 1 && type == "int" &&
               nl->SymbolValue(
               nl->Second(nl->First(rest))) == "int" )
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
    CHECK_COND( tidIndex != 0,
                "Operator creatertree_bulkload expects as first "
                "argument a stream "
                "with\none and only one attribute of type 'tid'\n'"
                "but gets\n'" + relDescriptionStr + "'.");

    return
        nl->ThreeElemList(
        nl->SymbolAtom("APPEND"),
    nl->TwoElemList(
        nl->IntAtom(attrIndex),
    nl->IntAtom(tidIndex)),
    nl->FourElemList(
        nl->SymbolAtom(rtreetype),
    nl->TwoElemList(
        nl->SymbolAtom("tuple"),
    newAttrList),
    attrType,
    nl->BoolAtom(doubleIndex)));
}

/*
5.2. Value Mapping for Operator ~creatertree\_bulkload<D>~

*/
template<unsigned dim>
int CreateRTreeBulkLoadStreamSpatial( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
// Step 0 - Initialization
  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
      (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  assert(rtree->InitializeBulkLoad());

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
        R_TreeLeafEntry<dim, TupleId> 
              e( box,
                 ((TupleIdentifier *)tuple->
                     GetAttribute(tidIndex))->GetTid() );
        rtree->InsertBulkLoad(e);
    }
    tuple->DeleteIfAllowed();
    qp->Request(args[0].addr, wTuple);
  }
  qp->Close(args[0].addr);
  assert( rtree->FinalizeBulkLoad() );

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
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
      (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  assert(rtree->InitializeBulkLoad());
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
  assert( rtree->FinalizeBulkLoad() );

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
  Word wTuple;
  R_Tree<dim, TupleId> *rtree =
      (R_Tree<dim, TupleId>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

  // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  assert(rtree->InitializeBulkLoad());
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

  assert( rtree->FinalizeBulkLoad() );
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
  Word wTuple;
  R_Tree<dim, TwoLayerLeafInfo> *rtree =
      (R_Tree<dim, TwoLayerLeafInfo>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  int attrIndex = ((CcInt*)args[2].addr)->GetIntval() - 1,
  tidIndex = ((CcInt*)args[3].addr)->GetIntval() - 1;

        // Get a reference to the message center
  static MessageCenter* msg = MessageCenter::GetInstance();
  int count = 0; // counter for progress indicator

  assert(rtree->InitializeBulkLoad());

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

  assert( rtree->FinalizeBulkLoad() );
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

  if ( algMgr->CheckKind("SPATIAL2D", attrType, errorInfo) )
    result = 0;
  else if ( algMgr->CheckKind("SPATIAL3D", attrType, errorInfo) )
    result = 1;
  else if ( algMgr->CheckKind("SPATIAL4D", attrType, errorInfo) )
    result = 2;
  else if ( algMgr->CheckKind("SPATIAL8D", attrType, errorInfo) )
    result = 3;
  else if( nl->SymbolValue(attrType) == "rect" )
    result = 4;
  else if( nl->SymbolValue(attrType) == "rect3" )
    result = 5;
  else if( nl->SymbolValue(attrType) == "rect4" )
    result = 6;
  else if( nl->SymbolValue(attrType) == "rect8" )
    result = 7;
  else
    return -1; /* should not happen */

  if( nl->SymbolValue(nl->First(relDescription)) == "stream")
  {
    ListExpr first,
    rest = attrList;
    while (!nl->IsEmpty(rest))
    {
      first = nl->First(rest);
      rest = nl->Rest(rest);
    }
    if( nl->IsEqual( nl->Second( first ), "int" ) )
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
5.9 Inquiry Operators ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

This operators can be used to inquire some basic R-tree statistics.

*/

/*
5.9.1 Type Mapping Functions for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

*/
ListExpr RTree2IntTypeMap(ListExpr args)
{
  string rtreeDescriptionStr, argstr;
  nl->WriteToString (argstr, args);
  string errmsg = "Incorrect input '" + argstr + 
                  "' for rtree inquiry operator.";

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 1, errmsg);

  ListExpr rtreeDescription = nl->First(args);
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);

  CHECK_COND(!nl->IsEmpty(rtreeDescription), errmsg); // X
  CHECK_COND(!nl->IsAtom(rtreeDescription), errmsg);  // X
  ListExpr rtreeSymbol = nl->First(rtreeDescription);

  /* handle rtree type constructor */
  CHECK_COND( 
      nl->IsAtom(rtreeSymbol) &&
      nl->AtomType(rtreeSymbol) == SymbolType &&
      (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
      nl->SymbolValue(rtreeSymbol) == "rtree3" ||
      nl->SymbolValue(rtreeSymbol) == "rtree4" ||
      nl->SymbolValue(rtreeSymbol) == "rtree8") ,
  "Rtree inquiry operator expects a R-Tree \n"
      "of type rtree, rtree3, rtree4 or rtree8.");

  return nl->SymbolAtom("int");
}

ListExpr RTree2RectTypeMap(ListExpr args)
{
  string rtreeDescriptionStr, argstr;
  nl->WriteToString (argstr, args);
  string errmsg = "Incorrect input '" + argstr + 
      "' for rtree inquiry operator.";

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 1, errmsg);

  ListExpr rtreeDescription = nl->First(args);
  CHECK_COND(nl->ListLength(rtreeDescription) == 4, errmsg);
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);

  CHECK_COND(!nl->IsEmpty(rtreeDescription), errmsg); // X
  CHECK_COND(!nl->IsAtom(rtreeDescription), errmsg);  // X
  ListExpr rtreeSymbol = nl->First(rtreeDescription);

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
      nl->AtomType(rtreeSymbol) == SymbolType &&
      (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
      nl->SymbolValue(rtreeSymbol) == "rtree3" ||
      nl->SymbolValue(rtreeSymbol) == "rtree4" ||
      nl->SymbolValue(rtreeSymbol) == "rtree8") ,
  "Rtree inquiry operator expects a R-Tree \n"
      "of type rtree, rtree3, rtree4 or rtree8.");

  if(nl->SymbolValue(rtreeSymbol) == "rtree") 
    return nl->SymbolAtom("rect");
  if(nl->SymbolValue(rtreeSymbol) == "rtree3")
    return nl->SymbolAtom("rect3");
  if(nl->SymbolValue(rtreeSymbol) == "rtree4")
    return nl->SymbolAtom("rect4");
  if(nl->SymbolValue(rtreeSymbol) == "rtree8")
    return nl->SymbolAtom("rect8");

  return nl->SymbolAtom("typeerror");
}

/*
5.9.2 Value Mapping Functions for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

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

/*
5.9.3 Selection Function for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

*/
int RTreeInquirySelect (ListExpr args)
{
  int result = -100;
  ListExpr rtreeDescription = nl->First(args);
  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  if(nl->SymbolValue(rtreeSymbol) == "rtree") 
    result = 0;
  else if(nl->SymbolValue(rtreeSymbol) == "rtree3")
    result = 1;
  else if(nl->SymbolValue(rtreeSymbol) == "rtree4")
    result = 2;
  else if(nl->SymbolValue(rtreeSymbol) == "rtree8")
    result = 3;

  if( nl->BoolValue(rtreeTwoLayer) == true )
    result += 4;

  return result;
}

/*
5.9.3 Value Mapping Arrays for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

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



/*
5.9.5 Specification Strings for ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

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

/*
5.9.6 Definitions of ~treeheight~, ~no\_nodes~, ~no\_entries~, ~bbox~

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
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator nodes.";
  string rtreeDescriptionStr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 1, errmsg);

  /* handle rtree part of argument */
  ListExpr rtreeDescription = nl->First(args);
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
      !nl->IsAtom(rtreeDescription) &&
      nl->ListLength(rtreeDescription) == 4,
  "Operator nodes expects a R-Tree with structure "
      "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))) attrtype "
      "bool)\nbut gets a R-Tree list with structure '"
      +rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
  rtreeTupleDescription = nl->Second(rtreeDescription),
  rtreeKeyType = nl->Third(rtreeDescription),
  rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
      nl->AtomType(rtreeKeyType) == SymbolType &&
      (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
      algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
      algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo)||
      nl->IsEqual(rtreeKeyType, "rect")||
      nl->IsEqual(rtreeKeyType, "rect3")||
      nl->IsEqual(rtreeKeyType, "rect4")),
  "Operator nodes expects a R-Tree with key type\n"
      "of kind SPATIAL2D, SPATIAL3D, and SPATIAL4D\n"
      "or rect, rect3, and rect4.");

  ListExpr MBR_ATOM;
  if(         nl->IsEqual(rtreeKeyType, "rect")
           || algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo))
    MBR_ATOM = nl->SymbolAtom("rect");
  else if(    nl->IsEqual(rtreeKeyType, "rect3") 
           || algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo))
    MBR_ATOM = nl->SymbolAtom("rect3");
  else if(    nl->IsEqual(rtreeKeyType, "rect4")
           || algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo))
    MBR_ATOM = nl->SymbolAtom("rect4");
  else if(    nl->IsEqual(rtreeKeyType, "rect8")
           || algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo))
    MBR_ATOM = nl->SymbolAtom("rect8");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
      nl->AtomType(rtreeSymbol) == SymbolType &&
      (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
      nl->SymbolValue(rtreeSymbol) == "rtree3" ||
      nl->SymbolValue(rtreeSymbol) == "rtree4") ,
  "Operator nodes expects a R-Tree \n"
      "of type rtree, rtree3 or rtree4.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
      !nl->IsAtom(rtreeTupleDescription) &&
      nl->ListLength(rtreeTupleDescription) == 2,
  "Operator nodes expects a R-Tree with structure "
      "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))) attrtype "
      "bool)\nbut gets a first list with wrong tuple description in "
      "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
      nl->AtomType(rtreeTupleSymbol) == SymbolType &&
      nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
      IsTupleDescription(rtreeAttrList),
  "Operator nodes expects a R-Tree with structure "
      "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))) attrtype "
      "bool)\nbut gets a first list with wrong tuple description in "
      "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
      nl->AtomType(rtreeTwoLayer) == BoolType,
  "Operator nodes expects a R-Tree with structure "
      "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))) attrtype "
      "bool)\nbut gets a first list with wrong tuple description in "
      "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr reslist = 
   nl->TwoElemList(
    nl->SymbolAtom("stream"),
    nl->TwoElemList(
     nl->SymbolAtom("tuple"),
     nl->Cons(
      nl->TwoElemList(nl->SymbolAtom("level"), nl->SymbolAtom("int")),
      nl->Cons(
       nl->TwoElemList(nl->SymbolAtom("nodeId"), nl->SymbolAtom("int")),
       nl->SixElemList(
        nl->TwoElemList(nl->SymbolAtom("MBR"), MBR_ATOM),
        nl->TwoElemList(nl->SymbolAtom("fatherID"), nl->SymbolAtom("int")),
        nl->TwoElemList(nl->SymbolAtom("isLeaf"), nl->SymbolAtom("bool")),
        nl->TwoElemList(nl->SymbolAtom("minEntries"), nl->SymbolAtom("int")),
        nl->TwoElemList(nl->SymbolAtom("maxEntries"), nl->SymbolAtom("int")),
        nl->TwoElemList(nl->SymbolAtom("countEntries"), nl->SymbolAtom("int"))
       )
      )
     )
    )
   );
//   string resstring;
//   nl->WriteToString (resstring, reslist);
//   cout << "RTreeNodesTypeMap(): reslist = " << resstring << endl;
  return reslist;
}


/*
5.10.2 Value Mapping for Operator ~nodes~

*/

template<unsigned dim>
struct RTreeNodesLocalInfo {
  bool firstCall;
  bool finished;
  TupleType *resultTupleType;
  R_Tree<dim, TupleId> *rtree;
};

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
      local = SetWord(lci);
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
        result = SetWord(tuple);
        return YIELD;
      }
    }

    case CLOSE :
    {
      lci = (RTreeNodesLocalInfo<dim> *)local.addr;
      lci->resultTupleType->DeleteIfAllowed();
      delete lci;
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

  if(         nl->IsEqual(rtreeKeyType, "rect")
           || algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo))
    return 0;
  else if(    nl->IsEqual(rtreeKeyType, "rect3") 
           || algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo))
    return 1;
  else if(    nl->IsEqual(rtreeKeyType, "rect4")
           || algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo))
    return 2;
  else if(    nl->IsEqual(rtreeKeyType, "rect8")
           || algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo))
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
  }
  ~RTreeAlgebra() {};
};

RTreeAlgebra rtreeAlgebra;


extern "C"
Algebra*
InitializeRTreeAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&rtreeAlgebra);
}

