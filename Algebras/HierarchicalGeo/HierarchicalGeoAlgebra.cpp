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

September - November 2007 Sascha Vaut

[TOC]

1 Overview

Up to now, this file contains the implementation of the type constructors 
~uncertain, cpoint, cupoint~ and ~cmpoint~. The memory data structures used for
these type constructors are implemented in the HierarchicalGeoAlgebra.h file.

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
#include "TemporalAlgebra.h"
#include "TypeMapUtils.h"
#include <math.h>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "HierarchicalGeoAlgebra.h"



/*
2.1 Definition of some constants


3 Implementation of C++ Classes


3.1 Class ~CUPoint~

*/

/*
3.2 Class ~CMPoint~

*/
void CMPoint::Clear()
{
  Mapping<CUPoint, Point>::Clear(); // call super
  bbox.SetDefined(false);          // invalidate bbox
}

void CMPoint::Add( const CUPoint& unit )
{
//   cout << "CALLED: MPoint::Add" << endl;
  assert( unit.IsValid() );
  assert( unit.IsDefined() );
  units.Append( unit );
  if(units.Size() == 1)
  {
//     cout << "        MPoint::Add FIRST ADD" << endl;
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox = unit.BoundingBox();
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  } else {
//     cout << "\t Old bbox = "; bbox.Print(cout);
    bbox = bbox.Union(unit.BoundingBox());
//     cout << "\n\t New bbox = "; bbox.Print(cout);
  }
  RestoreEpsilon();
  RestoreBoundingBox(false);
}

void CMPoint::MergeAdd(const CUPoint& unit){
  assert( unit.IsValid() );
  assert( unit.IsDefined() );

  int size = GetNoComponents();
  if(size==0){ // the first unit
     Add(unit);
     bbox = unit.BoundingBox();
     return;
  }
  const CUPoint* last;
  Get(size-1,last);
  if(last->timeInterval.end!=unit.timeInterval.start ||
     !( (last->timeInterval.rc )  ^ (unit.timeInterval.lc))){
     // intervals are not connected
     Add(unit);
     bbox = bbox.Union(unit.BoundingBox());
     return;
  }
  if(!AlmostEqual(last->p1, unit.p0)){
    // jump in spatial dimension
    Add(unit);
    bbox = bbox.Union(unit.BoundingBox());
    return;
  }
  // define the epsilon-value of the two merged uncertain units:
  double e;
  if (unit.epsilon > last->epsilon)
    e = unit.epsilon;
  else
    e = last->epsilon;
  
  Interval<Instant> complete(last->timeInterval.start,
                             unit.timeInterval.end,
                             last->timeInterval.lc,
                             unit.timeInterval.rc);
  CUPoint cupoint(e, complete,last->p0, unit.p1);
  delete &e;
  Point p;
  cupoint.TemporalFunction(last->timeInterval.end, p, true);
  if(!AlmostEqual(p,last->p0)){
     Add(unit);
     bbox = bbox.Union(unit.BoundingBox());
     return;
  }
  assert( cupoint.IsValid() );
  assert( cupoint.IsDefined() );
  units.Put(size-1,cupoint); // overwrite the last unit by a connected one
}


void CMPoint::Restrict( const vector< pair<int, int> >& intervals )
{
  units.Restrict( intervals ); // call super
  bbox.SetDefined(false);      // invalidate bbox
  RestoreBoundingBox();        // recalculate it
}

ostream& CMPoint::Print( ostream &os ) const
{
  if( !IsDefined() )
  {
    return os << "(CMPoint: undefined)";
  }
  os << "(CMPoint: defined, MBR = ";
  bbox.Print(os);
  os << ", contains " << GetNoComponents() << " units: ";
  for(int i=0; i<GetNoComponents(); i++)
  {
    const CUPoint *unit;
    Get( i , unit );
    os << "\n\t";
    unit->Print(os);
  }
  os << "\n)" << endl;
  return os;
}

