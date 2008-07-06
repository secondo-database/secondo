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

*/

ListExpr
distanceScanStartTypeMap( ListExpr args )
{	
  NList type(args);
  if ( type != NList(XRECTANGLE, XRECTANGLE) ) {
    return NList::typeError("Expecting two rectangles");
  }  

  return NList(BOOL).listExpr();
}


ListExpr
distanceScanNextTypeMap( ListExpr args )
{
  NList type(args);
  const string errMsg = "Expecting two rectangles "
	                "or a point and a rectangle";

  // first alternative: xpoint x xrectangle -> bool
  if ( type == NList(XPOINT, XRECTANGLE) ) {
    return NList(BOOL).listExpr();
  }  
  
  // second alternative: xrectangle x xrectangle -> bool
  if ( type == NList(XRECTANGLE, XRECTANGLE) ) {
    return NList(BOOL).listExpr();
  }  
  
  return NList::typeError(errMsg);
}


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
  ListExpr position = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);
  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) &&
	  (nl->AtomType(quantity) == SymbolType) &&
  (nl->SymbolValue(quantity) == "int"),
  "Operator distanceScan expects a fourth argument of type integer (k or -1).\n"
  "Operator distanceScan gets '" + argstr + "'.");


  /* Query window: find out type of key */
  CHECK_COND(nl->IsAtom(position) &&
    nl->AtomType(position) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", position, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", position, errorInfo)),
    "Operator distanceScan expects that the position\n"
    "is of TYPE SPATIAL2D or SPATIAL3D.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
    !nl->IsAtom(rtreeDescription) &&
    nl->ListLength(rtreeDescription) == 4,
    "Operator distanceScan expects a R-Tree with structure "
    "(rtree||rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a R-Tree list with structure '"
    +rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
    nl->AtomType(rtreeKeyType) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)),
   "Operator distanceScan expects a R-Tree with key type\n"
   "of kind SPATIAL2D or SPATIAL3D.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
    (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
     nl->SymbolValue(rtreeSymbol) == "rtree3") ,
   "Operator distanceScan expects a R-Tree \n"
   "of type rtree or rtree3.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator distanceScan expects a R-Tree with structure "
    "(rtree||rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator distanceScan expects a R-Tree with structure "
    "(rtree||rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
    nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator distanceScan expects a R-Tree with structure "
   "(rtree||rtree3 (tuple ((a1 t1)...(an tn))) attrtype "
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
    "Operator distanceScan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
    nl->AtomType(tupleSymbol) == SymbolType &&
    nl->SymbolValue(tupleSymbol) == "tuple" &&
    IsTupleDescription(attrList),
    "Operator distanceScan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  /* check that rtree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList),
   "Operator distanceScan: The tuple type of the R-tree\n"
   "differs from the tuple type of the relation.");

  string attrTypeRtree_str, attrTypeWindow_str;
  nl->WriteToString (attrTypeRtree_str, rtreeKeyType);
  nl->WriteToString (attrTypeWindow_str, position);

  CHECK_COND(
    ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL2D", position, errorInfo) ) ||
    ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL3D", position, errorInfo) ) ,
    "Operator distanceScan expects joining attributes of same "
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
  ListExpr position = nl->Third(args),
           errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  if (algMgr->CheckKind("SPATIAL2D", position, errorInfo))
    return 0;
  else if (algMgr->CheckKind("SPATIAL3D", position, errorInfo))
    return 1;

  return -1; /* should not happen */
}  


/*
5.3 Value Mapping Functions

For any operator a value mapping function must be defined. It contains
the code which gives an operator its functionality

5.3.1 The ~distanceScanStart~ prepares the scan

*/
int
distanceScanStartFun (Word* args, Word& result, int message, 
              Word& local, Supplier s)
{

/*
----
  XRectangle *r1 = static_cast<XRectangle*>( args[0].addr );
  XRectangle *r2 = static_cast<XRectangle*>( args[1].addr );

  result = qp->ResultStorage(s);  
                                query processor has provided
                                a CcBool instance for the result

  CcBool* b = static_cast<CcBool*>( result.addr );
  b->Set(true, r1->intersects(*r2));
                               the first argument says the boolean
                               value is defined, the second is the
                               real boolean value
----

*/

  return 0;
}

/*
4.3.2 The ~distanceScanNext~ gives always the next point after a 
distanceSanStart.
It can be used if only some values of the input set are needed, not all

*/
int
distanceScanNextFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{

/*
----
  cout << "insideFun_PR" << endl;
  XPoint* p = static_cast<XPoint*>( args[0].addr );
  XRectangle* r = static_cast<XRectangle*>( args[1].addr );

  result = qp->ResultStorage(s);   
                                query processor has provided
                                a CcBool instance for the result

  CcBool* b = static_cast<CcBool*>( result.addr );
  
  bool res = ( p->GetX() >= r->GetXLeft() 
            && p->GetX() <= r->GetXRight()
            && p->GetY() >= r->GetYBottom() 
            && p->GetY() <= r->GetYTop() );

  b->Set(true, res); the first argument says the boolean
                     value is defined, the second is the
                     real boolean value)
----

*/

  return 0;
}

