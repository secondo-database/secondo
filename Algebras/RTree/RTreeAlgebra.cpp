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

[TOC]

0 Overview

This Algebra implements the creation of two-, three- and four-dimensional
R-Trees.

First the the type constructors ~rtree~, ~rtree3~ and ~rtree4~ are defined.

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
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RectangleAlgebra.h"
#include "RTreeAlgebra.h"
#include "CPUTimeMeasurer.h"

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
  else if( ((SortedArrayItem *) a)->pri > ((SortedArrayItem *) b)->pri )
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
  nl->AppendText(examplelist, "<relation> creatertree [<attrname>]"
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
    && (nl->ListLength(type) == 3)
    && nl->Equal(nl->First(type), nl->SymbolAtom("rtree")))
  {
    algMgr = SecondoSystem::GetAlgebraManager();
    return
      algMgr->CheckKind("TUPLE", nl->Second(type), errorInfo)
      &&nl->Equal(nl->Third(type), nl->SymbolAtom("rect"));
  }
  else
  {
    errorInfo = nl->Append(errorInfo,
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("RTREE2"), type));
    return false;
  }
  return true;
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree( "rtree",              RTree2Prop,
                        OutRTree<2>,           InRTree<2>,
                        0,                     0,
                        CreateRTree<2>,        DeleteRTree<2>,
                        OpenRTree<2>,          SaveRTree<2>,
                        CloseRTree<2>,         CloneRTree<2>,
                        CastRTree<2>,          SizeOfRTree<2>,
                        CheckRTree2,
                        0,
                        TypeConstructor::DummyInModel,
                        TypeConstructor::DummyOutModel,
                        TypeConstructor::DummyValueToModel,
                        TypeConstructor::DummyValueListToModel );

/*
2 Type constructor ~rtree3~

2.1 Type property of type constructor ~rtree3~

*/
ListExpr RTree3Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist, "<relation> creatertree [<attrname>]"
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
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("RTREE3"), type));
    return false;
  }
  return true;
}

/*
1.12 Type Constructor object for type constructor ~rtree3~

*/
TypeConstructor rtree3( "rtree3",             RTree3Prop,
                        OutRTree<3>,          InRTree<3>,
                        0,                    0,
                        CreateRTree<3>,       DeleteRTree<3>,
                        OpenRTree<3>,         SaveRTree<3>,
                        CloseRTree<3>,        CloneRTree<3>,
                        CastRTree<3>,         SizeOfRTree<3>,
                        CheckRTree3,
                        0,
                        TypeConstructor::DummyInModel,
                        TypeConstructor::DummyOutModel,
                        TypeConstructor::DummyValueToModel,
                        TypeConstructor::DummyValueListToModel );

/*
3 Type constructor ~rtree4~

3.1 Type property of type constructor ~rtree4~

*/
ListExpr RTree4Prop()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"<relation> creatertree [<attrname>]"
                             " where <attrname> is the key of type rect4");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                             nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
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
      nl->ThreeElemList(nl->IntAtom(60), nl->SymbolAtom("RTREE4"), type));
    return false;
  }
  return true;
}

