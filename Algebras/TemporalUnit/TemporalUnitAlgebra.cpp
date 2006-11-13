/*

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[ue] [\"u]
//[ae] [\"a]

//[TOC] [\tableofcontents]

[1] TemporalUnitAlgebra - Implementing Units-Operators

May 2006, initial version implemented by Thomas Fischer for diploma
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Feruniversit[ae]t Hagen.

July 2006, Christian D[ue]ntgen: The so far implemented operators do not suffice
to model typical queries using the compact unit representation ~moving(T)~ with
relations of units. Instead, we also need variants of spatiotemporal operators
that process streams of units. Instead of implementing them directly, we will 
fake them using the set wrapper-operators ~suse~ on the native single-unit 
operators.

The ~suse~ operators passes single values to an operator one-by-one value and 
collects all returned values/streams of values in a flat stream of values.

This does not work for some binary predicates, like equal, but one could implement
an ordered pairwise comparison here.

It may be useful, to have some operators consuming a stream of units and
returning an aggregated vale, as e.g. initial, final, present, never, always.

----

State Operator/Signatures


OK    suse:  (stream X)            (map X Y)            --> (stream Y)
OK           (stream X)            (map X (stream Y))   --> (stream Y)

OK    suse2: (stream X) Y          (map X Y Z)          --> (stream Z)
OK           (stream X) Y          (map X Y stream(Z))  --> (stream Z)
OK           X          (stream Y) (map X y Z)          --> (stream Z)
OK           X          (stream Y) (map X y (stream Z)) --> (stream Z)
OK           (stream X) (stream Y) (map X Y Z)          --> (stream Z)
OK           (stream X) (stream Y) (map X Y (stream Z)) --> (stream Z)
             for X,Y,Z of kind DATA

OK    sfeed:                          T --> (stream T)

OK    transformstream: stream(tuple((id T))) --> (stream T)
OK                                (stream T) --> stream(tuple((element T)))
OK    saggregate:        (stream T) x (T x T --> T) x T  --> T
OK    count:                      (stream T) --> int
OK    filter:      ((stream T) (map T bool)) --> int
OK    printstream:                (stream T) --> (stream T)
(OK)  makemvalue   (**)  stream (tuple ([x1:t1,xi:uT,..,xn:tn])) -->  mT
OK    size                           periods --> real
(OK)  point2d                        periods --> point
(OK)  queryrect2d                    instant --> rect
OK    circle              point x real x int --> region

OK    isempty                             U  --> bool (for U in kind UNIT)
OK    trajectory                      upoint --> line
OK    velocity                        mpoint --> mpoint
OK                                    upoint --> upoint
OK    derivable                        mreal --> mbool
OK                                     ureal --> ubool
OK    derivative                       mreal --> mreal
OK                                     ureal --> ureal
OK    speed                           mpoint --> mreal
OK                                    upoint --> ureal

      passes:  For T in {bool, int, string, point}:
OK  +                            uT x      T --> bool
n/a +                         ureal x   real --> bool
n/a +                       uregion x region --> bool

OK  + deftime      (**)                   uT --> periods
OK  + atinstant    (**)         uT x instant --> iT
OK  + atperiods    (**          uT x periods --> (stream uT)
OK  + Initial      (**)                   uT --> iT
OK  + final        (**)                   uT --> iT
OK  + present      (**)         uT x instant --> bool
OK  +              (**)         uT x periods --> bool
     
      atmax:  For T in {bool, int, real, string}:
(OK)+                                     uT --> (stream uT)

      atmin:  For T in {bool, int, real, string}:
(OK)+                                     uT --> (stream uT)
 
(OK)+ at:                    ureal x    real --> (stream ureal)
OK  +                       upoint x   point --> upoint
n/a +                       upoint x  region --> (stream upoint)
n/a +                       upoint x uregion --> (stream upoint)


      distance:  T in {int, point}
OK  -           uT x uT -> ureal
OK  ?           uT x  T -> ureal 
OK  ?            T x uT -> ureal

     intersection: For T in {bool, int, string}:
OK  +          uT x      uT --> (stream uT)
OK  +          uT x       T --> (stream uT)
OK  +           T x      uT --> (stream uT)
(OK)+       ureal x    real --> (stream ureal)
(OK)+        real x   ureal --> (stream ureal)
Pre +       ureal x   ureal --> (stream ureal)
OK  -      upoint x   point --> (stream upoint) same as at: upoint x point
OK  -       point x  upoint --> (stream upoint) same as at: upoint x point
OK  -      upoint x  upoint --> (stream upoint)
OK  +      upoint x    line --> (stream upoint)
OK  +        line x  upoint --> (stream upoint)
Pre +      upoint x uregion --> (stream upoint)
Pre +     uregion x  upoint --> (stream upoint)
Pre -      upoint x  region --> (stream upoint)
Pre -      region x  upoint --> (stream upoint)
n/a +      upoint x  points --> (stream upoint)
n/a +      points x  upoint --> (stream upoint)

     intersects: For T in {bool, int, string, real, point}:
Pre -          uT x      uT --> (stream ubool)
n/a -          uT x       T --> (stream ubool)
n/a -           T x      uT --> (stream ubool)
n/a -       ureal x    real --> (stream ubool)
n/a -        real x   ureal --> (stream ubool)
n/a -       ureal x   ureal --> (stream ubool)
n/a -      upoint x   point --> (stream ubool)
n/a -       point x  upoint --> (stream ubool)
n/a -      upoint x  upoint --> (stream ubool)
n/a -      upoint x uregion --> (stream ubool) as inside
n/a -     uregion x  upoint --> (stream ubool) as inside
n/a -      upoint x    line --> (stream ubool) as inside
n/a -        line x  upoint --> (stream ubool) as inside
n/a -      upoint x  region --> (stream ubool)
n/a -      region x  upoint --> (stream ubool)

  inside: 
n/a +      upoint x uregion --> (stream ubool)
n/a +      upoint x  points --> (stream ubool)
n/a +      upoint x    line --> (stream ubool)
n/a +     uregion x  points --> (stream ubool)
n/a +     uregion x    line --> (stream ubool)

n/a + mdirection:    upoint --> ureal

OK  + no_components:     uT --> uint

n/a + area: uregion --> ureal             see TemporalLiftedAlgebra

Test+ and, or: ubool x ubool --> ubool
Test+           bool x ubool --> ubool
Test+          ubool x  bool --> ubool

      =, #, <, >, <=, >=: 
n/a +        uT x uT --> (stream ubool)

OK  + not:       ubool --> ubool

n/a   initial, final: (stream uT) --> iT
n/a   present: (stream uT) x instant --> bool
n/a   present: (stream uT) x periods --> bool
n/a + sometimes: ubool --> bool
n/a + never:     ubool --> bool
n/a + always:    ubool --> bool

n/a   uint2ureal:       uint --> ureal
n/a   int2real:          int --> real
n/a   floor:            real --> int
n/a   ceil:             real --> int
n/a   round:      (real int) --> real

COMMENTS:

(*):  These operators have been implemented for 
      T in {bool, int, real, point}
(**): These operators have been implemented for 
      T in {bool, int, real, point, string, region}

Key to STATE of implementation:

   OK : Operator has been implemented and fully tested
  (OK): Operator has been implemented and partially tested
  Test: Operator has been implemented, but tests have not been done
  Pre : Operator has not been functionally implemented, but 
        stubs (dummy code) exist
  n/a : Neither functionally nor dummy code exists for this ones

    + : Exists for according mType
    - : Does nor exist for according mType

----

*/

/*

August 2006, Christian D[ue]ntgen: Added missing checks for ~undefined~ 
argument values to all value mapping functions. The functions will now
return ~undefined~ result values for these cases.

Changed structure of the file to become ordered by operators rather than by
typemapping, valuemapping etc. This makes the file easier to extend.

*/

/*
0. Bug-List

----

(C)     intersection: (uint uint) -> (stream uint)
(C)     intersection:(uint int) -> (stream uint)
(C)     intersection: (int uint) -> (stream uint)


Key:
 (C): system crash
 (R): Wrong result

----

*/

/*

[TOC]

1 Overview

This file contains the implementation of the unit operators and
helping operators for indexing instant values in R-trees.

2 Defines, includes, and constants

*/


#include <cmath>
#include <stack>
#include <limits>
#include <sstream>
#include <vector>

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalExtAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "RectangleAlgebra.h"


extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;


#include "DateTime.h"
using namespace datetime;

bool TUA_DEBUG = false; // Set to true to activate debugging code
//bool TUA_DEBUG = true; // Set to true to activate debugging code

/*
2.1 Definition of some constants and auxiliary functions

*/
const double MAXDOUBLE = numeric_limits<double>::max();
const double MINDOUBLE = numeric_limits<double>::min();

/*
2.2 Auxiliary output functions

*/

// make a string from a numeric value
string TUn2s(const double& i)
{
  std::stringstream ss;
  std::string str;
  ss << i;
  ss >> str;
  return str;
}

// make a string representation from a time interval
string TUPrintTimeInterval( Interval<DateTime> iv )
{
  string Result;

  if (iv.lc)
    Result += "[";
  else 
    Result += "]";
  Result += iv.start.ToString();
  Result += ", ";
  Result += iv.end.ToString();
  if (iv.rc)
    Result += "]";
  else 
    Result += "[";
  return Result;
}

// make a string representation from a point
string TUPrintPoint( const Point& p )
{
  string Result;

  if ( p.IsDefined() )
    Result = "( def  : ";
  else 
    Result = "( undef: ";
  Result += TUn2s(p.GetX());
  Result += "/";
  Result += TUn2s(p.GetY());
  Result += " )";

  return Result;
}

// make a string representation from an upoint value
string TUPrintUPoint( const UPoint& upoint )
{
  std::string Result;

  if ( upoint.IsDefined() )
    Result = "( def  : ";
  else 
    Result = "( undef: ";
  Result += TUPrintTimeInterval(upoint.timeInterval);
  Result += ", ";
  Result += TUPrintPoint(upoint.p0);
  Result += ", ";
  Result += TUPrintPoint(upoint.p1);
  Result +=" )";
  Result += ((Attribute*)(&upoint))->AttrDelete2string();
  return Result;
}

// make a string representation from an ureal value
string TUPrintUReal( UReal* ureal )
{
  std::string Result;

  if ( ureal->IsDefined() )
    Result = "( def  : ";
  else 
    Result = "( undef: ";
  Result += TUPrintTimeInterval(ureal->timeInterval);
  Result += ", ";
  Result += TUn2s(ureal->a);
  Result += ", ";
  Result += TUn2s(ureal->b);
  Result += ", ";
  Result += TUn2s(ureal->c);
  if (ureal->r)
    Result += ", true )";
  else 
    Result +=", false )";
  Result += ureal->AttrDelete2string();
  return Result;
}

/*
3 Implementation of the unit class method operators

This section implements operators as member functions of the respective
unit class.

3.1 Operator ~speed~

*/
void MPoint::MSpeed( MReal& result ) const
{
  const UPoint *uPoint;
  UReal uReal;
  //  int counter = 0;

  result.Clear();
  if ( !IsDefined() )
    {
      // result.SetDefined( false ); // no undef Mappings by now
      if(TUA_DEBUG) cout << "\nMPoint::MSpeed is undef (undef arg)." << endl;
    }
  else
    {
      result.StartBulkLoad();

      for( int i = 0; i < GetNoComponents(); i++ )
        {
          Get( i, uPoint );

          uPoint->USpeed( uReal );
          if( uReal.IsDefined() )
            {
              result.Add( uReal ); // append ureal to mreal
              //              counter++;
            }
        }
      result.EndBulkLoad( true );
/*
Activating the following snippet would allow for creating undef objects
instead of empty ones. Alas, the SetDefined() and IsDefined() methods do
not have functional implementations by now. Instead, ~undef~ values are 
substituted by ~empty~ mappings.

----
      if(counter == 0)
        {
          result.SetDefined( false );
          if(TUA_DEBUG) cout << "\nMPoint::MSpeed is undef (empty result)." << endl;
        }
      else
        {        
          result.SetDefined( true );
          if(TUA_DEBUG) cout << "\nMPoint::MSpeed is defined." << endl;
        }

----

*/
    }
  if(TUA_DEBUG) cout << "MPoint::MSpeed() finished!" << endl;
}

void UPoint::USpeed( UReal& result ) const
{

  double x0, y0, x1, y1;
  double duration;

  if ( !IsDefined() )
    {
      result.SetDefined( false );
      if(TUA_DEBUG) cout << "\nUPoint::USpeed: undef (undef arg)." << endl;
    }
  else
    {
      
      x0 = p0.GetX();
      y0 = p0.GetY();
      
      x1 = p1.GetX();
      y1 = p1.GetY();
      
      result.timeInterval = timeInterval;
      
      DateTime dt = timeInterval.end - timeInterval.start;
      duration = dt.ToDouble() * 86400;   // value in seconds
      
      if(TUA_DEBUG) cout << "\nUPoint::USpeed duration=" 
                         << duration << "s." << endl;
      
      if( duration > 0.0 )
        {     
          /*
            The point unit can be represented as a function of
            f(t) = (x0 + x1 * t, y0 + y1 * t).
            The result of the derivation is the constant (x1,y1).
            The speed is constant in each time interval.
            Its value is represented by variable c. The variables a and b  
            are set to zero.
            
          */
          result.a = 0;  // speed is constant in the interval
          result.b = 0;
          result.c = sqrt(pow( (x1-x0), 2 ) + pow( (y1- y0), 2 ))/duration;
          result.r = false;
          result.SetDefined( true );
          if(TUA_DEBUG) cout << "\nUPoint::USpeed is defined." << endl;
        }
      else
        {
          result.SetDefined( false );
          if(TUA_DEBUG) cout 
            << "\nUPoint::USpeed is undef (empty result)." << endl;
        }
    }
}

/*
3.2 Operator ~Velocity~

*/
void MPoint::MVelocity( MPoint& result ) const
{
  const UPoint *uPoint;
  UPoint p;
  //  int counter = 0;

  result.Clear();
  if ( !IsDefined() )
    {
      // result.SetDefined( false ); // no undef Mappings by now
      if(TUA_DEBUG) cout << "\nMPoint::MVelocity: undef unit" << endl;
    }
  else
    {
      result.StartBulkLoad();
      for( int i = 0; i < GetNoComponents(); i++ )
        {
          Get( i, uPoint );
          /*
            Definition of a new point unit p. The velocity is constant
            at all times of the interval of the unit. This is exactly
            the same as within operator ~speed~. The result is a vector
            and can be represented as a upoint.
            
          */
          
          uPoint->UVelocity( p );
          if( p.IsDefined() )
            {
              result.Add( p );
              //              counter++;
            }
        }
      result.EndBulkLoad( true );

/*
Activating the following snippet would allow for creating undef objects
instead of empty ones. Alas, the SetDefined() and IsDefined() methods do
not have functional implementations by now. Instead, ~undef~ values are 
substituted by ~empty~ mappings.

----
      if(counter>0)
        {
          result.SetDefined( true );
          if(TUA_DEBUG) cout << "\nMPoint::MVelocity result defined." << endl;
        }
      else // counter == 0
        {
          result.SetDefined( false );
          if(TUA_DEBUG) cout << "\nMPoint::MVelocity result empty." << endl;
        }

----

*/

    }
  if(TUA_DEBUG) cout << "MPoint::MVelocity() finished!" << endl;
}


void UPoint::UVelocity( UPoint& result ) const
{

  double x0, y0, x1, y1;
  double duration;

  if ( ! IsDefined() )    
    {
      result.SetDefined( false );
      if(TUA_DEBUG) cout << "\nUPoint::UVelocity undef (undef arg)." << endl;
    }
  else
    {
      x0 = p0.GetX();
      y0 = p0.GetY();
  
      x1 = p1.GetX();
      y1 = p1.GetY();
  
      DateTime dt = timeInterval.end - timeInterval.start;
      duration = dt.ToDouble() * 86400;   // value in seconds

      if(TUA_DEBUG) cout << "\nUPoint::UVelocity duration=" 
                         << duration << "s." << endl;

      if( duration > 0.0 )
        {
          if(TUA_DEBUG) cout << "\nUPoint::UVelocity result defined." << endl;
          UPoint p(timeInterval,
                   (x1-x0)/duration,(y1-y0)/duration, // velocity is constant
                   (x1-x0)/duration,(y1-y0)/duration  // throughout the unit
                  );
          p.SetDefined( true );
          result.CopyFrom( &p );
          result.SetDefined( true );
        }
      else
        {
          if(TUA_DEBUG) cout << "\nUPoint::UVelocity undef (no result)." 
                             << endl;
          UPoint p(timeInterval,0,0,0,0);
          result.CopyFrom( &p );
          result.SetDefined( false );
        }
    }
}

/*
3.6 Operator ~Trajectory~

This function is introduced as an additional memberfunction of class
~UPoint~:

*/

void UPoint::UTrajectory( Line& line ) const
{
  HalfSegment hs;
  int edgeno = 0;

  line.Clear();
  line.StartBulkLoad();      
  if ( !IsDefined() )
    line.SetDefined( false ); // by now w/o functionality
  else 
    {      
      if( !AlmostEqual( p0, p1 ) )
        {
          hs.Set( true, p0, p1 );
          hs.attr.edgeno = ++edgeno;
          line += hs;
          hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
          line += hs;          
        }
      line.SetDefined( true ); // by now w/o functionality
    }
  line.EndBulkLoad();
}


/*
4 General Selection functions

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function within the operator's ~ValueMapping~ array, being able to deal 
with the respective combination of input parameter types.

Non-overloaded opertors (having only one single signature) do not need a
selection function or a ValueMapping array.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

Only some more general selection functions are listed here. More of them can
be found in the operator implementation section below.

4.1 Selection function ~UnitSimpleSelect~

Is used for the ~deftime~,~atinstant~,~atperiods~, ~initial~, ~final~  
operations.

*/
int
UnitSimpleSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "ubool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "uint" )
    return 1;
  if( nl->SymbolValue( arg1 ) == "ureal" )
    return 2;
  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 3;
  if( nl->SymbolValue( arg1 ) == "ustring" )
    return 4;
  if( nl->SymbolValue( arg1 ) == "uregion" )
    return 5;
  
  return -1; // This point should never be reached
}

/*

4.2 Selection function ~UnitCombinedUnitStreamSelect~

This extended version of ~UnitSimpleSelect~ can map (unit) as well as 
(stream unit):

*/

int
UnitCombinedUnitStreamSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if (nl->IsAtom( arg1 ) )
    {
      if( nl->SymbolValue( arg1 ) == "ubool" )
        return 0;
      if( nl->SymbolValue( arg1 ) == "uint" )
        return 1;
      if( nl->SymbolValue( arg1 ) == "ureal" )
        return 2;
      if( nl->SymbolValue( arg1 ) == "upoint" )
        return 3;
      if( nl->SymbolValue( arg1 ) == "ustring" )
        return 4;
      if( nl->SymbolValue( arg1 ) == "uregion" )
        return 5;
    }

  if(   !( nl->IsAtom( arg1 ) )
      && ( nl->ListLength(arg1) == 2 )
      && ( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
    { if( nl->IsEqual( nl->Second(arg1), "ubool" ) )
        return 4;
      if( nl->IsEqual( nl->Second(arg1), "uint" ) )
        return 5;
      if( nl->IsEqual( nl->Second(arg1), "ureal" ) )
        return 6;
      if( nl->IsEqual( nl->Second(arg1), "upoint" ) )
        return 7;
      if( nl->IsEqual( nl->Second(arg1), "ustring" ) )
        return 8;
      if( nl->IsEqual( nl->Second(arg1), "uregion" ) )
        return 9;
    }
  
  return -1; // This point should never be reached
}


/*
5 Implementation of Algebra Operators

5.1 Operator ~speed~

5.1.1 Type mapping function for ~speed~

Type mapping for ~speed~ is

----  mpoint  [->]  mreal

----

*/
ListExpr
TypeMapSpeed( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "mpoint" )  )
      return nl->SymbolAtom( "mreal" );
    if( nl->IsEqual( arg1, "upoint" )  )
      return nl->SymbolAtom( "ureal" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/* 
5.1.2 Value mapping for operator ~speed~

*/

int MPointSpeed(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  MReal  *res   = (MReal*)result.addr;
  MPoint *input = (MPoint*)args[0].addr;
  
  if ( input->IsDefined() )
    // call member function:
    input->MSpeed( *res ); 
  else
    {
      res->Clear();             // using empty mvalue instead
      //res->SetDefined(false); // of undef mvalue
    }
  return 0;
}

int UnitPointSpeed(Word* args,Word& result,int message,Word& local,Supplier s)
{
  result = qp->ResultStorage( s );
  UReal  *res   = (UReal*)result.addr;
  UPoint *input = (UPoint*)args[0].addr;
  
  if ( input->IsDefined() )
    { // call member function:
      input->USpeed( *res );
      if (TUA_DEBUG) 
        cout << "UnitPointSpeed(): input def" << endl;
    }  
  else
    {
      res->SetDefined(false);
      if (TUA_DEBUG) 
        cout << "UnitPointSpeed(): input undef" << endl;
    }
  return 0;
}

/* 
5.1.3 Specification for operator ~speed~

*/

const string
TemporalSpecSpeed  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mpoint -> mreal\n"
"upoint -> ureal</text--->"
"<text>speed( _ )</text--->"
"<text>return the speed of a temporal spatial object in "
"unit/s.</text--->"
"<text>query speed(mp1)</text---> ) )";

/* 
5.1.4 Selection Function of operator ~speed~

*/
int
SpeedSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );

  if( nl->SymbolValue( arg1 ) == "mpoint" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "upoint" )
    return 1;

  return -1; // This point should never be reached
}

ValueMapping temporalspeedmap[] = { MPointSpeed,
                                    UnitPointSpeed };

/* 
5.1.5 Definition of operator ~speed~

*/
Operator temporalspeed( "speed",
                      TemporalSpecSpeed,
                      2,
                      temporalspeedmap,
                      SpeedSelect,
                      TypeMapSpeed);

/*
5.2 Operator ~queryrect2d~

5.2.1 Type Mapping for ~queryresct2d~

Type mapping for ~queryrect2d~ is

----  instant  [->]  rect

----

*/
ListExpr
InstantTypeMapQueryrect2d( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "instant" )  )
      return nl->SymbolAtom( "rect" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.2.2 Value Mapping for ~queryrect2d~

*/

int Queryrect2d(Word* args, Word& result, int message, Word& local, Supplier s)
{
  const unsigned dim = 2;
  double min[dim], max[dim];
  double timevalue;
  double mininst, maxinst;
  Instant* Inv = (Instant*)args[0].addr;
  DateTime tmpinst;

  tmpinst.ToMinimum(); 
  mininst = tmpinst.ToDouble();
  tmpinst.ToMaximum();
  maxinst = tmpinst.ToDouble();

  result = qp->ResultStorage( s );
  
  if (Inv->IsDefined()) // Inv is defined: create rect2
    {
      timevalue = Inv->ToDouble();
      
      min[0] = mininst;   // x1
      min[1] = timevalue; // x2
      max[0] = timevalue; // y1
      max[1] = maxinst;   // y2
     
      Rectangle<dim>* rect = new Rectangle<dim>(true, min, max);
      ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
      delete rect;
    }
  else // Inv is undefined: return mininst^4
    {
      min[0] = mininst;
      min[1] = mininst;
      max[0] = mininst;
      max[1] = mininst;
    
      Rectangle<dim>* rect = new Rectangle<dim>(false, min, max);
      ((Rectangle<dim>*)result.addr)->CopyFrom(rect);
      delete rect;
    }
  
  return 0;
}

/*
5.2.3 Specification for operator ~queryrect2d~

*/

const string
TemporalSpecQueryrect2d  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(instant) -> rect</text--->"
"<text>queryrect2d( _ )</text--->"
"<text>Translate an instant object to a rect object to query the against "
"a periods object translated to a point using operator 'point2d'. The "
"undef instant is mapped to rect mininst^4</text--->"
"<text>query queryrect2d(instant)</text---> ) )";

/* 
5.2.4 Selection Function of operator ~queryrect2d~

Not necessary.

*/

/*
5.2.5  Definition of operator ~queryrect2d~

*/

Operator temporalunitqueryrect2d( "queryrect2d",
                      TemporalSpecQueryrect2d,
                      Queryrect2d,
                      Operator::SimpleSelect,
                      InstantTypeMapQueryrect2d);

/*
5.3 Operator ~point2d~

5.3.1 Type Mapping for ~point2d~

Type mapping for ~point2d~ is

----  periods  [->]  point

----

*/

ListExpr
PeriodsTypeMapPoint2d( ListExpr args )
{

  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "periods" )  )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.3.2 Value Mapping for ~point2d~

*/
int Point2d( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double  X, Y;
  Instant sup, inf;

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

  if ( !range->IsDefined() )
    ((Point*)result.addr)->SetDefined( false );
  else 
    {
      X = 0; 
      Y = 0;
      if( !range->IsEmpty()  )
        {
          const Interval<Instant> *intv1, *intv2;
          
          range->Get( 0, intv1 );
          range->Get( range->GetNoComponents()-1, intv2 );          
          sup = intv1->end;
          inf = intv2->start;
          Y = sup.ToDouble(); // Derives the maximum of all intervals.
          X = inf.ToDouble(); // Derives the minimum of all intervals.
        }
      else
        {
          DateTime tmpinst = DateTime(0,0,instanttype);
          tmpinst.ToMinimum(); 
          X = tmpinst.ToDouble();          
          Y = X;
        }
      ((Point*)result.addr)->SetDefined(true);          
      ((Point*)result.addr)->Set(X,Y); // Returns the calculated point.
    }
  
  return 0;
}


/*
5.3.3 Specification for operator ~point2d~

*/
const string
TemporalSpecPoint2d  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(periods) -> point</text--->"
"<text>point2d( _ )</text--->"
"<text>Translate a periods value to a point value representing "
"the period's total deftime interval. The empty periods value "
"is mapped to the point corresponding to mininstant^2.</text--->"
"<text>query point2d(periods)</text---> ) )";


/* 
5.3.4 Selection Function of operator ~point2d~

Not necessary.

*/

/*
5.3.5  Definition of operator ~point2d~

*/
Operator temporalunitpoint2d( "point2d",
                      TemporalSpecPoint2d,
                      Point2d,
                      Operator::SimpleSelect,
                      PeriodsTypeMapPoint2d);

/*
5.4 Operator ~size~

5.4.1 Type Mapping for ~size~

Type mapping for ~size~ is

----  periods  [->]  real

----

*/
ListExpr
PeriodsTypeMapSize( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "periods" )  )
      return nl->SymbolAtom( "real" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.4.2 Value Mapping for ~size~

*/
int Size( Word* args, Word& result, int message, Word& local, Supplier s )
{
  double   res = 0.0;
  DateTime dur = DateTime(0, 0, durationtype);

  result = qp->ResultStorage( s );
  Periods* range = (Periods*)args[0].addr;

  if ( !range->IsDefined() )
    ((CcReal*)result.addr)->SetDefined( false );
  else 
    {
      if( !range->IsEmpty()  )
        {
          const Interval<Instant> *intv;
      
          for( int i = 0; i < range->GetNoComponents(); i++ )
            {
              range->Get( i, intv );
              dur += (intv->end - intv->start);
            }
          // transform to seconds
          res = abs(dur.ToDouble())*86400;
        }
      ((CcReal*)result.addr)->Set(true, res);  // return the result
    }
  return 0;
}

/*
5.4.3 Specification for operator ~size~

*/
const string
TemporalSpecSize  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(periods) -> real</text--->"
"<text>size( _ )</text--->"
"<text>Return the duration in seconds spanned by a periods value "
"as a real value.</text--->"
"<text>query size(periods)</text---> ) )";

/* 
5.4.4 Selection Function of operator ~size~

Not necessary.

*/


/*
5.4.5  Definition of operator ~size~

*/
Operator temporalunitsize( "size",
                      TemporalSpecSize,
                      Size,
                      Operator::SimpleSelect,
                      PeriodsTypeMapSize );

/*
5.5 Operator ~makemvalue~

This operator creates a moving object type mT from a stream of unit type 
objects uT. The operator does not expect the stream to be ordered by their
timeintervals. Also, undefined units are allowed (but will be ignored).
If the stream contains amindst 2 units with overlapping timeIntervals,
the operator might crash. If the stream is empty, the result will be an 
empty mT (It would be better to create an undef mT, but the needed methods
have not been functionally implemented by now).

5.5.1 Type Mapping for ~makemvalue~

Type mapping for ~makemvalue~ is

----  stream (tuple ([x1:t1,x1:ubool,..,[xn:tn)))   ->  mbool
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:uint,..,[xn:tn)))    ->  mint
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:ureal,..,[xn:tn)))   ->  mreal
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:upoint,..,[xn:tn)))  ->  mpoint
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:ustring,..,[xn:tn)))  ->  mstring
              APPEND (i ti)
      stream (tuple ([x1:t1,x1:uregion,..,[xn:tn)))  ->  mregion
              APPEND (i ti)

----

*/
ListExpr MovingTypeMapMakemvalue( ListExpr args )

{
  ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr, listfull, attrtype;

  int j;
  string argstr, argstr2, attrname, inputtype, inputname, fulllist;


  //check the list length.
  CHECK_COND(nl->ListLength(args) == 2,
             "Operator makemvalue expects a list of length two.");
  
  first = nl->First(args);
  nl->WriteToString(argstr, first);


  // check the structure of the list.
  CHECK_COND(nl->ListLength(first) == 2  &&
             (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
             (nl->ListLength(nl->Second(first)) == 2) &&
             (IsTupleDescription(nl->Second(nl->Second(first)))),
  "Operator makemvalue expects as first argument a list with structure "
  "(stream (tuple ((a1 t1)...(an tn))))\n"
  "Operator makemvalue gets as first argument '" + argstr + "'." );


  // check the given parameter
  second  = nl->Second(args);
  nl->WriteToString(argstr, second);
  CHECK_COND(argstr != "typeerror",
  "Operator makemvalue expects a name of a attribute and not a type.");

  // inputname represented the name of the given attribute
  nl->WriteToString(inputname, second);

  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  firstr = nl->First(rest);
  rest = nl->Rest(rest);
  first2 = nl->First(firstr);
  second2 = nl->Second(firstr);
  nl->WriteToString(attrname, first2);
  nl->WriteToString(argstr2, second2);

  // compare with the attributes in the relation
  if (attrname == inputname)
     inputtype = argstr2;

  // save from the detected attribute the type
  while (!(nl->IsEmpty(rest)))
    {
      lastlistn = nl->Append(lastlistn,nl->First(rest));
      firstr = nl->First(rest);
      rest = nl->Rest(rest);
      first2 = nl->First(firstr);
      second2 = nl->Second(firstr);
      nl->WriteToString(attrname, first2);
      nl->WriteToString(argstr2, second2);

      // compare with the attributes in the relation
      if (attrname == inputname)
        inputtype = argstr2;

      // save from the detected attribute the type
    }
  rest = second;
  listfull = listn;
  nl->WriteToString(fulllist, listfull);
  

  CHECK_COND(!(inputtype == "") ,
             "Operator makemvalue: Attribute name '"+ inputname+
             "' is not known.\n"+
             "Known Attribute(s): " + fulllist);
  
  CHECK_COND( (inputtype == "ubool"
               || inputtype == "uint"
               || inputtype == "ureal"
               || inputtype == "upoint" 
               || inputtype == "ustring"
               || inputtype == "uregion"),
              "Attribute type is not of type ubool, uint, ustring, ureal"
              "upoint or uregion.");
  
  attrname = nl->SymbolValue(second);
  j = FindAttribute(nl->Second(nl->Second(first)), attrname, attrtype);
  
  assert( j !=0 );
  
  if( inputtype == "upoint" )
    attrtype = nl->SymbolAtom( "mpoint" ) ;
  if( inputtype == "ubool" )
    attrtype = nl->SymbolAtom( "mbool" );
  if( inputtype == "ureal" )
    attrtype = nl->SymbolAtom( "mreal" );
  if( inputtype == "uint" )
    attrtype = nl->SymbolAtom( "mint" );
  if( inputtype == "ustring" )
    attrtype = nl->SymbolAtom( "mstring" );
  if( inputtype == "uregion" )
    attrtype = nl->SymbolAtom( "mregion" );
  
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
           nl->TwoElemList(nl->IntAtom(j),
           nl->StringAtom(nl->SymbolValue(attrtype))), attrtype);
  
  // Appending the number of the attribute in the relation is very important,
  // because in the other case we can't work with it in the value function.
}

/*
5.5.2 Value Mapping for ~makemvalue~

*/

template <class Mapping, class Unit>
int MappingMakemvalue(Word* args,Word& result,int message,
                      Word& local,Supplier s)
{
  Mapping* m;
  Unit* unit;
  //  int definedcounter = 0;

  Word currentTupleWord;

  assert(args[2].addr != 0); // assert existence of input
  assert(args[3].addr != 0); // assert existence of input

  int attributeIndex = ((CcInt*)args[2].addr)->GetIntval() - 1;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  result = qp->ResultStorage(s);

/*
  ~Mapping~ is a template function for many datatypes.
  The mapping class can cast moving datatypes.
  The unit class can cast  unit datatypes.

*/
  
  m = (Mapping*) result.addr;
  m->Clear();
  m->StartBulkLoad();

  while ( qp->Received(args[0].addr) ) // get all tuples
    {
      Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
      Attribute* currentAttr = (Attribute*)currentTuple->
        GetAttribute(attributeIndex);
      
      if(currentAttr->IsDefined())
        {
          //          definedcounter++;
          unit = (Unit*) currentAttr;
          m->Add( *unit );
          currentTuple->DeleteIfAllowed();
        }
      qp->Request(args[0].addr, currentTupleWord);
    }
  m->EndBulkLoad( true ); // force Mapping to sort the units

/*
It would be better to use the following snippet, but the ~undefined~ flag
and the SetDefined() and IsDefined() methods only have dummy implementations
by now. Instead, ~undef~ values are substituted by ~empty~ mappings.
 
----
  if(definedcounter > 0 )
    {
      m->SetDefined( true );
    }
  else
    {
      m->SetDefined( false );
    }

----

*/  

  qp->Close(args[0].addr);

  return 0;

}

/*
5.5.3 Specification for operator ~makemvalue~

*/
const string
TemporalSpecMakemvalue  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, real, point, region}:"
"((stream (tuple ((x1 t1)...(xn tn)))"
" (uT)))-> mT</text--->"
"<text>_ makemvalue[ _ ]</text--->"
"<text>Create a moving object from a (not necessarily sorted) "
"tuple stream containing units. "
"No two unit timeintervals may overlap. Undefined units are "
"allowed and will be ignored. A stream with less than 1 defined "
"unit will result in an 'empty' moving object, not in an 'undef'.</text--->"
"<text>query units(zug5) transformstream makemvalue[elem]</text---> ) )";

/* 
5.5.4 Selection Function of operator ~makemvalue~

*/
int
MakemvalueSelect( ListExpr args )
{

 ListExpr first, second, rest, listn,
           lastlistn, first2, second2, firstr;


  string argstr, argstr2, attrname, inputtype, inputname;


  first = nl->First(args);
  second  = nl->Second(args);
  nl->WriteToString(argstr, first);


  nl->WriteToString(inputname, second);


  rest = nl->Second(nl->Second(first));
  listn = nl->OneElemList(nl->First(rest));
  lastlistn = listn;
  firstr = nl->First(rest);
  rest = nl->Rest(rest);
  first2 = nl->First(firstr);
  second2 = nl->Second(firstr);
  nl->WriteToString(attrname, first2);
  nl->WriteToString(argstr2, second2);
  if (attrname == inputname)
     inputtype = argstr2;

while (!(nl->IsEmpty(rest)))
  {
     lastlistn = nl->Append(lastlistn,nl->First(rest));
     firstr = nl->First(rest);
     rest = nl->Rest(rest);
     first2 = nl->First(firstr);
     second2 = nl->Second(firstr);
     nl->WriteToString(attrname, first2);
     nl->WriteToString(argstr2, second2);
     if (attrname == inputname)
         inputtype = argstr2;
  }

  if( inputtype == "ubool" )
    return 0;

  if( inputtype == "uint" )
    return 1;

  if( inputtype == "ureal" )
    return 2;

  if( inputtype == "upoint" )
    return 3;

  if( inputtype == "ustring" )
    return 4;

  if( inputtype == "uregion" )
    return 5;


  return -1; // This point should never be reached
}

