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

January 2004 Victor Almeida

March - April 2004 Zhiming Ding

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

12.03.2006 Juergen Schmidt seperated the ~=~ and ~\#~ Operater
because of wrong syntax of these operators.
For T in (bool, int, real, point) it should be mT x mT -> mbool,
but it was mT x mT -> bool. The new operators ~equal~ and
~nonequal~ represents the old meaning. The operators with the
"right" syntax can be found in TemporalLiftedAlgebra.
(Some formating work is done to re-check in the algebra to cvs)

09.04.2006 seperated the ~isempty~ Operater
because of wrong syntax of this operators.
For T in (bool, int, real, point) it should be mT -> mbool with
TRUE as result if the argument is defined and FALSE when the
argument is not defined, but it was mT -> bool with TRUE when it is
defined somewhere in time. (The total implementation has been left in
place.)
The operator with the "right" syntax can be found in
TemporalLiftedAlgebra.

01.06.2006 Christian D[ue]ntgen added operator ~bbox~ for ~range~-types.

Sept 2006 Christian D[ue]ntgen implemented ~defined~ flag for unit types

[TOC]

1 Overview

This file contains the implementation of the type constructors ~instant~,
~range~, ~intime~, ~const~, and ~mapping~. The memory data structures
used for these type constructors are implemented in the TemporalAlgebra.h
file.

2 Defines, includes, and constants

*/
#include <cmath>
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include <limits>

extern NestedList* nl;
extern QueryProcessor* qp;

#include "DateTime.h"
#include "TemporalAlgebra.h"

/*
1.1 Definition of some constants

*/
const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();



/*
3 Implementation of C++ Classes

3.1 Class ~UReal~

*/
void UReal::TemporalFunction( const Instant& t, 
                              CcReal& result, 
                              bool ignoreLimits ) const
{
  if ( !this->IsDefined() || 
       !t.IsDefined() || 
       (!this->timeInterval.Contains( t ) && !ignoreLimits) )
    {
      result.Set(false, 0.0);
    }
  else
    {
      double t2 = t.ToDouble() - timeInterval.start.ToDouble();      
      double res = a * pow( t2, 2 ) +
        b * t2 +
        c;
      if( r ) res = sqrt( res );
      result.Set( true, res );
    }
  

/*
2006-Sep-22, CD:

The following implemention is the original implementation following the
papers published, but suffers from rounding errors. 

----

    {
      double res = a * pow( t.ToDouble(), 2 ) +
                   b *      t.ToDouble()      +
                   c;
      if( r ) res = sqrt( res );
      result.Set( true, res );
    }

----

*/
}

bool UReal::Passes( const CcReal& val ) const
  // VTA - Not implemented yet
{
  return false;
}

bool UReal::At( const CcReal& val, TemporalUnit<CcReal>& result ) const
  // VTA - Not implemented yet
  // CD - Implementation causes problem, as the result could be a set of
  // 0-2 Units!
{
  return false;
}

void UReal::AtInterval( const Interval<Instant>& i,
 TemporalUnit<CcReal>& result ) const
{
  TemporalUnit<CcReal>::AtInterval( i, result );

  UReal *pResult = (UReal*)&result;
  pResult->a = a;
  pResult->b = b;
  pResult->c = c;
  pResult->r = r;
  pResult->StandardTemporalUnit<CcReal>::SetDefined(true);  

  // Now, we need to translate the result to the starting instant  
  DateTime tmp = pResult->timeInterval.start;
  tmp.SetType(durationtype);
  double tx = (timeInterval.start - tmp).ToDouble();
  pResult->TranslateParab(tx, 0.0);
}

// translate the parabolic/linear/constant curve within a ureal by (dx,dy)
// the ROOT flag is not considered at all!
void UReal::TranslateParab(const double& dx, const double& dy)
{
  long double xs,ys; // the coordinates of the new apex
  if (!AlmostEqual(a, 0.0) )
    {
      if (!AlmostEqual(b, 0.0) )
        { // quadratic function with b != 0
          xs =   -b/(2*a)         + dx;
          ys = c - pow(b,2)/(4*a) + dy;
          // a = a;
          b = -2 * a * xs;
          c = a*pow(xs,2) + ys;
        }
      else
        { // quadratic function with b == 0
          xs = dx;
          ys = c + dy;
        }
      // a = a;
      b = -2 * a * dx;
      c = c + dy;
      return;
    }
  else 
    { // a == 0
      if (!AlmostEqual(b, 0.0) )
        { // a linear function
          // translate by (dx) and (dy)
          a = 0.0;
          // b = b;
          c = c + dy - dx * b;          
        }
      else
        { // a constant function
          // translate by (dy) only
          a = 0.0;
          b = 0.0;
          c = c + dy;
        }
    }
}

/*
3.1 Class ~UPoint~

*/
void UPoint::TemporalFunction( const Instant& t, 
                               Point& result, 
                               bool ignoreLimits ) const
{
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
      Instant t0 = timeInterval.start;
      Instant t1 = timeInterval.end;
      
      double x = (p1.GetX() - p0.GetX()) * ((t - t0) / (t1 - t0)) + p0.GetX();
      double y = (p1.GetY() - p0.GetY()) * ((t - t0) / (t1 - t0)) + p0.GetY();
      
      result.Set( x, y );
      result.SetDefined(true);
    }
}


bool UPoint::Passes( const Point& p ) const
{
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

  if( timeInterval.lc && AlmostEqual( p, p0 ) ||
      timeInterval.rc && AlmostEqual( p, p1 ) )
    return true;

  if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
      AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
      return true;
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
      return true;
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() < p.GetX() && p1.GetX() > p.GetX() ) ||
          ( p0.GetX() > p.GetX() && p1.GetX() < p.GetX() ) ) )
      return true;
  }
  return false;
}

bool UPoint::Passes( const Region& r ) const
{
  if( AlmostEqual( p0, p1 ) )
    return r.Contains( p0 );

  HalfSegment hs( true, p0, p1 );
  return r.Intersects( hs );
}

bool UPoint::At( const Point& p, TemporalUnit<Point>& result ) const
{
/*
VTA - In the same way as ~Passes~, I could use the Spatial Algebra here.

*/
  assert( IsDefined() );
  assert( p.IsDefined() );

  UPoint *pResult = (UPoint*)&result;

  if( AlmostEqual( p0, p1 ) )
  {
    if( AlmostEqual( p, p0 ) )
    {
      *pResult = *this;
      return true;
    }
  }
  else if( AlmostEqual( p, p0 ) )
  {
    if( timeInterval.lc )
    {
      Interval<Instant> interval( timeInterval.start,
       timeInterval.start, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else if( AlmostEqual( p, p1 ) )
  {
    if( timeInterval.rc )
    {
      Interval<Instant> interval( timeInterval.end,
       timeInterval.end, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else if( AlmostEqual( p0.GetX(), p1.GetX() ) &&
           AlmostEqual( p0.GetX(), p.GetX() ) )
    // If the segment is vertical
  {
    if( ( p0.GetY() <= p.GetY() && p1.GetY() >= p.GetY() ) ||
        ( p0.GetY() >= p.GetY() && p1.GetY() <= p.GetY() ) )
    {
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetY() - p0.GetY() ) / ( p1.GetY() - p0.GetY() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else if( AlmostEqual( p0.GetY(), p1.GetY() ) &&
      AlmostEqual( p0.GetY(), p.GetY() ) )
    // If the segment is horizontal
  {
    if( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
        ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) )
    {
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  else
  {
    double k1 = ( p.GetX() - p0.GetX() ) / ( p.GetY() - p0.GetY() ),
           k2 = ( p1.GetX() - p0.GetX() ) / ( p1.GetY() - p0.GetY() );

    if( AlmostEqual( k1, k2 ) &&
        ( ( p0.GetX() <= p.GetX() && p1.GetX() >= p.GetX() ) ||
          ( p0.GetX() >= p.GetX() && p1.GetX() <= p.GetX() ) ) )
    {
      Instant t( timeInterval.start +
                   ( ( timeInterval.end - timeInterval.start ) *
                   ( ( p.GetX() - p0.GetX() ) / ( p1.GetX() - p0.GetX() ) ) ) );
      Interval<Instant> interval( t, t, true, true );
      UPoint unit( interval, p, p );
      *pResult = unit;
      return true;
    }
  }
  return false;
}

void UPoint::AtInterval( const Interval<Instant>& i,
 TemporalUnit<Point>& result ) const
{
  TemporalUnit<Point>::AtInterval( i, result );

  UPoint *pResult = (UPoint*)&result;

  assert( IsDefined() );
  assert( i.IsValid() );
  if( timeInterval.start == result.timeInterval.start )
    {
      pResult->p0 = p0;
      pResult->timeInterval.start = timeInterval.start;
      pResult->timeInterval.lc = pResult->timeInterval.lc && timeInterval.lc;
    }
  else
    TemporalFunction( result.timeInterval.start, pResult->p0 );
  
  if( timeInterval.end == result.timeInterval.end )
    {
      pResult->p1 = p1;
      pResult->timeInterval.end = timeInterval.end;
      pResult->timeInterval.rc = pResult->timeInterval.rc && timeInterval.rc;
    }
  else
    TemporalFunction( result.timeInterval.end, pResult->p1 );
  pResult->SetDefined ( true );
}

void UPoint::Distance( const Point& p, UReal& result ) const
{
  if( !IsDefined() || ! p.IsDefined() )
    {
      result.SetDefined(false);
    }
  else
    {
      result.timeInterval = timeInterval;
  
      DateTime DT = timeInterval.end - timeInterval.start;
      double dt = DT.ToDouble();
      double
        //t0 = timeInterval.start.ToDouble(),
        x0 = p0.GetX(), y0 = p0.GetY(),
        x1 = p1.GetX(), y1 = p1.GetY(),
        x  =  p.GetX(), y  =  p.GetY();
      
      if ( AlmostEqual(dt, 0.0) )
        { // single point unit
          result.a = 0.0;
          result.b = 0.0;
          result.c = pow(x0-x,2) + pow(y0-y,2);
          result.r = true;
        }
      else
        {
          result.a = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2);
          result.b = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y))/dt;
          result.c = pow(x0-x,2)+pow(y0-y,2);
          result.r = true;
          
/*

For the original representation of ureal, we need:

----
          double A = pow((x1-x0)/dt,2)+pow((y1-y0)/dt,2);
          double B = 2*((x1-x0)*(x0-x)+(y1-y0)*(y0-y))/dt;
          double C = pow(x0-x,2)+pow(y0-y,2);

          result.a = A;
          result.b = B-2*A*t0;
          result.c = t0*(t0*A-B)+C;
          result.r = true;

----

*/

        }
      result.SetDefined(true);
    }
  return;
}

/*
3.2 Class ~MInt~

*/
void MInt::ReadFrom(const MBool& arg){
  // remove all units
  Clear();
  if(!arg.IsDefined()){
    SetDefined(false);
    return; 
  }
  SetDefined(true);
  int size = arg.GetNoComponents();
  if(size>0){
     Resize(size);
  }
  const UBool* ubool;
  StartBulkLoad();
  CcInt currentValue;
  for(int i=0;i<size;i++){
    arg.Get(i,ubool);
    bool v;
    v = ubool->constValue.GetBoolval();
    currentValue.Set(true,v?1:0);
    UInt unit(ubool->timeInterval,currentValue);
    Add(unit); 
  } 
  EndBulkLoad(false);
} 


/*
3.2 Class ~MPoint~

*/
void MPoint::Trajectory( Line& line ) const
{
  line.Clear();
  line.StartBulkLoad();

  HalfSegment hs;
  const UPoint *unit;
  int edgeno = 0;

  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, unit );

    if( !AlmostEqual( unit->p0, unit->p1 ) )
    {
      hs.Set( true, unit->p0, unit->p1 );
      hs.attr.edgeno = ++edgeno;
      line += hs;
      hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
      line += hs;
    }
  }

  line.EndBulkLoad();
}

void MPoint::Distance( const Point& p, MReal& result ) const
{
  const UPoint *uPoint;
  UReal uReal(true);

  result.Clear();
  result.StartBulkLoad();
  for( int i = 0; i < GetNoComponents(); i++ )
  {
    Get( i, uPoint );
    uPoint->Distance( p, uReal );
    if ( uReal.IsDefined() )
      result.Add( uReal );
  }
  result.EndBulkLoad( false );
}

/*
4 Type Constructors

4.1 Type Constructor ~rint~

This type constructor implements the carrier set for ~range(int)~.

4.1.1 List Representation

The list representation of a ~rint~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( (1 5 TRUE FALSE) (6 9 FALSE FALSE) (11 11 TRUE TRUE) )
----

4.1.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeIntProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (0 1 TRUE FALSE) means the range [0, 1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rint) "),
        nl->StringAtom("((b1 e1 lci rci) ... (bn en lci rci))"),
        nl->StringAtom("((0 1 TRUE FALSE) (2 5 TRUE TRUE))"),
                             remarkslist)));
}

/*
4.1.3 Kind Checking Function

*/
bool
CheckRangeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rint" ));
}

/*
4.1.4 Creation of the type constructor ~rint~

*/
TypeConstructor rangeint(
        "rint",             //name
        RangeIntProperty,   //property function describing signature
        OutRange<CcInt, OutCcInt>,
        InRange<CcInt, InCcInt>,                 //Out and In functions
        0,            0,  //SaveToList and RestoreFromList functions
        CreateRange<CcInt>,DeleteRange<CcInt>,   //object creation and deletion
        OpenRange<CcInt>,  SaveRange<CcInt>,     // object open and save
        CloseRange<CcInt>, CloneRange<CcInt>,    //object close and clone
        CastRange<CcInt>,                        //cast function
        SizeOfRange<CcInt>,                      //sizeof function
        CheckRangeInt );                         //kind checking function


/*
4.2 Type Constructor ~rreal~

This type constructor implements the carrier set for ~range(real)~.

4.2.1 List Representation

The list representation of a ~rreal~ is

----    ( (r1b r1e lc1 rc1) (r2b r2e lc2 rc2) ... (rnb rne lcn rcn) )
----

For example:

----    ( (1.01 5 TRUE FALSE) (6.37 9.9 FALSE FALSE) (11.93 11.99 TRUE TRUE) )
----

4.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeRealProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
  " e.g. (0.5 1.1 TRUE FALSE) means the range [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rreal) "),
        nl->StringAtom("((b1 e1 lci rci) ... (bn en lci rci))"),
        nl->StringAtom("((0.5 1.1 TRUE FALSE) (2 5.04 TRUE TRUE))"),
                             remarkslist)));
}

/*
4.2.3 Kind Checking Function

*/
bool
CheckRangeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rreal" ));
}

/*
4.2.4 Creation of the type constructor ~rreal~

*/
TypeConstructor rangereal(
        "rreal",              //name
        RangeRealProperty,   //property function describing signature
        OutRange<CcReal, OutCcReal>,
        InRange<CcReal, InCcReal>,   //Out and In functions
        0,       0,      //SaveToList and RestoreFromList functions
        CreateRange<CcReal>,DeleteRange<CcReal>,  //object creation and deletion
        OpenRange<CcReal>,SaveRange<CcReal>,  // object open and save
        CloseRange<CcReal>,CloneRange<CcReal>,  //object close and clone
        CastRange<CcReal>,  //cast function
        SizeOfRange<CcReal>,                    //sizeof function
        CheckRangeReal );               //kind checking function

/*
4.3 Type Constructor ~periods~

This type constructor implements the carrier set for ~range(instant)~, which is
called ~periods~.

4.3.1 List Representation

The list representation of a ~periods~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( ( (instant 1.01)  (instant 5)     TRUE  FALSE)
          ( (instant 6.37)  (instant 9.9)   FALSE FALSE)
          ( (instant 11.93) (instant 11.99) TRUE  TRUE) )
----

4.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr
PeriodsProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
" e.g. ((instant 0.5) (instant 1.1) TRUE FALSE) means the interval [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(periods) "),
           nl->StringAtom("( (b1 e1 lci rci) ... (bn en lci rci) )"),
           nl->StringAtom("((i1 i2 TRUE FALSE) ...)"))));
}

/*
4.3.3 Kind Checking Function

*/
bool
CheckPeriods( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "periods" ));
}

