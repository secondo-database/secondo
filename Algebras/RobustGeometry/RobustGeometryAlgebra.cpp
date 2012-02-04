/*

[1] RobustGeometryAlgebra

Februar 2012 Katja Koch


1 Overview




2 Defines and Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "GenericTC.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoConfig.h"
#include "AvlTree.h"
#include "AVLSegment.h"
#include "AlmostEqual.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "RegionTools.h"
#include "Symbols.h"
#include "NList.h"
#include "LogMsg.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "SpatialAlgebra.h"
#include "RobustGeometryAlgebra.h"

#include <vector>
#include <queue>
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <string>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <queue>
#include <iterator>
#include <sstream>
#include <limits>
#include <errno.h>
#include <cerrno>


#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;

/*

2 Algebra Implementation

2.2 Type Mapping Functions

These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.

*/


/*
3 Type investigation auxiliaries

Within this algebra module, we have to handle with values of different
types:  ~line~ and ~region~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~RobustGeometryType~, containing the types, and a function,
~SpatialTypeOfSymbol~, taking a nested list as argument and returning the
corresponding ~RobustGeometryType~ type name.

*/
enum RobustGeometryType { srpoint, srline, srbox, srerror};

RobustGeometryType RobustGeometryOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == Point::BasicType()  ) return (srpoint);
    if ( s == Line::BasicType()   ) return (srline);
  }
  return (srerror);
}


/*
10 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

10.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

10.1.1 Type mapping function RobustGeometryTypeMapBool

It is for the compare operators which have ~bool~ as resulttype, like =, !=, <,
<=, >, >=.

*/

ListExpr RobustGeometryIntersectionBOTypeMap(ListExpr args)
{
  string err = "t1 x t2 expected, t_i in {line";
  if(nl->ListLength(args)!=2)
  {
    return listutils::typeError(err + ": wrong number of arguments");
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(!listutils::isSymbol(arg1))
  {
    return listutils::typeError(err+ ": first arg not a RobustGeometry type");
  }
  if(!listutils::isSymbol(arg2))
  {
    return listutils::typeError(err+ ": second arg not a RobustGeometry type");
  }
  string a1 = nl->SymbolValue(arg1);
  string a2 = nl->SymbolValue(arg2);

  if(a1==Line::BasicType())
  {
    if(a2==Line::BasicType())   return nl->SymbolAtom(Line::BasicType());
    return listutils::typeError(err+ ": second arg not a RobustGeometry type");
  }

  return listutils::typeError(err+ ": first arg not a RobustGeometry type");

}

/*
~IsRobustGeometryType~

This function checks whether the type given as a ListExpr is one of
~point~,  ~line~

*/

bool IsRobustGeometryType(ListExpr type){
   if(!nl->IsAtom(type)){
      return false;
   }
   if(nl->AtomType(type)!=SymbolType){
      return false;
   }
   string t = nl->SymbolValue(type);
   if(t==Line::BasicType()) return true;
   return false;
}


/*
Value Mapping for ~fromLine~

*/


int fromLineVM1(Word* args, Word& result, int message,
            Word& local, Supplier s){
   result = qp->ResultStorage(s);
   Line* line = static_cast<Line*>(args[0].addr);
   SimpleLine* res = static_cast<SimpleLine*>(result.addr);
   res->fromLine(*line);
   return 0;
}

int fromLineVM2(Word* args, Word& result, int message,
                Word& local, Supplier s){
  result = qp->ResultStorage(s);
  Line* line = static_cast<Line*>(args[0].addr);
  CcBool* smaller = static_cast<CcBool*> (args[1].addr);
  SimpleLine* res = static_cast<SimpleLine*>(result.addr);
  res->fromLine(*line,smaller->GetBoolval());
  return 0;
}

ValueMapping fromLineVM[] = {
  fromLineVM1,
  fromLineVM2
};
/*

*/
static int intersectionBO_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{
return 0;
}

ValueMapping RobustGeometryintersectionBOVM [] =   {intersectionBO_ll};

int RobustGeometrySetOpSelect(ListExpr args)
{
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if(a1==Line::BasicType())
  {
    if(a2==Line::BasicType())   return 1;
    return -1;
  }

  return -1;
}

const string RobustGeometryIntersectionBOSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{line} x"
  "   {line} -> T, "
  " where T = points if any point or point type is one of the "
  " arguments or the argument having the smaller dimension </text--->"
  "<text>intersectionBO(arg1, arg2)</text--->"
  "<text>intersectionBO of two spatial objects</text--->"
  "<text>query intersectionBO(tiergarten, thecenter) </text--->"
  ") )";



/*
10.5.3 Definition of the operators

*/

Operator RobustGeometryIntersectionBO (
  "intersectionBO",
  RobustGeometryIntersectionBOSpec,
  1,
  RobustGeometryintersectionBOVM,
  RobustGeometrySetOpSelect,
  RobustGeometryIntersectionBOTypeMap );


/*
11 Creating the Algebra


*/

class RobustGeometryAlgebra : public Algebra
{

 public:

	RobustGeometryAlgebra() : Algebra()
  {
	 AddOperator(&RobustGeometryIntersectionBO);
  }
  ~RobustGeometryAlgebra() {};
};

/*
12 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeRobustGeometryAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RobustGeometryAlgebra());
}
