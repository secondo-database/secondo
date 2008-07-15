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

June 2008, A. Braese

[toc]

0 Overview

This example algebra provides a distance-scan for a point set
in a R-Tree. A new datatype is not given but there are some operators:

  1. ~distanceScanStart~, which build the needed data structure for the scan

  2. ~distanceScanNext~, which gives the next nearest point.

  3. ~distanceScan~, which gives a stream of all input tuples in the right order

1 Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "NearestNeighborAlgebra.h"

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

/*
5 Creating Operators

5.1 Type Mapping Functions

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. Again we use interface ~NList.h~ for
manipulating list expressions. For every operator is one type mapping 
function given.

The function distanceScanCommonTypeMap is the type map for distancescan
mode: 1), distancescanstart (mode: 2), distancescannext (mode: 3),
distancescanlast (mode: 4)

*/
ListExpr
distanceScanTypeMap( ListExpr args )
{	
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator windowintersects.";
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
  CHECK_COND((nl->IsAtom(quantity)) &&
	  (nl->AtomType(quantity) == SymbolType) &&
  (nl->SymbolValue(quantity) == "int"),
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
5.2 Selection Function

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
5.3 Value Mapping Functions

For any operator a value mapping function must be defined. It contains
the code which gives an operator its functionality

The struct DistanceScanLocalInfo is needet to save the data
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
//  NNpriority_queue* pq;
};



/*
5.3.1 The ~distanceScan~ results a stream of all input tuples in the 
right order. It has the functionality of distanceScanStart and distanceSanNext
together and can be used if all input tuples are needed in a result set
ordered by distance to a given reference point 

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
5.1.4 Definition of value mapping vectors

*/
ValueMapping distanceScanMap [] = { distanceScanFun<2>,
                                    distanceScanFun<3>,
                                    distanceScanFun<4>,
                                    distanceScanFun<8>};


/*
5.1.5 Specification of operators

*/

const string distanceScanSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3}</text--->"
      "<text>_ _ distancescan [ _, _ ]</text--->"
      "<text>Uses the given rtree to find all k tuples"
      " in the given relation in order of their distance from the "
      " position T. The nearest tuple first.</text--->"
      "<text>query kinos_geoData Kinos distancescan "
      "[[const point value (10539.0 14412.0)], 5] consume; "
      "where kinos_geoData "
      "is e.g. created with 'let kinos_geoData = "
      "Kinos creatertree [geoData]'</text--->"
      ") )";




/*
5.1.6 Definition of operators

*/
Operator distancescan (
         "distancescan",        // name
         distanceScanSpec,      // specification
         4,                         //number of overloaded functions
         distanceScanMap,  // value mapping
         distanceScanSelect, // trivial selection function
         distanceScanTypeMap    // type mapping
);



/*
5 Implementation of the Algebra Class

*/

class NearestNeighborAlgebra : public Algebra
{
 public:
  NearestNeighborAlgebra() : Algebra()
  {


/*   
5.3 Registration of Operators

*/
    AddOperator( &distancescan );
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
7 Examples and Tests

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