ValueMapping temporalmakemvaluemap[] = {
      MappingMakemvalue<MBool, UBool>,
      MappingMakemvalue<MBool, UBool>,
      MappingMakemvalue<MReal, UReal>,
      MappingMakemvalue<MPoint, UPoint>,
      MappingMakemvalue<MString, UString>,
      MappingMakemvalue<MRegion, URegionEmb> };

/*
5.5.5  Definition of operator ~makemvalue~

*/
Operator temporalunitmakemvalue( "makemvalue",
                        TemporalSpecMakemvalue,
                        6,
                        temporalmakemvaluemap,
                        MakemvalueSelect,
                        MovingTypeMapMakemvalue );

/*
5.6 Operator ~trajectory~

5.6.1 Type Mapping for ~trajectory~

*/
ListExpr
UnitTrajectoryTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "upoint" ) )
      return nl->SymbolAtom( "line" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.6.2 Value Mapping for ~trajectory~

*/
int UnitPointTrajectory(Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Line   *line   = ((Line*)result.addr);
  UPoint *upoint = ((UPoint*)args[0].addr);

  line->Clear();                // clear result
  if ( upoint->IsDefined() )
    upoint->UTrajectory( *line );   // call memberfunction
  else 
    {
      // line->Clear();             // Use empty value
      // line->SetDefined( false ); // instead of undef
    }
  return 0;
}

/*
5.6.3 Specification for operator ~trajectory~

*/
const string
TemporalSpecTrajectory  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>upoint -> line</text--->"
"<text>trajectory( _ )</text--->"
"<text>get the trajectory of the corresponding"
"unit point object. Static or undef upoint objects "
"yield empty line objects.</text--->"
"<text>trajectory( up1 )</text---> ) )";

/* 
5.6.4 Selection Function of operator ~trajectory~

Not necessary.

*/

/*
5.6.5  Definition of operator ~trajectory~

*/
Operator temporalunittrajectory( "trajectory",
                          TemporalSpecTrajectory,
                          UnitPointTrajectory,
                          Operator::SimpleSelect,
                          UnitTrajectoryTypeMap );

/*
5.7 Operator ~deftime~

5.7.1 Type Mapping for ~deftime~

*/
ListExpr
UnitTypeMapPeriods( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

   if(  nl->IsEqual( arg1, "ubool" )  ||
        nl->IsEqual( arg1, "uint" )   ||
        nl->IsEqual( arg1, "ureal" )  ||
        nl->IsEqual( arg1, "upoint" ) ||
        nl->IsEqual( arg1, "ustring" ) ||
        nl->IsEqual( arg1, "uregion" ) )
   return nl->SymbolAtom( "periods" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.7.2 Value Mapping for ~deftime~

*/
template <class Mapping>
int MappingUnitDefTime( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Periods* r = ((Periods*)result.addr);
  Mapping* m = ((Mapping*)args[0].addr);

  if ( !m->IsDefined() )
    r->SetDefined( false );
  else
    {
      assert( r->IsOrdered() );
  
      r->Clear();
      r->StartBulkLoad();
      r->Add( m->timeInterval );
      r->EndBulkLoad( false );
    }
  return 0;
}

/*
5.7.3 Specification for operator ~deftime~

*/
const string
TemporalSpecDefTime  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>uT -> periods \n"
"(for T in {bool, int, real, string, point, region})</text--->"
"<text>deftime( _ )</text--->"
"<text>get the definition time for the "
" unit data objects.</text--->"
"<text>deftime( up1 )</text---> ) )";

/* 
5.7.4 Selection Function of operator ~deftime~

Uses ~UnitSimpleSelect~.

*/

ValueMapping temporalunitdeftimemap[] = { MappingUnitDefTime<UBool>,
                                          MappingUnitDefTime<UInt>,
                                          MappingUnitDefTime<UReal>,
                                          MappingUnitDefTime<UPoint>,
                                          MappingUnitDefTime<UString>,
                                          MappingUnitDefTime<URegion>};


/*
5.7.5  Definition of operator ~deftime~

*/

Operator temporalunitdeftime( "deftime",
                          TemporalSpecDefTime,
                          6,
                          temporalunitdeftimemap,
                          UnitSimpleSelect,
                          UnitTypeMapPeriods );
/*
5.8 Operator ~atinstant~

5.8.1 Type Mapping for ~atinstant~

*/
ListExpr
UnitInstantTypeMapIntime( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) )
    {
       if( nl->IsEqual( arg1, "ubool" ) )
        return nl->SymbolAtom( "ibool" );

      if( nl->IsEqual( arg1, "uint" ) )
        return nl->SymbolAtom( "iint" );

      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->SymbolAtom( "ireal" );

      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->SymbolAtom( "ipoint" );

      if( nl->IsEqual( arg1, "ustring" ) )
        return nl->SymbolAtom( "istring" );

      if( nl->IsEqual( arg1, "uregion" ) )
        return nl->SymbolAtom( "iregion" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.8.2 Value Mapping for ~atinstant~

*/
template <class Mapping, class Alpha>
int MappingUnitAtInstant( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;
  Mapping* posUnit = ((Mapping*)args[0].addr);
  Instant* t = ((Instant*)args[1].addr);
  Instant t1 = *t;

  if ( !t->IsDefined() || !posUnit->IsDefined() ) 
    pResult->SetDefined( false );
  else if( posUnit->timeInterval.Contains(t1) )
    {
      posUnit->TemporalFunction( *t, pResult->value );
      pResult->instant = *t;
      pResult->SetDefined( true );
    }
  else    // instant not contained by deftime interval
    pResult->SetDefined( false );
  
  return 0;
}

/*
5.8.3 Specification for operator ~atinstant~

*/
const string
TemporalSpecAtInstant  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(uT instant) -> iT\n"
"(T in {bool, int, real, string, point, region})</text--->"
"<text>_ atinstant _ </text--->"
"<text>From a unit type, get the Intime value corresponding to "
"the given instant.</text--->"
"<text>upoint1 atinstant instant1</text---> ) )";

/* 
5.8.4 Selection Function of operator ~atinstant~

Uses ~UnitSimpleSelect~.

*/



ValueMapping temporalunitatinstantmap[] =
  {MappingUnitAtInstant<UBool, CcBool>,
   MappingUnitAtInstant<UInt, CcInt>,
   MappingUnitAtInstant<UReal, CcReal>,
   MappingUnitAtInstant<UPoint, Point>,
   MappingUnitAtInstant<UString, CcString>,
   MappingUnitAtInstant<URegion, Region>};


/*
5.8.5  Definition of operator ~atinstant~

*/
Operator temporalunitatinstant( "atinstant",
                            TemporalSpecAtInstant,
                            6,
                            temporalunitatinstantmap,
                            UnitSimpleSelect,
                            UnitInstantTypeMapIntime );

/*
5.9 Operator ~atperiods~

5.9.1 Type Mapping for ~atperiods~

*/
ListExpr
UnitPeriodsTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr;

  if ( nl->ListLength( args ) != 2 )
    {
      ErrorReporter::ReportError("Operator atperiods expects "
                                 "a list of length 2.");
      return nl->SymbolAtom( "typeerror" );
    }

  arg1 = nl->First( args );
  arg2 = nl->Second( args );
  
  nl->WriteToString(argstr, arg2);
  if ( !( nl->IsEqual( arg2, "periods" ) ) )
    {
      ErrorReporter::ReportError("Operator atperiods expects a second argument"
                             " of type 'periods' but gets '" + argstr + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if( nl->IsAtom( arg1 ) )
    {
      if( nl->IsEqual( arg1, "ubool" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("ubool"));
      if( nl->IsEqual( arg1, "uint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("uint"));
      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("ureal"));
      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
                               nl->SymbolAtom("upoint"));
      if( nl->IsEqual( arg1, "ustring" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("ustring"));
      if( nl->IsEqual( arg1, "uregion" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
               nl->SymbolAtom("uregion"));

      nl->WriteToString(argstr, arg1);
      ErrorReporter::ReportError("Operator atperiods expects a first argument "
                                 "of type T in {ubool, uint, ureal, upoint, "
                                 "ustring, uregion} but gets a '" 
                                 + argstr + "'.");
      return nl->SymbolAtom( "typeerror" );
    }

  if( !( nl->IsAtom( arg1 ) ) && ( nl->ListLength( arg1 ) == 2) ) 
    {
      nl->WriteToString(argstr, arg1);
      if ( !( TypeOfRelAlgSymbol(nl->First(arg1)) == stream ) )
        {
          ErrorReporter::ReportError("Operator atperiods expects as first "
                                     "argument a list with structure 'T' or "
                                     "'stream(T)', T in {ubool, uint, ureal, "
                                     "upoint, ustring, ureagion} but gets a "
                                     "list with structure '" + argstr + "'.");
          return nl->SymbolAtom( "typeerror" );
        }
      
      if( nl->IsEqual( nl->Second(arg1), "ubool" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
                               nl->SymbolAtom("ubool"));
      if( nl->IsEqual( nl->Second(arg1), "uint" ) )
        return nl->TwoElemList(nl->SymbolAtom("stream"),
                               nl->SymbolAtom("uint"));
      if( nl->IsEqual( nl->Second(arg1), "ureal" ) )
         return nl->TwoElemList(nl->SymbolAtom("stream"),
                                nl->SymbolAtom("ureal"));
      if( nl->IsEqual( nl->Second(arg1), "upoint" ) )
         return nl->TwoElemList(nl->SymbolAtom("stream"),
                                 nl->SymbolAtom("upoint"));
      if( nl->IsEqual( nl->Second(arg1), "ustring" ) )
         return nl->TwoElemList(nl->SymbolAtom("stream"),
                                 nl->SymbolAtom("ustring"));
      if( nl->IsEqual( nl->Second(arg1), "uregion" ) )
         return nl->TwoElemList(nl->SymbolAtom("stream"),
                                 nl->SymbolAtom("uregion"));

      nl->WriteToString(argstr, nl->Second(arg1));
      ErrorReporter::ReportError("Operator atperiods expects a type "
                              "(stream T); T in {ubool, uint, ureal, upoint, "
                              "ustring, uregion} but gets '(stream " 
                              + argstr + ")'.");
      return nl->SymbolAtom( "typeerror" );
    };

  nl->WriteToString( argstr, args );
  ErrorReporter::ReportError("Operator atperiods encountered an "
                             "unmatched typerror for arguments '"
                             + argstr + "'.");
  return nl->SymbolAtom( "typeerror" );
}

/*
5.9.2 Value Mapping for ~atperiods~

*/
struct AtPeriodsLocalInfo
{
  Word uWord;     // the address of the unit value
  Word pWord;    //  the adress of the periods value
  int  j;       //   save the number of the interval
};

/*
Variant 1: first argument is a scalar value

*/

template <class Alpha>
int MappingUnitAtPeriods( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  AtPeriodsLocalInfo *localinfo;
  const Interval<Instant> *interval;

  Alpha* unit;
  Alpha r;
  Periods* periods;


  switch( message )
  {
  case OPEN:

    localinfo = new AtPeriodsLocalInfo;
    localinfo->uWord = args[0];
    localinfo->pWord = args[1];
    localinfo->j = 0;
    local = SetWord(localinfo);
    return 0;

  case REQUEST:
    
    if (TUA_DEBUG) cout << "\nMappingUnitAtPeriods: REQUEST" << endl;
    if( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfo *)local.addr;
    unit = (Alpha*)localinfo->uWord.addr;
    periods = (Periods*)localinfo->pWord.addr;
    
    if( !unit->IsDefined()    || 
        !periods->IsDefined() ||   // as a set-valued type, periods cannot be
        periods->IsEmpty()       ) // undefined, but only empty
      return CANCEL;
    if (TUA_DEBUG) 
      cout << "   Unit's timeInterval u=" 
           << TUPrintTimeInterval( unit->timeInterval ) << endl;
    if( localinfo->j == periods->GetNoComponents() )
      {
        if (TUA_DEBUG) 
          cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (1)" 
               << endl;
        return CANCEL;
      }
    periods->Get( localinfo->j, interval );
    if (TUA_DEBUG) cout << "   Probing timeInterval p =" 
                        << TUPrintTimeInterval(*interval)
                        << endl;
    while( interval->Before( unit->timeInterval ) && 
           localinfo->j < periods->GetNoComponents() )
      {
        localinfo->j++,
        periods->Get(localinfo->j, interval);
        if (TUA_DEBUG) 
          {
             cout << "   Probing timeInterval=" 
                  << TUPrintTimeInterval(*interval)
                  << endl;
             if (interval->Before( unit->timeInterval ))
               cout << "     p is before u" << endl;
             if (localinfo->j < periods->GetNoComponents())
               cout << "   j < #Intervals" << endl;
          }
      }

    if( localinfo->j >= periods->GetNoComponents() ) {
      result.addr = 0;
      if (TUA_DEBUG) 
        cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (2)"
             << endl;
      return CANCEL;
    }
    
    if( unit->timeInterval.Before( *interval ) )
      {
        result.addr = 0;
        if (TUA_DEBUG) 
          cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (3)" 
               << endl;
        return CANCEL;
      }
    else
      {
        // create unit restricted to interval
        unit->AtInterval( *interval, r );
        Alpha* aux = new Alpha( r );
        result = SetWord( aux );
        localinfo->j++;
        if (TUA_DEBUG) 
          {
            cout << "   Result interval=" 
                 << TUPrintTimeInterval(aux->timeInterval) 
                 << endl;
            cout << "   Result defined=" << aux->IsDefined()
                 << endl;
            cout << "MappingUnitAtPeriods: REQUEST finished: YIELD" 
                 << endl;
          }
        return YIELD;
      }
    
    if (TUA_DEBUG) 
      cout << "MappingUnitAtPeriods: REQUEST finished: CANCEL (4)" 
           << endl;
    return CANCEL; // should not happen
    
  case CLOSE:
    
    if( local.addr != 0 )
      delete (AtPeriodsLocalInfo *)local.addr;
    return 0;
  }
  // should not happen:
  return -1;
}

/*
Variant 2: first argument is a stream
Implemented on 07/17/2006 by Christian D[ue]ntgen

*/

struct AtPeriodsLocalInfoUS
{
  Word uWord;  // address of the input stream
  Word pWord;  // address of the input periods value
  int j;       // interval counter for within periods
};


template <class Alpha>
int MappingUnitStreamAtPeriods( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  AtPeriodsLocalInfoUS *localinfo;
  Alpha *unit, *aux;
  Alpha resultUnit;
  Periods *periods;
  const Interval<Instant> *interval;
  bool foundUnit = false;

  switch( message )
  { 
  case OPEN:
    localinfo = new AtPeriodsLocalInfoUS;
    localinfo->pWord = args[1]; 
    localinfo->j = 0;                              // init interval counter
    qp->Open( args[0].addr );                      // open stream of units
    qp->Request( args[0].addr, localinfo->uWord ); // request first unit
    if ( !( qp->Received( args[0].addr) ) )
      {result.addr = 0; return CANCEL; }
    local = SetWord(localinfo);                    // pass up link to localinfo
    return 0;

  case REQUEST:
    if ( local.addr == 0 )
      return CANCEL;
    localinfo = (AtPeriodsLocalInfoUS *) local.addr; // restore local data
    if ( localinfo->uWord.addr == 0 )
      { result.addr = 0; return CANCEL; }
    unit = (Alpha *) localinfo->uWord.addr;
    if ( localinfo->pWord.addr == 0 )
      { result.addr = 0; return CANCEL; }
    periods = (Periods *) localinfo->pWord.addr;

    if( !periods->IsDefined() ||   // by now, periods cannot be undefined
        periods->IsEmpty()       ) // only empty
      return CANCEL;
    
    // search for a pair of overlapping unit/interval:
    while (1)
      {
        if ( localinfo->j == periods->GetNoComponents() ) // redo first interval
          { localinfo->j = 0;
            unit->DeleteIfAllowed();                // delete original unit?
            foundUnit = false;
            while(!foundUnit)
              {
                qp->Request(args[0].addr, localinfo->uWord);  // get new unit
                if( qp->Received( args[0].addr ) )
                  unit = (Alpha *) localinfo->uWord.addr;
                else 
                  { result.addr = 0; return CANCEL; }   // end of unit stream
                foundUnit = unit->IsDefined();
              }
          }
        periods->Get(localinfo->j, interval);       // get an interval
        if (    !( interval->Before( unit->timeInterval ) )
                && !( unit->timeInterval.Before( *interval) ) )
          break;                           // found candidate, break while
        localinfo->j++;                             // next interval, loop
      }
    
    // We have an interval possibly overlapping the unit's interval now
    // Return unit restricted to overlapping part of both intervals
    unit->AtInterval( *interval, resultUnit); // intersect unit and interval
    aux = new Alpha( resultUnit );
    result = SetWord( aux );
    localinfo->j++;                           // increase interval counter
    return YIELD;
    
  case CLOSE:
    if ( local.addr != 0 )
      {
        localinfo = (AtPeriodsLocalInfoUS *) local.addr;
        if ( localinfo->uWord.addr != 0 )
          {
            unit = (Alpha *) localinfo->uWord.addr;
            unit->DeleteIfAllowed();   // delete remaining original unit
          }
        delete (AtPeriodsLocalInfoUS *)localinfo;
      }
    return 0;
    
  } // end switch
  
  return -1; // should never be reached
  
} // end MappingUnitStreamAtPeriods

/*
5.9.3 Specification for operator ~atperiods~

*/
const string
TemporalSpecAtPeriods  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\") "
"( <text>For T in {int, bool, real, string, point, region}:\n"
"(uT periods) -> stream uT\n"
"((stream uT) periods) -> stream uT</text--->"
"<text>_ atperiods _ </text--->"
"<text>restrict the movement to the given"
" periods.</text--->"
"<text>upoint1 atperiods thehour(2003,11,11,8)\n"
"sfeed(upoint1) atperiods thehour(2003,11,11,8)</text--->) )";

/* 
5.9.4 Selection Function of operator ~atperiods~

Uses ~UnitCombinedUnitStreamSelect~.

*/

ValueMapping temporalunitatperiodsmap[] = 
  { MappingUnitAtPeriods<UBool>,
    MappingUnitAtPeriods<UInt>,
    MappingUnitAtPeriods<UReal>,
    MappingUnitAtPeriods<UPoint>,
    MappingUnitAtPeriods<UString>,
    MappingUnitAtPeriods<URegion>,
    MappingUnitStreamAtPeriods<UBool>,
    MappingUnitStreamAtPeriods<UInt>,
    MappingUnitStreamAtPeriods<UReal>,
    MappingUnitStreamAtPeriods<UPoint>,  
    MappingUnitStreamAtPeriods<UString>,  
    MappingUnitStreamAtPeriods<URegion>  
  };

/*
5.9.5  Definition of operator ~atperiods~

*/
Operator temporalunitatperiods( "atperiods",
                            TemporalSpecAtPeriods,
                            12,
                            temporalunitatperiodsmap,
                            UnitCombinedUnitStreamSelect,
                            UnitPeriodsTypeMap );

/*
5.10 Operators ~initial~ and ~final~

5.10.1 Type Mapping for ~initial~ and ~final~

*/
ListExpr
UnitTypeMapIntime( ListExpr args )
{
 if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

       if( nl->IsEqual( arg1, "ubool" ) )
      return nl->SymbolAtom( "ibool" );

    if( nl->IsEqual( arg1, "uint" ) )
      return nl->SymbolAtom( "iint" );

    if( nl->IsEqual( arg1, "ureal" ) )
      return nl->SymbolAtom( "ireal" );

    if( nl->IsEqual( arg1, "upoint" ) )
      return nl->SymbolAtom( "ipoint" );

    if( nl->IsEqual( arg1, "ustring" ) )
      return nl->SymbolAtom( "istring" );

    if( nl->IsEqual( arg1, "uregion" ) )
      return nl->SymbolAtom( "iregion" );

  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.10.2 Value Mapping for ~initial~ and ~final~

*/
template <class Unit, class Alpha>
int MappingUnitInitial( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);

  if( !unit->IsDefined() || !(unit->timeInterval.start.IsDefined()) )
     res->SetDefined( false );
  else
   {
     unit->TemporalFunction( unit->timeInterval.start, res->value, true );
     res->instant.CopyFrom( &unit->timeInterval.start );
     res->SetDefined( true );    
   }
  return 0;
}

template <class Unit, class Alpha>
int MappingUnitFinal( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Unit* unit = ((Unit*)args[0].addr);
  Intime<Alpha>* res = ((Intime<Alpha>*)result.addr);

  if( !unit->IsDefined() || !(unit->timeInterval.end.IsDefined()) )
     res->SetDefined( false );
  else
   {
     unit->TemporalFunction( unit->timeInterval.end, res->value, true );
     res->instant.CopyFrom( &unit->timeInterval.end );
     res->SetDefined( true );
   }
  return 0;
}

/*
5.10.3 Specification for operator ~initial~ and ~final~

*/
const string
TemporalSpecInitial  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>uT -> iT\n"
"(T in {bool, int, real, string, point, region})</text--->"
"<text>initial( _ )</text--->"
"<text>From a unit type, get the intime value corresponding "
"to the initial instant.</text--->"
"<text>initial( upoint1 )</text---> ) )";

const string
TemporalSpecFinal  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>uT -> iT\n"
"(T in {bool, int, real, string, point, region})</text--->"
"<text>final( _ )</text--->"
"<text>get the intime value corresponding "
"to the final instant.</text--->"
"<text>final( upoint1 )</text---> ) )";

/* 
5.10.4 Selection Function of operator ~initial~ and ~final~

Use ~UnitSimpleSelect~ / ~UnitCombinedUnitStreamSelect~.

*/

ValueMapping temporalunitinitialmap[] = { MappingUnitInitial<UBool, CcBool>,
                                          MappingUnitInitial<UInt, CcInt>,
                                          MappingUnitInitial<UReal, CcReal>,
                                          MappingUnitInitial<UPoint, Point>, 
                                          MappingUnitInitial<UString, CcString>,
                                          MappingUnitInitial<URegion, Region>};

ValueMapping temporalunitfinalmap[] = {  MappingUnitFinal<UBool, CcBool>,
                                         MappingUnitFinal<UInt, CcInt>,
                                         MappingUnitFinal<UReal, CcReal>,
                                         MappingUnitFinal<UPoint, Point>, 
                                         MappingUnitFinal<UString, CcString>,
                                         MappingUnitFinal<URegion, Region>};

/*
5.10.5  Definition of operator ~initial~ and ~final~

*/
Operator temporalunitinitial( "initial",
                          TemporalSpecInitial,
                          6,
                          temporalunitinitialmap,
                          UnitSimpleSelect,
                          UnitTypeMapIntime );

Operator temporalunitfinal( "final",
                        TemporalSpecFinal,
                        6,
                        temporalunitfinalmap,
                        UnitSimpleSelect,
                        UnitTypeMapIntime );

/*
5.11 Operator ~present~

5.11.1 Type Mapping for ~present~

*/
ListExpr
UnitInstantPeriodsTypeMapBool( ListExpr args )
{
   if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );

    if( nl->IsEqual( arg2, "instant" ) ||
        nl->IsEqual( arg2, "periods" ) )
    {
      if( nl->IsEqual( arg1, "ubool" )  ||
          nl->IsEqual( arg1, "uint" )   ||
          nl->IsEqual( arg1, "ureal" )  ||
          nl->IsEqual( arg1, "upoint")  || 
          nl->IsEqual( arg1, "ustring") ||
          nl->IsEqual( arg1, "uregion")    )

        return nl->SymbolAtom( "bool" );
    }
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.11.2 Value Mapping for ~present~

*/

template <class Mapping>
int MappingUnitPresent_i( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Instant* inst = ((Instant*)args[1].addr);
  Instant t1 = *inst;

  if ( !inst->IsDefined() || !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );

  else if( m->timeInterval.Contains(t1) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

template <class Mapping>
int MappingUnitPresent_p( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Periods* periods = ((Periods*)args[1].addr);

  assert( periods->IsOrdered() );

  // create a periods containing the smallest superinterval 
  // of all intervals within the periods value.
  Periods deftime( 0 ), defTime( 0 );
  deftime.Clear();
  deftime.StartBulkLoad();
  deftime.Add( m->timeInterval );
  deftime.EndBulkLoad( false );
  deftime.Merge( defTime );

  if ( !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( periods->IsEmpty() ) // (undef periods are not defined)
    ((CcBool *)result.addr)->Set( false, false );
  else if( periods->Inside( defTime ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  
  return 0;
}


/*
5.11.3 Specification for operator ~present~

*/
const string
TemporalSpecPresent  =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>({m|u}T instant) -> bool\n"
"({m|u}T periods) -> bool\n"
"(T in {bool, int, real, string, point, region)</text--->"
"<text>_ present _ </text--->"
"<text>whether the moving/unit object is present at the"
" given instant or period. For an empty periods value, "
"the result is undefined.</text--->"
"<text>mpoint1 present instant1</text---> ) )";

/* 
5.11.4 Selection Function of operator ~present~

*/

int
UnitInstantPeriodsSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  // instant versions:

  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "uint" &&
     nl->SymbolValue( arg2 ) == "instant" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "ustring" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 4;

  if( nl->SymbolValue( arg1 ) == "uregion" &&
      nl->SymbolValue( arg2 ) == "instant" )
    return 5;

  // periods versions:

  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 6;

  if( nl->SymbolValue( arg1 ) == "uint" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 7;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 8;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 9;

  if( nl->SymbolValue( arg1 ) == "ustring" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 10;

  if( nl->SymbolValue( arg1 ) == "uregion" &&
      nl->SymbolValue( arg2 ) == "periods" )
    return 11;


  return -1; // This point should never be reached
}

ValueMapping temporalunitpresentmap[] = { MappingUnitPresent_i<UBool>,
                                          MappingUnitPresent_i<UInt>,
                                          MappingUnitPresent_i<UReal>,
                                          MappingUnitPresent_i<UPoint>,
                                          MappingUnitPresent_i<UString>,
                                          MappingUnitPresent_i<URegion>,
                                          MappingUnitPresent_p<UBool>,
                                          MappingUnitPresent_p<UInt>,
                                          MappingUnitPresent_p<UReal>,
                                          MappingUnitPresent_p<UPoint>,
                                          MappingUnitPresent_p<UString>,
                                          MappingUnitPresent_p<URegion> };

/*
5.11.5  Definition of operator ~present~

*/
Operator temporalunitpresent( "present",
                          TemporalSpecPresent,
                          12,
                          temporalunitpresentmap,
                          UnitInstantPeriodsSelect,
                          UnitInstantPeriodsTypeMapBool);

/*
5.12 Operator ~passes~

5.12.1 Type Mapping for ~passes~

*/
ListExpr
UnitBaseTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( ((nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "point" ))) )
      return nl->SymbolAtom( "bool" );
    if( ((nl->IsEqual( arg1, "uint" ) && nl->IsEqual( arg2, "int" ))) )
      return nl->SymbolAtom( "bool" );
    if( ((nl->IsEqual( arg1, "ubool" ) && nl->IsEqual( arg2, "bool" ))) )
      return nl->SymbolAtom( "bool" );
    if( ((nl->IsEqual( arg1, "ureal" ) && nl->IsEqual( arg2, "real" ))) )
      return nl->SymbolAtom( "bool" );
    if( ((nl->IsEqual( arg1, "ustring" ) && nl->IsEqual( arg2, "string" ))) )
      return nl->SymbolAtom( "bool" );
    if( ((nl->IsEqual( arg1, "uregion" ) && nl->IsEqual( arg2, "point" ))) )
      return nl->SymbolAtom( "bool" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.12.2 Value Mapping for ~passes~

*/
template <class Mapping, class Alpha>
int MappingUnitPasses( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);

  if( !val->IsDefined() || !m->IsDefined() )
    ((CcBool *)result.addr)->Set( false, false );
  else if( m->Passes( *val ) )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  
  return 0;
}

/*
5.12.3 Specification for operator ~passes~

*/
const string
TemporalSpecPasses =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(uT T) -> bool\n"
"for T in {bool, int, real*, string, point, region*}\n"
"(*): Not yet implemented</text--->"
"<text>_ passes _ </text--->"
"<text>whether the object unit passes the given"
" value.</text--->"
"<text>upoint1 passes point1</text---> ) )";

/* 
5.12.4 Selection Function of operator ~passes~

*/
int
UnitBaseSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "ubool" &&
      nl->SymbolValue( arg2 ) == "bool" )
    return 0;

  if( nl->SymbolValue( arg1 ) == "uint" &&
      nl->SymbolValue( arg2 ) == "int" )
    return 1;

  if( nl->SymbolValue( arg1 ) == "ureal" &&
      nl->SymbolValue( arg2 ) == "real" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "upoint" &&
      nl->SymbolValue( arg2 ) == "point" )
    return 3;

  if( nl->SymbolValue( arg1 ) == "ustring" &&
      nl->SymbolValue( arg2 ) == "string" )
    return 2;

  if( nl->SymbolValue( arg1 ) == "uregion" &&
      nl->SymbolValue( arg2 ) == "region" )
    return 3;

  return -1; // This point should never be reached
}

ValueMapping temporalunitpassesmap[] = { MappingUnitPasses<UBool, CcBool>,
                                         MappingUnitPasses<UInt, CcInt>,
                                         MappingUnitPasses<UReal, CcReal>,
                                         MappingUnitPasses<UPoint, Point>, 
                                         MappingUnitPasses<UString, CcString>,
                                         MappingUnitPasses<URegion, Region> };

/*
5.12.5  Definition of operator ~passes~

*/
Operator temporalunitpasses( "passes",
                         TemporalSpecPasses,
                         6,
                         temporalunitpassesmap,
                         UnitBaseSelect,
                         UnitBaseTypeMapBool);

/*
5.13 Operator ~at~

The operator restrict a unit type to interval, where it's value
is equal to a given value. For base types ~bool~, ~int~ and ~point~,
the result will be only a single unit, but for base type ~real~, there
may be two units, as ~ureal~ is represented by a quadratic polynomial
function (or it's radical).

5.13.1 Type Mapping for ~at~

----
     at:   upoint x point --> upoint
     at:    ureal x real  --> (stream ureal)

----

*/
ListExpr
TemporalUnitAtTypeMapUnit( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if (TUA_DEBUG) cout << "\nTemporalUnitAtTypeMapUnit: 0" << endl;

    if( nl->IsEqual( arg1, "ubool" ) && nl->IsEqual( arg2, "bool" ) )
      return nl->SymbolAtom( "ubool" );
    if( nl->IsEqual( arg1, "uint" ) && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "uint" );
    if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "point" ) )
      return nl->SymbolAtom( "upoint" );
    // for ureal, _ at _ will return a stream of ureals!
    if( nl->IsEqual( arg1, "ureal" ) && nl->IsEqual( arg2, "real" ) )
      return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                             nl->SymbolAtom( "ureal" ));  
    if( nl->IsEqual( arg1, "ustring" ) && nl->IsEqual( arg2, "string" ) )
      return nl->SymbolAtom( "ustring" );
    if( nl->IsEqual( arg1, "uregion" ) && nl->IsEqual( arg2, "region" ) )
      return nl->SymbolAtom( "uregion" );
  
  }
  if (TUA_DEBUG) cout << "\nTemporalUnitAtTypeMapUnit: 1" << endl;
  return nl->SymbolAtom( "typeerror" );
}

/*
5.13.2 Value Mapping for ~at~

We implement two variants, the first for unit types using ~ConstTemporalUnits~ 
and ~SpatialUnits~, and a second one for ~ureal~

*/

// first valuemapping, for all but ureal:
template <class Unit, class Alpha>
int MappingUnitAt( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  Word a0, a1;

  a0 = args[0];
  a1 = args[1];

  Unit* unit = ((Unit*)a0.addr);
  Alpha* val = ((Alpha*)a1.addr);

  Unit* pResult = ((Unit*)result.addr);

  if ( !unit->IsDefined() || !val->IsDefined() )
    pResult->SetDefined(false);
  else if (unit->At( *val, *pResult ))
    {
      pResult->SetDefined(true);
      pResult->timeInterval.start.SetDefined(true);
      pResult->timeInterval.end.SetDefined(true);
    }
  else
    {
      pResult->SetDefined(false);
      pResult->timeInterval.start.SetDefined(false);
      pResult->timeInterval.end.SetDefined(true);
    }
  
  return 0;
}

struct MappingUnitAt_rLocalInfo {
  bool finished;
  int  NoOfResults;  // the number of remaining results
  Word runits[2];    // the results
};

// second value mapping: (ureal real) -> (stream ureal)
int MappingUnitAt_r( Word* args, Word& result, int message,
                   Word& local, Supplier s )
{
  MappingUnitAt_rLocalInfo *localinfo;
  double radicand, a, b, c, r, y;
  DateTime t1 = DateTime(instanttype);
  DateTime t2 = DateTime(instanttype);
  Interval<Instant> rdeftime, deftime;
  UReal *uinput;
  CcReal *value;
  Word a0, a1;
  double tx, t0, A, B, C;
  

  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\nMappingUnitAt_r: OPEN" << endl;
      localinfo = new MappingUnitAt_rLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      if(TUA_DEBUG) cout << "  1" << endl;

      a0 = args[0];
      uinput = (UReal*)(a0.addr);
      if(TUA_DEBUG) cout << "  1.1" << endl;

      a1 = args[1];
      value = (CcReal*)(a1.addr);
      if(TUA_DEBUG) cout << "  1.2" << endl;

      if(TUA_DEBUG) cout << "  2" << endl;

      if(TUA_DEBUG) cout << "  2.1: " << uinput->IsDefined() << endl;
      if(TUA_DEBUG) cout << "  2.2: " << value->IsDefined() << endl;

      if ( !uinput->IsDefined() ||
           !value->IsDefined() )
        { // some input is undefined -> return empty stream
          if(TUA_DEBUG) cout << "  3: Some input is undefined. No result." 
                             << endl;
          localinfo->NoOfResults = 0;
          localinfo->finished = true;
          local = SetWord(localinfo);
          if(TUA_DEBUG) cout << "\nMappingUnitAt_r: finished OPEN (1)" 
                             << endl;
          return 0;
        }
      if(TUA_DEBUG) cout << "  4" << endl;

      y  = value->GetRealval();
      a  = uinput->a;
      b  = uinput->b;
      c  = uinput->c;
      r  = uinput->r;
      deftime = uinput->timeInterval;
      t0 = deftime.start.ToDouble();

      if(TUA_DEBUG) 
        {cout << "    The UReal is" << " a= " << a << " b= " 
              << b << " c= " << c << " r= " << r << endl;
          cout << "    The Real is y=" << y << endl;
          cout << "  5" << endl;
        }
            
      if ( (a == 0) && (b == 0) )
        { // constant function. Possibly return input unit
          if(TUA_DEBUG) cout << "  6: 1st arg is a constant value" << endl;
          if (c != y)
            { // There will be no result, just an empty stream
              if(TUA_DEBUG) cout << "  7" << endl;
              localinfo->NoOfResults = 0;
              localinfo->finished = true;
            }
          else
                { // Return the complete unit
                  if(TUA_DEBUG) 
                    {
                      cout << "  8: Found constant solution" << endl;
                      cout << "    T1=" << c << endl;
                      cout << "    Tstart=" << deftime.start.ToDouble() << endl;
                      cout << "    Tend  =" << deftime.end.ToDouble() << endl;
                    }
                  localinfo->runits[localinfo->NoOfResults].addr
                    = uinput->Copy();
                  localinfo->NoOfResults++;
                  localinfo->finished = false;
                  if(TUA_DEBUG) cout << "  9" << endl;
                }
          if(TUA_DEBUG) cout << "  10" << endl;
          local = SetWord(localinfo);
          if(TUA_DEBUG) 
            cout << "\nMappingUnitAt_r: finished OPEN (2)" << endl;
          return 0;
        }
      if ( (a == 0) && (b != 0) )
        { // linear function. Possibly return input unit restricted 
          // to single value
          if(TUA_DEBUG) cout << "  11: 1st arg is a linear function" << endl;
          double T1 = (y - c + b*t0)/b;
          if(TUA_DEBUG) 
            {
              cout << "    T1=" << T1 << endl;    
              cout << "    Tstart=" << deftime.start.ToDouble() << endl;
              cout << "    Tend  =" << deftime.end.ToDouble() << endl;    
            }
          t1.ReadFrom( T1 );
          if (deftime.Contains(t1))
            { // value is contained by deftime
              if(TUA_DEBUG) 
                cout << "  12: Found valid linear solution." << endl;
              localinfo->runits[localinfo->NoOfResults].addr = 
                uinput->Copy();
              ((UReal*)(localinfo
                        ->runits[localinfo->NoOfResults].addr))
                ->timeInterval = Interval<Instant>(t1, t1, true, true);
              // translate result to new starting instant!
              tx =  deftime.start.ToDouble() - T1;
              ((UReal*)(localinfo
                        ->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;                
              if(TUA_DEBUG) cout << "  13" << endl;
            }
          else
            { // value is not contained by deftime -> no result
              if(TUA_DEBUG) 
                cout << "  14: Found invalid linear solution." << endl;
              localinfo->NoOfResults = 0;
              localinfo->finished = true;
              if(TUA_DEBUG) cout << "  15" << endl;
            }
          if(TUA_DEBUG) cout << "  16" << endl;
          local = SetWord(localinfo);
          cout << "\nMappingUnitAt_r: finished OPEN (3)" << endl;
          return 0;
        }
      
      if(TUA_DEBUG) cout << "  17" << endl;
      A = a;
      B = b - 2*a*t0;
      C = t0*(a*t0-b)+c;
      radicand = pow(B,2) + 4*a*(y-C); 
      if(TUA_DEBUG) cout << "    radicand =" << radicand << endl;
      if ( (a != 0) && (radicand >= 0) )
        { // quadratic function. There are possibly two result units
          // calculate the possible t-values t1, t2
          
/*
The solution to the equation $at^2 + bt + c = y$ is 
\[t_{1,2} = \frac{-b \pm \sqrt{b^2-4a(c-y)}}{2a},\] for $b^2-4a(c-y) = b^2+4a(y-c) \geq 0$.


*/
          if(TUA_DEBUG) cout << "  18: 1st arg is a quadratic function" << endl;
          double T1 = (-B + sqrt(radicand)) / (2*A);
          double T2 = (-B - sqrt(radicand)) / (2*A);
          if(TUA_DEBUG) 
            {
              cout << "    T1=" << T1 << endl;
              cout << "    T2=" << T2 << endl;
              cout << "    Tstart=" << deftime.start.ToDouble() << endl;
              cout << "    Tend  =" << deftime.end.ToDouble() << endl;    
            }
          t1.ReadFrom( T1 );
          t2.ReadFrom( T2 );
          
          // check, whether t1 contained by deftime
          if (deftime.Contains( t1 ))
            {
              if(TUA_DEBUG) 
                cout << "  19: Found first quadratic solution" << endl;
              rdeftime.start = t1;
              rdeftime.end = t1;
              rdeftime.lc = true;
              rdeftime.rc = true;
              localinfo->runits[localinfo->NoOfResults].addr = 
                new UReal( rdeftime,a,b,c,r );
              ((UReal*) (localinfo->runits[localinfo->NoOfResults].addr))
                ->SetDefined( true );
              // translate result to new starting instant!
              tx = deftime.start.ToDouble() - T1;
              ((UReal*)(localinfo
                        ->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;
              if(TUA_DEBUG) cout << "  20" << endl;
            }
          // check, whether t2 contained by deftime
          if ( !(t1 == t2) && (deftime.Contains( t2 )) )
            {
              if(TUA_DEBUG) 
                cout << "  21: Found second quadratic solution" << endl;
              rdeftime.start = t2;
              rdeftime.end = t2;
              rdeftime.lc = true;
              rdeftime.rc = true;
              localinfo->runits[localinfo->NoOfResults].addr = 
                new UReal( rdeftime,a,b,c,r );
              ((UReal*) (localinfo->runits[localinfo->NoOfResults].addr))
                ->SetDefined (true );
              // translate result to new starting instant!
              tx =  deftime.start.ToDouble() - T2;
              ((UReal*)(localinfo
                        ->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;
              if(TUA_DEBUG) cout << "  22" << endl;
            }
        }
      else // negative discreminant -> there is no real solution 
           //                          and no result unit
        {
          if(TUA_DEBUG) cout << "  23: No real-valued solution" << endl;
          localinfo->NoOfResults = 0;
          localinfo->finished = true;
          if(TUA_DEBUG) cout << "  24" << endl;
        }
      if(TUA_DEBUG) cout << "  25" << endl;
      local = SetWord(localinfo);
      if(TUA_DEBUG) 
        cout << "\nMappingUnitAt_r: finished OPEN (4)" << endl;
      return 0;
      
    case REQUEST :
      
      if(TUA_DEBUG) cout << "\nMappingUnitAt_r: REQUEST" << endl;
      if (local.addr == 0)
        {
          cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (1)" << endl;
          return CANCEL;
        }
      localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
      if(TUA_DEBUG) cout << "\n   localinfo: finished=" << localinfo->finished 
                         << " NoOfResults==" << localinfo->NoOfResults << endl;

      if (localinfo->finished)
        {
          if(TUA_DEBUG) 
            cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (2)" << endl;
          return CANCEL;
        }
      if ( localinfo->NoOfResults <= 0 )
        { localinfo->finished = true;
          if(TUA_DEBUG) 
            cout << "\nMappingUnitAt_r: finished REQUEST CANCEL (3)" << endl;
          return CANCEL;
        }
      localinfo->NoOfResults--;
      result = SetWord( ((UReal*)(localinfo
                                  ->runits[localinfo->NoOfResults].addr))
                        ->Clone() );
      ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
        ->DeleteIfAllowed();
      if(TUA_DEBUG) cout << "\nMappingUnitAt_r: finished REQUEST YIELD" << endl;
      return YIELD;
      
    case CLOSE :

      if(TUA_DEBUG) cout << "\nMappingUnitAt_r: CLOSE" << endl;
      if (local.addr != 0)
        {
          localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
          for (;localinfo->NoOfResults>0;localinfo->NoOfResults--)
            ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
              ->DeleteIfAllowed();
          delete localinfo;
        }
      if(TUA_DEBUG) cout << "\nMappingUnitAt_r: finished CLOSE" << endl;
      return 0;
    } // end switch
  
      // should not be reached
  return 0;
}


/*
5.13.3 Specification for operator ~at~

*/
const string
TemporalSpecAt =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in {bool, int, string, point, region*}:\n"
"(uT    T   ) -> uT\n"
"(ureal real) -> (stream ureal)\n"
"(*): Not yet implemented</text--->"
"<text>_ at _ </text--->"
"<text>restrict the movement to the times "
"where the equality occurs.\n"
"Observe, that for type 'ureal', the result is a "
"'(stream ureal)' rather than a 'ureal'!</text--->"
"<text>upoint1 at point1</text---> ) )";

/* 
5.13.4 Selection Function of operator ~at~

Uses ~unitBaseSelect~.

*/

ValueMapping temporalunitatmap[] = {  MappingUnitAt< UBool, CcBool>,
                                      MappingUnitAt< UInt, CcInt>,
                                      MappingUnitAt_r,
                                      MappingUnitAt< UPoint, Point>, 
                                      MappingUnitAt< UString, CcString>, 
                                      MappingUnitAt< URegion, Region> };

/*
5.13.5  Definition of operator ~at~

*/
Operator temporalunitat( "at",
                     TemporalSpecAt,
                     6,
                     temporalunitatmap,
                     UnitBaseSelect,
                     TemporalUnitAtTypeMapUnit );

/*
5.14 Operator ~circle~

5.14.1 Type Mapping for ~circle~

*/
ListExpr
TypeMapCircle( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );


    if( nl->IsEqual( arg1, "point" ) && nl->IsEqual( arg2, "real" )
       && nl->IsEqual( arg3, "int" ) )
      return nl->SymbolAtom( "region" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.14.2 Value Mapping for ~circle~

*/
int Circle( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Point* p = (Point*)args[0].addr; // Centre of the circle
  CcReal* r = (CcReal*)args[1].addr; // Radius of the circle.
  CcInt* narg = (CcInt*)args[2].addr; // number of edges
  Region *res = (Region*)result.addr;
  double x, y;
  int n;
  double radius;
  double valueX, valueY;
  double angle;
  int partnerno = 0;
  HalfSegment hs;

  res->Clear();                // clear the result region
  if (!p->IsDefined() || !r->IsDefined() || !narg->IsDefined() )
    { // Nothing to do
      res->SetDefined( false );
    }
  else
    {
      x = p->GetX();
      y = p->GetY();
      
      n = narg->GetIntval();
      radius = r->GetRealval();
      
      res->StartBulkLoad();
      if ((n>2)&&(n<101)&&(radius >0.0))
        {
          
          //  Calculate a polygon with (n) vertices and (n) edges.
          //  To get the vertices, divide 360 degree in n parts using  
          //  a standardised circle around p with circumference U = 2 * PI * r.
          
          for( int i = 0; i < n; i++ )
            {
              // The first point/vertex of the segment
              angle = i * 2 * PI/n; // angle to starting vertex
              valueX = x + radius * cos(angle);
              valueY = y + radius * sin(angle);                            
              Point v1(true, valueX ,valueY);

              // The second point/vertex of the segment
              if ((i+1) >= n)            // angle to end vertex
                angle = 0 * 2 * PI/n;    // for inner vertex
              else
                angle = (i+1) * 2 * PI/n;// for ending = starting vertex
              valueX = x + radius * cos(angle);
              valueY = y + radius * sin(angle);
              Point v2(true, valueX ,valueY);
              
              // Create a halfsegment for this segment
              hs.Set(true, v1, v2);
              hs.attr.faceno = 0;         // only one face
              hs.attr.cycleno = 0;        // only one cycle
              hs.attr.edgeno = partnerno;
              hs.attr.partnerno = partnerno++;
              hs.attr.insideAbove = (hs.GetLeftPoint() == v1); 
              
              // Add halfsegments 2 times with opposite LeftDomPoints
              *res += hs;
              hs.SetLeftDomPoint( !hs.IsLeftDomPoint() );
              *res += hs;         
            }      
        }
      res->EndBulkLoad();
      res->SetDefined( true );
    }
  return 0;
}

/*
5.14.3 Specification for operator ~circle~

*/
const string
TemporalSpecCircle =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>(point real int) -> region</text--->"
"<text>circle ( p, r, n ) </text--->"
"<text>Creates a region with a shape approximating a circle "
"with a given a given center point p and radius r>0.0 by a "
"regular polygon with 2<n<101 edges.\n"
"Parameters out of the given perimeters result in an "
"empty region, undef values in an undef one .</text--->"
"<text>circle (p,10.0,10)</text---> ) )";

/* 
5.14.4 Selection Function of operator ~circle~

Not necessary.

*/

/*
5.14.5  Definition of operator ~circle~

*/
Operator temporalcircle( "circle",
                         TemporalSpecCircle,
                         Circle,
                         Operator::SimpleSelect,
                         TypeMapCircle);

/*
5.15 Operator ~makepoint~

5.15.1 Type Mapping for ~makepoint~

*/
ListExpr
TypeMapMakepoint( ListExpr args )
{
  ListExpr arg1, arg2;
  if( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );

    if( nl->IsEqual( arg1, "int" )
       && nl->IsEqual( arg2, "int" ) )
      return nl->SymbolAtom( "point" );
  }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.15.2 Value Mapping for ~makepoint~

*/
int MakePoint( Word* args, Word& result, int message, Word& local, Supplier s )
{
  
  result = qp->ResultStorage( s );
  CcInt* value1 = (CcInt*)args[0].addr;
  CcInt* value2 = (CcInt*)args[1].addr;
  
  if ( !value1->IsDefined() || !value2->IsDefined() )
    ((Point*)result.addr)->SetDefined( false );
  else
    ((Point*)result.addr)->Set(value1->GetIntval(),value2->GetIntval() );
  
  return 0;
}

/*
5.15.3 Specification for operator ~makepoint~

*/
const string
TemporalSpecMakePoint =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>int x int -> point</text--->"
"<text>makepoint ( _ ) </text--->"
"<text>create a point from two "
"given coordinates.</text--->"
"<text>makepoint (5,5)</text---> ) )";

/* 
5.15.4 Selection Function of operator ~makepoint~

Not necessary.

*/

/*
5.15.5  Definition of operator ~makepoint~

*/
Operator temporalmakepoint( "makepoint",
                            TemporalSpecMakePoint,
                            MakePoint,
                            Operator::SimpleSelect,
                            TypeMapMakepoint);

/*
5.16 Operator ~velocity~

Type mapping for ~velocity~ is

----  
      mpoint  ->  mpoint
      upoint  ->  upoint

----

*/
ListExpr
TypeMapVelocity( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
    {
      arg1 = nl->First( args );
      
      if( nl->IsEqual( arg1, "mpoint" ) )
        return nl->SymbolAtom( "mpoint" );
      if( nl->IsEqual( arg1, "upoint" ) )
        return nl->SymbolAtom( "upoint" );
    }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.16.2 Value Mapping for ~velocity~

*/
int MPointVelocity(Word* args, Word& result, int message,
                   Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  MPoint *res = (MPoint*) result.addr;
  MPoint *input = (MPoint*)args[0].addr;
  
  res->Clear();
  if ( input->IsDefined() )
    // call member function:
    input->MVelocity( *res ); 
  else
    {
      res->Clear();             // use empty value 
      //res->SetDefined(false); // instead of undef value
    }
  return 0;
}

int UnitPointVelocity(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UPoint *input = (UPoint*)args[0].addr;
  UPoint *res   = (UPoint*)result.addr;
  
  if ( !input->IsDefined() )
    res->SetDefined( false );
  else
    input->UVelocity( *res );
  return 0;
}

/*
5.16.3 Specification for operator ~velocity~

*/
const string
TemporalSpecVelocity=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mpoint -> mpoint\n"
"upoint -> upoint</text--->"
"<text>velocity ( _ ) </text--->"
"<text>describes the vector of the speed "
"of the given temporal spatial object (i.e. the "
"coponemtwise speed in unit/s).</text--->"
"<text>velocity (mpoint)</text---> ) )";

/* 
5.16.4 Selection Function of operator ~velocity~

*/
int
VelocitySelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  
  if( nl->SymbolValue( arg1 ) == "mpoint")
    return 0;
  
  if( nl->SymbolValue( arg1 ) == "upoint")
    return 1;
  
  return -1; // This point should never be reached
}

ValueMapping temporalvelocitymap[] = { MPointVelocity,
                                       UnitPointVelocity };

/*
5.16.5  Definition of operator ~velocity~

*/
Operator temporalvelocity( "velocity",
                           TemporalSpecVelocity,
                           2,
                           temporalvelocitymap,
                           VelocitySelect,
                           TypeMapVelocity);

/*
5.17 Operator ~derivable~

5.17.1 Type Mapping for ~derivable~

Type mapping for ~derivable~ is

----  mreal  ->  mbool

----

*/
ListExpr
TypeMapDerivable( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
    {
      arg1 = nl->First( args );
      
      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "mbool" );
      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->SymbolAtom( "ubool" );
    }
  return nl->SymbolAtom( "typeerror" );
}

/*
5.17.2 Value Mapping for ~derivable~

*/
int MPointDerivable( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  
  result = qp->ResultStorage( s );
  MReal* value = (MReal*)args[0].addr;
  MBool* res = ((MBool*)result.addr);
  
  const UReal *uReal;
  UBool boolvalue;
  CcBool b;
  
  res->Clear();
  
  if ( !value->IsDefined() )
    res->SetDefined(false);
  else
    {
      res->StartBulkLoad();
      for( int i = 0; i < value->GetNoComponents(); i++ )
        {         
          value->Get( i, uReal ); // Load a real unit.
          
          // FALSE means in this case that a real unit describes a quadratic
          // polynomial. A derivation is possible and the operator returns TRUE.
          if (uReal->r == false)
            b.Set(true,true);
          else
            b.Set(true,false);
          
          UBool boolvalue (uReal->timeInterval,b);
          res->Add( boolvalue );
        }
      res->EndBulkLoad( false );
    }
  return 0;
}

int UnitPointDerivable( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  UReal* uReal = (UReal*)args[0].addr;
  UBool* res = ((UBool*)result.addr);
  
  CcBool b;
  
  if (uReal->IsDefined())
    {     
      res->timeInterval = uReal->timeInterval;
      
      if (uReal->r == false)
        b.Set(true,true);
      else
        b.Set(true,false);
      
      UBool boolvalue (uReal->timeInterval,b);
      res->CopyFrom(&boolvalue);
    }
  else
    res->SetDefined( false );
  
  return 0;
}

/*
5.17.3 Specification for operator ~derivable~

*/
const string
TemporalSpecDerivable=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>mreal -> mbool\n"
"ureal -> ubool</text--->"
"<text>derivable ( _ ) </text--->"
"<text>Returns a moving/unit bool decribing the "
"derivability of the moving/unit real over time.</text--->"
"<text>derivable (mreal)</text---> ) )";

/* 
5.17.4 Selection Function of operator ~derivable~

*/

int
DerivableSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  
  if( nl->SymbolValue( arg1 ) == "mreal")
    return 0;
  
  if( nl->SymbolValue( arg1 ) == "ureal")
    return 1;
  
  return -1; // This point should never be reached
}

ValueMapping temporalderivablemap[] = { MPointDerivable,
                                        UnitPointDerivable };

/*
5.17.5  Definition of operator ~derivable~

*/

Operator temporalderivable( "derivable",
                            TemporalSpecDerivable,
                            2,
                            temporalderivablemap,
                            DerivableSelect,
                            TypeMapDerivable);

/*
5.18 Operator ~derivative~

5.18.1 Type Mapping for ~derivative~

Type mapping for ~derivative~ is

----  mreal  ->  mreal

----

*/
ListExpr 
TypeMapDerivative( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
    {
      arg1 = nl->First( args );
      
      if( nl->IsEqual( arg1, "mreal" ) )
        return nl->SymbolAtom( "mreal" );
      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->SymbolAtom( "ureal" );
    }
  return nl->SymbolAtom( "typeerror" );
}


/*
5.18.2 Value Mapping for ~derivative~

*/
int MPointDerivative( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  
  result = qp->ResultStorage( s );
  MReal* value = (MReal*)args[0].addr;
  MReal* res = ((MReal*)result.addr);
  
  const UReal *Unit;
  UReal uReal;
  
  res->Clear();
  if ( value->IsDefined() )
    {
      res->StartBulkLoad();
      
      for( int i = 0; i < value->GetNoComponents(); i++ )
        { // load a real unit
          value->Get( i, Unit );
          if (Unit->IsDefined() && !Unit->r)
            { // The derivative of this quadratic polynom is 2at + b + 0
              uReal.timeInterval = Unit->timeInterval;
              uReal.a = 0;
              uReal.b = 2 * Unit->a;
              uReal.c = Unit->b;
              uReal.r = Unit->r;
              uReal.SetDefined(true);
              res->Add( uReal );
            }
          // else: Do nothing. Do NOT add an undefined unit!
          //       (That would conflict e.g. with operator deftime())
        }
      res->EndBulkLoad( false ); // no sorting, assuming the input was ordered
    }
  else // value is undefined
    res->SetDefined( false );
  return 0;
}

int UnitPointDerivative( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  UReal *Unit = (UReal*)args[0].addr;
  UReal *res  = (UReal*)result.addr;
  
  if (Unit->IsDefined() && !Unit->r)
    {
      res->timeInterval = Unit->timeInterval;
      res->a = 0;
      res->b = 2 * Unit->a;
      res->c = Unit->b;
      res->r = Unit->r;
      res->SetDefined(true);      
    }
  else // Unit is undefined
    {
      DateTime t = DateTime(instanttype);
      res->timeInterval = Interval<Instant>(t,t,true,true);
      res->a = 0;
      res->b = 0;
      res->c = 0;
      res->r = false;
      res->SetDefined(false);
    }
  return 0;
}

/*
5.18.3 Specification for operator ~derivative~

*/
const string
TemporalSpecDerivative=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>{m|u}real -> {m|u}real</text--->"
"<text>derivative ( _ ) </text--->"
"<text>Determine the derivative"
" of a mreal/ureal value.</text--->"
"<text>derivable (mreal)</text---> ) )";

/* 
5.18.4 Selection Function of operator ~deriavtive~

Uses ~DerivableSelect~.

*/

ValueMapping temporalderivativemap[] = { MPointDerivative,
                                         UnitPointDerivative };

/*
5.18.5  Definition of operator ~derivative~

*/
Operator temporalderivative( "derivative",
                             TemporalSpecDerivative,
                             2,
                             temporalderivativemap,
                             DerivableSelect,
                             TypeMapDerivative);

/*
5.19 Operator ~sfeed~

The operator is used to cast a single value T to a (stream T)
having a single element of type T.

5.19.1 Type Mapping for ~sfeed~

*/
ListExpr 
TypeMapSfeed( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  if ( ( nl->ListLength(args) == 1 ) && ( nl->IsAtom(nl->First(args) ) ) )
    {
      arg1 = nl->First(args);
      if( am->CheckKind("DATA", arg1, errorInfo) ) 
        return nl->TwoElemList(nl->SymbolAtom("stream"), arg1);
    }
  ErrorReporter::ReportError("Operator sfeed  expects a list of length one, "
                             "containing a value of one type 'T' with T in "
                             "kind DATA.");
  return nl->SymbolAtom( "typeerror" );
}

/*
5.19.2 Value Mapping for ~sfeed~

*/
struct SFeedLocalInfo
{
  bool finished;
};

int MappingSFeed( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  SFeedLocalInfo *linfo;
  Word argValue;
  
  switch( message )
    {
    case OPEN:
      linfo = new SFeedLocalInfo;
      linfo->finished = false;
      local = SetWord(linfo);
      return 0;
      
    case REQUEST:
      if ( local.addr == 0 ) 
        return CANCEL;
      linfo = ( SFeedLocalInfo *)local.addr;
      if ( linfo->finished )
        return CANCEL;
      argValue = args[0];
      result = SetWord(((Attribute*) (argValue.addr))->Clone());
      linfo->finished = true;
      return YIELD;
      
    case CLOSE:
      if ( local.addr == 0 ) 
        { 
          linfo = ( SFeedLocalInfo*) local.addr;
          delete linfo;
        }
      return 0;     
    }
  return -1; // should not be reached
}

/*
5.19.3 Specification for operator ~sfeed~

*/
const string
TemporalSpecSfeed=
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"( <text>For T in kind DATA:\n"
"T -> (stream T)</text--->"
"<text>sfeed ( _ ) </text--->"
"<text>create a single-value stream from "
"a single value.</text--->"
"<text>query sfeed ([const int value 5]) count;</text---> ) )";

/* 
5.19.4 Selection Function of operator ~sfeed~

*/

ValueMapping temporalunitsfeedmap[] = { MappingSFeed };

int SfeedSelect( ListExpr args )
{
  return 0;
}

/*
5.19.5  Definition of operator ~sfeed~

*/
Operator temporalunitsfeed( "sfeed",
                            TemporalSpecSfeed,
                            1,
                            temporalunitsfeedmap,
                            SfeedSelect,
                            TypeMapSfeed);


/*
5.20 Operator ~suse~

The ~suse~ class of operators implements a set of functors, that derive
stream-valued operators from operators taking scalar arguments and returning
scalar values or streams of values:

----

     suse: (stream X)            (map X Y)            -> (stream Y)
           (stream X)            (map X (stream Y))   -> (stream Y)
           (stream X) Y          (map X Y Z)          -> (stream Z)
           (stream X) Y          (map X Y stream(Z))  -> (stream Z)
           X          (stream Y) (map X y Z)          -> (stream Z)
           X          (stream Y) (map X y (stream Z)) -> (stream Z)
           (stream X) (stream Y) (map X Y Z)          -> (stream Z)
           (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
           for X,Y,Z of kind DATA

----

5.20.1 Type Mapping for ~suse~

*/

ListExpr 
TypeMapSuse( ListExpr args )
{
  string outstr1, outstr2;            // output strings
  ListExpr errorInfo;
  ListExpr sarg1, map;                // arguments to suse
  ListExpr marg1, mres;               // argument to mapping
  ListExpr sarg1Type, sresType;       // 'flat' arg type 
  
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  if (TUA_DEBUG) cout << "\nTypeMapSuse: 0" << endl;
  
  if ( (nl->ListLength( args ) != 2) )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 1" << endl;
      ErrorReporter::ReportError("Operator suse expects a list of length two ");
      return nl->SymbolAtom( "typeerror" );
    }
  if (TUA_DEBUG) cout << "TypeMapSuse: 2" << endl;
  
  // get suse arguments
  sarg1 = nl->First( args );
  map = nl->Second( args ); 
  
  // check sarg1 for being a stream
  if(     nl->IsAtom( sarg1 )
          || ( nl->ListLength( sarg1 ) != 2) 
          || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 3" << endl;
      
      ErrorReporter::ReportError(
        "Operator suse expects its first Argument to "
        "be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );           
    }
  sarg1Type = nl->Second(sarg1);
  if (TUA_DEBUG) cout << "TypeMapSuse: 4" << endl;
  
  // check sarg1 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type ) 
         && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 5" << endl;
      nl->WriteToString(outstr1, sarg1Type);      
      ErrorReporter::ReportError("Operator suse expects its 1st argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");
      return nl->SymbolAtom( "typeerror" );      
    }
  if (TUA_DEBUG) cout << "TypeMapSuse: 6" << endl;
  
  // This check can be removed when operators working on tuplestreams have
  // been implemented:
  if ( !nl->IsAtom( sarg1Type ) &&
       (nl->ListLength( sarg1Type ) == 2) &&
       nl->IsEqual( nl->First(sarg1Type), "tuple") )      
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 7" << endl;
      ErrorReporter::ReportError("Operator suse still not implemented for "
                                 "arguments of type 'tuple(X)' or "
                                 "'(stream tuple(X))'.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse: 8" << endl;
  if ( !nl->IsAtom( sarg1Type ) &&
       ( (nl->ListLength( sarg1Type ) != 2) ||
         !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
         !IsTupleDescription(nl->Second(sarg1Type))
         ) 
       )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 9" << endl;
      nl->WriteToString(outstr1, sarg1);
      return nl->SymbolAtom( "typeerror" );      
    }
  if (TUA_DEBUG) cout << "TypeMapSuse: 10" << endl;
  
  // check for map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 11" << endl;
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator suse expects a map as "
                                 "2nd argument, but gets '" + outstr1 +
                                 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }    
  if (TUA_DEBUG) cout << "TypeMapSuse: 12" << endl;
  
  if ( nl->ListLength(map) != 3 )       
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 13" << endl;
      ErrorReporter::ReportError("Number of map arguments must be 1 "
                                 "for operator suse.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse: 14" << endl;
  // get map arguments
  marg1 = nl->Second(map);
  mres  = nl->Third(map);
  
  // check marg1
  
  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 15" << endl;
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator suse: 1st argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 1st argument. If e.g. the first "
                                 "is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 + 
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  if (TUA_DEBUG) cout << "TypeMapSuse: 16" << endl;
  
  // get map result type 'sresType'
  if( !( nl->IsAtom( mres ) ) && ( nl->ListLength( mres ) == 2) ) 
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 17" << endl;
      
      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if (TUA_DEBUG) cout << "TypeMapSuse: 18" << endl;
          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) )
            {
              if (TUA_DEBUG) cout << "TypeMapSuse: 19" << endl;
              
              ErrorReporter::ReportError(
                "Operator suse expects its 2nd Argument to "
                "return a '(stream T)', T of kind DATA'.");
              return nl->SymbolAtom( "typeerror" );           
            }
          if (TUA_DEBUG) cout << "TypeMapSuse: 20" << endl;
          
          sresType = mres; // map result type is already a stream
          nl->WriteToString(outstr1, sresType);
          if (TUA_DEBUG) cout << "\nTypeMapSuse Resulttype (1): " 
                              << outstr1 << "\n";
          return sresType;
        }
      if (TUA_DEBUG) cout << "TypeMapSuse: 21" << endl;
      
    }
  else // map result type is not a stream, so encapsulate it
    {
      if (TUA_DEBUG) cout << "TypeMapSuse: 22" << endl;
      
      if ( !am->CheckKind("DATA", mres, errorInfo) )
        {
          if (TUA_DEBUG) cout << "TypeMapSuse: 23" << endl;
          
          ErrorReporter::ReportError(
            "Operator suse expects its 2nd Argument to "
            "return a type of kind DATA.");
          return nl->SymbolAtom( "typeerror" );       
        }
      if (TUA_DEBUG) cout << "TypeMapSuse: 24" << endl;
      
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);  
      nl->WriteToString(outstr1, sresType);
      if (TUA_DEBUG) cout << "\nTypeMapSuse Resulttype (2): " 
                          << outstr1 << "\n";
      return sresType;
    }
  if (TUA_DEBUG) cout << "TypeMapSuse: 25" << endl;
  
  // otherwise (some unmatched error)
  return nl->SymbolAtom( "typeerror" );
}


ListExpr 
TypeMapSuse2( ListExpr args )
{
  string   outstr1, outstr2;               // output strings
  ListExpr errorInfo;
  ListExpr sarg1, sarg2, map;              // arguments to suse
  ListExpr marg1, marg2, mres;             // argument to mapping
  ListExpr sarg1Type, sarg2Type, sresType; // 'flat' arg type 
  ListExpr argConfDescriptor;
  bool 
    sarg1isstream = false, 
    sarg2isstream = false,
    resisstream   = false;
  int argConfCode = 0;
  
  if (TUA_DEBUG) cout << "\nTypeMapSuse2: 0" << endl;
  
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  // 0. Check number of arguments
  if ( (nl->ListLength( args ) != 3) )
    {
      ErrorReporter::ReportError("Operator suse2 expects a list of "
                                 "length three ");
      return nl->SymbolAtom( "typeerror" );
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 1" << endl;
  
  // 1. get suse arguments
  sarg1 = nl->First( args );
  sarg2 = nl->Second( args );
  map   = nl->Third( args ); 
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 2" << endl;
  
  // 2. First argument
  // check sarg1 for being a stream
  if( nl->IsAtom( sarg1 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // non-stream datatype
      sarg1Type = sarg1;
      sarg1isstream = false;
    }
  else if ( !nl->IsAtom( sarg1 )
            && ( nl->ListLength( sarg1 ) == 2) 
            && (TypeOfRelAlgSymbol(nl->First(sarg1) == stream )) )
    { // (stream datatype)
      sarg1Type = nl->Second(sarg1);
      sarg1isstream = true;
    }
  else // wrong type for sarg1
    {
      ErrorReporter::ReportError(
                                 "Operator suse2 expects its first Argument to "
        "be of type 'T' or '(stream T).");
      return nl->SymbolAtom( "typeerror" );           
    }
  
  // check sarg1 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg1Type ) 
      && !am->CheckKind("DATA", nl->Second( sarg1Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr1, sarg1Type);      
      ErrorReporter::ReportError("Operator suse2 expects its 1st argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");
      
      return nl->SymbolAtom( "typeerror" );      
    }
  else if ( !nl->IsAtom( sarg1Type ) &&
            ( (nl->ListLength( sarg1Type ) != 2) ||
              !nl->IsEqual( nl->First(sarg1Type), "tuple") ||
              !IsTupleDescription(nl->Second(sarg1Type))
              ) 
            )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg1Type);
      ErrorReporter::ReportError("Operator suse2 expects its 1st argument "
                                 "to be 'T' or '(stream T), T of kind DATA "
                                 "or of type 'tuple(X))', but"
                                 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 3" << endl;
  
  // 3. Second Argument
  // check sarg2 for being a stream
  if( nl->IsAtom( sarg2 )
      || !(TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // non-stream datatype
      sarg2Type = sarg2;
      sarg2isstream = false;
    }
  else if ( !nl->IsAtom( sarg2 )
            && ( nl->ListLength( sarg2 ) == 2) 
            && (TypeOfRelAlgSymbol(nl->First(sarg2) == stream )) )
    { // (stream datatype)
      sarg2Type = nl->Second(sarg2);
      sarg2isstream = true;
    }
  else // wrong type for sarg2
    {
      ErrorReporter::ReportError(
        "Operator suse2 expects its second Argument to "
        "be of type 'T' or '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );           
    }
  
  // check sarg2 to be a (stream T) for T in kind DATA 
  // or T of type tuple(X)
  if(    !nl->IsAtom( sarg2Type ) 
         && !am->CheckKind("DATA", nl->Second( sarg2Type ), errorInfo) )
    { // T is not of kind DATA
      nl->WriteToString(outstr2, sarg2Type);      
      ErrorReporter::ReportError("Operator suse2 expects its 2nd argument "
                                 "to be '(stream T)', T of kind DATA, but"
                                 "receives '" + outstr1 + "' as T.");
      
      return nl->SymbolAtom( "typeerror" );      
    }
  else if ( !nl->IsAtom( sarg2Type ) &&
            ( (nl->ListLength( sarg2Type ) != 2) ||
              !nl->IsEqual( nl->First(sarg2Type), "tuple") ||
              !IsTupleDescription(nl->Second(sarg2Type))
              ) 
            )
    { // neither T is tuple(X)
      nl->WriteToString(outstr1, sarg2Type);
      ErrorReporter::ReportError("Operator suse2 expects its 2nd argument "
                                 "to be 'T' or '(stream T), T of kind DATA "
                                 "or of type 'tuple(X))', but"
                                 "receives '" + outstr1 + "' for T.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 4" << endl;
  
  // 4. First and Second argument
  // check whether at least one stream argument is present
  if ( !sarg1isstream && !sarg2isstream )
    {
      ErrorReporter::ReportError(
        "Operator suse2 expects at least one of its both first "
        "argument to be of type '(stream T), for T in kind DATA)'.");
      return nl->SymbolAtom( "typeerror" );           
    }

  if (TUA_DEBUG) cout << "TypeMapSuse2: 5" << endl;
  
  // 5. Third argument
  // check third for being a map
  if (  nl->IsAtom( map ) || !( nl->IsEqual(nl->First(map), "map") ) )
    {
      nl->WriteToString(outstr1, map);
      ErrorReporter::ReportError("Operator suse2 expects a map as "
                                 "3rd argument, but gets '" + outstr1 +
                                 "' instead.");
      return nl->SymbolAtom( "typeerror" );
    }    
  
  if ( nl->ListLength(map) != 4 )       
    {
      ErrorReporter::ReportError("Number of map arguments must be 2 "
                                 "for operator suse2.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // get map arguments
  marg1 = nl->Second(map);
  marg2 = nl->Third(map);
  mres  = nl->Fourth(map);

  // check marg1
  
  if ( !( nl->Equal(marg1, sarg1Type) ) )
    {
      nl->WriteToString(outstr1, sarg1Type);
      nl->WriteToString(outstr2, marg1);
      ErrorReporter::ReportError("Operator suse2: 1st argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 1st argument. If e.g. the first "
                                 "is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 + 
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // check marg2
  if ( !( nl->Equal(marg2, sarg2Type) ) )
    {
      nl->WriteToString(outstr1, sarg2Type);
      nl->WriteToString(outstr2, marg2);
      ErrorReporter::ReportError("Operator suse2: 2nd argument's stream"
                                 "type does not match the type of the "
                                 "mapping's 2nd argument. If e.g. the second"
                                 " is 'stream X', then the latter must be 'X'."
                                 "The types passed are '" + outstr1 + 
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 6" << endl;
  
  // 6. Determine result type
  // get map result type 'sresType'
  if( !nl->IsAtom( mres )  && ( nl->ListLength( mres ) == 2) ) 
    { 
      if (TUA_DEBUG) cout << "TypeMapSuse2: 6.1" << endl;
      
      if (  TypeOfRelAlgSymbol(nl->First(mres) == stream ) )
        {
          if (TUA_DEBUG) cout << "TypeMapSuse2: 6.2" << endl;
          
          if ( !am->CheckKind("DATA", nl->Second(mres), errorInfo) &&
               !( !nl->IsAtom(nl->Second(mres)) &&
                  nl->ListLength(nl->Second(mres)) == 2 &&
                  TypeOfRelAlgSymbol(nl->First(nl->Second(mres)) == tuple) &&
                  IsTupleDescription(nl->Second(nl->Second(mres)))
                  ) 
               )
            {
              if (TUA_DEBUG) cout << "TypeMapSuse2: 6.3" << endl;
              
              ErrorReporter::ReportError(
                "Operator suse2 expects its 3rd Argument to "
                "return a '(stream T)', T of kind DATA or T = 'tuple(X)'.");
              return nl->SymbolAtom( "typeerror" );           
            }
          if (TUA_DEBUG) cout << "TypeMapSuse2: 6.4" << endl;
          
          resisstream = true;
          sresType = mres; // map result type is already a stream
        }
    }
  else // map result type is not a stream, so encapsulate it
    {
      if (TUA_DEBUG) cout << "TypeMapSuse2: 6.5" << endl;
      
      if (    !( nl->IsAtom(mres) && am->CheckKind("DATA", mres, errorInfo))  
              && !( !nl->IsAtom(mres) && 
                    nl->ListLength(mres) == 2 &&
                    !nl->IsAtom(nl->Second(mres)) &&
                    TypeOfRelAlgSymbol(nl->First(mres)  == tuple) &&
                    IsTupleDescription(nl->Second(mres))
                    )
              ) 
        {
          if (TUA_DEBUG) cout << "TypeMapSuse2: 6.6" << endl;
          
          ErrorReporter::ReportError(
            "Operator suse2 expects its 3rd Argument to "
            "return a type T of kind DATA or T = 'tuple(X)'.");
          return nl->SymbolAtom( "typeerror" );       
        }
      if (TUA_DEBUG) cout << "TypeMapSuse2: 6.7" << endl;
      
      resisstream = false;
      sresType = nl->TwoElemList(nl->SymbolAtom("stream"), mres);  
    }
  
  if(TUA_DEBUG) cout << "TypeMapSuse2: 7" << endl;
  
  // 7. This check can be removed when operators working on tuplestreams have
  //    been implemented:
  if (   (!nl->IsAtom(sarg1Type) && 
          TypeOfRelAlgSymbol(nl->First(sarg1Type)) == tuple )
         || (!nl->IsAtom(sarg1Type) && 
             TypeOfRelAlgSymbol(nl->First(sarg1Type)) == tuple ) )
    {
      ErrorReporter::ReportError("Operator suse2 still not implemented for "
                                 "arguments of type 'tuple(X)' or "
                                 "'(stream tuple(X))'.");
      return nl->SymbolAtom( "typeerror" );      
    }
  
  
  if (TUA_DEBUG) cout << "TypeMapSuse2: 8" << endl;
  
  // 8. Append flags describing argument configuration for value mapping:
  //     0: no stream
  //     1: sarg1 is a stream
  //     2: sarg2 is a stream
  //     4: map result is a stream
  //
  //    e.g. 7=4+2+1: both arguments are streams and the 
  //                  map result is a stream
  
  if(sarg1isstream) argConfCode += 1;
  if(sarg2isstream) argConfCode += 2;
  if(resisstream)   argConfCode += 4;
  
  argConfDescriptor = nl->OneElemList(nl->IntAtom(argConfCode));
  return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                           argConfDescriptor, sresType);  
}

/*
5.20.2 Value Mapping for ~suse~

*/

struct SuseLocalInfo{
  bool Xfinished, Yfinished, funfinished; // whether we have finished
  Word X, Y, fun;                         // pointers to the argument nodes
  Word XVal, YVal, funVal;                // the last arg values
  int  argConfDescriptor;          // type of argument configuration
};

// (stream X) (map X Y) -> (stream Y)
int Suse_SN( Word* args, Word& result, int message,
             Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              instream = args[0], fun = args[1]; 
  Word              funResult, argValue;
  ArgVectorPointer  funArgs;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "Suse_SN received OPEN" << endl;
      sli = new SuseLocalInfo;
      sli->Xfinished = true;
      qp->Open(instream.addr);
      sli->Xfinished = false;
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "Suse_SN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it to the parameter function and evalute the latter.
      // The result is simply passed on.
      
      if(TUA_DEBUG) cout << "Suse_SN received REQUEST" << endl;
      if( local.addr == 0 )
        {
          if(TUA_DEBUG) cout << "Suse_SN finished REQUEST: CANCEL (1)" << endl;
          return CANCEL;
        }
      sli = (SuseLocalInfo*)local.addr;
      
      if (sli->Xfinished)
        {
          if(TUA_DEBUG) cout << "Suse_SN finished REQUEST: CANCEL (2)" << endl;
          return CANCEL;
        }
      
      funResult.addr = 0;
      argValue.addr  = 0;
      qp->Request(instream.addr, argValue); // get one arg value from stream
      if(qp->Received(instream.addr))       
        {
          funArgs = qp->Argument(fun.addr); // set argument for the
          (*funArgs)[0] = argValue;         //   parameter function
          qp->Request(fun.addr, funResult); // call parameter function
          // copy result:
          result = SetWord(((Attribute*) (funResult.addr))->Clone());
          ((Attribute*) (argValue.addr))->DeleteIfAllowed(); // delete argument
          if(TUA_DEBUG) 
            cout << "        result.addr    =" << result.addr << endl;
          argValue.addr = 0;
          if(TUA_DEBUG) 
            cout << "Suse_SN finished REQUEST: YIELD" << endl;
          return YIELD;
        }
      else // (input stream consumed completely)
        {
          qp->Close(instream.addr);
          sli->Xfinished = true;
          result.addr = 0;
          if(TUA_DEBUG) 
            cout << "Suse_SN finished REQUEST: CANCEL (3)" << endl;  
          return CANCEL;
        }
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "Suse_SN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;
          if ( !sli->Xfinished )
            qp->Close( instream.addr );
          delete sli;
        }
      if(TUA_DEBUG) cout << "Suse_SN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "Suse_SN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}  


// (stream X) (map X (stream Y)) -> (stream Y)
int Suse_SS( Word* args, Word& result, int message,
             Word& local, Supplier s )
{
  SuseLocalInfo    *sli;
  Word             funResult;  
  ArgVectorPointer funargs;
  
  switch (message)
    {
    case OPEN :
      
      sli = new SuseLocalInfo;
      sli->X   = SetWord( args[0].addr );
      sli->fun = SetWord( args[1].addr );
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->XVal.addr   = 0;
      // open the ("outer") input stream and
      qp->Open( sli->X.addr );
      sli->Xfinished = false;
      // save the local information
      local = SetWord(sli);
      
      return 0;
      
    case REQUEST :
      
      // For each value from the 'outer' stream, an 'inner' stream
      // of values is generated by the parameter function.
      // For each REQUEST, we pass one value from the 'inner' stream
      // as the result value.
      // If the inner stream is consumed, we try to get a new value
      // from the 'outer' stream and re-open the inner stream 
      
      if(TUA_DEBUG) cout << "\nSuse_SS: Received REQUEST";
      //1. recover local information
      if( local.addr == 0 )
        return CANCEL;
      sli = (SuseLocalInfo*)local.addr;
      
      // create the next result 
      while( !sli->Xfinished )
        {
          if( sli->funfinished )
            {// end of map result stream reached -> get next X
              qp->Request( sli->X.addr, sli->XVal);
              if (!qp->Received( sli->X.addr ))
                { // Stream X is exhaused
                  qp->Close( sli->X.addr );
                  sli->Xfinished = true;
                  result.addr = 0;
                  return CANCEL;
                } // got an X-elem
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              qp->Open( sli->fun.addr );
              sli->funfinished = false;
            } // Now, we have an open map result stream         
          qp->Request( sli->fun.addr, funResult );
          if(qp->Received( sli->fun.addr ))
            { // cloning and passing the result
              result = SetWord(((Attribute*) (funResult.addr))->Clone());
              ((Attribute*) (funResult.addr))->DeleteIfAllowed(); 
              if(TUA_DEBUG) cout << "     result.addr=" << result.addr << endl;
              return YIELD;      
            }
          else 
            { // end of map result stream reached
              qp->Close( sli->fun.addr );
              sli->funfinished = true;      
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();           
            }
        } // end while
      
    case CLOSE :
      
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;               
          if( !sli->funfinished )
            {
              qp->Close( sli->fun.addr );
              ((Attribute*)(sli->X.addr))->DeleteIfAllowed();   
            }
          if ( !sli->Xfinished )
            qp->Close( sli->X.addr );
          delete sli;
        }
      return 0;
    }  // end switch
  cout << "\nSuse_SS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}  


// (stream X) Y          (map X Y Z) -> (stream Z)
// X          (stream Y) (map X y Z) -> (stream Z)
int Suse_SNN( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              xval, funresult;
  ArgVectorPointer  funargs;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\nSuse_SNN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun = SetWord(args[2].addr); 
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
        { 
          delete( sli );
          local.addr = 0;
          if(TUA_DEBUG) 
            cout << "\nSuse_SNN was called with stream result mapping!" 
                 <<  endl;
          return 0;
        }
      if(sli->argConfDescriptor & 1)
        { // the first arg is the stream
          sli->X = SetWord(args[0].addr); // X is the stream
          sli->Y = SetWord(args[1].addr); // Y is the constant value
        } 
      else
        { // the second arg is the stream
          sli->X = SetWord(args[1].addr); // X is the stream
          sli->Y = SetWord(args[0].addr); // Y is the constant value
        }
      
      qp->Open(sli->X.addr);              // open outer stream argument
      sli->Xfinished = false;
      
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // For each REQUEST, we get one value from the stream,
      // pass it (and the remaining constant argument) to the parameter 
      // function and evalute the latter. The result is simply passed on.
      // sli->X is the stream, sli->Y the constant argument.
      
      if(TUA_DEBUG) cout << "Suse_SNN received REQUEST" << endl;
      
      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (SuseLocalInfo*) local.addr;
      if (sli->Xfinished)
        { // stream already exhausted earlier
          result.addr = 0;
          if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: CLOSE (2)" << endl;
          return CANCEL;
        }
      
      // 2. request value from outer stream
      qp->Request( sli->X.addr, xval );
      if(!qp->Received( sli->X.addr ))
        { // stream exhausted now
          qp->Close( sli->X.addr );
          sli->Xfinished = true;
          if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
          return CANCEL;
        }
      
      // 3. call parameter function, delete args and return result
      funargs = qp->Argument( sli->fun.addr );
      if (sli->argConfDescriptor & 1)
        {
          (*funargs)[0] = xval;
          (*funargs)[1] = sli->Y;
        }
      else
        {
          (*funargs)[0] = sli->Y;
          (*funargs)[1] = xval;
        }
      qp->Request( sli->fun.addr, funresult );     
      result = SetWord(((Attribute*) (funresult.addr))->Clone());
      if(TUA_DEBUG) cout << "     result.addr=" << result.addr << endl;
      ((Attribute*) (xval.addr))->DeleteIfAllowed(); 
      if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: YIELD" << endl;
      return YIELD;
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close input
          delete sli;
        }
      if(TUA_DEBUG) cout << "Suse_SNN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "\nSuse_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}


// (stream X) Y          (map X Y (stream Z)) -> (stream Z)
// X          (stream Y) (map X y (stream Z)) -> (stream Z)
int Suse_SNS( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  
  SuseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\nSuse_SNS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->funfinished = true;
      sli->X.addr = 0;
      sli->Y.addr = 0;  
      sli->fun.addr = 0;
      sli->XVal.addr = 0;
      sli->YVal.addr = 0;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(! (sli->argConfDescriptor & 4))
        { 
          delete( sli );
          local.addr = 0;
          cout << "\nSuse_SNS was called with non-stream result mapping!" 
               <<  endl;          
          return 0;
        }
      if(sli->argConfDescriptor & 1)
        { // the first arg is the stream
          sli->X = SetWord(args[0].addr); // X is the stream
          sli->Y = SetWord(args[1].addr); // Y is the constant value
        } 
      else
        { // the second arg is the stream
          sli->X = SetWord(args[1].addr); // X is the stream
          sli->Y = SetWord(args[0].addr); // Y is the constant value
        }
      sli->YVal = sli->Y; // save value of constant argument
      qp->Open(sli->X.addr);               // open the ("outer") input stream
      sli->Xfinished = false;
      sli->fun = SetWord(args[2].addr);
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "Suse_SNN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // First, we check whether an inner stream is finished 
      // (sli->funfinished). If so, we try to get a value from 
      // the outer stream and try to re-open the inner stream.
      // sli->X is a pointer to the OUTER stream, 
      // sli->Y is a pointer to the constant argument.
      
      if(TUA_DEBUG) cout << "Suse_SNN received REQUEST" << endl;
      
      // 1. get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (SuseLocalInfo*) local.addr;
      // 2. request values from inner stream
      while (!sli->Xfinished)
        {
          while (sli->funfinished)
            { // the inner stream is closed, try to (re-)open it
              // try to get the next X-value from outer stream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // stream X exhaused. CANCEL
                  sli->Xfinished = true;
                  if(TUA_DEBUG) 
                    cout << "Suse_SNN finished REQUEST: CLOSE (3)" << endl;
                  return CANCEL;
                }
              funargs = qp->Argument( sli->fun.addr );
              if (sli->argConfDescriptor & 1)
                {
                  (*funargs)[0] = sli->XVal;
                  (*funargs)[1] = sli->YVal;      
                }
              else
                {
                  (*funargs)[0] = sli->YVal;
                  (*funargs)[1] = sli->XVal;      
                }
              qp->Open( sli->fun.addr );
              sli->funfinished = false;
            } // end while - Now, the inner stream is open again
          qp->Request(sli->fun.addr, funresult);
          if (qp->Received(sli->fun.addr))
            { // inner stream returned a result
              result = SetWord(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (funresult.addr))->DeleteIfAllowed(); 
              if(TUA_DEBUG) 
                {
                  cout << "     result.addr=" << result.addr << endl;
                  cout << "Suse_SNN finished REQUEST: YIELD" << endl;
                }
              return YIELD;
            }
          else{ // inner stream exhausted
            qp->Close(sli->fun.addr);
            sli->funfinished = true;
            ((Attribute*)(sli->XVal.addr))->DeleteIfAllowed();
            sli->XVal.addr = 0;
          }
        } // end while
      result.addr = 0;
      if(TUA_DEBUG) cout << "Suse_SNN finished REQUEST: CLOSE (4)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "Suse_SNN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;
          if (!sli->funfinished)
            qp->Close( sli->fun.addr ); // close map result stream
          if (!sli->Xfinished)
            qp->Close( sli->X.addr );   // close outer stream
          delete sli;
        }
      if(TUA_DEBUG) cout << "Suse_SNN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "Suse_SNN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
  
}

// (stream X) (stream Y) (map X Y Z) -> (stream Z)
int Suse_SSN( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\nSuse_SSN received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished = true;
      sli->Yfinished = true;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(sli->argConfDescriptor & 4)
        { 
          delete( sli );
          local.addr = 0;
          cout << "\nSuse_SSN was called with stream result mapping!" 
               <<  endl;
          return 0;
        }
      if(!(sli->argConfDescriptor & 3))
        { 
          delete( sli );
          local.addr = 0;
          cout << "\nSuse_SSN was called with non-stream arguments!" 
               <<  endl;
          return 0;
        }
      sli->X = SetWord(args[0].addr);   // X is the stream
      sli->Y = SetWord(args[1].addr);   // Y is the constant value
      sli->fun = SetWord(args[2].addr); // fun is the mapping function
      
      qp->Open(sli->X.addr);            // open outer stream argument
      sli->Xfinished = false;
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "Suse_SSN finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we evaluate the
      // parameter function (sli->fun), which return a single result.
      // A clone of the result is passed as the result.
      // We also need to delete each element, when it is not required
      // anymore.
      
      if(TUA_DEBUG) cout << "Suse_SSN received REQUEST" << endl;
      
      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(TUA_DEBUG) cout << "Suse_SSN finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (SuseLocalInfo*) local.addr;
      
      while(!sli->Xfinished)
        {
          if (sli->Yfinished)
            { // try to (re-) start outer instream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // outer instream exhaused
                  qp->Close(sli->X.addr);
                  sli->Xfinished = true;
                  result.addr = 0;
                  if(TUA_DEBUG) 
                    cout << "Suse_SSN finished REQUEST: CANCEL (2)" << endl;
                  return CANCEL;
                } 
              // Got next X-elem. (Re-)Start inner instream:
              qp->Open(sli->Y.addr);
              sli->Yfinished = false;
            } 
          // Now, we have open inner and outer streams
          qp->Request(sli->Y.addr, sli->YVal);
          if (!qp->Received(sli->Y.addr))
            { // inner stream is exhausted
              qp->Close(sli->Y.addr);
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
              sli->Yfinished = true;
            } 
          // got next Y-elem
          if (!sli->Xfinished && !sli->Yfinished)
            { // pass parameters and call mapping, clone result
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              (*funargs)[1] = sli->YVal;
              qp->Request( sli->fun.addr, funresult );
              result = SetWord(((Attribute*) (funresult.addr))->Clone());
              ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed(); 
              if(TUA_DEBUG) cout << "Suse_SSN finished REQUEST: YIELD" << endl;
              return YIELD;
            }
        } // end while
      if(TUA_DEBUG) cout << "Suse_SSN finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "Suse_SSN received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;
          if (!sli->Yfinished)
            {
              qp->Close( sli->Y.addr ); // close inner instream
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close outer instream
          delete sli;
        }
      result.addr = 0;
      if(TUA_DEBUG) cout << "Suse_SSN finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  result.addr = 0;
  cout << "\nSuse_SSN received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



// (stream X) (stream Y) (map X Y (stream Z)) -> (stream Z)
int Suse_SSS( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  SuseLocalInfo     *sli;
  Word              funresult;
  ArgVectorPointer  funargs;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\nSuse_SSS received OPEN" << endl;
      sli = new SuseLocalInfo ;
      sli->Xfinished   = true;
      sli->Yfinished   = true;
      sli->funfinished = true;
      // get argument configuration info
      sli->argConfDescriptor = ((CcInt*)args[3].addr)->GetIntval();
      if(!(sli->argConfDescriptor & 4) )
        { 
          delete( sli );
          local.addr = 0;
          cout << "\nSuse_SSS was called with non-stream result mapping!" 
               <<  endl;
          return 0;
        }
      if(!(sli->argConfDescriptor & 3))
        { 
          delete( sli );
          local.addr = 0;
          cout << "\nSuse_SSS was called with non-stream arguments!" 
               <<  endl;
          return 0;
        }
      sli->X   = args[0]; // X is the stream
      sli->Y   = args[1]; // Y is the constant value
      sli->fun = args[2]; // fun is the mapping function
      qp->Open(sli->X.addr);            // open X stream argument      
      sli->Xfinished = false;
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "Suse_SSS finished OPEN" << endl;
      return 0;
      
    case REQUEST :
      
      // We do a nested loop to join the elements of the outer (sli->X)
      // and inner (sli->Y) stream. For each pairing, we open the
      // parameter function (sli->fun), which returns a stream result.
      // We consume this map result stream one-by-one.
      // When it is finally consumed, we try to restart it with the next 
      // X/Y value pair.
      // A clone of the result is passed as the result.
      // We also need to delete each X/Y element, when it is not required
      // any more.
      
      if(TUA_DEBUG) cout << "Suse_SSS received REQUEST" << endl;
      
      // get local data object
      if (local.addr == 0)
        {
          result.addr = 0;
          if(TUA_DEBUG) cout << "Suse_SSS finished REQUEST: CLOSE (1)" << endl;
          return CANCEL;
        }
      sli = (SuseLocalInfo*) local.addr;
      
      while(!sli->Xfinished)
        {
          if (sli->Yfinished)
            { // get next X-value from outer instream
              // and restart inner (Y-) instream
              qp->Request(sli->X.addr, sli->XVal);
              if (!qp->Received(sli->X.addr))
                { // X-instream exhaused
                  qp->Close(sli->X.addr);
                  sli->Xfinished = true;
                  if(TUA_DEBUG) 
                    cout << "Suse_SSS finished REQUEST: CANCEL (2)" << endl;
                  result.addr = 0;
                  return CANCEL;
                } 
              // Got next X-elem. (Re-)Start inner Y-instream:
              qp->Open(sli->Y.addr);
              sli->Yfinished = false;
            } // Now, we have open X- and Y- streams
          if (sli->funfinished)
            { // get next Y-value from inner instream
              // and open new map result stream
              qp->Request(sli->Y.addr, sli->YVal);
              if (!qp->Received(sli->Y.addr))
                {
                  qp->Close(sli->Y.addr);
                  ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
                  sli->Yfinished = true;                  
                }
              else
                {
                  funargs = qp->Argument( sli->fun.addr );
                  (*funargs)[0] = sli->XVal;
                  (*funargs)[1] = sli->YVal;              
                  qp->Open( sli->fun.addr );
                  sli->funfinished = false;
                }
            }
          // Now, we have an open map result streams
          if (!sli->Xfinished && !sli->Yfinished && !sli->funfinished)
            { // pass parameters and call mapping, clone result
              funargs = qp->Argument( sli->fun.addr );
              (*funargs)[0] = sli->XVal;
              (*funargs)[1] = sli->YVal;
              qp->Request( sli->fun.addr, funresult );
              if ( qp->Received(sli->fun.addr) )
                { // got a value from map result stream
                  result=SetWord(((Attribute*)(funresult.addr))->Clone());
                  ((Attribute*) (funresult.addr))->DeleteIfAllowed();
                  if(TUA_DEBUG) 
                    cout << "Suse_SSS finished REQUEST: YIELD" << endl;
                  return YIELD;
                }
              else
                { // map result stream exhausted
                  qp->Close( sli->fun.addr) ;
                  ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
                  sli->funfinished = true;
                } // try to restart with new X/Y pairing
            }
        } // end while
      result.addr = 0;
      if(TUA_DEBUG) cout << "Suse_SSS finished REQUEST: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "Suse_SSS received CLOSE" << endl;
      if( local.addr != 0 )
        {
          sli = (SuseLocalInfo*)local.addr;
          if (!sli->funfinished)
            {
              qp->Close( sli->fun.addr ); // close map result stream
              // Delete current Y-elem:
              ((Attribute*) (sli->YVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Yfinished)
            {
              qp->Close( sli->Y.addr ); // close inner instream
              // Delete current X-elem:
              ((Attribute*) (sli->XVal.addr))->DeleteIfAllowed();
            }
          if (!sli->Xfinished)
            qp->Close( sli->X.addr ); // close outer instream
          delete sli;
        }
      if(TUA_DEBUG) cout << "Suse_SSS finished CLOSE" << endl;
      return 0;
      
    }  // end switch
  cout << "\nSuse_SSS received UNKNOWN COMMAND" << endl;
  return -1; // should not be reached
}



/*
5.20.3 Specification for operator ~suse~

*/
const string
TemporalSpecSuse=
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For X in kind DATA or X = tuple(Z)*, Y in kind DATA:\n"
  "(*: not yet implemented)\n"
  "((stream X) (map X Y)         ) -> (stream Y) \n"
  "((stream X) (map X (stream Y))) -> (stream Y)</text--->"
  "<text>_ suse [ _ ]</text--->"
  "<text>The suse class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values.</text--->"
  "<text>query intstream(1,5) suse[ fun(i:int) i*i ] printstream count;\n"
  "query intstream(1,5) suse[ fun(i:int) intstream(i,5) ] printstream count;"
  "</text---> ) )";

const string
TemporalSpecSuse2=
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For X in kind DATA or X = tuple(W)*, Y,Z in kind DATA:\n"
  "(*: not yet implemented)\n"
  "((stream X) Y          (map X Y Z)         ) -> (stream Z) \n"
  "((stream X) Y          (map X Y stream(Z)) ) -> (stream Z) \n"
  "(X          (stream Y) (map X Y Z)         ) -> (stream Z) \n"
  "(X          (stream Y) (map X Y (stream Z))) -> (stream Z) \n"
  "((stream X) (stream Y) (map X Y Z)         ) -> (stream Z) \n"
  "((stream X) (stream Y) (map X Y (stream Z))) -> (stream Z)</text--->"
  "<text>_ _ suse2 [ _ ]</text--->"
  "<text>The suse2 class of operators implements "
  "a set of functors, that derive stream-valued "
  "operators from operators taking scalar "
  "arguments and returning scalar values or "
  "streams of values. suse2 performs a product "
  "between the two first of its arguments, passing each "
  "combination to the mapped function once.</text--->"
  "<text>query intstream(1,5) [const int value 5] suse2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query [const int value 3] intstream(1,5) suse2[ fun(i:int, j:int) i+j ] "
  "printstream count;\n"
  "query intstream(1,5) [const int value 3] suse2[ fun(i:int, j:int) i+j ] "
  "printstream count;\n"
  "query [const int value 2] intstream(1,5) suse2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query [const int value 3] intstream(1,5) suse2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;\n"
  "query intstream(1,2) intstream(1,3) suse2[ fun(i:int, j:int) "
  "intstream(i,j) ] printstream count;</text---> ) )";

/* 
5.20.4 Selection Function of operator ~suse~

*/

ValueMapping temporalunitsusemap[] = 
  { Suse_SN,
    Suse_SS 
    //    ,
    //    Suse_TsN,
    //    Suse_TsS
  };

int
temporalunitSuseSelect( ListExpr args )
{ 
  ListExpr sarg1     = nl->First( args );
  ListExpr mapresult = nl->Third(nl->Second(args));
  bool     isStream  = false;
  bool     isTuple   = false;
  
  // check type of sarg1
  if( TypeOfRelAlgSymbol(nl->First(sarg1)) == stream &&
      (!nl->IsAtom(nl->Second(sarg1))) &&
      (nl->ListLength(nl->Second(sarg1)) == 2) &&
      TypeOfRelAlgSymbol(nl->First(nl->Second(sarg1))) == tuple &&
      IsTupleDescription(nl->Second((nl->Second(sarg1)))) )
    isTuple = true;
  else 
    isTuple = false;
  
  // check type of map result (stream or non-stream type)
  if(   !( nl->IsAtom(mapresult) ) 
        && ( nl->ListLength( mapresult ) == 2)
        && TypeOfRelAlgSymbol(nl->First(sarg1) == stream ) )
    isStream = true;
  else 
    isStream = false;  
  
  // compute index without offset
  if      (!isTuple && !isStream) return 0;
  else if (!isTuple &&  isStream) return 1;
  else if ( isTuple && !isStream) return 2;
  else if ( isTuple &&  isStream) return 3;
  else
    {
      cout << "\ntemporalunitSuseSelect: Something's wrong!\n";
    }
  return -1;
}


ValueMapping temporalunitsuse2map[] =
  { Suse_SNN,
    Suse_SNS,
    Suse_SSN,
    Suse_SSS
    //     ,
    //    Suse_TsNN,
    //    Suse_TsNS,
    //    Suse_TsTsN,
    //    Suse_TsTsS
  };

int
temporalunitSuse2Select( ListExpr args )
{ 
  ListExpr 
    X = nl->First(args), 
    Y = nl->Second(args),
    M = nl->Third(args);
  bool 
    xIsStream = false, 
    yIsStream = false, 
    resIsStream = false;
  bool 
    xIsTuple = false,
    yIsTuple = false, 
    resIsTuple = false;
  int index = 0;
  
  // examine first arg
  // check type of sarg1
  if( nl->IsAtom(X) )
    { xIsTuple = false; xIsStream = false;}
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == tuple )
    { xIsTuple = true; xIsStream = false; }
  if( !nl->IsAtom(X) &&
      TypeOfRelAlgSymbol(nl->First(X)) == stream )
    {
      xIsStream = true;
      if(!nl->IsAtom(nl->Second(X)) &&
         (nl->ListLength(nl->Second(X)) == 2) &&
         TypeOfRelAlgSymbol(nl->First(X)) == tuple )
        xIsTuple = true;
      else
        xIsTuple = false;
    }
  
  // examine second argument
  if( nl->IsAtom(Y) )
    { yIsTuple = false; yIsStream = false;}
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
    { yIsTuple = true; yIsStream = false; }
  if( !nl->IsAtom(Y) &&
      TypeOfRelAlgSymbol(nl->First(Y)) == stream )
    {
      yIsStream = true;
      if(!nl->IsAtom(nl->Second(Y)) &&
         (nl->ListLength(nl->Second(Y)) == 2) &&
         TypeOfRelAlgSymbol(nl->First(Y)) == tuple )
        yIsTuple = true;
      else
        yIsTuple = false;
    }
  
  // examine mapping result type
  if( nl->IsAtom(nl->Fourth(M)) )
    { resIsTuple = false; resIsStream = false;}
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
    { resIsTuple = true; resIsStream = false; }
  if( !nl->IsAtom(nl->Fourth(M)) &&
      TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == stream )
    {
      resIsStream = true;
      if(!nl->IsAtom(nl->Second(nl->Fourth(M))) &&
         (nl->ListLength(nl->Second(nl->Fourth(M))) == 2) &&
         TypeOfRelAlgSymbol(nl->First(nl->Fourth(M))) == tuple )
        resIsTuple = true;
      else
        resIsTuple = false;
    }
  
  // calculate appropriate index value
  
  // tuple variants offest    : +4
  // both args streams        : +2
  // mapping result is stream : +1
  index = 0;
  if ( xIsTuple || yIsTuple )   index += 4;
  if ( xIsStream && yIsStream ) index += 2;
  if ( resIsStream )            index += 1;
  
  if (index > 3)
    cout << "\nWARNING: index =" << index 
         << ">3 in temporalunitSuse2Select!" << endl;
  
  return index;
}

/*
5.20.5  Definition of operator ~suse~

*/


Operator temporalunitsuse( "suse",
                           TemporalSpecSuse,
                           2,
                           temporalunitsusemap,
                           temporalunitSuseSelect,
                           TypeMapSuse);


Operator temporalunitsuse2( "suse2",
                            TemporalSpecSuse2,
                            4,
                            temporalunitsuse2map,
                            temporalunitSuse2Select,
                            TypeMapSuse2);

/*
5.21 Operator ~distance~

The operator calculates the minimum distance between two units of base
types ~int~ or ~point~. The distance is always a non-negative ~ureal~ value.

----
    For T in {int, point} 
    distance: uT x uT -> ureal
              uT x  T -> ureal 
               T x uT -> ureal

----


5.21.1 Type mapping function for ~distance~

For signatures 

----       
           For T in {int, point}:
              uT x  T -> ureal 
               T x uT -> ureal

----

typemapping appends the argument number of the argument containing
the unitvalue (either 0 or 1).

*/

ListExpr 
TypeMapTemporalUnitDistance( ListExpr args )
{
  ListExpr first, second;
  string outstr1, outstr2;
  
  if ( nl->IsAtom( args ) || nl->ListLength( args ) != 2 )
    {
      nl->WriteToString(outstr1, args);
      ErrorReporter::ReportError("Operator distance expects a list of "
                                 "length two, but gets '" + outstr1 + 
                                 "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  first = nl->First(args);
  second = nl->Second(args);
  
  // check for compatibility of arguments
  if( !nl->IsAtom(first) || !nl->IsAtom(second) )
    {
      nl->WriteToString(outstr1, first);
      nl->WriteToString(outstr2, second);
      ErrorReporter::ReportError("Operator distance expects as arguments "
                                 "lists of length one, but gets '" + outstr1 + 
                                 "' and '" + outstr2 + "'.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  if( nl->IsEqual(first, "upoint") && nl->IsEqual(second, "upoint") )
    { 
      return nl->SymbolAtom("ureal");
    }
  
  if( nl->IsEqual(first, "upoint") && nl->IsEqual(second, "point") )
    { 
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                               nl->OneElemList(nl->IntAtom(0)), 
                               nl->SymbolAtom( "ureal" ));  
    }
  
  if( nl->IsEqual(first, "point") && nl->IsEqual(second, "upoint") )
    { 
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                               nl->OneElemList(nl->IntAtom(1)), 
                               nl->SymbolAtom( "ureal" ));  
    }
  
  if( nl->IsEqual(first, "uint") && nl->IsEqual(second, "uint") )
    { 
      return nl->SymbolAtom("ureal");
    }
    
  if( nl->IsEqual(first, "uint") && nl->IsEqual(second, "int") )
    { 
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                               nl->OneElemList(nl->IntAtom(0)), 
                               nl->SymbolAtom( "ureal" ));  
    }
  
  if( nl->IsEqual(first, "int") && nl->IsEqual(second, "uint") )
    { 
      return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                               nl->OneElemList(nl->IntAtom(1)), 
                               nl->SymbolAtom( "ureal" ));  
    }
  
  nl->WriteToString(outstr1, first);
  nl->WriteToString(outstr2, second);
  ErrorReporter::ReportError("Operator distance found wrong argument "
                             "configuration '" + outstr1 + 
                             "' and '" + outstr2 + "'.");
  return nl->SymbolAtom( "typeerror" );
}

/*
5.21.2 Value mapping for operator ~distance~
*/


/*
(a1) Value mapping for 

---- (upoint upoint) -> ureal

----

Method ~UPointDistance~

Returns the distance between two UPoints in the given interval as UReal 

*/
void UPointDistance( const UPoint& p1, const UPoint& p2, 
                     UReal& result, Interval<Instant>& iv)
{
  if (TUA_DEBUG)
    {
      cout << "UPointDistance:" << endl;
      cout << "  p1=" << TUPrintUPoint(p1) << endl;
      cout << "  p2=" << TUPrintUPoint(p2) << endl;
    }
  result.timeInterval = iv;
  
  Point rp10, rp11, rp20, rp21;
  double 
    x10, x11, x20, x21, 
    y10, y11, y20, y21, 
    dx1, dy1, 
    dx2, dy2,
    dx12, dy12;

  // for calculation of temporal function:
  // ignore temporal limits
  p1.TemporalFunction(iv.start, rp10, true);
  p1.TemporalFunction(iv.end,   rp11, true);
  p2.TemporalFunction(iv.start, rp20, true);
  p2.TemporalFunction(iv.end,   rp21, true);

  if (TUA_DEBUG)
    {
      cout << "   iv=" << TUPrintTimeInterval(iv) << endl;
      cout << "  rp10=" << TUPrintPoint(rp10) << ", rp11=" 
           << TUPrintPoint(rp11) << endl;
      cout << "  rp20=" << TUPrintPoint(rp20) << ", rp21=" 
           << TUPrintPoint(rp21) << endl;
    }
  if ( AlmostEqual(rp10,rp20) && AlmostEqual(rp11,rp21) )
    { // identical points -> zero distance!
      if (TUA_DEBUG) 
        cout << "  identical points -> zero distance!" << endl;
      result.a = 0.0;
      result.b = 0.0;
      result.c = 0.0;
      result.r = false;
      return;
    }

  DateTime DT = iv.end - iv.start;
  double   dt = DT.ToDouble();
  //double   t0 = iv.start.ToDouble();
  x10 = rp10.GetX(); y10 = rp10.GetY();
  x11 = rp11.GetX(); y11 = rp11.GetY();
  x20 = rp20.GetX(); y20 = rp20.GetY();
  x21 = rp21.GetX(); y21 = rp21.GetY();
  dx1 = x11 - x10;   // x-difference final-initial for u1
  dy1 = y11 - y10;   // y-difference final-initial for u1
  dx2 = x21 - x20;   // x-difference final-initial for u2
  dy2 = y21 - y20;   // y-difference final-initial for u2
  dx12 = x10 - x20;  // x-distance at initial instant
  dy12 = y10 - y20;  // y-distance at initial instant

  if ( AlmostEqual(dt, 0) )
    { // almost equal start and end time -> constant distance
      if (TUA_DEBUG) 
        cout << "  almost equal start and end time -> constant distance!" 
             << endl;
      result.a = 0.0;
      result.b = 0.0;
      result.c =   pow( ( (x11-x10) - (x21-x20) ) / 2, 2) 
                 + pow( ( (y11-y10) - (y21-y20) ) / 2, 2);
      result.r = true;
      return;
    }

  if (TUA_DEBUG) 
    cout << "  Normal distance calculation." << endl;

  double a1 = (pow((dx1-dx2),2)+pow(dy1-dy2,2))/pow(dt,2);
  double b1 = dx12 * (dx1-dx2);
  double b2 = dy12 * (dy1-dy2);

  result.a = a1;
  result.b = 2*(b1+b2)/dt;
  result.c = pow(dx12,2) + pow(dy12,2);
  result.r = true;

/*
For using the original ureal representation (without translation), 
use the following code instead:

----

  result.a = a1;
  result.b = -2*(  (t0*a1) 
                 - ( b1 + b2 )/dt 
               );
  result.c =   pow(t0,2) * a1 
             - 2*t0*(b1 + b2)/dt
             + pow(dx12,2) + pow(dy12,2);
  result.r = true;

----

*/


}


int TUDistance_UPoint_UPoint( Word* args, Word& result, int message,
                              Word& local, Supplier s )
{
  Interval<Instant> iv;
  
  //  Word a1, a2;
  UPoint *u1, *u2;
  result = qp->ResultStorage( s );
  UReal* res = (UReal*) result.addr;
  
  if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 1" << endl;
  
  u1 = (UPoint*)(args[0].addr);
  u2 = (UPoint*)(args[1].addr);
  if (!u1->IsDefined() || 
      !u2->IsDefined() || 
      !u1->timeInterval.Intersects( u2->timeInterval ) )
    { // return undefined ureal
      res->SetDefined( false );
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 2" << endl;
    }
  else
    { // get intersection of deftime intervals
      if (TUA_DEBUG) 
        cout << "TUDistance_UPoint_UPoint:" << endl
             << "   iv1=" << TUPrintTimeInterval(u1->timeInterval) << endl
             << "   iv2=" << TUPrintTimeInterval(u2->timeInterval) << endl;

      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 3" << endl;
      u1->timeInterval.Intersection( u2->timeInterval, iv );  
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: iv=" 
                          << TUPrintTimeInterval(iv) << endl;
      
      // calculate result
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 5" << endl;
      UPointDistance( *u1, *u2,  *res, iv);
      if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 6" << endl;
      res->SetDefined( true );
    }
  // pass on result
  if (TUA_DEBUG) cout << "TUDistance_UPoint_UPoint: 7" << endl;
  return 0;
}



/*
(a2) value mapping for 

---- 
     (upoint point) -> ureal  
     (point upoint) -> ureal

----

*/

int TUDistance_UPoint_Point( Word* args, Word& result, int message,
                             Word& local, Supplier s )
{
  Word  thePoint, theUPoint;
  int   argConfDescriptor2;
  
  // get argument configuration
  argConfDescriptor2 = ((CcInt*)args[2].addr)->GetIntval();
  if (argConfDescriptor2 == 0) 
    {
      theUPoint = args[0];
      thePoint  = args[1];
    }
  else if (argConfDescriptor2 == 1)
    {
      theUPoint = args[1];
      thePoint  = args[0];
    }
  else
    {
      cout << "\nWrong argument configuration in "
           << "'TUDistance_UPoint_Point'. argConfDescriptor2=" 
           << argConfDescriptor2 << endl;
      return 0;
    }
  
  result = qp->ResultStorage( s );
  
  if ( !((Point*)(thePoint.addr))->IsDefined() || 
       !((UPoint*)(theUPoint.addr))->IsDefined() )
    {
      ((UReal*)(result.addr))->SetDefined ( false );
    }
  else
    {
      ((UPoint*)(theUPoint.addr))->Distance( *((Point*)(thePoint.addr)), 
                                             *((UReal*)(result.addr)));
      ((UReal*)(result.addr))->SetDefined ( true );
    }
  return 0;
}


/*
(b1) value mapping for 

---- (uint uint) -> ureal

----

*/
int TUDistance_UInt_UInt( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  Interval<Instant> iv;
  
  Word a1, a2;
  UInt *u1, *u2;
  double c1, c2, c;
  
  result = qp->ResultStorage( s );
  
  a1 = args[0];
  a2 = args[1];
  
  u1 = (UInt*)(a1.addr);
  u2 = (UInt*)(a2.addr);

  if (!u1->IsDefined() || 
      !u2->IsDefined() || 
      !u1->timeInterval.Intersects( u2->timeInterval ) )
    { // return undefined ureal
      ((UReal*)(result.addr))->SetDefined( false );
    }
  else
    { // get intersection of deftime intervals
      u1->timeInterval.Intersection( u2->timeInterval, iv );  
      
      // calculate  result
      // (as the result is constant, no translation step is required)
      c1 = (double) u2->constValue.GetIntval();
      c2 = (double) u2->constValue.GetIntval();
      c = fabs(c1 - c2);
      *((UReal*)(result.addr)) = UReal(iv, 0, 0, c, false);
      ((UReal*)(result.addr))->SetDefined( true );
    }
  // pass on result
  return 0;
}

/*
(b2) value mapping for 

---- ((uint int) -> ureal) and ((int uint) -> ureal)

----

*/
int TUDistance_UInt_Int( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  Word  ii, ui;
  int   argConfDescriptor2;
  UInt  *u;
  CcInt *i;
  double c1, c2, c;
  
  // get argument configuration
  argConfDescriptor2 = ((CcInt*)args[2].addr)->GetIntval();
  if (argConfDescriptor2 == 0) 
    {
      ui = args[0];
      ii = args[1];
    }
  else if (argConfDescriptor2 == 1)
    {
      ui = args[1];
      ii = args[0];
    }
  else
    {
      cout << "\nWrong argument configuration in "
           << "'TUDistance_UInt_Int'. argConfDescriptor2=" 
           << argConfDescriptor2 << endl;
      return 0;
    }
  
  result = qp->ResultStorage( s );
  
  u = (UInt*)(ui.addr);
  i = (CcInt*)(ii.addr);
  
  if (!u->IsDefined() || 
      !i->IsDefined() )
    { // return undefined ureal
      ((UReal*)(result.addr))->SetDefined( false );
    }
  else
    { // calculate  result
      // (as the result is constant, no translation step is required)
      c1 = (double) u->constValue.GetIntval();
      c2 = (double) i->GetIntval();
      c = fabs(c1 - c2);
      *((UReal*)(result.addr)) = UReal(u->timeInterval, 0, 0, c, false);
      ((UReal*)(result.addr))->SetDefined( true );
    }
  // pass on result
  return 0;
}



/*
5.21.3 Specification for operator ~distance~

*/

const string TemporalSpecDistance = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(" 
  "<text>For T in {point, int}:\n"
  "(uT uT) -> ureal\n"
  "(uT  T) -> ureal\n"
  "( T uT) -> ureal</text--->"
  "<text>distance( _, _)</text--->"
  "<text>Calculates the distance of both arguments, "
  "whereof at least one is a unittype, "
  "as a 'ureal' value. </text--->"
  "<text>distance(upoint1,point1)</text--->"
  ") )";

/*
5.21.4 Selection Function of operator ~distance~

*/

ValueMapping temporalunitdistancemap[] =
  { 
    TUDistance_UPoint_UPoint,
    TUDistance_UPoint_Point,
    TUDistance_UInt_UInt,
    TUDistance_UInt_Int
  };

int temporalunitDistanceSelect( ListExpr args )
{
  ListExpr first  = nl->First(args);
  ListExpr second = nl->Second(args);
  
  if( nl->IsEqual(first, "upoint") && nl->IsEqual(second, "upoint") )
    return 0;
  
  else if( nl->IsEqual(first, "upoint") && nl->IsEqual(second, "point") )
    return 1;

  else if( nl->IsEqual(first, "point") && nl->IsEqual(second, "upoint") )
    return 1;
  
  else if( nl->IsEqual(first, "uint") && nl->IsEqual(second, "uint") )
    return 2;
  
  else if( nl->IsEqual(first, "uint") && nl->IsEqual(second, "int") )
    return 3;
  
  else if( nl->IsEqual(first, "int") && nl->IsEqual(second, "uint") )
    return 3;

  else 
    cout << "\nERROR in temporalunitDistanceSelect!" << endl;
  
  return -1;
}

/*
5.21.5 Definition of operator ~distance~

*/

Operator temporalunitdistance( "distance",
                               TemporalSpecDistance,
                               4,
                               temporalunitdistancemap,
                               temporalunitDistanceSelect,
                               TypeMapTemporalUnitDistance);

/*
5.22 Operator ~atmax~

From a given unit ~u~, the operator creates a stream of units that
are restrictions of ~u~ to the periods where it reaches its maximum
value.

----   
       For T in {int, real}
       atmax: uT --> (stream uT)

----

5.22.1 Type mapping function for ~atmax~

*/
ListExpr
UnitBaseTypeMapAtmax( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
    {
      arg1 = nl->First( args );
      
      if( nl->IsEqual( arg1, "ubool" ) )
        return nl->SymbolAtom( "ubool" );
      if( nl->IsEqual( arg1, "uint" ) )
        return nl->SymbolAtom( "uint" );
      if( nl->IsEqual( arg1, "ustring" ) )
        return nl->SymbolAtom( "ustring" );
      // for ureal, atmax/atmin will return a stream of ureals!
      if( nl->IsEqual( arg1, "ureal" ) )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "ureal" ));    
    }
  return nl->SymbolAtom( "typeerror" );
}
/*
5.22.2 Value mapping for operator ~atmax~

*/

struct AtExtrURealLocalInfo 
{
  int NoOfResults;
  int ResultsDelivered;
  UReal* t_res[3];
};

int getMaxValIndex( double& first, double& second, double& third)
{ // returns a 3-bit field indicating smallest values
  
  if ( (first > second) && (first > third) )
    return 1;
  if ( (second > first) && (second > third) )
    return 2;
  if ( (first == second) && (second > third) )
    return 3;
  if ( (third > first) && (third > second) )
    return 4;
  if ( (first == third) && (first > second) )
    return 5;
  if ( (second == third) && (second > first) )
    return 6;
  return 7; // they are all equal
}

double getValUreal(const double& t,
                   const double& a, 
                   const double& b, 
                   const double& c, 
                   const bool& r)
{
  double tmp;
  tmp = a*pow(t,2) + b*t + c;
  if (r)
    return sqrt(tmp);
  else 
    return tmp;      
}


int atmaxUReal( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  AtExtrURealLocalInfo *sli;
  UReal                *ureal;
  double  t_start, t_end, t_extr; // instants of interest
  double  v_start, v_end, v_extr; // values at resp. instants
  double  a, b, c, r, tx;
  int     maxValIndex;
  Instant t = DateTime(instanttype);
  Word    a0;
  
  result = qp->ResultStorage( s );
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) cout << "\natmaxUReal: OPEN " << endl;
      a0 = args[0];
      ureal = (UReal*)(a0.addr);
      if(TUA_DEBUG)
        cout << "  Argument ureal value: " << TUPrintUReal(ureal) << endl
             << "  1" << endl;      
      sli = new AtExtrURealLocalInfo;
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local = SetWord(sli);
      if(TUA_DEBUG) cout << "  2" << endl;
      
      if ( !(ureal->IsDefined()) )
        { // ureal undefined
          // -> return empty stream
          if(TUA_DEBUG) cout << "  2.1: ureal undefined" << endl;
          sli->NoOfResults = 0;
          if(TUA_DEBUG) 
            cout << "atmaxUReal: OPEN  finished (1)" << endl;
          return 0;
        }
      if(TUA_DEBUG) cout << "  3" << endl;
      
      if ( (ureal->timeInterval.start).ToDouble() == 
           (ureal->timeInterval.end).ToDouble() )
        { // ureal contains only a single point.
          // -> return a copy of the ureal        
          if(TUA_DEBUG) 
            cout << "  3.1: ureal contains only a single point" << endl;
          sli->t_res[sli->NoOfResults] = (UReal*) (ureal->Copy());
          if(TUA_DEBUG) 
            cout << "       res=" 
                 << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                 << endl;
          sli->NoOfResults++;
          if(TUA_DEBUG) 
            cout << "atmaxUReal: OPEN  finished (2)" << endl;
          return 0;
        }
      if(TUA_DEBUG) cout << "  4" << endl;
      
      if (ureal->a == 0)
        { 
          if ( ureal->b == 0 )
            { //  constant function
              // the only result is a copy of the argument ureal
              if(TUA_DEBUG) 
                cout << "  4.1: constant function" << endl;
              sli->t_res[sli->NoOfResults] = (UReal*) (ureal->Copy());
              // no translation needed for constant ureal!
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              if(TUA_DEBUG) 
                cout << "atmaxUReal: OPEN  finished (3)" << endl;
              return 0;
            }
          if ( ureal->b < 0 )
            { // linear fuction
              // the result is a clone of the argument, restricted to
              // its starting instant
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              if(TUA_DEBUG) 
                cout << "  4.2: linear function/initial" << endl;
              sli->t_res[sli->NoOfResults]->timeInterval.end =
                sli->t_res[sli->NoOfResults]->timeInterval.start;
              sli->t_res[sli->NoOfResults]->timeInterval.rc = true;
              // no translation needed, since remaining starting instant
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              if(TUA_DEBUG) 
                cout << "atmaxUReal: OPEN  finished (4)" << endl;
              return 0;
            }
          if ( ureal->b > 0 )
            { // linear fuction
              // the result is a clone of the argument, restricted to
              // its final instant
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              if(TUA_DEBUG) 
                cout << "  4.3: linear function/final" << endl;
              sli->t_res[sli->NoOfResults]->timeInterval.start =
                sli->t_res[sli->NoOfResults]->timeInterval.end;
              sli->t_res[sli->NoOfResults]->timeInterval.lc = true;            
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() -
                   ureal->timeInterval.end.ToDouble();
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              if(TUA_DEBUG) 
                cout << "atmaxUReal: OPEN  finished (5)" << endl;
              return 0;
            }
        }
      if(TUA_DEBUG) cout << "  5" << endl;
      
      if (ureal->a != 0) 
        { // quadratic function
          // we have to additionally check for the extremum 
          if(TUA_DEBUG) cout << "  5.1: quadratic function" << endl;
          
          // get the times of interest
          a = ureal->a;
          b = ureal->b;
          c = ureal->c;
          r = ureal->r;
          t_start = (ureal->timeInterval.start).ToDouble();
          t_extr  = -b/(2*a); 
          t_end   = (ureal->timeInterval.end).ToDouble();
          // get the values of interest
          v_start = getValUreal(t_start,a,b,c,r);
          v_extr  = getValUreal(t_extr, a,b,c,r);
          v_end   = getValUreal(t_end,  a,b,c,r);
          if(TUA_DEBUG) 
            cout << "  5.2" << endl 
                 << "\tt_start=" << t_start << "\t v_start=" << v_start << endl
                 << "\tt_extr =" << t_extr  << "\t v_extr =" << v_extr  << endl
                 << "\tt_end  =" << t_end   << "\t v_end  =" << v_end   << endl;
          
          // compute, which values are maximal
          if ( (t_start < t_extr) && (t_end   > t_extr) )
            { // check all 3 candidates
              if(TUA_DEBUG) cout << "  5.3: check all 3 candidates" << endl;
              maxValIndex = getMaxValIndex(v_extr,v_start,v_end);
              if(TUA_DEBUG) 
                cout << "  5.3  maxValIndex=" << maxValIndex << endl;
            }
          else 
            { // extremum not within interval --> possibly 2 results
              if(TUA_DEBUG) 
                cout << "  5.4: extremum not in interv (2 candidates)" << endl;
              maxValIndex = 0;
              if (v_start >= v_end) // max at t_start
                maxValIndex += 2;
              if (v_end >= v_start) // max at t_end
                maxValIndex += 4;
              if(TUA_DEBUG) 
                cout << "  5.4  maxValIndex=" << maxValIndex << endl;
            }
          if(TUA_DEBUG) cout << "  5.5" << endl;
          if (maxValIndex & 2)
            { // start value
              if(TUA_DEBUG) cout << "  5.6: added start value" << endl;
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t = ureal->timeInterval.start;
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // no translation required
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          if ( ( maxValIndex & 4 ) && ( t_end != t_start ) )
            { // end value
              if(TUA_DEBUG) cout << "  5.7: added end value" << endl;
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t = ureal->timeInterval.end;
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() - t.ToDouble();
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          
          if ( (maxValIndex & 1)   && 
               (t_extr != t_start) && 
               (t_extr != t_end)      )     
            {
              if(TUA_DEBUG) cout << "  5.8: added extremum" << endl;
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t.ReadFrom(t_extr);
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() - t_extr;
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          if(TUA_DEBUG) 
            cout << "atmaxUReal: OPEN  finished (6)" << endl;        
          return 0;
        }
      if(TUA_DEBUG) cout << "  6" << endl;
      cout << "\natmaxUReal (OPEN): This should not happen!" << endl;
      if(TUA_DEBUG) cout << "atmaxUReal: OPEN  finished (7)" << endl;
      return 0;
      
    case REQUEST :
      
      if(TUA_DEBUG) cout << "\natmaxUReal: REQUEST" << endl;
      if (local.addr == 0)
        {
          if(TUA_DEBUG) 
            cout << "atmaxUReal: REQUEST CANCEL(1)" << endl;
          return CANCEL;
        }
      sli = (AtExtrURealLocalInfo*) local.addr;
      if(TUA_DEBUG) 
        cout << " 1" << endl;
      if (sli->NoOfResults <= sli->ResultsDelivered)
        {
          if(TUA_DEBUG) 
            cout << "atmaxUReal: REQUEST CANCEL (2)" << endl;
          return CANCEL;
        }
      if(TUA_DEBUG) 
        cout << " 2" << endl;
      result = SetWord( sli->t_res[sli->ResultsDelivered]->Copy() );
      if(TUA_DEBUG) 
        cout << " 3" << endl;
      sli->t_res[sli->ResultsDelivered]->DeleteIfAllowed();
      if(TUA_DEBUG) 
        cout << " 4: delivered result[" << sli->ResultsDelivered+1 
             << "/" << sli->NoOfResults<< "]=" 
             << TUPrintUReal((UReal*)(result.addr)) 
             << endl;
      sli->ResultsDelivered++;
      if(TUA_DEBUG) 
        cout << "atmaxUReal: REQUEST YIELD" << endl;
      return YIELD;
      
    case CLOSE :
      
      if(TUA_DEBUG) cout << "\natmaxUReal: CLOSE" << endl;
      if (local.addr != 0)
        {
          sli = (AtExtrURealLocalInfo*) local.addr;
          while (sli->NoOfResults > sli->ResultsDelivered)
            {
              sli->t_res[sli->ResultsDelivered]->DeleteIfAllowed();
              sli->ResultsDelivered++;
            }
          delete sli;
        }
      if(TUA_DEBUG) cout << "atmaxUReal: CLOSE finished" << endl;
      return 0;
      
    } // end switch
  cout << "\natmaxUReal (UNKNOWN COMMAND): This should not happen!" << endl;
  return 0;   // should not be reached
}

template<class T>
int atmaxUConst( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  // This operator is not very interesting. It implements
  // the atmax operator for constant unit types, like uint, ustring or ubool.
  // In fact, it returns just a copy of the argument.
  
  result = SetWord(((ConstTemporalUnit<T>*)(args[0].addr))->Clone());
  return 0;
}

/*
5.22.3 Specification for operator ~atmax~

*/

const string TemporalSpecAtmax = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(" 
  "<text>For T in {int, bool, string}:\n"
  "uT    -> uT\n"
  "ureal -> (stream ureal)</text--->"
  "<text>atmax( _ )</text--->"
  "<text>Restricts a the unittype value to the time where "
  "it takes it's maximum value.\n"
  "Observe, that for type 'ureal', the result is a '(stream ureal)' "
  "rather than a 'ureal'!</text--->"
  "<text>atmax( ureal1 )</text--->"
  ") )";


/*
5.22.4 Selection Function of operator ~atmax~

*/

ValueMapping temporalunitatmaxmap[] = 
  {
    atmaxUConst<CcBool>,
    atmaxUConst<CcInt>,
    atmaxUConst<CcString>,
    atmaxUReal
  };

int temporalunitAtmaxSelect( ListExpr args )
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
    {
      arg1 = nl->First( args );
      
      if( nl->IsEqual( arg1, "ubool" ) )
        return 0;
      if( nl->IsEqual( arg1, "uint" ) )
        return 1;
      if( nl->IsEqual( arg1, "ustring" ) )
        return 2;
      if( nl->IsEqual( arg1, "ureal" ) )
        return 3;    
    }
  cout << "\ntemporalunitAtmaxSelect: Wrong type!" << endl;
  return -1;
}

/*
5.22.5 Definition of operator ~atmax~

*/

Operator temporalunitatmax( "atmax",
                            TemporalSpecAtmax,
                            4,
                            temporalunitatmaxmap,
                            temporalunitAtmaxSelect,
                            UnitBaseTypeMapAtmax);


/*
5.23 Operator ~atmin~

From a given unit ~u~, the operator creates a stream of units that
are restrictions of ~u~ to the periods where it reaches its minimum
value.

----   
       For T in {int, real}
       atmin: uT --> (stream uT)

----

5.23.1 Type mapping function for ~atmin~

Uses typemapping ~UnitBaseTypeMapAtmax~ intended for related operator ~atmax~

*/

/*
5.23.2 Value mapping for operator ~atmin~

*/

int getMinValIndex( double& first, double& second, double& third)
{ // returns a 3-bit field indicating smallest values
  
  if ( (first < second) && (first < third) )
    return 1;
  if ( (second < first) && (second < third) )
    return 2;
  if ( (first == second) && (second < third) )
    return 3;
  if ( (third < first) && (third < second) )
    return 4;
  if ( (first == third) && (first < second) )
    return 5;
  if ( (second == third) && (second < first) )
    return 6;
  return 7; // they are all equal
}

int atminUReal( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  AtExtrURealLocalInfo *sli;
  UReal                *ureal;
  double  t_start, t_end, t_extr; // instants of interest
  double  v_start, v_end, v_extr; // values at resp. instants
  double  a, b, c, r, tx;
  int     minValIndex;
  Instant t = DateTime(instanttype);
  Word    a0;
  
  result = qp->ResultStorage( s );
  
  switch (message)
    {
    case OPEN :
      
      a0 = args[0];
      ureal = (UReal*)(a0.addr);
      if(TUA_DEBUG)
        cout << "  Argument ureal value: " << TUPrintUReal(ureal) << endl
             << "  1" << endl;      
      
      sli = new AtExtrURealLocalInfo;
      sli->NoOfResults = 0;
      sli->ResultsDelivered = 0;
      local = SetWord(sli);
      
      if ( !ureal->IsDefined() )
        { // ureal undefined
          // -> return empty stream
          sli->NoOfResults = 0;
          if(TUA_DEBUG) cout << "       ureal undef: no solution" << endl;
          return 0;
        }
      
      if ( (ureal->timeInterval.start).ToDouble() == 
           (ureal->timeInterval.end).ToDouble() )
        { // ureal contains only a single point.
          // -> return a copy of the ureal        
          sli->t_res[sli->NoOfResults] = (UReal*) (ureal->Copy());
          // no translation required
          if(TUA_DEBUG) 
            cout << "       single point" << endl
                 << "       res " << sli->NoOfResults+1 << "=" 
                 << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                 << endl;
          sli->NoOfResults++;
          return 0;
        }
      
      if (ureal->a == 0)
        { 
          if ( ureal->b == 0 )
            { //  constant function
              // the only result is a copy of the argument ureal
              sli->t_res[sli->NoOfResults] = (UReal*) (ureal->Copy());
              // no translation required
              if(TUA_DEBUG) 
                cout << "       constant function" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              return 0;
            }
          if ( ureal->b < 0 )
            { // linear fuction
              // the result is a clone of the argument, restricted to
              // its ending instant
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              sli->t_res[sli->NoOfResults]->timeInterval.end =
                sli->t_res[sli->NoOfResults]->timeInterval.start;
              sli->t_res[sli->NoOfResults]->timeInterval.lc = true;
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() -
                   ureal->timeInterval.end.ToDouble();
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       linear function: final" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              return 0;
            }
          if ( ureal->b > 0 )
            { // linear fuction
              // the result is a clone of the argument, restricted to
              // its starting instant
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              sli->t_res[sli->NoOfResults]->timeInterval.start =
                sli->t_res[sli->NoOfResults]->timeInterval.end;
              sli->t_res[sli->NoOfResults]->timeInterval.rc = true;
              // no translation required
              if(TUA_DEBUG) 
                cout << "       linear function: initial" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
              return 0;
            }
        }
      
      if (ureal->a !=0) 
        { // quadratic function
          // we have to additionally ckeck for the extremum 
          
          // get the times of interest
          a = ureal->a;
          b = ureal->b;
          c = ureal->c;
          r = ureal->r;
          t_extr  = -b/(2*a); 
          t_start = (ureal->timeInterval.start).ToDouble();
          t_end   = (ureal->timeInterval.end).ToDouble();
          // get the values of interest
          v_extr  = getValUreal(t_extr, a,b,c,r);
          v_start = getValUreal(t_start,a,b,c,r);
          v_end   = getValUreal(t_end,  a,b,c,r);
          if(TUA_DEBUG) 
            cout << "\nQuadratic function. Cadidates are: " << endl 
                 << "\tt_start=" << t_start << "\t v_start=" << v_start << endl
                 << "\tt_extr =" << t_extr  << "\t v_extr =" << v_extr  << endl
                 << "\tt_end  =" << t_end   << "\t v_end  =" << v_end   << endl;
          // compute, which values are minimal
          
          if ( (t_start < t_extr) && (t_end > t_extr) )
            // 3 possible results
            minValIndex = getMinValIndex(v_extr,v_start,v_end);
          else 
            { // only 2 possible results
              minValIndex = 0;
              if (v_start <= v_end) // min at start
                minValIndex += 2; 
              if (v_end <= v_start) // min at end
                minValIndex += 4; 
            }
          if(TUA_DEBUG) cout << "\tminValIndex = " <<  minValIndex << endl;
          
          if (minValIndex & 2)
            {
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t = ureal->timeInterval.start;
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // no translation required
              if(TUA_DEBUG) 
                cout << "       added start" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          if ( (minValIndex & 4) && (t_start != t_end) )
            {
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t = ureal->timeInterval.end;
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() -
                   ureal->timeInterval.end.ToDouble();
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       added end" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          if ( (minValIndex & 1)   &&
               (t_extr != t_start) &&
               (t_extr != t_end)      )
            {
              sli->t_res[sli->NoOfResults] = ureal->Clone();
              t.ReadFrom(t_extr);
              Interval<Instant> i( t, t, true, true );
              sli->t_res[sli->NoOfResults]->timeInterval = i;
              // translate result to new starting instant!
              tx = ureal->timeInterval.start.ToDouble() - t_extr;
              (sli->t_res[sli->NoOfResults])->TranslateParab(tx, 0.0);
              if(TUA_DEBUG) 
                cout << "       added extr" << endl
                     << "       res " << sli->NoOfResults+1 << "=" 
                     << TUPrintUReal(sli->t_res[sli->NoOfResults]) 
                     << endl;
              sli->NoOfResults++;
            }
          return 0;
        }
      cout << "\natminUReal (OPEN): This should not happen!" << endl;
      return 0;
      
    case REQUEST :
      
      if (local.addr == 0)
        return CANCEL;
      sli = (AtExtrURealLocalInfo*) local.addr;
      
      if (sli->NoOfResults <= sli->ResultsDelivered)
        return CANCEL;
      
      result = SetWord( sli->t_res[sli->ResultsDelivered]->Clone() );
      sli->t_res[sli->ResultsDelivered]->DeleteIfAllowed();
      if(TUA_DEBUG) 
        cout << "    delivered result[" << sli->ResultsDelivered+1 
             << "/" << sli->NoOfResults<< "]=" 
             << TUPrintUReal((UReal*)(result.addr)) 
             << endl;
      sli->ResultsDelivered++;
      return YIELD;
      
    case CLOSE :
      
      if (local.addr != 0)
        {
          sli = (AtExtrURealLocalInfo*) local.addr;
          while (sli->NoOfResults > sli->ResultsDelivered)
            {
              sli->t_res[sli->ResultsDelivered]->DeleteIfAllowed();
              sli->ResultsDelivered++;
            }
          delete sli;
        }
      return 0;
      
    } // end switch
  return 0;   // should not be reached
}

template<class T>
int atminUConst( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  // This operator is not very interesting. It implements
  // the atmin operator for constant unit types, like uint, ustring or ubool.
  // In fact, it returns just a copy of the argument.
  
  result = SetWord(((ConstTemporalUnit<T>*)(args[0].addr))->Clone());
  return 0;
}

/*
5.23.3 Specification for operator ~atmin~

*/

const string TemporalSpecAtmin = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(" 
  "<text>For T in {int, bool, string}:\n"
  "uT    -> uT\n"
  "ureal -> (stream ureal)</text--->"
  "<text>atmin( _ )</text--->"
  "<text>Restricts a the unittype value to the time where "
  "it takes it's minimum value.\n"
  "Observe, that for type 'ureal', the result is a '(stream ureal)' "
  "rather than a 'ureal'!</text--->"
  "<text>atmin( ureal1 )</text--->"
  ") )";


/*
5.23.4 Selection Function of operator ~atmin~

Uses selection function ~temporalunitAtmaxSelect~.

*/

ValueMapping temporalunitatminmap[] = 
  {
    atminUConst<CcBool>,
    atminUConst<CcInt>,
    atminUConst<CcString>,
    atminUReal
  };

/*
  5.23.5 Definition of operator ~atmin~
  
*/
Operator temporalunitatmin( "atmin",
                            TemporalSpecAtmin,
                            4,
                            temporalunitatminmap,
                            temporalunitAtmaxSelect,
                            UnitBaseTypeMapAtmax);


/*
5.24 Operator ~saggregate~

Stream aggregation operator

This operator applies an aggregation function (which must be binary, 
associative and commutative) to a stream of data using a given neutral (initial) 
value (which is also returned if the stream is empty). If the stream contains 
only one single element, this element is returned as the result. 
The result a single value of the same kind.

----   
       For T in kind DATA:
       saggregate: ((stream T) x (T x T --> T) x T) --> T

----

The first argument is the input stream.
The second argument is the function used in the aggregation.
The third value is used to initialize the mapping (for the first elem) 
and will also be return if the input stream is empty.

5.24.1 Type mapping function for ~saggregate~

*/
ListExpr TemporalUnitSaggregateTypeMap( ListExpr args )
{
  string outstr1, outstr2;
  ListExpr TypeT;
  
  // check for correct length
  if (nl->ListLength(args) != 3)
    {
      ErrorReporter::ReportError("Operator saggregate expects a list of length "
                                 "three.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // get single arguments
  ListExpr instream   = nl->First(args),
    map        = nl->Second(args),
    zerovalue  = nl->Third(args),
    errorInfo  = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  // check for first arg to be atomic and of kind DATA
  if ( nl->IsAtom(instream) || 
       ( nl->ListLength( instream ) != 2) ||
       !(TypeOfRelAlgSymbol(nl->First(instream) == stream )) ||
       !am->CheckKind("DATA", nl->Second(instream), errorInfo) )
    {
      ErrorReporter::ReportError("Operator saggregate expects a list of length "
                                 "two as first argument, having structure "
                                 "'(stream T)', for T in kind DATA.");
      return nl->SymbolAtom( "typeerror" );
    }
  else 
    TypeT = nl->Second(instream);
  
  // check for second to be of length 4, (map T T T)
  // T of same type as first
  if ( nl->IsAtom(map) ||
       !(nl->ListLength(map) == 4) ||
       !( nl->IsEqual(nl->First(map), "map") ) ||
       !( nl->Equal(nl->Fourth(map), nl->Second(map)) ) ||
       !( nl->Equal(nl->Third(map), nl->Second(map)) ) ||
       !( nl->Equal(nl->Third(map), TypeT) ) )
    {
      ErrorReporter::ReportError("Operator saggregate expects a list of length "
                                 "four as second argument, having structure "
                                 "'(map T T T)', where T has the base type of "
                                 "the first argument.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // check for third to be atomic and of the same type T 
  if ( !nl->IsAtom(zerovalue) ||
       !nl->Equal(TypeT, zerovalue) )
    {
      ErrorReporter::ReportError("Operator saggregate expects a list of length"
                                 "one as third argument (neutral elem), having "
                                 "structure 'T', where T is also the type of "
                                 "the mapping's arguments and result. Also, "
                                 "T must be of kind DATA.");
      return nl->SymbolAtom( "typeerror" );
    }
  
  // return T as the result type.
  return TypeT;
}

/*
5.24.2 Value mapping for operator ~saggregate~

*/

struct AggregStruct
{
  inline AggregStruct( long level, Word value ):
    level( level ), value( value )
  {}
  
  inline AggregStruct( const AggregStruct& a ):
    level( a.level ), value( a.value )
  {}
  
  inline AggregStruct& operator=( const AggregStruct& a )
  { level = a.level; value = a.value; return *this; }
  
  long level;
  Word value;
  // if the level is 0 then value contains an element pointer, 
  // otherwise it contains a previous result of the aggregate operator
};

int Saggregate( Word* args, Word& result, int message,
                Word& local, Supplier s )
{
  // The argument vector contains the following values:
  Word 
    stream  = args[0], // stream of elements T
    aggrmap = args[1], // mapping function T x T --> T
    nval    = args[2]; // zero value/neutral element T
  
  Word t1, t2, iterWord, resultWord;
  ArgVectorPointer vector = qp->Argument(aggrmap.addr);
  
  qp->Open(stream.addr);
  result = qp->ResultStorage(s);
  
  // read the first tuple
  qp->Request( stream.addr, t1 );
  if( !qp->Received( stream.addr ) )
    { // Case 1: empty stream
      result.addr = ((Attribute*) nval.addr)->Copy();
    }
  else
    {
      stack<AggregStruct> aggrStack;
      
      // read the second tuple
      qp->Request( stream.addr, t2 );
      if( !qp->Received( stream.addr ) )
        { // Case 2: only one single elem in stream
          result.addr = ((Attribute*)t1.addr)->Copy();
        }
      else
        { // there are at least two stream elements
          // match both elements and put the result into the stack
          (*vector)[0] = SetWord(t1.addr);
          (*vector)[1] = SetWord(t2.addr);
          qp->Request( aggrmap.addr, resultWord );
          aggrStack.push( AggregStruct( 1, resultWord ) ); 
          // level 1 because we matched a level 0 elem
          qp->ReInitResultStorage( aggrmap.addr );
          ((Attribute*)t1.addr)->DeleteIfAllowed();
          ((Attribute*)t2.addr)->DeleteIfAllowed();
          
          // process the rest of the stream
          qp->Request( stream.addr, t1 );
          while( qp->Received( stream.addr ) )
            {
              long level = 0;
              iterWord = SetWord( ((Attribute*)t1.addr)->Copy() );
              while( !aggrStack.empty() && aggrStack.top().level == level )
                {
                  (*vector)[0] = aggrStack.top().value;
                  (*vector)[1] = iterWord;
                  qp->Request(aggrmap.addr, resultWord);
                  ((Attribute*)iterWord.addr)->DeleteIfAllowed();
                  iterWord = resultWord;
                  qp->ReInitResultStorage( aggrmap.addr );
                  if( aggrStack.top().level == 0 )
                    ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
                  else
                    delete (Attribute*)aggrStack.top().value.addr;
                  aggrStack.pop();
                  level++;
                }
              if( level == 0 )
                aggrStack.push( AggregStruct( level, t1 ) );
              else
                {
                  aggrStack.push( AggregStruct( level, iterWord ) );
                  ((Attribute*)t1.addr)->DeleteIfAllowed();
                }
              qp->Request( stream.addr, t1 );
            }
          
          // if the stack contains only one entry, then we are done
          if( aggrStack.size() == 1 )
            {
              result.addr = ((Attribute*)aggrStack.top().value.addr)->Copy();
              ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
            }
          else
            // the stack must contain more elements and we call the 
            // aggregate function for them
            {
              iterWord = aggrStack.top().value;
              int level = aggrStack.top().level;
              aggrStack.pop();
              while( !aggrStack.empty() )
                {
                  (*vector)[0] = level == 0 ? 
                    SetWord( ((Attribute*)iterWord.addr) ) :
                    iterWord;
                  (*vector)[1] = aggrStack.top().value;
                  qp->Request( aggrmap.addr, resultWord );
                  ((Attribute*)iterWord.addr)->DeleteIfAllowed();
                  ((Attribute*)aggrStack.top().value.addr)->DeleteIfAllowed();
                  iterWord = resultWord;
                  qp->ReInitResultStorage( aggrmap.addr );
                  level++;        
                  aggrStack.pop();
                }
              result.addr = ((Attribute*)iterWord.addr)->Copy();
              ((Attribute*)iterWord.addr)->DeleteIfAllowed();
            }
        }
    }
  
  qp->Close(stream.addr);
  
  return 0;
}


/*
5.24.3 Specification for operator ~saggregate~

*/

const string TemporalUnitSaggregateSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(" 
  "<text>For T in kind DATA:\n"
  "((stream T) ((T T) -> T) T ) -> T\n</text--->"
  "<text>_ saggregate [ fun ; _ ]</text--->"
  "<text>Aggregates the values from the stream (1st arg) "
  "using a binary associative and commutative "
  "aggregation function (2nd arg), "
  "and a 'neutral value' (3rd arg, also passed as the "
  "result if the stream is empty). If the stream contains"
  "only one single element, that element will be returned"
  "as the result.</text--->"
  "<text>query intstream(1,5) saggregate[ "
  "fun(i1:int, i2:int) i1+i2 ; 0]\n"
  "query intstream(1,5) saggregate[ "
  "fun(i1:STREAMELEM, i2:STREAMELEM) ifthenelse(i1>i2,i1,i2) ; 0]</text--->"
  ") )";

/*
5.24.4 Selection Function of operator ~saggregate~

*/

ValueMapping temporalunitsaggregatemap[] = 
  {
    Saggregate
  };


int temporalunitSaggregateSelect( ListExpr args )
{
  return 0;
}

/*
5.24.5 Definition of operator ~saggregate~

*/

Operator temporalunitsaggregate( "saggregate",
                                 TemporalUnitSaggregateSpec,
                                 1,
                                 temporalunitsaggregatemap,
                                 temporalunitSaggregateSelect,
                                 TemporalUnitSaggregateTypeMap);



/*
5.25 Operator ~intersects~

*/

/*
5.25.1 Type mapping function for ~intersects~

*/

/*
5.25.2 Value mapping for operator ~intersects~

*/

/*
5.24.3 Specification for operator ~intersects~

*/

/*
5.25.4 Selection Function of operator ~intersects~

*/

/*
5.25.5 Definition of operator ~intersects~

*/


/*
5.26 Operator ~intersection~

----  

     intersection: For T in {bool, int, string}:
OK             uT x      uT --> (stream uT)
OK             uT x       T --> (stream uT)
OK              T x      uT --> (stream uT)
(OK)        ureal x    real --> (stream ureal)
(OK)         real x   ureal --> (stream ureal)
Pre         ureal x   ureal --> (stream ureal)
OK         upoint x   point --> (stream upoint) same as at: upoint x point
OK          point x  upoint --> (stream upoint) same as at: upoint x point
OK         upoint x  upoint --> (stream upoint)
OK         upoint x    line --> (stream upoint)
OK           line x  upoint --> (stream upoint)
Pre        upoint x  region --> (stream upoint)
Pre        region x  upoint --> (stream upoint)
Pre        upoint x uregion --> (stream upoint)
Pre       uregion x  upoint --> (stream upoint)

----

A. (T mT) [->] mT   and   (mT T) -> mT

B. (mT mT) [->] mT 

The operator always returns a stream of units (to handle both, empty and set 
valued results).

  1. For all types of arguments, we return an empty stream, if both timeIntervals 
don't overlap.

  2. Now, the algorithms depends on the actual dataype:

Then, for ~constant temporal units~ (ubool, uint, ustring), the operator 
restricts two unit values with equal values to the intersection of both time 
intervals, or otherwise (if the const values are nit equal) returns an empty 
stream.

For ~ureal~, we test whether the functions of both units are equal, for we
can pass its restriction to the intersection of deftimes then (resp. create an 
empty stream). Otherwise, we restrict both arguments' deftimes to their
intersection. Then we calculate the minimum and maximum value for both
arguments with respect to the restricted deftime. If both ranges don't 
overlap, we return an empty stream. Otherwise, we compute the intersection 
points (1 or 2) and return them as the result - or
an UNDEFINED value, if the result is not represeantable as a ureal value.

For ~upoint~, we interpret the units as straight lines. If both are parallel,
return the empty stream. If both are identical, 

Otherwise calculate the intersection point ~S~. From ~S~, calculate the 
instants $t_{u1}$ and $t_{u2}$. If both coincide, return that point if within 
the common timeInterval, otherwise the empty stream.

C. (upoint line) [->] (stream upoint)  and  (line upoint) [->] (stream upoint)


*/

/*
5.26.1 Type mapping function for ~intersection~

*/

ListExpr TemporalUnitIntersectionTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr1, argstr2;
  
  if( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      
      // First case: uT uT -> stream uT
      if (nl->Equal( arg1, arg2 ))
        {
          if( nl->IsEqual( arg1, "ubool" ) )
            return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                    nl->SymbolAtom( "ubool" ));    
          if( nl->IsEqual( arg1, "uint" ) )
            return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                   nl->SymbolAtom( "uint" ));    
          if( nl->IsEqual( arg1, "ureal" ) )
            return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                   nl->SymbolAtom( "ureal" ));    
          if( nl->IsEqual( arg1, "upoint" ) )
            return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                   nl->SymbolAtom( "upoint" ));  
          if( nl->IsEqual( arg1, "ustring" ) )
            return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                   nl->SymbolAtom( "ustring" ));  
        }
      
      // Second case: uT T -> stream uT
      if( nl->IsEqual( arg1, "ubool" ) && nl->IsEqual( arg2, "bool") )
        return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                nl->SymbolAtom( "ubool" ));    
      if( nl->IsEqual( arg1, "uint" ) && nl->IsEqual( arg2, "int") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "uint" ));    
      if( nl->IsEqual( arg1, "ureal" ) && nl->IsEqual( arg2, "real") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "ureal" ));    
      if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "point") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "upoint" ));  
      if( nl->IsEqual( arg1, "ustring" ) && nl->IsEqual( arg2, "string") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "ustring" ));  
      
      // Third case: T uT -> stream uT
      if( nl->IsEqual( arg1, "bool" ) && nl->IsEqual( arg2, "ubool") )
        return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                nl->SymbolAtom( "ubool" ));    
      if( nl->IsEqual( arg1, "int" ) && nl->IsEqual( arg2, "uint") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "uint" ));    
      if( nl->IsEqual( arg1, "real" ) && nl->IsEqual( arg2, "ureal") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "ureal" ));    
      if( nl->IsEqual( arg1, "point" ) && nl->IsEqual( arg2, "upoint") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "upoint" ));  
      if( nl->IsEqual( arg1, "string" ) && nl->IsEqual( arg2, "ustring") )
        return nl->TwoElemList(nl->SymbolAtom( "stream" ),
                               nl->SymbolAtom( "ustring" ));  
      
      // Fourth case: upoint line -> stream upoint
      if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "line") )
        return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                nl->SymbolAtom( "upoint" ));    
      
      // Fifth case: line upoint -> stream upoint
      if( nl->IsEqual( arg1, "line" ) && nl->IsEqual( arg2, "upoint") )
        return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
                                nl->SymbolAtom( "upoint" ));  
      
      // Sixth case: upoint uregion -> stream upoint
      //if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "uregion") )
      //  return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
      //                          nl->SymbolAtom( "upoint" ));  

      // Eighth case: uregion upoint -> stream upoint
      //if( nl->IsEqual( arg1, "uregion" ) && nl->IsEqual( arg2, "upoint") )
      //  return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
      //                          nl->SymbolAtom( "upoint" ));  

      // Ninth case: upoint region -> stream upoint
      //if( nl->IsEqual( arg1, "upoint" ) && nl->IsEqual( arg2, "region") )
      //  return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
      //                          nl->SymbolAtom( "upoint" ));  

      // Tenth case: region upoint -> stream upoint
      //if( nl->IsEqual( arg1, "region" ) && nl->IsEqual( arg2, "upoint") )
      //  return  nl->TwoElemList(nl->SymbolAtom( "stream" ),
      //                          nl->SymbolAtom( "upoint" ));  
    }
  
  // Error case:
  nl->WriteToString(argstr1, arg1); 
  nl->WriteToString(argstr2, arg2); 
  ErrorReporter::ReportError(
    "Operator intersection expects two arguments of the same type T, "
    "where T in {ubool, uint, ureal, ustring, upoint}"
    "The passed arguments have types '"+ argstr1 +"' and '"
    + argstr2 + "'.");
  return nl->SymbolAtom("typeerror");   
}

