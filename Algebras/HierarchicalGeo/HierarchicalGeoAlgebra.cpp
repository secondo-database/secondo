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

[1] Implementation of Module

April 2007 Sascha Vaut

[TOC]

1 Overview

Up to now, this file contains the implementation of the type constructors 
~uncertain~ and ~cpoint~. The memory data structures used for these 
type constructors are implemented in the HierarchicalGeoAlgebra.h file.

2 Defines, includes, and constants

*/
#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include <vector>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "PolySolver.h"
#include "RelationAlgebra.h"
#include <math.h>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "HierarchicalGeoAlgebra.h"



/*
2.1 Definition of some constants





3 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
uncertain basic-types: ~cbool~, ~cint~, ~creal~, ~cpoint~. 

Later on we will examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~UncertainBaseType~, containing the four types, and
 a function, ~UncertainBaseTypeOfSymbol~, taking a nested list as argument
 and returning the corresponding ~UncertainBaseType~ type name.


Some functions of template class ~uncertain~

*/
template <class Alpha>
void Uncertain<Alpha>::Epsylon(CcReal& result)
{
  result = (CcReal)epsylon;
}

/*
Type Constructor ~CPoint~

Type ~cpoint~ represents an (epsylon, (x, y))-pair.

List Representation

The list representation of a ~cpoint~ is

----    ( epsylon ( x y ) )
----

For example:

----    ( 20.5 ( 329.456 22.289 ) )
----

Function describing the signature of the Type Constructor

*/
ListExpr CPointProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(
                  nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List"),
                  nl->StringAtom("Remarks")),
            nl->FiveElemList(
                  nl->StringAtom("-> UNCERTAIN"),
                  nl->StringAtom("cpoint"),
                  nl->StringAtom("(<epsylon>(<x> <y>))"),
                  nl->StringAtom("( 20.5 ( 329.456 22.289) )"),
                  nl->StringAtom(" All 3 values must be of type real." ))));
}

/*
Kind Checking Function

*/

bool CheckCPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cpoint"));
}

/*
~Out~-function

*/
ListExpr OutCPoint( ListExpr typeInfo, Word value )
{
  CPoint* cpoint = (CPoint*)(value.addr);
  
  if ( !cpoint->IsDefined() )
    return (nl->SymbolAtom("undef"));
  else
    {
      ListExpr coordinates = nl->TwoElemList(
            nl->RealAtom( cpoint->value.GetX() ),
            nl->RealAtom( cpoint->value.GetY() ));
                  
      return nl->TwoElemList(
            nl->RealAtom( cpoint->GetEpsylon() ),
            coordinates );
    }
}

/*
~In~-function

*/

Word InCPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )    
  // 2 arguments are necessary: epsylon and a point
  {
    ListExpr first = nl->First( instance );               // the epsylon value
    ListExpr second = nl->Second( instance );    // the point representation
    
    if ( nl->IsAtom( first ) && nl->AtomType( first ) == RealType )
    {
      if ( nl->ListLength( second ) == 2 &&
            nl->IsAtom( nl->First( second )) &&
            nl->AtomType( nl->First( second )) == RealType &&
            nl->IsAtom( nl->Second( second )) &&
            nl->AtomType( nl->Second( second )) == RealType)
      // if the second list element contains two real-values, representing 
      // point-coordinates
      { 
        correct = true;
        Point *p = (Point *)InPoint( nl->TheEmptyList(), second, 
                                        errorPos, errorInfo, correct ).addr;
        if ( !correct )
        {
          errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete p;
          return SetWord( Address(0) );
        }
        
        CPoint cpoint( nl->RealValue( first ), (StandardAttribute*) p );
        delete p;
        correct = cpoint.IsValid();
      }
      // the other possibility for the second argument: a name 
      // for a point-object
      else if ( nl->ListLength( second ) == 1 &&
                    nl->IsAtom( second ) &&
                    nl->AtomType( second == RealType ) )
      {
        correct = true;
        Point *p = (Point *)InPoint( nl->TheEmptyList(), second,
                                        errorPos, errorInfo, correct ).addr;
        if ( !correct )
        {
          errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete p;
          return SetWord( Address(0) );
        }
      }
      else
      {
        correct = false;
        errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
      }
    }
    else
    {
      correct = false;
      errmsg = "InCPoint(): Error in first instant. First instant must be an "
            "atomic value of type Real.";
      errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
    }
  }
  errmsg = "InCPoint(): List must contain 2 elements. ";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  return SetWord( Address(0) );
}