void CMPoint::EndBulkLoad(const bool sort)
{
  Mapping<CUPoint, Point>::EndBulkLoad( sort ); // call super
  RestoreBoundingBox();                        // recalculate, if necessary
}

/*
RestoreEpsilon() checks, wether the CMPoints ~epsilon~ value equals the maximum
epsilon value of all contained uncertain units, and resets it if needed.

*/
void CMPoint::RestoreEpsilon()
{
  if(!IsDefined() || GetNoComponents() == 0)
  {  // If the cmpoint isn't defined or has no components, the epsilon-value
      // is set to undefined.
    UncertainSetDefined(false);
  }
  else
  { // Determine the maximum value of the epsilon-values of all units.
    const CUPoint *unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        epsilon = unit->GetEpsilon();
        UncertainSetDefined( true );
        isfirst = false;
      }
      else if( epsilon < unit->GetEpsilon() )
      {
        epsilon = unit->GetEpsilon();
      }
      // else: there's nothing to do.
    }
  } 
}


/*
RestoreBoundingBox() checks, whether the MPoint's MBR ~bbox~ is ~undefined~
and thus may needs to be recalculated and if, does so.

*/

void CMPoint::RestoreBoundingBox(const bool force)
{
  if(!IsDefined() || GetNoComponents() == 0)
  { // invalidate bbox
    bbox.SetDefined(false);
  }
  else if(force || !bbox.IsDefined())
  { // construct bbox
    const CUPoint *unit;
    int size = GetNoComponents();
    bool isfirst = true;
    for( int i = 0; i < size; i++ )
    {
      Get( i, unit );
      if (isfirst)
      {
        bbox = unit->BoundingBox();
        isfirst = false;
      }
      else
      {
        bbox = bbox.Union(unit->BoundingBox());
      }
    }
  } // else: bbox unchanged and still correct
}

// Class functions
Rectangle<3u> CMPoint::BoundingBox() const
{
  return bbox;
}



/*
4 Type Constructors

4.1 Type Constructor ~CPoint~

Type ~cpoint~ represents an (epsilon, (x, y))-pair.

List Representation

The list representation of a ~cpoint~ is

----    ( epsilon ( x y ) )
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
                  nl->StringAtom("(<epsilon>(<x> <y>))"),
                  nl->StringAtom("( 20.5 ( 329.456 22.289) )"),
                  nl->StringAtom(" epsilon must be a pos. real-value." ))));
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
      ListExpr coordinates =  nl->TwoElemList(
      				nl->RealAtom( cpoint->p.GetX() ),
      				nl->RealAtom( cpoint->p.GetY() )); 
                  
      return nl->TwoElemList(
            nl->RealAtom( cpoint->GetEpsilon() ),
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
  // 2 arguments are necessary: epsilon and a point
  {
    ListExpr first = nl->First( instance );               // the epsilon value
    ListExpr second = nl->Second( instance );    // the point representation
    
    if ( nl->IsAtom( first ) && (nl->AtomType( first ) == RealType ||
            nl->AtomType( first ) == IntType) )
    {
      // The following commands are switched 'off' because they caused 
      // to crash the SecondoTTYBDB while updating a cpoint-object.
      // The only error-message was: 
      //*** glibc detected ***  free(): invalid pointer: 0xbfab9dc8 ***
      
      /*double e;
      if (nl->AtomType( first ) == IntType)
        e = nl->IntValue( first );
      else if(nl->AtomType( first ) == RealType)
        e = nl->RealValue( first );
      else
        correct = false;
        
      if ( !correct )
      {
        errmsg = "InCPoint(): First instant must be a real or int.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete &e;
        return SetWord( Address(0) );
      }*/
      
      if ( nl->ListLength( second ) == 2 &&
            nl->IsAtom( nl->First( second )) &&
            (nl->AtomType( nl->First( second )) == RealType ||
            nl->AtomType( nl->First( second )) == IntType) &&
            nl->IsAtom( nl->Second( second )) &&
            (nl->AtomType( nl->Second( second )) == RealType ||
            nl->AtomType( nl->Second( second )) == IntType))
      // if the second list element contains two real- or int-values, 
      // representing point-coordinates
      { 
        correct = true;
        Point *p = (Point *)InPoint( nl->TheEmptyList(), second, 
                                        errorPos, errorInfo, correct ).addr;
        if ( !correct )
        {
          errmsg = "InCPoint(): Second instant must be a representation" 
                         "of a point value.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete &p;
          return SetWord( Address(0) );
        }
        
        CPoint* cpoint = new CPoint( nl->RealValue( first ), 
                (StandardAttribute*) p );
        delete p;
        //delete &e;
        correct = cpoint->IsValid();
        
        if ( !correct )
        {
          errmsg = "InCPoint(): The cpoint-value is invalid!";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          delete p;
          return SetWord( Address(0) );
        }
        return SetWord( cpoint );
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
        CreateUncertain<Point>,
        DeleteUncertain<Point>,        //object creation and deletion
        0,
        0,                         // object open and save
        CloseCPoint,   
        CloneCPoint,         //object close and clone
        CastCPoint,           //cast function
        SizeOfCPoint,       //sizeof function
        CheckCPoint );      //kind checking function