/*
5.26.2 Value mapping for operator ~intersection~

*/

struct TUIntersectionLocalInfo 
{
  bool finished;
  int  NoOfResults;
  int  NoOfResultsDelivered;
  Word resultValues[2];       // Used if at most 2 results can occur
  vector<Word> resultValues2; // Used if more than 2 results may occur
  MPoint *mpoint;             // Used for upoint x lines
};

// value mapping for constant units (uT uT) -> (stream uT)
template<class T>
int temporalUnitIntersection_CU_CU( Word* args, Word& result, int message,
                                    Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word u1, u2;
  T    *uv1, *uv2;
  Interval<Instant> iv;
  
  // test for overlapping intervals
  switch( message )
    {
    case OPEN:
      
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_CU_CU: received OPEN" << endl;
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      
      u1 = args[0];
      u2 = args[1];
      
      uv1 = (T*) (u1.addr);
      uv2 = (T*) (u2.addr);
      
      if ( uv1->IsDefined() && 
           uv2->IsDefined() && 
           uv1->timeInterval.Intersects( uv2->timeInterval ) &&
           uv1->EqualValue(*uv2) )         
        { // get intersection of deftime intervals
          uv1->timeInterval.Intersection( uv2->timeInterval, iv );  
          if (TUA_DEBUG) cout << "  iv=" << TUPrintTimeInterval( iv ) << endl;
          // store result
          sli->resultValues[sli->NoOfResults] = SetWord( uv1->Clone() );
          ((T*)(sli->resultValues[sli->NoOfResults].addr))->timeInterval = iv;
          sli->NoOfResults++;
          sli->finished = false;
          if (TUA_DEBUG) 
            cout << "  added result" << endl;
        }// else: no result
      local = SetWord(sli);
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_CU_CU: finished OPEN" << endl;
      
      return 0;
      
    case REQUEST:
      
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_CU_CU: received REQUEST" << endl;
      if(local.addr == 0)
        {
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_CU_CU: CANCEL (1)" << endl;
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_CU_CU: CANCEL (2)" << endl;
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result = SetWord( ((T*)
            (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((T*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_CU_CU: YIELD" << endl;
          return YIELD;
        }
      sli->finished = true;
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_CU_CU: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          while(sli->NoOfResultsDelivered < sli->NoOfResults)
            {
              ((T*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
                ->DeleteIfAllowed();
              sli->NoOfResultsDelivered++;
            }
          delete sli;
        }
      return 0;
    } // end switch
  
  return 0;
}

// value mapping for constant units (uT  T) -> (stream uT)
//                            and   ( T uT) -> (stream uT)
template<class UT, class T, int uargindex>
int temporalUnitIntersection_CU_C( Word* args, Word& result, int message,
                                   Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word u1, u2;
  UT    *uv1;
  T     *uv2;
  Interval<Instant> iv;
  
  // test for overlapping intervals
  switch( message )
    {
    case OPEN:
      
      if (TUA_DEBUG)
        cout << "temporalUnitIntersection_CU_C: received OPEN" << endl;      
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      
      // get arguments, such that u1 is the unit type
      //                and u2 is the simple type
      if (uargindex == 0)
        { u1 = args[0]; u2 = args[1]; }
      else
        { u1 = args[1]; u2 = args[0];}
      
      if (TUA_DEBUG)
        cout << "  uargindex =" << uargindex << endl;
      uv1 = (UT*) (u1.addr);
      uv2 = (T*) (u2.addr);
      
      if ( uv1->IsDefined() && 
           uv2->IsDefined() && 
           (uv1->constValue.Compare( uv2 ) == 0 ) )        
        { // store result
          sli->resultValues[sli->NoOfResults] = SetWord( uv1->Clone() );
          sli->NoOfResults++; 
          sli->finished = false;
          if (TUA_DEBUG) cout << "  Added Result" << endl;
        }// else: no result
      local = SetWord(sli);
      if (TUA_DEBUG)
        cout << "temporalUnitIntersection_CU_C: finished OPEN" << endl;
      return 0;
      
    case REQUEST:
      
      if (TUA_DEBUG)
        cout << "temporalUnitIntersection_CU_C: received REQUEST" << endl;      
      if(local.addr == 0)
        {
          if (TUA_DEBUG)            
            cout << "temporalUnitIntersection_CU_C: finished REQUEST: " 
                 << "CANCEL (1)" << endl;      
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
          if (TUA_DEBUG)            
            cout << "temporalUnitIntersection_CU_C: finished REQUEST: " 
                 << "CANCEL (2)" << endl;      
          return CANCEL;
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result = SetWord( ((UT*)
            (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );   
          ((UT*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
          if (TUA_DEBUG)            
            cout << "temporalUnitIntersection_CU_C: finished REQUEST: " 
                 << "YIELD" << endl;      
          return YIELD;
        }
      sli->finished = true;
      if (TUA_DEBUG)
        cout << "temporalUnitIntersection_CU_C: finished REQUEST: " 
             << "CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          while(sli->NoOfResultsDelivered < sli->NoOfResults)
            {
              ((UT*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
                ->DeleteIfAllowed();
              sli->NoOfResultsDelivered++;
            }
          delete sli;
        }
      return 0;
    } // end switch
  
  return 0;
  
}


/* 

Value mapping for ureal

We will calculate the intersection points (time instants) of both ureal 
functions. Therefore, we need to distinguish 2 different cases:

 1. $ureal_1.r = ureal_2.r$

  Then, we have to solve the equation
 \[ (a_1-a_2)t^2 + (b_1-b_2)t + (c_1-c_2) = 0 \]
With $a\neq 0$, this computes to 
\[ t_{1,2} = \frac{-(b_1-b_2) \pm \sqrt{(b_1-b_2)^2 - 4(a_1-a_2)(c_1-c_2)}}
{2(a_1-a_2)} \]

And depending on the value of $D = (b_1-b_2)^2 - 4(a_1-a_2)(c_1-c_2)$, we will 
have 0 ($D<0$), 1 ($D=0$) or 2 ($D>0$) solutions.


 2. $ureal_1.r \neq ureal2_.r$

Then we have to solve the equation
\[ {a_1}^2 t^4 + 2 a_1 b_1 t^3 + (2 a_1 c_1 + {b_1}^2 - a_2) 
t^2 + (2 b_1 c_1 - b_2) t - c_2 = 0 \]

  where $x_1$ stands for parameters from the 
~ureal~ value that does have $r = false$, $x_2$ for the parameters of the 
~ureal~ value with $r = true$.

*/

int temporalUnitIntersection_ureal_ureal( Word* args, Word& result, int message,
                                          Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitIntersection_ureal_ureal "
       << "not yet implemented!" << endl;  
  return 0;
}
               
               
// value mapping for constant units (uT  T) -> (stream uT)
//                            and   ( T uT) -> (stream uT)
// The method is almost identical to that used for operator  
//                            at: (ureal real) -> (stream ureal)
template<int uargindex>
int temporalUnitIntersection_ureal_real( Word* args, Word& result, int message,
                                         Word& local, Supplier s )
{
  MappingUnitAt_rLocalInfo *localinfo;
  double radicand, a, b, c, r, y, tx;
  DateTime t1 = DateTime(instanttype);
  DateTime t2 = DateTime(instanttype);
  Interval<Instant> rdeftime, deftime;
  UReal *uinput;
  CcReal *value;
  Word a0, a1;
  
  switch (message)
    {
    case OPEN :
      
      if(TUA_DEBUG) 
        cout << "\ntemporalUnitIntersection_ureal_real: OPEN" << endl;
      localinfo = new MappingUnitAt_rLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      
      // initialize arguments, such that a0 always contains the ureal
      //                       and a1 the real 
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else
        { a0 = args[1]; a1 = args[0]; }
      
      localinfo = new MappingUnitAt_rLocalInfo;
      localinfo->finished = true;
      localinfo->NoOfResults = 0;
      if(TUA_DEBUG) cout << "  1" << endl;

      uinput = (UReal*)(a0.addr);
      if(TUA_DEBUG) cout << "  1.1" << endl;

      value = (CcReal*)(a1.addr);
      if(TUA_DEBUG) cout << "  1.2" << endl;

      if(TUA_DEBUG) cout << "  2" << endl;

      if(TUA_DEBUG) cout << "  2.1: " << uinput->IsDefined() << endl;
      if(TUA_DEBUG) cout << "  2.2: " << value->IsDefined() << endl;

      if ( !uinput->IsDefined() ||
           !value->IsDefined() )
        { // some input is undefined -> return empty stream
          if(TUA_DEBUG) cout << "  3: Some input is undefined. No result." 
                             << endl;
          localinfo->NoOfResults = 0;
          localinfo->finished = true;
          local = SetWord(localinfo);
          if(TUA_DEBUG) 
            cout << "\ntemporalUnitIntersection_ureal_real: finished OPEN (1)" 
                 << endl;
          return 0;
        }
      if(TUA_DEBUG) cout << "  4" << endl;

      y = value->GetRealval();
      a = uinput->a;
      b = uinput->b;
      c = uinput->c;
      r = uinput->r;
      deftime = uinput->timeInterval;

      if(TUA_DEBUG) 
        {cout << "    The UReal is" << " a= " << a << " b= " 
              << b << " c= " << c << " r= " << r << endl;
          cout << "    The Real is y=" << y << endl;
          cout << "  5" << endl;
        }
            
      if ( (a == 0) && (b == 0) )
        { // constant function. Possibly return input unit
          if(TUA_DEBUG) cout << "  6: 1st arg is a constant value" << endl;
          if (c != y)
            { // There will be no result, just an empty stream
              if(TUA_DEBUG) cout << "  7" << endl;
              localinfo->NoOfResults = 0;
              localinfo->finished = true;
            }
          else
                { // Return the complete unit
                  if(TUA_DEBUG) 
                    {
                      cout << "  8: Found constant solution" << endl;
                      cout << "    T1=" << c << endl;
                      cout << "    Tstart=" << deftime.start.ToDouble() << endl;
                      cout << "    Tend  =" << deftime.end.ToDouble() << endl;
                    }
                  localinfo->runits[localinfo->NoOfResults].addr
                    = uinput->Copy();
                  // no translation required
                  localinfo->NoOfResults++;
                  localinfo->finished = false;
                  if(TUA_DEBUG) cout << "  9" << endl;
                }
          if(TUA_DEBUG) cout << "  10" << endl;
          local = SetWord(localinfo);
          if(TUA_DEBUG) 
            cout << "\ntemporalUnitIntersection_ureal_real: finished OPEN (2)" 
                 << endl;
          return 0;
        }
      if ( (a == 0) && (b != 0) )
        { // linear function. Possibly return input unit restricted 
          // to single value
          if(TUA_DEBUG) cout << "  11: 1st arg is a linear function" << endl;
          double T1 = (y - c)/b;
          if(TUA_DEBUG) 
            {
              cout << "    T1=" << T1 << endl;    
              cout << "    Tstart=" << deftime.start.ToDouble() << endl;
              cout << "    Tend  =" << deftime.end.ToDouble() << endl;    
            }
          t1.ReadFrom( T1 );
          if (deftime.Contains(t1))
            { // value is contained by deftime
              if(TUA_DEBUG) 
                cout << "  12: Found valid linear solution." << endl;
              localinfo->runits[localinfo->NoOfResults].addr = 
                uinput->Copy();
              ((UReal*)(localinfo
                        ->runits[localinfo->NoOfResults].addr))
                ->timeInterval = Interval<Instant>(t1, t1, true, true);
              // translate result to new starting instant!
              tx = uinput->timeInterval.start.ToDouble() - T1;
              ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;                
              if(TUA_DEBUG) cout << "  13" << endl;
            }
          else
            { // value is not contained by deftime -> no result
              if(TUA_DEBUG) 
                cout << "  14: Found invalid linear solution." << endl;
              localinfo->NoOfResults = 0;
              localinfo->finished = true;
              if(TUA_DEBUG) cout << "  15" << endl;
            }
          if(TUA_DEBUG) cout << "  16" << endl;
          local = SetWord(localinfo);
          cout << "\ntemporalUnitIntersection_ureal_real: finished OPEN (3)" 
               << endl;
          return 0;
        }
      
      if(TUA_DEBUG) cout << "  17" << endl;
      radicand = (b*b + 4*a*(y-c));
      if(TUA_DEBUG) cout << "    radicand =" << radicand << endl;
      if ( (a != 0) && (radicand >= 0) )
        { // quadratic function. There are possibly two result units
          // calculate the possible t-values t1, t2
          
/*
The solution to the equation $at^2 + bt + c = y$ is 
\[t_{1,2} = \frac{-b \pm \sqrt{b^2-4a(c-y)}}{2a},\] for $b^2-4a(c-y) = b^2+4a(y-c) \geq 0$.


*/
          if(TUA_DEBUG) cout << "  18: 1st arg is a quadratic function" << endl;
          double T1 = (-b + sqrt(radicand)) / (2*a);
          double T2 = (-b - sqrt(radicand)) / (2*a);
          if(TUA_DEBUG) 
            {
              cout << "    T1=" << T1 << endl;
              cout << "    T2=" << T2 << endl;
              cout << "    Tstart=" << deftime.start.ToDouble() << endl;
              cout << "    Tend  =" << deftime.end.ToDouble() << endl;    
            }
          t1.ReadFrom( T1 );
          t2.ReadFrom( T2 );
          
          // check, whether t1 contained by deftime
          if (deftime.Contains( t1 ))
            {
              if(TUA_DEBUG) 
                cout << "  19: Found first quadratic solution" << endl;
              rdeftime.start = t1;
              rdeftime.end = t1;
              rdeftime.lc = true;
              rdeftime.rc = true;
              localinfo->runits[localinfo->NoOfResults].addr = 
                new UReal( rdeftime,a,b,c,r );
              ((UReal*) (localinfo->runits[localinfo->NoOfResults].addr))
                ->SetDefined( true );
              // translate result to new starting instant!
              tx = uinput->timeInterval.start.ToDouble() - T1;
              ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;
              if(TUA_DEBUG) cout << "  20" << endl;
            }
          // check, whether t2 contained by deftime
          if ( !(t1 == t2) && (deftime.Contains( t2 )) )
            {
              if(TUA_DEBUG) 
                cout << "  21: Found second quadratic solution" << endl;
              rdeftime.start = t2;
              rdeftime.end = t2;
              rdeftime.lc = true;
              rdeftime.rc = true;
              localinfo->runits[localinfo->NoOfResults].addr = 
                new UReal( rdeftime,a,b,c,r );
              ((UReal*) (localinfo->runits[localinfo->NoOfResults].addr))
                ->SetDefined (true );
              // translate result to new starting instant!
              tx = uinput->timeInterval.start.ToDouble() - T2;
              ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
                ->TranslateParab(tx, 0.0);
              localinfo->NoOfResults++;
              localinfo->finished = false;
              if(TUA_DEBUG) cout << "  22" << endl;
            }
        }
      else // negative discreminant -> there is no real solution 
           //                          and no result unit
        {
          if(TUA_DEBUG) cout << "  23: No real-valued solution" << endl;
          localinfo->NoOfResults = 0;
          localinfo->finished = true;
          if(TUA_DEBUG) cout << "  24" << endl;
        }
      if(TUA_DEBUG) cout << "  25" << endl;
      local = SetWord(localinfo);
      if(TUA_DEBUG) 
        cout << "\ntemporalUnitIntersection_ureal_real: finished OPEN (4)" 
             << endl;
      return 0;
      
    case REQUEST :
      
      if(TUA_DEBUG) cout << "\ntemporalUnitIntersection_ureal_real: REQUEST" 
                         << endl;
      if (local.addr == 0)
        {
          cout << "\ntemporalUnitIntersection_ureal_real: REQUEST CANCEL (1)" 
               << endl;
          return CANCEL;
        }
      localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
      if(TUA_DEBUG) cout << "\n   localinfo: finished=" << localinfo->finished 
                         << " NoOfResults==" << localinfo->NoOfResults << endl;

      if (localinfo->finished)
        {
          if(TUA_DEBUG) 
            cout << "\ntemporalUnitIntersection_ureal_real: REQUEST CANCEL (2)" 
                 << endl;
          return CANCEL;
        }
      if ( localinfo->NoOfResults <= 0 )
        { localinfo->finished = true;
          if(TUA_DEBUG) 
            cout << "\ntemporalUnitIntersection_ureal_real: REQUEST CANCEL (3)" 
                 << endl;
          return CANCEL;
        }
      localinfo->NoOfResults--;
      result = SetWord( ((UReal*)(localinfo
                                  ->runits[localinfo->NoOfResults].addr))
                        ->Clone() );
      ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
        ->DeleteIfAllowed();
      if(TUA_DEBUG) 
        cout << "\ntemporalUnitIntersection_ureal_real: REQUEST YIELD" << endl;
      return YIELD;
      
    case CLOSE :

      if(TUA_DEBUG) cout << "\ntemporalUnitIntersection_ureal_real: CLOSE" 
                         << endl;
      if (local.addr != 0)
        {
          localinfo = (MappingUnitAt_rLocalInfo*) local.addr;
          for (;localinfo->NoOfResults>0;localinfo->NoOfResults--)
            ((UReal*)(localinfo->runits[localinfo->NoOfResults].addr))
              ->DeleteIfAllowed();
          delete localinfo;
        }
      if(TUA_DEBUG) 
        cout << "\ntemporalUnitIntersection_ureal_real: finished CLOSE" << endl;
      return 0;
    } // end switch
  
      // should not be reached
  return 0;
}

/*

value mapping for 

----
     (upoint upoint) -> (stream upoint)

----

*/
int 
temporalUnitIntersection_upoint_upoint( Word* args, Word& result, int message,
                                        Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word u1, u2;
  UPoint *uv1, *uv2;
  Interval<Instant> iv;
  Instant t;
  Point p1n_start, p1n_end, p2n_start, p2n_end, p_intersect, d1, d2, p1;
  UPoint p1norm, p2norm;
  double t_x, t_y, t1, t2, dxp1, dxp2, dyp1, dyp2, dt;

  // test for overlapping intervals
  switch( message )
    {
    case OPEN:
      
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_upoint: received OPEN" << endl;
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      local = SetWord(sli);
      u1 = args[0];
      u2 = args[1];
      uv1 = (UPoint*) (u1.addr);
      uv2 = (UPoint*) (u2.addr);
      
      if ( !uv1->IsDefined() ||
           !uv2->IsDefined() ||
           !uv1->timeInterval.Intersects( uv2->timeInterval ) )
        {
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_upoint_upoint: finished OPEN (1)" 
                 << endl;
          return 0; // nothing to do
        }
      
      // get common time interval
      uv1->timeInterval.Intersection(uv2->timeInterval, iv);
      
      // normalize both starting and ending points to interval
      uv1->TemporalFunction( iv.start, p1n_start, true);
      uv1->TemporalFunction( iv.end, p1n_end, true);
      p1norm = UPoint( iv, p1n_start, p1n_end );
      
      uv2->TemporalFunction( iv.start, p2n_start, true);
      uv2->TemporalFunction( iv.end, p2n_end, true);
      p2norm = UPoint( iv, p2n_start, p2n_end );
      
      if (TUA_DEBUG) 
        {
          cout << "  uv1=" << TUPrintUPoint( *uv1 ) << endl
               << "  uv2=" << TUPrintUPoint( *uv2 ) << endl
               << "  iv=" << TUPrintTimeInterval( iv ) << endl
               << "  p1norm=" << TUPrintUPoint( p1norm ) << endl
               << "  p2norm=" << TUPrintUPoint( p2norm ) << endl;
        }
      
      // test for identity:
      if ( p1norm.EqualValue( p2norm ))
        { // both upoints have the same linear function
          sli->resultValues[sli->NoOfResults] = SetWord( p1norm.Clone() );
          sli->NoOfResults++;      
          sli->finished = false;       
          if (TUA_DEBUG) 
            cout << "  Identity test passed. Added result." << endl
                 << "temporalUnitIntersection_upoint_upoint: finished OPEN (3)" 
                 << endl;
          return 0;
        }
      if (TUA_DEBUG) 
        cout << "  Identity test failed." << endl;
      
      // test for ordinary intersection of the normalized upoints
      d1 = p2n_start - p1n_start;
      d2 = p2n_end   - p1n_end;
      if ( ((d1.GetX() > 0) && (d2.GetX() > 0)) ||
           ((d1.GetX() < 0) && (d2.GetX() < 0)) ||
           ((d1.GetY() > 0) && (d2.GetY() > 0)) ||
           ((d1.GetY() < 0) && (d2.GetY() < 0)))
        { // no intersection
          if (TUA_DEBUG) 
            cout << "  No intersection. No Result." << endl
                 << "temporalUnitIntersection_upoint_upoint: finished OPEN (4)" 
                 << endl;
          return 0; // nothing to do          
        }
      if (TUA_DEBUG) cout << "  Some Intersection." << endl;
      
      dxp1 = (p1n_end - p1n_start).GetX();
      dyp1 = (p1n_end - p1n_start).GetY();
      dxp2 = (p2n_end - p2n_start).GetX();
      dyp2 = (p2n_end - p2n_start).GetY();
      
/*     
Trying to find an intersection point $t$ with $A_1t + B_1 = A_2t + B_2$ 
we get:
    
\[ t_x = \frac{px_{21} - px_{11}}{dxp_1 - dxp_2} \quad
t_y = \frac{py_{21} - py_{11}}{dyp_1 - pyp_2} \]
     
where $t = t_x = t_y$. If $t_x \neq t_y$, then there is no intersection!

*/
      
      dt = (iv.end - iv.start).ToDouble();

      t1 = iv.start.ToDouble();
      t2 = iv.end.ToDouble();

      t_x = (dt*d1.GetX() + t1*(dxp1-dxp2)) / (dxp1-dxp2);
      t_y = (dt*d1.GetY() + t1*(dyp1-dyp2)) / (dyp1-dyp2);
      
      if (TUA_DEBUG)
        {
          cout << "  dt =" << dt  << endl
               << "  t_x=" << t_x << endl
               << "  t_y=" << t_y << endl
               << "  t1 =" << t1  << endl
               << "  t2 =" << t2  << endl;
        }
      
      if ( AlmostEqual(t_x, t_y) &&
           ( t_x >= t1) &&
           ( t_x <= t2) )
        { // We found an intersection
          t.ReadFrom(t_x); // create Instant
          t.SetType(instanttype); 
          iv = Interval<Instant>( t, t, true, true ); // create Interval
          uv1->TemporalFunction(t, p1, true);
          sli->resultValues[sli->NoOfResults] = 
            SetWord( new UPoint(iv, p1, p1) );
          // HIER MUESSEN NOCH P1, P2 AN t ANGEPASST WERDEN!


          ((UPoint*)(sli->resultValues[sli->NoOfResults].addr))
            ->timeInterval=iv;
          sli->NoOfResults++;   
          sli->finished = false;                         
          if (TUA_DEBUG) 
            cout << "  Found intersection. Added result." << endl;
        }      
      // else: no result
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_upoint: finished OPEN (6)" 
             << endl;          
      return 0;
      
    case REQUEST:
      
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_upoint: received REQUEST" 
             << endl;          
      if(local.addr == 0)
        {
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_upoint_upoint: CANCEL (1)" 
                 << endl;                    
          return CANCEL;
        }
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        {
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_upoint_upoint: CANCEL (2)" 
                 << endl;                    
          return CANCEL;
        }
      if (TUA_DEBUG) 
        {
          cout << "  NoOfResults=" << sli->NoOfResults << endl
               << "  NoOfResultsDelivered=" << sli->NoOfResultsDelivered
               << endl;          
        }
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result = SetWord( ((UPoint*)
             (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((UPoint*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
          if (TUA_DEBUG) 
            cout << "temporalUnitIntersection_upoint_upoint: YIELD" 
                 << endl;                    
          return YIELD;
        }
      sli->finished = true;
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_upoint: CANCEL (3)" << endl;
      return CANCEL;
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          while(sli->NoOfResultsDelivered < sli->NoOfResults)
            {
              ((UPoint*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
                ->DeleteIfAllowed();
              sli->NoOfResultsDelivered++;
            }
          delete sli;
        }
      return 0;
    } // end switch
  
  return 0;
}

/*
   value mapping for (upoint point) -> (stream upoint)
                 and (point upoint) -> (stream upoint)

   is identical with    at: (upoint point) -> (stream upoint).
   We just add switches for both signatures and the stream framework

*/

template<int uargindex>
int 
temporalUnitIntersection_upoint_point( Word* args, Word& result, int message,
                                       Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word a0, a1;
  UPoint *unit, pResult;
  Point *val;
  
  
  switch( message )
    {
    case OPEN:
      
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      local = SetWord(sli);
      
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else
        { a0 = args[1]; a1 = args[0]; }
      
      unit = ((UPoint*)a0.addr);
      val  = ((Point*) a1.addr);
      
      if ( !unit->IsDefined() || !val->IsDefined() )
        return 0;
      
      if (unit->At( *val, pResult ))
        {
          pResult.SetDefined(true);
          pResult.timeInterval.start.SetDefined(true);
          pResult.timeInterval.end.SetDefined(true);
          sli->resultValues[sli->NoOfResults] = SetWord( pResult.Clone() );
          sli->NoOfResults++;  
          sli->finished = false;   
        }
      
      return 0;
      
    case REQUEST:
      
      if(local.addr == 0)
        return CANCEL;
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        return CANCEL;
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          result = SetWord( ((UPoint*)
              (sli->resultValues[sli->NoOfResultsDelivered].addr))->Clone() );
          ((UPoint*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
            ->DeleteIfAllowed();
          sli->NoOfResultsDelivered++;
          return YIELD;
        }
      sli->finished = true;
      return CANCEL;
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          while(sli->NoOfResultsDelivered < sli->NoOfResults)
            {
              ((UPoint*)(sli->resultValues[sli->NoOfResultsDelivered].addr))
                ->DeleteIfAllowed();
              sli->NoOfResultsDelivered++;
            }
          delete sli;
        }
      return 0;
    } // end switch
  
  return 0;
}

/*
Value mapping for 

----

  intersection:     (upoint line) -> (stream upoint)
                    (line upoint) -> (stream upoint)

----

Method ~TUUPointInsideLine~ 

Copied from TempralLiftedAlgebra

calcultates the periods where the given UPoint lies
inside the given Line. It returns the existing intervals in a Periods-Object. 

*/
static void TUUPointInsideLine(UPoint *u, Line& ln, Periods& pResult)
{
  if(TUA_DEBUG)
    cout<<"MPointLineInside called"<<endl;
  const HalfSegment *l;

  const UPoint* up = (UPoint*) u;
  Periods* period = new Periods(0);
  Periods* between = new Periods(0);
  Point pt;
  Interval<Instant> newper; //part of the result
  
  pResult.Clear();

    if(TUA_DEBUG){
      cout<<"UPoint # "<<" ["<<up->timeInterval.start.ToString()<<" "
      <<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc<<" "
      <<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()
      <<")->("<<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;}

    for( int n = 0; n < ln.Size(); n++)
    {
      Instant t;
      ln.Get(n, l);
      if(TUA_DEBUG){
        cout<<"UPoint # "<<" ["<<up->timeInterval.start.ToString()
        <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc
        <<" "<<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()
        <<")->("<<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;
        cout<<"l      # "<<n<<" ("<<l->GetLeftPoint().GetX()
        <<" "<<l->GetLeftPoint().GetY()
        <<" "<<l->GetRightPoint().GetX()<<" "
        <<l->GetRightPoint().GetY()<<") "<<endl;}
      if (l->GetRightPoint().GetX() == l->GetDomPoint().GetX() 
       && l->GetRightPoint().GetY() == l->GetDomPoint().GetY()) {
        if(TUA_DEBUG)
          cout<<"right point is dominating -> continue"<<endl;
        continue;
      }
      if(( l->GetRightPoint().GetX() < up->p0.GetX() 
       &&  l->GetRightPoint().GetX() < up->p1.GetX()) 
       || (l->GetLeftPoint().GetX() > up->p0.GetX() 
       &&  l->GetLeftPoint().GetX() > up->p1.GetX()) 
       || (l->GetRightPoint().GetY() < up->p0.GetY() 
       &&  l->GetRightPoint().GetY() < up->p1.GetY() 
       && (l->GetLeftPoint().GetY() < up->p0.GetY() 
       &&  l->GetLeftPoint().GetY() < up->p1.GetY())) 
       || (l->GetRightPoint().GetY() > up->p0.GetY() 
       &&  l->GetRightPoint().GetY() > up->p1.GetY() 
       && (l->GetLeftPoint().GetY() > up->p0.GetY() 
       &&  l->GetLeftPoint().GetY() > up->p1.GetY()))) {
        if(TUA_DEBUG)
          cout<<"Bounding Boxes not crossing!"<<endl;
        continue;
      }
      double al, bl, aup, bup;
      bool vl, vup;
      vl = l->GetRightPoint().GetX() == l->GetLeftPoint().GetX();
      if(!vl){
        al = (l->GetRightPoint().GetY() - l->GetLeftPoint().GetY()) 
           / (l->GetRightPoint().GetX() - l->GetLeftPoint().GetX());
        bl =  l->GetLeftPoint().GetY() - l->GetLeftPoint().GetX() * al;
          if(TUA_DEBUG)
            cout<<"al: "<<al<<" bl: "<<bl<<endl;
      }
      else
        if(TUA_DEBUG)
          cout<<"l is vertical"<<endl;
      vup = up->p1.GetX() == up->p0.GetX();
      if(!vup){
        aup = (up->p1.GetY() - up->p0.GetY()) 
            / (up->p1.GetX() - up->p0.GetX());
        bup =  up->p0.GetY() - up->p0.GetX() * aup;
        if(TUA_DEBUG)
          cout<<"aup: "<<aup<<" bup: "<<bup<<endl;
      }
      else 
        if(TUA_DEBUG)
          cout<<"up is vertical"<<endl;
      if(vl && vup){
        if(TUA_DEBUG)
          cout<<"both elements are vertical!"<<endl;
        if(up->p1.GetX() != l->GetLeftPoint().GetX()){
        if(TUA_DEBUG)
          cout<<"elements are vertical but not at same line"<<endl;
          continue;
        }
        else {
          if(TUA_DEBUG)
            cout<<"elements on same line"<<endl;
          if(up->p1.GetY() < l->GetLeftPoint().GetY() 
           && up->p0.GetY() < l->GetLeftPoint().GetY()){
            if(TUA_DEBUG)
              cout<<"uPoint lower as linesegment"<<endl;
            continue;
          }
          else if(up->p1.GetY() > l->GetRightPoint().GetY() 
           && up->p0.GetY() > l->GetRightPoint().GetY()){
            if(TUA_DEBUG)
              cout<<"uPoint higher as linesegment"<<endl;
            continue;
          }
          else{
            if(TUA_DEBUG)
              cout<<"uPoint and linesegment partequal"<<endl;
            if (up->p0.GetY() <= l->GetLeftPoint().GetY() 
             && up->p1.GetY() >= l->GetLeftPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint starts below linesegemet"<<endl;
              t.ReadFrom((l->GetLeftPoint().GetY() - up->p0.GetY()) 
                     / (up->p1.GetY() - up->p0.GetY()) 
                     * (up->timeInterval.end.ToDouble() 
                     -  up->timeInterval.start.ToDouble()) 
                     +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TUA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t) 
                         ? up->timeInterval.lc : true;
            }
            if(up->p1.GetY() <= l->GetLeftPoint().GetY() 
             && up->p0.GetY() >= l->GetLeftPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint ends below linesegemet"<<endl;
              t.ReadFrom((l->GetLeftPoint().GetY() - up->p0.GetY()) 
                      / (up->p1.GetY() - up->p0.GetY()) 
                      * (up->timeInterval.end.ToDouble() 
                      -  up->timeInterval.start.ToDouble()) 
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TUA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t) 
                         ? up->timeInterval.rc : true;
            }
            if(up->p0.GetY() <= l->GetRightPoint().GetY() 
             && up->p1.GetY() >= l->GetRightPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint ends above linesegemet"<<endl;
              t.ReadFrom((l->GetRightPoint().GetY() - up->p0.GetY()) 
                      / (up->p1.GetY() - up->p0.GetY()) 
                      * (up->timeInterval.end.ToDouble() 
                      -  up->timeInterval.start.ToDouble()) 
                      +  up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TUA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.end = t;
              newper.rc = (up->timeInterval.end == t) 
                         ? up->timeInterval.rc : true;
            }
            if(up->p1.GetY() <= l->GetRightPoint().GetY() 
             && up->p0.GetY() >= l->GetRightPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint starts above linesegemet"<<endl;
              t.ReadFrom((l->GetRightPoint().GetY() - up->p0.GetY()) 
                      / (up->p1.GetY() - up->p0.GetY()) 
                      * (up->timeInterval.end.ToDouble() 
                      - up->timeInterval.start.ToDouble()) 
                      + up->timeInterval.start.ToDouble());
              t.SetType(instanttype);
              if(TUA_DEBUG)
                cout<<"t "<<t.ToString()<<endl;
              newper.start = t;
              newper.lc = (up->timeInterval.start == t) 
                         ? up->timeInterval.lc : true;
            }
            if (up->p0.GetY() <= l->GetRightPoint().GetY() 
             && up->p0.GetY() >= l->GetLeftPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint starts inside linesegemet"<<endl;
              newper.start = up->timeInterval.start;
              newper.lc =    up->timeInterval.lc;
            }
            if( up->p1.GetY() <= l->GetRightPoint().GetY() 
             && up->p1.GetY() >= l->GetLeftPoint().GetY()){
              if(TUA_DEBUG)
                cout<<"uPoint ends inside linesegemet"<<endl;
              newper.end = up->timeInterval.end;
              newper.rc =  up->timeInterval.rc;
            }
            if(newper.start == newper.end 
             && (!newper.lc || !newper.rc)){
              if(TUA_DEBUG)
                cout<<"not an interval"<<endl;
              continue;
            }
          }
        }
      }
      else if(vl){
        if(TUA_DEBUG)
          cout<<"vl is vertical vup not"<<endl;
        t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX()) 
                / (up->p1.GetX() - up->p0.GetX()) 
                * (up->timeInterval.end.ToDouble() 
                -  up->timeInterval.start.ToDouble()) 
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if(TUA_DEBUG)
          cout<<"t "<<t.ToString()<<endl;
        if((up->timeInterval.start == t && !up->timeInterval.lc) 
         ||  (up->timeInterval.end == t && !up->timeInterval.rc))
          continue;
          
        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          if(TUA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt);
        if(  pt.GetX() < l->GetLeftPoint().GetX() || 
             pt.GetX() > l->GetRightPoint().GetX()
         || (pt.GetY() < l->GetLeftPoint().GetY() && 
             pt.GetY() < l->GetRightPoint().GetY())
         || (pt.GetY() > l->GetLeftPoint().GetY() && 
             pt.GetY() > l->GetRightPoint().GetY())){
          if(TUA_DEBUG)
            cout<<"pt outside up!"<<endl;
          continue;
        }
        
        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      else if(vup){
        if(TUA_DEBUG)
          cout<<"vup is vertical vl not"<<endl;
        if(up->p1.GetY() != up->p0.GetY()) {
          t.ReadFrom((up->p0.GetX() * al + bl - up->p0.GetY()) 
                  / (up->p1.GetY() - up->p0.GetY()) 
                  * (up->timeInterval.end.ToDouble() 
                  -  up->timeInterval.start.ToDouble()) 
                  +  up->timeInterval.start.ToDouble());
          t.SetType(instanttype);
          if(TUA_DEBUG)
            cout<<"t "<<t.ToString()<<endl;
          if((up->timeInterval.start == t && !up->timeInterval.lc) 
           ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
            if(TUA_DEBUG)
              cout<<"continue"<<endl;
            continue;
          }
          
          if(up->timeInterval.start > t|| up->timeInterval.end < t){
            if(TUA_DEBUG)
              cout<<"up outside line"<<endl;
            continue;
          }
          up->TemporalFunction(t, pt);
          if(  pt.GetX() < l->GetLeftPoint().GetX() ||  
               pt.GetX() > l->GetRightPoint().GetX()
           || (pt.GetY() < l->GetLeftPoint().GetY() && 
               pt.GetY() < l->GetRightPoint().GetY())
           || (pt.GetY() > l->GetLeftPoint().GetY() && 
               pt.GetY() > l->GetRightPoint().GetY())){
            if(TUA_DEBUG)
              cout<<"pt outside up!"<<endl;
            continue;
          }
          
          newper.start = t;
          newper.lc = true;
          newper.end = t;
          newper.rc = true;
        }
        else {
          if(TUA_DEBUG)
            cout<<"up is not moving"<<endl;
          if(al * up->p1.GetX() + bl == up->p1.GetY()){
            if(TUA_DEBUG)
              cout<<"Point lies on line"<<endl;
            newper = up->timeInterval;
          }
          else {
            if(TUA_DEBUG)
              cout<<"continue 2"<<endl;
            continue;
          }
        }
      }
      else if(aup == al){
        if(TUA_DEBUG)
          cout<<"both lines have same gradient"<<endl;
        if(bup != bl){
          if(TUA_DEBUG)
            cout<<"colinear but not equal"<<endl;
          continue;
        }
         if(up->p0.GetX() <= l->GetLeftPoint().GetX() 
         && up->p1.GetX() >= l->GetLeftPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint starts left of linesegemet"<<endl;
           t.ReadFrom((l->GetLeftPoint().GetX() - up->p0.GetX()) 
                   / (up->p1.GetX() - up->p0.GetX()) 
                   * (up->timeInterval.end.ToDouble() 
                   -  up->timeInterval.start.ToDouble()) 
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TUA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t) 
                      ? up->timeInterval.lc : true;
        }
        if(up->p1.GetX() <= l->GetLeftPoint().GetX() 
        && up->p0.GetX() >= l->GetLeftPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint ends left of linesegemet"<<endl;
           t.ReadFrom((l->GetLeftPoint().GetX() - up->p0.GetX()) 
                   / (up->p1.GetX() - up->p0.GetX()) 
                   * (up->timeInterval.end.ToDouble() 
                   -  up->timeInterval.start.ToDouble()) 
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TUA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t) 
                      ? up->timeInterval.rc : true;
        }
        if(up->p0.GetX() <= l->GetRightPoint().GetX() 
        && up->p1.GetX() >= l->GetRightPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint ends right of linesegemet"<<endl;
           t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX()) 
                   / (up->p1.GetX() - up->p0.GetX()) 
                   * (up->timeInterval.end.ToDouble() 
                   -  up->timeInterval.start.ToDouble()) 
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TUA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.end = t;
           newper.rc = (up->timeInterval.end == t) 
                      ? up->timeInterval.rc : true;
        }
        if(up->p1.GetX() <= l->GetRightPoint().GetX() 
        && up->p0.GetX() >= l->GetRightPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint starts right of linesegemet"<<endl;
           t.ReadFrom((l->GetRightPoint().GetX() - up->p0.GetX()) 
                   / (up->p1.GetX() - up->p0.GetX()) 
                   * (up->timeInterval.end.ToDouble() 
                   -  up->timeInterval.start.ToDouble()) 
                   +  up->timeInterval.start.ToDouble());
           t.SetType(instanttype);
           if(TUA_DEBUG)
             cout<<"t "<<t.ToString()<<endl;
           newper.start = t;
           newper.lc = (up->timeInterval.start == t) 
                      ? up->timeInterval.lc : true;
        }
        if(up->p0.GetX() <= l->GetRightPoint().GetX() 
        && up->p0.GetX() >= l->GetLeftPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint starts inside linesegemet"<<endl;
           newper.start = up->timeInterval.start;
           newper.lc =    up->timeInterval.lc;
        }
        if(up->p1.GetX() <= l->GetRightPoint().GetX() 
        && up->p1.GetX() >= l->GetLeftPoint().GetX()){
           if(TUA_DEBUG)
             cout<<"uPoint ends inside linesegemet"<<endl;
           newper.end = up->timeInterval.end;
           newper.rc =  up->timeInterval.rc;
        }
        if(newper.start == newper.end 
        && (!newper.lc || !newper.rc)){
          if(TUA_DEBUG)
            cout<<"not an interval"<<endl;
          continue;
        }
      }
      else{
        if(TUA_DEBUG)
          cout<<"both lines have different gradients"<<endl;
        t.ReadFrom(((bl - bup) / (aup - al) - up->p0.GetX()) 
                / (up->p1.GetX() - up->p0.GetX()) 
                * (up->timeInterval.end.ToDouble() 
                -  up->timeInterval.start.ToDouble()) 
                +  up->timeInterval.start.ToDouble());
        t.SetType(instanttype);
        if((up->timeInterval.start == t && !up->timeInterval.lc) 
         ||  (up->timeInterval.end == t && !up->timeInterval.rc)){
          if(TUA_DEBUG)
            cout<<"continue"<<endl;
          continue;
        }
        
        if(up->timeInterval.start > t|| up->timeInterval.end < t){
          if(TUA_DEBUG)
            cout<<"up outside line"<<endl;
          continue;
        }
        up->TemporalFunction(t, pt);
        if(  pt.GetX() < l->GetLeftPoint().GetX() ||  
             pt.GetX() > l->GetRightPoint().GetX()
         || (pt.GetY() < l->GetLeftPoint().GetY() && 
             pt.GetY() < l->GetRightPoint().GetY())
         || (pt.GetY() > l->GetLeftPoint().GetY() && 
             pt.GetY() > l->GetRightPoint().GetY())){
          if(TUA_DEBUG)
            cout<<"pt outside up!"<<endl;
          continue;
        }
        
        newper.start = t;
        newper.lc = true;
        newper.end = t;
        newper.rc = true;
      }
      if(TUA_DEBUG){
        cout<<"newper ["<< newper.start.ToString()<<" "<<newper.end.ToString()
        <<" "<<newper.lc<<" "<<newper.rc<<"]"<<endl;}
      period->Clear();
      period->StartBulkLoad();
      period->Add(newper);
      period->EndBulkLoad(false);
      if (!pResult.IsEmpty()) {
        between->Clear();
        period->Union(pResult, *between);
        pResult.Clear();
        pResult.CopyFrom(between);
      }
      else 
        pResult.CopyFrom(period);
    }
  delete between;
  delete period;
}

/*
Method ~TUCompletePeriods2MPoint~

Copied from TempralLiftedAlgebra

Completes a Periods-value to a MPoint-value. For this it adds the starting
and end points.

*/
static void TUCompletePeriods2MPoint(UPoint* u, Periods* pResult,
  MPoint* endResult){
  if(TUA_DEBUG)
    cout<<"TUCompletePeriods2MPoint called"<<endl;
  
  const UPoint* up = (UPoint*) u;
  endResult->Clear();
  endResult->StartBulkLoad();
  const Interval<Instant> *per;
  UPoint newUp;
  Point pt;
  int m = 0;
  bool pfinished = (pResult->GetNoComponents() == 0);
  for ( int i = 0; i < 1; i++) {
    if(!up->IsDefined())
        continue;
    if(TUA_DEBUG){
      cout<<"UPoint # "<<" ["<<up->timeInterval.start.ToString()
      <<" "<<up->timeInterval.end.ToString()<<" "<<up->timeInterval.lc<<" "
      <<up->timeInterval.rc<<"] ("<<up->p0.GetX()<<" "<<up->p0.GetY()<<")->("
      <<up->p1.GetX()<<" "<<up->p1.GetY()<<")"<<endl;}
    if(!pfinished) {
      pResult->Get(m, per);
      if(TUA_DEBUG){
        cout<<"per "<<m<<" ["<<per->start.ToString()<<" "
        <<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
    }
    if(pfinished) {
      if(TUA_DEBUG)
        cout<<"no per any more. break 1"<<endl;
      break;
    }
    if(!(pfinished || up->timeInterval.end < per->start 
     || (up->timeInterval.end == per->start 
     && !up->timeInterval.rc && per->lc))) {
      if(TUA_DEBUG)
        cout<<"per not totally after up"<<endl;
      if(up->timeInterval.start < per->start 
       || (up->timeInterval.start == per->start 
       && up->timeInterval.lc && !per->lc)) {
        if(TUA_DEBUG)
          cout<<"up starts before per"<<endl;
        newUp.timeInterval = *per;
      }
      else {
        if(TUA_DEBUG)
          cout<<"per starts before or with up"<<endl;
        newUp.timeInterval.start = up->timeInterval.start;
        newUp.timeInterval.lc = up->timeInterval.lc;
      }
      while(true) {
        if(up->timeInterval.end < per->end
         || (up->timeInterval.end == per->end 
         && per->rc && !up->timeInterval.rc)) {
            if(TUA_DEBUG)
              cout<<"per ends after up (break)"<<endl;
            newUp.timeInterval.end = up->timeInterval.end; 
            newUp.timeInterval.rc = up->timeInterval.rc; 
            up->TemporalFunction(newUp.timeInterval.start, pt, true);
            newUp.p0 = pt;
            up->TemporalFunction(newUp.timeInterval.end, pt, true);
            newUp.p1 = pt;
            if(TUA_DEBUG){
              cout<<"Add3 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
              <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
              <<") ["<<newUp.timeInterval.start.ToString()<<" "
              <<newUp.timeInterval.end.ToString()<<" "
              <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;}
            endResult->Add(newUp); 
            break;
        }
        else {
          if(TUA_DEBUG)
            cout<<"per ends inside up"<<endl;
          newUp.timeInterval.end = per->end;
          newUp.timeInterval.rc = per->rc;
          up->TemporalFunction(newUp.timeInterval.start, pt, true);
          newUp.p0 = pt;
          up->TemporalFunction(newUp.timeInterval.end, pt, true);
          newUp.p1 = pt;
          if(TUA_DEBUG){
            cout<<"Add4 ("<<newUp.p0.GetX()<<" "<<newUp.p0.GetY()
             <<")->("<<newUp.p1.GetX()<<" "<<newUp.p1.GetY()
            <<") ["<<newUp.timeInterval.start.ToString()<<" "
            <<newUp.timeInterval.end.ToString()<<" "
            <<newUp.timeInterval.lc<<" "<<newUp.timeInterval.rc<<"]"<<endl;}
          endResult->Add(newUp);
        }
        if(m == pResult->GetNoComponents() - 1){
          if(TUA_DEBUG)
            cout<<"last per"<<endl;
          pfinished = true;
        }
        else {
          pResult->Get(++m, per);
          if(TUA_DEBUG){
            cout<<"per "<<m<<" ["<<per->start.ToString()
            <<" "<<per->end.ToString()<<" "<<per->lc<<" "<<per->rc<<"]"<<endl;}
        }
        if(!pfinished && (per->start < up->timeInterval.end 
           || (per->start == up->timeInterval.end 
           && up->timeInterval.rc && per->rc))){
          if(TUA_DEBUG)
            cout<<"next per starts in same up"<<endl;
          newUp.timeInterval.start = per->start; 
          newUp.timeInterval.lc = per->lc; 
        }
        else {
          if(TUA_DEBUG)
            cout<<"next interval after up -> finish up"<<endl;
          break;
        }
      } //while
    }
  }
  endResult->EndBulkLoad(false);
}

/*
The value mapping function:

*/

template<int uargindex>
int temporalUnitIntersection_upoint_line( Word* args, Word& result, 
                                          int message,
                                          Word& local, Supplier s )
{
  TUIntersectionLocalInfo *sli;
  Word     a0, a1;
  UPoint   res;
  UPoint  *u;
  Line    *l;
  Periods *p;

  const UPoint* cu;

  switch( message )
    {
    case OPEN:
      
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_line<" 
             << uargindex << ">: Received OPEN" << endl;

      p = new Periods(10);
      sli = new TUIntersectionLocalInfo;
      sli->finished = true;
      sli->NoOfResults = 0;
      sli->NoOfResultsDelivered = 0;
      sli->mpoint = new MPoint(10);
      local = SetWord(sli);

      // initialize arguments, such that a0 always contains the upoint
      //                       and a1 the line 
      if (TUA_DEBUG) cout << "  uargindex=" << uargindex << endl;
      if (uargindex == 0)
        { a0 = args[0]; a1 = args[1]; }
      else
        { a0 = args[1]; a1 = args[0]; }
      u = (UPoint*)(a0.addr);
      l = (Line*)(a1.addr);

      // test for definedness
      if ( !u->IsDefined() || !l->IsDefined() || l->IsEmpty() )
        {
          if (TUA_DEBUG) 
            cout << "  Undef/Empty arg -> Empty Result" << endl << endl;
          // nothing to do
        }
      else
        {
          TUUPointInsideLine(u, *l, *p);    // get intersecting timeintervals
          TUCompletePeriods2MPoint(u, p, sli->mpoint); // create upoints
          sli->NoOfResults = sli->mpoint->GetNoComponents();
          sli->finished = (sli->NoOfResults <= 0);
          if (TUA_DEBUG) 
            cout << "  " << sli->NoOfResults << " result units" << endl << endl;
        }
      delete p;
      if (TUA_DEBUG) 
        cout << "temporalUnitIntersection_upoint_line: Finished OPEN" 
             << endl;      
      return 0;
      
    case REQUEST:
      
      if(local.addr == 0)
        return CANCEL;
      sli = (TUIntersectionLocalInfo*) local.addr;
      if(sli->finished)
        return CANCEL;
      if(sli->NoOfResultsDelivered < sli->NoOfResults)
        {
          sli->mpoint->Get(sli->NoOfResultsDelivered, cu);
          result = SetWord( cu->Clone() );
          sli->NoOfResultsDelivered++;
          return YIELD;
        }
      sli->finished = true;
      return CANCEL;
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TUIntersectionLocalInfo*) local.addr;
          delete sli->mpoint;
          delete sli;
        }
      return 0;
    } // end switch
  
  return 0;
}  

// for (upoint uregion) -> (stream upoint)
//     (uregion upoint) -> (stream upoint)
template<int uargindex>
int temporalUnitIntersection_upoint_uregion( Word* args, Word& result, 
                                             int message,
                                             Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitIntersection_upoint_uregion "
       << "not yet implemented!" << endl;  

  return 0;
}

// for (upoint region) -> (stream upoint)
//     (region upoint) -> (stream upoint)
template<int uargindex>
int temporalUnitIntersection_upoint_region( Word* args, Word& result, 
                                            int message,
                                            Word& local, Supplier s )
{
  cout << "\nATTENTION: temporalUnitIntersection_upoint_region "
       << "not yet implemented!" << endl;  

  return 0;
}

/*
5.26.3 Specification for operator ~intersection~

*/

const string  TemporalUnitIntersectionSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(" 
  "<text>For T in {bool, int, real, string, point}:\n"
  "(uT uT) -> (stream uT)**\n"
  "(uT  T) -> (stream uT)\n"
  "( T uT) -> (stream uT)\n"
  "(line upoint) -> (stream upoint)\n"
  "(upoint line) -> (stream upoint)\n"
  "(upoint uregion) -> (stream upoint)*\n"
  "(uregion upoint) -> (stream upoint)*\n"
  "(upoint region) -> (stream upoint)*\n"
  "(region upoint) -> (stream upoint)*\n"
  "(*):  Not yet implemented\n"
  "(**): Not yet implemented for T = real</text--->"
  "<text>intersection(_, _)</text--->"
  "<text>Returns the intersection of two unit datatype values "
  "or of a unit datatype and its corrosponding simple datatype "
  "as a stream of that unit datatype.</text--->"
  "<text>query intersection(upoint1, upoint2) count</text--->"
  ") )";


/*
5.26.4 Selection Function of operator ~intersection~

*/

ValueMapping temporalunitintersectionmap[] =
  {
    temporalUnitIntersection_CU_CU<UBool>,
    temporalUnitIntersection_CU_CU<UInt>,
    temporalUnitIntersection_ureal_ureal,
    temporalUnitIntersection_upoint_upoint,
    temporalUnitIntersection_CU_CU<UString>,
    
    temporalUnitIntersection_CU_C<UBool, CcBool, 0>,
    temporalUnitIntersection_CU_C<UInt, CcInt, 0>,
    temporalUnitIntersection_ureal_real<0>,
    temporalUnitIntersection_upoint_point<0>,
    temporalUnitIntersection_CU_C<UString, CcString, 0>,
    
    temporalUnitIntersection_CU_C<UBool, CcBool, 1>,
    temporalUnitIntersection_CU_C<UInt, CcInt, 1>,
    temporalUnitIntersection_ureal_real<1>,
    temporalUnitIntersection_upoint_point<1>,
    temporalUnitIntersection_CU_C<UString, CcString, 1>,
    
    temporalUnitIntersection_upoint_line<0>,
    temporalUnitIntersection_upoint_line<1>,
    
    temporalUnitIntersection_upoint_uregion<0>,
    temporalUnitIntersection_upoint_uregion<1>,

    temporalUnitIntersection_upoint_region<0>,
    temporalUnitIntersection_upoint_region<1>
  };

int temporalunitIntersectionSelect( ListExpr args )
{
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);
  if( nl->IsEqual( arg1, "ubool" )   && 
      nl->IsEqual( arg2, "ubool") )   return 0;
  if( nl->IsEqual( arg1, "uint" )    &&
      nl->IsEqual( arg2, "uint" ) )    return 1;
  if( nl->IsEqual( arg1, "ureal" )   &&
      nl->IsEqual( arg2, "ureal" ) )   return 2;
  if( nl->IsEqual( arg1, "upoint" )  &&
      nl->IsEqual( arg2, "upoint" ) )  return 3;
  if( nl->IsEqual( arg1, "ustring" ) &&
      nl->IsEqual( arg2, "ustring" ) ) return 4;
  
  if( nl->IsEqual( arg1, "ubool" )   &&
      nl->IsEqual( arg2, "bool" ))     return 5;
  if( nl->IsEqual( arg1, "uint" )    &&
      nl->IsEqual( arg2, "int" ))      return 6;
  if( nl->IsEqual( arg1, "ureal" )   && 
      nl->IsEqual( arg2, "real" ))     return 7;
  if( nl->IsEqual( arg1, "upoint" )  &&
      nl->IsEqual( arg2, "point"  ) )  return 8;
  if( nl->IsEqual( arg1, "ustring" ) &&
      nl->IsEqual( arg2, "string" ))   return 9;
  
  if( nl->IsEqual( arg2, "ubool" )   &&
      nl->IsEqual( arg1, "bool" ))     return 10;
  if( nl->IsEqual( arg2, "uint" )    &&
      nl->IsEqual( arg1, "int" ))      return 11;
  if( nl->IsEqual( arg2, "ureal" )   &&
      nl->IsEqual( arg1, "real" ))     return 12;
  if( nl->IsEqual( arg2, "upoint" )  &&
      nl->IsEqual( arg1, "point" ))    return 13;
  if( nl->IsEqual( arg2, "ustring" ) &&
      nl->IsEqual( arg1, "ustring" ))  return 14;
  
  if( nl->IsEqual( arg1, "upoint" )      &&
      nl->IsEqual( arg2, "line" ))     return 15;
  if( nl->IsEqual( arg1, "line" )    &&
      nl->IsEqual( arg2, "upoint" ) )      return 16;
  if( nl->IsEqual( arg1, "upoint" )    &&
      nl->IsEqual( arg2, "uregion" ) )   return 17;
  if( nl->IsEqual( arg1, "uregion" )   &&
      nl->IsEqual( arg2, "upoint" ) )    return 18;
  if( nl->IsEqual( arg1, "upoint" )    &&
      nl->IsEqual( arg2, "region" ) )    return 19;
  if( nl->IsEqual( arg1, "region" )    &&
      nl->IsEqual( arg2, "upoint" ) )    return 20;

  return -1;
}


/*
5.26.5 Definition of operator ~intersection~

*/

Operator temporalunitintersection( "intersection",
                                   TemporalUnitIntersectionSpec,
                                   18,
                                   temporalunitintersectionmap,
                                   temporalunitIntersectionSelect,
                                   TemporalUnitIntersectionTypeMap);



/*
5.27 Operator ~transformstream~

----
  transformstream: (stream T) -> stream(tuple((element T)))
                   stream(tuple((id T))) -> (stream T)

  for T in kind DATA, id some arbitrary identifier

----

Operator ~transformstream~ transforms a (stream DATA) into a 
(stream(tuple((element DATA)))) and vice versa. ~element~ is the name for the 
attribute created. 

The result of the first variant can e.g. be consumed to form a relation
or be processed using ordinary tuplestream operators.

*/

/*
5.27.1 Type mapping function for ~transformstream~

*/

ListExpr TemporalUnitTransformstreamTypeMap(ListExpr args)
{
  ListExpr first ;
  string argstr;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  ListExpr TupleDescr, T;
  
  if (nl->ListLength(args) != 1)
    {
      ErrorReporter::ReportError("Operator transformstream expects a list of "
                                 "length one.");
      return nl->SymbolAtom("typeerror");
    }
  
  first = nl->First(args);
  nl->WriteToString(argstr, first);
  
  // check for variant 1: (stream T)
  if ( !nl->IsAtom(first) && 
       (nl->ListLength(first) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       nl->IsAtom(nl->Second(first)) &&
       am->CheckKind("DATA", nl->Second(first), errorInfo) )
    {
      T = nl->Second(first);
      return nl->TwoElemList( 
        nl->SymbolAtom("stream"),
        nl->TwoElemList( 
            nl->SymbolAtom("tuple"), 
            nl->OneElemList( 
                nl->TwoElemList( 
                    nl->SymbolAtom("elem"),
                    T))));
    }
  // check for variant 2: stream(tuple((id T)))
  if ( !nl->IsAtom(first) && 
       (nl->ListLength(first) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       !nl->IsAtom(nl->Second(first)) &&
       (nl->ListLength(nl->Second(first)) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) )
    {
      TupleDescr = nl->Second(nl->Second(first));
      nl->WriteToString(argstr, TupleDescr);
      if(TUA_DEBUG) 
        cout << "\n In tupledescr = " << argstr << endl;      
      if ( !nl->IsAtom(TupleDescr) &&
           (nl->ListLength(TupleDescr) == 1) &&
           !nl->IsAtom(nl->First(TupleDescr)) &&
           (nl->ListLength(nl->First(TupleDescr)) == 2) &&
           (nl->IsAtom(nl->First(nl->First(TupleDescr)))) &&
           (nl->IsAtom(nl->Second(nl->First(TupleDescr)))) &&
           am->CheckKind("DATA", nl->Second(nl->First(TupleDescr)), errorInfo))
        {
          T = nl->Second(nl->First(TupleDescr));
          return nl->TwoElemList(
                                 nl->SymbolAtom("stream"),
                                 T);
        }
    }
  
  // Wrong argument format!
  ErrorReporter::ReportError(
    "Operator transformstream expects exactly one argument. either "
    "of type '(stream T)',or 'stream(tuple((id T))))', where T is of "
    "kind DATA.\n"
    "The passed argument has type '"+ argstr +"'.");
  return nl->SymbolAtom("typeerror");
}

/*
5.27.2 Value mapping for operator ~transformstream~

*/

struct TransformstreamLocalInfo 
{
  bool     finished;
  TupleType *resultTupleType;
};

// The first variant creates a tuplestream from a stream:
int Transformstream_S_TS(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  TransformstreamLocalInfo *sli;
  Word      value;
  ListExpr  resultType;
  Tuple     *newTuple;
  
  
  switch ( message )
    {
    case OPEN:
      
      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;
      
      resultType = GetTupleResultType( s );
      sli->resultTupleType = new TupleType( nl->Second( resultType ) );
      sli->finished = false;
      local = SetWord(sli);
      return 0;
      
    case REQUEST:
      
      if (local.addr == 0)
        return CANCEL;
      
      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        return CANCEL;
      
      result = SetWord((Attribute*)((qp->ResultStorage(s)).addr));
      
      qp->Request( args[0].addr, value );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          qp->Close( args[0].addr );
          sli->finished = true;
          result.addr = 0;
          return CANCEL;
        }
      // create tuple, copy and pass result, delete value
      newTuple = new Tuple( sli->resultTupleType );
      newTuple->PutAttribute( 0, ((Attribute*)value.addr)->Clone() );
      ((Attribute*)(value.addr))->DeleteIfAllowed();
      result = SetWord(newTuple);
      return YIELD;       
      
    case CLOSE:
      
      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished)
            qp->Close( args[0].addr );
          sli->resultTupleType->DeleteIfAllowed();
          delete sli;
        }
      return 0;
    }
  cout << "Transformstream_S_TS: UNKNOWN MESSAGE!" << endl;
  return 0;
}

// The second variant creates a stream from a tuplestream:
int Transformstream_TS_S(Word* args, Word& result, int message, 
                         Word& local, Supplier s)
{
  TransformstreamLocalInfo *sli;
  Word   tuple;
  Tuple* tupleptr;
  
  switch ( message )
    {
    case OPEN:
      if (TUA_DEBUG) cout << "Transformstream_TS_S: OPEN called" << endl;
      qp->Open( args[0].addr );
      sli = new TransformstreamLocalInfo;
      sli->finished = false;
      local = SetWord(sli);
      if (TUA_DEBUG) cout << "Transformstream_TS_S: OPEN finished" << endl;
      return 0;
      
    case REQUEST:
      if (TUA_DEBUG) cout << "Transformstream_TS_S: REQUEST called" << endl;
      if (local.addr == 0)
        {
          if (TUA_DEBUG) cout 
            << "Transformstream_TS_S: REQUEST return CANCEL (1)" << endl;
          return CANCEL;
        }
      
      sli = (TransformstreamLocalInfo*) (local.addr);
      if (sli->finished)
        {
          if (TUA_DEBUG) cout 
            << "Transformstream_TS_S: REQUEST return CANCEL (2)" << endl;
          return CANCEL;
        }
      
      qp->Request( args[0].addr, tuple );
      if (!qp->Received( args[0].addr ))
        { // input stream consumed
          qp->Close( args[0].addr );
          sli->finished = true;
          result.addr = 0;
          if (TUA_DEBUG) cout 
            << "Transformstream_TS_S: REQUEST return CANCEL (3)" << endl;
          return CANCEL;
        }
      // extract, copy and pass value, delete tuple
      tupleptr = (Tuple*)tuple.addr;
      result.addr = tupleptr->GetAttribute(0)->Clone();
      tupleptr->DeleteIfAllowed();
      if (TUA_DEBUG) cout 
        << "Transformstream_TS_S: REQUEST return YIELD" << endl;
      return YIELD;       
      
    case CLOSE:
      
      if (TUA_DEBUG) cout << "Transformstream_TS_S: CLOSE called" << endl;
      if (local.addr != 0)
        {
          sli = (TransformstreamLocalInfo*) (local.addr);
          if (!sli->finished)
            qp->Close( args[0].addr );
          delete sli;
        }
      if (TUA_DEBUG) cout << "Transformstream_TS_S: CLOSE finished" << endl;
      return 0;
      
    }
  cout << "Transformstream_TS_S: UNKNOWN MESSAGE!" << endl;
  return 0;
}

/*
5.27.3 Specification for operator ~transformstream~

*/
const string TemporalUnitTransformstreamSpec = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "(" 
  "<text>For T in kind DATA:\n"
  "(stream T) -> stream(tuple((elem T)))\n"
  "stream(tuple(attrname T)) -> (stream T)</text--->"
  "<text>_ transformstream</text--->"
  "<text>Transforms a 'stream T' into a tuplestream "
  "with a single attribute 'elem' containing the "
  "values coming from the input stream and vice "
  "versa. The identifier 'elem' is fixed, the "
  "attribute name 'attrname' may be arbitrary "
  "chosen, but the tuplestream's tupletype may "
  "have only a single attribute.</text--->"
  "<text>query intstream(1,5) transformstream consume\n "
  "query ten feed transformstream printstream count</text--->"
  ") )";

/*
5.27.4 Selection Function of operator ~transformstream~

*/

ValueMapping temporalunittransformstreammap[] = 
  {
    Transformstream_S_TS,
    Transformstream_TS_S
  };

int temporalunitTransformstreamSelect( ListExpr args )
{
  ListExpr first = nl->First( args );
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  if ( !nl->IsAtom(first) && 
       (nl->ListLength(first) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       nl->IsAtom(nl->Second(first)) &&
       am->CheckKind("DATA", nl->Second(first), errorInfo) )
    return 0;
  if ( !nl->IsAtom(first) && 
       (nl->ListLength(first) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
       !nl->IsAtom(nl->Second(first)) &&
       (nl->ListLength(nl->Second(first)) == 2) && 
       (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple))
    return 1;
  cout << "\ntemporalunitTransformstreamSelect: Wrong type!" << endl;
  return -1;  
}


/*
5.27.5 Definition of operator ~transformstream~

*/

Operator temporalunittransformstream( "transformstream",
                      TemporalUnitTransformstreamSpec,
                      2,
                      temporalunittransformstreammap,
                      temporalunitTransformstreamSelect,
                      TemporalUnitTransformstreamTypeMap);


/*
5.28 Operator ~count~

Signature:

----
     For T in kind DATA:
     (stream T) -> int

----

The operator counts the number of stream elements.

*/

/*
5.28.1 Type mapping function for ~count~

*/

ListExpr
streamCountType( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  string outstr;
  
  if ( nl->ListLength(args) == 1 )
    {
      arg1 = nl->First(args);
      
    if ( !nl->IsAtom(arg1) && nl->ListLength(arg1) == 2 )
      {
        if ( nl->IsEqual(nl->First(arg1), "stream")
             && ( nl->IsAtom(nl->Second(arg1) ) )
             && am->CheckKind("DATA", nl->Second(arg1), errorInfo) )
          return nl->SymbolAtom("int");
        else
          {
            nl->WriteToString(outstr, arg1);
            ErrorReporter::ReportError("Operator count expects a (stream T), "
                                       "T in kind DATA. The argument profided "
                                       "has type '" + outstr + "' instead.");
          }
      }
    }
  nl->WriteToString(outstr, nl->First(args));
  ErrorReporter::ReportError("Operator count expects only a single "
                             "argument of type (stream T), T "
                             "in kind DATA. The argument provided "
                             "has type '" + outstr + "' instead.");
  return nl->SymbolAtom("typeerror");
}

/*
5.28.2 Value mapping for operator ~count~

*/

int
streamCountFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
  Count the number of elements in a stream. An example for consuming a stream.
  
*/
{
  Word elem;
  int count = 0;
  
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, elem);
  
  while ( qp->Received(args[0].addr) )
    {
      count++;
      ((Attribute*) elem.addr)->DeleteIfAllowed();// consume the stream objects
    qp->Request(args[0].addr, elem);
    }
  result = qp->ResultStorage(s);
  ((CcInt*) result.addr)->Set(true, count);
  
  qp->Close(args[0].addr);
  
  return 0;
}

/*
5.28.3 Specification for operator ~count~

*/
const string streamCountSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T)) -> int</text--->"
  "<text>_ count</text--->"
  "<text>Counts the number of elements of a stream.</text--->"
  "<text>query intstream (1,10) count</text--->"
  ") )";

/*
5.28.4 Selection Function of operator ~count~

*/
int
streamCountSelect (ListExpr args ) { return 0; }

/*
5.28.5 Definition of operator ~count~

*/
Operator temporalunitcount (
  "count",           //name
  streamCountSpec,   //specification
  streamCountFun,    //value mapping
  streamCountSelect, //trivial selection function
  streamCountType    //type mapping
);


/*
5.29 Operator ~printstream~

----
    For T in kind DATA:
    (stream T) -> (stream T)

----

For every stream element, the operator calls the ~print~ function  
and passes on the element.

*/

/*
5.29.1 Type mapping function for ~printstream~

*/
ListExpr
streamPrintstreamType( ListExpr args )
{
  ListExpr stream, errorInfo;
  string out;
  
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  stream = nl->First(args);
  
  if ( nl->ListLength(args) != 1 )
    {
      ErrorReporter::ReportError("Operator printstream expects only a single "
                                 "argument.");
      return nl->SymbolAtom("typeerror");
    }
  
  // test first argument for stream(T), T in kind DATA
  if (     nl->IsAtom(stream)
           || !(nl->ListLength(stream) == 2)
           || !nl->IsEqual(nl->First(stream), "stream")
           || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
    {
      nl->WriteToString(out, stream);
      ErrorReporter::ReportError("Operator printstream expects a (stream T), "
                                 "T in kind DATA, as its first argument. "
                                 "The argument provided "
                                 "has type '" + out + "' instead.");
      return nl->SymbolAtom("typeerror");
    }
  
  // return the input type as result
  return stream; 
}

/*
5.29.2 Value mapping for operator ~printstream~

*/
int
streamPrintstreamFun (Word* args, Word& result, 
                      int message, Word& local, Supplier s)
/*
Print the elements of an Attribute-type stream. 
An example for a pure stream operator (input and output are streams).

*/
{
  Word elem;
  
  switch( message )
    {
    case OPEN:
      
      qp->Open(args[0].addr);
      return 0;
      
    case REQUEST:
      
      qp->Request(args[0].addr, elem);
      if ( qp->Received(args[0].addr) )
        {
          ((Attribute*) elem.addr)->Print(cout); cout << endl;
          result = elem;
          return YIELD;
        }
      else return CANCEL;
      
    case CLOSE:
      
      qp->Close(args[0].addr);
      return 0;
    }
  /* should not happen */
  return -1;
}

/*
5.29.3 Specification for operator ~printstream~

*/
const string streamPrintstreamSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T)) -> (stream T)</text--->"
  "<text>_ printstream</text--->"
  "<text>Prints the elements of an arbitrary stream.</text--->"
  "<text>query intstream (1,10) printstream count</text--->"
  ") )";


/*
5.29.4 Selection Function of operator ~printstream~

Uses the same function as for ~count~.

*/


/*
5.29.5 Definition of operator ~printstream~

*/
Operator temporalunitprintstream (
  "printstream",         //name
  streamPrintstreamSpec, //specification
  streamPrintstreamFun,  //value mapping
  streamCountSelect,     //trivial selection function
  streamPrintstreamType  //type mapping
);


/*
5.30 Operator ~sfilter~

----
    For T in kind DATA:
    ((stream T) (map T bool)) -> (stream T)

----

The operator filters the elements of an arbitrary stream by a predicate.

*/

/*
5.30.1 Type mapping function for ~sfilter~

*/
ListExpr
streamFilterType( ListExpr args )
{
  ListExpr stream, map, errorInfo;
  string out, out2;
  
  errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  if ( nl->ListLength(args) == 2 )
    {
      stream = nl->First(args);
      map = nl->Second(args);
      
      // test first argument for stream(T), T in kind DATA
      if ( nl->IsAtom(stream)
           || !(nl->ListLength(stream) == 2)
           || !nl->IsEqual(nl->First(stream), "stream")
           || !am->CheckKind("DATA", nl->Second(stream), errorInfo) )
        {
          nl->WriteToString(out, stream);
          ErrorReporter::ReportError("Operator filter expects a (stream T), "
                                     "T in kind DATA as its first argument. "
                                     "The argument provided "
                                     "has type '" + out + "' instead.");
          return nl->SymbolAtom("typeerror");
        }
      
      // test second argument for map T' bool. T = T'
      if ( nl->IsAtom(map)
           || !nl->ListLength(map) == 3
           || !nl->IsEqual(nl->First(map), "map")
           || !nl->IsEqual(nl->Third(map), "bool") )
        {
          nl->WriteToString(out, map);
          ErrorReporter::ReportError("Operator filter expects a "
                                     "(map T bool), T in kind DATA, "
                                     "as its second argument. "
                                     "The second argument provided "
                                     "has type '" + out + "' instead.");
          return nl->SymbolAtom("typeerror");
        }
      
    if ( !( nl->Equal( nl->Second(stream), nl->Second(map) ) ) )
      {
        nl->WriteToString(out, nl->Second(stream));
        nl->WriteToString(out2, nl->Second(map));
        ErrorReporter::ReportError("Operator filter: the stream base type "
                                   "T must match the map's argument type, "
                                   "e.g. 1st: (stream T), 2nd: (map T bool). "
                                   "The actual types are 1st: '" + out +
                                   "', 2nd: '" + out2 + "'.");
        return nl->SymbolAtom("typeerror");
      }
    }
  else 
    { // wrong number of arguments
      ErrorReporter::ReportError("Operator filter expects two arguments.");
      return nl->SymbolAtom("typeerror");      
    }
  return stream; // return type of first argument
}

/*
5.30.2 Value mapping for operator ~sfilter~

*/
int
streamFilterFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Filter the elements of a stream by a predicate. An example for a stream
operator and also for one calling a parameter function.

*/
{
  Word elem, funresult;
  ArgVectorPointer funargs;
  
  switch( message )
    {
    case OPEN:
      
      qp->Open(args[0].addr);
      return 0;
      
    case REQUEST:

      funargs = qp->Argument(args[1].addr);  //Get the argument vector for
      //the parameter function.
      qp->Request(args[0].addr, elem);
      while ( qp->Received(args[0].addr) )
        {
          (*funargs)[0] = elem;     
          //Supply the argument for the
          //parameter function.
          qp->Request(args[1].addr, funresult);
          //Ask the parameter function
          //to be evaluated.
          if ( ((CcBool*) funresult.addr)->GetBoolval() )
            {
              result = elem;
              return YIELD;
            }
          //consume the stream object:
        ((Attribute*) elem.addr)->DeleteIfAllowed(); 
        qp->Request(args[0].addr, elem); // get next element
        }
      return CANCEL;
      
    case CLOSE:
      
      qp->Close(args[0].addr);
      return 0;
    }
  /* should not happen */
  return -1;
}

/*
5.30.3 Specification for operator ~sfilter~

*/
const string streamFilterSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in kind DATA:\n"
  "((stream T) (map T bool)) -> (stream T)</text--->"
  "<text>_ filter [ fun ]</text--->"
  "<text>Filters the elements of a stream by a predicate.</text--->"
  "<text>query intstream (1,10) filter[. > 7] printintstream count</text--->"
  ") )";

/*
5.30.4 Selection Function of operator ~sfilter~

Uses the same function as for ~count~.

*/

/*
5.30.5 Definition of operator ~sfilter~

*/
Operator streamFilter (
  "filter",            //name
  streamFilterSpec,   //specification
  streamFilterFun,    //value mapping
  streamCountSelect,  //trivial selection function
  streamFilterType    //type mapping
);


/*
5.31 Operator ~no_components~

Return the number of components (units) contained by the object. For unit types,
the result is either undef (for undef values) or a const unit with value=1 
(otherwise). 

----
     n/a + no_components:     (uT) --> uint

----

*/

/*
5.31.1 Type mapping function for ~no_components~

*/

static ListExpr TUNoComponentsTypeMap(ListExpr args) {

  if (nl->ListLength(args) == 1)
    {
      if (nl->IsEqual(nl->First(args), "ubool"))
        return nl->SymbolAtom("uint");
      if (nl->IsEqual(nl->First(args), "uint"))
        return nl->SymbolAtom("uint");
      if (nl->IsEqual(nl->First(args), "ureal"))
        return nl->SymbolAtom("uint");
      if (nl->IsEqual(nl->First(args), "ustring"))
        return nl->SymbolAtom("uint");
      if (nl->IsEqual(nl->First(args), "upoint"))
        return nl->SymbolAtom("uint");
      if (nl->IsEqual(nl->First(args), "uregion"))
        return nl->SymbolAtom("uint");
    }
  return nl->SymbolAtom("typeerror");
}

/*
5.31.2 Value mapping for operator ~no_components~

*/

template<class T>
int TUNoComponentsValueMap(Word* args, Word& result, 
                           int message, Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UInt  *res   = (UInt*)result.addr;
  T     *input = (T*)args[0].addr;
  
  if ( input->IsDefined() )
    {
      res->SetDefined(true);
      res->timeInterval.CopyFrom(input->timeInterval);
      res->constValue.Set(true,1);
    }
  else
    {
      res->SetDefined(false);
      res->constValue.Set(true,0);
    }
  return 0;
}

/*
5.31.3 Specification for operator ~no_components~

*/
const string TUNoComponentsSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For T in {bool, int, real, point, region}:\n"
  "(uT) -> uint</text--->"
  "<text>no_components( _ )</text--->"
  "<text>Returns an undef uint for undef unit, an uint with the "
  "argument's deftime and value '1' otherwise.</text--->"
  "<text>query no_components([const uint value undef])</text--->"
  ") )";

/*
5.31.4 Selection Function of operator ~no_components~

Uses ~UnitSimpleSelect( ListExpr args )~

*/

ValueMapping temporalunitNoComponentsvaluemap[] = {
  TUNoComponentsValueMap<UBool>,
  TUNoComponentsValueMap<UInt>,
  TUNoComponentsValueMap<UReal>,
  TUNoComponentsValueMap<UPoint>,
  TUNoComponentsValueMap<UString>,
  TUNoComponentsValueMap<URegion>};
/*
5.31.5 Definition of operator ~no_components~

*/
Operator temporalunitnocomponents
( 
 "no_components",
 TUNoComponentsSpec,
 6,
 temporalunitNoComponentsvaluemap,
 UnitSimpleSelect,
 TUNoComponentsTypeMap 
);

/*
5.32 Operator ~isempty~

The operator returns TRUE, if the unit is undefined, otherwise it returns FALSE.
It will never return an undefined result!

----
      For U in kind UNIT and U in kind DATA:
      isempty  U --> bool

----

*/

/*
5.32.1 Type mapping function for ~isempty~

*/
ListExpr 
TUIsemptyTypeMap( ListExpr args )
{
  ListExpr arg1;
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  
  if ( ( nl->ListLength(args) == 1 ) && ( nl->IsAtom(nl->First(args) ) ) )
    {
      arg1 = nl->First(args);
      if( am->CheckKind("DATA", arg1, errorInfo) && 
          am->CheckKind("UNIT", arg1, errorInfo)) 
        return arg1;
    }
  ErrorReporter::ReportError("Operator isempty expects a list of length one, "
                             "containing a value of type 'U' with U in "
                             "kind UNIT and in kind DATA.");
  return nl->SymbolAtom( "typeerror" );
}

/*
5.32.2 Value mapping for operator ~isempty~

*/
int TUIsemptyValueMap( Word* args, Word& result, int message,
                       Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Attribute* val = ((Attribute*)args[0].addr);
  if( val->IsDefined() )
    ((CcBool *)result.addr)->Set( true, true );
  else
    ((CcBool *)result.addr)->Set( true, false );
  return 0;
}

/*
5.32.3 Specification for operator ~isempty~

*/
const string TUIsemptySpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>For U in kind UNIT and in kind DATA:\n"
  "U -> bool</text--->"
  "<text>isempty( _ )</text--->"
  "<text>The operator returns TRUE, if the unit is undefined, "
  "otherwise it returns FALSE. It will never return an "
  "undefined result!</text--->"
  "<text>query is_empty([const uint value undef])</text--->"
  ") )";

/*
5.32.4 Selection Function of operator ~isempty~

Simple selection is used, as there is only one value mapping function.

*/

ValueMapping temporalunitIsemptyvaluemap[] = { TUIsemptyValueMap };

/*
5.32.5 Definition of operator ~isempty~

*/
Operator temporalunitisempty
( 
 "isempty",
 TUIsemptySpec,
 1,
 temporalunitIsemptyvaluemap,
 Operator::SimpleSelect,
 TUIsemptyTypeMap
);

/*
5.33 Operator ~not~

Negates an ubool.

----
     not:   ubool --> ubool

----

*/

/*
5.33.1 Type mapping function for ~not~

*/

ListExpr
TUNotTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 1 )
  {
    ListExpr arg1 = nl->First( args );

    if( nl->IsEqual( arg1, "ubool" )  )
      return nl->SymbolAtom( "ubool" );
  }
  return nl->SymbolAtom( "typeerror" );
}
/*
5.33.2 Value mapping for operator ~not~

*/

int TUNotValueMap(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  result = (qp->ResultStorage( s ));
  UBool *res = (UBool*) result.addr;
  UBool *input = (UBool*)args[0].addr;
  
  if(!input->IsDefined())
    res->SetDefined( false );
  else
    {
      res->SetDefined( true );
      res->CopyFrom(input);
      res->constValue.Set(res->constValue.IsDefined(),
                          !(res->constValue.GetBoolval()));
    }
  return 0;
}
/*
5.33.3 Specification for operator ~not~

*/
const string TUNotSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ubool -> ubool</text--->"
  "<text>not( _ )</text--->"
  "<text>The operator returns the logical complement of "
  "its argument. Undef argument resullts in an undef result."
  "</text--->"
  "<text>query not([const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)])</text--->"
  ") )";
/*
5.33.4 Selection Function of operator ~not~

We use simple selection:

*/

ValueMapping temporalunitNotvaluemap[] = { TUNotValueMap };

/*
5.33.5 Definition of operator ~not~

*/
Operator temporalunitnot
( 
 "not",
 TUNotSpec,
 1,
 temporalunitNotvaluemap,
 Operator::SimpleSelect,
 TUNotTypeMap
);

/*
5.34 Operators ~and~ and ~or~

These operators perform the binary conjunction/disjunction for ubool (and bool):

----

    and, or:  ubool x ubool -> ubool
               bool x ubool -> ubool
              ubool x  bool -> ubool
----

*/

/*
5.34.1 Type mapping function for ~and~ and ~or~

We implement a genric function for binary boolean functions here:

*/

ListExpr TUBinaryBoolFuncTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr1, argstr2;
  
  if( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      
      // First case: ubool ubool -> ubool
      if (nl->Equal( arg1, arg2 ))
        {
          if( nl->IsEqual( arg1, "ubool" ) )
            return  arg1;
        }
      
      // Second case: ubool bool -> ubool
      if( nl->IsEqual( arg1, "ubool" ) && nl->IsEqual( arg2, "bool") )
        return  arg1;
      
      // Third case: bool ubool -> ubool
      if( nl->IsEqual( arg1, "bool" ) && nl->IsEqual( arg2, "ubool") )
        return  arg2;
    }
  
  // Error case:
  nl->WriteToString(argstr1, arg1); 
  nl->WriteToString(argstr2, arg2); 
  ErrorReporter::ReportError(
    "Binary booleon operator expects any combination of {bool, ubool} "
    "as arguments, but the passed arguments have types '"+ argstr1 +
    "' and '" + argstr2 + "'.");
  return nl->SymbolAtom("typeerror");   
}
/*
5.34.2 Value mapping for operators ~and~ and ~or~

*/
template<int ArgConf>
int TUAndValueMap(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  assert( (ArgConf>=0) && (ArgConf<=2));

  result = (qp->ResultStorage( s ));
  UBool *res = (UBool*) result.addr;
  UBool *u1, *u2;
  CcBool *cb;
  Interval<Instant> iv;

  if(ArgConf == 0) // case: ubool x ubool -> ubool
    {
      u1 = (UBool*) args[0].addr;
      u2 = (UBool*) args[1].addr;
      
      if  (!u1->IsDefined() || 
           !u2->IsDefined() || 
           !u1->timeInterval.Intersects( u2->timeInterval) )
        res->SetDefined( false );
      else
        {
          u1->timeInterval.Intersection( u2->timeInterval, iv );
          res->SetDefined( true );
          res->timeInterval = iv;
          res->constValue.Set(
            u1->constValue.IsDefined() && u2->constValue.IsDefined(),
            u1->constValue.GetBoolval() && u2->constValue.GetBoolval());
        }
      return 0;
    }
  else if(ArgConf == 1) // case: ubool x bool -> ubool
    {
      u1 = (UBool*)args[0].addr;
      cb = (CcBool*) args[1].addr;
    }
  else if(ArgConf == 1) // case: bool x ubool -> ubool
    {
      u1 = (UBool*)args[1].addr;
      cb = (CcBool*) args[0].addr;
    }
  if (!u1->IsDefined() || !cb->IsDefined())
    res->SetDefined( false );
  else
    {
      res->CopyFrom(u1);
      res->constValue.Set(u1->constValue.IsDefined(),
           u1->constValue.GetBoolval() && cb->GetBoolval() );
    }
  return 0;
}

template<int ArgConf>
int TUOrValueMap(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  assert( (ArgConf>=0) && (ArgConf<=2));

  result = (qp->ResultStorage( s ));
  UBool *res = (UBool*) result.addr;
  UBool *u1, *u2;
  CcBool *cb;
  Interval<Instant> iv;

  if(ArgConf == 0) // case: ubool x ubool -> ubool
    {
      u1 = (UBool*) args[0].addr;
      u2 = (UBool*) args[1].addr;
      
      if  (!u1->IsDefined() || 
           !u2->IsDefined() || 
           !u1->timeInterval.Intersects( u2->timeInterval) )
        res->SetDefined( false );
      else
        {
          u1->timeInterval.Intersection( u2->timeInterval, iv );
          res->SetDefined( true );
          res->timeInterval = iv;
          res->constValue.Set(
            u1->constValue.IsDefined() && u2->constValue.IsDefined(),
            u1->constValue.GetBoolval() || u2->constValue.GetBoolval());
        }
      return 0;
    }
  else if(ArgConf == 1) // case: ubool x bool -> ubool
    {
      u1 = (UBool*)args[0].addr;
      cb = (CcBool*) args[1].addr;
    }
  else if(ArgConf == 1) // case: bool x ubool -> ubool
    {
      u1 = (UBool*)args[1].addr;
      cb = (CcBool*) args[0].addr;
    }
  if (!u1->IsDefined() || !cb->IsDefined())
    res->SetDefined( false );
  else
    {
      res->CopyFrom(u1);
      res->constValue.Set(u1->constValue.IsDefined(),
           u1->constValue.GetBoolval() || cb->GetBoolval() );
    }
  return 0;
}



/*
5.34.3 Specification for operators ~and~ and ~or~

*/
const string TUAndSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(ubool ubool) -> ubool\n"
  "(ubool  bool) -> ubool\n"
  "( bool ubool) -> ubool</text--->"
  "<text>_ and _</text--->"
  "<text>The operator returns the logical conjunction of its "
  "arguments, restricted to their common deftime. Any undefined "
  "argument or non-overlapping intervals result in an "
  "undefined result."
  "</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] and [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUOrSpec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(ubool ubool) -> ubool\n"
  "(ubool  bool) -> ubool\n"
  "( bool ubool) -> ubool</text--->"
  "<text>_ or _</text--->"
  "<text>The operator returns the logical disjunction of its "
  "arguments, restricted to their common deftime. Any undefined "
  "argument or non-overlapping intervals result in an "
  "undefined result."
  "</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] or [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";


/*
5.34.4 Selection Function of operators ~and~ and ~or~

*/
int TUBinaryBoolFuncSelect( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( nl->SymbolValue( arg1 ) == "ubool" 
   && nl->SymbolValue( arg2 ) == "ubool" )
    return 0;
  if( nl->SymbolValue( arg1 ) == "ubool" 
   && nl->SymbolValue( arg2 ) == "bool" )
    return 1;  
  if( nl->SymbolValue( arg1 ) == "bool" 
   && nl->SymbolValue( arg2 ) == "ubool" )
    return 2;
    
  return -1; // This point should never be reached
}

ValueMapping temporalunitAndValuemap[] = { 
  TUAndValueMap<0>,
  TUAndValueMap<1>,
  TUAndValueMap<2>
};

ValueMapping temporalunitOrValuemap[] = { 
  TUOrValueMap<0>,
  TUOrValueMap<1>,
  TUOrValueMap<2> 
};

/*
5.34.5 Definition of operators ~and~ and ~or~

*/
Operator temporalunitand
( 
 "and",
 TUAndSpec,
 1,
 temporalunitAndValuemap,
 TUBinaryBoolFuncSelect,
 TUBinaryBoolFuncTypeMap
);

Operator temporalunitor
( 
 "or",
 TUOrSpec,
 1,
 temporalunitOrValuemap,
 TUBinaryBoolFuncSelect,
 TUBinaryBoolFuncTypeMap
);


/*
5.35 Operator ~ComparePredicates~

Here, we implement the binary comparison operators/predicates for (uT uT). 
The predicates are = (equality), # (unequality), < (smaller than),
> (bigger than), <= (smaller tah or equal to), >= (bigger than or equal to).

The operators use the internat ~Compare~ function, which implements an ordering on the
elements, but does not need to respect inuitive operator semantics (e.g. in case ureal).

----
      =, #, <, >, <=, >=: T in {int, bool, real, string, point, region}
n/a +        uT x uT --> (stream ubool)

----

*/

/*
5.35.1 Type mapping function for ~ComparePredicates~

*/
ListExpr TUComparePredicatesTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  string argstr1, argstr2;
  
  if( nl->ListLength( args ) == 2 )
    {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if (nl->Equal( arg1, arg2 ))
        {
          if( (nl->IsEqual( arg1, "ubool" ) )   ||
              (nl->IsEqual( arg1, "uint" ) )    ||
              (nl->IsEqual( arg1, "ureal" ) )   ||
              (nl->IsEqual( arg1, "upoint" ) )  ||
              (nl->IsEqual( arg1, "ustring" ) ) ||
              (nl->IsEqual( arg1, "uregion" ) ) ||
              (nl->IsEqual( arg1, "upoint" ) ) )
            return nl->SymbolAtom( "bool" );
        }
      
    }
  
  // Error case:
  nl->WriteToString(argstr1, arg1); 
  nl->WriteToString(argstr2, arg2); 
  ErrorReporter::ReportError(
    "Compare Operator (one of =, #, <, <=, >, >=) expects two arguments of "
    "type 'uT', where T in {bool, int, real, string, point, region}. The "
    "passed arguments have types '"+ argstr1 +"' and '"
    + argstr2 + "'.");
  return nl->SymbolAtom("typeerror");   
}
/*
5.35.2 Value mapping for operator ~ComparePredicates~

template parameter ~OPType~ gives the character of the operator: 0 =, 1 #, 2 <, 3 >, 4 <=, 5 >=

*/

template<int OpType>
int TUComparePredicatedValueMap(Word* args, Word& result, int message,
                                Word& local, Supplier s)
{
  assert( (OpType>=0) && (OpType<=5));

  result = (qp->ResultStorage( s ));
  CcBool *res = (CcBool*) result.addr;
  Attribute *u1, *u2;
  int p1;

  u1 = (Attribute*)args[0].addr;
  u2 = (Attribute*)args[1].addr;
  
  p1 = u1->Compare(u2);

  switch (OpType) 
    {
    case 0: // is_equal
      res->Set(true, (p1 == 0));
      return 0;

    case 1: // is_not_equal
      res->Set(true, (p1 != 0));
      return 0;
      
    case 2: // less_than
      res->Set(true, (p1 == -1));
      return 0;

    case 3: // bigger_than
      res->Set(true, (p1 == 1));
      return 0;

    case 4: // less_or_equal
      res->Set(true, (p1 < 1) );
      return 0;
      
    case 5: // bigger_or_equal
      res->Set(true, (p1 > -1));
      return 0;
    }

  res->Set(false, false);
  return -1; // should not happen
}



/*
5.35.3 Specification for operator ~ComparePredicates~

*/
const string TUEq  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ = _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] = [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUNEq  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ # _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] # [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TULt  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ < _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] < [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUBt  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ > _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] > [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TULtEq  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ <= _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] <= [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

const string TUBtEq  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>(uT uT) -> bool\n</text--->"
  "<text>_ >= _</text--->"
  "<text>The operator checks if the internal ordering predicate "
  "holds for both arguments.</text--->"
  "<text>query [const ubool value ((\"2010-11-11\" "
  "\"2011-01-03\" TRUE FALSE) TRUE)] >= [const ubool value "
  "((\"2011-01-01\"2012-09-17\" FALSE TRUE) TRUE)]</text--->"
  ") )";

/*
5.35.4 Selection Function of operator ~ComparePredicates~

We can use Operator::SimpleSelect:

*/

Operator temporalunitisequal
(
 "=",
 TUEq,
 TUComparePredicatedValueMap<0>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitisnotequal
(
 "#",
 TUNEq,
 TUComparePredicatedValueMap<1>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitsmaller
(
 "<",
 TULt,
 TUComparePredicatedValueMap<2>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitbigger
(
 ">",
 TUBt,
 TUComparePredicatedValueMap<3>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitsmallereq
(
 "<=",
 TULtEq,
 TUComparePredicatedValueMap<4>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

Operator temporalunitbiggereq
(
 ">=",
 TUBtEq,
 TUComparePredicatedValueMap<5>,
 Operator::SimpleSelect,
 TUComparePredicatesTypeMap
 );

/*
5.35.5 Definition of operator ~ComparePredicates~

*/

/*
5.36 Operator ~~

----
     (insert signature here)

----

*/

/*
5.36.1 Type mapping function for ~~

*/

/*
5.36.2 Value mapping for operator ~~

*/

/*
5.36.3 Specification for operator ~~

*/

/*
5.36.4 Selection Function of operator ~~

*/

/*
5.36.5 Definition of operator ~~

*/

/*
5.37 Operator ~~

----
     (insert signature here)

----

*/

/*
5.37.1 Type mapping function for ~~

*/

/*
5.37.2 Value mapping for operator ~~

*/

/*
5.37.3 Specification for operator ~~

*/

/*
5.37.4 Selection Function of operator ~~

*/

/*
5.37.5 Definition of operator ~~

*/

/*
5.38 Operator ~~

----
     (insert signature here)

----

*/

/*
5.38.1 Type mapping function for ~~

*/

/*
5.38.2 Value mapping for operator ~~

*/

/*
5.38.3 Specification for operator ~~

*/

/*
5.38.4 Selection Function of operator ~~

*/

/*
5.38.5 Definition of operator ~~

*/

/*
6 Type operators

Type operators are used only for inferring argument types of parameter functions. They have a type mapping but no evaluation function.

*/

/*
6.1 Type Operator ~STREAMELEM~

This type operator extracts the type of the elements from a stream type given as the first argument and otherwise just forwards its type.

----    
     ((stream T1) ...) -> T1
              (T1 ...) -> T1
----

*/
ListExpr
STREAMELEMTypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 1)
  {
    ListExpr first = nl->First(args);
    if (nl->ListLength(first) == 2)
    {
      if (nl->IsEqual(nl->First(first), "stream")) {
        return nl->Second(first);
      }
      else {
        return first;
      }      
    }
    else {
      return first;
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string STREAMELEMSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>((stream T1) ... ) -> T1\n"
      "(T1 ... ) -> T1</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the stream elements if the first "
      "argument is a stream and forwards the first argument's type "
      "otherwise.</text--->"
      "<text>Not for use with sos-syntax</text---> ))";

Operator STREAMELEM (
      "STREAMELEM",
      STREAMELEMSpec,
      0,
      Operator::SimpleSelect,
      STREAMELEMTypeMap );

/*
6.2 Type Operator ~STREAMELEM2~

This type operator extracts the type of the elements from the stream type within the second element within a list of argument types. Otherwise, the first arguments type is simplyforwarded.

----    
     (T1 (stream T2) ...) -> T2
              (T1 T2 ...) -> T2
----

*/
ListExpr
STREAMELEM2TypeMap( ListExpr args )
{
  if(nl->ListLength(args) >= 2)
  {
    ListExpr second = nl->Second(args);
    if (nl->ListLength(second) == 2)
    {
      if (nl->IsEqual(nl->First(second), "stream")) {
        return nl->Second(second);
      }
      else {
        return second;
      }
    }
    else {
      return second;
    }
  }
  return nl->SymbolAtom("typeerror");
}

const string STREAMELEM2Spec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
    "( <text>(T1 (stream T2) ... ) -> T2\n"
      "( T1 T2 ... ) -> T2</text--->"
      "<text>type operator</text--->"
      "<text>Extracts the type of the elements from a stream given "
      "as the second argument if it is a stream. Otherwise, it forwards "
      "the original type.</text--->"
      "<text>Not for use with sos-syntax.</text---> ))";

Operator STREAMELEM2 (
      "STREAMELEM2",
      STREAMELEM2Spec,
      0,
      Operator::SimpleSelect,
      STREAMELEM2TypeMap );


/*
7 Creating the Algebra

*/

class TemporalUnitAlgebra : public Algebra
{
public:
  TemporalUnitAlgebra() : Algebra()
  {
    AddOperator( &temporalunitmakemvalue );
    AddOperator( &temporalunitcount );
    AddOperator( &temporalunitprintstream );
    AddOperator( &temporalunittransformstream );
    AddOperator( &temporalunitsfeed );
    AddOperator( &temporalunitsuse );
    AddOperator( &temporalunitsuse2 );
    AddOperator( &temporalunitsaggregate );
    AddOperator( &temporalunitqueryrect2d );
    AddOperator( &temporalunitpoint2d );
    AddOperator( &temporalcircle );
    AddOperator( &temporalmakepoint );
    AddOperator( &temporalunitisempty );
    AddOperator( &temporalunitdeftime );
    AddOperator( &temporalunitpresent );
    AddOperator( &temporalunitinitial );
    AddOperator( &temporalunitfinal );
    AddOperator( &temporalunitatinstant );
    AddOperator( &temporalunitatperiods );
    AddOperator( &temporalunitat );
    AddOperator( &temporalunitatmax );
    AddOperator( &temporalunitatmin );
    AddOperator( &temporalunitintersection );
    AddOperator( &temporalunitpasses );
    AddOperator( &temporalunitsize );
    AddOperator( &temporalunittrajectory );
    AddOperator( &temporalunitdistance );
    AddOperator( &temporalspeed );
    AddOperator( &temporalvelocity );
    AddOperator( &temporalderivable );
    AddOperator( &temporalderivative );
    AddOperator( &temporalunitnocomponents );
    AddOperator( &temporalunitnot );
    AddOperator( &temporalunitand );
    AddOperator( &temporalunitor );
    AddOperator( &temporalunitisequal );
    AddOperator( &temporalunitisnotequal );
    AddOperator( &temporalunitsmaller );
    AddOperator( &temporalunitbigger );
    AddOperator( &temporalunitsmallereq );
    AddOperator( &temporalunitbiggereq );
    AddOperator( &streamFilter );
    AddOperator( &STREAMELEM );
    AddOperator( &STREAMELEM2 );
  }
  ~TemporalUnitAlgebra() {};
};

TemporalUnitAlgebra temporalUnitAlgebra;

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
InitializeTemporalUnitAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalUnitAlgebra);
}


