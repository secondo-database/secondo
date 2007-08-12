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

May 2007 Martin Scheppokat

[TOC]

1 Overview

2 Defines, includes, and constants

*/
#include "NestedList.h"
#include "TupleIdentifier.h"
#include "DBArray.h"

#include "GPoint.h"
#include "NetworkAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "OpMPoint2MGPoint.h"

#include <iostream>
#include <sstream>
#include <string>
#include "QueryProcessor.h"
#include "Algebra.h"


#include "DateTime.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3.1 Class ~UPoint~

*/
void UGPoint::TemporalFunction( const Instant& t,
                                GPoint& result,
                                bool ignoreLimits ) const
{
  throw SecondoException("Method UGPoint::TemporalFunction not implemented.");
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.SetDefined(false);
    }
  else if( t == timeInterval.start )
    {
      result = p0;
      result.SetDefined(true);
    }
  else if( t == timeInterval.end )
    {
      result = p1;
      result.SetDefined(true);
    }
  else
    {
return;
//      Instant t0 = timeInterval.start;
//      Instant t1 = timeInterval.end;
//
//      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
//      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();
//
//      result.Set( x, y );
//      result.SetDefined(true);
    }
}


bool UGPoint::Passes( const GPoint& p ) const
{
  throw SecondoException("Method UGPoint::Passes not implemented.");
/*
VTA - I could use the spatial algebra like this

----    HalfSegment hs;
        hs.Set( true, p0, p1 );
        return hs.Contains( p );
----
but the Spatial Algebra admit rounding errors (floating point operations). It
would then be very hard to return a true for this function.

*/
  assert( p.IsDefined() );
  assert( IsDefined() );

return false;

//  if( timeInterval.lc && AlmostEqual( p, p0 ) ||
//      timeInterval.rc && AlmostEqual( p, p1 ) )
//    return true;
//
//  if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
//      AlmostEqual( p0.GetX(), p.GetX() ) )
//    // If the segment is vertical
//  {
//    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
//        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
//      return true;
//  }
//  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
//      AlmostEqual( p0.GetY(), p.GetY() ) )
//    // If the segment is horizontal
//  {
//    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
//        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
//      return true;
//  }
//  else
//  {
//    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
//           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );
//
//    if( AlmostEqual( k1, k2 ) &&
//        ( ( p0.GetX() < p.GetX() && p1.GetX() > p.GetX() ) ||
//          ( p0.GetX() > p.GetX() && p1.GetX() < p.GetX() ) ) )
//      return true;
//  }
//  return false;
}


bool UGPoint::At( const GPoint& p, TemporalUnit<GPoint>& result ) const
{
  throw SecondoException("Method UGPoint::At not implemented.");
//  assert( IsDefined() );
//  assert( p.IsDefined() );
//
//  UGPoint *pResult = (UGPoint*)&result;
//
}


ListExpr UGPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ugpoint) "),
nl->TextAtom("( timeInterval (<nid> <rid> <side> <pos1> <pos2> ) ) "),
nl->StringAtom("((i1 i2 TRUE FALSE) (1 1 0 0.0 0.3))"))));
}

/*
4.9.3 Kind Checking Function

*/
bool
CheckUGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ugpoint" ));
}

/*
4.9.4 ~Out~-function

*/
ListExpr OutUGPoint( ListExpr typeInfo, Word value )
{
  UGPoint* ugpoint = (UGPoint*)(value.addr);

  if( !(((UGPoint*)value.addr)->IsDefined()) )
  {
    return (nl->SymbolAtom("undef"));
  }
  else
  {
    ListExpr timeintervalList = 
        nl->FourElemList(OutDateTime( nl->TheEmptyList(),
                                      SetWord(&ugpoint->timeInterval.start) ),
                         OutDateTime( nl->TheEmptyList(), 
                                      SetWord(&ugpoint->timeInterval.end) ),
          		           nl->BoolAtom( ugpoint->timeInterval.lc ),
          		           nl->BoolAtom( ugpoint->timeInterval.rc));

    ListExpr pointsList = 
         nl->FiveElemList(nl->IntAtom( ugpoint->p0.GetNetworkId() ),
                          nl->IntAtom( ugpoint->p0.GetRouteId() ),
                          nl->IntAtom( ugpoint->p0.GetSide() ),
                          nl->RealAtom( ugpoint->p0.GetPosition()),
                          nl->RealAtom( ugpoint->p1.GetPosition()));
      return nl->TwoElemList( timeintervalList, pointsList );
  }
}