/*
4.2 The Type Constructor ~cupoint~

Type ~cupoint~ represents a pair (epsilon, (tinterval, (x0, y0, x1, y1)))
consisting of an uncertainty-value and a value of type upoint.

4.2.1 List Representation

The list representation of an ~upoint~ is

----   ( epsilon ( timeinterval (x0 yo x1 y1) ) )
----

For example:

----    ( 37.5 ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   
----                    (1.0 2.3 4.1 2.1) ) )

4.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr CUPointProperty()
{
  return (nl->TwoElemList(
          nl->FourElemList(nl->StringAtom("Signature"),
                  nl->StringAtom("Example Type List"),
                  nl->StringAtom("List Rep"),
                  nl->StringAtom("Example List")),
          nl->FourElemList(nl->StringAtom("-> UNCERTAIN UNIT"),
                  nl->StringAtom("(cupoint) "),
                  nl->TextAtom("( epsilon ( timeInterval "
                          "(real_x0 real_y0 real_x1 real_y1) ) ) "),
                  nl->StringAtom("(0.7 ((i1 i2 TRUE FALSE)" 
                          "(1.0 2.2 2.5 2.1)))"))));
}


/*
4.2.3 Kind Checking Function

*/
bool CheckCUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cupoint" ));
}

/*
4.2.4 ~Out~-function

*/
ListExpr OutCUPoint( ListExpr typeInfo, Word value )
{
  CUPoint* cupoint = (CUPoint*)(value.addr);
  
  if( !(((CUPoint*)value.addr)->IsValid()) )
    return (nl->SymbolAtom("undef"));
  else
    {
      ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&cupoint->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), 
                  SetWord(&cupoint->timeInterval.end) ),
          nl->BoolAtom( cupoint->timeInterval.lc ),
          nl->BoolAtom( cupoint->timeInterval.rc));

      ListExpr pointsList = nl->FourElemList(
          nl->RealAtom( cupoint->p0.GetX() ),
          nl->RealAtom( cupoint->p0.GetY() ),
          nl->RealAtom( cupoint->p1.GetX() ),
          nl->RealAtom( cupoint->p1.GetY() ));
      
      ListExpr unitpointList = nl->TwoElemList(
          timeintervalList, pointsList );
          
      return nl->TwoElemList( nl->RealAtom( cupoint->GetEpsilon() ), 
          unitpointList );
    }
}

