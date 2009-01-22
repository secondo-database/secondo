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

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "NearestNeighborAlgebra.h"
#include "TemporalAlgebra.h"
#include "BBTree.h"

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

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;

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
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator distancescan.";
  string rtreeDescriptionStr, relDescriptionStr, argstr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 4, errmsg);

  /* Split argument in four parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);

  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) 
    && (nl->AtomType(quantity) == SymbolType) 
    && (nl->SymbolValue(quantity) == "int"),
  "Operator distancescan expects a fourth argument of type integer (k or -1).\n"
  "Operator distancescan gets '" + argstr + "'.");

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
    "Operator distancescan expects that the search window\n"
    "is of TYPE rect, rect3, rect4, rect8 or "
    "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
    !nl->IsAtom(rtreeDescription) &&
    nl->ListLength(rtreeDescription) == 4,
    "Operator distancescan expects a R-Tree with structure "
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
   "Operator distancescan expects a R-Tree with key type\n"
   "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D\n"
   "or rect, rect3, rect4, rect8.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
    (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
     nl->SymbolValue(rtreeSymbol) == "rtree3" ||
     nl->SymbolValue(rtreeSymbol) == "rtree4" ||
     nl->SymbolValue(rtreeSymbol) == "rtree8") ,
   "Operator distancescan expects a R-Tree \n"
   "of type rtree, rtree3, rtree4,  or rtree8.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator distancescan expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator distancescan expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
    nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator distancescan expects a R-Tree with structure "
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
    "Operator distancescan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
    nl->AtomType(tupleSymbol) == SymbolType &&
    nl->SymbolValue(tupleSymbol) == "tuple" &&
    IsTupleDescription(attrList),
    "Operator distancescan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  /* check that rtree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList),
   "Operator distancescan: The tuple type of the R-tree\n"
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
    "Operator distancescan expects joining attributes of same "
    "dimension.\nBut gets "+attrTypeRtree_str+
    " as left type and "+attrTypeWindow_str+" as right type.\n");


    return
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        tupleDescription);
}

/*
The function knearestTypeMap is the type map for the operator knearest

*/
ListExpr
knearestTypeMap( ListExpr args )
{
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 4,
    "Operator knearest expects a list of length four.");

  ListExpr first = nl->First(args),
           second = nl->Second(args),
           third = nl->Third(args),
           quantity = nl->Fourth(args);

  ListExpr tupleDescription = nl->Second(first);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (IsTupleDescription(nl->Second(tupleDescription))),
    "Operator knearest expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator knearest gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->IsAtom(second) &&
    nl->AtomType(second) == SymbolType,
    "Operator knearest expects as second argument an atom "
    "(an attribute name).\n"
    "Operator knearest gets '" + argstr + "'.\n");

  string attrName = nl->SymbolValue(second);
  ListExpr attrType;
  int j;
  if( !(j = FindAttribute(nl->Second(nl->Second(first)), 
                          attrName, attrType)) )
  {
    nl->WriteToString(argstr, nl->Second(nl->Second(first)));
    ErrorReporter::ReportError(
      "Operator knearest expects as secong argument an attribute name.\n"
      "Attribute name '" + attrName + 
      "' does not belong to the tuple of the first argument.\n"
      "Known Attribute(s): " + argstr + "\n");
    return nl->SymbolAtom("typeerror");
  }

  // check for third argument type == mpoint
  nl->WriteToString(argstr, third);
  CHECK_COND((nl->IsEqual( third, "mpoint" )),
  "Operator knearest expects a third argument of type point.\n"
  "Operator knearest gets '" + argstr + "'.");

  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) 
    && (nl->AtomType(quantity) == SymbolType) 
    && (nl->SymbolValue(quantity) == "int"),
  "Operator knearest expects a fourth argument of type integer (k or -1).\n"
  "Operator knearest gets '" + argstr + "'.");

  return
    nl->ThreeElemList(
    nl->SymbolAtom("APPEND"),
    nl->OneElemList(nl->IntAtom(j)),
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription));
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

  CHECK_COND(nl->ListLength(args) == 4,
    "Operator knearest expects a list of length four.");

  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr third = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);

  // check for third argument type == mpoint
  nl->WriteToString(argstr, third);
  CHECK_COND((nl->IsEqual( third, "mpoint" )),
  "Operator knearestfilter expects a third argument of type mpoint.\n"
  "Operator knearestfilter gets '" + argstr + "'.");

  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) 
    && (nl->AtomType(quantity) == SymbolType) 
    && (nl->SymbolValue(quantity) == "int"),
  "Operator expects a fourth arg. of type integer.\n"
  "Operator knearestfilter gets '" + argstr + "'.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
    !nl->IsAtom(rtreeDescription) &&
    nl->ListLength(rtreeDescription) == 4,
    "Operator knearestfilter expects a R-Tree with structure "
    "(rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a R-Tree list with structure '"
    +rtreeDescriptionStr+"'.");

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

  return
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription);
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
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
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
  const UPoint *upn;
  Instant time1 = up->timeInterval.start;
  Instant time2 = up->timeInterval.end;
  int noCo = mp->GetNoComponents();
  for (int ii=mpos; ii < noCo; ++ii)
  {
    mp->Get( ii, upn);
    if( time1 < upn->timeInterval.end 
      || (time1 == upn->timeInterval.end && upn->timeInterval.rc))
    { 
      mpos = ii;
      UReal firstu(true);
      Instant start = up->timeInterval.start < upn->timeInterval.start
        ? upn->timeInterval.start : up->timeInterval.start;
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
  int pos, max, start;
  max = v.size() - 1;
  start = 0;
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
    else if( dist > storeDistance || (dist == storeDistance && slope1 > slope2))
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
fills the eventQueue with the start- and the endpoints of
all units in the timeinterval of the mpoint.
All other tupels are deleted

*/
void fillEventQueue( Word *args, priority_queue<EventElem> &evq,
                    Instant &startTime, Instant &endTime)
{
  const MPoint *mp = (MPoint*)args[2].addr;
  int mpos = 0;
  int j = ((CcInt*)args[4].addr)->GetIntval() - 1;
  qp->Open(args[0].addr);
  Word currentTupleWord; 
  qp->Request( args[0].addr, currentTupleWord );
  while( qp->Received( args[0].addr ) )
  {
    //fill eventqueue with start- and endpoints
    Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
    const UPoint* upointAttr 
        = (const UPoint*)currentTuple->GetAttribute(j);
    Instant t1 = upointAttr->timeInterval.start;  
    Instant t2 = upointAttr->timeInterval.end; 
    if( t1 >= endTime || t2 <= startTime)
    {
      currentTuple->DeleteIfAllowed();
    }
    else
    {
      MReal *mr = new MReal(0);
      GetDistance( mp, upointAttr, mpos, mr);
      Instant t3 = t1 >= startTime ? t1 : startTime;
      Instant t4 = t2 <= endTime ? t2 : endTime;
      evq.push( EventElem(E_LEFT, t3, 
          currentTuple, upointAttr, mr) );
      evq.push( EventElem(E_RIGHT, t4, 
        currentTuple, upointAttr, mr) );
    }
    qp->Request( args[0].addr, currentTupleWord );
  }
}

/*
checks if there are intersections and puts them into the eventqueue
this is the right function if the eventqueue is a NNTree

*/
void checkIntersections(EventType type, Instant &time, IT pos,
                      NNTree< ActiveElem > &t,
                      priority_queue<EventElem> &evq, Instant &endTime)
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
void deleteDupElements(priority_queue<EventElem> &evq, EventType type,
                       Tuple* tuple1, Tuple* tuple2, Instant& time)
{
  while( !evq.empty() )
  {
    //eleminate same elements
    EventElem elem2 = evq.top();
    if( type != elem2.type || time != elem2.pointInTime 
      || tuple1 != elem2.tuple || tuple2 != elem2.tuple2)
    {
      break;
    }
    else
    {
      evq.pop();
    }
  }
}

/*
  To use the plane sweep algrithmus a priority queue for the events and
  a NNTree to save the active segments is needed. 

*/
struct KnearestLocalInfo
{
  unsigned int k;
  bool scanFlag;
  Instant startTime, endTime;
  NNTree< ActiveElem > activeLine;
  priority_queue<EventElem> eventQueue;
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
  KnearestLocalInfo *localInfo;
  const MPoint *mp = (MPoint*)args[2].addr;

  switch (message)
  {
    /* 
    in open the eventqueue is initialized with the left
    and the right elements

    */
    case OPEN :
    {
      localInfo = new KnearestLocalInfo;
      localInfo->k = (unsigned)((CcInt*)args[3].addr)->GetIntval();
      localInfo->scanFlag = true;
      local = SetWord(localInfo);
      if (mp->IsEmpty())
      {
        return 0;
      }
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      localInfo->startTime = up1->timeInterval.start;
      localInfo->endTime = up2->timeInterval.end;

      fillEventQueue( args, localInfo->eventQueue, localInfo->startTime,
            localInfo->endTime);
      return 0;
    }

    /* 
    in request the eventqueue is executed. new intersect events
    are computed

    */
    case REQUEST :
    {
     int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;
     localInfo = (KnearestLocalInfo*)local.addr;
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
              localInfo->activeLine.erase( posFirst );
              va.push_back(newElem1);
              s.insert(posFirst->tuple);

              ActiveElem newElem2(posSec->distance, 
                  posSec->tuple, elem.pointInTime, 
                  posSec->end, posSec->lc, posSec->rc); 
              localInfo->activeLine.erase( posSec );
              va.push_back(newElem2);
              s.insert(posSec->tuple);

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
      localInfo = (KnearestLocalInfo*)local.addr;
      delete localInfo;
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
                      priority_queue<EventElem> &evq, Instant &endTime)
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
  unsigned int k;
  //int max;
  bool scanFlag;
  Instant startTime, endTime;
  vector< ActiveElem > activeLine;
  priority_queue<EventElem> eventQueue;
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
  const MPoint *mp = (MPoint*)args[2].addr;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new KnearestLocalInfoVector;
      localInfo->activeLine.reserve(100);
      localInfo->k = (unsigned)((CcInt*)args[3].addr)->GetIntval();
      localInfo->scanFlag = true;
      local = SetWord(localInfo);
      if (mp->IsEmpty())
      {
        return 0;
      }
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      localInfo->startTime = up1->timeInterval.start;
      localInfo->endTime = up2->timeInterval.end;
    
      fillEventQueue( args, localInfo->eventQueue, localInfo->startTime,
            localInfo->endTime);
      return 0;
    }

    case REQUEST :
    {
     int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;
     localInfo = (KnearestLocalInfoVector*)local.addr;
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
      delete localInfo;
      return 0;
    }
  }

  return 0;
}

/*
The ~knearestfilter~ operator results a stream of all input tuples which 
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
  double d = MIN( (MIN( d1, d2)), (MIN( d3, d4)) );

  return sqrt(d);
}

/*
the recursive helpfunction of the function coverage

*/
void helpcoverage(int cycle, map<Instant, int> &m, R_Tree<3, TupleId>* rtree, 
                  Instant &t, SmiRecordId nodeid, int level)
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
      Instant ts(t.GetType());
      Instant te(t.GetType());
      ts.ReadFrom(e.box.MinD(2));
      te.ReadFrom(e.box.MaxD(2));
      if( cycle == 0)
      {
        if( ts < t ) m[ts] = 0;
        if( te < t ) m[te] = 0;
      }
      else
      {
        typedef map<Instant, int>::iterator ITMAP;
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
      helpcoverage( cycle, m, rtree, t, e.pointer, level + 1);
    }
  }
  delete tmp;
}