/*
4.3.4 Creation of the type constructor ~periods~

*/
TypeConstructor periods(
        "periods",         //name
        PeriodsProperty,  //property function describing signature
        OutRange<Instant, OutDateTime>,
        InRange<Instant, InInstant>, //Out and In functions
        0,        0,        //SaveToList and RestoreFromList functions
        CreateRange<Instant>, DeleteRange<Instant>,
   //object creation and deletion
        OpenRange<Instant>,   SaveRange<Instant>,       // object open and save
        CloseRange<Instant>,  CloneRange<Instant>,      //object close and clone
        CastRange<Instant>,                          //cast function
        SizeOfRange<Instant>,                         //sizeof function
        CheckPeriods );    //kind checking function

/*
4.4 Type Constructor ~ibool~

Type ~ibool~ represents an (instant, value)-pair of booleans.

4.4.1 List Representation

The list representation of an ~ibool~ is

----    ( t bool-value )
----

For example:

----    ( (instant 1.0) FALSE )
----

4.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ibool) "),
                             nl->StringAtom("(instant bool-value) "),
                             nl->StringAtom("((instant 0.5) FALSE)"))));
}

/*
4.4.3 Kind Checking Function

*/
bool
CheckIntimeBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ibool" ));
}

/*
4.4.4 Creation of the type constructor ~ibool~

*/
TypeConstructor intimebool(
        "ibool",                   //name
        IntimeBoolProperty,             //property function describing signature
        OutIntime<CcBool, OutCcBool>,
        InIntime<CcBool, InCcBool>,     //Out and In functions
        0,
        0,     //SaveToList and RestoreFromList functions
        CreateIntime<CcBool>,
        DeleteIntime<CcBool>,           //object creation and deletion
        0,
        0,                              // object open and save
        CloseIntime<CcBool>,
        CloneIntime<CcBool>,            //object close and clone
        CastIntime<CcBool>,             //cast function
        SizeOfIntime<CcBool>,           //sizeof function
        CheckIntimeBool );              //kind checking function

/*
4.4 Type Constructor ~iint~

Type ~iint~ represents an (instant, value)-pair of integers.

4.4.1 List Representation

The list representation of an ~iint~ is

----    ( t int-value )
----

For example:

----    ( (instant 1.0) 5 )
----

4.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(iint) "),
                             nl->StringAtom("(instant int-value) "),
                             nl->StringAtom("((instant 0.5) 1)"))));
}

/*
4.4.3 Kind Checking Function

*/
bool
CheckIntimeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "iint" ));
}

/*
4.4.4 Creation of the type constructor ~iint~

*/
TypeConstructor intimeint(
        "iint",                   //name
        IntimeIntProperty,             //property function describing signature
        OutIntime<CcInt, OutCcInt>,
        InIntime<CcInt, InCcInt>,      //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcInt>,
        DeleteIntime<CcInt>,           //object creation and deletion
        0,
        0,                        // object open and save
        CloseIntime<CcInt>,
        CloneIntime<CcInt>,            //object close and clone
        CastIntime<CcInt>,             //cast function
        SizeOfIntime<CcInt>,           //sizeof function
        CheckIntimeInt );              //kind checking function

/*
4.5 Type Constructor ~ireal~

Type ~ireal~ represents an (instant, value)-pair of reals.

4.5.1 List Representation

The list representation of an ~ireal~ is

----    ( t real-value )
----

For example:

----    ( (instant 1.0) 5.0 )
----

4.5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ireal) "),
                             nl->StringAtom("(instant real-value)"),
                             nl->StringAtom("((instant 0.5) 1.0)"))));
}

/*
4.5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ireal" ));
}

/*
4.5.4 Creation of the type constructor ~ireal~

*/
TypeConstructor intimereal(
        "ireal",              //name
        IntimeRealProperty,   //property function describing signature
        OutIntime<CcReal, OutCcReal>,
        InIntime<CcReal, InCcReal>,        //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateIntime<CcReal>,
        DeleteIntime<CcReal>,              //object creation and deletion
        0,
        0,                                // object open and save
        CloseIntime<CcReal>,
        CloneIntime<CcReal>,               //object close and clone
        CastIntime<CcReal>,                //cast function
        SizeOfIntime<CcReal>,              //sizeof function
        CheckIntimeReal );                 //kind checking function

/*
4.6 Type Constructor ~ipoint~

Type ~ipoint~ represents an (instant, value)-pair of points.

4.6.1 List Representation

The list representation of an ~ipoint~ is

----    ( t point-value )
----

For example:

----    ( (instant 1.0) (5.0 0.0) )
----

4.6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimePointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(ipoint) "),
                             nl->StringAtom("(instant point-value)"),
                             nl->StringAtom("((instant 0.5) (1.0 2.0))"))));
}

/*
4.6.3 Kind Checking Function

*/
bool
CheckIntimePoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ipoint" ));
}

/*
4.6.4 Creation of the type constructor ~ipoint~

*/
TypeConstructor intimepoint(
        "ipoint",                    //name
        IntimePointProperty,  //property function describing signature
        OutIntime<Point, OutPoint>,
        InIntime<Point, InPoint>,         //Out and In functions
        0,
        0,       //SaveToList and RestoreFromList functions
        CreateIntime<Point>,
        DeleteIntime<Point>,              //object creation and deletion
        0,
        0,                           // object open and save
        CloseIntime<Point>,
        CloneIntime<Point>,               //object close and clone
        CastIntime<Point>,                //cast function
        SizeOfIntime<Point>,              //sizeof function
        CheckIntimePoint );               //kind checking function

/*
4.7 Type Constructor ~ubool~

Type ~ubool~ represents an (tinterval, boolvalue)-pair.

4.7.1 List Representation

The list representation of an ~ubool~ is

----    ( timeinterval bool-value )
----

For example:

----    ( ((instant 6.37)  (instant 9.9)   TRUE FALSE) TRUE )
----

4.7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ubool) "),
                             nl->StringAtom("(timeInterval bool) "),
                             nl->StringAtom("((i1 i2 FALSE FALSE) TRUE)"))));
}

/*
4.7.3 Kind Checking Function

*/
bool
CheckUBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ubool" ));
}

/*
4.7.4 Creation of the type constructor ~ubool~

*/
TypeConstructor unitbool(
        "ubool",        //name
        UBoolProperty,    //property function describing signature
        OutConstTemporalUnit<CcBool, OutCcBool>,
        InConstTemporalUnit<CcBool, InCcBool>,    //Out and In functions
        0,     0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcBool>,
        DeleteConstTemporalUnit<CcBool>,          //object creation and deletion
        0,                      0,                // object open and save
        CloseConstTemporalUnit<CcBool>,
        CloneConstTemporalUnit<CcBool>,           //object close and clone
        CastConstTemporalUnit<CcBool>,            //cast function
        SizeOfConstTemporalUnit<CcBool>,          //sizeof function
        CheckUBool );                             //kind checking function

/*
4.7 Type Constructor ~uint~

Type ~uint~ represents an (tinterval, intvalue)-pair.

4.7.1 List Representation

The list representation of an ~uint~ is

----    ( timeinterval int-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   5 )
----

4.7.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(uint) "),
                             nl->StringAtom("(timeInterval int) "),
                             nl->StringAtom("((i1 i2 FALSE FALSE) 1)"))));
}

/*
4.7.3 Kind Checking Function

*/
bool
CheckUInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "uint" ));
}

/*
4.7.4 Creation of the type constructor ~uint~

*/
TypeConstructor unitint(
        "uint",     //name
        UIntProperty, //property function describing signature
        OutConstTemporalUnit<CcInt, OutCcInt>,
        InConstTemporalUnit<CcInt, InCcInt>, //Out and In functions
        0,                      0,//SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcInt>,
        DeleteConstTemporalUnit<CcInt>, //object creation and deletion
        0,                      0,     // object open and save
        CloseConstTemporalUnit<CcInt>,
        CloneConstTemporalUnit<CcInt>, //object close and clone
        CastConstTemporalUnit<CcInt>,       //cast function
        SizeOfConstTemporalUnit<CcInt>, //sizeof function
        CheckUInt );                    //kind checking function

/*
4.8 Type Constructor ~ureal~

Type ~ureal~ represents an (tinterval, (a, b, c, r))-pair, where
 a, b, c are real numbers, r is a boolean flag indicating which
function to use.

11.1 List Representation

The list representation of an ~ureal~ is

----    ( timeinterval (a b c r) )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 TRUE) )
----

4.8.2 Function Describing the Signature of the Type Constructor

*/
ListExpr
URealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ureal) "),
         nl->StringAtom("( timeInterval (real1 real2 real3 bool)) "),
         nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 TRUE))"))));
}

/*
4.8.3 Kind Checking Function

*/
bool
CheckUReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "ureal" ));
}