/*
4.2.5 ~In~-function

The Nested list form is like this:  
  ( 37.4 ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) ) )

*/
Word InCUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )    
  // 2 arguments are necessary: epsilon and a upoint
  {
    ListExpr first = nl->First( instance );               // the epsilon value
    ListExpr second = nl->Second( instance );    // the upoint representation
    
    if ( nl->IsAtom( first ) && (nl->AtomType( first ) == RealType ||
            nl->AtomType( first ) == IntType ) )
    {
      double e;
      if (nl->AtomType( first ) == IntType)
        e = nl->IntValue( first );
      else if(nl->AtomType( first ) == RealType)
        e = nl->RealValue( first );
      
      if ( nl->ListLength( second ) == 2 )
      // the upoint also consists of two components...
      {
        ListExpr tintvl = nl->First( second );        // the time-interval
        ListExpr endpoints = nl->Second( second );     // the two point values
      
        if( nl->ListLength( tintvl ) == 4 &&
            nl->IsAtom( nl->Third( tintvl ) ) &&
            nl->AtomType( nl->Third( tintvl ) ) == BoolType &&
            nl->IsAtom( nl->Fourth( tintvl ) ) &&
            nl->AtomType( nl->Fourth( tintvl ) ) == BoolType )
        {
          correct = true;
          Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
             nl->First( tintvl ), errorPos, errorInfo, correct ).addr;
    
          if( !correct )
          {
            errmsg = "InCUPoint(): Error in time-interval defining instant.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete start;
            return SetWord( Address(0) );
          }
      
          Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
              nl->Second( tintvl ), errorPos, errorInfo, correct ).addr;
      
          if( !correct )
          {
            errmsg = "InCUPoint(): Error in time-interval defining instant.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete start;
            delete end;
            return SetWord( Address(0) );
          }
    
          Interval<Instant> tinterval( *start, *end,
                                       nl->BoolValue( nl->Third( tintvl ) ),
                                       nl->BoolValue( nl->Fourth( tintvl ) ) );
          delete start;
          delete end;
        
          correct = tinterval.IsValid();
          if (!correct)
          {
            errmsg = "InCUPoint(): Non valid time interval.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            return SetWord( Address(0) );
          }

          if( nl->ListLength( endpoints ) == 4 )
          {
            Coord x0, y0, x1, y1;
            
            if( nl->IsAtom( nl->First( endpoints ) ) &&
                nl->AtomType( nl->First( endpoints )) == IntType)
              x0 = nl->IntValue(nl->First( endpoints ));
            else if ( nl->IsAtom( nl->First( endpoints ) ) &&
                nl->AtomType( nl->First( endpoints )) == RealType)
              x0 = nl->RealValue(nl->First( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Second( endpoints ) ) &&
                nl->AtomType( nl->Second( endpoints )) == IntType)
              y0 = nl->IntValue(nl->Second( endpoints ));
            else if ( nl->IsAtom( nl->Second( endpoints ) ) &&
                nl->AtomType( nl->Second( endpoints )) == RealType)
              y0 = nl->RealValue(nl->Second( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Third( endpoints ) ) &&
                nl->AtomType( nl->Third( endpoints )) == IntType)
              x1 = nl->IntValue(nl->Third( endpoints ));
            else if ( nl->IsAtom( nl->Third( endpoints ) ) &&
                nl->AtomType( nl->Third( endpoints )) == RealType)
              x1 = nl->RealValue(nl->Third( endpoints ));
            else
              correct = false;
            
            if( nl->IsAtom( nl->Fourth( endpoints ) ) &&
                nl->AtomType( nl->Fourth( endpoints )) == IntType)
              y1 = nl->IntValue(nl->Fourth( endpoints ));
            else if ( nl->IsAtom( nl->Fourth( endpoints ) ) &&
                nl->AtomType( nl->Fourth( endpoints )) == RealType)
              y1 = nl->RealValue(nl->Fourth( endpoints ));
            else
              correct = false;
            
            if( !correct )
            {
              errmsg = "InCUPoint(): Non valid point-coordinates.";
              errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
              return SetWord( Address(0) );
            }
              
            CUPoint *cupoint = new CUPoint( e, tinterval, x0, y0, x1, y1  );
    
            correct = cupoint->UnitIsValid();
            if( correct )
              return SetWord( cupoint );
        
            errmsg = errmsg + "InCUPoint(): Error in start/end point.";
            errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
            delete cupoint;
          }
        }
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
  {
    CUPoint *cupoint = new CUPoint(false);
    cupoint->timeInterval=
      Interval<DateTime>(DateTime(instanttype),
                         DateTime(instanttype),true,true);
    correct = cupoint->timeInterval.IsValid();
    if ( correct )
      return (SetWord( cupoint ));
  }
  errmsg = "InCUPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}


/*
4.2.6 ~Create~-function

*/
Word CreateCUPoint( const ListExpr typeInfo )
{
  return (SetWord( new CUPoint() ));
}

/*
4.2.7 ~Delete~-function

*/
void DeleteCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
4.2.8 ~Close~-function

*/
void CloseCUPoint( const ListExpr typeInfo, Word& w )
{
  delete (CUPoint *)w.addr;
  w.addr = 0;
}

/*
4.2.9 ~Clone~-function

*/
Word CloneCUPoint( const ListExpr typeInfo, const Word& w )
{
  CUPoint *cupoint = (CUPoint *)w.addr;
  return SetWord( new CUPoint( *cupoint ) );
}

/*
4.2.10 ~Sizeof~-function

*/
int SizeOfCUPoint()
{
  return sizeof(CUPoint);
}

/*
4.2.11 ~Cast~-function

*/
void* CastCUPoint( void* addr ) 
{
  return (new (addr) CUPoint);
}

/*
Creation of the type constructor ~cpoint~

*/

TypeConstructor uncertainunitpoint(
        "cupoint",                //name
        CUPointProperty,     //property function describing signature
        OutCUPoint,
        // For consequent implementation, the Out-function in the previous line
        // should be 'OutUncertain<Point, OutPoint>' instead of 'OutCPoint',
        // but the use of 'OutUncertain...' leads to a compiler error according
        // to this template-function in 'HierarchicalGeoAlgebra.h'! See there 
        // for further information! (Sascha Vaut)
        InCUPoint,               //Out and In functions
        0,
        0,                         //SaveToList and RestoreFromList functions
        CreateCUPoint,
        DeleteCUPoint,        //object creation and deletion
        0,
        0,                         // object open and save
        CloseCUPoint,   
        CloneCUPoint,         //object close and clone
        CastCUPoint,           //cast function
        SizeOfCUPoint,       //sizeof function
        CheckCUPoint );      //kind checking function


/*
4.3 Type Constructor CMPoint

Type ~cmpoint~ represents a moving point.

4.3.1 List Representation

The list representation of a ~cmpoint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~cupoint~.

For example:

----  (
          ( 0.4 
              ((instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1)))
          ( 0.5
              ((instant 11.4)  (instant 13.9)  FALSE FALSE) (4.1 2.1 8.9 4.3)))
        )
----

4.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr CMPointProperty()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"((0.42((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)))" 
          " ...)");
  
  return (nl->TwoElemList(
      nl->FourElemList(nl->StringAtom("Signature"),
                         nl->StringAtom("Example Type List"),
                         nl->StringAtom("List Rep"),
                         nl->StringAtom("Example List")),
      nl->FourElemList(nl->StringAtom("-> MAPPING"),
                         nl->StringAtom("(cmpoint) "),
                         nl->StringAtom("( u1 ... un ) "),
                         examplelist)));
}

/*
4.3.3 Kind Checking Function

*/
bool
CheckCMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "cmpoint" ));
}

