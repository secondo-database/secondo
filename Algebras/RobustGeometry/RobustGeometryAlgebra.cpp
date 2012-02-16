/*

[1] RobustGeometryAlgebra

Februar 2012 Katja Koch


 Overview




 Defines and Includes

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
Algebra Implementation
Type Mapping Functions
These functions check whether the correct argument types are supplied for an
operator; if so, returns a list expression for the result type, otherwise the
symbol ~typeerror~.

*/

/*
An algebra module must provide for each type functions which take a nested
list and create an instance of the class representing the type and vice versa.
These functions are called In- and Out-functions. ->see SpatialAlgebra In/Out

*/

/*
This means that for a type constructor the functions create, delete,
close, and clone must be implemented. The remaining two functions open
and save may be implemented. If they are not implemented,
the default persistent storage mechanism is used. ->see SpatialAlgebra

*/

/*
Type investigation auxiliaries

Within this algebra module, we have to handle with values of different
types:  ~line~ and ~Points~.
->see SpatialAlgebra

*/

enum SpatialTypeRG {stpoint,stpoints,stline,stregion,stbox,sterror};

SpatialTypeRG
SpatialTypeOfSymbolRG( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == Point::BasicType()  ) return (stpoint);
    if ( s == Points::BasicType() ) return (stpoints);
    if ( s == Line::BasicType()   ) return (stline);
    if ( s == Region::BasicType() ) return (stregion);
    if ( s == Rectangle<2>::BasicType()   ) return (stbox);
  }
  return (sterror);
}

/*
Type mapping

*/

static ListExpr intersectionTM( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( SpatialTypeOfSymbolRG( arg1 ) == stline &&
         SpatialTypeOfSymbolRG( arg2 ) == stline )
         return (nl->SymbolAtom( Points::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

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

const string intersectionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>{line} x"
  "   {line} -> T, "
  " where T = points if any point or point type is one of the "
  " arguments or the argument having the smaller dimension </text--->"
  "<text>intersectionBO(arg1, arg2)</text--->"
  "<text>intersectionBO of two spatial objects</text--->"
  "<text>query intersectionBO(tiergarten, thecenter) </text--->"
  ") )";


ValueMapping intersectionVM [] =   {intersectionBO_ll};

/*
class MakeBO

*/

class MakeBo
{
public:
   MakeBo() {};
   ~MakeBo() {};

   void IntersectionBO(const Line& line1, const Line& line2,Points& result);
 };

/*
~IsRobustGeometryType~

This function checks whether the type given as a ListExpr is one of
~point~,  ~line~

*/
bool IsRobustGeometryType(ListExpr type)
{
   if(!nl->IsAtom(type))
   {
      return false;
   }
   if(nl->AtomType(type)!=SymbolType)
   {
      return false;
   }
   string t = nl->SymbolValue(type);
   if(t==Line::BasicType()) return true;
   return false;
}

/*
intersection-operator for two line-objects

*/
void MakeBo::IntersectionBO(const Line& line1,
		const Line& line2, Points& result)
{

	// Initialisation
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

	// Initialize event queue: all segment endpoints
	// Sort x by increasing x and y


	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q1;

	  priority_queue<avlseg::ExtendedHalfSegment,
	                 vector<avlseg::ExtendedHalfSegment>,
	                 greater<avlseg::ExtendedHalfSegment> > q2;

	avltree::AVLTree<avlseg::AVLSegment> sss;
	// Initialize sweep line sss empty
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
  	    if(leftN)
  	    {
  	    	tmpL = *leftN;
  	    	leftN = &tmpL;
  	    }
  	    if(rightN)
  	    {
  	    	tmpR = *rightN;
  	    	rightN = &tmpR;
  	    }
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

  	  // remove nextHs from
    }


  result.EndBulkLoad(true,false,false);
}

/*
~Intersection~ operation.

*/

static int intersectionBO_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);


   if (line1->IsDefined() && line2->IsDefined() )
   {
      if (line1->IsEmpty() || line2->IsEmpty() )
      {
          ((Points *)result.addr)->SetDefined( false );
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
            ((Points *)result.addr)->Clear();
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
     ((Points *)result.addr)->Clear();
     ((Points *)result.addr)->SetDefined( false );

     return (0);
   }
}

/*
Operator Description

*/

struct intersectionInfo : OperatorInfo
{
	intersectionInfo()
	{
		name = "intersectionBO";
		signature = Line::BasicType() + " x " + Line::BasicType()
		+ " -> " + Points::BasicType();
		syntax = "_ intersectionBO _";
		meaning = "Intersection predicate for two Lines.";
	}
};

/*
Selection function ~RGSetOpSelect~
This select function is used for the ~intersectionBO~ operator.

*/
int
RGSetOpSelect(ListExpr args)
{
  string a1 = nl->SymbolValue(nl->First(args));
  string a2 = nl->SymbolValue(nl->Second(args));

  if(a1==Point::BasicType())
  {
    if(a2==Point::BasicType())  return 0;
    if(a2==Points::BasicType()) return 1;
    if(a2==Line::BasicType())   return 2;
    if(a2==Region::BasicType()) return 3;
    return -1;
  }
  if(a1==Points::BasicType())
  {
    if(a2==Point::BasicType())  return 4;
    if(a2==Points::BasicType()) return 5;
    if(a2==Line::BasicType())   return 6;
    if(a2==Region::BasicType()) return 7;
    return -1;
  }
  if(a1==Line::BasicType())
  {
    if(a2==Point::BasicType())  return 8;
    if(a2==Points::BasicType()) return 9;
    if(a2==Line::BasicType())   return 10;
    if(a2==Region::BasicType()) return 11;
    return -1;
  }

  if(a1==Region::BasicType())
  {
    if(a2==Point::BasicType())  return 12;
    if(a2==Points::BasicType()) return 13;
    if(a2==Line::BasicType())   return 14;
    if(a2==Region::BasicType()) return 15;
    return -1;
  }
  return -1;
}

/*
10.5.3 Definition of the operators

*/
Operator test
( "intersectionBO", intersectionSpec,16,
intersectionVM,RGSetOpSelect,intersectionTM );

/*
Creating the Algebra

*/

class RobustGeometryAlgebra : public Algebra
{

	public:
	RobustGeometryAlgebra() : Algebra()
	{
		AddOperator( &test );
	}
	~RobustGeometryAlgebra() {};
};

/*
Algebra Initialization

*/

extern "C"
Algebra*
InitializeRobustGeometryAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RobustGeometryAlgebra());
}