/*
4.8.4 ~Out~-function

*/
ListExpr OutUReal( ListExpr typeInfo, Word value )
{
  UReal* ureal = (UReal*)(value.addr);

  if ( !ureal->IsDefined() )
    return (nl->SymbolAtom("undef"));
  else
    {

      ListExpr timeintervalList = nl->FourElemList(
             OutDateTime( nl->TheEmptyList(),
             SetWord(&ureal->timeInterval.start) ),
             OutDateTime( nl->TheEmptyList(), 
                          SetWord(&ureal->timeInterval.end) ),
             nl->BoolAtom( ureal->timeInterval.lc ),
             nl->BoolAtom( ureal->timeInterval.rc));

      ListExpr realfunList = nl->FourElemList(
             nl->RealAtom( ureal->a),
             nl->RealAtom( ureal->b),
             nl->RealAtom( ureal->c ),
             nl->BoolAtom( ureal->r));

      return nl->TwoElemList(timeintervalList, realfunList );
    }
}

/*
4.8.5 ~In~-function

The Nested list form is like this:

----    
       ( ( 6.37 9.9 TRUE FALSE)   (1.0 2.3 4.1 TRUE) )
or:    undef

----

*/
Word InUReal( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );
    if( nl->ListLength( first ) == 4 &&
      nl->IsAtom( nl->Third( first ) ) &&
      nl->AtomType( nl->Third( first ) ) == BoolType &&
      nl->IsAtom( nl->Fourth( first ) ) &&
      nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;
      if( correct == false )
      {
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;
      if( correct == false )
      {
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );

      delete start;
      delete end;

      ListExpr second = nl->Second( instance );

      if( nl->ListLength( second ) == 4 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == RealType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == BoolType )
      {
        UReal *ureal = 
          new UReal( tinterval,
                     nl->RealValue( nl->First( second ) ),
                     nl->RealValue( nl->Second( second ) ),
                     nl->RealValue( nl->Third( second ) ),
                     nl->BoolValue( nl->Fourth( second ) ) );

        if( ureal->IsValid() )
        {
          correct = true;
          return SetWord( ureal );
        }
        delete ureal;
      }
    }    
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType 
            && nl->SymbolValue( instance ) == "undef" )
    {
      UReal *ureal = new UReal();
      ureal->SetDefined(false);
      ureal->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = true;
      return (SetWord( ureal ));
    }
  correct = false;
  return SetWord( Address(0) );
}

/*
4.8.6 ~Create~-function

*/
Word CreateUReal( const ListExpr typeInfo )
{
  return (SetWord( new UReal() ));
}

/*
4.8.7 ~Delete~-function

*/
void DeleteUReal( const ListExpr typeInfo, Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
4.8.8 ~Close~-function

*/
void CloseUReal( const ListExpr typeInfo, Word& w )
{
  delete (UReal *)w.addr;
  w.addr = 0;
}

/*
4.8.9 ~Clone~-function

*/
Word CloneUReal( const ListExpr typeInfo, const Word& w )
{
  UReal *ureal = (UReal *)w.addr;
  return SetWord( new UReal( *ureal ) );
}

/*
4.8.10 ~Sizeof~-function

*/
int SizeOfUReal()
{
  return sizeof(UReal);
}

/*
4.8.11 ~Cast~-function

*/
void* CastUReal(void* addr)
{
  return new (addr) UReal;
}

/*
4.8.12 Creation of the type constructor ~ureal~

*/
TypeConstructor unitreal(
        "ureal",              //name
        URealProperty,  //property function describing signature
        OutUReal,     InUReal, //Out and In functions
        0,            0,   //SaveToList and RestoreFromList functions
        CreateUReal,
        DeleteUReal, //object creation and deletion
        0,            0,   // object open and save
        CloseUReal,   CloneUReal, //object close and clone
        CastUReal, //cast function
        SizeOfUReal, //sizeof function
        CheckUReal );                     //kind checking function

/*
4.9 Type Constructor ~upoint~

Type ~upoint~ represents an (tinterval, (x0, y0, x1, y1))-pair.

4.9.1 List Representation

The list representation of an ~upoint~ is

----    ( timeinterval (x0 yo x1 y1) )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   (1.0 2.3 4.1 2.1) )
----

4.9.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(upoint) "),
      nl->StringAtom("( timeInterval (real1 real2 real3 real4) ) "),
      nl->StringAtom("((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1))"))));
}

/*
4.9.3 Kind Checking Function

*/
bool
CheckUPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "upoint" ));
}

/*
4.9.4 ~Out~-function

*/
ListExpr OutUPoint( ListExpr typeInfo, Word value )
{
  UPoint* upoint = (UPoint*)(value.addr);

  if( !(((UPoint*)value.addr)->IsDefined()) )
    return (nl->SymbolAtom("undef"));
  else
    {
      ListExpr timeintervalList = nl->FourElemList(
          OutDateTime( nl->TheEmptyList(),
          SetWord(&upoint->timeInterval.start) ),
          OutDateTime( nl->TheEmptyList(), SetWord(&upoint->timeInterval.end) ),
          nl->BoolAtom( upoint->timeInterval.lc ),
          nl->BoolAtom( upoint->timeInterval.rc));

      ListExpr pointsList = nl->FourElemList(
          nl->RealAtom( upoint->p0.GetX() ),
          nl->RealAtom( upoint->p0.GetY() ),
          nl->RealAtom( upoint->p1.GetX() ),
          nl->RealAtom( upoint->p1.GetY() ));

      return nl->TwoElemList( timeintervalList, pointsList );
    }
}

/*
4.9.5 ~In~-function

The Nested list form is like this:  ( ( 6.37  9.9  TRUE FALSE)   (1.0 2.3 4.1 2.1) )

*/
Word InUPoint( const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct )
{
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

      if( correct == false )
      {
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
                                           errorPos, errorInfo, correct ).addr;

      if( correct == false )
      {
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
       nl->BoolValue( nl->Fourth( first ) ) );

      delete start;
      delete end;

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 4 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == RealType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == RealType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == RealType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType )
      {
        UPoint *upoint = new UPoint( tinterval,
                                     nl->RealValue( nl->First( second ) ),
                                     nl->RealValue( nl->Second( second ) ),
                                     nl->RealValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ) );

        if( upoint->IsValid() )
        {
          correct = true;
          return SetWord( upoint );
        }
        delete upoint;
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType 
            && nl->SymbolValue( instance ) == "undef" )
    {
      UPoint *upoint = new UPoint(true);
      upoint->SetDefined(false);
      upoint->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = true;
      return (SetWord( upoint ));
    }
  correct = false;
  return SetWord( Address(0) );
}

/*
4.9.6 ~Create~-function

*/
Word CreateUPoint( const ListExpr typeInfo )
{
  return (SetWord( new UPoint() ));
}

/*
4.9.7 ~Delete~-function

*/
void DeleteUPoint( const ListExpr typeInfo, Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.8 ~Close~-function

*/
void CloseUPoint( const ListExpr typeInfo, Word& w )
{
  delete (UPoint *)w.addr;
  w.addr = 0;
}

/*
4.9.9 ~Clone~-function

*/
Word CloneUPoint( const ListExpr typeInfo, const Word& w )
{
  UPoint *upoint = (UPoint *)w.addr;
  return SetWord( new UPoint( *upoint ) );
}

/*
4.9.10 ~Sizeof~-function

*/
int SizeOfUPoint()
{
  return sizeof(UPoint);
}

/*
4.9.11 ~Cast~-function

*/
void* CastUPoint(void* addr)
{
  return new (addr) UPoint;
}

/*
4.9.12 Creation of the type constructor ~upoint~

*/
TypeConstructor unitpoint(
        "upoint",      //name
        UPointProperty,               //property function describing signature
        OutUPoint,     InUPoint, //Out and In functions
        0,             0,  //SaveToList and RestoreFromList functions
        CreateUPoint,
        DeleteUPoint, //object creation and deletion
        0,             0,        // object open and save
        CloseUPoint,   CloneUPoint, //object close and clone
        CastUPoint, //cast function
        SizeOfUPoint, //sizeof function
        CheckUPoint );                    //kind checking function

/*
4.10 Type Constructor ~mbool~

Type ~mbool~ represents a moving boolean.

4.10.1 List Representation

The list representation of a ~mbool~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ubool~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) TRUE )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) FALSE )
        )
----

4.10.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mbool) "),
                             nl->StringAtom("( u1 ... un)"),
           nl->StringAtom("(((i1 i2 TRUE TRUE) TRUE) ...)"))));
}

/*
4.10.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mbool" ));
}

/*
4.10.4 Creation of the type constructor ~mbool~

*/
TypeConstructor movingbool(
        "mbool", //name
        MBoolProperty,  //property function describing signature
        OutMapping<MBool, UBool, OutConstTemporalUnit<CcBool, OutCcBool> >,
        InMapping<MBool, UBool, InConstTemporalUnit<CcBool, InCcBool> >,
       //Out and In functions
        0,
        0,    //SaveToList and RestoreFromList functions
        CreateMapping<MBool>,
        DeleteMapping<MBool>,        //object creation and deletion
        0,
        0,                // object open and save
        CloseMapping<MBool>,
        CloneMapping<MBool>,     //object close and clone
        CastMapping<MBool>,     //cast function
        SizeOfMapping<MBool>,    //sizeof function
        CheckMBool );          //kind checking function

/*
4.10 Type Constructor ~mint~

Type ~mint~ represents a moving integer.

4.10.1 List Representation

The list representation of a ~mint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~uint~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) 1 )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) 4 )
        )
----

4.10.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mint) "),
                             nl->StringAtom("( u1 ... un)"),
                             nl->StringAtom("(((i1 i2 TRUE TRUE) 1) ...)"))));
}

/*
4.10.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mint" ));
}

/*
4.10.4 Creation of the type constructor ~mint~

*/
TypeConstructor movingint(
        "mint",               //name
        MIntProperty,   //property function describing signature
        OutMapping<MInt, UInt, OutConstTemporalUnit<CcInt, OutCcInt> >,
        InMapping<MInt, UInt, InConstTemporalUnit<CcInt, InCcInt> >,
    //Out and In functions
        0,
        0,            //SaveToList and RestoreFromList functions
        CreateMapping<MInt>,
        DeleteMapping<MInt>,   //object creation and deletion
        0,
        0,           // object open and save
        CloseMapping<MInt>,
        CloneMapping<MInt>, //object close and clone
        CastMapping<MInt>,  //cast function
        SizeOfMapping<MInt>,  //sizeof function
        CheckMInt );      //kind checking function

/*
4.11 Type Constructor ~mreal~

Type ~mreal~ represents a moving real.

4.11.1 List Representation

The list representation of a ~mreal~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ureal~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 TRUE) )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) (1.0 2.3 4.1 FALSE) )
        )
----

4.11.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mreal) "),
                             nl->StringAtom("( u1 ... un) "),
           nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 FALSE)) ...)"))));
}

/*
4.11.3 Kind Checking Function

*/
bool
CheckMReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mreal" ));
}

/*
4.11.4 Creation of the type constructor ~mreal~

*/
TypeConstructor movingreal(
        "mreal",                    //name
        MRealProperty,    //property function describing signature
        OutMapping<MReal, UReal, OutUReal>,
        InMapping<MReal, UReal, InUReal>,   //Out and In functions
        0,
        0,      //SaveToList and RestoreFromList functions
        CreateMapping<MReal>,
        DeleteMapping<MReal>,    //object creation and deletion
        0,
        0,        // object open and save
        CloseMapping<MReal>,
        CloneMapping<MReal>,    //object close and clone
        CastMapping<MReal>,    //cast function
        SizeOfMapping<MReal>,    //sizeof function
        CheckMReal );                        //kind checking function

/*
4.12 Type Constructor ~mpoint~

Type ~mpoint~ represents a moving point.

4.12.1 List Representation

The list representation of a ~mpoint~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~upoint~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) (1.0 2.3 4.1 2.1) )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) (4.1 2.1 8.9 4.3) )
        )
----

4.12.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
        nl->StringAtom("(((i1 i2 TRUE FALSE) (1.0 2.2 2.5 2.1)) ...)"))));
}

/*
4.12.3 Kind Checking Function

*/
bool
CheckMPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mpoint" ));
}