TypeConstructor uncertainmovingpoint(
        "cmpoint",                           //name
        CMPointProperty,        //property function describing signature
        OutMapping<CMPoint, CUPoint, OutCUPoint>,
        InMapping<CMPoint, CUPoint, InCUPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<CMPoint>,
        DeleteMapping<CMPoint>,     //object creation and deletion
        0,
        0,      // object open and save
        CloseMapping<CMPoint>,
        CloneMapping<CMPoint>, //object close and clone
        CastMapping<CMPoint>,    //cast function
        SizeOfMapping<CMPoint>, //sizeof function
        CheckCMPoint );  //kind checking function


/*
Type Constructor +++++ hier weitere Typkonstruktoren anfuegen +++++

5 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

5.1 Type mapping functions

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.


5.1.1 Type mapping function ~UncertainTypeMapReal~

This type mapping function is used for the Operation ~Epsilon()~.

*/
ListExpr UncertainTypeMapReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );
    
    if ( nl->IsEqual( arg1, "cpoint" ) || 
          nl->IsEqual( arg1, "cupoint" ) || 
          nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom("real");
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + ".");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom("typeerror");
}

/*
5.1.2 Type mapping function ~UncertainTypeMapBase~

This type mapping function is used for the Operation ~Val()~. The keyword
'base' indicates a reduction of an uncertain type to its particular base type.
So in this case a 'base type' can also be a spatial or temporal type.

*/