/*
~Create~-function

*/
Word CreateCPoint( const ListExpr typeInfo )
{
  return (SetWord( new CPoint() ));
}


/*
~Delete~-function

*/
void DeleteCPoint( const ListExpr typeInfo, Word& w )
{
  delete (CPoint *)w.addr;
  w.addr = 0;
}


/*
~Close~-function

*/
void CloseCPoint( const ListExpr typeInfo, Word& w )
{
  delete (CPoint *)w.addr;
  w.addr = 0;
}


/*
~Clone~-function

*/
Word CloneCPoint( const ListExpr typeInfo, const Word& w )
{
  CPoint *cpoint = (CPoint *)w.addr;
  return SetWord( new CPoint( *cpoint ) );
}


/*
~Sizeof~-function

*/
int SizeOfCPoint()
{
  return sizeof(CPoint);
}


/*
~Cast~-function

*/

void * CastCPoint(void* addr)
{
  return new (addr) CPoint;
}

/*
Creation of the type constructor ~cpoint~

*/

TypeConstructor uncertainpoint(
        "cpoint",                //name
        CPointProperty,     //property function describing signature
        OutCPoint,
        InCPoint,               //Out and In functions
        0,
        0,                         //SaveToList and RestoreFromList functions
        CreateCPoint,
        DeleteCPoint,        //object creation and deletion
        0,
        0,                         // object open and save
        CloseCPoint,   
        CloneCPoint,         //object close and clone
        CastCPoint,           //cast function
        SizeOfCPoint,       //sizeof function
        CheckCPoint );      //kind checking function


/*
Type Constructor +++++ hier weitere Typkonstruktoren anfuegen +++++


Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.


Type mapping function ~UncertainTypeMapReal~

This type mapping function is used for the Operation ~Epsylon()~.

*/
ListExpr UncertainTypeMapReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    if ( nl->IsEqual ( args, "cpoint" ) )
      return nl->SymbolAtom("real");
  }
  return nl->SymbolAtom("typeerror");
}

/*
Value mapping functions




Definition fo operators

Definition of operators is done in a way similar to definition of 
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first to define an 
array of value mapping functions for each operator. For nonoverloaded
operators there is also such an array defined, so it is easier to make them
overloaded.

ValueMapping arrays

*/

ValueMapping uncertainepsylonmap[] = { 
                                      UncertainEpsylon<Point> };

/*
Specification strings

*/

const string UncertainSpecEpsylon  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> epsylon</text--->"
  "<text>epsylon ( _ )</text--->"
  "<text>Returns an uncertain values' epsylon value.</text--->"
  "<text>epsylon ( i1 )</text--->"
  ") )";


const string UncertainSpecVal =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> x</text--->"
  "<text>val ( _ )</text--->"
  "<text>Returns an uncertain value's value.</text--->"
  "<text>val ( i1 )</text--->"
  ") )";



/*
Operators

*/

Operator uncertainepsylon( "epsylon",
                              UncertainSpecEpsylon,
                              UncertainEpsylon<Point>,
                              Operator::SimpleSelect,
                              UncertainTypeMapReal );

/*
Creating the Algebra
 
*/
class HierarchicalGeoAlgebra : public Algebra
{
  public:
  HierarchicalGeoAlgebra() : Algebra()
  {
    AddTypeConstructor( &uncertainpoint );
    uncertainpoint.AssociateKind( "UNCERTAIN" );
    uncertainpoint.AssociateKind( "SPATIAL" );
    AddOperator( &uncertainepsylon );
  }
  ~HierarchicalGeoAlgebra() {};
};
HierarchicalGeoAlgebra hierarchicalGeoAlgebra;

/*
Initialization

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
InitializeHierarchicalGeoAlgebra( NestedList* nlRef, 
                                    QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&hierarchicalGeoAlgebra);
}