/*
4.12.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor movingpoint(
        "mpoint",                           //name
        MPointProperty,        //property function describing signature
        OutMapping<MPoint, UPoint, OutUPoint>,
        InMapping<MPoint, UPoint, InUPoint>,//Out and In functions
        0,
        0,                 //SaveToList and RestoreFromList functions
        CreateMapping<MPoint>,
        DeleteMapping<MPoint>,     //object creation and deletion
        0,
        0,      // object open and save
        CloseMapping<MPoint>,
        CloneMapping<MPoint>, //object close and clone
        CastMapping<MPoint>,    //cast function
        SizeOfMapping<MPoint>, //sizeof function
        CheckMPoint );  //kind checking function

/*
16 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

16.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

16.1.1 Type mapping function ~TemporalTypeMapBool~

Is used for the ~isempty~ operator.

*/
ListExpr
TemporalTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "instant" ) ||
        nl->IsEqual( arg1, "rint" ) ||
        nl->IsEqual( arg1, "rreal" ) ||
        nl->IsEqual( arg1, "periods" ) ||
        nl->IsEqual( arg1, "ubool" ) ||
        nl->IsEqual( arg1, "uint" ) ||
        nl->IsEqual( arg1, "ureal" ) ||
        nl->IsEqual( arg1, "upoint" )) // ||
//        nl->IsEqual( arg1, "mbool" ) ||
//        nl->IsEqual( arg1, "mint" ) ||
//        nl->IsEqual( arg1, "mreal" ) ||
//        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.1 Type mapping function ~TemporalTemporalTypeMapBool~

Is used for the ~=~, and ~\#~ operators.

*/
ListExpr
TemporalTemporalTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "instant" )) ||
        (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

ListExpr
TemporalTemporalTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if(  (nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "mbool" )) ||
        (nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "mint" )) ||
        (nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "mreal" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "mpoint" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
Type Mapping for the mbool2mint function

*/
ListExpr TemporalMBool2MInt(ListExpr args){
  if(nl->ListLength(args)!=1){
    ErrorReporter::ReportError("Single argument expected");
    return nl->SymbolAtom( "typeerror" );
  }
  if(nl->IsEqual(nl->First(args),"mbool")){
    return   nl->SymbolAtom("mint");
  }
  ErrorReporter::ReportError("mint expected");
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.1 Type Mapping for the ~ExtendDeftime~ function

*/
ListExpr ExtDeftimeTypeMap(ListExpr args){
  if(nl->ListLength(args)!=2){
     ErrorReporter::ReportError("two argument expected");
     return nl->SymbolAtom( "typeerror" );
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if(nl->AtomType(arg1)!=SymbolType || nl->AtomType(arg2)!=SymbolType){
     ErrorReporter::ReportError("simple types required");
     return nl->SymbolAtom( "typeerror" );
  }
  string sarg1 = nl->SymbolValue(arg1);
  string sarg2 = nl->SymbolValue(arg2);
  if( (sarg1=="mint" && sarg2=="uint" ) ||
      (sarg1=="mbool" && sarg2=="ubool") ){
     return nl->SymbolAtom(sarg1);
  }
  ErrorReporter::ReportError("(mint x uint)  or (mbool x ubool) needed");
  return nl->SymbolAtom( "typeerror" );
}


/*
16.1.1 Type mapping function ~InstantInstantTypeMapBool~

Is used for the ~<~, ~<=~, ~>~, and ~>=~ operators.

*/
ListExpr
InstantInstantTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "instant" ) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.2 Type mapping function ~RangeRangeTypeMapBool~

It is for the ~intersects~, which have two
~ranges~ as input and a ~bool~ result type.

*/
ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.3 Type mapping function ~RangeBaseTypeMapBool1~

It is for the operator ~inside~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in this order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "int" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "real" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "periods" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.4 Type mapping function ~RangeBaseTypeMapBool2~

It is for the operator ~before~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in any order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "int" ) && nl->IsEqual( arg2, "rint" )) ||
        (nl->IsEqual( arg1, "real" ) && nl->IsEqual( arg2, "rreal" )) ||
        (nl->IsEqual( arg1, "instant" ) && nl->IsEqual( arg2, "periods" )) ||
        (nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "int" )) ||
        (nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "real" )) ||
        (nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "instant" )) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.5 Type mapping function ~RangeRangeTypeMapRange~

It is for the operators ~intersection~, ~union~, and ~minus~ which have two
~ranges~ as input and a ~range~ as result type.

*/
ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "rint" ) && nl->IsEqual( arg2, "rint" ) )
      return nl->SymbolAtom( "rint" );

    if( nl->IsEqual( arg1, "rreal" ) && nl->IsEqual( arg2, "rreal" ) )
      return nl->SymbolAtom( "rreal" );

    if( nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
}


/*
16.1.6 Type mapping function ~RangeTypeMapBase~

It is for the aggregate operators ~min~, ~max~, and ~avg~ which have one
~range~ as input and a ~BASE~ as result type.

*/
ListExpr
RangeTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "rint" ) )
      return nl->SymbolAtom( "int" );

    if( nl->IsEqual( arg1, "rreal" ) )
      return nl->SymbolAtom( "real" );

    if( nl->IsEqual( arg1, "periods" ) )
      return nl->SymbolAtom( "instant" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.7 Type mapping function ~TemporalSetValueTypeMapInt~

It is for the ~no\_components~ operator.

*/
ListExpr
TemporalSetValueTypeMapInt( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "rint" ) ||
        nl->IsEqual( arg1, "rreal" ) ||
        nl->IsEqual( arg1, "periods" ) ||
        nl->IsEqual( arg1, "mbool" ) ||
        nl->IsEqual( arg1, "mint" ) ||
        nl->IsEqual( arg1, "mreal" ) ||
        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "int" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.6 Type mapping function ~IntimeTypeMapBase~

It is for the operator ~val~.

*/
ListExpr
IntimeTypeMapBase( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "ibool" ) )
      return nl->SymbolAtom( "bool" );

    if( nl->IsEqual( arg1, "iint" ) )
      return nl->SymbolAtom( "int" );

    if( nl->IsEqual( arg1, "ireal" ) )
      return nl->SymbolAtom( "real" );

    if( nl->IsEqual( arg1, "ipoint" ) )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.6 Type mapping function ~IntimeTypeMapInstant~

It is for the operator ~inst~.

*/
ListExpr
IntimeTypeMapInstant( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "ibool" ) ||
        nl->IsEqual( arg1, "iint" ) ||
        nl->IsEqual( arg1, "ireal" ) ||
        nl->IsEqual( arg1, "ipoint" ) )
      return nl->SymbolAtom( "instant" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.7 Type mapping function ~MovingInstantTypeMapIntime~

It is for the operator ~atinstant~.

*/
ListExpr
MovingInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) )
        return nl->SymbolAtom( "ibool" );

      if( nl->IsEqual( arg1, "mint" ) )
        return nl->SymbolAtom( "iint" );

      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "ireal" );

      if( nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "ipoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.7 Type mapping function ~MovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr
MovingPeriodsTypeMapMoving( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) )
        return nl->SymbolAtom( "mbool" );

      if( nl->IsEqual( arg1, "mint" ) )
        return nl->SymbolAtom( "mint" );

      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "mreal" );

      if( nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "mpoint" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.8 Type mapping function ~MovingTypeMapRange~

It is for the operator ~deftime~.

*/
ListExpr
MovingTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mbool" ) ||
        nl->IsEqual( arg1, "mint" ) ||
        nl->IsEqual( arg1, "mreal" ) ||
        nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.8 Type mapping function ~MovingTypeMapSpatial~

It is for the operator ~trajectory~.

*/
ListExpr
MovingTypeMapSpatial( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "line" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.9 Type mapping function ~MovingInstantPeriodsTypeMapBool~

It is for the operator ~present~.

*/
ListExpr
MovingInstantPeriodsTypeMapBool( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) ||
        nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "mbool" ) ||
          nl->IsEqual( arg1, "mint" ) ||
          nl->IsEqual( arg1, "mreal" ) ||
          nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.10 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~passes~.

*/
ListExpr
MovingBaseTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( (nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" )) ||
        (nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "int" )) ||
// VTA - This operator is not yet implemented for the type of ~mreal~
//        (nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" )) ||
        (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "region" )) ) 
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.10 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~at~.

*/
ListExpr
MovingBaseTypeMapMoving( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" ) )
      return nl->SymbolAtom( "mbool" );

    if( nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "mint" );

// VTA - This operator is not yet implemented for the type of ~mreal~
//    if( nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" ) )
//      return nl->SymbolAtom( "mreal" );

    if( nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "mpoint" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.11 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mbool" ) )
      return nl->SymbolAtom( "ibool" );

    if( nl->IsEqual( arg1, "mint" ) )
      return nl->SymbolAtom( "iint" );

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->SymbolAtom( "ireal" );

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->SymbolAtom( "ipoint" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.11 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingBaseTypeMapMReal( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "mreal" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.12 Type Mapping Function ~MovingTypeMapUnits~

It is used for the operator ~units~

Type mapping for ~units~ is

----    (mbool)  -> (stream ubool)
        (mint)   -> (stream uint)
  (mreal)  -> (stream ureal)
  (mpoint) -> (stream upoint)
----

*/
ListExpr MovingTypeMapUnits( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, "mbool" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("ubool"));

    if( nl->IsEqual( arg1, "mint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("uint"));

    if( nl->IsEqual( arg1, "mreal" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"), nl->SymbolAtom("ureal"));

    if( nl->IsEqual( arg1, "mpoint" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("upoint"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
16.1.13 Type mapping function ~IntSetTypeMapPeriods~

It is used for the operators ~theyear~, ~themonth~, ~theday~, ~thehour~, ~theminute~,
~thesecond~

*/
ListExpr
IntSetTypeMapPeriods( ListExpr args )
{
  ListExpr argi;
  bool correct=true;

  if( ( nl->ListLength(args) < 1 ) || ( nl->ListLength(args) > 6) )
    return nl->SymbolAtom("typeerror");

  if( nl->ListLength(args) >= 1 )
  {
    argi = nl->First(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 2 )
  {
    argi = nl->Second(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 3 )
  {
    argi = nl->Third(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct = false;
  }

  if ( nl->ListLength(args) >= 4 )
  {
    argi = nl->Fourth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if( nl->ListLength(args) >= 5 )
  {
    argi = nl->Fifth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if ( nl->ListLength(args) >= 6 )
  {
    argi = nl->Sixth(args);
    if( !nl->IsEqual( argi, "int" ) )
      correct=false;
  }

  if( correct )
    return nl->SymbolAtom( "periods" );

  return nl->SymbolAtom("typeerror");
}

/*
16.1.13 Type mapping function ~PeriodsPeriodsTypeMapPeriods~

It is used for the operator ~theperiod~

*/
ListExpr
PeriodsPeriodsTypeMapPeriods( ListExpr args )
{
  if( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args),
             arg2 = nl->Second(args);

    if( nl->IsEqual( arg1, "periods" ) && nl->IsEqual( arg2, "periods" ) )
      return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom("typeerror");
}

/*
16.1.15 Type mapping function "MPointTypeMapTranslate"

This type mapping function is used for the ~translate~ operator.

*/
ListExpr MPointTypeMapTranslate( ListExpr args )
{
  ListExpr arg1, arg2;

  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "mpoint" ) &&
        nl->IsEqual(nl->First( arg2 ), "duration") &&
        nl->IsEqual(nl->Second( arg2 ), "real") &&
        nl->IsEqual(nl->Third( arg2 ), "real")) {
      return (nl->SymbolAtom( "mpoint" )); }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.16 Type mapping function "box3d"

*/
ListExpr Box3dTypeMap(ListExpr args){
  int len = nl->ListLength(args);
  if(len==2){
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if(nl->IsEqual(arg1,"rect")){
       if(nl->IsEqual(arg2,"instant") || nl->IsEqual(arg2,"periods"))
         return nl->SymbolAtom("rect3");
    }
    ErrorReporter::ReportError(" rect x {instant, periods } expected\n");
    return nl->SymbolAtom("typeerror");
  } else if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,"rect") || (nl->IsEqual(arg,"instant") ||
       nl->IsEqual(arg,"periods")))
        return nl->SymbolAtom("rect3");
    ErrorReporter::ReportError("rect, instant, or periods required\n");
    return nl->SymbolAtom("typeerror" );
  }
   else{
    ErrorReporter::ReportError("one or two arguments required");
    return nl->SymbolAtom("typeerror");
  }

}

/*
16.1.17 Type mapping function "box2d"

*/
ListExpr Box2dTypeMap(ListExpr args)
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "rect3" ) )
      return nl->SymbolAtom( "rect" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.1.18 Type mapping function "TemporalBBoxTypeMap" 

For operator ~bbox~

*/

ListExpr TemporalBBoxTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "upoint" ) )
      return (nl->SymbolAtom( "rect3" ));

    if( nl->IsEqual( arg1, "rint" )  )
      return (nl->SymbolAtom( "rint" ));

    if( nl->IsEqual( arg1, "rreal" ) )
      return (nl->SymbolAtom( "rreal" ));

    if( nl->IsEqual( arg1, "periods" ) )
      return (nl->SymbolAtom( "periods" ));
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
16.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

16.2.1 Selection function ~RangeSimpleSelect~

Is used for the ~min~ and ~max~ operators.

*/
int
RangeSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 2;

  return -1; // This point should never be reached
}

/*
16.2.1 Selection function ~RangeDualSelect~

Is used for the ~intersects~, ~inside~, ~before~, ~intersection~, ~union~,
and ~minus~ operations.

*/
int
RangeDualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "int" && nl->SymbolValue( arg2 ) == "rint" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "real" && nl->SymbolValue( arg2 ) == "rreal" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "instant" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "int" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "real" )
    return 7;

  if( nl->SymbolValue( arg1 ) == "periods" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 8;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSimpleSelect~

Is used for the ~isempty~ operation.

*/
int
TemporalSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "ubool" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "uint" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "ureal" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 7;

//  if( nl->SymbolValue( arg1 ) == "mbool" )
//    return 8;

//  if( nl->SymbolValue( arg1 ) == "mint" )
//    return 9;

//  if( nl->SymbolValue( arg1 ) == "mreal" )
//    return 10;

//  if( nl->SymbolValue( arg1 ) == "mpoint" )
//    return 11;

  return (-1); // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalDualSelect~

Is used for the ~=~, and ~\#~ operation.

*/
int
TemporalDualSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "instant" &&
    nl->SymbolValue( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" && nl->SymbolValue( arg2 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" && nl->SymbolValue( arg2 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" &&
    nl->SymbolValue( arg2 ) == "periods" )
    return 3;

  return -1; // This point should never be reached
}

int
TemporalDualSelect2( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue( arg2 ) == "mbool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue( arg2 ) == "mint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue( arg2 ) == "mreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "mpoint" )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~TemporalSetValueSelect~

Is used for the ~no\_components~ operation.

*/
int
TemporalSetValueSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mbool" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mint" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "mreal" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "mpoint" )
    return 6;

  return (-1); // This point should never be reached
}

/*
16.2.3 Selection function ~IntimeSimpleSelect~

Is used for the ~inst~ and ~val~ operations.

*/
int
IntimeSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "ibool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "iint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ireal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "ipoint" )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.3 Selection function ~MovingSimpleSelect~

Is used for the ~deftime~, ~initial~, ~final~, ~inst~, ~val~, ~atinstant~,
~atperiods~  operations.

*/
int
MovingSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mbool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" )
    return 3;

  return -1; // This point should never be reached
}

/*
16.2.3 Selection function ~MovingInstantPeriodsSelect~

Is used for the ~present~ operations.

*/
int
MovingInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "instant" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mbool" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "mint" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 5;

  if( nl->SymbolValue( arg1 ) == "mreal" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "mpoint" && nl->SymbolValue
  ( arg2 ) == "periods" )
    return 7;

  return -1; // This point should never be reached
}

/*
16.2.2 Selection function ~MovingBaseSelect~

Is used for the ~passes~ operations.

*/
int
MovingBaseSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "mbool" &&
      nl->SymbolValue( arg2 ) == "bool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "mint" &&
      nl->SymbolValue( arg2 ) == "int" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "mreal" && 
      nl->SymbolValue( arg2 ) == "real" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "mpoint" && 
      nl->SymbolValue( arg2 ) == "point" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "mpoint" && 
      nl->SymbolValue( arg2 ) == "region" )
    return 4;

  return -1; // This point should never be reached
}


/*
Selection function for the box3d operator

*/

int Box3dSelect(ListExpr args){
  int len = nl->ListLength(args);
  if(len==1){
    ListExpr arg = nl->First(args);
    if(nl->IsEqual(arg,"rect"))
        return 0;
    if(nl->IsEqual(arg,"instant"))
        return 1;
    if(nl->IsEqual(arg,"periods"))
        return 3;
  }
  if(len==2){
     if(nl->IsEqual(nl->First(args),"rect")){
         ListExpr arg2 = nl->Second(args);
         if(nl->IsEqual(arg2,"instant"))
            return 2;
         if(nl->IsEqual(arg2,"periods"))
            return 4;
     }
  }
  return -1; // when this occurs, the type mapping is not correct

}

/*
Selection function for the bbox operator

*/

int TemporalBBoxSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "rint" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "rreal" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "periods" )
    return 3;

  return -1; // This point should never be reached
}