/*
4.3.3 The ~distanceScan~ results a stream of all input tuples in the 
right order. It has the functionality of distanceScanStart and distanceSanNext
together and can be used if all input tuples are needed in a result set
ordered by distance to a given reference point 

*/
template <unsigned dim, class LeafInfo>
R_TreeNode<dim, LeafInfo> *R_Tree<dim, LeafInfo>::GetPubNode(
    const SmiRecordId recno, 
    const bool leaf,
    const int min, 
    const int max )
{
  assert( file.IsOpen() );
  R_TreeNode<dim, LeafInfo> *node = 
      new R_TreeNode<dim, LeafInfo>( leaf, min, max );
  node->Read( file, recno );
  return node;
}

template <unsigned dim>
struct DistanceScanLocalInfo
{
  Relation* relation;
  R_Tree<dim, TupleId>* rtree;
  BBox<dim> position;
  int quantity, noFound;
  bool first;
  bool scanFlag;
  int min, max;
  NNpriority_queue* qp;
};


template <unsigned dim>
int distanceScanFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  DistanceScanLocalInfo<dim> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new DistanceScanLocalInfo<dim>;
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      Point pos = *(Point*)args[2].addr;
      localInfo->position = pos.BoundingBox();
      localInfo->quantity = ((CcInt*)args[3].addr)->GetIntval();
      localInfo->noFound = 0;
      localInfo->first = true;
      localInfo->scanFlag = true;
      localInfo->qp = new NNpriority_queue;

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);
      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (DistanceScanLocalInfo<dim>*)local.addr;
      if ( (localInfo->quantity > 0 && localInfo->noFound >=localInfo->quantity)
          || (localInfo->noFound >= localInfo->rtree->EntryCount()))
      {
        return CANCEL;
      }

      if(localInfo->first)
      {
        localInfo->first = false;
        localInfo->min = localInfo->rtree->MinEntries(0);
        localInfo->max = localInfo->rtree->MaxEntries(0);
        long nodeId = localInfo->rtree->RootRecordId();
        R_TreeNode<dim, TupleId> *tmp = localInfo->rtree->GetPubNode( nodeId, 
        false, localInfo->min, localInfo->max );
        localInfo->qp->push( DistanceElement( nodeId, tmp->BoundingBox(), 
                false, -1, tmp->BoundingBox().Distance(localInfo->position)));
        delete tmp;
      }
      while ( !localInfo->qp->empty() )
      {
        DistanceElement elem = localInfo->qp->top();
        localInfo->qp->pop();
        if ( elem.IsLeaf() )
        {
          Tuple *tuple = localInfo->relation->GetTuple(elem.TupleId());
          result = SetWord(tuple);
          ++localInfo->noFound;
          return YIELD;
        }
        else
        {
          R_TreeNode<2, TupleId> *tmp = localInfo->rtree->GetPubNode( 
                elem.NodeId(), elem.IsLeaf(), localInfo->min, localInfo->max );
          cout << "Read Node " << elem.NodeId() << endl;
          for ( int ii = 0; ii < tmp->EntryCount(); ++ii )
          {
            if ( tmp->IsLeaf() )
            {
              R_TreeLeafEntry<dim, TupleId> e = 
                (R_TreeLeafEntry<dim, TupleId>&)(*tmp)[ii];

              localInfo->qp->push( DistanceElement( 0, e.box, 
                  true, e.info, e.box.Distance(localInfo->position)));
            }
            else
            {
              R_TreeInternalEntry<dim> e = 
                (R_TreeInternalEntry<dim>&)(*tmp)[ii];
              localInfo->qp->push( DistanceElement( e.pointer, e.box, 
                  false, -1, e.box.Distance(localInfo->position)));
            }
          }
          delete tmp;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      localInfo = (DistanceScanLocalInfo<dim>*)local.addr;
      delete localInfo->qp;
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
                                             distanceScanFun<2> };


/*

4.4 Operator Descriptions

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/
struct distanceScanStartInfo : OperatorInfo {

  distanceScanStartInfo()
  {
    name      = "distancescanstart";
    signature = XRECTANGLE + " x " + XRECTANGLE + " -> " + BOOL;
    syntax    = "_" + INTERSECTS + "_";
    meaning   = "Intersection predicate for two xrectangles.";
  }

}; // Don't forget the semicolon here. Otherwise the compiler 
   // returns strange error messages


struct distanceScanNextInfo : OperatorInfo {

  distanceScanNextInfo()
  {
    name      = "distancescannext"; 
    signature = XPOINT + " x " + XRECTANGLE + " -> " + BOOL;
    syntax    = "_" + INSIDE + "_";
    meaning   = "Inside predicate.";
  }
};  


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
      "<text>query citiesInd cities windowintersects"
      " [r] consume; where citiesInd "
      "is e.g. created with 'let citiesInd = "
      "cities creatertree [pos]'</text--->"
      ") )";



/*
5.1.6 Definition of operators

*/
Operator distancescan (
         "distancescan",        // name
         distanceScanSpec,      // specification
         2,                         //number of overloaded functions
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
// AddOperator( distanceScanStartInfo(), 
//        distanceScanStartFun, distanceScanStartTypeMap );
// AddOperator( distanceScanNextInfo(), distanceScanNextFun, 
//        distanceScanNextTypeMap );
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

