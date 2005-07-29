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
#include "TupleIdentifier.h"

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
  string errmsg = "Incorrect input for operator creatertree.";
  int attrIndex;
  ListExpr attrType;
  AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  CHECK_COND(!nl->IsEmpty(args) &&
              nl->ListLength(args) == 2,
              errmsg + "\nOperator creatertree expects two arguments.");

  ListExpr relDescription = nl->First(args);
  ListExpr attrNameLE = nl->Second(args);

  CHECK_COND(nl->IsAtom(attrNameLE) &&
             nl->AtomType(attrNameLE) == SymbolType,
             errmsg + "\nThe second argument must be the name of the attribute to index.");
  attrName = nl->SymbolValue(attrNameLE);

  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(!nl->IsEmpty(relDescription) &&
             nl->ListLength(relDescription) == 2,
             errmsg + "\nOperator creatertree expects a first argument with structure "
             "(rel (tuple ((a1 t1)...(an tn))))\n"
             "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsEqual(nl->First(relDescription), "rel") &&
             nl->ListLength(tupleDescription) == 2,
             errmsg + "\nOperator creatertree expects a first argument with structure "
             "(rel (tuple ((a1 t1)...(an tn))))\n"
             "but gets it with structure '" + relDescriptionStr + "'.");

  ListExpr attrList = nl->Second(tupleDescription);
  CHECK_COND(nl->IsEqual(nl->First(tupleDescription), "tuple") &&
             IsTupleDescription(attrList),
             errmsg + "\nOperator creatertree expects a first argument with structure "
             "(rel (tuple ((a1 t1)...(an tn))))\n"
             "but gets it with structure '" + relDescriptionStr + "'.");

  CHECK_COND((attrIndex = FindAttribute(attrList, attrName, attrType)) > 0,
             errmsg + "\nOperator creatertree expects that the attribute " + attrName +
             "\npassed as second argument to be part of the relation description\n'"
             + relDescriptionStr + "'.");

  CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)||
             algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)||
             algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)||
             nl->IsEqual(attrType, "rect")||
             nl->IsEqual(attrType, "rect3")||
             nl->IsEqual(attrType, "rect4"),
             errmsg + "\nOperator creatertree expects that attribute "+attrName+"\n"
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
7.3 Operator ~insertrtree~

For each tuple of the inputstream inserts an entry into the rtree. The entry is built from the 
spatial-attribute over which the tree is built and the tuple-identifier wich is extracted from the 
input-tuples as the last attribute. The inputstream is returned again as the result of this operator.


7.3.0 General Type mapping function of operators ~insertrtree~, ~deletertree~ and ~updatertree~




Type mapping ~insertrtree~ and ~deletertree~ on a rtree