/*
16.2.31 Selection function for extdeftime

*/
int ExtDeftimeSelect(ListExpr args){
   // the selection is only dependend on the type of the
   // first argument
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"mbool"))
      return 0;
   if(nl->IsEqual(arg,"mint"))
      return 1;
   return -1;
}

/*
16.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

16.3.1 Value mapping functions of operator ~isempty~

*/
int InstantIsEmpty( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( !((Instant*)args[0].addr)->IsDefined() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Range>
int RangeIsEmpty( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->IsEmpty() )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
16.3.2 Value mapping functions of operator $=$ (~equal~)

*/
int
InstantEqual( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)==0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

template <class Range>
int RangeEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) == *((Range*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
16.3.3 Value mapping functions of operator $\#$ (~not equal~)

*/
int
InstantNotEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)!=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

template <class Range>
int RangeNotEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range*)args[0].addr) != *((Range*)args[1].addr) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
16.3.4 Value mapping functions of operator $<$

*/
int
InstantLess( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)<0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.5 Value mapping functions of operator $<=$

*/
int
InstantLessEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)<=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.6 Value mapping functions of operator $>$

*/
int
InstantGreater( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)>0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.7 Value mapping functions of operator $>=$

*/
int
InstantGreaterEqual( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Instant* I1 = (Instant*) args[0].addr;
  Instant* I2 = (Instant*) args[1].addr;
  if(I1->IsDefined() && I2->IsDefined()){
      ((CcBool *)result.addr)->Set( true, (I1->CompareTo(I2)>=0)) ;
  } else {
      ((CcBool *)result.addr)->Set( false,false) ;
  }
  return 0;
}

/*
16.3.8 Value mapping functions of operator ~intersects~

*/
template <class Range>
int RangeIntersects( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Intersects( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
16.3.9 Value mapping functions of operator ~inside~

*/
template <class Range>
int RangeInside_rr( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Inside( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}


template <class Alpha, class Range>
int RangeInside_ar( Word* args, Word& result, int message, Word& local,
  Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Alpha*)args[0].addr)->IsDefined() &&
       ((Range*)args[1].addr)->IsDefined() )
  {

    if( ((Range*)args[1].addr)->Contains( *((Alpha*)args[0].addr) ) )
      ((CcBool*)result.addr)->Set( true, true );
    else
      ((CcBool *)result.addr)->Set( true, false );
  }

  else
    ((CcBool *)result.addr)->Set( false, false );

  return 0;
}