/*
calculates the minimum coverage of a rtree-node
in its timeintervall

*/
int coverage(R_Tree<3, TupleId>* rtree, Instant &tEnd,
                 SmiRecordId nodeid, int level)
{
  map<Instant, int> m;
  helpcoverage( 0, m, rtree, tEnd, nodeid, level );
  helpcoverage( 1, m, rtree, tEnd, nodeid, level );
  typedef map<Instant, int>::const_iterator ITMAP;
  int result = 0;
  ITMAP it = m.begin();
  if( it != m.end() ) result = it->second;
  for( ; it != m.end(); ++it)
  {
    if( it->second < result ) result = it->second;
  }
  return result;
}

/*
checks if the coverage k is reached. If yes, the new node or tuple
must not be inserted into the segment tree. If no, this
funtion inserts the node or tuple into the segment tree

*/
bool checkInsert( NNSegTree &timeTree, NNSegTree::SegEntry &s, 
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
    s.coverage = coverage(rtree,s.end,s.nodeid,level);
  }
  timeTree.insert( s, k );
  return true;
}

/*
Some elements are needed to save between the knearestfilter calls. 

*/
typedef map<NNSegTree::SegEntry, TupleId>::const_iterator CIMAP;

struct KnearestFilterLocalInfo
{
  unsigned int k;
  //int max;
  bool scanFlag;
  Instant startTime, endTime;
  vector<FieldEntry> vectorA;
  vector<FieldEntry> vectorB;
  NNSegTree timeTree;
  Relation* relation;
  R_Tree<3, TupleId>* rtree;
  map< NNSegTree::SegEntry, TupleId> resultMap;
  CIMAP mapit;
  KnearestFilterLocalInfo( const Instant &s, const Instant &e) :
    startTime(s), endTime(e), timeTree( s, e )
    {}
};