ListExpr UncertainTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );
    
    if( nl->IsEqual( arg1, "cpoint") )
      return nl->SymbolAtom( "point" );
      
    if( nl->IsEqual( arg1, "cupoint") )
      return nl->SymbolAtom( "upoint" );
    if (nl->AtomType( args ) == SymbolType)
    {
      ErrorReporter::ReportError("Type mapping function got a "
              "parameter of type " +nl->SymbolValue(args) + 
              "which is no uncertain type.");
      return nl->SymbolAtom("typeerror");
    }
  }
  ErrorReporter::ReportError("Type mapping function got a "
        "parameter of length != 1.");
  return nl->SymbolAtom( "typeerror" );
}


/*
5.1.3 Type mapping function ~CertainToUncertain~

This type mapping function is used for the ~<certaintype>to<uncertaintype>~ 
Operations.

*/

ListExpr CertainToUncertain( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr first = nl->First(args);
    ListExpr second = nl->Second(args);
    if ( nl->IsEqual( first, "real" ) )
    {
      // find out if the second argument is of an implemented uncertain type
      if ( nl->IsEqual( second, "point" ) )
        return nl->SymbolAtom("cpoint");
      if ( nl->IsEqual( second, "upoint" ) )
        return nl->SymbolAtom("cupoint");
    }
    if ( (nl->AtomType(first) == SymbolType) && (nl->AtomType(second) == 
            SymbolType))
      ErrorReporter::ReportError("Type mapping function got parameters of "
        "type "
          + nl->SymbolValue(first) + " and "
          + nl->SymbolValue(second));
    else
      ErrorReporter::ReportError("Type mapping function got wrong types "
        "as parameters.");
  }
  ErrorReporter::ReportError("Type mapping function got a parameter of length "
    "!= 2.");
  return nl->SymbolAtom("typeerror");
}