/*
16.3.10 Value mapping functions of operator ~before~

*/
template <class Range>
int RangeBefore_rr( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Before( *((Range*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Alpha, class Range>
int RangeBefore_ar( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[1].addr)->After( *((Alpha*)args[0].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Range, class Alpha>
int RangeBefore_ra( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range*)args[0].addr)->Before( *((Alpha*)args[1].addr) ) )
    ((CcBool*)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
16.3.11 Value mapping functions of operator ~intersection~

*/
template <class Range>
int RangeIntersection( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Intersection( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.12 Value mapping functions of operator ~union~

*/
template <class Range>
int RangeUnion( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Union( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.13 Value mapping functions of operator ~minus~

*/
template <class Range>
int RangeMinus( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range*)args[0].addr)->Minus( *((Range*)args[1].addr),
   (*(Range*)result.addr) );
  return 0;
}

/*
16.3.14 Value mapping functions of operator ~min~

*/
template <class Range, class Alpha>
int RangeMinimum( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Alpha*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->Minimum( *(Alpha*)result.addr);
  return 0;
}

/*
16.3.15 Value mapping functions of operator ~max~

*/
template <class Range, class Alpha>
int RangeMaximum( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Alpha*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->Maximum( *(Alpha*)result.addr);
  return 0;
}

/*
16.3.16 Value mapping functions of operator ~no\_components~

*/
template <class Range>
int RangeNoComponents( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true,
   ((Range*)args[0].addr)->GetNoComponents() );
  return 0;
}

/*
16.3.23 Value mapping functions of operator ~trajectory~

*/
int MPointTrajectory( Word* args, Word& result, int message,
 Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line *line = ((Line*)result.addr);
  MPoint *mpoint = ((MPoint*)args[0].addr);
  mpoint->Trajectory( *line );

  return 0;
}

int MBool2MInt( Word* args, Word& result, int message, Word&
 local, Supplier s ){
 result = qp->ResultStorage(s);
 ((MInt*)result.addr)->ReadFrom(*((MBool*) args[0].addr));
 return 0;

}
/*
16.3.29 Value mapping functions of operator ~distance~

*/
int MPointDistance( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((MPoint*)args[0].addr)->Distance( *((Point*)args[1].addr),
   *((MReal*)result.addr) );
  return 0;
}

/*
16.3.31 Value mapping functions of operator ~bbox~

*/
static int
UPointBBox( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  *((Rectangle<3>*)result.addr) = ((UPoint*)args[0].addr)->BoundingBox();
  return (0);
}

template <class Range>
int RangeBBox( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range*)args[0].addr)->IsEmpty() )
    ((Range*)result.addr)->SetDefined( false );
  else
    ((Range*)args[0].addr)->RBBox( *(Range*)result.addr);
  return 0;
}

/*
16.3.31 Value mapping functions of operator ~translate~

*/
static int
MPointTranslate( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  Word t;
  double dx,dy;
  DateTime* dd;
  const UPoint *uPoint;
  MPoint* mp, *mpResult;

  result = qp->ResultStorage( s );

  mp= (MPoint*)args[0].addr,
  mpResult = (MPoint*)result.addr;

  Supplier son = qp->GetSupplier( args[1].addr, 0 );
  qp->Request( son, t );
  dd = (DateTime *)t.addr;

  son = qp->GetSupplier( args[1].addr, 1 );
  qp->Request( son, t );
  dx = ((CcReal *)t.addr)->GetRealval();

  son = qp->GetSupplier( args[1].addr, 2 );
  qp->Request( son, t );
  dy = ((CcReal *)t.addr)->GetRealval();

  if ( mp->IsDefined() )
  {
    mpResult->Clear();
    mpResult->StartBulkLoad();
    for( int i = 0; i < mp->GetNoComponents(); i++ )
    {
      mp->Get( i, uPoint );
      UPoint aux( *uPoint );
      aux.p0.Set( aux.p0.GetX() + dx, aux.p0.GetY() + dy );
      aux.p1.Set( aux.p1.GetX() + dx, aux.p1.GetY() + dy );
      aux.timeInterval.start.Add(dd);
      aux.timeInterval.end.Add(dd);
      mpResult->Add(aux);
    }
    mpResult->EndBulkLoad();
    return 0;
  }
  else
  {
    mpResult->SetDefined( false );
    return 0;
  }
}

/*
16.3.32 Value mapping functions of operator ~theyear~

*/
int TheYear( Word* args, Word& result, int message, Word& local,
 Supplier s )
{ // int --> periods (=range(instant))
  result = qp->ResultStorage( s );
  Periods* pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval();

  Instant inst1, inst2;
  inst1.SetType( instanttype ) ;
  inst1.Set( intyear, 1, 1, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  inst2.Set( intyear + 1, 1, 1, 0, 0, 0, 0 );

  Interval<Instant> timeInterval(inst1, inst2, true, false);

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.33 Value mapping functions of operator ~themonth~

*/
int TheMonth( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval();

  Instant inst1, inst2;

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, 1, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  if( intmonth < 12 )
    inst2.Set(intyear, intmonth+1, 1, 0, 0, 0, 0);
  else
    inst2.Set(intyear+1, 1, 1, 0, 0, 0, 0);

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.34 Value mapping functions of operator ~theday~

*/
int TheDay( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval();

  Instant inst1, inst2,
          oneday( 1, 0, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, 0, 0, 0, 0 );

  inst2.SetType( instanttype );
  inst2 = inst1 + oneday;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.35 Value mapping functions of operator ~thehour~

*/
int TheHour( Word* args, Word& result, int message, Word& local,
 Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval();

  Instant inst1, inst2,
          onehour( 0, 1*60*60*1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, 0, 0, 0 );

  inst2 .SetType( instanttype );
  inst2 = inst1 + onehour;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.36 Value mapping functions of operator ~theminute~

*/
int TheMinute( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval(),
      intminute = ((CcInt*)args[4].addr)->GetIntval();

  Instant inst1, inst2,
          oneminute( 0, 1*60*1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, intminute, 0, 0 );

  inst2 .SetType( instanttype );
  inst2 = inst1 + oneminute;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.37 Value mapping functions of operator ~thesecond~

*/
int TheSecond( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr;

  int intyear = ((CcInt*)args[0].addr)->GetIntval(),
      intmonth = ((CcInt*)args[1].addr)->GetIntval(),
      intday = ((CcInt*)args[2].addr)->GetIntval(),
      inthour = ((CcInt*)args[3].addr)->GetIntval(),
      intminute = ((CcInt*)args[4].addr)->GetIntval(),
      intsecond = ((CcInt*)args[5].addr)->GetIntval();

  Instant inst1, inst2,
          onesecond( 0, 1000, durationtype );

  inst1.SetType( instanttype );
  inst1.Set( intyear, intmonth, intday, inthour, intminute, intsecond, 0 );

  inst2.SetType( instanttype );
  inst2 = inst1 + onesecond;

  Interval<Instant> timeInterval( inst1, inst2, true, false );

  pResult->Clear();
  pResult->StartBulkLoad();
  pResult->Add( timeInterval );
  pResult->EndBulkLoad( false );

  return 0;
}

/*
16.3.38 Value mapping functions of operator ~theperiod~

*/
int ThePeriod( Word* args, Word& result, int message, Word&
 local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods *pResult = (Periods*)result.addr,
          *range1 = ((Periods*)args[0].addr),
          *range2 = ((Periods*)args[1].addr);

  pResult->Clear();

  if( !range1->IsEmpty() || !range2->IsEmpty() )
  {
    const Interval<Instant> *intv1, *intv2;
    if( range1->IsEmpty() )
    {
      range2->Get( 0, intv1 );
      range2->Get( range2->GetNoComponents()-1, intv2 );
    }
    else if( range2->IsEmpty() )
    {
      range1->Get( 0, intv1 );
      range1->Get( range1->GetNoComponents()-1, intv2 );
    }
    else
    {
      range1->Get( 0, intv1 );
      range2->Get( range2->GetNoComponents()-1, intv2 );
    }

    Interval<Instant> timeInterval( intv1->start, intv2->end,
     intv1->lc, intv2->rc );

    pResult->StartBulkLoad();
    pResult->Add( timeInterval );
    pResult->EndBulkLoad( false );
  }
  return 0;
}

/*
16.3.39 Value Mappings function for box3d

*/

int Box3d_rect( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg = (Rectangle<2>*) args[0].addr;
   double min[3];
   double max[3];
   min[0] = arg->MinD(0);
   min[1] = arg->MinD(1);
   min[2] = MINDOUBLE;
   max[0] = arg->MaxD(0);
   max[1] = arg->MaxD(1);
   max[2] = MAXDOUBLE;
   res->Set(true,min,max);
   return 0;
}

int Box3d_instant( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Instant* arg = (Instant*) args[0].addr;
   double min[3];
   double max[3];
   min[0] = MINDOUBLE;
   min[1] = MINDOUBLE;
   max[0] = MAXDOUBLE;
   max[1] = MAXDOUBLE;
   double v = arg->ToDouble();
   max[2] = v;
   min[2] = v;
   res->Set(true,min,max);
   return 0;
}


int Box3d_rect_instant( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Instant* arg2 = (Instant*) args[1].addr;
   double v = arg2->ToDouble();
   double min[3];
   double max[3];
   min[0] = arg1->MinD(0);
   min[1] = arg1->MinD(1);
   min[2] = v;
   max[0] = arg1->MaxD(0);
   max[1] = arg1->MaxD(1);
   max[2] = v;
   res->Set(true,min,max);
   return 0;
}


int Box3d_periods( Word* args, Word& result, int message, Word&
 local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Periods* arg = (Periods*) args[0].addr;
   Instant i;
   arg->Minimum(i);
   double v1 = i.ToDouble();
   arg->Maximum(i);
   double v2 = i.ToDouble();
   double min[3];
   double max[3];
   min[0] = MINDOUBLE;
   min[1] = MINDOUBLE;
   min[2] = v1;
   max[0] = MAXDOUBLE;
   max[1] = MAXDOUBLE;
   max[2] = v2;
   res->Set(true,min,max);
   return 0;
}

int Box3d_rect_periods( Word* args, Word& result, int message,
 Word& local, Supplier s ){
   result = qp->ResultStorage(s);
   Rectangle<3>* res = (Rectangle<3>*) result.addr;
   Rectangle<2>* arg1 = (Rectangle<2>*) args[0].addr;
   Periods* arg2 = (Periods*) args[1].addr;
   Instant i;
   arg2->Minimum(i);
   double v1 = i.ToDouble();
   arg2->Maximum(i);
   double v2 = i.ToDouble();
   double min[3];
   double max[3];
   min[0] = arg1->MinD(0);
   min[1] = arg1->MinD(1);
   min[2] = v1;
   max[0] = arg1->MaxD(0);
   max[1] = arg1->MaxD(1);
   max[2] = v2;
   res->Set(true,min,max);
   return 0;
}

/*
16.3.39 Value Mapping function for box2d

*/

int TemporalBox2d( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Rectangle<2>* res = (Rectangle<2>*) result.addr;
  Rectangle<3>* arg = (Rectangle<3>*) args[0].addr;

  double min[2], max[2];
  min[0] = arg->MinD(0);
  min[1] = arg->MinD(1);
  max[0] = arg->MaxD(0);
  max[1] = arg->MaxD(1);
  res->Set( true, min, max );

  return 0;
}

/*
16.3.40 Value Mapping function for ExtDefTime

*/
template <class Unit,class Alpha>
int TemporalExtDeftime( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  Mapping<Unit,Alpha>* arg1 = (Mapping<Unit,Alpha>*) args[0].addr;
  Unit* arg2 = (Unit*) args[1].addr;
  Mapping<Unit,Alpha>* res = (Mapping<Unit,Alpha>*) result.addr;
  arg1->ExtendDefTime(*arg2,*res);
  return 0;
}

/*
16.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

16.4.1 ValueMapping arrays

*/
ValueMapping temporalisemptymap[] = { InstantIsEmpty,
                                      RangeIsEmpty<RInt>,
                                      RangeIsEmpty<RReal>,
                                      RangeIsEmpty<Periods>,
                                 //   MappingIsEmpty<MBool>,
                                 //   MappingIsEmpty<MInt>,
                                 //   MappingIsEmpty<MReal>,
                                 //   MappingIsEmpty<MPoint>,
                                      UnitIsEmpty<UBool>,
                                      UnitIsEmpty<UInt>,
                                      UnitIsEmpty<UReal>,
                                      UnitIsEmpty<UPoint> };


ValueMapping temporalequalmap[] = { InstantEqual,
                                    RangeEqual<RInt>,
                                    RangeEqual<RReal>,
                                    RangeEqual<Instant>};

ValueMapping temporalequalmap2[] = { MappingEqual<MBool>,
                                    MappingEqual<MInt>,
                                    MappingEqual<MReal>,
                                    MappingEqual<MPoint> };

ValueMapping temporalnotequalmap[] = { InstantNotEqual,
                                       RangeNotEqual<RInt>,
                                       RangeNotEqual<RReal>,
                                       RangeNotEqual<Periods>};

ValueMapping temporalnotequalmap2[] = { MappingNotEqual<MBool>,
                                       MappingNotEqual<MInt>,
                                       MappingNotEqual<MReal>,
                                       MappingNotEqual<MPoint> };

ValueMapping temporalintersectsmap[] = { RangeIntersects<RInt>,
                                         RangeIntersects<RReal>,
                                         RangeIntersects<Periods> };

ValueMapping temporalinsidemap[] = { RangeInside_rr<RInt>,
                                     RangeInside_rr<RReal>,
                                     RangeInside_rr<Periods>,
                                     RangeInside_ar<CcInt, RInt>,
                                     RangeInside_ar<CcReal, RReal>,
                                     RangeInside_ar<Instant, Periods> };

ValueMapping temporalbeforemap[] = { RangeBefore_rr<RInt>,
                                     RangeBefore_rr<RReal>,
                                     RangeBefore_rr<Periods>,
                                     RangeBefore_ar<CcInt, RInt>,
                                     RangeBefore_ar<CcReal, RReal>,
                                     RangeBefore_ar<Instant, Periods>,
                                     RangeBefore_ra<RInt, CcInt>,
                                     RangeBefore_ra<RReal, CcReal>,
                                     RangeBefore_ra<Periods, Instant> };

ValueMapping temporalintersectionmap[] = { RangeIntersection<RInt>,
                                           RangeIntersection<RReal>,
                                           RangeIntersection<Periods> };

ValueMapping temporalunionmap[] = { RangeUnion<RInt>,
                                    RangeUnion<RReal>,
                                    RangeUnion<Periods> };

ValueMapping temporalminusmap[] = { RangeMinus<RInt>,
                                    RangeMinus<RReal>,
                                    RangeMinus<Periods> };

ValueMapping temporalminmap[] = { RangeMinimum<RInt, CcInt>,
                                  RangeMinimum<RReal, CcReal>,
                                  RangeMinimum<Periods, Instant> };

ValueMapping temporalmaxmap[] = { RangeMaximum<RInt, CcInt>,
                                  RangeMaximum<RReal, CcReal>,
                                  RangeMaximum<Periods, Instant> };

ValueMapping temporalnocomponentsmap[] = { RangeNoComponents<RInt>,
                                           RangeNoComponents<RReal>,
                                           RangeNoComponents<Periods>,
                                           MappingNoComponents<MBool, CcBool>,
                                           MappingNoComponents<MInt, CcInt>,
                                           MappingNoComponents<MReal, CcReal>,
                                           MappingNoComponents<MPoint, Point>};

ValueMapping temporalbboxmap[] = { UPointBBox,
                                   RangeBBox<RInt>,
                                   RangeBBox<RReal>,
                                   RangeBBox<Periods> };


ValueMapping temporalinstmap[] = { IntimeInst<CcBool>,
                                   IntimeInst<CcInt>,
                                   IntimeInst<CcReal>,
                                   IntimeInst<Point> };

ValueMapping temporalvalmap[] = { IntimeVal<CcBool>,
                                  IntimeVal<CcInt>,
                                  IntimeVal<CcReal>,
                                  IntimeVal<Point> };

ValueMapping temporalatinstantmap[] = { MappingAtInstant<MBool, CcBool>,
                                        MappingAtInstant<MInt, CcInt>,
                                        MappingAtInstant<MReal, CcReal>,
                                        MappingAtInstant<MPoint, Point> };

ValueMapping temporalatperiodsmap[] = { MappingAtPeriods<MBool>,
                                        MappingAtPeriods<MInt>,
                                        MappingAtPeriods<MReal>,
                                        MappingAtPeriods<MPoint> };

ValueMapping temporaldeftimemap[] = { MappingDefTime<MBool>,
                                      MappingDefTime<MInt>,
                                      MappingDefTime<MReal>,
                                      MappingDefTime<MPoint> };

ValueMapping temporalpresentmap[] = { MappingPresent_i<MBool>,
                                      MappingPresent_i<MInt>,
                                      MappingPresent_i<MReal>,
                                      MappingPresent_i<MPoint>,
                                      MappingPresent_p<MBool>,
                                      MappingPresent_p<MInt>,
                                      MappingPresent_p<MReal>,
                                      MappingPresent_p<MPoint> };

ValueMapping temporalpassesmap[] = { MappingPasses<MBool, CcBool, CcBool>,
                                     MappingPasses<MInt, CcInt, CcInt>,
                                     MappingPasses<MReal, CcReal, CcReal>,
                                     MappingPasses<MPoint, Point, Point>, 
                                     MappingPasses<MPoint, Point, Region> };

ValueMapping temporalinitialmap[] = { MappingInitial<MBool, UBool, CcBool>,
                                      MappingInitial<MInt, UInt, CcInt>,
                                      MappingInitial<MReal, UReal, CcReal>,
                                      MappingInitial<MPoint, UPoint, Point> };

ValueMapping temporalfinalmap[] = { MappingFinal<MBool, UBool, CcBool>,
                                    MappingFinal<MInt, UInt, CcInt>,
                                    MappingFinal<MReal, UReal, CcReal>,
                                    MappingFinal<MPoint, UPoint, Point> };

ValueMapping temporalatmap[] = { MappingAt<MBool, UBool, CcBool>,
                                 MappingAt<MInt, UInt, CcInt>,
                                 MappingAt<MReal, UReal, CcReal>,
                                 MappingAt<MPoint, UPoint, Point> };

ValueMapping temporalunitsmap[] = { MappingUnits<MBool, UBool>,
                                    MappingUnits<MBool, UBool>,
                                    MappingUnits<MReal, UReal>,
                                    MappingUnits<MPoint, UPoint> };

ValueMapping temporalbox3dmap[] = { Box3d_rect,
                                    Box3d_instant,
                                    Box3d_rect_instant,
                                    Box3d_periods,
                                    Box3d_rect_periods};

ValueMapping extdeftimemap[] = { TemporalExtDeftime<UBool, CcBool>,
                                 TemporalExtDeftime<UInt, CcInt> 
                               };


/*
16.4.2 Specification strings

*/
const string TemporalSpecIsEmpty  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>instant -> bool,\n"
  "rT -> bool \n"
  "uT -> bool</text--->"
  "<text>isempty ( _ )</text--->"
  "<text>Returns whether the instant/range/unit type value is "
  "empty or not.</text--->"
  "<text>query isempty( mpoint1 )</text--->"
  ") )";

const string TemporalSpecEQ  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool, \n"
  "(rT rT) -> bool</text--->"
  "<text>_ = _</text--->"
  "<text>Is-Equal predicate for instant and range type values.</text--->"
  "<text>query i1 = i2</text--->"
  ") )";

const string TemporalSpecEQ2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(moving(x) moving(x)) -> bool</text--->"
  "<text>_ equal _</text--->"
  "<text>Equal.</text--->"
  "<text>query mi equal mi2</text--->"
  ") )";

const string TemporalSpecNE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool,\n"
  "(rT rT)) -> bool</text--->"
  "<text>_ # _</text--->"
  "<text>Not-Equal predicate for instant and rage type values.</text--->"
  "<text>query i1 # i2</text--->"
  ") )";

const string TemporalSpecNE2  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(mT mT) -> bool</text--->"
  "<text>_ nonequal _</text--->"
  "<text>Not-Equal predicate for moving objects.</text--->"
  "<text>query mi1 nonequal mi2</text--->"
  ") )";

const string TemporalSpecLT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ < _</text--->"
  "<text>Less than.</text--->"
  "<text>query i1 < i2</text--->"
  ") )";

const string TemporalSpecLE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ <= _</text--->"
  "<text>Less or equal than.</text--->"
  "<text>query i1 <= i2</text--->"
  ") )";

const string TemporalSpecGT  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ > _</text--->"
  "<text>Greater than.</text--->"
  "<text>query i1 > i2</text--->"
  ") )";

const string TemporalSpecGE  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text>(instant instant) -> bool</text--->"
  "<text>_ >= _</text--->"
  "<text>Greater or equal than.</text--->"
  "<text>query i1 >= i2</text--->"
  ") )";

const string TemporalSpecIntersects =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool</text--->"
  "<text>_ intersects _</text--->"
  "<text>Intersects predicate for range type values.</text--->"
  "<text>query range1 intersects range2</text--->"
  ") )";

const string TemporalSpecInside  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool,\n"
  "(T  rT) -> bool</text--->"
  "<text>_ inside _</text--->"
  "<text>Inside predicate for range type values.</text--->"
  "<text>query 5 inside rint</text--->"
  ") )";

const string TemporalSpecBefore  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> bool,\n"
  "(T rT) -> bool, (rT T) -> bool</text--->"
  "<text>_ before _ for for range type values and value</text--->"
  "<text>Before.</text--->"
  "<text>query 5 before rint</text--->"
  ") )";

const string TemporalSpecIntersection =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> rT</text--->"
  "<text>_ intersection _</text--->"
  "<text>Intersection for range type values</text--->"
  "<text>query range1 intersection range2</text--->"
  ") )";

const string TemporalSpecUnion  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT) -> rT</text--->"
  "<text>_ union _</text--->"
  "<text>Union for range type values.</text--->"
  "<text>query range1 union range2</text--->"
  ") )";

const string TemporalSpecMinus  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(rT rT ) -> rT</text--->"
  "<text>_ minus _</text--->"
  "<text>Minus for range type values.</text--->"
  "<text>query range1 minus range2</text--->"
  ") )";

const string TemporalSpecMinimum  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rT -> T</text--->"
  "<text>minimum ( _ )</text--->"
  "<text>Minimum of a range type value.</text--->"
  "<text>minimum ( range1 )</text--->"
  ") )";

const string TemporalSpecMaximum  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rT -> T</text--->"
  "<text>maximum ( _ )</text--->"
  "<text>Maximum of a range type value.</text--->"
  "<text>maximum ( range1 )</text--->"
  ") )";

const string TemporalSpecNoComponents =
  "( ( \"Signature\" \"Syntax\"\"Meaning\" \"Example\" ) "
  "( <text>rT -> int,\n"
  "mT -> int</text--->"
  "<text>no_components ( _ )</text--->"
  "<text>Number of components within a range type value."
  "Number of units inside a moving object value.</text--->"
  "<text>no_components ( mpoint1 )</text--->"
  ") )";

const string TemporalSpecInst  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>iT -> instant</text--->"
  "<text>inst ( _ )</text--->"
  "<text>Return an intime values' time instant.</text--->"
  "<text>inst ( i1 )</text--->"
  ") )";

const string TemporalSpecVal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>iT -> x</text--->"
  "<text>val ( _ )</text--->"
  "<text>Return an intime value's value.</text--->"
  "<text>val ( i1 )</text--->"
  ") )";

const string TemporalSpecAtInstant =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> iT</text--->"
  "<text>_ atinstant _ </text--->"
  "<text>From a moving object Get the intime value "
  "corresponding to the instant.</text--->"
  "<text>mpoint1 atinstant instant1</text--->"
  ") )";

const string TemporalSpecAtPeriods =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT periods) -> mT</text--->"
  "<text>_ atperiods _ </text--->"
  "<text>Restrict the moving object to the given periods.</text--->"
  "<text>mpoint1 atperiods periods1</text--->"
  ") )";

const string TemporalSpecDefTime  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>moving(x) -> periods</text--->"
  "<text>deftime( _ )</text--->"
  "<text>get the defined time of the corresponding moving data "
  "objects.</text--->"
  "<text>deftime( mp1 )</text--->"
  ") )";

const string TemporalSpecTrajectory =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint -> line</text--->"
  "<text> trajectory( _ )</text--->"
  "<text>get the trajectory of the corresponding moving point object.</text--->"
  "<text>trajectory( mp1 )</text--->"
  ") )";

const string TemporalSpecPresent  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT instant) -> bool,\n"
  "(mT periods) -> bool</text--->"
  "<text>_ present _ </text--->"
  "<text>whether the moving object is present at the given "
  "instant or period.</text--->"
  "<text>mpoint1 present instant1</text--->"
  ") )";

const string TemporalSpecPasses =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT T) -> bool</text--->"
  "<text>_ passes _ </text--->"
  "<text>whether the moving object passes the given value.</text--->"
  "<text>mpoint1 passes point1</text--->"
                                ") )";

const string TemporalSpecInitial  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT -> iT</text--->"
  "<text>initial( _ )</text--->"
  "<text>get the intime value corresponding to the initial instant.</text--->"
  "<text>initial( mpoint1 )</text--->"
  ") )";

const string TemporalSpecFinal  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT -> iT</text--->"
  "<text>final( _ )</text--->"
  "<text>From an moving object, get the intime value "
  "corresponding to its final instant.</text--->"
  "<text>final( mpoint1 )</text--->"
  ") )";

const string TemporalSpecAt =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mT T) -> mT</text--->"
  "<text> _ at _ </text--->"
  "<text>Restrict the moving object to the times where its value "
  "equals the given value.</text--->"
  "<text>mpoint1 at point1</text--->"
  ") )";

const string TemporalSpecDistance =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(mpoint point) -> mreal</text--->"
  "<text>distance( _, _ ) </text--->"
  "<text>returns the moving distance</text--->"
  "<text>distance( mpoint1, point1 )</text--->"
  ") )";

const string TemporalSpecUnits  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, real, point}:\n"
  "mT -> (stream uT)</text--->"
  "<text> units( _ )</text--->"
  "<text>get the stream of units of the moving value.</text--->"
  "<text>units( mpoint1 )</text--->"
  ") )";

const string TemporalSpecBBox  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>upoint -> rect3,\n"
  "rT -> rT</text--->"
  "<text>bbox ( _ )</text--->"
  "<text>Returns the 3d bounding box of the unit (for upoint)\n"
  "or the range value with the smallest closed interval that\n"
  "contains all intervals of a range-value (for range-value).</text--->"
  "<text>query bbox( upoint1 )</text--->"
  ") )";

const string MPointSpecTranslate  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mpoint x duration x real x real -> mpoint</text--->"
  "<text>_ translate[list]</text--->"
  "<text>Moves the object parallely for distance and time.</text--->"
  "<text>query mp1 translate[[const duration value (5 10)],5.0,8.0]</text--->"
  ") )";

const string TemporalSpecTheYear  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int -> periods</text--->"
  "<text> theyear( _ )</text--->"
  "<text>get the periods value of the year.</text--->"
  "<text>theyear(2002)</text--->"
                                ") )";

const string TemporalSpecTheMonth  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int -> periods</text--->"
  "<text> themonth( _, _ )</text--->"
  "<text>get the periods value of the month.</text--->"
  "<text>themonth(2002, 3)</text--->"
                                ") )";

const string TemporalSpecTheDay  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int -> periods</text--->"
  "<text>theday( _, _, _ )</text--->"
  "<text>get the periods value of the day.</text--->"
  "<text>theday(2002, 6,3)</text--->"
  ") )";

const string TemporalSpecTheHour  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int -> periods</text--->"
  "<text>thehour( _, _, _ , _)</text--->"
  "<text>get the periods value of the hour.</text--->"
  "<text>thehour(2002, 2, 28, 8)</text--->"
  ") )";

const string TemporalSpecTheMinute =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( "
  "<text>int x int x int x int x int -> periods</text--->"
  "<text>theminute( _ )</text--->"
  "<text>get the periods value of the minute.</text--->"
  "<text>theminute(2002, 3, 28, 8, 59)</text--->"
  ") )";

const string TemporalSpecTheSecond =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>int x int x int x int x int x int -> periods</text--->"
  "<text>thesecond( _ )</text--->"
  "<text>get the periods value of the second.</text--->"
  "<text>thesecond(2002, 12, 31, 23, 59, 59)</text--->"
  ") )";

const string TemporalSpecThePeriod =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(periods periods) -> periods</text--->"
  "<text> theperiod( _, _ )</text--->"
  "<text>Create a period that spans the starting instant of the first,"
  "to the ending instance of the second periods argument.</text--->"
  "<text>theperiod(theyear(2002), theyear(2004))</text--->"
  ") )";

const string Box3dSpec  =
  "( ( \"Signatures\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For S in {rect, instant, periods}:        S -> rect3 \n"
          "For T in {instant, periods}:       rect x T -> rect3  </text--->"
  "<text>box3d(_)</text--->"
  "<text>returns a threedimensional box which is unlimited "
  "in non-specified parts</text--->"
  "<text>query box3d(bbox(mehringdamm))</text--->"
  ") )";

const string TemporalBox2dSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>rect3 -> rect</text--->"
  "<text>box2d( _ )</text--->"
  "<text>returns the 2d part of a rect3. Can be used to eliminate the temporal"
  "dimension of a 3d bounding box.</text--->"
  "<text>box2d(r3)</text--->"
  ") )";

const string TemporalMBool2MIntSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mbool -> mint </text--->"
  "<text>mbool2mint( _ ) </text--->"
  "<text>converts the mbool value into a mint value"
  "</text--->"
  "<text>mbool2mint(mb1)</text--->"
  ") )";

const string TemporalExtDeftimeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mT x uT -> mT with T in {bool, int}  </text--->"
  "<text>extenddeftime( _ _) </text--->"
  "<text>extends the moving object's deftime by that of the unit's"
  " filling all gaps with the value taken from the unit.</text--->"
  "<text>query extdeftime(mb ub)</text--->"
  ") )";

/*
16.4.3 Operators

*/
Operator temporalisempty( "isempty",
                          TemporalSpecIsEmpty,
                          8,//12
                          temporalisemptymap,
                          TemporalSimpleSelect,
                          TemporalTypeMapBool );

Operator temporalequal( "=",
                        TemporalSpecEQ,
                        4,
                        temporalequalmap,
                        TemporalDualSelect,
                        TemporalTemporalTypeMapBool );

Operator temporalequal2( "equal",
                        TemporalSpecEQ2,
                        4,
                        temporalequalmap2,
                        TemporalDualSelect2,
                        TemporalTemporalTypeMapBool2 );

Operator temporalnotequal( "#",
                           TemporalSpecNE,
                           4,
                           temporalnotequalmap,
                           TemporalDualSelect,
                           TemporalTemporalTypeMapBool );

Operator temporalnotequal2( "nonequal",
                           TemporalSpecNE2,
                           4,
                           temporalnotequalmap2,
                           TemporalDualSelect2,
                           TemporalTemporalTypeMapBool2 );

Operator temporalless( "<",
                       TemporalSpecLT,
                       InstantLess,
                       Operator::SimpleSelect,
                       InstantInstantTypeMapBool );

Operator temporallessequal( "<=",
                            TemporalSpecLE,
                            InstantLessEqual,
                            Operator::SimpleSelect,
                            InstantInstantTypeMapBool );

Operator temporalgreater( ">",
                          TemporalSpecLT,
                          InstantGreater,
                          Operator::SimpleSelect,
                          InstantInstantTypeMapBool );

Operator temporalgreaterequal( ">=",
                               TemporalSpecLE,
                               InstantGreaterEqual,
                               Operator::SimpleSelect,
                               InstantInstantTypeMapBool );

Operator temporalintersects( "intersects",
                             TemporalSpecIntersects,
                             3,
                             temporalintersectsmap,
                             RangeDualSelect,
                             RangeRangeTypeMapBool );

Operator temporalinside( "inside",
                         TemporalSpecInside,
                         6,
                         temporalinsidemap,
                         RangeDualSelect,
                         RangeBaseTypeMapBool1 );

Operator temporalbefore( "before",
                         TemporalSpecBefore,
                         9,
                         temporalbeforemap,
                         RangeDualSelect,
                         RangeBaseTypeMapBool2 );

Operator temporalintersection( "intersection",
                               TemporalSpecIntersection,
                               3,
                               temporalintersectionmap,
                               RangeDualSelect,
                               RangeRangeTypeMapRange );

Operator temporalunion( "union",
                        TemporalSpecUnion,
                        3,
                        temporalunionmap,
                        RangeDualSelect,
                        RangeRangeTypeMapRange );

Operator temporalminus( "minus",
                        TemporalSpecMinus,
                        3,
                        temporalminusmap,
                        RangeDualSelect,
                        RangeRangeTypeMapRange );

Operator temporalmin( "minimum",
                      TemporalSpecMinimum,
                      3,
                      temporalminmap,
                      RangeSimpleSelect,
                      RangeTypeMapBase );

Operator temporalmax( "maximum",
                      TemporalSpecMaximum,
                      3,
                      temporalmaxmap,
                      RangeSimpleSelect,
                      RangeTypeMapBase );

Operator temporalnocomponents( "no_components",
                               TemporalSpecNoComponents,
                               7,
                               temporalnocomponentsmap,
                               TemporalSetValueSelect,
                               TemporalSetValueTypeMapInt );

Operator temporalinst( "inst",
                       TemporalSpecInst,
                       4,
                       temporalinstmap,
                       IntimeSimpleSelect,
                       IntimeTypeMapInstant );

Operator temporalval( "val",
                      TemporalSpecVal,
                      4,
                      temporalvalmap,
                      IntimeSimpleSelect,
                      IntimeTypeMapBase );

Operator temporalatinstant( "atinstant",
                            TemporalSpecAtInstant,
                            4,
                            temporalatinstantmap,
                            MovingSimpleSelect,
                            MovingInstantTypeMapIntime );

Operator temporalatperiods( "atperiods",
                            TemporalSpecAtPeriods,
                            4,
                            temporalatperiodsmap,
                            MovingSimpleSelect,
                            MovingPeriodsTypeMapMoving );

Operator temporaldeftime( "deftime",
                          TemporalSpecDefTime,
                          4,
                          temporaldeftimemap,
                          MovingSimpleSelect,
                          MovingTypeMapPeriods );

Operator temporaltrajectory( "trajectory",
                             TemporalSpecTrajectory,
                             MPointTrajectory,
                             Operator::SimpleSelect,
                             MovingTypeMapSpatial);

Operator temporalpresent( "present",
                          TemporalSpecPresent,
                          8,
                          temporalpresentmap,
                          MovingInstantPeriodsSelect,
                          MovingInstantPeriodsTypeMapBool);

Operator temporalpasses( "passes",
                         TemporalSpecPasses,
                         5,
                         temporalpassesmap,
                         MovingBaseSelect,
                         MovingBaseTypeMapBool);

Operator temporalinitial( "initial",
                          TemporalSpecInitial,
                          4,
                          temporalinitialmap,
                          MovingSimpleSelect,
                          MovingTypeMapIntime );

Operator temporalfinal( "final",
                        TemporalSpecFinal,
                        4,
                        temporalfinalmap,
                        MovingSimpleSelect,
                        MovingTypeMapIntime );

Operator temporalat( "at",
                     TemporalSpecAt,
                     4,
                     temporalatmap,
                     MovingBaseSelect,
                     MovingBaseTypeMapMoving );

Operator temporalbox3d( "box3d",
                        Box3dSpec,
                        5,
                        temporalbox3dmap,
                        Box3dSelect,
                        Box3dTypeMap );

Operator temporaldistance( "distance",
                           TemporalSpecDistance,
                           MPointDistance,
                           Operator::SimpleSelect,
                           MovingBaseTypeMapMReal );

Operator temporalunits( "units",
                        TemporalSpecUnits,
                        5,
                        temporalunitsmap,
                        MovingSimpleSelect,
                        MovingTypeMapUnits );

Operator temporalbbox( "bbox",
                       TemporalSpecBBox,
                       4,
                       temporalbboxmap,
                       TemporalBBoxSelect,
                       TemporalBBoxTypeMap );

Operator temporaltranslate( "translate",
                       MPointSpecTranslate,
                       MPointTranslate,
                       Operator::SimpleSelect,
                       MPointTypeMapTranslate );

Operator temporaltheyear( "theyear",
                          TemporalSpecTheYear,
                          TheYear,
                          Operator::SimpleSelect,
                          IntSetTypeMapPeriods );

Operator temporalthemonth( "themonth",
                           TemporalSpecTheMonth,
                           TheMonth,
                           Operator::SimpleSelect,
                           IntSetTypeMapPeriods );

Operator temporaltheday( "theday",
                         TemporalSpecTheDay,
                         TheDay,
                         Operator::SimpleSelect,
                         IntSetTypeMapPeriods );

Operator temporalthehour( "thehour",
                          TemporalSpecTheHour,
                          TheHour,
                          Operator::SimpleSelect,
                          IntSetTypeMapPeriods );

Operator temporaltheminute( "theminute",
                            TemporalSpecTheMinute,
                            TheMinute,
                            Operator::SimpleSelect,
                            IntSetTypeMapPeriods );

Operator temporalthesecond( "thesecond",
                            TemporalSpecTheSecond,
                            TheSecond,
                            Operator::SimpleSelect,
                            IntSetTypeMapPeriods );

Operator temporaltheperiod( "theperiod",
                            TemporalSpecThePeriod,
                            ThePeriod,
                            Operator::SimpleSelect,
                            PeriodsPeriodsTypeMapPeriods );

Operator temporalbox2d( "box2d",
                         TemporalBox2dSpec,
                         TemporalBox2d,
                         Operator::SimpleSelect,
                         Box2dTypeMap );

Operator mbool2mint( "mbool2mint",
                       TemporalMBool2MIntSpec,
                       MBool2MInt,
                       Operator::SimpleSelect,
                       TemporalMBool2MInt );

Operator extdeftime( "extdeftime",
                      TemporalExtDeftimeSpec,
                      2,
                      extdeftimemap,
                      ExtDeftimeSelect,
                      ExtDeftimeTypeMap );

/*
6 Creating the Algebra

*/

class TemporalAlgebra : public Algebra
{
 public:
  TemporalAlgebra() : Algebra()
  {
    AddTypeConstructor( &rangeint );
    AddTypeConstructor( &rangereal );
    AddTypeConstructor( &periods );
    AddTypeConstructor( &intimebool );
    AddTypeConstructor( &intimeint );
    AddTypeConstructor( &intimereal );
    AddTypeConstructor( &intimepoint );

    AddTypeConstructor( &unitbool );
    AddTypeConstructor( &unitint );
    AddTypeConstructor( &unitreal );
    AddTypeConstructor( &unitpoint );

    AddTypeConstructor( &movingbool );
    AddTypeConstructor( &movingint );
    AddTypeConstructor( &movingreal );
    AddTypeConstructor( &movingpoint );

    rangeint.AssociateKind( "RANGE" );
    rangeint.AssociateKind( "DATA" );
    rangereal.AssociateKind( "RANGE" );
    rangereal.AssociateKind( "DATA" );
    periods.AssociateKind( "RANGE" );
    periods.AssociateKind( "DATA" );

    intimebool.AssociateKind( "TEMPORAL" );
    intimebool.AssociateKind( "DATA" );
    intimeint.AssociateKind( "TEMPORAL" );
    intimeint.AssociateKind( "DATA" );
    intimereal.AssociateKind( "TEMPORAL" );
    intimereal.AssociateKind( "DATA" );
    intimepoint.AssociateKind( "TEMPORAL" );
    intimepoint.AssociateKind( "DATA" );

    unitbool.AssociateKind( "TEMPORAL" );
    unitbool.AssociateKind( "DATA" );
    unitint.AssociateKind( "TEMPORAL" );
    unitint.AssociateKind( "DATA" );
    unitreal.AssociateKind( "TEMPORAL" );
    unitreal.AssociateKind( "DATA" );
    unitpoint.AssociateKind( "TEMPORAL" );
    unitpoint.AssociateKind( "DATA" );
    unitpoint.AssociateKind( "SPATIAL3D" );

    movingbool.AssociateKind( "TEMPORAL" );
    movingbool.AssociateKind( "DATA" );
    movingint.AssociateKind( "TEMPORAL" );
    movingint.AssociateKind( "DATA" );
    movingreal.AssociateKind( "TEMPORAL" );
    movingreal.AssociateKind( "DATA" );
    movingpoint.AssociateKind( "TEMPORAL" );
    movingpoint.AssociateKind( "DATA" );

    AddOperator( &temporalisempty );
    AddOperator( &temporalequal );
    AddOperator( &temporalnotequal );
    AddOperator( &temporalequal2 );
    AddOperator( &temporalnotequal2 );
    AddOperator( &temporalless );
    AddOperator( &temporallessequal );
    AddOperator( &temporalgreater );
    AddOperator( &temporalgreaterequal );
    AddOperator( &temporalintersects );
    AddOperator( &temporalinside );
    AddOperator( &temporalbefore );
    AddOperator( &temporalintersection );
    AddOperator( &temporalunion );
    AddOperator( &temporalminus );
    AddOperator( &temporalmin );
    AddOperator( &temporalmax );
    AddOperator( &temporalnocomponents );

    AddOperator( &temporalinst );
    AddOperator( &temporalval );
    AddOperator( &temporalatinstant );
    AddOperator( &temporalatperiods );
    AddOperator( &temporaldeftime );
    AddOperator( &temporaltrajectory );
    AddOperator( &temporalpresent );
    AddOperator( &temporalpasses );
    AddOperator( &temporalinitial );
    AddOperator( &temporalfinal );
    AddOperator( &temporalunits );
    AddOperator( &temporalbbox );

    AddOperator( &temporalat );
    AddOperator( &temporaldistance );
    AddOperator( &temporaltranslate );

    AddOperator( &temporaltheyear );
    AddOperator( &temporalthemonth );
    AddOperator( &temporaltheday );
    AddOperator( &temporalthehour );
    AddOperator( &temporaltheminute );
    AddOperator( &temporalthesecond );
    AddOperator( &temporaltheperiod );

    AddOperator(&temporalbox3d);
    AddOperator(&temporalbox2d);
    AddOperator(&mbool2mint);
    AddOperator(&extdeftime);

  }
  ~TemporalAlgebra() {};
};

TemporalAlgebra temporalAlgebra;

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
InitializeTemporalAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalAlgebra);
}