----     (stream (tuple ((a1 x1) ... (an xn) (tid int)))) (rtree X ti) ai

        -> (stream (tuple ((a1 x1) ... (an xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

Type mapping ~updatertree~ on a rtree

----     (stream (tuple ((a1 x1) ... (an xn)(a1_old x1)... (an_old xn) (TID tid)))) (rtree X ti) ai

        -> (stream (tuple ((a1 x1) ... (an xn)(a1_old x1) (an_old xn) (TID tid))))

        where X = (tuple ((a1 x1) ... (an xn)))
----

*/

ListExpr allUpdatesRTreeTypeMap( ListExpr& args, string opName )
{
  	ListExpr  rest,next,listn,lastlistn,restRTreeAttrs, oldAttribute,outList;
  	string argstr, argstr2, oldName;
  
  	AlgebraManager* algMgr = SecondoSystem::GetAlgebraManager();
  	ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  

   	/* Split argument in three parts */
  	ListExpr streamDescription = nl->First(args);
  	ListExpr rtreeDescription = nl->Second(args);
  	ListExpr nameOfKeyAttribute = nl->Third(args);

	// Test stream
  	nl->WriteToString(argstr, streamDescription);
  	CHECK_COND(nl->ListLength(streamDescription) == 2  &&
             	(TypeOfRelAlgSymbol(nl->First(streamDescription)) == stream) &&
             	(nl->ListLength(nl->Second(streamDescription)) == 2) &&
             	(TypeOfRelAlgSymbol(nl->First(nl->Second(streamDescription))) == tuple) &&
       			(nl->ListLength(nl->Second(streamDescription)) == 2) &&
       			(IsTupleDescription(nl->Second(nl->Second(streamDescription)))),
    			"Operator " + opName + " expects as first argument a list with structure "
    			"(stream (tuple ((a1 t1)...(an tn)(tid real)))\n"
    			"Operator " + opName + " gets as first argument '" + argstr + "'." );
    // Test if last attribute is of type 'tid'
  	rest = nl->Second(nl->Second(streamDescription));
  	while (!(nl->IsEmpty(rest)))
  	{
     	next = nl->First(rest);
     	rest = nl->Rest(rest);
  	}
  	CHECK_COND(!(nl->IsAtom(next)) &&
             	(nl->IsAtom(nl->Second(next)))&&
             	(nl->AtomType(nl->Second(next)) == SymbolType)&&
             	(nl->SymbolValue(nl->Second(next)) == "tid"),
    			"Operator " + opName + ": Type of last attribute of tuples of the inputstream must be tid" );
 	// Test rtree

  	/* handle rtree part of argument */
  	CHECK_COND(!nl->IsEmpty(rtreeDescription), "Operator " + opName + ": Description for the rtree may not be empty");
  	CHECK_COND(!nl->IsAtom(rtreeDescription), "Operator " + opName + ": Description for the rtree may not be an atom");
  	CHECK_COND(nl->ListLength(rtreeDescription) == 3, "Operator " + opName + ": Description for the rtree must consist of three parts");

  	ListExpr rtreeSymbol = nl->First(rtreeDescription);;
  	ListExpr rtreeTupleDescription = nl->Second(rtreeDescription);
  	ListExpr rtreeKeyType = nl->Third(rtreeDescription);

  	/* handle rtree type constructor */
  	CHECK_COND(nl->IsAtom(rtreeSymbol), "Operator " + opName + ": First part of the rtree-description has to be 'rtree'");
  	CHECK_COND(nl->AtomType(rtreeSymbol) == SymbolType, "Operator " + opName + ": First part of the rtree-description has to be 'bree' ");
  	CHECK_COND(nl->SymbolValue(rtreeSymbol) == "rtree","Operator " + opName + ": First part of the rtree-description has to be 'bree' ");
	/* handle btree tuple description */
  	CHECK_COND(!nl->IsEmpty(rtreeTupleDescription), "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	CHECK_COND(!nl->IsAtom(rtreeTupleDescription), "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	CHECK_COND(nl->ListLength(rtreeTupleDescription) == 2, "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);;
  	ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  	CHECK_COND(nl->IsAtom(rtreeTupleSymbol),"Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	CHECK_COND(nl->AtomType(rtreeTupleSymbol) == SymbolType, "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	CHECK_COND(nl->SymbolValue(rtreeTupleSymbol) == "tuple", "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  	CHECK_COND(IsTupleDescription(rtreeAttrList), "Operator " + opName + ": Second part of the rtree-description has to be a tuple-description ");
  
   	/* Handle key-part of rtreedescription */
  	CHECK_COND(nl->IsAtom(rtreeKeyType), "Operator " + opName + ": Key of the rtree has to be an atom");
  	CHECK_COND(nl->AtomType(rtreeKeyType) == SymbolType,"Operator " + opName + ": Key of the rtree has to be an atom");
  
  	// Handle third argument which shall be the name of the attribute of the streamtuples
  	// that serves as the key for the rtree
  	// Later on it is checked if this name is an attributename of the inputtuples
  	CHECK_COND(nl->IsAtom(nameOfKeyAttribute), "Operator " + opName + ": Name of the key-attribute of the streamtuples has to be an atom");
  	CHECK_COND(nl->AtomType(nameOfKeyAttribute) == SymbolType, "Operator " + opName + ": Name of the key-attribute of the streamtuples has to be an atom");

  	// Check whether tupledescription of the stream without the last attribute is the same as the tupledescription of the rtree 
  	rest = nl->Second(nl->Second(streamDescription));
  	CHECK_COND(nl->ListLength(rest) > 1 , "Operator " + opName + ": There must be at least two attributes in the tuples of the tuple-stram");
	//Test if stream-tupledescription fits to btree-tupledescription
  	listn = nl->OneElemList(nl->First(rest));
  	lastlistn = listn;
  	rest = nl->Rest(rest);
  	// For updates the inputtuples need to carry the old attributevalues after the
  	// new values but their names with an additional _old at the end
  	if (opName == "updatebtree"){
  		// Compare first part of the streamdescription
  		while (nl->ListLength(rest) > nl->ListLength(rtreeAttrList) + 1){
     		lastlistn = nl->Append(lastlistn,nl->First(rest));
     		rest = nl->Rest(rest);
  		}
  		CHECK_COND(nl->Equal(listn,rtreeAttrList), "Operator " + opName + ": First part of the tupledescription of the stream "
  			"has to be the same as the tupledescription of the rtree");
  		// Compare second part of the streamdescription
  		restRTreeAttrs = rtreeAttrList;
  		while (nl->ListLength(rest) >  1){
  	 		nl->WriteToString(oldName, nl->First(nl->First(restRTreeAttrs)));
  	 		oldName += "_old";
  	 		oldAttribute = nl->TwoElemList(nl->SymbolAtom(oldName),nl->Second(nl->First(restRTreeAttrs)));
  	 		CHECK_COND(nl->Equal(oldAttribute,nl->First(rest)), "Operator " + opName + ": Second part of the tupledescription of the stream "
     					"without the last attribute has to be the same as the tupledescription of the rtree except for that"
     					" the attributenames carry an additional '_old.'");
     		rest = nl->Rest(rest);
     		restRTreeAttrs = nl->Rest(restRTreeAttrs);
  		}		
  	}
  	// For insert and delete check whether tupledescription of the stream without the last 
  	//attribute is the same as the tupledescription of the rtree 
  	else{ // operators insertrtree and deletertree
  		while (nl->ListLength(rest) > 1){
     		lastlistn = nl->Append(lastlistn,nl->First(rest));
     		rest = nl->Rest(rest);
  		}
  		CHECK_COND(nl->Equal(listn,rtreeAttrList), "Operator " + opName + ": tupledescription of the stream without the"
  					"last attribute has to be the same as the tupledescription of the rtree");
  	}
  	
	// Test if attributename of the third argument exists as a name in the attributlist of the streamtuples
	string attrname = nl->SymbolValue(nameOfKeyAttribute);
	ListExpr attrType;
	int j = FindAttribute(listn,attrname,attrType);
	CHECK_COND(j != 0, "Operator " + opName + ": Name of the attribute that shall contain the keyvalue for the"
	 			"rtree was not found as a name of the attributes of the tuples of the inputstream"); 
	//Test if type of the attriubte which shall be taken as a key is the same as the keytype of the rtree
	CHECK_COND(nl->Equal(attrType,rtreeKeyType), "Operator " + opName + ": Type of the attribute that shall contain the keyvalue for the"
	 			"rtree is not the same as the keytype of the rtree");
	// Check if indexed attribute has a spatial-type
	CHECK_COND(algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)||
             	algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)||
             	algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)||
             	nl->IsEqual(attrType, "rect")||
             	nl->IsEqual(attrType, "rect3")||
             	nl->IsEqual(attrType, "rect4"),
             	"Operator " + opName + " expects that attribute "+attrname+"\n"
             	"belongs to kinds SPATIAL2D, SPATIAL3D, or SPATIAL4D\n"
             	"or rect, rect3, and rect4.");
     // Extract dimension and spatianltype to append them to the resultlist
	 int dim = 0;
	 int spatial = 0;
	 if (nl->IsEqual(attrType, "rect"))
		dim = 2; 
	 if (nl->IsEqual(attrType, "rect3"))
		dim = 3;
	 if (nl->IsEqual(attrType, "rect4"))
		dim = 4;
	 if (algMgr->CheckKind("SPATIAL2D", attrType, errorInfo)){
		dim = 2;
		spatial = 1; 
	 }
	 if (algMgr->CheckKind("SPATIAL3D", attrType, errorInfo)){
		dim = 3;
		spatial = 1; 
	 }
	 if (algMgr->CheckKind("SPATIAL4D", attrType, errorInfo)){
		dim = 4;
		spatial = 1; 
	 }
	 //Append the index of the attribute over which the btree is built to the resultlist. 
	 ListExpr append = nl->OneElemList(nl->IntAtom(j));
	 ListExpr lastAppend = append;
	 //Append the dimension of the spatial-attribute to the resutllist.
	 lastAppend = nl->Append(lastAppend,nl->IntAtom(dim));
	 //Append if the index-attribute is of 'rect'- or 'spatial'-type
	 lastAppend = nl->Append(lastAppend,nl->IntAtom(spatial));
	 //Append the index of the attribute over which the btree is built to the resultlist. 
	 outList = nl->ThreeElemList(nl->SymbolAtom("APPEND"), nl->OneElemList(append),streamDescription);
  	 return outList;
}

/*

7.3.1 TypeMapping of operator ~insertrtree~

*/

ListExpr insertRTreeTypeMap(ListExpr args){
	return allUpdatesRTreeTypeMap(args, "insertrtree");
}

/*

7.3.2 ValueMapping of operator ~insertrtree~

*/


template<unsigned dim>
 void insertRTree_rect(Word& rtreeWord, Attribute* keyAttr, TupleId& oldTid){
	R_Tree<dim> *rtree = (R_Tree<dim>*)rtreeWord.addr;  
    BBox<dim> *box = (BBox<dim>*)keyAttr;
    R_TreeEntry<dim> e( *box, oldTid );
    rtree->Insert( e );
}

template<unsigned dim>
 void insertRTree_spatial(Word& rtreeWord, Attribute* keyAttr, TupleId& oldTid){
	R_Tree<dim> *rtree = (R_Tree<dim>*)rtreeWord.addr;  
    BBox<dim> box = ((StandardSpatialAttribute<dim>*)keyAttr)->BoundingBox();
    R_TreeEntry<dim> e( box, oldTid );
    rtree->Insert( e );
}

int insertRTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRTree, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;
  

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local = SetWord(localTransport );
      return 0;

    case REQUEST :
	  localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      qp->Request(args[1].addr, argRTree);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1);      
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	if (spatial){
       		switch(dim){
       		case 2: insertRTree_spatial<2>(argRTree,keyAttr,oldTid); break;
       		case 3: insertRTree_spatial<3>(argRTree,keyAttr,oldTid); break;
       		case 4: insertRTree_spatial<4>(argRTree,keyAttr,oldTid); break;
       		default: cout << "Error: Trying to enter an entry with dimension: " << dim << " into a rtree" << endl;
       		}
       	}
       	else{
       		switch(dim){
       		case 2: insertRTree_rect<2>(argRTree,keyAttr,oldTid); break;
       		case 3: insertRTree_rect<3>(argRTree,keyAttr,oldTid); break;
       		case 4: insertRTree_rect<4>(argRTree,keyAttr,oldTid); break;
       		default: cout << "Error: Trying to enter an entry with dimension: " << dim << " into a rtree" << endl;
       		}
       	}
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
  		localTransport = (int*) local.addr;
    	delete[] localTransport;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
        return 0;
  }
  return 0;
}

/*
7.3.3 Specification of operator ~insertrtree~

*/
const string insertRTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x@[TID:tid])) x rtree(tuple(x) ti) x xi"
                           " -> stream(tuple(x@[TID:tid]))] "
                           "</text--->"
                           "<text>_ _ insertrtree [_]</text--->"
                           "<text>Inserts references to the tuples with TupleId 'TID' "
                           "into the rtree.</text--->"
                           "<text>query neueStaedte feed staedte insert staedte_Ort "
                           " insertrtree [Ort] count "
                           "</text--->"
                           ") )";