/*
3.12 Type Constructor object for type constructor ~rtree~

*/
TypeConstructor rtree4( "rtree4",             RTree4Prop,
                        OutRTree<4>,          InRTree<4>,
                        0,                    0,
                        CreateRTree<4>,       DeleteRTree<4>,
                        OpenRTree<4>,         SaveRTree<4>,
                        CloseRTree<4>,        CloneRTree<4>,
                        CastRTree<4>,         SizeOfRTree<4>,
                        CheckRTree4,
                        0,
                        TypeConstructor::DummyInModel,
                        TypeConstructor::DummyOutModel,
                        TypeConstructor::DummyValueToModel,
                        TypeConstructor::DummyValueListToModel );


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
  string attrName, relDescriptionStr;
  char* errmsg = "Incorrect input for operator creatertree.";
  int attrIndex;
  ListExpr attrType;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  CHECK_COND(!nl->IsEmpty(args) &&
              nl->ListLength(args) == 2,
   "Operator creatertree expects two arguments.");


  ListExpr relDescription = nl->First(args);
  ListExpr attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE), errmsg);
  CHECK_COND(nl->AtomType(attrNameLE) == SymbolType, errmsg);

  attrName = nl->SymbolValue(attrNameLE);

  nl->WriteToString (relDescriptionStr, relDescription); //for error message
  CHECK_COND(!nl->IsEmpty(relDescription) &&
             nl->ListLength(relDescription) == 2,
    "Operator creatertree expects a first list with structure "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure '"+relDescriptionStr+"'.");

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol) &&
             nl->AtomType(relSymbol) == SymbolType &&
             nl->SymbolValue(relSymbol) == "rel" &&
             nl->ListLength(tupleDescription) == 2,
    "Operator creatertree expects a first list with structure "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure '"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
             nl->AtomType(tupleSymbol) == SymbolType &&
             nl->SymbolValue(tupleSymbol) == "tuple" &&
             IsTupleDescription(attrList),
    "Operator creatertree expects a first list with structure "
    "(rel (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with structure '"+relDescriptionStr+"'.");


  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
    "Operator creatertree expects an attributename "+attrName+" in first list\n"
    "but gets a first list with structure '"+relDescriptionStr+"'.");

  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)||
             algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)||
             algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)||
             nl->IsEqual(attrType, "rect")||
             nl->IsEqual(attrType, "rect3")||
             nl->IsEqual(attrType, "rect4"),
    "Operator creatertree expects that attribute "+attrName+"\n"
    "belongs to kinds SPATIAL2D, SPATIAL3D, or SPATIAL4D\n"
    "or rect, rect3, and rect4.");

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
  else
    assert (false); /* should not happen */

  ListExpr resultType =
    nl->ThreeElemList(
      nl->SymbolAtom("APPEND"),
      nl->TwoElemList(
        nl->IntAtom(attrIndex),
        nl->StringAtom(nl->SymbolValue(attrType))),
      nl->ThreeElemList(
        nl->SymbolAtom(rtreetype),
        tupleDescription,
        attrType));

  return resultType;
}