/*
4.9.5 ~In~-function

*/
Word InUGPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUGPoint(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ), 
       errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUPoint(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct)
        {
          errmsg = "InUPoint(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 5 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == IntType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == IntType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == IntType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType &&
          nl->IsAtom( nl->Fifth( second ) ) &&
          nl->AtomType( nl->Fifth( second ) ) == RealType )
      {
        UGPoint *ugpoint = new UGPoint(tinterval,
                                     nl->IntValue( nl->First( second ) ),
                                     nl->IntValue( nl->Second( second ) ),
                                     (Side)nl->IntValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ), 
                                     nl->RealValue( nl->Fifth( second ) ));

        correct = ugpoint->IsValid();
        if( correct )
          return SetWord( ugpoint );

        errmsg = "InUGPoint(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete ugpoint;
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
    {
      UGPoint *ugpoint = new UGPoint(true);
      ugpoint->SetDefined(false);
      ugpoint->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = ugpoint->timeInterval.IsValid();
      if ( correct )
        return (SetWord( ugpoint ));
    }
  errmsg = "InUGPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
4.9.6 ~Create~-function

*/
Word CreateUGPoint( const ListExpr typeInfo )
{
  return (SetWord( new UGPoint() ));
}

/*
4.9.7 ~Delete~-function

*/
void DeleteUGPoint( const ListExpr typeInfo, Word& w )
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.8 ~Close~-function

*/
void CloseUGPoint( const ListExpr typeInfo, Word& w )
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.9 ~Clone~-function

*/
Word CloneUGPoint( const ListExpr typeInfo, const Word& w )
{
  UGPoint *ugpoint = (UGPoint *)w.addr;
  return SetWord( new UGPoint( *ugpoint ) );
}

/*
4.9.10 ~Sizeof~-function

*/
int SizeOfUGPoint()
{
  return sizeof(UGPoint);
}

/*
4.9.11 ~Cast~-function

*/
void* CastUGPoint(void* addr)
{
  return new (addr) UGPoint;
}

/*
4.9.12 Creation of the type constructor ~upoint~

*/
TypeConstructor unitgpoint(
        "ugpoint",      //name
        UGPointProperty,               //property function describing signature
        OutUGPoint,     InUGPoint, //Out and In functions
        0,             0,  //SaveToList and RestoreFromList functions
        CreateUGPoint,
        DeleteUGPoint, //object creation and deletion
        0,             0,        // object open and save
        CloseUGPoint,   CloneUGPoint, //object close and clone
        CastUGPoint, //cast function
        SizeOfUGPoint, //sizeof function
        CheckUGPoint );                    //kind checking function


/*
4.12 Type Constructor ~mgpoint~


4.12.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MGPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mgpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
nl->StringAtom("(((i1 i2 TRUE FALSE) (1 1 0 0.1 2.4)) ...)"))));
}

/*
4.12.3 Kind Checking Function

*/
bool
CheckMGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mgpoint" ));
}

/*
4.12.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor movinggpoint(
        "mgpoint",                           //name
        MGPointProperty,        //property function describing signature
        OutMapping<MGPoint, UGPoint, OutUGPoint>,
        InMapping<MGPoint, UGPoint, InUGPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<MGPoint>,
        DeleteMapping<MGPoint>,     //object creation and deletion
        0,
        0,      // object open and save
        CloseMapping<MGPoint>,
        CloneMapping<MGPoint>, //object close and clone
        CastMapping<MGPoint>,    //cast function
        SizeOfMapping<MGPoint>, //sizeof function
        CheckMGPoint );  //kind checking function


/*
4.4.4 Definition 

*/
Operator mpoint2mgpoint (
          "mpoint2mgpoint",                // name
          OpMPoint2MGPoint::Spec,          // specification
          OpMPoint2MGPoint::ValueMapping,  // value mapping
          Operator::SimpleSelect,          // trivial selection function
          OpMPoint2MGPoint::TypeMap        // type mapping
);




/*
6 Creating the Algebra

*/

class TemporalNetAlgebra : public Algebra
{
 public:
  TemporalNetAlgebra() : Algebra()
  {
    AddTypeConstructor( &unitgpoint );
    AddTypeConstructor( &movinggpoint );

    movinggpoint.AssociateKind( "TEMPORAL" );
    movinggpoint.AssociateKind( "DATA" );

    AddOperator(&mpoint2mgpoint);

  }
  ~TemporalNetAlgebra() {};
};

TemporalNetAlgebra temporalNetAlgebra;

/*
7 Initialization

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
InitializeTemporalNetAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalNetAlgebra);
}


