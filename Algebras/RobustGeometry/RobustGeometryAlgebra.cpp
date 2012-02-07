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
#include "AVLSegment.h"

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
types:  ~line~ and ~Points~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~RobustGeometryType~, containing the types, and a function,
~SpatialTypeOfSymbol~, taking a nested list as argument and returning the
corresponding ~RobustGeometryType~ type name.

*/
enum RobustGeometryType { rgpoints, rgline, srbox, srerror};

RobustGeometryType RobustGeometryOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == Points::BasicType()  ) return (rgpoints);
    if ( s == Line::BasicType()   ) return (rgline);
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
	PriorityQueue


*/


/*
	class MakeOp


*/

class MakeBo
{
public:
   MakeBo() {};
   ~MakeBo() {};

   void IntersectionBO(const Line& reg1, const Line& reg2i,Points& result);
   bool Intersects(const Line& line1, const Line& line2);
   bool P_Intersects(const Line& line1, const Line& line2);
};

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
intersection-operator for two line-objects

*/
void MakeBo::IntersectionBO(const Line& line1,
		const Line& line2, Points& result)
{
    //Initialisation
	result.Clear();

	if(!line1.IsDefined() || !line2.IsDefined())
	{
		result.SetDefined(false);
		return;
	}

	result.SetDefined(true);
	if(line1.Size()==0 || line2.Size()==0)
	{
		return; // empty line
	}
	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q1;

	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q2;

	avltree::AVLTree<avlseg::AVLSegment> sss;
	avlseg::ownertype owner;
	int pos1 = 0;
	int pos2 = 0;
	int src = 0;
	avlseg::ExtendedHalfSegment nextHs;

  	const avlseg::AVLSegment* member=0;
  	const avlseg::AVLSegment* leftN = 0;
 	const avlseg::AVLSegment* rightN = 0;

  	avlseg::AVLSegment left1,right1,left2,right2;
  	avlseg::AVLSegment tmpL,tmpR;

  	result.StartBulkLoad();

  	while( (owner=selectNext(line1,pos1,line2,pos2,
  			q1,q2,nextHs,src))!=avlseg::none)

  	{
  	     avlseg::AVLSegment current(nextHs,owner);
  	     member = sss.getMember(current,leftN,rightN);

       if(nextHs.IsLeftDomPoint())
       {

       }

 /*      else if (nextHs.IsRightDomPoint())
       {


       }
       else
       {

       }

       */
    }


  result.EndBulkLoad(true,false,false);
}

/*
~Intersection~ operation.

*/

/*
line- line

*/
static int intersectionBO_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{
//args immer nur 2 Linien betrachtet, != BO???
   result = qp->ResultStorage( s );
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);


   if (line1->IsDefined() && line2->IsDefined() )
   {
      if (line1->IsEmpty() || line2->IsEmpty() )
      {
          ((Line *)result.addr)->SetDefined( false );
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() &&
    		  line2->BoundingBox().IsDefined() )
      {
         if (line1->BoundingBox().Intersects(line2->BoundingBox()))
         {
            MakeBo bo;
            bo.IntersectionBO( *line1, *line2,
            		*static_cast<Points*>(result.addr));
            return(0);
         }
         else
         {
            ((Line *)result.addr)->Clear();
            return (0);
         }
      }
      else
      {
    	 MakeBo bo;
         bo.IntersectionBO( *line1, *line2,*static_cast<Points*>(result.addr));
         return(0);
      }
   }
   else
   {
     ((Line *)result.addr)->Clear();
     ((Line *)result.addr)->SetDefined( false );

     return (0);
   }
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