/*
4.1.2 Selection function of operator ~creatertree~

*/
int
CreateRTreeSelect (ListExpr args)
{
  ListExpr relDescription = nl->First(args);
  ListExpr attrNameLE = nl->Second(args);
  string attrName = nl->SymbolValue(attrNameLE);
  ListExpr tupleDescription = nl->Second(relDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  int attrIndex;
  ListExpr attrType;
  attrIndex = FindAttribute(attrList, attrName, attrType);

  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  if ( algMgr->CheckKind("SPATIAL2D", attrType, errorInfo) )
    return 0;
  else if ( algMgr->CheckKind("SPATIAL3D", attrType, errorInfo) )
    return 1;
  else if ( algMgr->CheckKind("SPATIAL4D", attrType, errorInfo) )
    return 2;
  else if( nl->SymbolValue(attrType) == "rect" )
    return 3;
  else if( nl->SymbolValue(attrType) == "rect3" )
    return 4;
  else if( nl->SymbolValue(attrType) == "rect4" )
    return 5;
  else
    return -1; /* should not happen */
}

/*
4.1.3 Value mapping function of operator ~creatertree~

*/
template<unsigned dim>
int CreateRTreeValueMapping_spatial(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Relation* relation;
  CcInt* attrIndexCcInt;
  int attrIndex;
  CcString* attrTypeStr;
  RelationIterator* iter;
  Tuple* tuple;

  R_Tree<dim> *rtree = (R_Tree<dim>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  relation = (Relation*)args[0].addr;
  attrIndexCcInt = (CcInt*)args[2].addr;
  attrTypeStr = (CcString*)args[3].addr;

  assert(rtree != 0);
  assert(relation != 0);
  assert(attrIndexCcInt != 0);
  assert(attrTypeStr != 0);

  attrIndex = attrIndexCcInt->GetIntval() - 1;

  iter = relation->MakeScan();

  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    BBox<dim> box = ((StandardSpatialAttribute<dim>*)tuple->GetAttribute(attrIndex))->BoundingBox();
    R_TreeEntry<dim> e( box, tuple->GetTupleId() );
    rtree->Insert( e );

    tuple->DeleteIfAllowed();
  }

  delete iter;
  return 0;
}

template<unsigned dim>
int CreateRTreeValueMapping_rect(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Relation* relation;
  CcInt* attrIndexCcInt;
  int attrIndex;
  CcString* attrTypeStr;
  RelationIterator* iter;
  Tuple* tuple;

  R_Tree<dim> *rtree = (R_Tree<dim>*)qp->ResultStorage(s).addr;
  result = SetWord( rtree );

  relation = (Relation*)args[0].addr;
  attrIndexCcInt = (CcInt*)args[2].addr;
  attrTypeStr = (CcString*)args[3].addr;

  assert(rtree != 0);
  assert(relation != 0);
  assert(attrIndexCcInt != 0);
  assert(attrTypeStr != 0);

  attrIndex = attrIndexCcInt->GetIntval() - 1;

  iter = relation->MakeScan();

  while( (tuple = iter->GetNextTuple()) != 0 )
  {
    BBox<dim> *box = (BBox<dim>*)tuple->GetAttribute(attrIndex);
    R_TreeEntry<dim> e( *box, tuple->GetTupleId() );
    rtree->Insert( e );

    tuple->DeleteIfAllowed();
  }

  delete iter;
  return 0;
}

/*
4.1.4 The dummy model mapping

*/
Word
RTreeNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

/*
4.1.5 Definition of value mapping vectors

*/
ValueMapping rtreecreatertreemap [] = { CreateRTreeValueMapping_spatial<2>,
                                        CreateRTreeValueMapping_spatial<3>,
                                        CreateRTreeValueMapping_spatial<4>,
                                        CreateRTreeValueMapping_rect<2>,
                                        CreateRTreeValueMapping_rect<3>,
                                        CreateRTreeValueMapping_rect<4> };

ModelMapping rtreenomodelmap[] = { RTreeNoModelMapping,
                                   RTreeNoModelMapping,
                                   RTreeNoModelMapping };

/*
4.1.6 Specification of operator ~creatertree~

*/
const string CreateRTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                "\"Example\" ) "
                                "( <text>((rel (tuple ((x1 t1)...(xn tn))))"
                                " xi)"
                                " -> (rtree<d> (tuple ((x1 t1)...(xn tn))) ti)"
                                "</text--->"
                                "<text>_ creatertree [ _ ]</text--->"
                                "<text>Creates an rtree<d>. The key type ti must"
                                " be of kind SPATIAL2D, SPATIAL3D or SPATIAL4D.</text--->"
                                "<text>let myrtree = countries creatertree [boundary]"
                                "</text--->"
                                ") )";

/*
4.1.7 Definition of operator ~creatertree~

*/
Operator creatertree (
          "creatertree",                   // name
          CreateRTreeSpec,                 // specification
          6,                               //Number of overloaded functions
          rtreecreatertreemap,             // value mapping
          rtreenomodelmap,                 // dummy model mapping, defines in Algebra.h
          CreateRTreeSelect,               // trivial selection function
          CreateRTreeTypeMap               // type mapping
);

/*
7.2 Operator ~windowintersects~

7.2.1 Type mapping function of operator ~windowintersects~

*/
ListExpr WindowIntersectsTypeMap(ListExpr args)
{
  AlgebraManager *algMgr;
  algMgr = SecondoSystem::GetAlgebraManager();
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
             ((nl->SymbolValue(searchWindow) == "rect")  ||
              (nl->SymbolValue(searchWindow) == "rect3") ||
              (nl->SymbolValue(searchWindow) == "rect4") ),
    "Operator windowintersects expects that the search window\n"
    "is of TYPE rect, rect3, rect4 or of Spatial-Kind.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription); //for error message
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
             !nl->IsAtom(rtreeDescription) &&
             nl->ListLength(rtreeDescription) == 3,
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a R-Tree list with structure '"+rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription);;
  ListExpr rtreeTupleDescription = nl->Second(rtreeDescription);
  ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
             nl->AtomType(rtreeKeyType) == SymbolType &&
             (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
              algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
              algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo)||
              nl->IsEqual(rtreeKeyType, "rect")||
              nl->IsEqual(rtreeKeyType, "rect3")||
              nl->IsEqual(rtreeKeyType, "rect4")),
   "Operator windowintersects expects a R-Tree with key type\n"
   "of kind SPATIAL2D, SPATIAL3D, and SPATIAL4D\n"
   "or rect, rect3, and rect4.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
             nl->AtomType(rtreeSymbol) == SymbolType &&
             (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
              nl->SymbolValue(rtreeSymbol) == "rtree3" ||
              nl->SymbolValue(rtreeSymbol) == "rtree4") ,
   "Operator windowintersects expects a R-Tree \n"
   "of type rtree, rtree3 or rtree4.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
             !nl->IsAtom(rtreeTupleDescription) &&
             nl->ListLength(rtreeTupleDescription) == 2,
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with wrong tuple description in structure \n"
    "'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
             nl->AtomType(rtreeTupleSymbol) == SymbolType &&
             nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
             IsTupleDescription(rtreeAttrList),
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4 (tuple ((a1 t1)...(an tn))))\n"
    "but gets a first list with wrong tuple description in structure \n"
    "'"+rtreeDescriptionStr+"'.");

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription); //for error message
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

  CHECK_COND( ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
                nl->IsEqual(searchWindow, "rect") ) ||
              ( nl->IsEqual(rtreeKeyType, "rect") &&
                nl->IsEqual(searchWindow, "rect") ) ||
              ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
                nl->IsEqual(searchWindow, "rect3") ) ||
              ( nl->IsEqual(rtreeKeyType, "rect3") &&
                nl->IsEqual(searchWindow, "rect3") ) ||
              ( algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo) &&
                nl->IsEqual(searchWindow, "rect4") ) ||
              ( nl->IsEqual(rtreeKeyType, "rect4") &&
                nl->IsEqual(searchWindow, "rect4") ),
    "Operator windowintersects expects joining attributes of same dimension.\n"
    "But gets "+attrTypeRtree_str+" as left type and "+attrTypeWindow_str+" as right type.\n");


  ListExpr resultType =
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);

  return resultType;
}