/*
7.3.4 Definition of operator ~insertrtree~

*/
Operator insertrtree (
         "insertrtree",              // name
         insertRTreeSpec,            // specification
         insertRTreeValueMap,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         insertRTreeTypeMap          // type mapping
);

/*
7.4 Operator ~deletertree~

For each tuple of the inputstream removes an entry from the rtree. The entry is built from the 
spatial-attribute over which the tree is built and the tuple-identifier wich is extracted from the 
input-tuples as the last attribute. The inputstream is returned again as the result of this operator.


7.4.1 Type mapping function of operator ~deletertree~


*/

ListExpr deleteRTreeTypeMap( ListExpr args )
{
  return allUpdatesRTreeTypeMap(args, "deletertree");
}

/*
7.4.1 Value mapping function of operator ~deletertree~


*/

template<unsigned dim>
 void deleteRTree_rect(Word& rtreeWord, Attribute* keyAttr, TupleId& oldTid){
	R_Tree<dim> *rtree = (R_Tree<dim>*)rtreeWord.addr;  
    BBox<dim> *box = (BBox<dim>*)keyAttr;
    R_TreeEntry<dim> e( *box, oldTid );
    rtree->Remove( e );
}

template<unsigned dim>
 void deleteRTree_spatial(Word& rtreeWord, Attribute* keyAttr, TupleId& oldTid){
	R_Tree<dim> *rtree = (R_Tree<dim>*)rtreeWord.addr;  
    BBox<dim> box = ((StandardSpatialAttribute<dim>*)keyAttr)->BoundingBox();
    R_TreeEntry<dim> e( box, oldTid );
    rtree->Remove( e );
}

int deleteRTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRTree, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;
  

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local = SetWord(localTransport );
      return 0;

    case REQUEST :
	  localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      qp->Request(args[1].addr, argRTree);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1);      
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	if (spatial){
       		switch(dim){
       		case 2: deleteRTree_spatial<2>(argRTree,keyAttr,oldTid); break;
       		case 3: deleteRTree_spatial<3>(argRTree,keyAttr,oldTid); break;
       		case 4: deleteRTree_spatial<4>(argRTree,keyAttr,oldTid); break;
       		default: cout << "Error: Trying to remove an entry with dimension: " << dim << " from a rtree" << endl;
       		}
       	}
       	else{
       		switch(dim){
       		case 2: deleteRTree_rect<2>(argRTree,keyAttr,oldTid); break;
       		case 3: deleteRTree_rect<3>(argRTree,keyAttr,oldTid); break;
       		case 4: deleteRTree_rect<4>(argRTree,keyAttr,oldTid); break;
       		default: cout << "Error: Trying to remove an entry with dimension: " << dim << " from a rtree" << endl;
       		}
       	}
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
  		localTransport = (int*) local.addr;
    	delete[] localTransport;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
        return 0;
  }
  return 0;
}

/*
7.4.3 Specification of operator ~deletertree~

*/
const string deleteRTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x@[TID: tid])) x rtree(tuple(x) ti) x xi)"
                           " -> stream(tuple(x@[TID: tid]))] "
                           "</text--->"
                           "<text>_ _ deletertree [_]</text--->"
                           "<text>Deletes references to the tuples with TupleId 'TID' "
                           "from the rtree.</text--->"
                           "<text>query staedte feed filter [.Name = 'Hagen'] "
                           " staedte deletedirect staedte_Ort deletertree [Ort] count "
                           "</text--->"
                           ") )";