/*
5.1.7 Type mapping function ~UncertainTempSetValueTypeMapInt~

It is for the ~no\_components~ operator.

*/
ListExpr
UncertainTempSetValueTypeMapInt( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom( "int" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.1.11 Type mapping function ~UncertainMovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
/*ListExpr
UncertainMovingTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->SymbolAtom( "cipoint" );
  }
  return nl->SymbolAtom( "typeerror" );
}*/

/*
5.1.12 Type Mapping Function ~UncertainMovingTypeMapUnits~

It is used for the operator ~units~

Type mapping for ~units~ is

----    (mbool)  -> (stream ubool)
        (mint)   -> (stream uint)
  (mreal)  -> (stream ureal)
  (mpoint) -> (stream upoint)
----

*/
ListExpr UncertainMovingTypeMapUnits( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("cupoint"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
5.1.18 Type mapping function "UncertainTemporalBBoxTypeMap"

For operator ~bbox~

*/

ListExpr UncertainTemporalBBoxTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "cupoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    if( nl->IsEqual( arg1, "cmpoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    if( nl->IsEqual( arg1, "cipoint" ) )
      return (nl->SymbolAtom( "rect3" ));

  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that
it is applied to correct arguments.

5.2.1 Selection function ~UncertainSimpleSelect~

Is used for the ~epsilon~ and ~val~ operators.

*/
int UncertainSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  
  if( nl->SymbolValue( arg1 ) == "cpoint" )
    return 0;
    
  if( nl->SymbolValue( arg1 ) == "cupoint" )
    return 1;
    
  if( nl->SymbolValue( arg1 ) == "cmpoint" )
    return 2;
  // ...space for further possible argument types
  
  return -1; // This point should never be reached
}



/*
6 Value mapping functions

6.1 Value mapping functions for class cpoint


6.1.1 Value mapping function for operator ~tocpoint~

*/

int ToCPoint( Word* args, Word& result, int message, Word& local,
                                        Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* e = (CcReal*)args[0].addr;
  Point* p = (Point*)args[1].addr;
  
  if ( e >= 0 )
    if ( p->IsDefined() )
    {
      CPoint cp( e->GetValue(), (StandardAttribute*) p );
      ((CPoint*)result.addr)->Set(cp);
    }
    else
    {
      ((CPoint*)result.addr)->UncertainSetDefined( false );
      cerr << "Result object is set to state: defined = false." << endl;
    }
  return 0;
}



/*
Definition of operators

Definition of operators is done in a way similar to definition of 
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first to define an 
array of value mapping functions for each operator. For nonoverloaded
operators there is also such an array defined, so it is easier to make them
overloaded.

ValueMapping arrays

*/

ValueMapping uncertainepsilonmap[] = { 
                                      UncertainEpsilon<Point>,
                                      UncertainEpsilon<Point>,
                                      UncertainEpsilon<Point> };


/*ValueMapping uncertainvalmap[] = {
                                      UncertainVal<Point>,
                                      UncertainVal<UPoint> };*/
/*
Specification strings

*/

const string UncertainSpecEpsilon  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> epsilon</text--->"
  "<text>epsilon ( _ )</text--->"
  "<text>Returns an uncertain values' epsilon value.</text--->"
  "<text>epsilon ( i1 )</text--->"
  ") )";


/*const string UncertainSpecVal =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>uT -> x</text--->"
  "<text>val ( _ )</text--->"
  "<text>Returns an uncertain value's value.</text--->"
  "<text>val ( i1 )</text--->"
  ") )";*/


const string CPointSpecToCPoint =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>point, real -> cpoint</text--->"
  "<text>toCPoint ( _, _ )</text--->"
  "<text>Builds a new CPoint from the given Real- and Point-values.</text--->"
  "<text>cpt = tocpoint ( 50.0, alexanderplatz )</text--->"
  ") )";


/*
Operators

*/

Operator uncertainepsilon( "epsilon",
                              UncertainSpecEpsilon,
                              3,
                              uncertainepsilonmap,
                              UncertainSimpleSelect,
                              UncertainTypeMapReal );

/*Operator uncertainval( "val",
                              UncertainSpecVal,
                              2,
                              uncertainvalmap,
                              UncertainSimpleSelect,
                              UncertainTypeMapBase );*/
                             
Operator tocpoint( "tocpoint",
                              CPointSpecToCPoint,
                              ToCPoint,
                              Operator::SimpleSelect,
                              CertainToUncertain );
                              
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
    
    AddTypeConstructor( &uncertainunitpoint );
    uncertainunitpoint.AssociateKind( "UNCERTAIN" );
    //uncertainunitpoint.AssociateKind( "TEMPORAL" );
    
    AddTypeConstructor( &uncertainmovingpoint );
    uncertainunitpoint.AssociateKind( "UNCERTAIN" );
    //uncertainunitpoint.AssociateKind( "TEMPORAL" );
    
    AddOperator( &uncertainepsilon );
    //AddOperator( &uncertainval );
    //AddOperator( &tocpoint );
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