/*
5.1.2 Selection function of operator ~windowintersects~

*/
int
WindowIntersectsSelection( ListExpr args )
{
  /* Split argument in three parts */
  ListExpr searchWindow = nl->Third(args);

  /* find out type of key */
  if (nl->SymbolValue(searchWindow) == "rect")
    return 0;
  else if (nl->SymbolValue(searchWindow) == "rect3")
    return 1;
  else if (nl->SymbolValue(searchWindow) == "rect4")
    return 2;
  else return -1; /* should not happen */
}

/*
5.1.3 Value mapping function of operator ~windowintersects~

*/

template <unsigned dim>
struct WindowIntersectsLocalInfo
{
  Relation* relation;
  R_Tree<dim>* rtree;
  BBox<dim> *searchBox;
  bool first;
};

template <unsigned dim>
int WindowIntersectsValueMapping( Word* args, Word& result,
                                  int message, Word& local,
                                  Supplier s )
{
  WindowIntersectsLocalInfo<dim> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      Word rtreeWord, relWord, boxWord;
      qp->Request(args[0].addr, rtreeWord);
      qp->Request(args[1].addr, relWord);
      qp->Request(args[2].addr, boxWord);

      localInfo = new WindowIntersectsLocalInfo<dim>;
      localInfo->rtree = (R_Tree<dim>*)rtreeWord.addr;
      localInfo->relation = (Relation*)relWord.addr;
      localInfo->first = true;
      localInfo->searchBox = new BBox<dim> ( (((StandardSpatialAttribute<dim> *)boxWord.addr)->BoundingBox()) );

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (WindowIntersectsLocalInfo<dim>*)local.addr;
      R_TreeEntry<dim> e;

      if(localInfo->first)
      {
        localInfo->first = false;
        if( localInfo->rtree->First( *localInfo->searchBox, e ) )
        {
          Tuple *tuple = localInfo->relation->GetTuple(e.pointer);
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
          Tuple *tuple = localInfo->relation->GetTuple(e.pointer);
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
ValueMapping rtreewindowintersectsmap [] = { WindowIntersectsValueMapping<2>,
                                             WindowIntersectsValueMapping<3>,
                                             WindowIntersectsValueMapping<4> };


/*
5.1.5 Specification of operator ~windowintersects~

*/
const string windowintersectsSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>((rtree (tuple ((x1 t1)...(xn tn)))"
      " ti)(rel (tuple ((x1 t1)...(xn tn)))) rect||rect3||rect4) ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
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
         3,                         //number of overloaded functions
         rtreewindowintersectsmap,  // value mapping
         rtreenomodelmap,           // dummy model mapping
         WindowIntersectsSelection, // trivial selection function
         WindowIntersectsTypeMap    // type mapping
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

    AddOperator( &creatertree );
    AddOperator( &windowintersects );
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