/*
7.4.4 Definition of operator ~deletertree~

*/
Operator deletertree (
         "deletertree",              // name
         deleteRTreeSpec,            // specification
         deleteRTreeValueMap,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         deleteRTreeTypeMap          // type mapping
);

/*
7.5 Operator ~updatertree~

For each tuple of the inputstream checks if the attribute over which the tree has been built has changed.
If it has changed the old entry for this tuple is removed and a new one is inserted. 
The entry is built from the spatial-attribute over which the tree is built and the tuple-identifier 
wich is extracted from the input-tuples as the last attribute. The inputstream is returned 
again as the result of this operator.


7.5.1 Type mapping function of operator ~updatertree~


*/

ListExpr updateRTreeTypeMap( ListExpr args )
{
  return allUpdatesRTreeTypeMap (args, "updatebtree");
}

/*
7.5.1 Value mapping function of operator ~updatertree~


*/

int updateRTreeValueMap(Word* args, Word& result, int message, Word& local, Supplier s)
{
  Word t, argRTree, attrPos, dimWord, spatialWord;
  Tuple* tup;
  CcInt* indexp;
  CcInt* dimp;
  CcInt* spatialp;
  int index, dim, spatial;
  Attribute* keyAttr;
  Attribute* oldKeyAttr;
  Attribute* tidAttr;
  TupleId oldTid;
  SmiKey key;
  int* localTransport;
  Supplier suppIndex, suppDim, suppSpatial;
  

  switch (message)
  {
    case OPEN :
      qp->Open(args[0].addr);
      suppIndex = qp->GetSupplier(args[3].addr,0);
      qp->Request(suppIndex,attrPos);
      indexp = ((CcInt*)attrPos.addr);
      suppDim = qp->GetSupplier(args[3].addr,1);
      qp->Request(suppDim,dimWord);
      dimp = ((CcInt*)dimWord.addr);
      suppSpatial = qp->GetSupplier(args[3].addr,2);
      qp->Request(suppSpatial,spatialWord);
      spatialp = ((CcInt*)spatialWord.addr);
      localTransport = new int[3];
      localTransport[0] = indexp->GetIntval();
      localTransport[1] = dimp->GetIntval();
      localTransport[2] = spatialp->GetIntval();
      local = SetWord(localTransport );
      return 0;

    case REQUEST :
	  localTransport = (int*) local.addr;
      index = localTransport[0];
      dim = localTransport[1];
      spatial = localTransport[2];
      qp->Request(args[0].addr,t);
      qp->Request(args[1].addr, argRTree);
      if (qp->Received(args[0].addr))
      {
        tup = (Tuple*)t.addr;
       	keyAttr = tup->GetAttribute(index - 1); 
       	int noOfAttrs = tup->GetNoAttributes();
       	int oldIndex = (noOfAttrs - 1)/2 + index -1;
       	oldKeyAttr = tup->GetAttribute(oldIndex); 
       	tidAttr = tup->GetAttribute(tup->GetNoAttributes() - 1);
       	oldTid = ((TupleIdentifier*)tidAttr)->GetTid();
       	if (spatial){
       		switch(dim){
       			case 2: if ((((StandardSpatialAttribute<2>*) keyAttr)->BoundingBox()) != (((StandardSpatialAttribute<2>*) oldKeyAttr)->BoundingBox())){ 
       						deleteRTree_spatial<2>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_spatial<2>(argRTree,keyAttr,oldTid); 	     						
       					}
       					break;
       			case 3: if ((((StandardSpatialAttribute<3>*) keyAttr)->BoundingBox()) != (((StandardSpatialAttribute<3>*) oldKeyAttr)->BoundingBox())){ 
       						deleteRTree_spatial<3>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_spatial<3>(argRTree,keyAttr,oldTid); 	
       					}
       					break;
       			case 4: if ((((StandardSpatialAttribute<4>*) keyAttr)->BoundingBox()) != (((StandardSpatialAttribute<4>*) oldKeyAttr)->BoundingBox())){ 
       						deleteRTree_spatial<4>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_spatial<4>(argRTree,keyAttr,oldTid); 	
       					}
       					break;
       			default: cout << "Error: Trying to update an entry with dimension: " << dim << " from a rtree" << endl;
       		}
       	}
       	else{
       		switch(dim){
       			case 2: if (((Rectangle<2>*) keyAttr) != ((Rectangle<2>*) oldKeyAttr)){ 
       						deleteRTree_rect<2>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_rect<2>(argRTree,keyAttr,oldTid); 	
       					}
       					break;
       			case 3: if (((Rectangle<3>*) keyAttr) != ((Rectangle<3>*) oldKeyAttr)){ 
       						deleteRTree_rect<3>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_rect<3>(argRTree,keyAttr,oldTid); 	
       					}
       					break;
       			case 4: if (((Rectangle<4>*) keyAttr) != ((Rectangle<4>*) oldKeyAttr)){ 
       						deleteRTree_rect<4>(argRTree,oldKeyAttr,oldTid);
       						insertRTree_rect<4>(argRTree,keyAttr,oldTid); 	
       					}
       					break;
       			default: cout << "Error: Trying to update an entry with dimension: " << dim << " from a rtree" << endl;
       		}
       	}
        result = SetWord(tup);
        return YIELD;
      }
      else
        return CANCEL;

    case CLOSE :
  		localTransport = (int*) local.addr;
    	delete[] localTransport;
    	qp->Close(args[0].addr);
    	qp->SetModified(args[1].addr);
        return 0;
  }
  return 0;
}

/*
7.5.3 Specification of operator ~updatertree~

*/
const string updateRTreeSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" ) "
                           "( <text>stream(tuple(x@[(a1_old x1) ... (an_old xn) (TID: tid)])) "
                           "x rtree(tuple(x) ti) x xi"
                           " -> stream(tuple(x@[(a1_old x1) ... (an_old xn) (TID: tid)]))] "
                           "</text--->"
                           "<text>_ _ updatertree [_]</text--->"
                           "<text>Updates references to the tuples with TupleId 'TID'"
                           "in the rtree.</text--->"
                           "<text>query staedte feed filter [.Name = 'Hagen'] "
                           " staedte updatedirect [Ort : newRegion] staedte_Ort updatertree [Ort] count "
                           "</text--->"
                           ") )";

/*
7.5.4 Definition of operator ~updatertree~

*/
Operator updatertree (
         "updatertree",              // name
         updateRTreeSpec,            // specification
         updateRTreeValueMap,                // value mapping
         Operator::DummyModel,  // dummy model mapping, defines in Algebra.h
         Operator::SimpleSelect,          // trivial selection function
         updateRTreeTypeMap          // type mapping
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
    AddOperator( &insertrtree );
    AddOperator( &deletertree );
    AddOperator( &updatertree );
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