/*
knearestFilterFun is the value function for the knearestfilter operator
It is a filter operator for the knearest operator. It can be called
if there exists a rtree for the unit attribute
The argument vector contains the following values:
args[0] = a rtree with the unit attribute as key
args[1] = the relation of the rtree
args[2] = mpoint
args[3] = int k, how many nearest are searched

*/
int knearestFilterFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  const int dim = 3;
  KnearestFilterLocalInfo *localInfo;

  switch (message)
  {
    case OPEN :
    {
      const MPoint *mp = (MPoint*)args[2].addr;
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      BBTree* t = new BBTree(*mp);

      localInfo = new KnearestFilterLocalInfo(
        up1->timeInterval.start, up2->timeInterval.end);
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
      Instant t1(localInfo->startTime.GetType());
      Instant t2(localInfo->startTime.GetType());
      t1.ReadFrom(tmp->BoundingBox().MinD(2));
      t2.ReadFrom(tmp->BoundingBox().MaxD(2));

      if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
      {
        localInfo->vectorA.push_back( FieldEntry( 
          localInfo->rtree->RootRecordId(), 0, t1, t2, 0)); 
        const BBox<2> xyBox = makexyBox( tmp->BoundingBox() );
        NNSegTree::SegEntry se(xyBox,t1, t2, 0.0, 0.0, 0,
              localInfo->rtree->RootRecordId(), -1);
        localInfo->timeTree.insert( se, localInfo->k );
      }
      delete tmp;
      while( !localInfo->vectorA.empty() )
      {
        unsigned int vpos;
        for( vpos = 0; vpos < localInfo->vectorA.size(); ++vpos)
        {
          FieldEntry &f = localInfo->vectorA[ vpos ];
          SmiRecordId adr = f.nodeid;
          // check if this entry is not deleted
          if( localInfo->timeTree.erase( f.start, f.end, f.nodeid,
            f.maxdist) )
          {
            R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetMyNode(
                     adr, false, 
                     localInfo->rtree->MinEntries( f.level ), 
                     localInfo->rtree->MaxEntries( f.level ) );
            Instant t1(localInfo->startTime.GetType());
            Instant t2(localInfo->startTime.GetType());
            for ( int ii = 0; ii < tmp->EntryCount(); ++ii )
            {
              if ( tmp->IsLeaf() )
              {
                R_TreeLeafEntry<dim, TupleId> e = 
                  (R_TreeLeafEntry<dim, TupleId>&)(*tmp)[ii];
                t1.ReadFrom((double)(e.box.MinD(2)));
                t2.ReadFrom((double)(e.box.MaxD(2)));
                if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
                {
                  const BBox<2> xyBox = makexyBox( e.box );
                  Interval<Instant> d(t1,t2,true,true);
                  const BBox<2> mBox(t->getBox(d));
                  NNSegTree::SegEntry se(xyBox,t1, t2, 
                        xyBox.Distance( mBox), 
                        maxDistance( xyBox, mBox), 1,
                        -1, e.info);
                  checkInsert( localInfo->timeTree, se, mBox, 
                    localInfo->k, localInfo->rtree, f.level+1);
                }
              }
              else
              {
                R_TreeInternalEntry<dim> e = 
                  (R_TreeInternalEntry<dim>&)(*tmp)[ii];
               t1.ReadFrom(e.box.MinD(2));
                t2.ReadFrom(e.box.MaxD(2));
               if( !(t1 >= localInfo->endTime || t2 <= localInfo->startTime))
               {
                  const BBox<2> xyBox = makexyBox( e.box );
                  Interval<Instant> d(t1,t2,true,true);
                  const BBox<2> mBox(t->getBox(d));
                  NNSegTree::SegEntry se(xyBox,t1, t2, 
                        xyBox.Distance( mBox), 
                        maxDistance( xyBox, mBox), 0,
                        e.pointer, -1);
                  if( checkInsert( localInfo->timeTree, se, 
                        mBox, localInfo->k, localInfo->rtree, f.level+1))
                  {
                   localInfo->vectorB.push_back( FieldEntry( 
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
     localInfo = (KnearestFilterLocalInfo*)local.addr;
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
      localInfo = (KnearestFilterLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }

  return 0;
}

/*
2.4 Definition of value mapping vectors

*/

ValueMapping distanceScanMap [] = { distanceScanFun<2>,
                                    distanceScanFun<3>,
                                    distanceScanFun<4>,
                                    distanceScanFun<8>};


/*
2.5 Specification of operators

*/

const string distanceScanSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_ _ distancescan [ _, _ ]</text--->"
      "<text>Uses the given rtree to find k tuples in the"
      " relation in order of their distance from the "
      " position T. The nearest tuple first.</text--->"
      "<text>query kinos_geoData Kinos distancescan "
      "[[const point value (10539.0 14412.0)], 5] consume; "
      "where kinos_geoData "
      "is e.g. created with 'let kinos_geoData = "
      "Kinos creatertree [geoData]'</text--->"
      ") )";

const string knearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ knearest [ _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which contains the k-nearest units to the given mpoint. "
      "The tuples are splitted into multiple tuples with disjoint "
      "units if necessary. The tuples in the result stream are "
      "not necessarily ordered by time or distance to the given "
      "mpoint. The operator expects that the input stream with "
      "the tuples are sorted by the time of the units</text--->"
      "<text>query query UnitTrains feed head[20] UTrip knearest "
      "[train1, 2] consume;</text--->"
      ") )";

const string knearestFilterSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ knearestfilter [ _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which are the k-nearest tupels to the given mpoint. "
      "The operator do not separate tupels if necessary. The "
      "result may have more than the k-nearest tupels. It is a "
      "filter operator for the knearest operator, if there are a great "
      "many input tupels. "
      "The operator expects a thee dimensional rtree where the "
      "third dimension is the time</text--->"
      "<text>query UTOrdered_RTree UTOrdered knearestfilter "
      "[train1, 5] count;</text--->"
      ") )";



/*
2.6 Definition of operators

*/
Operator distancescan (
         "distancescan",        // name
         distanceScanSpec,      // specification
         4,                     //number of overloaded functions
         distanceScanMap,       // value mapping
         distanceScanSelect,    // trivial selection function
         distanceScanTypeMap    // type mapping
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

Operator knearestfilter (
         "knearestfilter",        // name
         knearestFilterSpec,      // specification
         knearestFilterFun,       // value mapping
         Operator::SimpleSelect,  // trivial selection function
         knearestFilterTypeMap    // type mapping
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
       delete static_cast<BBTree*>(local.addr);
       local.addr = 0;
     }
     if(mp->IsDefined()){
        BBTree* t = new BBTree(*mp);
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
     BBTree* t = static_cast<BBTree*>(local.addr);
     result.addr = new Rectangle<2>(t->getBox(d));
     periods->DeleteIfAllowed();
     return YIELD;  
   }
   case CLOSE : {
     qp->Close(args[0].addr);
     if(local.addr){
       delete static_cast<BBTree*>(local.addr);
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
    AddOperator( &knearest );
    AddOperator( &knearestvector );
    AddOperator( &knearestfilter );
    AddOperator( &bboxes );
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

